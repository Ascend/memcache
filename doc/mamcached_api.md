# MemCache API

## C接口列表
C语言接口功能齐全，包含metaservice/localservice服务启停接口，客户端初始化和反初始化接口，数据操作接口和日志设置接口。

### 1. 服务启动停止接口

#### mmcs_meta_service_start
```c
mmc_meta_service_t mmcs_meta_service_start(mmc_meta_service_config_t *config);
```
**功能**: 启动分布式内存缓存的元数据服务，该服务是全局的。

**参数**:
- `config`: 元数据服务配置信息

**返回值**:
- `mmc_meta_service_t`: 元数据服务句柄（成功），NULL（失败）

#### mmcs_meta_service_stop
```c
void mmcs_meta_service_stop(mmc_meta_service_t handle);
```
**功能**: 停止分布式内存缓存的元数据服务。

**参数**:
- `handle`: 元数据服务句柄

#### mmcs_local_service_start
```c
mmc_local_service_t mmcs_local_service_start(mmc_local_service_config_t *config);
```
**功能**: 启动分布式内存缓存的本地服务，负责本地内存对象的管理。

**参数**:
- `config`: 本地服务配置信息

**返回值**:
- `mmc_local_service_t`: 本地服务句柄（成功），NULL（失败）

#### mmcs_local_service_stop
```c
void mmcs_local_service_stop(mmc_local_service_t handle);
```
**功能**: 停止分布式内存缓存的本地服务。

**参数**:
- `handle`: 本地服务句柄

### 2. 客户端初始化和反初始化

#### mmcc_init
```c
int32_t mmcc_init(mmc_client_config_t *config);
```
**功能**: 初始化分布式内存缓存客户端，这是一个单例模式。

**参数**:
- `config`: 客户端配置信息

**返回值**:
- `0`: 成功
- 其他: 失败

#### mmcc_uninit
```c
void mmcc_uninit();
```
**功能**: 反初始化客户端，释放相关资源。

### 3. 数据操作接口

#### mmcc_register_buffer
```c
int32_t mmcc_register_buffer(uint64_t addr, uint64_t size);
```
**功能**: 将指定地址和大小的内存缓冲区注册到BM中，以加速device rdma操作。

**参数**:
- `addr`: 要注册的内存缓冲区地址
- `size`: 要注册的内存缓冲区大小

**返回值**:
- `0`: 成功
- 其他: 失败

#### mmcc_local_service_id
```c
int32_t mmcc_local_service_id(uint32_t *localServiceId);
```
**功能**: 获取本地服务的实例id。

**参数**:
- `localServiceId`: 本地服务的实例id

**返回值**:
- `0`: 成功
- 其他: 失败

#### mmcc_put
```c
int32_t mmcc_put(const char *key, mmc_buffer *buf, mmc_put_options options, uint32_t flags);
```
**功能**: 将指定 key 的数据放入分布式内存缓存中。支持同步和异步操作。

**参数**:
- `key`: 数据的键，长度小于256字节
- `buf`: 要放入的数据缓冲区
- `options`: 放置操作的选项
- `flags`: 可选标志，保留字段

**返回值**:
- `0`: 成功
- 其他: 失败

#### mmcc_get
```c
int32_t mmcc_get(const char *key, mmc_buffer *buf, uint32_t flags);
```
**功能**: 从分布式内存缓存中获取指定 key 的数据。支持同步和异步操作。

**参数**:
- `key`: 数据的键，长度小于256字节
- `buf`: 存储获取数据的缓冲区
- `flags`: 可选标志，保留字段

**返回值**:
- `0`: 成功
- 其他: 失败

#### mmcc_query
```c
int32_t mmcc_query(const char *key, mmc_data_info *info, uint32_t flags);
```
**功能**: 查询分布式内存缓存中指定 key 的数据信息。支持同步和异步操作。

**参数**:
- `key`: 数据的键，长度小于256字节
- `info`: 存储数据信息的结构体
- `flags`: 可选标志，保留字段

**返回值**:
- `0`: 成功
- 其他: 失败

#### mmcc_remove
```c
int32_t mmcc_remove(const char *key, uint32_t flags);
```
**功能**: 从分布式内存缓存中删除指定 key 的对象。支持同步和异步操作。

**参数**:
- `key`: 数据的键，长度小于256字节
- `flags`: 可选标志，保留字段

**返回值**:
- `0`: 成功
- 其他: 失败

#### mmcc_exist
```c
int32_t mmcc_exist(const char *key, uint32_t flags);
```
**功能**: 判断指定 key 是否存在于分布式内存缓存中。

**参数**:
- `key`: 数据的键，长度小于256字节
- `flags`: 可选标志，保留字段

**返回值**:
- `0`: 成功
- 其他: 失败

#### mmcc_batch_query
```c
int32_t mmcc_batch_query(const char **keys, size_t keys_count, mmc_data_info *info, uint32_t flags);
```
**功能**: 批量查询指定 keys 的 blob 信息。

**参数**:
- `keys`: 数据键数组，每个键长度小于256字节
- `keys_count`: 键的数量
- `info`: 输出的键的 Blob 信息
- `flags`: 操作标志

**返回值**:
- `0`: 成功
- 其他: 失败

#### mmcc_batch_remove
```c
int32_t mmcc_batch_remove(const char **keys, uint32_t keys_count, int32_t *remove_results, uint32_t flags);
```
**功能**: 从 BM 中批量删除多个键。

**参数**:
- `keys`: 要从 BM 中删除的键列表
- `keys_count`: 键的数量
- `remove_results`: 每个删除操作的结果
- `flags`: 操作标志

**返回值**:
- `0`: 成功
- 正值: 发生错误
- 负值: 其他错误

#### mmcc_batch_exist
```c
int32_t mmcc_batch_exist(const char **keys, uint32_t keys_count, int32_t *exist_results, uint32_t flags);
```
**功能**: 判断多个键是否存在于 BM 中。

**参数**:
- `keys`: 数据键数组，每个键长度小于256字节
- `keys_count`: 键的数量
- `exist_results`: BM 中键的存在状态列表
- `flags`: 可选标志，保留字段

**返回值**:
- `0`: 成功
- 其他: 失败

#### mmcc_batch_put
```c
int32_t mmcc_batch_put(const char **keys, uint32_t keys_count, const mmc_buffer *bufs,
                       mmc_put_options& options, uint32_t flags);
```
**功能**: 批量将多个数据对象放入分布式内存缓存中。支持同步和异步操作。

**参数**:
- `keys`: 数据对象的键数组
- `keys_count`: 数组中键的数量
- `bufs`: 要放入的数据缓冲区数组
- `options`: 批量放置操作的选项
- `flags`: 可选标志，保留字段

**返回值**:
- `0`: 成功
- 其他: 失败

#### mmcc_batch_get
```c
int32_t mmcc_batch_get(const char **keys, uint32_t keys_count, mmc_buffer *bufs, uint32_t flags);
```
**功能**: 批量从分布式内存缓存中获取多个数据对象。支持同步和异步操作。

**参数**:
- `keys`: 数据对象的键数组
- `keys_count`: 数组中键的数量
- `bufs`: 存储检索数据的数据缓冲区数组
- `flags`: 可选标志，保留字段

**返回值**:
- `0`: 成功
- 其他: 失败

### 4. 日志设置接口

#### mmc_set_extern_logger
```c
int32_t mmc_set_extern_logger(void (*func)(int level, const char *msg));
```
**功能**: 设置外部日志函数，用户可以设置自定义的日志记录函数。在自定义的日志记录函数中，用户可以使用统一的日志工具，这样日志消息就可以写入与调用者相同的日志文件中。如果未设置，acc_links日志消息将被打印到标准输出。

日志级别说明：
- `0`: DEBUG
- `1`: INFO
- `2`: WARN
- `3`: ERROR

**参数**:
- `func`: 外部日志记录函数

**返回值**:
- `0`: 成功
- 其他: 失败

#### mmc_set_log_level
```c
int32_t mmc_set_log_level(int level);
```
**功能**: 设置日志打印级别。

**参数**:
- `level`: 日志级别，0:debug 1:info 2:warn 3:error

**返回值**:
- `0`: 成功
- 其他: 失败

### MmcCopyDirect 枚举类型

用于指定数据拷贝方向的枚举类型：
- `SMEMB_COPY_H2G`: 从主机内存到全局内存
- `SMEMB_COPY_L2G`: 从卡上内存到全局内存
- `SMEMB_COPY_G2H`: 从全局内存到主机内存
- `SMEMB_COPY_G2L`: 从全局内存到卡上内存

## 数据结构

### mmc_client_config_t
客户端配置结构体，包含以下字段：
- `discoveryURL`: 发现服务URL
- `rankId`: Rank ID
- `timeOut`: 超时时间
- `logLevel`: 日志级别
- `logFunc`: 外部日志函数
- `tlsConfig`: TLS配置

### ReplicateConfig
客户端配置结构体，包含以下字段：
- `replicaNum`: 副本数，最大为8，默认为1
- `preferredLocalServiceIDs`: 强制分配的实例id列表，列表大小必须小于或等于replicaNum

### mmc_meta_service_config_t
元数据服务配置结构体，包含以下字段：
- `discoveryURL`: 发现服务URL
- `logLevel`: 日志级别
- `evictThresholdHigh`: 高水位驱逐阈值
- `evictThresholdLow`: 低水位驱逐阈值
- `tlsConfig`: TLS配置

### mmc_local_service_config_t
本地服务配置结构体，包含以下字段：
- `discoveryURL`: 发现服务URL
- `deviceId`: 设备ID
- `rankId`: BM全局统一编号
- `worldSize`: 世界大小
- `bmIpPort`: BM IP端口
- `bmHcomUrl`: BM HCOM URL
- `createId`: 创建ID
- `dataOpType`: 数据操作类型
- `localDRAMSize`: 本地DRAM大小
- `localHBMSize`: 本地HBM大小
- `flags`: 标志
- `tlsConfig`: TLS配置
- `logLevel`: 日志级别
- `logFunc`: 外部日志函数

### mmc_buffer
内存缓冲区结构体，包含内存地址、类型和维度信息。

### mmc_put_options
放置操作选项，包含媒体类型和亲和策略。

### mmc_data_info
数据信息结构体，包含大小、保护标志、blob数量和有效性标志。

### tls_config
TLS配置结构体，包含以下字段：
- `tlsEnable`: TLS启用标志
- `caPath`: 根证书文件路径
- `crlPath`: 证书吊销列表文件路径
- `certPath`: 证书文件路径
- `keyPath`: 私钥文件路径
- `keyPassPath`: 私钥加密密码（口令）文件路径
- `packagePath`: openssl动态库路径
- `decrypterLibPath`: 自定义口令解密算法动态库路径

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