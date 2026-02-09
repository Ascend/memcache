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
#ifndef MEM_FABRIC_MMC_CLIENT_DEFAULT_H
#define MEM_FABRIC_MMC_CLIENT_DEFAULT_H

#include "mmc_common_includes.h"
#include "mmc_meta_net_client.h"
#include "mmc_def.h"
#include "mmc_bm_proxy.h"
#include "mmc_thread_pool.h"
#include "mmc_msg_client_meta.h"

namespace ock {
namespace mmc {
class MmcClientDefault : public MmcReferable {
public:
    ~MmcClientDefault() override = default;

    Result Start(const mmc_client_config_t &config);

    void Stop();

    const std::string &Name() const;

    Result Put(const char *key, mmc_buffer *buf, mmc_put_options &options, uint32_t flags);

    Result Get(const char *key, mmc_buffer *buf, uint32_t flags);

    Result Put(const std::string &key, const MmcBufferArray &bufArr, mmc_put_options &options, uint32_t flags);

    Result Get(const std::string &key, const MmcBufferArray &bufArr, uint32_t flags);

    Result BatchPut(const std::vector<std::string> &keys, const std::vector<mmc_buffer> &bufs, mmc_put_options &options,
                    uint32_t flags, std::vector<int> &batchResult);

    Result BatchGet(const std::vector<std::string> &keys, std::vector<mmc_buffer> &bufs, uint32_t flags,
                    std::vector<int> &batchResult);

    Result BatchPut(const std::vector<std::string> &keys, const std::vector<MmcBufferArray> &bufArrs,
                    mmc_put_options &options, uint32_t flags, std::vector<int> &batchResult);

    Result BatchGet(const std::vector<std::string> &keys, const std::vector<MmcBufferArray> &bufArrs, uint32_t flags,
                    std::vector<int> &batchResult);

    Result Remove(const char *key, uint32_t flags) const;

    Result BatchRemove(const std::vector<std::string> &keys, std::vector<Result> &remove_results, uint32_t flags) const;

    Result RemoveAll(uint32_t flags) const;

    Result IsExist(const std::string &key, uint32_t flags) const;

    Result BatchIsExist(const std::vector<std::string> &keys, std::vector<int32_t> &exist_results,
                        uint32_t flags) const;

    Result Query(const std::string &key, mmc_data_info &query_info, uint32_t flags) const;

    Result BatchQuery(const std::vector<std::string> &keys, std::vector<mmc_data_info> &query_infos,
                      uint32_t flags) const;

    Result RegisterBuffer(uint64_t addr, uint64_t size);

    Result UnRegisterBuffer(uint64_t addr, uint64_t size);

    uint32_t RankId() const
    {
        return rankId_;
    }

    static Result RegisterInstance()
    {
        std::lock_guard<std::mutex> lock(gClientHandlerMtx);
        if (gClientHandler != nullptr) {
            MMC_LOG_INFO("client handler already registered");
            return MMC_OK;
        }
        gClientHandler = new (std::nothrow) MmcClientDefault("mmc_client");
        if (gClientHandler == nullptr) {
            MMC_LOG_ERROR("alloc client handler failed");
            return MMC_ERROR;
        }
        return MMC_OK;
    }

    static Result UnregisterInstance()
    {
        std::lock_guard<std::mutex> lock(gClientHandlerMtx);
        if (gClientHandler == nullptr) {
            MMC_LOG_INFO("client handler already unregistered");
            return MMC_OK;
        }
        delete gClientHandler;
        gClientHandler = nullptr;
        return MMC_OK;
    }

    static MmcClientDefault *GetInstance()
    {
        std::lock_guard<std::mutex> lock(gClientHandlerMtx);
        return gClientHandler;
    }

private:
    explicit MmcClientDefault(const std::string &name) : name_(name) {}
    explicit MmcClientDefault(const MmcClientDefault &) = delete;
    MmcClientDefault &operator=(const MmcClientDefault &) = delete;

    inline uint32_t RankId(const affinity_policy &policy);
    Result PrepareAllocOpt(const MmcBufferArray &bufArr, const mmc_put_options &options, uint32_t flags,
                           AllocOptions &allocOpt);
    Result PrepareBlob(const MmcBufferArray &bufArr, const MmcMemBlobDesc &blob, MediaType &mediaType,
                       BatchCopyDesc &copyDesc, bool blobIsSrc);
    Result PrepareMultiBlobs(const MmcBufferArray &bufArr, const std::vector<MmcMemBlobDesc> &blobs,
                             MediaType &mediaType, BatchCopyDesc &copyDesc, bool blobIsSrc);
    Result PutData2Blobs(const std::vector<std::string> &keys, const std::vector<MmcBufferArray> &bufArrs,
                         const BatchAllocResponse &allocResponse, std::vector<int> &batchResult);
    void WaitFeatures(std::vector<std::tuple<uint32_t, uint32_t, std::future<int32_t>>> &futures,
                      std::vector<int> &batchResult);
    void SyncUpdateState(BatchUpdateRequest &updateRequest);
    void AsyncUpdateState(BatchUpdateRequest &updateRequest);
    std::future<int32_t> SubmitPutTask(BatchCopyDesc &copyDesc, MediaType mediaType, bool asyncExec);
    std::future<int32_t> SubmitGetTask(BatchCopyDesc &copyDesc, MediaType mediaType, bool asyncExec);

private:
    static std::mutex gClientHandlerMtx;
    static MmcClientDefault *gClientHandler;
    std::mutex mutex_;
    bool started_ = false;
    MetaNetClientPtr metaNetClient_;
    MmcBmProxyPtr bmProxy_;
    std::string name_;
    uint32_t rankId_{UINT32_MAX};
    uint32_t rpcRetryTimeOut_ = 0;
    uint64_t defaultTtlMs_ = MMC_DATA_TTL_MS;
    MmcThreadPoolPtr threadPool_;
    MmcThreadPoolPtr readThreadPool_;
    bool aggregateIO_{false};
    size_t aggregateNum_{0};
    MmcThreadPoolPtr writeThreadPool_;
};

uint32_t MmcClientDefault::RankId(const affinity_policy &policy)
{
    if (policy == NATIVE_AFFINITY) {
        return rankId_;
    } else {
        MMC_LOG_ERROR("affinity policy " << policy << " not supported");
        return 0;
    }
}
using MmcClientDefaultPtr = MmcRef<MmcClientDefault>;
} // namespace mmc
} // namespace ock

#endif // MEM_FABRIC_MMC_CLIENT_DEFAULT_H