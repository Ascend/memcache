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
#ifndef __MMC_STORE_H__
#define __MMC_STORE_H__

#include <mutex>
#include <string>
#include <unordered_set>
#include <sstream>
#include <csignal>

#include "mmc_def.h"
#include "mmc_types.h"
#include "mmcache.h"

namespace ock {
namespace mmc {

class MmcacheStore;
// Global resource tracker to handle cleanup on abnormal termination
class ResourceTracker {
public:
    // Get the singleton instance
    static ResourceTracker &getInstance();

    // Prevent copying
    ResourceTracker(const ResourceTracker &) = delete;
    ResourceTracker &operator=(const ResourceTracker &) = delete;

    // Register a MmcacheStore instance for cleanup
    void registerInstance(MmcacheStore *instance);

    // Unregister a MmcacheStore instance
    void unregisterInstance(MmcacheStore *instance);

private:
    ResourceTracker();
    ~ResourceTracker();

    // Cleanup all registered resources
    void cleanupAllResources();

    // Signal handler function
    static void signalHandler(int signal);

    // Exit handler function
    static void exitHandler();

    std::mutex mutex_;
    std::unordered_set<MmcacheStore *> instances_;
};

class MmcacheStore : public ock::mmc::ObjectStore {
public:
    MmcacheStore();
    ~MmcacheStore() override;

    int Init(const uint32_t deviceId, const bool initBm = true) override;

    int TearDown() override;

    int RegisterBuffer(void *buffer, size_t size) override;

    int UnRegisterBuffer(void *buffer, size_t size) override;

    int GetInto(const std::string &key, void *buffer, size_t size, const int32_t direct = 2) override;

    std::vector<int> BatchGetInto(const std::vector<std::string> &keys, const std::vector<void *> &buffers,
                                  const std::vector<size_t> &sizes, const int32_t direct = 2) override;

    int GetIntoLayers(const std::string &key, const std::vector<void *> &buffers, const std::vector<size_t> &sizes,
                      const int32_t direct = 9) override;

    std::vector<int> BatchGetIntoLayers(const std::vector<std::string> &keys,
                                        const std::vector<std::vector<void *>> &buffers,
                                        const std::vector<std::vector<size_t>> &sizes,
                                        const int32_t direct = 2) override;

    int PutFrom(const std::string &key, void *buffer, size_t size, const int32_t direct = 3,
                const ReplicateConfig &replicateConfig = {}) override;

    int GetLocalServiceId(uint32_t &localServiceId) override;

    std::vector<int> BatchPutFrom(const std::vector<std::string> &keys, const std::vector<void *> &buffers,
                                  const std::vector<size_t> &sizes, const int32_t direct = 3,
                                  const ReplicateConfig &replicateConfig = {}) override;

    int PutFromLayers(const std::string &key, const std::vector<void *> &buffers, const std::vector<size_t> &sizes,
                      const int32_t direct = 9, const ReplicateConfig &replicateConfig = {}) override;

    std::vector<int> BatchPutFromLayers(const std::vector<std::string> &keys,
                                        const std::vector<std::vector<void *>> &buffers,
                                        const std::vector<std::vector<size_t>> &sizes, const int32_t direct = 3,
                                        const ReplicateConfig &replicateConfig = {}) override;

    int Remove(const std::string &key) override;

    std::vector<int> BatchRemove(const std::vector<std::string> &keys) override;

    int RemoveAll() override;

    int IsExist(const std::string &key) override;

    std::vector<int> BatchIsExist(const std::vector<std::string> &keys) override;

    KeyInfo GetKeyInfo(const std::string &key) override;

    std::vector<KeyInfo> BatchGetKeyInfo(const std::vector<std::string> &keys) override;

    // bellow only python api use
    mmc_buffer Get(const std::string &key);

    std::vector<mmc_buffer> GetBatch(const std::vector<std::string> &key);

    int Put(const std::string &key, mmc_buffer &buffer, const ReplicateConfig &replicateConfig = {});

    int PutBatch(const std::vector<std::string> &keys, std::vector<mmc_buffer> &buffers,
                 const ReplicateConfig &replicateConfig);

private:
    int CheckInput(size_t batchSize, const std::vector<std::vector<void *>> &buffers,
                   const std::vector<std::vector<size_t>> &sizes);

    void GetBufferArrays(size_t batchSize, uint32_t type, const std::vector<std::vector<void *>> &bufferLists,
                         const std::vector<std::vector<size_t>> &sizeLists,
                         std::vector<ock::mmc::MmcBufferArray> &bufferArrays);

    bool IsInHybmDeviceRange(uint64_t va);

    static int ReturnWrapper(const int result, const std::string &key);
};

} // namespace mmc
} // namespace ock

#endif