import os
from time import sleep
from typing import Callable, Dict, List
import torch
import torch_npu

from memcache import DistributedObjectStore

process_count: int = 8
object_count: int = 1024
key_prefix: str = "key_"

def tensor_sum(tensor, sizes: List[int] = None):
    if tensor is None:
        return 0
    if sizes is None:
        return tensor.sum().item()

    return sum(layer[:size].sum().item() for layer, size in zip(tensor, sizes))

def malloc_tensor(layer_num: int = 1, mini_block_size: int = 1024, device='cpu'):
    if device not in ('cpu', 'npu'):
        raise RuntimeError(f"invalid device: {device}")
    if mini_block_size <= 0:
        return None

    raw_blocks = torch.randint(
        low=0, high=256,
        size=(layer_num, mini_block_size),
        dtype=torch.uint8,
        device=torch.device(device)
    )
    torch_npu.npu.current_stream().synchronize()
    return raw_blocks

def set_device(device_id):
    import acl
    acl.init()
    ret = acl.rt.set_device(device_id)
    if ret != 0:
        raise RuntimeError("acl set device failed")

def write_worker(device_id: int):
    set_device(device_id)
    print(f"npu:{device_id} 开始，PID: {os.getpid()}")
    store = DistributedObjectStore()
    res = store.init(device_id)
    if res != 0:
        raise f"Failed to start pid:{os.getpid()} deviceId:{device_id}"
    print("==== Success to init device:{device_id}")
    layer = 122
    size1 = [16 * 1024 for _ in range(61)]
    size2 = [128 * 1024 for _ in range(61)]
    size = size1 + size2
    tensor = malloc_tensor(layer_num=layer, mini_block_size=1024 * 128, device='npu')
    store.register_buffer(tensor.data_ptr(), 1024 * 128 * layer)
    for index in range(object_count):
        key = key_prefix + str(device_id) + '_' + str(index)
        tensor.fill_(index + 1)
        torch_npu.npu.current_stream().synchronize()
        store.put_from_layers(key, [] if tensor is None else [layer.data_ptr() for layer in tensor], size, 0)
        print(f"==== key({key}) res({res}) sum({tensor_sum(tensor, size)})")
    print(f"===== npu:{device_id} 结束 wait......")
    sleep(30 * 60)


def read_worker(device_id: int):
    set_device(device_id)
    print(f"npu:{device_id} 开始，PID: {os.getpid()}")
    store = DistributedObjectStore()
    res = store.init(device_id)
    if res != 0:
        raise f"Failed to start pid:{os.getpid()} deviceId:{device_id}"
    print("==== Success to init device:{device_id}")
    layer = 122
    size1 = [16 * 1024 for _ in range(61)]
    size2 = [128 * 1024 for _ in range(61)]
    size = size1 + size2
    tensor = malloc_tensor(layer_num=layer, mini_block_size=1024 * 128, device='npu')
    store.register_buffer(tensor.data_ptr(), 1024 * 128 * layer)
    for index in range(object_count):
        key = key_prefix + str(device_id) + '_' + str(index)
        res = store.get_into_layers(key, [] if tensor is None else [layer.data_ptr() for layer in tensor], size, 1)
        print(f"==== key({key}) res({res}) sum({tensor_sum(tensor, size)})")
    print(f"==== npu:{device_id} 结束 wait......")
    sleep(30 * 60)
