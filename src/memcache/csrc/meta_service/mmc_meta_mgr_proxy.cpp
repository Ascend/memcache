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
#include "mmc_meta_mgr_proxy.h"
#include "mmc_ptracer.h"

#include <algorithm>
#include <cctype>

namespace {

std::string ToLower(std::string value)
{
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) { return std::tolower(ch); });
    return value;
}

std::string BuildSegmentId(uint32_t rank, const std::string &medium)
{
    return std::string("rank-") + std::to_string(rank) + "-" + ToLower(medium);
}

} // namespace

namespace ock {
namespace mmc {
Result MmcMetaMgrProxy::Alloc(const AllocRequest &req, AllocResponse &resp)
{
    MmcMetaMetricManager &metricManager = MmcMetaMetricManager::GetInstance();
    metricManager.IncrementRequestCounter(RestMetricType::ALLOC);
    metaMangerPtr_->CheckAndEvict(static_cast<MediaType>(req.options_.mediaType_),
                                  req.options_.blobSize_ * req.options_.numBlobs_);
    MmcMemMetaDesc objMeta;
    auto ret = metaMangerPtr_->Alloc(req.key_, req.options_, req.operateId_, objMeta);
    IncrementResultCounter(metricManager, RestMetricType::ALLOC, ret);
    if (ret != MMC_OK) {
        if (ret != MMC_DUPLICATED_OBJECT) {
            MMC_RETURN_ERROR(ret, "Meta Alloc Fail, key  " << req.key_);
        } else {
            return ret;
        }
    }
    resp.numBlobs_ = objMeta.numBlobs_;
    resp.prot_ = objMeta.prot_;
    resp.priority_ = objMeta.priority_;
    resp.blobs_ = objMeta.blobs_;
    return MMC_OK;
}

Result MmcMetaMgrProxy::BatchAlloc(const BatchAllocRequest &req, BatchAllocResponse &resp)
{
    MmcMetaMetricManager &metricManager = MmcMetaMetricManager::GetInstance();
    metricManager.IncrementRequestCounter(RestMetricType::BATCH_ALLOC);
    resp.results_.resize(req.keys_.size());
    resp.blobs_.resize(req.keys_.size());
    if (req.keys_.size() != req.options_.size()) {
        metricManager.IncrementFailureCounter(RestMetricType::BATCH_ALLOC);
        MMC_LOG_ERROR("BatchAlloc input size mismatch, key count: " << req.keys_.size()
                                                                    << ", option count: " << req.options_.size());
        return MMC_ERROR;
    }
    for (size_t i = 0; i < req.keys_.size(); ++i) {
        MmcMemMetaDesc objMeta{};
        metaMangerPtr_->CheckAndEvict(static_cast<MediaType>(req.options_[i].mediaType_),
                                      req.options_[i].blobSize_ * req.options_[i].numBlobs_);
        TP_TRACE_BEGIN(TP_MMC_META_MGR_ALLOC);
        metricManager.IncrementRequestCounter(RestMetricType::ALLOC);
        Result ret = metaMangerPtr_->Alloc(req.keys_[i], req.options_[i], req.operateId_, objMeta);
        TP_TRACE_END(TP_MMC_META_MGR_ALLOC, ret);
        IncrementResultCounter(metricManager, RestMetricType::ALLOC, ret);
        if (ret != MMC_OK) {
            if (ret != MMC_DUPLICATED_OBJECT) {
                MMC_LOG_ERROR("Allocation failed for key: " << req.keys_[i] << ", error: " << ret);
            }
            resp.numBlobs_.push_back(0);
            resp.prots_.push_back(0);
            resp.priorities_.push_back(0);
            resp.leases_.push_back(0);
            resp.blobs_[i] = {};
        } else {
            resp.numBlobs_.push_back(objMeta.numBlobs_);
            resp.prots_.push_back(objMeta.prot_);
            resp.priorities_.push_back(objMeta.priority_);
            resp.leases_.push_back(0);
            resp.blobs_[i] = objMeta.blobs_;
        }
        resp.results_[i] = ret;
    }
    IncrementBatchResultCounter(metricManager, RestMetricType::BATCH_ALLOC, resp.results_);
    return MMC_OK;
}

Result MmcMetaMgrProxy::UpdateState(const UpdateRequest &req, Response &resp)
{
    MmcMetaMetricManager &metricManager = MmcMetaMetricManager::GetInstance();
    metricManager.IncrementRequestCounter(RestMetricType::UPDATE_STATE);
    MmcLocation loc{req.rank_, static_cast<MediaType>(req.mediaType_)};
    Result ret = metaMangerPtr_->UpdateState(req.key_, loc, req.actionResult_, req.operateId_);
    resp.ret_ = ret;
    IncrementResultCounter(metricManager, RestMetricType::UPDATE_STATE, ret);
    return MMC_OK;
}

Result MmcMetaMgrProxy::BatchUpdateState(const BatchUpdateRequest &req, BatchUpdateResponse &resp)
{
    MmcMetaMetricManager &metricManager = MmcMetaMetricManager::GetInstance();
    metricManager.IncrementRequestCounter(RestMetricType::BATCH_UPDATE_STATE);
    const size_t keyCount = req.keys_.size();
    if (keyCount != req.ranks_.size() || keyCount != req.mediaTypes_.size() || keyCount != req.actionResults_.size()) {
        metricManager.IncrementFailureCounter(RestMetricType::BATCH_UPDATE_STATE);
        MMC_LOG_ERROR("BatchUpdateState: Input vectors size mismatch {keyNum:"
                      << req.keys_.size() << ", rankNum:" << req.ranks_.size()
                      << ", mediaNum:" << req.mediaTypes_.size() << "}");
        return MMC_ERROR;
    }

    for (size_t i = 0; i < keyCount; ++i) {
        MmcLocation loc{req.ranks_[i], static_cast<MediaType>(req.mediaTypes_[i])};
        metricManager.IncrementRequestCounter(RestMetricType::UPDATE_STATE);
        Result ret = metaMangerPtr_->UpdateState(req.keys_[i], loc, req.actionResults_[i], req.operateId_);
        IncrementResultCounter(metricManager, RestMetricType::UPDATE_STATE, ret);
        if (ret != MMC_OK) {
            MMC_LOG_ERROR("update for key: " << req.keys_[i] << " failed, error: " << ret);
        }
        resp.results_.push_back(ret);
    }
    IncrementBatchResultCounter(metricManager, RestMetricType::BATCH_UPDATE_STATE, resp.results_);
    return MMC_OK;
}

Result MmcMetaMgrProxy::Get(const GetRequest &req, AllocResponse &resp)
{
    MmcMetaMetricManager &metricManager = MmcMetaMetricManager::GetInstance();
    metricManager.IncrementRequestCounter(RestMetricType::GET);
    MmcMemMetaDesc objMeta;
    MmcBlobFilterPtr filterPtr = MmcMakeRef<MmcBlobFilter>(UINT32_MAX, MEDIA_NONE, READABLE);
    Result ret = metaMangerPtr_->Get(req.key_, req.operateId_, filterPtr, objMeta);
    if (ret != MMC_OK) {
        IncrementResultCounter(metricManager, RestMetricType::GET, ret);
        MMC_LOG_ERROR("failed to get objMeta for key " << req.key_ << ", ret=" << ret);
        return ret;
    }
    if (objMeta.numBlobs_ == 0 || objMeta.blobs_.empty()) {
        metricManager.IncrementFailureCounter(RestMetricType::GET);
        MMC_LOG_ERROR("key " << req.key_ << " already released ");
        return MMC_ERROR;
    }

    metricManager.IncrementSuccessCounter(RestMetricType::GET);
    resp.blobs_ = objMeta.blobs_;
    resp.numBlobs_ = objMeta.blobs_.size();
    resp.prot_ = objMeta.prot_;
    resp.priority_ = objMeta.priority_;
    return MMC_OK;
}

Result MmcMetaMgrProxy::GetAllKeys(std::vector<std::string> &keys)
{
    MmcMetaMetricManager &metricManager = MmcMetaMetricManager::GetInstance();
    metricManager.IncrementRequestCounter(RestMetricType::GET_ALL_KEYS);
    Result ret = metaMangerPtr_->GetAllKeys(keys);
    IncrementResultCounter(metricManager, RestMetricType::GET_ALL_KEYS, ret);
    return ret;
}

Result MmcMetaMgrProxy::GetAllSegmentInfo(nlohmann::json &result)
{
    result = metaMangerPtr_->GetAllSegmentInfo();
    if (!result.is_array()) {
        return MMC_ERROR;
    }

    for (size_t i = 0; i < result.size(); ++i) {
        if (!result.at(i).is_object()) {
            MMC_LOG_ERROR("Segment info item is not an object, index=" << i);
            return MMC_ERROR;
        }
    }
    return MMC_OK;
}

Result MmcMetaMgrProxy::QuerySegment(const std::string &segmentId, nlohmann::json &segment)
{
    nlohmann::json segmentInfo = metaMangerPtr_->GetAllSegmentInfo();
    if (!segmentInfo.is_array()) {
        return MMC_ERROR;
    }

    for (size_t i = 0; i < segmentInfo.size(); ++i) {
        const nlohmann::json &item = segmentInfo.at(i);
        if (!item.is_object()) {
            MMC_LOG_ERROR("Segment info item is not an object, index=" << i);
            return MMC_ERROR;
        }

        const uint32_t rank = item.value("rank", UINT32_MAX);
        const std::string medium = item.value("medium", std::string("unknown"));
        if (segmentId == BuildSegmentId(rank, medium)) {
            segment = item;
            return MMC_OK;
        }
    }

    return MMC_UNMATCHED_KEY;
}

Result MmcMetaMgrProxy::BatchGet(const BatchGetRequest &req, BatchAllocResponse &resp)
{
    MmcMetaMetricManager &metricManager = MmcMetaMetricManager::GetInstance();
    metricManager.IncrementRequestCounter(RestMetricType::BATCH_GET);
    resp.numBlobs_.resize(req.keys_.size(), 0);
    resp.prots_.resize(req.keys_.size(), 0);
    resp.priorities_.resize(req.keys_.size(), 0);
    resp.leases_.resize(req.keys_.size(), 0);
    resp.blobs_.resize(req.keys_.size());

    MmcBlobFilterPtr filterPtr = MmcMakeRef<MmcBlobFilter>(UINT32_MAX, MEDIA_NONE, READABLE);
    for (size_t i = 0; i < req.keys_.size(); ++i) {
        MmcMemMetaDesc objMeta{};
        TP_TRACE_BEGIN(TP_MMC_META_MGR_GET);
        metricManager.IncrementRequestCounter(RestMetricType::GET);
        auto ret = metaMangerPtr_->Get(req.keys_[i], req.operateId_, filterPtr, objMeta);
        TP_TRACE_END(TP_MMC_META_MGR_GET, ret);
        if (ret != MMC_OK || objMeta.blobs_.empty() || objMeta.numBlobs_ != objMeta.blobs_.size()) {
            resp.numBlobs_[i] = 0;
            resp.blobs_[i] = {};
            resp.prots_[i] = 0;
            resp.priorities_[i] = 0;
            MMC_LOG_ERROR("Key " << req.keys_[i] << " not found");
        } else {
            resp.numBlobs_[i] = objMeta.numBlobs_;
            resp.blobs_[i] = objMeta.blobs_;
            resp.prots_[i] = objMeta.prot_;
            resp.priorities_[i] = objMeta.priority_;
        }
        IncrementResultCounter(metricManager, RestMetricType::GET, ret);
        resp.results_.push_back(ret);
    }
    IncrementBatchResultCounter(metricManager, RestMetricType::BATCH_GET, resp.results_);
    return MMC_OK;
}

Result MmcMetaMgrProxy::BatchExistKey(const BatchIsExistRequest &req, BatchIsExistResponse &resp)
{
    MmcMetaMetricManager &metricManager = MmcMetaMetricManager::GetInstance();
    metricManager.IncrementRequestCounter(RestMetricType::BATCH_EXIST_KEY);
    resp.results_.reserve(req.keys_.size());
    for (size_t i = 0; i < req.keys_.size(); ++i) {
        metricManager.IncrementRequestCounter(RestMetricType::EXIST_KEY);
        auto ret = metaMangerPtr_->ExistKey(req.keys_[i]);
        if (ret != MMC_OK && ret != MMC_UNMATCHED_KEY) {
            MMC_LOG_ERROR("get key: " << req.keys_[i] << " unexpected result: " << ret);
        }
        resp.results_.emplace_back(ret);
        IncrementResultCounter(metricManager, RestMetricType::EXIST_KEY, ret);
    }
    IncrementBatchResultCounter(metricManager, RestMetricType::BATCH_EXIST_KEY, resp.results_);

    if (ubsIoEnable_) {
        std::vector<std::string> ubsIoKeys;
        std::vector<size_t> ubsIoIndices;
        ubsIoKeys.reserve(req.keys_.size());
        ubsIoIndices.reserve(req.keys_.size());
        for (size_t i = 0; i < req.keys_.size(); ++i) {
            if (resp.results_[i] == MMC_UNMATCHED_KEY) {
                ubsIoKeys.emplace_back(req.keys_[i]);
                ubsIoIndices.emplace_back(i);
            }
        }
        if (!ubsIoKeys.empty()) {
            bool *ubsIoResults = new bool[ubsIoKeys.size()];
            Result ret = ubsIoProxy_->BatchExist(ubsIoKeys, ubsIoResults);
            if (ret != 0) {
                MMC_LOG_ERROR("ubsIo batch exist failed, ret: " << ret);
                delete[] ubsIoResults;
                return MMC_ERROR;
            }
            for (size_t idx = 0; idx < ubsIoIndices.size(); ++idx) {
                const size_t resultIndex = ubsIoIndices[idx];
                resp.results_[resultIndex] = ubsIoResults[idx] ? MMC_OK : MMC_UNMATCHED_KEY;
            }
            delete[] ubsIoResults;
        }
    }
    return MMC_OK;
}

} // namespace mmc
} // namespace ock
