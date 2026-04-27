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

#include "mmc_rest_api_facade.h"

#include <algorithm>
#include <chrono>
#include <cctype>
#include <ctime>
#include <sstream>

#include "ptracer.h"

#include "mmc_logger.h"
#include "mmc_meta_metric_manager.h"
#include "smem_config_store_errno.h"

namespace {

constexpr char kStatusOk[] = "ok";
constexpr int kStatusOkCode = 1;
constexpr char kUnknownValue[] = "unknown";
constexpr char kHaStateUnknown[] = "unknown";
constexpr char kLowerMediumHbm[] = "hbm";
constexpr char kLowerMediumDram[] = "dram";
constexpr char kLowerMediumSsd[] = "ssd";
constexpr char kPtracerAllocName[] = "TP_MMC_META_ALLOC";
constexpr char kPtracerRemoveName[] = "TP_MMC_META_REMOVE";

std::string ToLower(std::string value)
{
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) { return std::tolower(ch); });
    return value;
}

void AppendMetricHeader(std::ostringstream &oss, const std::string &metricName, const std::string &help,
                        const std::string &type)
{
    oss << "# HELP " << metricName << ' ' << help << '\n';
    oss << "# TYPE " << metricName << ' ' << type << '\n';
}

void AppendMetricValue(std::ostringstream &oss, const std::string &metricName, uint64_t value)
{
    oss << metricName << ' ' << value << '\n';
}

void AppendLabeledMetricValue(std::ostringstream &oss, const std::string &metricName, const std::string &labelName,
                              const std::string &labelValue, uint64_t value)
{
    oss << metricName << '{' << labelName << "=\"" << labelValue << "\"} " << value << '\n';
}

std::string BuildUsedText(const ock::mmc::RestUsageSnapshot &usage)
{
    return std::to_string(usage.usedBytes) + "/" + std::to_string(usage.totalBytes);
}

void AppendOperationMetrics(std::ostringstream &oss, const std::string &metricName, const std::string &helpName,
                            uint64_t requestCount, uint64_t successCount, uint64_t failureCount,
                            bool hasNotFoundCount = false, uint64_t notFoundCount = 0)
{
    const std::string requestMetricName = "memcache_" + metricName + "_requests_total";
    const std::string successMetricName = "memcache_" + metricName + "_successes_total";
    const std::string failureMetricName = "memcache_" + metricName + "_failures_total";
    AppendMetricHeader(oss, requestMetricName, "Total number of " + helpName + " requests", "counter");
    AppendMetricValue(oss, requestMetricName, requestCount);
    AppendMetricHeader(oss, successMetricName, "Total number of " + helpName + " successes", "counter");
    AppendMetricValue(oss, successMetricName, successCount);
    AppendMetricHeader(oss, failureMetricName, "Total number of " + helpName + " failures", "counter");
    AppendMetricValue(oss, failureMetricName, failureCount);
    if (hasNotFoundCount) {
        const std::string notFoundMetricName = "memcache_" + metricName + "_not_found_total";
        AppendMetricHeader(oss, notFoundMetricName, "Total number of " + helpName + " not found results", "counter");
        AppendMetricValue(oss, notFoundMetricName, notFoundCount);
    }
}

} // namespace

namespace ock {
namespace mmc {

MmcRestApiFacade::MmcRestApiFacade(MmcMetaService *metaService, const MmcMetaMgrProxyPtr &metaMgrProxy,
                                   const HaSnapshotProvider &haSnapshotProvider)
    : metaService_(metaService), metaMgrProxy_(metaMgrProxy), haSnapshotProvider_(haSnapshotProvider)
{
    if (metaMgrProxy_ != nullptr) {
        metaManager_ = metaMgrProxy_->GetMetaManager();
    }
}

Result MmcRestApiFacade::GetMetadata(const std::string &key, std::string &value) const
{
    MMC_VALIDATE_RETURN(metaService_ != nullptr, "meta service is nullptr", MMC_NOT_INITIALIZED);
    return metaService_->GetMetadata(key, value, 0);
}

Result MmcRestApiFacade::PutMetadata(const std::string &key, const std::string &value) const
{
    MMC_VALIDATE_RETURN(metaService_ != nullptr, "meta service is nullptr", MMC_NOT_INITIALIZED);
    return metaService_->PutMetadata(key, value);
}

Result MmcRestApiFacade::DeleteMetadata(const std::string &key) const
{
    MMC_VALIDATE_RETURN(metaService_ != nullptr, "meta service is nullptr", MMC_NOT_INITIALIZED);
    return metaService_->DeleteMetadata(key);
}

std::string MmcRestApiFacade::GetRole() const
{
    return GetHaSnapshot().role;
}

std::string MmcRestApiFacade::GetHaStatus() const
{
    return GetHaSnapshot().haState;
}

nlohmann::json MmcRestApiFacade::BuildHealth(bool serviceReady) const
{
    const RestHaSnapshot haSnapshot = GetHaSnapshot();
    nlohmann::json result;
    result["status"] = kStatusOk;
    result["role"] = haSnapshot.role;
    result["ha_state"] = haSnapshot.haState;
    result["service_ready"] = serviceReady;
    result["leader_address"] = haSnapshot.leaderAddress;
    result["view_version"] = haSnapshot.viewVersion;
    return result;
}

nlohmann::json MmcRestApiFacade::BuildLeader() const
{
    const RestHaSnapshot haSnapshot = GetHaSnapshot();
    nlohmann::json result;
    result["present"] = haSnapshot.leaderPresent;
    result["leader_address"] = haSnapshot.leaderAddress;
    result["view_version"] = haSnapshot.viewVersion;
    return result;
}

Result MmcRestApiFacade::QueryKey(const std::string &key, nlohmann::json &result) const
{
    MMC_VALIDATE_RETURN(metaMgrProxy_ != nullptr, "meta manager proxy is nullptr", MMC_NOT_INITIALIZED);
    QueryRequest request(key);
    QueryResponse response;
    Result ret = metaMgrProxy_->Query(request, response);
    if (ret != MMC_OK) {
        return ret;
    }
    result = response.queryInfo_.toJson(key);
    return MMC_OK;
}

Result MmcRestApiFacade::BatchQueryKeys(const std::vector<std::string> &keys, nlohmann::json &result) const
{
    MMC_VALIDATE_RETURN(metaMgrProxy_ != nullptr, "meta manager proxy is nullptr", MMC_NOT_INITIALIZED);
    BatchQueryRequest request(keys);
    BatchQueryResponse response;
    Result ret = metaMgrProxy_->BatchQuery(request, response);
    if (ret != MMC_OK) {
        return ret;
    }

    nlohmann::json data = nlohmann::json::array();
    for (size_t i = 0; i < keys.size(); ++i) {
        if (i < response.batchQueryInfos_.size()) {
            data.push_back(response.batchQueryInfos_[i].toJson(keys[i]));
        } else {
            MemObjQueryInfo queryInfo;
            data.push_back(queryInfo.toJson(keys[i]));
        }
    }

    result["success"] = true;
    result["data"] = data;
    return MMC_OK;
}

Result MmcRestApiFacade::GetAllKeys(std::vector<std::string> &keys) const
{
    MMC_VALIDATE_RETURN(metaManager_ != nullptr, "meta manager is nullptr", MMC_NOT_INITIALIZED);
    return metaManager_->GetAllKeys(keys);
}

Result MmcRestApiFacade::GetAllKeysText(std::string &result) const
{
    std::vector<std::string> keys;
    MMC_VALIDATE_RETURN(metaMgrProxy_ != nullptr, "meta manager proxy is nullptr", MMC_NOT_INITIALIZED);
    Result ret = metaMgrProxy_->GetAllKeys(keys);
    if (ret != MMC_OK) {
        return ret;
    }
    result = JoinLines(keys);
    return MMC_OK;
}

Result MmcRestApiFacade::GetAllSegmentSnapshots(std::vector<RestSegmentSnapshot> &segments) const
{
    nlohmann::json segmentInfo;
    Result ret = GetSegmentInfoJson(segmentInfo);
    if (ret != MMC_OK) {
        return ret;
    }

    if (!segmentInfo.is_array()) {
        MMC_LOG_ERROR("Segment info is not an array");
        return MMC_ERROR;
    }

    segments.clear();
    for (size_t i = 0; i < segmentInfo.size(); ++i) {
        const nlohmann::json &item = segmentInfo.at(i);
        if (!item.is_object()) {
            MMC_LOG_ERROR("Segment info item is not an object, index=" << i);
            return MMC_ERROR;
        }

        const uint32_t rank = item.value("rank", UINT32_MAX);
        const std::string medium = item.value("medium", std::string(kUnknownValue));
        const uint64_t totalBytes = item.value("capacity", static_cast<uint64_t>(0));
        const uint64_t usedBytes = item.value("allocatedSize", static_cast<uint64_t>(0));
        const uint64_t remainingBytes = totalBytes >= usedBytes ? (totalBytes - usedBytes) : 0;
        const double remainingRatio = totalBytes == 0 ? 0.0 : static_cast<double>(remainingBytes) / totalBytes;

        RestSegmentSnapshot snapshot;
        snapshot.segmentName = BuildSegmentId(rank, medium);
        snapshot.medium = medium;
        snapshot.totalBytes = totalBytes;
        snapshot.usedBytes = usedBytes;
        snapshot.remainingBytes = remainingBytes;
        snapshot.remainingRatio = remainingRatio;
        segments.push_back(snapshot);
    }
    return MMC_OK;
}

Result MmcRestApiFacade::GetAllSegmentsText(std::string &result) const
{
    MMC_VALIDATE_RETURN(metaMgrProxy_ != nullptr, "meta manager proxy is nullptr", MMC_NOT_INITIALIZED);
    nlohmann::json segmentInfo;
    Result ret = metaMgrProxy_->GetAllSegmentInfo(segmentInfo);
    if (ret != MMC_OK) {
        return ret;
    }

    if (!segmentInfo.is_array()) {
        MMC_LOG_ERROR("Segment info is not an array");
        return MMC_ERROR;
    }

    std::vector<std::string> segmentNames;
    segmentNames.reserve(segmentInfo.size());
    for (size_t i = 0; i < segmentInfo.size(); ++i) {
        const nlohmann::json &item = segmentInfo.at(i);
        if (!item.is_object()) {
            MMC_LOG_ERROR("Segment info item is not an object, index=" << i);
            return MMC_ERROR;
        }
        const uint32_t rank = item.value("rank", UINT32_MAX);
        const std::string medium = item.value("medium", std::string(kUnknownValue));
        segmentNames.push_back(BuildSegmentId(rank, medium));
    }
    result = JoinLines(segmentNames);
    return MMC_OK;
}

Result MmcRestApiFacade::QuerySegment(const std::string &segmentId, RestSegmentSnapshot &segment) const
{
    MMC_VALIDATE_RETURN(metaMgrProxy_ != nullptr, "meta manager proxy is nullptr", MMC_NOT_INITIALIZED);
    nlohmann::json segmentInfo;
    Result ret = metaMgrProxy_->QuerySegment(segmentId, segmentInfo);
    if (ret != MMC_OK) {
        return ret;
    }

    if (!segmentInfo.is_object()) {
        MMC_LOG_ERROR("Segment info item is not an object");
        return MMC_ERROR;
    }

    const uint32_t rank = segmentInfo.value("rank", UINT32_MAX);
    const std::string medium = segmentInfo.value("medium", std::string(kUnknownValue));
    segment.segmentName = BuildSegmentId(rank, medium);
    segment.medium = medium;
    segment.totalBytes = segmentInfo.value("capacity", static_cast<uint64_t>(0));
    segment.usedBytes = segmentInfo.value("allocatedSize", static_cast<uint64_t>(0));
    segment.remainingBytes = segment.totalBytes >= segment.usedBytes ? (segment.totalBytes - segment.usedBytes) : 0;
    segment.remainingRatio =
        segment.totalBytes == 0 ? 0.0 : static_cast<double>(segment.remainingBytes) / segment.totalBytes;
    return MMC_OK;
}

nlohmann::json MmcRestApiFacade::BuildSegment(const RestSegmentSnapshot &segment) const
{
    nlohmann::json result;
    result["segment"] = segment.segmentName;
    result["medium"] = segment.medium;
    result["total_bytes"] = segment.totalBytes;
    result["used_bytes"] = segment.usedBytes;
    result["remaining_bytes"] = segment.remainingBytes;
    result["remaining_ratio"] = segment.remainingRatio;
    return result;
}

Result MmcRestApiFacade::BuildSegmentStatus(const std::string &segmentId, nlohmann::json &result) const
{
    std::vector<RestSegmentSnapshot> segments;
    Result ret = GetAllSegmentSnapshots(segments);
    if (ret != MMC_OK) {
        return ret;
    }

    RestSegmentSnapshot segment;
    bool found = false;
    for (size_t i = 0; i < segments.size(); ++i) {
        if (segments[i].segmentName == segmentId) {
            segment = segments[i];
            found = true;
            break;
        }
    }
    if (!found) {
        return MMC_UNMATCHED_KEY;
    }

    result["success"] = true;
    result["segment"] = segment.segmentName;
    result["status"] = kStatusOkCode;
    result["status_name"] = "OK";
    return MMC_OK;
}

Result MmcRestApiFacade::BuildCapacityUsage(nlohmann::json &result) const
{
    std::vector<RestSegmentSnapshot> segments;
    Result ret = GetAllSegmentSnapshots(segments);
    if (ret != MMC_OK) {
        return ret;
    }

    RestUsageSnapshot hbmUsage;
    RestUsageSnapshot dramUsage;
    ret = BuildUsageFromMedium(segments, kLowerMediumHbm, hbmUsage);
    if (ret != MMC_OK) {
        return ret;
    }
    ret = BuildUsageFromMedium(segments, kLowerMediumDram, dramUsage);
    if (ret != MMC_OK) {
        return ret;
    }

    result["timestamp"] = CurrentTimestamp();
    result["data_source_ready"] = true;
    result["degraded"] = false;
    result["npu"] = {
        {"total_bytes", hbmUsage.totalBytes},
        {"used_bytes", hbmUsage.usedBytes},
        {"free_bytes", hbmUsage.freeBytes},
        {"usage_ratio", hbmUsage.usageRatio},
    };
    result["cpu"] = {
        {"total_bytes", dramUsage.totalBytes},
        {"used_bytes", dramUsage.usedBytes},
        {"free_bytes", dramUsage.freeBytes},
        {"usage_ratio", dramUsage.usageRatio},
    };
    return MMC_OK;
}

Result MmcRestApiFacade::BuildSegmentRemaining(nlohmann::json &result) const
{
    std::vector<RestSegmentSnapshot> segments;
    Result ret = GetAllSegmentSnapshots(segments);
    if (ret != MMC_OK) {
        return ret;
    }

    nlohmann::json segmentArray = nlohmann::json::array();
    for (size_t i = 0; i < segments.size(); ++i) {
        nlohmann::json item;
        item["segment_name"] = segments[i].segmentName;
        item["medium"] = segments[i].medium;
        item["total_bytes"] = segments[i].totalBytes;
        item["used_bytes"] = segments[i].usedBytes;
        item["remaining_bytes"] = segments[i].remainingBytes;
        item["remaining_ratio"] = segments[i].remainingRatio;
        segmentArray.push_back(item);
    }

    result["timestamp"] = CurrentTimestamp();
    result["data_source_ready"] = true;
    result["degraded"] = false;
    result["segments"] = segmentArray;
    return MMC_OK;
}

Result MmcRestApiFacade::BuildMetricsSummary(bool serviceReady, std::string &result) const
{
    (void)serviceReady;
    std::vector<RestSegmentSnapshot> segments;
    Result ret = GetAllSegmentSnapshots(segments);
    if (ret != MMC_OK) {
        return ret;
    }

    std::vector<std::string> keys;
    ret = GetAllKeys(keys);
    if (ret != MMC_OK) {
        return ret;
    }

    MmcMetaMetricManager &metricManager = MmcMetaMetricManager::GetInstance();
    metricManager.SetKeyCount(keys.size());
    const MmcMetaMetricSnapshot metricSnapshot = metricManager.GetSnapshot();
    RestUsageSnapshot hbmUsage;
    RestUsageSnapshot dramUsage;
    ret = BuildUsageFromMedium(segments, kLowerMediumHbm, hbmUsage);
    if (ret != MMC_OK) {
        return ret;
    }
    ret = BuildUsageFromMedium(segments, kLowerMediumDram, dramUsage);
    if (ret != MMC_OK) {
        return ret;
    }

    std::ostringstream oss;
    oss << "keys=" << keys.size() << " evict=" << metricSnapshot.evictCount << " hbm_used=" << BuildUsedText(hbmUsage)
        << " dram_used=" << BuildUsedText(dramUsage) << " alloc_req=" << metricSnapshot.allocRequestCount
        << " alloc_success=" << metricSnapshot.allocSuccessCount << " alloc_fail=" << metricSnapshot.allocFailureCount
        << " batch_alloc_req=" << metricSnapshot.batchAllocRequestCount
        << " batch_alloc_success=" << metricSnapshot.batchAllocSuccessCount
        << " batch_alloc_fail=" << metricSnapshot.batchAllocFailureCount
        << " get_req=" << metricSnapshot.getRequestCount << " get_success=" << metricSnapshot.getSuccessCount
        << " get_fail=" << metricSnapshot.getFailureCount << " get_not_found=" << metricSnapshot.getNotFoundCount
        << " batch_get_req=" << metricSnapshot.batchGetRequestCount
        << " batch_get_success=" << metricSnapshot.batchGetSuccessCount
        << " batch_get_fail=" << metricSnapshot.batchGetFailureCount
        << " batch_get_not_found=" << metricSnapshot.batchGetNotFoundCount
        << " remove_req=" << metricSnapshot.removeRequestCount
        << " remove_success=" << metricSnapshot.removeSuccessCount
        << " remove_fail=" << metricSnapshot.removeFailureCount
        << " remove_not_found=" << metricSnapshot.removeNotFoundCount
        << " batch_remove_req=" << metricSnapshot.batchRemoveRequestCount
        << " batch_remove_success=" << metricSnapshot.batchRemoveSuccessCount
        << " batch_remove_fail=" << metricSnapshot.batchRemoveFailureCount
        << " batch_remove_not_found=" << metricSnapshot.batchRemoveNotFoundCount
        << " remove_all_req=" << metricSnapshot.removeAllRequestCount
        << " remove_all_success=" << metricSnapshot.removeAllSuccessCount
        << " remove_all_fail=" << metricSnapshot.removeAllFailureCount
        << " update_state_req=" << metricSnapshot.updateStateRequestCount
        << " update_state_success=" << metricSnapshot.updateStateSuccessCount
        << " update_state_fail=" << metricSnapshot.updateStateFailureCount
        << " update_state_not_found=" << metricSnapshot.updateStateNotFoundCount
        << " batch_update_state_req=" << metricSnapshot.batchUpdateStateRequestCount
        << " batch_update_state_success=" << metricSnapshot.batchUpdateStateSuccessCount
        << " batch_update_state_fail=" << metricSnapshot.batchUpdateStateFailureCount
        << " batch_update_state_not_found=" << metricSnapshot.batchUpdateStateNotFoundCount
        << " query_req=" << metricSnapshot.queryRequestCount << " query_success=" << metricSnapshot.querySuccessCount
        << " query_fail=" << metricSnapshot.queryFailureCount
        << " query_not_found=" << metricSnapshot.queryNotFoundCount
        << " batch_query_req=" << metricSnapshot.batchQueryRequestCount
        << " batch_query_success=" << metricSnapshot.batchQuerySuccessCount
        << " batch_query_fail=" << metricSnapshot.batchQueryFailureCount
        << " batch_query_not_found=" << metricSnapshot.batchQueryNotFoundCount
        << " get_all_keys_req=" << metricSnapshot.getAllKeysRequestCount
        << " get_all_keys_success=" << metricSnapshot.getAllKeysSuccessCount
        << " get_all_keys_fail=" << metricSnapshot.getAllKeysFailureCount
        << " exist_key_req=" << metricSnapshot.existKeyRequestCount
        << " exist_key_success=" << metricSnapshot.existKeySuccessCount
        << " exist_key_fail=" << metricSnapshot.existKeyFailureCount
        << " exist_key_not_found=" << metricSnapshot.existKeyNotFoundCount
        << " batch_exist_key_req=" << metricSnapshot.batchExistKeyRequestCount
        << " batch_exist_key_success=" << metricSnapshot.batchExistKeySuccessCount
        << " batch_exist_key_fail=" << metricSnapshot.batchExistKeyFailureCount
        << " batch_exist_key_not_found=" << metricSnapshot.batchExistKeyNotFoundCount
        << " mount_req=" << metricSnapshot.mountRequestCount << " mount_success=" << metricSnapshot.mountSuccessCount
        << " mount_fail=" << metricSnapshot.mountFailureCount << " unmount_req=" << metricSnapshot.unmountRequestCount
        << " unmount_success=" << metricSnapshot.unmountSuccessCount
        << " unmount_fail=" << metricSnapshot.unmountFailureCount;
    result = oss.str();
    return MMC_OK;
}

Result MmcRestApiFacade::BuildPrometheusMetrics(bool serviceReady, std::string &result) const
{
    (void)serviceReady;
    std::vector<RestSegmentSnapshot> segments;
    Result ret = GetAllSegmentSnapshots(segments);
    if (ret != MMC_OK) {
        return ret;
    }

    std::vector<std::string> keys;
    ret = GetAllKeys(keys);
    if (ret != MMC_OK) {
        return ret;
    }

    MmcMetaMetricManager &metricManager = MmcMetaMetricManager::GetInstance();
    metricManager.SetKeyCount(keys.size());
    const MmcMetaMetricSnapshot metricSnapshot = metricManager.GetSnapshot();

    RestUsageSnapshot hbmUsage;
    RestUsageSnapshot dramUsage;
    RestUsageSnapshot ssdUsage;
    ret = BuildUsageFromMedium(segments, kLowerMediumHbm, hbmUsage);
    if (ret != MMC_OK) {
        return ret;
    }
    ret = BuildUsageFromMedium(segments, kLowerMediumDram, dramUsage);
    if (ret != MMC_OK) {
        return ret;
    }
    ret = BuildUsageFromMedium(segments, kLowerMediumSsd, ssdUsage);
    if (ret != MMC_OK) {
        return ret;
    }

    std::ostringstream oss;
    AppendOperationMetrics(oss, "alloc", "Alloc", metricSnapshot.allocRequestCount, metricSnapshot.allocSuccessCount,
                           metricSnapshot.allocFailureCount);
    AppendOperationMetrics(oss, "batch_alloc", "BatchAlloc", metricSnapshot.batchAllocRequestCount,
                           metricSnapshot.batchAllocSuccessCount, metricSnapshot.batchAllocFailureCount);
    AppendOperationMetrics(oss, "get", "Get", metricSnapshot.getRequestCount, metricSnapshot.getSuccessCount,
                           metricSnapshot.getFailureCount, true, metricSnapshot.getNotFoundCount);
    AppendOperationMetrics(oss, "batch_get", "BatchGet", metricSnapshot.batchGetRequestCount,
                           metricSnapshot.batchGetSuccessCount, metricSnapshot.batchGetFailureCount, true,
                           metricSnapshot.batchGetNotFoundCount);
    AppendOperationMetrics(oss, "remove", "Remove", metricSnapshot.removeRequestCount,
                           metricSnapshot.removeSuccessCount, metricSnapshot.removeFailureCount, true,
                           metricSnapshot.removeNotFoundCount);
    AppendOperationMetrics(oss, "batch_remove", "BatchRemove", metricSnapshot.batchRemoveRequestCount,
                           metricSnapshot.batchRemoveSuccessCount, metricSnapshot.batchRemoveFailureCount, true,
                           metricSnapshot.batchRemoveNotFoundCount);
    AppendOperationMetrics(oss, "remove_all", "RemoveAll", metricSnapshot.removeAllRequestCount,
                           metricSnapshot.removeAllSuccessCount, metricSnapshot.removeAllFailureCount);
    AppendOperationMetrics(oss, "update_state", "UpdateState", metricSnapshot.updateStateRequestCount,
                           metricSnapshot.updateStateSuccessCount, metricSnapshot.updateStateFailureCount, true,
                           metricSnapshot.updateStateNotFoundCount);
    AppendOperationMetrics(oss, "batch_update_state", "BatchUpdateState", metricSnapshot.batchUpdateStateRequestCount,
                           metricSnapshot.batchUpdateStateSuccessCount, metricSnapshot.batchUpdateStateFailureCount,
                           true, metricSnapshot.batchUpdateStateNotFoundCount);
    AppendOperationMetrics(oss, "query", "Query", metricSnapshot.queryRequestCount, metricSnapshot.querySuccessCount,
                           metricSnapshot.queryFailureCount, true, metricSnapshot.queryNotFoundCount);
    AppendOperationMetrics(oss, "batch_query", "BatchQuery", metricSnapshot.batchQueryRequestCount,
                           metricSnapshot.batchQuerySuccessCount, metricSnapshot.batchQueryFailureCount, true,
                           metricSnapshot.batchQueryNotFoundCount);
    AppendOperationMetrics(oss, "get_all_keys", "GetAllKeys", metricSnapshot.getAllKeysRequestCount,
                           metricSnapshot.getAllKeysSuccessCount, metricSnapshot.getAllKeysFailureCount);
    AppendOperationMetrics(oss, "exist_key", "ExistKey", metricSnapshot.existKeyRequestCount,
                           metricSnapshot.existKeySuccessCount, metricSnapshot.existKeyFailureCount, true,
                           metricSnapshot.existKeyNotFoundCount);
    AppendOperationMetrics(oss, "batch_exist_key", "BatchExistKey", metricSnapshot.batchExistKeyRequestCount,
                           metricSnapshot.batchExistKeySuccessCount, metricSnapshot.batchExistKeyFailureCount, true,
                           metricSnapshot.batchExistKeyNotFoundCount);
    AppendOperationMetrics(oss, "mount", "Mount", metricSnapshot.mountRequestCount, metricSnapshot.mountSuccessCount,
                           metricSnapshot.mountFailureCount);
    AppendOperationMetrics(oss, "unmount", "Unmount", metricSnapshot.unmountRequestCount,
                           metricSnapshot.unmountSuccessCount, metricSnapshot.unmountFailureCount);
    AppendMetricHeader(oss, "memcache_evict_operations_total", "Total number of evict operations", "counter");
    AppendMetricValue(oss, "memcache_evict_operations_total", metricSnapshot.evictCount);
    AppendMetricHeader(oss, "memcache_stored_keys", "Total number of stored keys", "gauge");
    AppendMetricValue(oss, "memcache_stored_keys", keys.size());

    AppendMetricHeader(oss, "memcache_segment_capacity_bytes", "Segment total capacity in bytes", "gauge");
    for (size_t i = 0; i < segments.size(); ++i) {
        AppendLabeledMetricValue(oss, "memcache_segment_capacity_bytes", "segment", segments[i].segmentName,
                                 segments[i].totalBytes);
    }
    AppendMetricHeader(oss, "memcache_segment_allocated_bytes", "Segment allocated bytes", "gauge");
    for (size_t i = 0; i < segments.size(); ++i) {
        AppendLabeledMetricValue(oss, "memcache_segment_allocated_bytes", "segment", segments[i].segmentName,
                                 segments[i].usedBytes);
    }

    AppendMetricHeader(oss, "memcache_total_capacity_bytes", "Total capacity by medium in bytes", "gauge");
    AppendLabeledMetricValue(oss, "memcache_total_capacity_bytes", "medium", kLowerMediumHbm, hbmUsage.totalBytes);
    AppendLabeledMetricValue(oss, "memcache_total_capacity_bytes", "medium", kLowerMediumDram, dramUsage.totalBytes);
    if (ssdUsage.totalBytes > 0 || ssdUsage.usedBytes > 0) {
        AppendLabeledMetricValue(oss, "memcache_total_capacity_bytes", "medium", kLowerMediumSsd, ssdUsage.totalBytes);
    }

    AppendMetricHeader(oss, "memcache_allocated_bytes", "Allocated bytes by medium", "gauge");
    AppendLabeledMetricValue(oss, "memcache_allocated_bytes", "medium", kLowerMediumHbm, hbmUsage.usedBytes);
    AppendLabeledMetricValue(oss, "memcache_allocated_bytes", "medium", kLowerMediumDram, dramUsage.usedBytes);
    if (ssdUsage.totalBytes > 0 || ssdUsage.usedBytes > 0) {
        AppendLabeledMetricValue(oss, "memcache_allocated_bytes", "medium", kLowerMediumSsd, ssdUsage.usedBytes);
    }

    result = oss.str();
    return MMC_OK;
}

Result MmcRestApiFacade::GetPtracerText(std::string &result) const
{
    const char *ptracerText = ptracer_get_all_tp_string();
    if (ptracerText == nullptr) {
        MMC_LOG_ERROR("ptracer_get_all_tp_string failed");
        return MMC_ERROR;
    }
    result = ptracerText;
    return MMC_OK;
}

Result MmcRestApiFacade::GetAllocFreeLatencyText(std::string &result) const
{
    std::string ptracerText;
    Result ret = GetPtracerText(ptracerText);
    if (ret != MMC_OK) {
        return ret;
    }

    std::istringstream iss(ptracerText);
    std::ostringstream oss;
    std::string line;
    bool firstLine = true;
    while (std::getline(iss, line)) {
        if (firstLine) {
            oss << line << '\n';
            firstLine = false;
            continue;
        }
        if (line.find(kPtracerAllocName) != std::string::npos || line.find(kPtracerRemoveName) != std::string::npos) {
            oss << line << '\n';
        }
    }
    result = oss.str();
    return MMC_OK;
}

RestHaSnapshot MmcRestApiFacade::GetHaSnapshot() const
{
    if (!haSnapshotProvider_) {
        return RestHaSnapshot{};
    }
    return haSnapshotProvider_();
}

Result MmcRestApiFacade::GetSegmentInfoJson(nlohmann::json &result) const
{
    MMC_VALIDATE_RETURN(metaManager_ != nullptr, "meta manager is nullptr", MMC_NOT_INITIALIZED);
    result = metaManager_->GetAllSegmentInfo();
    return MMC_OK;
}

Result MmcRestApiFacade::BuildUsageFromMedium(const std::vector<RestSegmentSnapshot> &segments,
                                              const std::string &medium, RestUsageSnapshot &usage) const
{
    usage = RestUsageSnapshot{};
    const std::string targetMedium = ToLower(medium);
    for (size_t i = 0; i < segments.size(); ++i) {
        if (ToLower(segments[i].medium) != targetMedium) {
            continue;
        }
        usage.totalBytes += segments[i].totalBytes;
        usage.usedBytes += segments[i].usedBytes;
    }
    usage.freeBytes = usage.totalBytes >= usage.usedBytes ? (usage.totalBytes - usage.usedBytes) : 0;
    usage.usageRatio = usage.totalBytes == 0 ? 0.0 : static_cast<double>(usage.usedBytes) / usage.totalBytes;
    return MMC_OK;
}

std::string MmcRestApiFacade::BuildSegmentId(uint32_t rank, const std::string &medium)
{
    return std::string("rank-") + std::to_string(rank) + "-" + ToLower(medium);
}

std::string MmcRestApiFacade::JoinLines(const std::vector<std::string> &items)
{
    std::ostringstream oss;
    for (size_t i = 0; i < items.size(); ++i) {
        oss << items[i] << '\n';
    }
    return oss.str();
}

uint64_t MmcRestApiFacade::CurrentTimestamp()
{
    return static_cast<uint64_t>(std::time(nullptr));
}

} // namespace mmc
} // namespace ock
