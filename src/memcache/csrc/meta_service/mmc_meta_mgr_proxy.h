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

#include "mmc_meta_manager.h"
#include "mmc_msg_client_meta.h"
#include "mmc_meta_net_server.h"

namespace ock {
namespace mmc {

class MmcMetaMgrProxy : public MmcReferable {
public:
    explicit MmcMetaMgrProxy(const MetaNetServerPtr &netServerPtr) : netServerPtr_(netServerPtr) {}

    ~MmcMetaMgrProxy() override = default;

    Result Start(uint64_t defaultTtl, uint16_t evictThresholdHigh, uint16_t evictThresholdLow)
    {
        std::lock_guard<std::mutex> guard(mutex_);
        if (started_) {
            MMC_LOG_INFO("MmcMetaMgrProxyDefault already started");
            return MMC_OK;
        }
        metaMangerPtr_ = MmcMakeRef<MmcMetaManager>(defaultTtl, evictThresholdHigh, evictThresholdLow);
        if (metaMangerPtr_ == nullptr) {
            MMC_LOG_ERROR("new object failed, probably out of memory");
            return MMC_NEW_OBJECT_FAILED;
        }
        metaMangerPtr_->SetMetaNetServer(netServerPtr_);
        MMC_RETURN_ERROR(metaMangerPtr_->Start(), "MmcMetaManager start failed");
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

    Result Remove(const RemoveRequest &req, Response &resp)
    {
        return resp.ret_ = metaMangerPtr_->Remove(req.key_);
    }

    Result BatchRemove(const BatchRemoveRequest &req, BatchRemoveResponse &resp)
    {
        resp.results_.reserve(req.keys_.size());
        for (const std::string &key : req.keys_) {
            resp.results_.emplace_back(metaMangerPtr_->Remove(key));
        }
        return MMC_OK;
    }

    Result RemoveAll(const RemoveAllRequest &req, Response &resp)
    {
        return resp.ret_ = metaMangerPtr_->RemoveAll();
    }

    Result Mount(const std::vector<MmcLocation> &loc, const std::vector<MmcLocalMemlInitInfo> &localMemInitInfo,
                 std::map<std::string, MmcMemBlobDesc> &blobMap)
    {
        return metaMangerPtr_->Mount(loc, localMemInitInfo, blobMap);
    }

    Result Unmount(const MmcLocation &loc)
    {
        return metaMangerPtr_->Unmount(loc);
    }

    Result ExistKey(const IsExistRequest &req, IsExistResponse &resp)
    {
        return resp.ret_ = metaMangerPtr_->ExistKey(req.key_);
    }

    Result BatchExistKey(const BatchIsExistRequest &req, BatchIsExistResponse &resp);

    Result Query(const QueryRequest &req, QueryResponse &resp)
    {
        return metaMangerPtr_->Query(req.key_, resp.queryInfo_);
    }

    Result BatchQuery(const BatchQueryRequest &req, BatchQueryResponse &resp)
    {
        for (const std::string &key : req.keys_) {
            MemObjQueryInfo queryInfo;
            metaMangerPtr_->Query(key, queryInfo);
            resp.batchQueryInfos_.push_back(queryInfo);
        }
        return MMC_OK;
    }

    const MmcMetaManagerPtr &GetMetaManager()
    {
        return metaMangerPtr_;
    }

private:
    std::mutex mutex_;
    bool started_ = false;
    MmcMetaManagerPtr metaMangerPtr_;
    MetaNetServerPtr netServerPtr_;
    const int32_t timeOut_ = 60;
};
using MmcMetaMgrProxyPtr = MmcRef<MmcMetaMgrProxy>;

} // namespace mmc
} // namespace ock

#endif // MEM_FABRIC_MMC_META_PROXY_IMPL_H