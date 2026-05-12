/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 * MemCache_Hybrid is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 */

#include <cstring>
#include <map>
#include <mutex>
#include <string>
#include <vector>
#include <algorithm>

// 全局存储模拟
std::map<std::string, std::string> gUbsioStorage;
std::mutex gUbsioMutex;

// 存储分配的内存，以便后续释放
std::vector<void*> gAllocatedBuffers;
std::mutex gBufferMutex;

// 初始化函数
extern "C" int32_t UbsioKvCacheInit(int32_t deviceId)
{
    (void)deviceId;
    return 0;
}

// 写入函数
extern "C" int32_t UbsioKvCachePut(const char *key, void *buf, size_t length, uint32_t flags)
{
    (void)flags;
    if (key == nullptr || buf == nullptr) {
        return -1;
    }
    
    std::lock_guard<std::mutex> lock(gUbsioMutex);
    std::string keyStr(key);
    std::string valueStr(static_cast<char*>(buf), length);
    gUbsioStorage[keyStr] = valueStr;
    return 0;
}

// 读取函数
extern "C" int32_t UbsioKvCacheGet(const char *key, void *buf, size_t length, uint32_t flags)
{
    (void)flags;
    if (key == nullptr || buf == nullptr) {
        return -1;
    }
    
    std::lock_guard<std::mutex> lock(gUbsioMutex);
    std::string keyStr(key);
    auto it = gUbsioStorage.find(keyStr);
    if (it == gUbsioStorage.end()) {
        return -1;
    }
    
    const std::string& value = it->second;
    if (value.size() > length) {
        return -1;
    }
    
    memcpy(buf, value.c_str(), value.size());
    return 0;
}

// 检查存在函数
extern "C" bool UbsioKvCacheExist(const char *key, uint32_t flags)
{
    (void)flags;
    if (key == nullptr) {
        return false;
    }
    
    std::lock_guard<std::mutex> lock(gUbsioMutex);
    std::string keyStr(key);
    return gUbsioStorage.find(keyStr) != gUbsioStorage.end();
}

// 删除函数
extern "C" int32_t UbsioKvCacheDelete(const char *key, uint32_t flags)
{
    (void)flags;
    if (key == nullptr) {
        return -1;
    }
    
    std::lock_guard<std::mutex> lock(gUbsioMutex);
    std::string keyStr(key);
    size_t erased = gUbsioStorage.erase(keyStr);
    return erased > 0 ? 0 : -1;
}

// 获取长度函数
extern "C" int32_t UbsioKvCacheGetLength(const char *key, size_t *length, uint32_t flags)
{
    (void)flags;
    if (key == nullptr || length == nullptr) {
        return -1;
    }
    
    std::lock_guard<std::mutex> lock(gUbsioMutex);
    std::string keyStr(key);
    auto it = gUbsioStorage.find(keyStr);
    if (it == gUbsioStorage.end()) {
        return -1;
    }
    
    *length = it->second.size();
    return 0;
}

// 批量写入函数
extern "C" int32_t UbsioKvCacheBatchPut(const char **keys, uint32_t keys_count, void **bufs, size_t *lengths,
                                        int *results, uint32_t flags)
{
    (void)flags;
    if (keys == nullptr || bufs == nullptr || lengths == nullptr || results == nullptr) {
        return -1;
    }
    
    std::lock_guard<std::mutex> lock(gUbsioMutex);
    for (uint32_t i = 0; i < keys_count; ++i) {
        if (keys[i] == nullptr || bufs[i] == nullptr) {
            results[i] = -1;
            continue;
        }
        
        std::string keyStr(keys[i]);
        std::string valueStr(static_cast<char*>(bufs[i]), lengths[i]);
        gUbsioStorage[keyStr] = valueStr;
        results[i] = 0;
    }
    return 0;
}

// 批量读取函数
extern "C" int32_t UbsioKvCacheBatchGet(const char **keys, uint32_t keys_count, void **bufs, size_t *lengths,
                                        int *results, uint32_t flags)
{
    (void)flags;
    if (keys == nullptr || bufs == nullptr || lengths == nullptr || results == nullptr) {
        return -1;
    }
    
    std::lock_guard<std::mutex> lock(gUbsioMutex);
    for (uint32_t i = 0; i < keys_count; ++i) {
        if (keys[i] == nullptr) {
            results[i] = -1;
            continue;
        }
        
        std::string keyStr(keys[i]);
        auto it = gUbsioStorage.find(keyStr);
        if (it == gUbsioStorage.end()) {
            results[i] = -1;
            continue;
        }
        
        const std::string& value = it->second;
        // UBSIO为buf分配内存
        void* allocatedBuf = malloc(value.size() + 1);
        if (allocatedBuf == nullptr) {
            results[i] = -1;
            continue;
        }
        
        memcpy(allocatedBuf, value.c_str(), value.size());
        static_cast<char*>(allocatedBuf)[value.size()] = '\0';
        
        // 存储分配的内存
        {
            std::lock_guard<std::mutex> bufLock(gBufferMutex);
            gAllocatedBuffers.push_back(allocatedBuf);
        }
        
        // 设置返回值
        bufs[i] = allocatedBuf;
        lengths[i] = value.size();
        results[i] = 0;
    }
    return 0;
}

// 批量检查存在函数
extern "C" int32_t UbsioKvCacheBatchExist(const char **keys, uint32_t keys_count, bool *results, uint32_t flags)
{
    (void)flags;
    if (keys == nullptr || results == nullptr) {
        return -1;
    }
    
    std::lock_guard<std::mutex> lock(gUbsioMutex);
    for (uint32_t i = 0; i < keys_count; ++i) {
        if (keys[i] == nullptr) {
            results[i] = false;
            continue;
        }
        
        std::string keyStr(keys[i]);
        results[i] = (gUbsioStorage.find(keyStr) != gUbsioStorage.end());
    }
    return 0;
}

// 批量删除函数
extern "C" int32_t UbsioKvCacheBatchDelete(const char **keys, uint32_t keys_count, int32_t *results, uint32_t flags)
{
    (void)flags;
    if (keys == nullptr || results == nullptr) {
        return -1;
    }
    
    std::lock_guard<std::mutex> lock(gUbsioMutex);
    for (uint32_t i = 0; i < keys_count; ++i) {
        if (keys[i] == nullptr) {
            results[i] = -1;
            continue;
        }
        
        std::string keyStr(keys[i]);
        size_t erased = gUbsioStorage.erase(keyStr);
        results[i] = (erased > 0) ? 0 : -1;
    }
    return 0;
}

// 批量获取长度函数
extern "C" int32_t UbsioKvCacheBatchGetLength(const char **keys, uint32_t keys_count, size_t *lengths,
                                              int32_t *results, uint32_t flags)
{
    (void)flags;
    if (keys == nullptr || lengths == nullptr || results == nullptr) {
        return -1;
    }
    
    std::lock_guard<std::mutex> lock(gUbsioMutex);
    for (uint32_t i = 0; i < keys_count; ++i) {
        if (keys[i] == nullptr) {
            results[i] = -1;
            continue;
        }
        
        std::string keyStr(keys[i]);
        auto it = gUbsioStorage.find(keyStr);
        if (it == gUbsioStorage.end()) {
            results[i] = -1;
            continue;
        }
        
        lengths[i] = it->second.size();
        results[i] = 0;
    }
    return 0;
}

// 批量释放地址函数
extern "C" int32_t UbsioKvCacheBatchFree(void **bufs, uint32_t keys_count)
{
    if (bufs == nullptr) {
        return -1;
    }
    
    std::lock_guard<std::mutex> bufLock(gBufferMutex);
    for (uint32_t i = 0; i < keys_count; ++i) {
        if (bufs[i] != nullptr) {
            // 释放分配的内存
            free(bufs[i]);
            
            // 从跟踪列表中移除
            auto it = std::find(gAllocatedBuffers.begin(), gAllocatedBuffers.end(), bufs[i]);
            if (it != gAllocatedBuffers.end()) {
                gAllocatedBuffers.erase(it);
            }
        }
    }
    return 0;
}

// 批量直接读取函数（带HBM）
extern "C" int32_t UbsioKvCacheBatchGetDirect(const char **keys, uint32_t keys_count, void ***bufs, size_t **lengths,
    uint32_t lengths_rows, uint32_t lengths_cols, int *results, uint32_t flags)
{
    (void)flags;
    (void)bufs;
    (void)lengths;
    (void)lengths_rows;
    (void)lengths_cols;
    if (keys == nullptr || results == nullptr) {
        return -1;
    }
    
    // 简化实现，标记所有为成功
    for (uint32_t i = 0; i < keys_count; ++i) {
        results[i] = 0;
    }
    return 0;
}
