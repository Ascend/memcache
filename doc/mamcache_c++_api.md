# MemCache API

## C++接口列表
C++语言接口功能齐全，基于面向对象设计，提供统一的 `ObjectStore` 抽象基类，封装了实例管理、缓冲区注册、数据操作、批量处理及分层张量等功能。

### 1. 实例创建与生命周期管理接口

#### ObjectStore::CreateObjectStore
```c++
static std::shared_ptr<ObjectStore> CreateObjectStore();
```
**功能**: 创建一个分布式内存缓存存储实例。

**返回值**:
- 返回 std::shared_ptr 管理的智能指针，确保资源自动释放，非空 shared_ptr 表示成功。

#### Init
```c++
virtual int Init(const uint32_t deviceId, bool initBm = true) = 0;
```
**功能**: 初始化当前存储实例，绑定到指定设备。

**参数**:
- `deviceId`: 目标设备ID
- `initBm`: 是否初始化BM提供内存，默认值为 true。设 false 时将启动纯client模式

**返回值**:
- `0`：成功
- 其他：失败

#### TearDown
```c++
virtual int TearDown() = 0;
```
**功能**: 释放当前实例占用的所有资源，断开与元服务和本地服务的连接。

**返回值**:
- `0`：成功
- 其他：失败

### 2. 缓冲区注册接口

#### RegisterBuffer
```c++
virtual int RegisterBuffer(void *buffer, size_t size) = 0;
```
**功能**: 将用户分配的内存区域注册到系统中，以启用 RDMA 或零拷贝传输。

**参数**:
- `buffer`: 内存起始地址
- `size`: 缓冲区字节大小

**返回值**:
- `0`：成功
- 其他：失败

#### UnRegisterBuffer
```c++
virtual int UnRegisterBuffer(void *buffer, size_t size) = 0;
```
**功能**: 注销已注册的内存区域。

**参数**:
- `buffer`: 内存起始地址
- `size`: 缓冲区字节大小

**返回值**:
- `0`: 成功
- 其他: 失败

### 3. 数据操作接口

#### GetInto
```c++
virtual int GetInto(const std::string &key, void *buffer, size_t size, const int32_t direct = 2) = 0;
```
**功能**: 将指定键的数据读入预分配的 buffer 中。

**参数**:
- `key`: 数据键（长度 < 256字节）
- `buffer`: 目标内存地址
- `size`: 缓冲区容量
- `direct`: 数据流向

**返回值**:
- `0`: 成功
- 其他: 失败

#### PutFrom
```c++
virtual int PutFrom(const std::string &key, void *buffer, size_t size, const int32_t direct = 3,
                        const ReplicateConfig &replicateConfig = {}) = 0;
```
**功能**: 将 buffer 中的数据写入缓存并关联到 key。

**参数**:
- `ReplicateConfig`: 副本策略配置
- `key`: 数据键（长度 < 256字节）
- `buffer`: 目标内存地址
- `size`: 缓冲区容量
- `direct`: 数据流向

**返回值**:
- `0`: 成功
- 其他: 失败

#### Remove
```c++
virtual int Remove(const std::string &key) = 0;
```
**功能**: 删除指定键的数据对象。

**参数**:
- `key`: 数据键（长度 < 256字节）

**返回值**:
- `0`: 成功
- 其他: 失败

#### IsExist
```c++
virtual int IsExist(const std::string &key) = 0;
```
**功能**: 检查键是否存在。

**参数**:
- `key`: 数据键（长度 < 256字节）

**返回值**:
- `0`: 成功
- 其他: 失败

#### GetKeyInfo
```c++
virtual KeyInfo GetKeyInfo(const std::string &key) = 0;
```
**功能**: 获取键的元信息。

**参数**:
- `key`: 数据的键，长度小于256个字节

**返回值**:
返回 KeyInfo 结构体，包含：
- `key`: 键名
- `size`: 数据字节数
- `deviceId`: 数据所属设备 ID
- `status`: 状态码（0 表示成功，负值表示错误）

### 4. 批量操作接口

#### BatchGetInto
```c++
virtual std::vector<int> BatchGetInto(const std::vector<std::string> &keys, const std::vector<void *> &buffers,
                                          const std::vector<size_t> &sizes, const int32_t direct = 2) = 0;
```
**功能**: 批量读取多个键到各自缓冲区。

**参数**:
- `keys`: 数据键列表（每个键长度 < 256字节）
- `buffers`: 目标内存地址列表，必须与keys一一对应
- `sizes`: 缓冲区容量列表，必须与buffers长度一致
- `direct`: 数据流向

**返回值**:
- `0`: 成功
- 其他: 失败

#### BatchPutFrom
```c++
virtual std::vector<int> BatchPutFrom(const std::vector<std::string> &keys, const std::vector<void *> &buffers,
                                          const std::vector<size_t> &sizes, const int32_t direct = 3,
                                          const ReplicateConfig &replicateConfig = {}) = 0;
```
**功能**: 批量写入多个键。

**参数**:
- `ReplicateConfig`: 副本策略配置
- `keys`: 数据键列表（每个键长度 < 256字节）
- `buffers`: 目标内存地址列表，必须与keys一一对应
- `sizes`: 缓冲区容量列表，必须与buffers长度一致
- `direct`: 数据流向

**返回值**:
- `0`: 成功
- 其他: 失败

#### BatchRemove
```c++
virtual std::vector<int> BatchRemove(const std::vector<std::string> &keys) = 0;
```
**功能**: 批量删除。

**参数**:
- `keys`: 数据键列表（每个键长度 < 256字节）

**返回值**:
- `0`: 成功
- 其他: 失败

#### BatchIsExist
```c++
virtual std::vector<int> BatchIsExist(const std::vector<std::string> &keys) = 0;
```
**功能**: 批量存在性检查。

**参数**:
- `keys`: 数据键（长度 < 256字节）

**返回值**:
- `0`: 成功
- 其他: 失败

#### BatchGetKeyInfo
```c++
virtual std::vector<KeyInfo> BatchGetKeyInfo(const std::vector<std::string> &keys) = 0;
```
**功能**: 批量查询元信息。

**参数**:
- `keys`: 数据键（长度 < 256字节）

**返回值**:
- `0`: 成功
- 其他: 失败

### 5. 分层张量操作

#### PutFromLayers
```c++
virtual int PutFromLayers(const std::string &key, const std::vector<void *> &buffers,
                              const std::vector<size_t> &sizes, const int32_t direct = 3,
                              const ReplicateConfig &replicateConfig = {}) = 0;

```
**功能**: 将多个内存块（layers）拼接后作为一个逻辑对象写入缓存，并关联到指定键。

**参数**:
- `key`: 数据键（长度 < 256字节）
- `buffers`: 多层内存地址列表，必须与keys一一对应
- `sizes`: 每层缓冲区容量大小列表，必须与buffers长度一致
- `direct`: 数据流向
- `ReplicateConfig`: 副本策略配置

**返回值**:
- `0`: 成功
- 其他: 失败

#### GetIntoLayers
```c++
virtual int GetIntoLayers(const std::string &key, const std::vector<void *> &buffers,
                              const std::vector<size_t> &sizes, const int32_t direct = 2) = 0;
```
**功能**: 从缓存中读取指定键的逻辑对象，并按预定义大小分发到多个目标缓冲区。

**参数**:
- `keys`: 数据键列表（每个键长度 < 256字节）
- `buffers`: 多层内存地址列表，必须与keys一一对应
- `sizes`: 每层缓冲区容量大小列表，必须与buffers长度一致
- `direct`: 数据流向

**返回值**:
- `0`: 成功
- 其他: 失败

#### BatchPutFromLayers
```c++
virtual std::vector<int> BatchPutFromLayers(const std::vector<std::string> &keys,
                                                const std::vector<std::vector<void *>> &buffers,
                                                const std::vector<std::vector<size_t>> &sizes, const int32_t direct = 3,
                                                const ReplicateConfig &replicateConfig = {}) = 0;
```
**功能**: 批量将多个逻辑对象（每个由多层内存块组成）写入缓存。

**参数**:
- `keys`: 数据键列表（每个键长度 < 256字节）
- `buffers`: 多层内存地址列表，必须与keys一一对应
- `sizes`: 每层缓冲区容量大小列表，必须与buffers长度一致
- `direct`: 数据流向
- `ReplicateConfig`: 副本策略配置

**返回值**:
- `0`: 成功
- 其他: 失败

#### BatchGetIntoLayers
```c++
virtual std::vector<int> BatchGetIntoLayers(const std::vector<std::string> &keys,
                                                const std::vector<std::vector<void *>> &buffers,
                                                const std::vector<std::vector<size_t>> &sizes,
                                                const int32_t direct = 2) = 0;
```
**功能**: 批量从缓存中读取多个逻辑对象，并分别分发到各自的多层缓冲区。

**参数**:
- `keys`: 数据键列表（每个键长度 < 256字节）
- `buffers`: 多层内存地址列表，必须与keys一一对应
- `sizes`: 每层缓冲区容量大小列表，必须与buffers长度一致
- `direct`: 数据流向

**返回值**:
- `0`: 成功
- 其他: 失败

### 6. 辅助接口

#### GetLocalServiceId
```c++
virtual int GetLocalServiceId(uint32_t &localServiceId) = 0;
```
**功能**: 获取当前实例关联的本地服务 ID（用于调试或日志追踪）。

**参数**:
- `localServiceId`: 输出参数

**返回值**:
- `0`: 成功
- 其他: 失败

## 数据结构

### ReplicateConfig
副本策略配置，包含以下字段：
- `replicaNum`: 副本数，默认1，最大8
- `preferredLocalServiceIDs`: 优先分配的本地服务 ID 列表，列表大小必须小于或等于replicaNum

### KeyInfo
客户端配置结构体，包含以下字段：
- `key`: 键名
- `size`: 数据字节数
- `deviceId`: 数据所属设备ID
- `status`: 状态码(0=OK)

## 常量定义

| 常量 | 值 | 说明 |
| ------------------- | ---- | --------------------------- |
| COPY_DEVICE_TO_HOST | 2 | 从设备（如 GPU）复制到主机（CPU）|
| COPY_HOST_TO_DEVICE | 3 | 从主机（CPU）复制到设备（如 GPU）|

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

- 所有键的长度必须小于256个字节
- 支持同步和异步两种操作模式
- 批量操作可以提高处理效率