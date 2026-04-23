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

#include "mmc_meta_metric_manager.h"

namespace prometheus {
namespace simpleapi {
auto registry_ptr = std::make_shared<Registry>();
Registry &registry = *registry_ptr;
} // namespace simpleapi
} // namespace prometheus

namespace ock {
namespace mmc {

MmcMetaMetricManager::MmcMetaMetricManager()
    : allocRequestCounter_("memcache_alloc_requests_total", "Total number of Alloc requests"),
      allocFailureCounter_("memcache_alloc_failures_total", "Total number of Alloc failures"),
      batchAllocRequestCounter_("memcache_batch_alloc_requests_total", "Total number of BatchAlloc requests"),
      batchAllocFailureCounter_("memcache_batch_alloc_failures_total", "Total number of BatchAlloc failures"),
      getRequestCounter_("memcache_get_requests_total", "Total number of Get requests"),
      getFailureCounter_("memcache_get_failures_total", "Total number of Get failures"),
      batchGetRequestCounter_("memcache_batch_get_requests_total", "Total number of BatchGet requests"),
      batchGetFailureCounter_("memcache_batch_get_failures_total", "Total number of BatchGet failures"),
      removeRequestCounter_("memcache_remove_requests_total", "Total number of Remove requests"),
      removeFailureCounter_("memcache_remove_failures_total", "Total number of Remove failures"),
      batchRemoveRequestCounter_("memcache_batch_remove_requests_total", "Total number of BatchRemove requests"),
      batchRemoveFailureCounter_("memcache_batch_remove_failures_total", "Total number of BatchRemove failures"),
      removeAllRequestCounter_("memcache_remove_all_requests_total", "Total number of RemoveAll requests"),
      removeAllFailureCounter_("memcache_remove_all_failures_total", "Total number of RemoveAll failures"),
      updateStateRequestCounter_("memcache_update_state_requests_total", "Total number of UpdateState requests"),
      updateStateFailureCounter_("memcache_update_state_failures_total", "Total number of UpdateState failures"),
      batchUpdateStateRequestCounter_("memcache_batch_update_state_requests_total",
                                      "Total number of BatchUpdateState requests"),
      batchUpdateStateFailureCounter_("memcache_batch_update_state_failures_total",
                                      "Total number of BatchUpdateState failures"),
      queryRequestCounter_("memcache_query_requests_total", "Total number of Query requests"),
      queryFailureCounter_("memcache_query_failures_total", "Total number of Query failures"),
      batchQueryRequestCounter_("memcache_batch_query_requests_total", "Total number of BatchQuery requests"),
      batchQueryFailureCounter_("memcache_batch_query_failures_total", "Total number of BatchQuery failures"),
      getAllKeysRequestCounter_("memcache_get_all_keys_requests_total", "Total number of GetAllKeys requests"),
      getAllKeysFailureCounter_("memcache_get_all_keys_failures_total", "Total number of GetAllKeys failures"),
      existKeyRequestCounter_("memcache_exist_key_requests_total", "Total number of ExistKey requests"),
      existKeyFailureCounter_("memcache_exist_key_failures_total", "Total number of ExistKey failures"),
      batchExistKeyRequestCounter_("memcache_batch_exist_key_requests_total", "Total number of BatchExistKey requests"),
      batchExistKeyFailureCounter_("memcache_batch_exist_key_failures_total", "Total number of BatchExistKey failures"),
      mountRequestCounter_("memcache_mount_requests_total", "Total number of Mount requests"),
      mountFailureCounter_("memcache_mount_failures_total", "Total number of Mount failures"),
      unmountRequestCounter_("memcache_unmount_requests_total", "Total number of Unmount requests"),
      unmountFailureCounter_("memcache_unmount_failures_total", "Total number of Unmount failures"),
      evictCounter_("memcache_evict_operations_total", "Total number of eviction operations"),
      keyCountGauge_("memcache_stored_keys", "Current number of stored keys")
{}

MmcMetaMetricSnapshot MmcMetaMetricManager::GetSnapshot() const
{
    MmcMetaMetricSnapshot snapshot;
    snapshot.allocRequestCount = static_cast<uint64_t>(allocRequestCounter_.value());
    snapshot.allocFailureCount = static_cast<uint64_t>(allocFailureCounter_.value());
    snapshot.batchAllocRequestCount = static_cast<uint64_t>(batchAllocRequestCounter_.value());
    snapshot.batchAllocFailureCount = static_cast<uint64_t>(batchAllocFailureCounter_.value());
    snapshot.getRequestCount = static_cast<uint64_t>(getRequestCounter_.value());
    snapshot.getFailureCount = static_cast<uint64_t>(getFailureCounter_.value());
    snapshot.batchGetRequestCount = static_cast<uint64_t>(batchGetRequestCounter_.value());
    snapshot.batchGetFailureCount = static_cast<uint64_t>(batchGetFailureCounter_.value());
    snapshot.removeRequestCount = static_cast<uint64_t>(removeRequestCounter_.value());
    snapshot.removeFailureCount = static_cast<uint64_t>(removeFailureCounter_.value());
    snapshot.batchRemoveRequestCount = static_cast<uint64_t>(batchRemoveRequestCounter_.value());
    snapshot.batchRemoveFailureCount = static_cast<uint64_t>(batchRemoveFailureCounter_.value());
    snapshot.removeAllRequestCount = static_cast<uint64_t>(removeAllRequestCounter_.value());
    snapshot.removeAllFailureCount = static_cast<uint64_t>(removeAllFailureCounter_.value());
    snapshot.updateStateRequestCount = static_cast<uint64_t>(updateStateRequestCounter_.value());
    snapshot.updateStateFailureCount = static_cast<uint64_t>(updateStateFailureCounter_.value());
    snapshot.batchUpdateStateRequestCount = static_cast<uint64_t>(batchUpdateStateRequestCounter_.value());
    snapshot.batchUpdateStateFailureCount = static_cast<uint64_t>(batchUpdateStateFailureCounter_.value());
    snapshot.queryRequestCount = static_cast<uint64_t>(queryRequestCounter_.value());
    snapshot.queryFailureCount = static_cast<uint64_t>(queryFailureCounter_.value());
    snapshot.batchQueryRequestCount = static_cast<uint64_t>(batchQueryRequestCounter_.value());
    snapshot.batchQueryFailureCount = static_cast<uint64_t>(batchQueryFailureCounter_.value());
    snapshot.getAllKeysRequestCount = static_cast<uint64_t>(getAllKeysRequestCounter_.value());
    snapshot.getAllKeysFailureCount = static_cast<uint64_t>(getAllKeysFailureCounter_.value());
    snapshot.existKeyRequestCount = static_cast<uint64_t>(existKeyRequestCounter_.value());
    snapshot.existKeyFailureCount = static_cast<uint64_t>(existKeyFailureCounter_.value());
    snapshot.batchExistKeyRequestCount = static_cast<uint64_t>(batchExistKeyRequestCounter_.value());
    snapshot.batchExistKeyFailureCount = static_cast<uint64_t>(batchExistKeyFailureCounter_.value());
    snapshot.mountRequestCount = static_cast<uint64_t>(mountRequestCounter_.value());
    snapshot.mountFailureCount = static_cast<uint64_t>(mountFailureCounter_.value());
    snapshot.unmountRequestCount = static_cast<uint64_t>(unmountRequestCounter_.value());
    snapshot.unmountFailureCount = static_cast<uint64_t>(unmountFailureCounter_.value());
    snapshot.evictCount = static_cast<uint64_t>(evictCounter_.value());
    snapshot.keyCount = static_cast<uint64_t>(keyCountGauge_.value());
    return snapshot;
}

void MmcMetaMetricManager::IncrementRequestCounter(RestMetricType type)
{
    switch (type) {
        case RestMetricType::ALLOC:
            allocRequestCounter_++;
            return;
        case RestMetricType::BATCH_ALLOC:
            batchAllocRequestCounter_++;
            return;
        case RestMetricType::GET:
            getRequestCounter_++;
            return;
        case RestMetricType::BATCH_GET:
            batchGetRequestCounter_++;
            return;
        case RestMetricType::REMOVE:
            removeRequestCounter_++;
            return;
        case RestMetricType::BATCH_REMOVE:
            batchRemoveRequestCounter_++;
            return;
        case RestMetricType::REMOVE_ALL:
            removeAllRequestCounter_++;
            return;
        case RestMetricType::UPDATE_STATE:
            updateStateRequestCounter_++;
            return;
        case RestMetricType::BATCH_UPDATE_STATE:
            batchUpdateStateRequestCounter_++;
            return;
        case RestMetricType::QUERY:
            queryRequestCounter_++;
            return;
        case RestMetricType::BATCH_QUERY:
            batchQueryRequestCounter_++;
            return;
        case RestMetricType::GET_ALL_KEYS:
            getAllKeysRequestCounter_++;
            return;
        case RestMetricType::EXIST_KEY:
            existKeyRequestCounter_++;
            return;
        case RestMetricType::BATCH_EXIST_KEY:
            batchExistKeyRequestCounter_++;
            return;
        case RestMetricType::MOUNT:
            mountRequestCounter_++;
            return;
        case RestMetricType::UNMOUNT:
            unmountRequestCounter_++;
            return;
        default:
            return;
    }
}

void MmcMetaMetricManager::IncrementFailureCounter(RestMetricType type)
{
    switch (type) {
        case RestMetricType::ALLOC:
            allocFailureCounter_++;
            return;
        case RestMetricType::BATCH_ALLOC:
            batchAllocFailureCounter_++;
            return;
        case RestMetricType::GET:
            getFailureCounter_++;
            return;
        case RestMetricType::BATCH_GET:
            batchGetFailureCounter_++;
            return;
        case RestMetricType::REMOVE:
            removeFailureCounter_++;
            return;
        case RestMetricType::BATCH_REMOVE:
            batchRemoveFailureCounter_++;
            return;
        case RestMetricType::REMOVE_ALL:
            removeAllFailureCounter_++;
            return;
        case RestMetricType::UPDATE_STATE:
            updateStateFailureCounter_++;
            return;
        case RestMetricType::BATCH_UPDATE_STATE:
            batchUpdateStateFailureCounter_++;
            return;
        case RestMetricType::QUERY:
            queryFailureCounter_++;
            return;
        case RestMetricType::BATCH_QUERY:
            batchQueryFailureCounter_++;
            return;
        case RestMetricType::GET_ALL_KEYS:
            getAllKeysFailureCounter_++;
            return;
        case RestMetricType::EXIST_KEY:
            existKeyFailureCounter_++;
            return;
        case RestMetricType::BATCH_EXIST_KEY:
            batchExistKeyFailureCounter_++;
            return;
        case RestMetricType::MOUNT:
            mountFailureCounter_++;
            return;
        case RestMetricType::UNMOUNT:
            unmountFailureCounter_++;
            return;
        default:
            return;
    }
}

} // namespace mmc
} // namespace ock
