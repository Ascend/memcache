#!/usr/bin/env python
# coding=utf-8
# Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
# MemCache_Hybrid is licensed under Mulan PSL v2.
# You can use this software according to the terms and conditions of the Mulan PSL v2.
# You may obtain a copy of Mulan PSL v2 at:
#          http://license.coscl.org.cn/MulanPSL2
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
# EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
# MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
# See the Mulan PSL v2 for more details.

import os
import time
from time import sleep
from typing import List
import torch
import torch_npu

from status_file_manager import StatusFileManager
from memcache_hybrid import DistributedObjectStore

g_local_type = "npu"

layers_num = 61
k_size = 64 * 1024
v_size = 16 * 1024
k_sizes = [k_size for _ in range(layers_num)]
v_sizes = [v_size for _ in range(layers_num)]
layers_block_size = [item for pair in zip(k_sizes, v_sizes) for item in pair]
key_prefix: str = "key_"


def set_device(device_id):
    import acl
    acl.init()
    ret = acl.rt.set_device(device_id)
    if ret != 0:
        raise RuntimeError("acl set device failed")


def tensor_sum(tensor: List[torch.Tensor], sizes: List[int] = None):
    if tensor is None:
        return 0
    if sizes is None:
        return sum(layer.sum().item() for layer in tensor)
    return sum(layer[:size].sum().item() for layer, size in zip(tensor, sizes))


def allocate_aligned_tensor(shape, dtype=torch.float32, alignment=2*1024*1024):
    num_elements = torch.prod(torch.tensor(shape)).item()
    element_size = torch.finfo(dtype).bits // 8 if dtype.is_floating_point else torch.iinfo(dtype).bits // 8
    total_bytes = num_elements * element_size

    padding = alignment - 1
    buffer = torch.empty(total_bytes + padding, dtype=dtype, device=g_local_type)

    address = buffer.data_ptr()
    aligned_address = (address + alignment - 1) & ~(alignment - 1)
    offset = (aligned_address - address) // element_size

    aligned_tensor = buffer[offset:offset + num_elements].view(*shape)
    print(f"Aligned tensor address: {aligned_tensor.data_ptr():x}")
    return aligned_tensor


def malloc_npu_blocks(min_block_size: int, layer_num: int, block_num: int):
    raw_blocks = allocate_aligned_tensor((layer_num, block_num, min_block_size), torch.uint8)
    torch_npu.npu.current_stream().synchronize()
    return raw_blocks


def get_col_tensors_by_index(tensors, layer_num, block_index):
    block_tensor = []
    for li in range(layer_num):
        block_tensor.append(tensors[li][block_index])
    return block_tensor


def get_col_tensors_ptr_by_index(tensors, layer_num, block_index):
    block_ptrs = []
    for li in range(layer_num):
        block_ptrs.append(tensors[li][block_index].data_ptr())
    return block_ptrs


def init_mooncake(device_id: int):
    from mooncake_store import Mooncakestore, MooncakeConfig
    config = MooncakeConfig(
        device=device_id,
        protocol='rdma',
        device_name='',
        local_hostname='29.210.144.83',
        metadata_server='P2PHANDSHAKE',
        global_segment_size=1024 * 1024 * 1024 * 64,
        local_buffer_size=128 * 1024 * 1024,
        master_server_address='29.210.144.84:50051')
    store = Mooncakestore(config)
    return store


def write_worker(*args):
    device_id = args[0]
    batch_size = args[1]
    block_size = [args[2]]
    call_count = args[3]
    data_dim = args[4]
    backend = args[5]
    global g_local_type
    g_local_type = args[6]
    process_count = args[7]

    set_device(device_id)
    status_manager = []
    for i in range(process_count):
        status_manager.append(StatusFileManager(f"wtask_{i}.txt"))
    status_manager[device_id].reset_to_preparing()

    print(f"npu:{device_id} 开始，PID: {os.getpid()}")
    continus_tensor = malloc_npu_blocks(max(block_size, default=0), 1, batch_size)
    k_tensors = malloc_npu_blocks(max(k_sizes, default=0), len(k_sizes), batch_size)
    v_tensors = malloc_npu_blocks(max(v_sizes, default=0), len(v_sizes), batch_size)
    if backend == "mooncake":
        store = init_mooncake(device_id)
        print(f"==== Start to init mooncake device:{device_id}")
    else:
        store = DistributedObjectStore()
        print(f"==== Start to init memcache device:{device_id}")
        res = store.init(device_id)
        if res != 0:
            raise f"Failed to start pid:{os.getpid()} deviceId:{device_id}"
    print(f"==== Success to init device:{device_id}")
    store.register_buffer(continus_tensor.data_ptr(), max(block_size, default=0) * batch_size)
    store.register_buffer(k_tensors.data_ptr(), max(k_sizes, default=0) * len(k_sizes) * batch_size)
    store.register_buffer(v_tensors.data_ptr(), max(v_sizes, default=0) * len(v_sizes) * batch_size)
    keys_list = []
    buffs_list = []
    sizes_list = []    
    for i in range(call_count):
        keys = []
        buffs = []
        sizes = []
        for j in range(batch_size):
            key = key_prefix + str(device_id) + '_' + str(i) + '_' + str(j)
            keys.append(key)
            if data_dim == 2:
                block_buffs = [item for pair in zip(get_col_tensors_ptr_by_index(k_tensors, len(k_sizes), j),
                                                    get_col_tensors_ptr_by_index(v_tensors, len(v_sizes), j)) 
                                                    for item in pair]
                sizes.append(layers_block_size)
            else:
                block_buffs = get_col_tensors_ptr_by_index(continus_tensor, 1, j)
                sizes.append(block_size)
            buffs.append(block_buffs)
        keys_list.append(keys)
        buffs_list.append(buffs)
        sizes_list.append(sizes)

    status_manager[device_id].set_to_ready()
    for i in range(process_count):
        status_manager[i].wait_until_ready(timeout=5 * 60)
    
    start = time.perf_counter()
    print(f"===== npu:{device_id} begin on {start} ......")

    for keys, buffs, sizes in zip(keys_list, buffs_list, sizes_list):
        ret = store.batch_put_from_layers(keys, buffs, sizes, 0)

    print(f"===== npu:{device_id} finish on {time.perf_counter()} ......")
    status_manager[device_id].reset_to_preparing()
    for i in range(process_count):
        status_manager[i].wait_until_ready(timeout=5 * 60, check_ready=False)

    end = time.perf_counter()
    duration_us = (end - start) * 1_000_000
    total_size_bytes = sum(sum(size) for size in sizes for sizes in sizes_list)
    total_size_gb = total_size_bytes / (1024 * 1024 * 1024)
    total_duration_seconds = duration_us / 1_000_000
    bandwidth_gb_per_sec = total_size_gb / total_duration_seconds
    print(f"\033[91mdevice_id:{device_id} write_total_size:{total_size_bytes} bytes, "
          f"single_size:{total_size_bytes / call_count:.0f} bytes, call count:{call_count}, "
          f"total_time:{duration_us:.2f} us, avg_time:{duration_us / call_count:.2f} us, "
          f"bw:{bandwidth_gb_per_sec:.3f} GB/s\033[0m")

    print(f"===== npu:{device_id} write finish, wait read testing ......")
    sleep(30 * 60)


def read_worker(*args):
    device_id = args[0]
    batch_size = args[1]
    block_size = [args[2]]
    call_count = args[3]
    data_dim = args[4]
    backend = args[5]
    global g_local_type
    g_local_type = args[6]
    process_count = args[7]

    set_device(device_id)
    status_manager = []
    for i in range(process_count):
        status_manager.append(StatusFileManager(f"task_{i}.txt"))
    status_manager[device_id].reset_to_preparing()
    sleep(5)
    print(f"npu:{device_id} 开始，PID: {os.getpid()}")
    continus_tensor = malloc_npu_blocks(max(block_size, default=0), 1, batch_size)
    k_tensors = malloc_npu_blocks(max(k_sizes, default=0), len(k_sizes), batch_size)
    v_tensors = malloc_npu_blocks(max(v_sizes, default=0), len(v_sizes), batch_size)
    
    if backend == "mooncake":
        store = init_mooncake(device_id)
        print(f"==== Start to init mooncake device:{device_id}")
    else:
        store = DistributedObjectStore()
        print(f"==== Start to init memcache device:{device_id}")
        res = store.init(device_id)
        if res != 0:
            raise f"Failed to start pid:{os.getpid()} deviceId:{device_id}"
    print(f"==== Success to init device:{device_id}")
    store.register_buffer(continus_tensor.data_ptr(), max(block_size, default=0) * batch_size)
    store.register_buffer(k_tensors.data_ptr(), max(k_sizes, default=0) * len(k_sizes) * batch_size)
    store.register_buffer(v_tensors.data_ptr(), max(v_sizes, default=0) * len(v_sizes) * batch_size)
    if g_local_type == "npu":
        direct_t = 1
    else:
        direct_t = 2
    # 实测此步骤很耗时，提前准备
    keys_list = []
    buffs_list = []
    sizes_list = []
    for i in range(call_count):
        keys = []
        buffs = []
        sizes = []
        for j in range(batch_size):
            key = key_prefix + str(device_id) + '_' + str(i) + '_' + str(j)
            keys.append(key)
            if data_dim == 2:
                block_buffs = [item for pair in zip(get_col_tensors_ptr_by_index(k_tensors, len(k_sizes), j),
                                                    get_col_tensors_ptr_by_index(v_tensors, len(v_sizes), j)) 
                                                    for item in pair]
                sizes.append(layers_block_size)
            else:
                block_buffs = get_col_tensors_ptr_by_index(continus_tensor, 1, j)
                sizes.append(block_size)
            buffs.append(block_buffs)
        
        keys_list.append(keys)
        buffs_list.append(buffs)
        sizes_list.append(sizes)

    status_manager[device_id].set_to_ready()
    for i in range(process_count):
        status_manager[i].wait_until_ready(timeout=5 * 60)
    
    start = time.perf_counter()
    print(f"===== npu:{device_id} begin on {start} ......")
    for keys, buffs, sizes in zip(keys_list, buffs_list, sizes_list):
        ret = store.batch_get_into_layers(keys, buffs, sizes, direct_t)
    print(f"===== npu:{device_id} finish on {time.perf_counter()} ......")

    status_manager[device_id].reset_to_preparing()
    for i in range(process_count):
        status_manager[i].wait_until_ready(timeout=5 * 60, check_ready=False)
    end = time.perf_counter()
    duration_us = (end - start) * 1_000_000
    total_size_bytes = sum(sum(size) for size in sizes for sizes in sizes_list)
    total_size_gb = total_size_bytes / (1024 * 1024 * 1024)
    total_duration_seconds = duration_us / 1_000_000
    bandwidth_gb_per_sec = total_size_gb / total_duration_seconds
    print(f"\033[91mdevice_id:{device_id} read_total_size:{total_size_bytes} bytes, "
          f"single_size:{total_size_bytes / call_count:.0f} bytes, call count:{call_count}, "
          f"total_time:{duration_us:.2f} us, avg_time:{duration_us / call_count:.2f} us, "
          f"bw:{bandwidth_gb_per_sec:.3f} GB/s\033[0m")
    sleep(10)
