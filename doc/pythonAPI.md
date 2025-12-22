### Python接口

安装完成run包并source安装路径下的set_env.sh后，即可在python中通过

```python
from memcache_hybrid import DistributedObjectStore
```

DistributedObjectStore类，然后调用python接口

### python接口文档

```shell
pydoc memcache_hybrid
```

结果如下

```text
Help on package memcache_hybrid:

NAME
    memcache_hybrid

DESCRIPTION
    # coding=utf-8
    # Copyright (c) Huawei Technologies Co., Ltd. 2025-2026. All rights reserved.

PACKAGE CONTENTS
    _pymmc
    meta_service_leader_election

CLASSES
    pybind11_builtins.pybind11_object(builtins.object)
        _pymmc.DistributedObjectStore
        _pymmc.KeyInfo
        _pymmc.MetaService
        _pymmc.ReplicateConfig

    class DistributedObjectStore(pybind11_builtins.pybind11_object)
     |  Method resolution order:
     |      DistributedObjectStore
     |      pybind11_builtins.pybind11_object
     |      builtins.object
     |
     |  Methods defined here:
     |
     |  __init__(...)
     |      __init__(self: _pymmc.DistributedObjectStore) -> None
     |
     |  batch_get_into(...)
     |      batch_get_into(self: _pymmc.DistributedObjectStore, keys: List[str], buffer_ptrs: List[int], sizes: List[int], direct: int = <MmcCopyDirect.SMEMB_COPY_G2H: 2>) -> List[int]
     |
     |      Get object data directly into pre-allocated buffers for multiple keys
     |
     |  batch_get_into_layers(...)
     |      batch_get_into_layers(self: _pymmc.DistributedObjectStore, keys: List[str], buffer_ptrs: List[List[int]], sizes: List[List[int]], direct: int = <MmcCopyDirect.SMEMB_COPY_G2H: 2>) -> List[int]
     |
     |  batch_get_key_info(...)
     |      batch_get_key_info(self: _pymmc.DistributedObjectStore, keys: List[str]) -> List[_pymmc.KeyInfo]
     |
     |  batch_is_exist(...)
     |      batch_is_exist(self: _pymmc.DistributedObjectStore, keys: List[str]) -> List[int]
     |
     |      Check if multiple objects exist. Returns list of results: 1 if exists, 0 if not exists, -1 if error
     |
     |  batch_put_from(...)
     |      batch_put_from(self: _pymmc.DistributedObjectStore, keys: List[str], buffer_ptrs: List[int], sizes: List[int], direct: int = <MmcCopyDirect.SMEMB_COPY_H2G: 3>, replicateConfig: _pymmc.ReplicateConfig = <_pymmc.ReplicateConfig object at 0xffffa0b46230>) -> List[int]
     |
     |      Put object data directly from pre-allocated buffers for multiple keys
     |
     |  batch_put_from_layers(...)
     |      batch_put_from_layers(self: _pymmc.DistributedObjectStore, keys: List[str], buffer_ptrs: List[List[int]], sizes: List[List[int]], direct: int = <MmcCopyDirect.SMEMB_COPY_H2G: 3>, replicateConfig: _pymmc.ReplicateConfig = <_pymmc.ReplicateConfig object at 0xffffa0b467f0>) ->
 List[int]
     |
     |  close(...)
     |      close(self: _pymmc.DistributedObjectStore) -> int
     |
     |  get(...)
     |      get(self: _pymmc.DistributedObjectStore, arg0: str) -> bytes
     |
     |  get_into(...)
     |      get_into(self: _pymmc.DistributedObjectStore, key: str, buffer_ptr: int, size: int, direct: int = <MmcCopyDirect.SMEMB_COPY_G2H: 2>) -> int
     |
     |      Get object data directly into a pre-allocated buffer
     |
     |  get_into_layers(...)
     |      get_into_layers(self: _pymmc.DistributedObjectStore, key: str, buffer_ptrs: List[int], sizes: List[int], direct: int = <MmcCopyDirect.SMEMB_COPY_G2H: 2>) -> int
     |
     |  get_key_info(...)
     |      get_key_info(self: _pymmc.DistributedObjectStore, arg0: str) -> _pymmc.KeyInfo
     |
     |  get_local_service_id(...)
     |      get_local_service_id(self: _pymmc.DistributedObjectStore) -> int
     |
     |      Get local serviceId
     |
     |  init(...)
     |      init(self: _pymmc.DistributedObjectStore, device_id: int) -> int
     |
     |  is_exist(...)
     |      is_exist(self: _pymmc.DistributedObjectStore, arg0: str) -> int
     |
     |  put(...)
     |      put(self: _pymmc.DistributedObjectStore, key: str, buf: buffer, replicateConfig: _pymmc.ReplicateConfig = <_pymmc.ReplicateConfig object at 0xffffa0b285b0>) -> int
     |
     |  put_from(...)
     |      put_from(self: _pymmc.DistributedObjectStore, key: str, buffer_ptr: int, size: int, direct: int = <MmcCopyDirect.SMEMB_COPY_H2G: 3>, replicateConfig: _pymmc.ReplicateConfig = <_pymmc.ReplicateConfig object at 0xffffa0b45ef0>) -> int
     |
     |      Put object data directly from a pre-allocated buffer
     |
     |  put_from_layers(...)
     |      put_from_layers(self: _pymmc.DistributedObjectStore, key: str, buffer_ptrs: List[int], sizes: List[int], direct: int = <MmcCopyDirect.SMEMB_COPY_H2G: 3>, replicateConfig: _pymmc.ReplicateConfig = <_pymmc.ReplicateConfig object at 0xffffa0b464f0>) -> int
     |
     |  register_buffer(...)
     |      register_buffer(self: _pymmc.DistributedObjectStore, buffer_ptr: int, size: int) -> int
     |
     |      Register a memory buffer for direct access operations
     |
     |  remove(...)
     |      remove(self: _pymmc.DistributedObjectStore, arg0: str) -> int
     |
     |  remove_batch(...)
     |      remove_batch(self: _pymmc.DistributedObjectStore, arg0: List[str]) -> List[int]
     |
     |  ----------------------------------------------------------------------
     |  Static methods inherited from pybind11_builtins.pybind11_object:
     |
     |  __new__(*args, **kwargs) class method of pybind11_builtins.pybind11_object
     |      Create and return a new object.  See help(type) for accurate signature.

    class KeyInfo(pybind11_builtins.pybind11_object)
     |  Method resolution order:
     |      KeyInfo
     |      pybind11_builtins.pybind11_object
     |      builtins.object
     |
     |  Methods defined here:
     |
     |  __init__(self, /, *args, **kwargs)
     |      Initialize self.  See help(type(self)) for accurate signature.
     |
     |  __repr__(...)
     |      __repr__(self: _pymmc.KeyInfo) -> str
     |
     |  __str__(...)
     |      __str__(self: _pymmc.KeyInfo) -> str
     |
     |  loc_list(...)
     |      loc_list(self: _pymmc.KeyInfo) -> List[int]
     |
     |  size(...)
     |      size(self: _pymmc.KeyInfo) -> int
     |
     |  type_list(...)
     |      type_list(self: _pymmc.KeyInfo) -> List[int]
     |
     |  ----------------------------------------------------------------------
     |  Static methods inherited from pybind11_builtins.pybind11_object:
     |
     |  __new__(*args, **kwargs) class method of pybind11_builtins.pybind11_object
     |      Create and return a new object.  See help(type) for accurate signature.

    class MetaService(pybind11_builtins.pybind11_object)
     |  Method resolution order:
     |      MetaService
     |      pybind11_builtins.pybind11_object
     |      builtins.object
     |
     |  Methods defined here:
     |
     |  __init__(self, /, *args, **kwargs)
     |      Initialize self.  See help(type(self)) for accurate signature.
     |
     |  ----------------------------------------------------------------------
     |  Static methods defined here:
     |
     |  main(...) method of builtins.PyCapsule instance
     |      main() -> int
     |
     |      Start the meta service process directly. This is a blocking call.
     |
     |  ----------------------------------------------------------------------
     |  Static methods inherited from pybind11_builtins.pybind11_object:
     |
     |  __new__(*args, **kwargs) class method of pybind11_builtins.pybind11_object
     |      Create and return a new object.  See help(type) for accurate signature.

    class ReplicateConfig(pybind11_builtins.pybind11_object)
     |  Configuration for replica allocation policy.
     |
     |  Method resolution order:
     |      ReplicateConfig
     |      pybind11_builtins.pybind11_object
     |      builtins.object
     |
     |  Methods defined here:
     |
     |  __init__(...)
     |      __init__(self: _pymmc.ReplicateConfig) -> None
     |
     |
     |      Default constructor.
     |      Initializes:
     |        - preferredLocalServiceIDs = {}
     |        - replicaNum = 0
     |
     |  ----------------------------------------------------------------------
     |  Data descriptors defined here:
     |
     |  preferredLocalServiceIDs
     |      List of instance IDs for forced storage. The values in the list must be unique,
     |      and the list size must be equal to replicaNum.
     |
     |  replicaNum
     |      Less than or equal to 8.
     |
     |  ----------------------------------------------------------------------
     |  Static methods inherited from pybind11_builtins.pybind11_object:
     |
     |  __new__(*args, **kwargs) class method of pybind11_builtins.pybind11_object
     |      Create and return a new object.  See help(type) for accurate signature.

DATA
    __all__ = ['DistributedObjectStore', 'ReplicateConfig', 'KeyInfo', 'Me...

FILE
    /usr/local/lib/python3.11/site-packages/memcache_hybrid/__init__.py
```

## Python 接口列表

说明：Python接口只封装了客户端，需要配置metaservice独立进程使用

### DistributedObjectStore 类

DistributedObjectStore是分布式对象存储的Python接口封装类，提供了完整的分布式内存缓存操作接口。

#### \_\_init\_\_

```python
store = DistributedObjectStore()
```

**功能**: 创建DistributedObjectStore实例

#### init

```python
result = store.init(device_id)
```

**功能**: 初始化分布式内存缓存客户端

**参数**:

- `device_id`: 使用HBM时的NPU卡用户ID（支持ASCEND_RT_VISIBLE_DEVICES映射）

**返回值**:

- `0`: 成功
- 其他: 失败

#### get_local_service_id

```python
store.get_local_service_id()
```

**功能**: Get local serviceId

#### close

```python
store.close()
```

**功能**: 关闭并清理分布式内存缓存客户端

#### put

```python
result = store.put(key, data, replicateConfig=defaultConfig)
```

**功能**: 将指定key的数据写入分布式内存缓存中

**参数**:

- `key`: 数据的键，字符串类型
- `data`: 要存储的字节数据
- `replicateConfig`: 具体看ReplicateConfig数据结构

**返回值**:

- `0`: 成功
- 其他: 失败

#### put_from

```python
result = store.put_from(key, buffer_ptr, size, direct=SMEMB_COPY_H2G, replicateConfig=defaultConfig)
```

**功能**: 从预分配的缓冲区中写入数据

**参数**:

- `key`: 数据的键
- `buffer_ptr`: 缓冲区指针
- `size`: 数据大小
- `direct`: 数据拷贝方向，可选值：
    - `SMEMB_COPY_H2G`: 从主机内存到全局内存（默认）
    - `SMEMB_COPY_L2G`: 从卡上内存到全局内存
- `replicateConfig`: 具体看ReplicateConfig数据结构

**返回值**:

- `0`: 成功
- 其他: 失败

#### batch_put_from

```python
result = store.batch_put_from(keys, buffer_ptrs, sizes, direct=SMEMB_COPY_H2G, replicateConfig=defaultConfig)
```

**功能**: 从预分配的缓冲区中批量写入数据

**参数**:

- `keys`: 键列表
- `buffer_ptrs`: 缓冲区指针列表
- `sizes`: 数据大小列表
- `direct`: 数据拷贝方向，可选值：
    - `SMEMB_COPY_H2G`: 从主机内存到全局内存（默认）
    - `SMEMB_COPY_L2G`: 从卡上内存到全局内存
- `replicateConfig`: 具体看ReplicateConfig数据结构

**返回值**:

- 结果列表，每个元素表示对应写入操作的结果
    - `0`: 成功
    - 其他: 错误

#### put_from_layers

```python
result = store.put_from_layers(key, buffer_ptrs, sizes, direct=SMEMB_COPY_H2G, replicateConfig=defaultConfig)
```

**功能**: 从多个预分配的缓冲区中写入数据（分层数据）

**参数**:

- `key`: 数据的键
- `buffer_ptrs`: 缓冲区指针列表，每个指针对应一个数据层
- `sizes`: 数据大小列表，每个元素对应一个数据层的大小
- `direct`: 数据拷贝方向，可选值：
    - `SMEMB_COPY_H2G`: 从主机内存到全局内存（默认）
    - `SMEMB_COPY_L2G`: 从卡上内存到全局内存
- `replicateConfig`: 具体看ReplicateConfig数据结构

**返回值**:

- `0`: 成功
- 其他: 失败

#### batch_put_from_layers

```python
result = store.batch_put_from_layers(keys, buffer_ptrs_list, sizes_list, direct=SMEMB_COPY_H2G,
                                     replicateConfig=defaultConfig)
```

**功能**: 从多个预分配的缓冲区中批量写入分层数据

**参数**:

- `keys`: 数据键列表，每个键对应一个分层数据对象
- `buffer_ptrs_list`: 缓冲区指针二维列表，外层列表对应每个键，内层列表对应每个键的各个数据层指针
- `sizes_list`: 数据大小二维列表，外层列表对应每个键，内层列表对应每个键的各个数据层大小
- `direct`: 数据拷贝方向，可选值：
    - `SMEMB_COPY_H2G`: 从主机内存到全局内存（默认）
    - `SMEMB_COPY_L2G`: 从卡上内存到全局内存
- `replicateConfig`: 具体看ReplicateConfig数据结构

**返回值**:

- 结果列表，每个元素表示对应写入操作的结果
    - `0`: 成功
    - 其他: 错误

#### get

```python
data = store.get(key)
```

**功能**: 获取指定key的数据

**参数**:

- `key`: 数据的键

**返回值**:

- 成功时返回数据字节串
- 失败时返回空字节串

#### get_into

```python
result = store.get_into(key, buffer_ptr, size, direct=SMEMB_COPY_G2H)
```

**功能**: 将数据直接获取到预分配的缓冲区中

**参数**:

- `key`: 数据的键
- `buffer_ptr`: 目标缓冲区指针
- `size`: 缓冲区大小
- `direct`: 数据拷贝方向，可选值：
    - `SMEMB_COPY_G2H`: 从全局内存到主机内存（默认）
    - `SMEMB_COPY_G2L`: 从全局内存到卡上内存

**返回值**:

- `0`: 成功
- 其他: 失败

#### batch_get_into

```python
results = store.batch_get_into(keys, buffer_ptrs, sizes, direct=SMEMB_COPY_G2H)
```

**功能**: 批量将数据获取到预分配的缓冲区中

**参数**:

- `keys`: 键列表
- `buffer_ptrs`: 缓冲区指针列表
- `sizes`: 缓冲区大小列表
- `direct`: 数据拷贝方向，可选值：
    - `SMEMB_COPY_G2H`: 从全局内存到主机内存（默认）
    - `SMEMB_COPY_G2L`: 从全局内存到卡上内存

**返回值**:

- 结果列表，每个元素表示对应操作的结果
    - `0`: 成功
    - 其他: 错误

#### get_into_layers

```python
result = store.get_into_layers(key, buffer_ptrs, sizes, direct=SMEMB_COPY_G2H)
```

**功能**: 将数据分层获取到预分配的缓冲区中

**参数**:

- `key`: 数据的键
- `buffer_ptrs`: 目标缓冲区指针列表，每个指针对应一个数据层
- `sizes`: 缓冲区大小列表，每个元素对应一个数据层的大小
- `direct`: 数据拷贝方向，可选值：
    - `SMEMB_COPY_G2H`: 从全局内存到主机内存（默认）
    - `SMEMB_COPY_G2L`: 从全局内存到卡上内存

**返回值**:

- `0`: 成功
- 其他: 失败

#### batch_get_into_layers

```python
results = store.batch_get_into_layers(keys, buffer_ptrs_list, sizes_list, direct=SMEMB_COPY_G2H)
```

**功能**: 批量将分层数据获取到预分配的缓冲区中

**参数**:

- `keys`: 数据键列表，每个键对应一个分层数据对象
- `buffer_ptrs_list`: 缓冲区指针二维列表，外层列表对应每个键，内层列表对应每个键的各个目标数据层指针
- `sizes_list`: 缓冲区大小二维列表，外层列表对应每个键，内层列表对应每个键的各个数据层大小
- `direct`: 数据拷贝方向，可选值：
    - `SMEMB_COPY_G2H`: 从全局内存到主机内存（默认）
    - `SMEMB_COPY_G2L`: 从全局内存到卡上内存

**返回值**:

- 结果列表，每个元素表示对应操作的结果
    - `0`: 成功
    - 其他: 错误

#### remove

```python
result = store.remove(key)
```

**功能**: 删除指定key的数据

**参数**:

- `key`: 要删除的数据键

**返回值**:

- `0`: 成功
- 其他: 失败

#### remove_batch

```python
results = store.remove_batch(keys)
```

**功能**: 批量删除数据

**参数**:

- `keys`: 要删除的键列表

**返回值**:

- 结果列表，每个元素表示对应删除操作的结果
    - `0`: 成功
    - 其他: 错误

#### is_exist

```python
result = store.is_exist(key)
```

**功能**: 检查指定key是否存在

**参数**:

- `key`: 要检查的键

**返回值**:

- `1`: 存在
- `0`: 不存在
- 其他: 错误

#### batch_is_exist

```python
results = store.batch_is_exist(keys)
```

**功能**: 批量检查键是否存在

**参数**:

- `keys`: 要检查的键列表

**返回值**:

- 结果列表，每个元素表示对应键的存在状态：
    - `1`: 存在
    - `0`: 不存在
    - 其他: 错误

#### get_key_info

```python
key_info = store.get_key_info(key)
```

**功能**: 获取指定key的数据信息

**参数**:

- `key`: 数据的键

**返回值**:

- `KeyInfo`对象，包含以下方法：
    - `size()`: 获取数据大小
    - `loc_list()`: 获取数据位置列表
    - `type_list()`: 获取数据类型列表
    - `__str__()`: 获取信息的字符串表示

#### batch_get_key_info

```python
key_infos = store.batch_get_key_info(keys)
```

**功能**: 批量获取多个key的数据信息

**参数**:

- `keys`: 数据键列表

**返回值**:

- `KeyInfo`对象列表，每个对象包含以下方法：
    - `size()`: 获取数据大小
    - `loc_list()`: 获取数据位置列表
    - `type_list()`: 获取数据类型列表
    - `__str__()`: 获取信息的字符串表示

#### register_buffer

```python
result = store.register_buffer(buffer_ptr, size)
```

**功能**: 注册内存缓冲区，用于Device RDMA的加速操作

**参数**:

- `buffer_ptr`: 缓冲区指针
- `size`: 缓冲区大小

**返回值**:

- `0`: 成功
- 其他: 失败

#### unregister_buffer

```python
result = store.unregister_buffer(buffer_ptr, size)
```

**功能**: 注销内存缓冲区，用于Device RDMA的加速操作

**参数**:

- `buffer_ptr`: 缓冲区指针
- `size`: 缓冲区大小

**返回值**:

- `0`: 成功
- 其他: 失败

### 使用示例

```python
store = DistributedObjectStore()
store.init(device_id=0)
replica_cfg = ReplicateConfig()
replica_cfg.replicaNum = 3
replica_cfg.preferredLocalServiceIDs = [101, 102, 103]
buffer = bytearray(b"example_data")
store.put("key1", buffer, replica_cfg)
data = store.get("key1")
store.close()
```

## 文档中涉及到的结构体含义

### MmcCopyDirect 枚举类型

用于指定数据拷贝方向的枚举类型：

- `SMEMB_COPY_H2G`: 从主机内存到全局内存
- `SMEMB_COPY_L2G`: 从卡上内存到全局内存
- `SMEMB_COPY_G2H`: 从全局内存到主机内存
- `SMEMB_COPY_G2L`: 从全局内存到卡上内存

## 数据结构

### ReplicateConfig

客户端配置结构体，包含以下字段：

- `replicaNum`: 副本数，最大为8，默认为1
- `preferredLocalServiceIDs`: 强制分配的实例id列表，列表大小必须小于或等于replicaNum

## 错误码

| 值     | 说明        |
|-------|-----------|
| 0     | 操作成功      |
| -1    | 一般错误      |
| -3000 | 参数无效      |
| -3001 | 内存分配失败    |
| -3002 | 对象创建失败    |
| -3003 | 服务未启动     |
| -3004 | 操作超时      |
| -3005 | 重复调用      |
| -3006 | 对象已存在     |
| -3007 | 对象不存在     |
| -3008 | 未初始化      |
| -3009 | 网络序列号重复   |
| -3010 | 网络序列号未找到  |
| -3011 | 已通知       |
| -3013 | 超出容量限制    |
| -3014 | 连接未找到     |
| -3015 | 网络请求句柄未找到 |
| -3016 | 内存不足      |
| -3017 | 未连接到元数据服务 |
| -3018 | 未连接到本地服务  |
| -3019 | 客户端未初始化   |
| -3101 | 状态不匹配     |
| -3102 | 键不匹配      |
| -3103 | 返回值不匹配    |
| -3104 | 租约未到期     |

## 注意事项

- 所有键的长度必须小于256字节
- 支持同步和异步两种操作模式
- 批量操作可以提高处理效率