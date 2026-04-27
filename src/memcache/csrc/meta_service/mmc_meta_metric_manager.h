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

#ifndef MMC_META_METRIC_MANAGER_H
#define MMC_META_METRIC_MANAGER_H

#include <cstdint>
#include <stdexcept>

#include "prometheus/simpleapi.h"

namespace ock {
namespace mmc {

enum class RestMetricType : uint8_t {
    ALLOC,
    BATCH_ALLOC,
    GET,
    BATCH_GET,
    REMOVE,
    BATCH_REMOVE,
    REMOVE_ALL,
    UPDATE_STATE,
    BATCH_UPDATE_STATE,
    QUERY,
    BATCH_QUERY,
    GET_ALL_KEYS,
    EXIST_KEY,
    BATCH_EXIST_KEY,
    MOUNT,
    UNMOUNT,
};

struct MmcMetaMetricSnapshot {
    uint64_t allocRequestCount{0};
    uint64_t allocSuccessCount{0};
    uint64_t allocFailureCount{0};
    uint64_t batchAllocRequestCount{0};
    uint64_t batchAllocSuccessCount{0};
    uint64_t batchAllocFailureCount{0};
    uint64_t getRequestCount{0};
    uint64_t getSuccessCount{0};
    uint64_t getFailureCount{0};
    uint64_t getNotFoundCount{0};
    uint64_t batchGetRequestCount{0};
    uint64_t batchGetSuccessCount{0};
    uint64_t batchGetFailureCount{0};
    uint64_t batchGetNotFoundCount{0};
    uint64_t removeRequestCount{0};
    uint64_t removeSuccessCount{0};
    uint64_t removeFailureCount{0};
    uint64_t removeNotFoundCount{0};
    uint64_t batchRemoveRequestCount{0};
    uint64_t batchRemoveSuccessCount{0};
    uint64_t batchRemoveFailureCount{0};
    uint64_t batchRemoveNotFoundCount{0};
    uint64_t removeAllRequestCount{0};
    uint64_t removeAllSuccessCount{0};
    uint64_t removeAllFailureCount{0};
    uint64_t updateStateRequestCount{0};
    uint64_t updateStateSuccessCount{0};
    uint64_t updateStateFailureCount{0};
    uint64_t updateStateNotFoundCount{0};
    uint64_t batchUpdateStateRequestCount{0};
    uint64_t batchUpdateStateSuccessCount{0};
    uint64_t batchUpdateStateFailureCount{0};
    uint64_t batchUpdateStateNotFoundCount{0};
    uint64_t queryRequestCount{0};
    uint64_t querySuccessCount{0};
    uint64_t queryFailureCount{0};
    uint64_t queryNotFoundCount{0};
    uint64_t batchQueryRequestCount{0};
    uint64_t batchQuerySuccessCount{0};
    uint64_t batchQueryFailureCount{0};
    uint64_t batchQueryNotFoundCount{0};
    uint64_t getAllKeysRequestCount{0};
    uint64_t getAllKeysSuccessCount{0};
    uint64_t getAllKeysFailureCount{0};
    uint64_t existKeyRequestCount{0};
    uint64_t existKeySuccessCount{0};
    uint64_t existKeyFailureCount{0};
    uint64_t existKeyNotFoundCount{0};
    uint64_t batchExistKeyRequestCount{0};
    uint64_t batchExistKeySuccessCount{0};
    uint64_t batchExistKeyFailureCount{0};
    uint64_t batchExistKeyNotFoundCount{0};
    uint64_t mountRequestCount{0};
    uint64_t mountSuccessCount{0};
    uint64_t mountFailureCount{0};
    uint64_t unmountRequestCount{0};
    uint64_t unmountSuccessCount{0};
    uint64_t unmountFailureCount{0};
    uint64_t evictCount{0};
    uint64_t keyCount{0};
};

class MmcMetaMetricManager {
public:
    static MmcMetaMetricManager &GetInstance()
    {
        static MmcMetaMetricManager staticInstance;
        return staticInstance;
    }

    MmcMetaMetricManager(const MmcMetaMetricManager &) = delete;
    MmcMetaMetricManager &operator=(const MmcMetaMetricManager &) = delete;
    MmcMetaMetricManager(MmcMetaMetricManager &&) = delete;
    MmcMetaMetricManager &operator=(MmcMetaMetricManager &&) = delete;

    MmcMetaMetricSnapshot GetSnapshot() const;

    void IncrementRequestCounter(RestMetricType type);

    void IncrementSuccessCounter(RestMetricType type);

    void IncrementFailureCounter(RestMetricType type);

    void IncrementNotFoundCounter(RestMetricType type);

    void IncrementEvictCounter()
    {
        evictCounter_++;
    }
    void SetKeyCount(const size_t count)
    {
        keyCountGauge_ = static_cast<int64_t>(count);
    }

private:
    MmcMetaMetricManager();
    ~MmcMetaMetricManager() = default;

    prometheus::simpleapi::counter_metric_t allocRequestCounter_;
    prometheus::simpleapi::counter_metric_t allocSuccessCounter_;
    prometheus::simpleapi::counter_metric_t allocFailureCounter_;
    prometheus::simpleapi::counter_metric_t batchAllocRequestCounter_;
    prometheus::simpleapi::counter_metric_t batchAllocSuccessCounter_;
    prometheus::simpleapi::counter_metric_t batchAllocFailureCounter_;
    prometheus::simpleapi::counter_metric_t getRequestCounter_;
    prometheus::simpleapi::counter_metric_t getSuccessCounter_;
    prometheus::simpleapi::counter_metric_t getFailureCounter_;
    prometheus::simpleapi::counter_metric_t getNotFoundCounter_;
    prometheus::simpleapi::counter_metric_t batchGetRequestCounter_;
    prometheus::simpleapi::counter_metric_t batchGetSuccessCounter_;
    prometheus::simpleapi::counter_metric_t batchGetFailureCounter_;
    prometheus::simpleapi::counter_metric_t batchGetNotFoundCounter_;
    prometheus::simpleapi::counter_metric_t removeRequestCounter_;
    prometheus::simpleapi::counter_metric_t removeSuccessCounter_;
    prometheus::simpleapi::counter_metric_t removeFailureCounter_;
    prometheus::simpleapi::counter_metric_t removeNotFoundCounter_;
    prometheus::simpleapi::counter_metric_t batchRemoveRequestCounter_;
    prometheus::simpleapi::counter_metric_t batchRemoveSuccessCounter_;
    prometheus::simpleapi::counter_metric_t batchRemoveFailureCounter_;
    prometheus::simpleapi::counter_metric_t batchRemoveNotFoundCounter_;
    prometheus::simpleapi::counter_metric_t removeAllRequestCounter_;
    prometheus::simpleapi::counter_metric_t removeAllSuccessCounter_;
    prometheus::simpleapi::counter_metric_t removeAllFailureCounter_;
    prometheus::simpleapi::counter_metric_t updateStateRequestCounter_;
    prometheus::simpleapi::counter_metric_t updateStateSuccessCounter_;
    prometheus::simpleapi::counter_metric_t updateStateFailureCounter_;
    prometheus::simpleapi::counter_metric_t updateStateNotFoundCounter_;
    prometheus::simpleapi::counter_metric_t batchUpdateStateRequestCounter_;
    prometheus::simpleapi::counter_metric_t batchUpdateStateSuccessCounter_;
    prometheus::simpleapi::counter_metric_t batchUpdateStateFailureCounter_;
    prometheus::simpleapi::counter_metric_t batchUpdateStateNotFoundCounter_;
    prometheus::simpleapi::counter_metric_t queryRequestCounter_;
    prometheus::simpleapi::counter_metric_t querySuccessCounter_;
    prometheus::simpleapi::counter_metric_t queryFailureCounter_;
    prometheus::simpleapi::counter_metric_t queryNotFoundCounter_;
    prometheus::simpleapi::counter_metric_t batchQueryRequestCounter_;
    prometheus::simpleapi::counter_metric_t batchQuerySuccessCounter_;
    prometheus::simpleapi::counter_metric_t batchQueryFailureCounter_;
    prometheus::simpleapi::counter_metric_t batchQueryNotFoundCounter_;
    prometheus::simpleapi::counter_metric_t getAllKeysRequestCounter_;
    prometheus::simpleapi::counter_metric_t getAllKeysSuccessCounter_;
    prometheus::simpleapi::counter_metric_t getAllKeysFailureCounter_;
    prometheus::simpleapi::counter_metric_t existKeyRequestCounter_;
    prometheus::simpleapi::counter_metric_t existKeySuccessCounter_;
    prometheus::simpleapi::counter_metric_t existKeyFailureCounter_;
    prometheus::simpleapi::counter_metric_t existKeyNotFoundCounter_;
    prometheus::simpleapi::counter_metric_t batchExistKeyRequestCounter_;
    prometheus::simpleapi::counter_metric_t batchExistKeySuccessCounter_;
    prometheus::simpleapi::counter_metric_t batchExistKeyFailureCounter_;
    prometheus::simpleapi::counter_metric_t batchExistKeyNotFoundCounter_;
    prometheus::simpleapi::counter_metric_t mountRequestCounter_;
    prometheus::simpleapi::counter_metric_t mountSuccessCounter_;
    prometheus::simpleapi::counter_metric_t mountFailureCounter_;
    prometheus::simpleapi::counter_metric_t unmountRequestCounter_;
    prometheus::simpleapi::counter_metric_t unmountSuccessCounter_;
    prometheus::simpleapi::counter_metric_t unmountFailureCounter_;
    prometheus::simpleapi::counter_metric_t evictCounter_;
    prometheus::simpleapi::gauge_metric_t keyCountGauge_;
};

} // namespace mmc
} // namespace ock

#endif
