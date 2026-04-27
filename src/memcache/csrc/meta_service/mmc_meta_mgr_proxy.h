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
#ifndef MEM_FABRIC_MMC_META_PROXY_IMPL_H
#define MEM_FABRIC_MMC_META_PROXY_IMPL_H

#include <glob.h>

#include <string>
#include <vector>

#include "nlohmann/json.hpp"

#include "mmc_meta_manager.h"
#include "mmc_meta_metric_manager.h"
#include "mmc_msg_client_meta.h"
#include "mmc_meta_net_server.h"
#include "mmc_ubs_io_proxy.h"

namespace ock {
namespace mmc {

class MmcMetaMgrProxy : public MmcReferable {
public:
    explicit MmcMetaMgrProxy(const MetaNetServerPtr &netServerPtr) : netServerPtr_(netServerPtr) {}

    ~MmcMetaMgrProxy() override = default;

    Result Start(uint64_t defaultTtl, uint16_t evictThresholdHigh, uint16_t evictThresholdLow, bool ubsIoEnable)
    {
        std::lock_guard<std::mutex> guard(mutex_);
        if (started_) {
            MMC_LOG_INFO("MmcMetaMgrProxyDefault already started");
            return MMC_OK;
        }
        metaMangerPtr_ = MmcMakeRef<MmcMetaManager>(defaultTtl, evictThresholdHigh, evictThresholdLow, ubsIoEnable);
        if (metaMangerPtr_ == nullptr) {
            MMC_LOG_ERROR("new object failed, probably out of memory");
            return MMC_NEW_OBJECT_FAILED;
        }
        if (ubsIoEnable) {
            MmcUbsIoProxyPtr ubsIoProxy = MmcUbsIoProxyFactory::GetInstance("ubsIoProxyDefault");
            MMC_ASSERT_RETURN(ubsIoProxy != nullptr, MMC_MALLOC_FAILED);
            ubsIoProxy_ = ubsIoProxy;
        }
        metaMangerPtr_->SetMetaNetServer(netServerPtr_);
        MMC_RETURN_ERROR(metaMangerPtr_->Start(), "MmcMetaManager start failed");
        ubsIoEnable_ = ubsIoEnable;
        started_ = true;
        return MMC_OK;
    }

    void Stop()
    {
        std::lock_guard<std::mutex> guard(mutex_);
        if (!started_) {
            MMC_LOG_WARN("MmcMetaMgrProxyDefault has not been started");
            return;
        }
        metaMangerPtr_->Stop();
        started_ = false;
    }

    Result Alloc(const AllocRequest &req, AllocResponse &resp);

    Result BatchAlloc(const BatchAllocRequest &req, BatchAllocResponse &resp);

    Result UpdateState(const UpdateRequest &req, Response &resp);

    Result BatchUpdateState(const BatchUpdateRequest &req, BatchUpdateResponse &resp);

    Result Get(const GetRequest &req, AllocResponse &resp);

    Result BatchGet(const BatchGetRequest &req, BatchAllocResponse &resp);

    Result GetAllKeys(std::vector<std::string> &keys);

    Result GetAllSegmentInfo(nlohmann::json &result);

    Result QuerySegment(const std::string &segmentId, nlohmann::json &segment);

    Result Remove(const RemoveRequest &req, Response &resp)
    {
        MmcMetaMetricManager &metricManager = MmcMetaMetricManager::GetInstance();
        metricManager.IncrementRequestCounter(RestMetricType::REMOVE);
        resp.ret_ = metaMangerPtr_->Remove(req.key_);
        IncrementResultCounter(metricManager, RestMetricType::REMOVE, resp.ret_);
        if (ubsIoEnable_) {
            Result ubsIoRet = ubsIoProxy_->Delete(req.key_);
            if (resp.ret_ != MMC_OK) {
                resp.ret_ = ubsIoRet;
            }
        }
        return resp.ret_;
    }

    Result BatchRemove(const BatchRemoveRequest &req, BatchRemoveResponse &resp)
    {
        MmcMetaMetricManager &metricManager = MmcMetaMetricManager::GetInstance();
        metricManager.IncrementRequestCounter(RestMetricType::BATCH_REMOVE);
        resp.results_.reserve(req.keys_.size());
        for (const std::string &key : req.keys_) {
            metricManager.IncrementRequestCounter(RestMetricType::REMOVE);
            Result metaRet = metaMangerPtr_->Remove(key);
            resp.results_.emplace_back(metaRet);
            IncrementResultCounter(metricManager, RestMetricType::REMOVE, metaRet);
        }
        IncrementBatchResultCounter(metricManager, RestMetricType::BATCH_REMOVE, resp.results_);
        if (ubsIoEnable_) {
            std::vector<int> ubsIoResults(req.keys_.size(), MMC_OK);
            Result ret = ubsIoProxy_->BatchDelete(req.keys_, ubsIoResults);
            if (ret != MMC_OK) {
                MMC_LOG_ERROR("ubsIo batch delete failed, ret: " << ret);
                return MMC_ERROR;
            }
            for (size_t i = 0; i < req.keys_.size(); ++i) {
                if (resp.results_[i] != MMC_OK) {
                    resp.results_[i] = ubsIoResults[i];
                }
            }
        }
        return MMC_OK;
    }

    Result RemoveAll(const RemoveAllRequest &req, Response &resp)
    {
        (void)req;
        MmcMetaMetricManager &metricManager = MmcMetaMetricManager::GetInstance();
        metricManager.IncrementRequestCounter(RestMetricType::REMOVE_ALL);
        resp.ret_ = metaMangerPtr_->RemoveAll();
        IncrementResultCounter(metricManager, RestMetricType::REMOVE_ALL, resp.ret_);
        return resp.ret_;
    }

    Result Mount(const std::vector<MmcLocation> &loc, const std::vector<MmcLocalMemlInitInfo> &localMemInitInfo,
                 std::map<std::string, MmcMemBlobDesc> &blobMap)
    {
        MmcMetaMetricManager &metricManager = MmcMetaMetricManager::GetInstance();
        metricManager.IncrementRequestCounter(RestMetricType::MOUNT);
        Result ret = metaMangerPtr_->Mount(loc, localMemInitInfo, blobMap);
        IncrementResultCounter(metricManager, RestMetricType::MOUNT, ret);
        return ret;
    }

    Result Unmount(const MmcLocation &loc)
    {
        MmcMetaMetricManager &metricManager = MmcMetaMetricManager::GetInstance();
        metricManager.IncrementRequestCounter(RestMetricType::UNMOUNT);
        Result ret = metaMangerPtr_->Unmount(loc);
        IncrementResultCounter(metricManager, RestMetricType::UNMOUNT, ret);
        return ret;
    }

    Result ExistKey(const IsExistRequest &req, IsExistResponse &resp)
    {
        MmcMetaMetricManager &metricManager = MmcMetaMetricManager::GetInstance();
        metricManager.IncrementRequestCounter(RestMetricType::EXIST_KEY);
        resp.ret_ = metaMangerPtr_->ExistKey(req.key_);
        IncrementResultCounter(metricManager, RestMetricType::EXIST_KEY, resp.ret_);
        if (ubsIoEnable_ && resp.ret_ == MMC_UNMATCHED_KEY) {
            resp.ret_ = ubsIoProxy_->Exist(req.key_) == 1 ? MMC_OK : MMC_UNMATCHED_KEY;
        }
        return resp.ret_;
    }

    Result BatchExistKey(const BatchIsExistRequest &req, BatchIsExistResponse &resp);

    Result Query(const QueryRequest &req, QueryResponse &resp)
    {
        constexpr uint32_t ssdQueryBlobCount = 1;

        MmcMetaMetricManager &metricManager = MmcMetaMetricManager::GetInstance();
        metricManager.IncrementRequestCounter(RestMetricType::QUERY);
        Result metaRet = metaMangerPtr_->Query(req.key_, resp.queryInfo_);
        IncrementResultCounter(metricManager, RestMetricType::QUERY, metaRet);
        if (metaRet != MMC_OK && ubsIoEnable_) {
            size_t length = 0;
            Result ubsIoRet = ubsIoProxy_->GetLength(req.key_, length);
            if (ubsIoRet == MMC_OK) {
                resp.queryInfo_.valid_ = true;
                resp.queryInfo_.size_ = length;
                resp.queryInfo_.numBlobs_ = ssdQueryBlobCount;
                resp.queryInfo_.blobRanks_[0] = UINT32_MAX;
                resp.queryInfo_.blobTypes_[0] = MEDIA_SSD;
                resp.queryInfo_.prot_ = 0;
                return MMC_OK;
            }
            MMC_LOG_WARN("ubsIo get length failed, ret: " << ubsIoRet);
            return MMC_UNMATCHED_KEY;
        }
        return metaRet;
    }

    Result BatchQuery(const BatchQueryRequest &req, BatchQueryResponse &resp)
    {
        constexpr uint32_t ssdQueryBlobCount = 1;

        MmcMetaMetricManager &metricManager = MmcMetaMetricManager::GetInstance();
        metricManager.IncrementRequestCounter(RestMetricType::BATCH_QUERY);
        std::vector<Result> results;
        results.reserve(req.keys_.size());
        for (const std::string &key : req.keys_) {
            MemObjQueryInfo queryInfo;
            metricManager.IncrementRequestCounter(RestMetricType::QUERY);
            Result metaRet = metaMangerPtr_->Query(key, queryInfo);
            results.push_back(metaRet);
            resp.batchQueryInfos_.push_back(queryInfo);
            IncrementResultCounter(metricManager, RestMetricType::QUERY, metaRet);
        }
        IncrementBatchResultCounter(metricManager, RestMetricType::BATCH_QUERY, results);
        if (ubsIoEnable_) {
            std::vector<std::string> ubsIoKeys;
            std::vector<size_t> ubsIoIndices;
            ubsIoKeys.reserve(req.keys_.size());
            ubsIoIndices.reserve(req.keys_.size());
            for (size_t i = 0; i < req.keys_.size(); ++i) {
                if (!resp.batchQueryInfos_[i].valid_) {
                    ubsIoKeys.emplace_back(req.keys_[i]);
                    ubsIoIndices.emplace_back(i);
                }
            }
            if (!ubsIoKeys.empty()) {
                std::vector<size_t> ubsIoLengths(ubsIoKeys.size(), 0);
                std::vector<int> ubsIoResults(ubsIoKeys.size(), MMC_OK);
                Result ret = ubsIoProxy_->BatchGetLength(ubsIoKeys, ubsIoLengths, ubsIoResults);
                if (ret != MMC_OK) {
                    MMC_LOG_ERROR("ubsIo batch get length failed, ret: " << ret);
                    return MMC_ERROR;
                }
                for (size_t idx = 0; idx < ubsIoIndices.size(); ++idx) {
                    const size_t resultIndex = ubsIoIndices[idx];
                    if (ubsIoResults[idx] == MMC_OK) {
                        resp.batchQueryInfos_[resultIndex].valid_ = true;
                        resp.batchQueryInfos_[resultIndex].size_ = ubsIoLengths[idx];
                        resp.batchQueryInfos_[resultIndex].numBlobs_ = ssdQueryBlobCount;
                        resp.batchQueryInfos_[resultIndex].blobRanks_[0] = UINT32_MAX;
                        resp.batchQueryInfos_[resultIndex].blobTypes_[0] = MEDIA_SSD;
                        resp.batchQueryInfos_[resultIndex].prot_ = 0;
                    }
                }
            }
        }
        return MMC_OK;
    }

    const MmcMetaManagerPtr &GetMetaManager()
    {
        return metaMangerPtr_;
    }

private:
    // Increments exactly one terminal result counter for a single operation: MMC_OK -> success, MMC_UNMATCHED_KEY ->
    // not_found, other errors including MMC_DUPLICATED_OBJECT -> failure.
    static void IncrementResultCounter(MmcMetaMetricManager &metricManager, RestMetricType type, Result ret)
    {
        if (ret == MMC_OK) {
            metricManager.IncrementSuccessCounter(type);
            return;
        }
        if (ret == MMC_UNMATCHED_KEY) {
            metricManager.IncrementNotFoundCounter(type);
            return;
        }
        metricManager.IncrementFailureCounter(type);
    }

    static void IncrementBatchResultCounter(MmcMetaMetricManager &metricManager, RestMetricType type,
                                            const std::vector<Result> &results)
    {
        for (Result ret : results) {
            if (ret != MMC_OK && ret != MMC_UNMATCHED_KEY) {
                metricManager.IncrementFailureCounter(type);
                return;
            }
        }
        metricManager.IncrementSuccessCounter(type);
    }

    std::mutex mutex_;
    bool started_ = false;
    bool ubsIoEnable_ = false;
    MmcMetaManagerPtr metaMangerPtr_;
    MetaNetServerPtr netServerPtr_;
    MmcUbsIoProxyPtr ubsIoProxy_;
    const int32_t timeOut_ = 60;
};
using MmcMetaMgrProxyPtr = MmcRef<MmcMetaMgrProxy>;

} // namespace mmc
} // namespace ock

#endif // MEM_FABRIC_MMC_META_PROXY_IMPL_H
