/*
* Copyright (c) Huawei Technologies Co., Ltd. 2024-2025. All rights reserved.
 */

#include <bits/fs_fwd.h>
#include <sys/mman.h>
#include <unistd.h>
#include <cassert>
#include <iostream>
#include <cstring>
#include <sstream>
#include <cstdint>
#include <cstdlib>
#include <functional>
#include <iomanip>
#include <iostream>
#include <random>
#include "mmcache.h"
#include "mmc_logger.h"
#include "mf_str_util.h"

using namespace ock::mmc;
static const auto KEY = "test1";
uint64_t calculate_hash(const void *data, size_t size)
{
    if (data == nullptr || size == 0) {
        return 0;
    }
    const auto *blocks = static_cast<const uint64_t *>(data);
    size_t block_count = size / sizeof(uint64_t);
    uint64_t hash = 14695981039346656037ULL;
    for (size_t i = 0; i < block_count; ++i) {
        hash ^= blocks[i];
        hash *= 1099511628211ULL;
    }
    const uint8_t *tail = static_cast<const uint8_t *>(data) + block_count * sizeof(uint64_t);
    for (size_t i = 0; i < size % sizeof(uint64_t); ++i) {
        hash ^= static_cast<uint64_t>(tail[i]);
        hash *= 1099511628211ULL;
    }
    return hash;
}

bool IOLoop(std::shared_ptr<ObjectStore> &store, const size_t size, void *putAddr, uint64_t hash_value, void *getAddr,
            const std::string &key)
{
    std::string input;
    std::cin >> input;
    if (input == "exit") {
        return true;
    }
    if (input == "get") {
        MMC_LOG_INFO("store->IsExist(KEY): " << store->IsExist(key));
        if (store->IsExist(key) != 0) {
            auto ret = store->GetInto(key, getAddr, size);
            MMC_LOG_INFO("store->GetInto(KEY): " << ret);
            auto getHash = calculate_hash(getAddr, size);
            MMC_LOG_INFO("getStr内存内容的哈希值: " << getHash);
            if (getHash != hash_value) {
                std::cerr << "getHash != hash_value" << std::endl;
                return true;
            }
        }
    } else if (input == "remove") {
        auto ret = store->Remove(key);
        MMC_LOG_INFO("store->Remove(KEY): " << ret);
    } else if (input == "put") {
        auto ret = store->PutFrom(key, putAddr, size);
        MMC_LOG_INFO("store->PutFrom: " << ret);
        if (ret != 0) {
            return true;
        }
    }
    return false;
}

int main(int argc, char *argv[])
{
    (void)argv;
    auto store = ObjectStore::CreateObjectStore();
    uint32_t deviceId = 0;
    if (argc > 1) {
        if (!ock::mf::StrUtil::String2Uint(argv[1], deviceId)) {
            std::cerr << "Invalid device id" << std::endl;
            return -1;
        }
        std::cout << "Device ID: " << deviceId << std::endl;
    }
    if (store->Init(deviceId) != 0) {
        std::cerr << "Failed to initialize MMC Store" << std::endl;
        return -1;
    }
    const size_t size = 64ULL * 1024 * 1024;
    // 申请内存
    void *putAddr = std::malloc(size);
    if (!putAddr) {
        std::cerr << "内存申请失败!" << std::endl;
        return 1;
    }
    auto *memory = static_cast<uint8_t *>(putAddr);
    for (size_t i = 0; i < size; ++i) {
        memory[i] = i % std::numeric_limits<uint8_t>::max();
    }
    // 计算哈希值
    uint64_t hash_value = calculate_hash(putAddr, size);
    std::cout << "put 内存内容的哈希值: " << hash_value << std::endl;
    void *getAddr = std::malloc(size);
    MMC_LOG_INFO("store->IsExist(KEY): " << store->IsExist(KEY));
    if (store->IsExist(KEY) == 0) {
        auto ret = store->PutFrom(KEY, putAddr, size);
        MMC_LOG_INFO("store->PutFrom: " << ret);
        MMC_ASSERT_RETURN(ret == 0, -1);
    }
    auto ret2 = store->BatchPutFrom({"kBatchPutFrom"}, {putAddr}, {size});
    MMC_LOG_INFO("store->BatchPutFrom: " << ret2[0]);
    MMC_ASSERT_RETURN(ret2[0] == 0, -1);
    MMC_LOG_INFO("store->IsExist(KEY): " << store->IsExist(KEY));
    if (store->IsExist(KEY) != 0) {
        auto ret = store->GetInto(KEY, getAddr, size);
        MMC_LOG_INFO("store->GetInto(KEY): " << ret);
        auto getHash = calculate_hash(getAddr, size);
        MMC_LOG_INFO("getStr内存内容的哈希值: " << getHash);
        if (getHash != hash_value) {
            std::cerr << "getHash != hash_value" << std::endl;
            store->TearDown();
            return -1;
        }
    }
    while (true) {
        if (IOLoop(store, size, putAddr, hash_value, getAddr, KEY)) {
            break;
        }
    }
    free(putAddr);
    free(getAddr);
    store->TearDown();
    return 0;
}