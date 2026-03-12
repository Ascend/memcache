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

// 定义与DFC库相同的类型和函数

enum class kv_worker_mode {
    KV_CONVERGENCE,
    KV_SEPARATES
};

// 全局存储模拟
std::map<std::string, std::string> gDfcStorage;
std::mutex gDfcMutex;

// 初始化函数
extern "C" int32_t DfcClientInit(kv_worker_mode mode)
{
    (void)mode;
    return 0;
}

// 写入函数
extern "C" int32_t DfcPut(const char *key, void *buf, size_t length, uint32_t flags)
{
    (void)flags;
    if (key == nullptr || buf == nullptr) {
        return -1;
    }
    
    std::lock_guard<std::mutex> lock(gDfcMutex);
    std::string keyStr(key);
    std::string valueStr(static_cast<char*>(buf), length);
    gDfcStorage[keyStr] = valueStr;
    return 0;
}

// 读取函数
extern "C" int32_t DfcGet(const char *key, void *buf, size_t length, uint32_t flags)
{
    (void)flags;
    if (key == nullptr || buf == nullptr) {
        return -1;
    }
    
    std::lock_guard<std::mutex> lock(gDfcMutex);
    std::string keyStr(key);
    auto it = gDfcStorage.find(keyStr);
    if (it == gDfcStorage.end()) {
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
extern "C" bool DfcExist(const char *key, uint32_t flags)
{
    (void)flags;
    if (key == nullptr) {
        return false;
    }
    
    std::lock_guard<std::mutex> lock(gDfcMutex);
    std::string keyStr(key);
    return gDfcStorage.find(keyStr) != gDfcStorage.end();
}

// 删除函数
extern "C" int32_t DfcDelete(const char *key, uint32_t flags)
{
    (void)flags;
    if (key == nullptr) {
        return -1;
    }
    
    std::lock_guard<std::mutex> lock(gDfcMutex);
    std::string keyStr(key);
    size_t erased = gDfcStorage.erase(keyStr);
    return erased > 0 ? 0 : -1;
}

// 获取长度函数
extern "C" int32_t DfcGetLength(const char *key, size_t *length, uint32_t flags)
{
    (void)flags;
    if (key == nullptr || length == nullptr) {
        return -1;
    }
    
    std::lock_guard<std::mutex> lock(gDfcMutex);
    std::string keyStr(key);
    auto it = gDfcStorage.find(keyStr);
    if (it == gDfcStorage.end()) {
        return -1;
    }
    
    *length = it->second.size();
    return 0;
}

// 批量写入函数
extern "C" int32_t DfcBatchPut(const char **keys, uint32_t keys_count, void **bufs,
                               size_t *lengths, int *results, uint32_t flags)
{
    (void)flags;
    if (keys == nullptr || bufs == nullptr || lengths == nullptr || results == nullptr) {
        return -1;
    }
    
    std::lock_guard<std::mutex> lock(gDfcMutex);
    for (uint32_t i = 0; i < keys_count; ++i) {
        if (keys[i] == nullptr || bufs[i] == nullptr) {
            results[i] = -1;
            continue;
        }
        
        std::string keyStr(keys[i]);
        std::string valueStr(static_cast<char*>(bufs[i]), lengths[i]);
        gDfcStorage[keyStr] = valueStr;
        results[i] = 0;
    }
    return 0;
}

// 存储分配的内存，以便后续释放
std::vector<void*> gAllocatedBuffers;
std::mutex gBufferMutex;

// 批量读取函数
extern "C" int32_t DfcBatchGet(const char **keys, uint32_t keys_count, void **bufs,
                               size_t *lengths, int *results, uint32_t flags)
{
    (void)flags;
    if (keys == nullptr || bufs == nullptr || lengths == nullptr || results == nullptr) {
        return -1;
    }
    
    std::lock_guard<std::mutex> lock(gDfcMutex);
    for (uint32_t i = 0; i < keys_count; ++i) {
        if (keys[i] == nullptr) {
            results[i] = -1;
            continue;
        }
        
        std::string keyStr(keys[i]);
        auto it = gDfcStorage.find(keyStr);
        if (it == gDfcStorage.end()) {
            results[i] = -1;
            continue;
        }
        
        const std::string& value = it->second;
        // DFC为buf分配内存
        void* allocatedBuf = malloc(value.size() + 1); // 额外加1用于字符串结束符
        if (allocatedBuf == nullptr) {
            results[i] = -1;
            continue;
        }
        
        memcpy(allocatedBuf, value.c_str(), value.size());
        static_cast<char*>(allocatedBuf)[value.size()] = '\0'; // 添加字符串结束符
        
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
extern "C" int32_t DfcBatchExist(const char **keys, uint32_t keys_count, bool *results, uint32_t flags)
{
    (void)flags;
    if (keys == nullptr || results == nullptr) {
        return -1;
    }
    
    std::lock_guard<std::mutex> lock(gDfcMutex);
    for (uint32_t i = 0; i < keys_count; ++i) {
        if (keys[i] == nullptr) {
            results[i] = false;
            continue;
        }
        
        std::string keyStr(keys[i]);
        results[i] = (gDfcStorage.find(keyStr) != gDfcStorage.end());
    }
    return 0;
}

// 批量删除函数
extern "C" int32_t DfcBatchDelete(const char **keys, uint32_t keys_count, int32_t *results, uint32_t flags)
{
    (void)flags;
    if (keys == nullptr || results == nullptr) {
        return -1;
    }
    
    std::lock_guard<std::mutex> lock(gDfcMutex);
    for (uint32_t i = 0; i < keys_count; ++i) {
        if (keys[i] == nullptr) {
            results[i] = -1;
            continue;
        }
        
        std::string keyStr(keys[i]);
        size_t erased = gDfcStorage.erase(keyStr);
        results[i] = (erased > 0) ? 0 : -1;
    }
    return 0;
}

// 批量获取长度函数
extern "C" int32_t DfcBatchGetLength(const char **keys, uint32_t keys_count, size_t *lengths,
                                     int32_t *results, uint32_t flags)
{
    (void)flags;
    if (keys == nullptr || lengths == nullptr || results == nullptr) {
        return -1;
    }
    
    std::lock_guard<std::mutex> lock(gDfcMutex);
    for (uint32_t i = 0; i < keys_count; ++i) {
        if (keys[i] == nullptr) {
            results[i] = -1;
            continue;
        }
        
        std::string keyStr(keys[i]);
        auto it = gDfcStorage.find(keyStr);
        if (it == gDfcStorage.end()) {
            results[i] = -1;
            continue;
        }
        
        lengths[i] = it->second.size();
        results[i] = 0;
    }
    return 0;
}

// 批量释放地址函数
extern "C" int32_t DfcBatchFreeAddress(void **bufs, uint32_t keys_count)
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
