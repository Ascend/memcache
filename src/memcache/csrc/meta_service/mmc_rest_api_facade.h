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

#ifndef MMC_REST_API_FACADE_H
#define MMC_REST_API_FACADE_H

#include <functional>
#include <string>
#include <vector>

#include "nlohmann/json.hpp"

#include "mmc_meta_mgr_proxy.h"
#include "mmc_meta_service.h"
#include "mmc_ref.h"

namespace ock {
namespace mmc {

struct RestHaSnapshot {
    std::string role{"unknown"};
    std::string haState{"unknown"};
    bool leaderPresent{false};
    std::string leaderAddress{"unknown"};
    uint64_t viewVersion{0};
};

struct RestUsageSnapshot {
    uint64_t totalBytes{0};
    uint64_t usedBytes{0};
    uint64_t freeBytes{0};
    double usageRatio{0.0};
};

struct RestSegmentSnapshot {
    std::string segmentName;
    std::string medium;
    uint64_t totalBytes{0};
    uint64_t usedBytes{0};
    uint64_t remainingBytes{0};
    double remainingRatio{0.0};
};

class MmcRestApiFacade : public MmcReferable {
public:
    using HaSnapshotProvider = std::function<RestHaSnapshot(void)>;

    MmcRestApiFacade(MmcMetaService *metaService, const MmcMetaMgrProxyPtr &metaMgrProxy,
                     const HaSnapshotProvider &haSnapshotProvider = HaSnapshotProvider());
    ~MmcRestApiFacade() override = default;

    Result GetMetadata(const std::string &key, std::string &value) const;
    Result PutMetadata(const std::string &key, const std::string &value) const;
    Result DeleteMetadata(const std::string &key) const;

    std::string GetRole() const;
    std::string GetHaStatus() const;
    nlohmann::json BuildHealth(bool serviceReady) const;
    nlohmann::json BuildLeader() const;

    Result QueryKey(const std::string &key, nlohmann::json &result) const;
    Result BatchQueryKeys(const std::vector<std::string> &keys, nlohmann::json &result) const;
    Result GetAllKeys(std::vector<std::string> &keys) const;
    Result GetAllKeysText(std::string &result) const;

    Result GetAllSegmentSnapshots(std::vector<RestSegmentSnapshot> &segments) const;
    Result GetAllSegmentsText(std::string &result) const;
    Result QuerySegment(const std::string &segmentId, RestSegmentSnapshot &segment) const;
    nlohmann::json BuildSegment(const RestSegmentSnapshot &segment) const;
    Result BuildSegmentStatus(const std::string &segmentId, nlohmann::json &result) const;
    Result BuildCapacityUsage(nlohmann::json &result) const;
    Result BuildSegmentRemaining(nlohmann::json &result) const;

    Result BuildMetricsSummary(bool serviceReady, std::string &result) const;
    Result BuildPrometheusMetrics(bool serviceReady, std::string &result) const;

    Result GetPtracerText(std::string &result) const;
    Result GetAllocFreeLatencyText(std::string &result) const;

private:
    RestHaSnapshot GetHaSnapshot() const;
    Result GetSegmentInfoJson(nlohmann::json &result) const;
    Result BuildUsageFromMedium(const std::vector<RestSegmentSnapshot> &segments, const std::string &medium,
                                RestUsageSnapshot &usage) const;
    static std::string BuildSegmentId(uint32_t rank, const std::string &medium);
    static std::string JoinLines(const std::vector<std::string> &items);
    static uint64_t CurrentTimestamp();

private:
    MmcMetaService *metaService_{nullptr};
    MmcMetaMgrProxyPtr metaMgrProxy_;
    MmcMetaManagerPtr metaManager_;
    HaSnapshotProvider haSnapshotProvider_;
};

using MmcRestApiFacadePtr = MmcRef<MmcRestApiFacade>;

} // namespace mmc
} // namespace ock

#endif
