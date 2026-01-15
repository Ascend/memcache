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

#include <string>

#include "prometheus/simpleapi.h"

namespace ock {
namespace mmc {

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

    // Get metrics summary in human-readable format
    std::string GetSummary() const;
    // Get metrics summary in prometheus format
    static std::string GetPrometheusSummary();

    void IncrementAllocCounter()
    {
        allocCounter_++;
    }
    void IncrementRemoveCounter()
    {
        removeCounter_++;
    }
    void IncrementGetCounter()
    {
        getCounter_++;
    }
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

    prometheus::simpleapi::counter_metric_t allocCounter_;
    prometheus::simpleapi::counter_metric_t removeCounter_;
    prometheus::simpleapi::counter_metric_t getCounter_;
    prometheus::simpleapi::counter_metric_t evictCounter_;
    prometheus::simpleapi::gauge_metric_t keyCountGauge_;
};

} // namespace mmc
} // namespace ock

#endif