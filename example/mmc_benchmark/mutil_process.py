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


is_2d: bool = True
one_block_size = [8 * 1024 * 1024]
local_mem_type: str = 'npu'
process_count: int = 16
one_batch_count: int = 32
call_count: int = 64
#size1 = [64 * 1024 for _ in range(64)]
#size2 = [64 * 1024 for _ in range(64)]
size1 = [128 * 1024 for _ in range(61)]
size2 = [16 * 1024 for _ in range(61)]
block_size = [item for pair in zip(size1, size2) for item in pair]
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
    buffer = torch.empty(total_bytes + padding, dtype=dtype, device=local_mem_type)

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


def write_worker(device: int):
    device_id = device
    set_device(device_id)
    print(f"npu:{device_id} 开始，PID: {os.getpid()}")
    one_tensor = malloc_npu_blocks(max(one_block_size, default=0), 1, one_batch_count)
    tensor1 = malloc_npu_blocks(max(size1, default=0), len(size1), one_batch_count)
    tensor2 = malloc_npu_blocks(max(size2, default=0), len(size2), one_batch_count)
    store = DistributedObjectStore()
    print(f"==== Start to init memcache device:{device_id}")
    res = store.init(device_id)
    if res != 0:
        raise f"Failed to start pid:{os.getpid()} deviceId:{device_id}"
    print(f"==== Success to init device:{device_id}")
    store.register_buffer(one_tensor.data_ptr(), max(one_block_size, default=0) * one_batch_count)
    store.register_buffer(tensor1.data_ptr(), max(size1, default=0) * len(size1) * one_batch_count)
    store.register_buffer(tensor2.data_ptr(), max(size2, default=0) * len(size2) * one_batch_count)
    for i in range(call_count):
        keys = []
        buffs = []
        sizes = []
        for j in range(one_batch_count):
            key = key_prefix + str(device) + '_' + str(i) + '_' + str(j)
            keys.append(key)
            if is_2d is True:
                block_buffs = [item for pair in zip(get_col_tensors_ptr_by_index(tensor1, len(size1), j),
                                                    get_col_tensors_ptr_by_index(tensor2, len(size2), j)) for item in pair]
                sizes.append(block_size)
            else:
                block_buffs = get_col_tensors_ptr_by_index(one_tensor, 1, j)
                sizes.append(one_block_size)
            buffs.append(block_buffs)
        ret = store.batch_put_from_layers(keys, buffs, sizes, 0)
    print(f"===== npu:{device_id} 结束 wait......")
    sleep(30 * 60)


def read_worker(device: int):
    device_id = device
    set_device(device_id)
    status_manager = []
    for i in range(process_count):
        status_manager.append(StatusFileManager(f"task_{i}.txt"))
    status_manager[device_id].reset_to_preparing()
    sleep(5)
    print(f"npu:{device_id} 开始，PID: {os.getpid()}")
    one_tensor = malloc_npu_blocks(max(one_block_size, default=0), 1, one_batch_count)
    tensor1 = malloc_npu_blocks(max(size1, default=0), len(size1), one_batch_count)
    tensor2 = malloc_npu_blocks(max(size2, default=0), len(size2), one_batch_count)
    store = DistributedObjectStore()
    print(f"==== Start to init memcache device:{device_id}")
    res = store.init(device_id)
    if res != 0:
        raise f"Failed to start pid:{os.getpid()} deviceId:{device_id}"
    print(f"==== Success to init device:{device_id}")
    store.register_buffer(one_tensor.data_ptr(), max(one_block_size, default=0) * one_batch_count)
    store.register_buffer(tensor1.data_ptr(), max(size1, default=0) * len(size1) * one_batch_count)
    store.register_buffer(tensor2.data_ptr(), max(size2, default=0) * len(size2) * one_batch_count)
    if local_mem_type == 'npu':
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
        for j in range(one_batch_count):
            key = key_prefix + str(device_id) + '_' + str(i) + '_' + str(j)
            keys.append(key)
            if is_2d is True:
                block_buffs = [item for pair in zip(get_col_tensors_ptr_by_index(tensor1, len(size1), j), get_col_tensors_ptr_by_index(tensor2, len(size2), j)) for item in pair]
                sizes.append(block_size)
            else:
                block_buffs = get_col_tensors_ptr_by_index(one_tensor, 1, j)
                sizes.append(one_block_size)
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
    total_size_bytes = sum(sum(size) for size in sizes) * call_count
    total_size_gb = total_size_bytes / (1024 * 1024 * 1024)
    total_duration_seconds = duration_us / 1_000_000
    bandwidth_gb_per_sec = total_size_gb / total_duration_seconds
    print(f"device_id:{device_id} read total size:{total_size_bytes} bytes, "
          f"single size:{total_size_bytes / call_count} bytes, "
          f"spend time:{duration_us} us, avg:{duration_us / call_count} us, call num:{call_count} "
          f"bw:{bandwidth_gb_per_sec} GB/s")
    sleep(10)
