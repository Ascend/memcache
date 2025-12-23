# MemCache Hybrid RESTful API 文档

本文档介绍了 MemCache Hybrid 服务提供的 RESTful API 接口。这些接口通过 HTTP 服务器暴露，用于管理和监控缓存服务。

## 基础信息

- **协议**: HTTP
- **数据格式**: JSON / Plain Text

## API 列表

### 1. 健康检查接口

##### 接口URL
```
GET /health
```

##### 功能描述
检查 HTTP 服务是否正常运行

##### 请求参数
无

##### 返回结果
- 成功: HTTP 200 OK，返回 "OK"
- 失败: HTTP 500 Internal Server Error

---

### 2. 数据管理接口

#### 2.1 获取所有键

##### 接口URL
```
GET /get_all_keys
```

##### 功能描述
获取缓存中的所有键列表

##### 请求参数
无

##### 返回结果示例
```
key_2
key_1
```

#### 2.2 查询特定键信息

##### 接口URL
```
GET /query_key?key={key}
```

##### 功能描述
查询指定键的详细信息

##### 请求方法
GET

##### 请求参数

| 参数名 | 类型     | 必填 | 描述     |
|-----|--------|----|--------|
| key | String | 是  | 要查询的键名 |

##### 返回结果示例
```json
{
    "blobs": [
        {
            "medium": "HBM",
            "rank": 0
        }
    ],
    "key": "key_a",
    "numBlobs": 1,
    "prot": 0,
    "size": 100,
    "valid": true
}
```

---

### 3. Segment管理接口

##### 接口URL
```
GET /get_all_segments
```

##### 功能描述
获取所有segment信息

##### 请求参数
无

##### 返回结果示例
```json
[
    {
        "allocatedSize": 368640,
        "bmAddr": 26375394164736,
        "capacity": 5368709120,
        "medium": "HBM",
        "rank": 0
    },
    {
        "allocatedSize": 360448,
        "bmAddr": 26380762873856,
        "capacity": 5368709120,
        "medium": "HBM",
        "rank": 1
    },
    {
        "allocatedSize": 0,
        "bmAddr": 3298534883328,
        "capacity": 5368709120,
        "medium": "DRAM",
        "rank": 0
    },
    {
        "allocatedSize": 0,
        "bmAddr": 3303903592448,
        "capacity": 5368709120,
        "medium": "DRAM",
        "rank": 1
    }
]
```

---

### 4. 监控指标接口

#### 4.1 Prometheus 格式指标

##### 接口URL
```
GET /metrics
```

##### 功能描述
以 Prometheus 格式导出各项监控指标

##### 请求参数
无

##### 返回结果示例
```
# HELP memcache_alloc_operations_total Total number of allocation operations
# TYPE memcache_alloc_operations_total counter
memcache_alloc_operations_total 68
# HELP memcache_remove_operations_total Total number of remove operations
# TYPE memcache_remove_operations_total counter
memcache_remove_operations_total 0
# HELP memcache_get_operations_total Total number of get operations
# TYPE memcache_get_operations_total counter
memcache_get_operations_total 68
# HELP memcache_evict_operations_total Total number of eviction operations
# TYPE memcache_evict_operations_total counter
memcache_evict_operations_total 0
# HELP memcache_stored_keys Current number of stored keys
# TYPE memcache_stored_keys gauge
memcache_stored_keys 0
```

#### 4.2 指标摘要

##### 接口URL
```
GET /metrics/summary
```

##### 功能描述
获取指标摘要信息

##### 请求参数
无

##### 返回结果示例
```
=== MemCache Metrics Summary ===
Allocation Operations: 68
Remove Operations: 0
Get Operations: 68
```

---

## 错误码说明

| HTTP状态码 | 错误类型                  | 描述       |
|---------|-----------------------|----------|
| 200     | OK                    | 请求成功     |
| 400     | Bad Request           | 请求参数错误   |
| 404     | Not Found             | 请求的资源不存在 |
| 500     | Internal Server Error | 服务器内部错误  |
