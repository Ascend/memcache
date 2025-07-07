/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2026. All rights reserved.
 */
#include "mmc_client_default.h"
#include "mmc_msg_client_meta.h"
#include "mmc_mem_obj_meta.h"
#include "mmc_bm_proxy.h"

namespace ock {
namespace mmc {
Result MmcClientDefault::Start(const mmc_client_config_t& config)
{
    MMC_LOG_INFO("Starting client " << name_);
    std::lock_guard<std::mutex> guard(mutex_);
    if (started_) {
        MMC_LOG_INFO("MetaService " << name_ << " already started");
        return MMC_OK;
    }
    metaNetClient_ = MetaNetClientFactory::GetInstance(config.discoveryURL, "MetaClientCommon").Get();
    MMC_ASSERT_RETURN(metaNetClient_.Get() != nullptr, MMC_NEW_OBJECT_FAILED);
    if (!metaNetClient_->Status()) {
        MMC_LOG_ERROR_AND_RETURN_NOT_OK(metaNetClient_->Start(),
                                        "Failed to start net server of local service " << name_);
        MMC_LOG_ERROR_AND_RETURN_NOT_OK(metaNetClient_->Connect(config.discoveryURL),
                                        "Failed to connect net server of local service " << name_);
    }
    MMC_ASSERT_RETURN(metaNetClient_.Get() != nullptr, MMC_NOT_CONNET_META);
    randId_ = config.rankId;
    bmProxy_ = MmcBmProxyFactory::GetInstance("bmProxyDefault");
    started_ = true;
    return MMC_OK;
}

void MmcClientDefault::Stop()
{
    std::lock_guard<std::mutex> guard(mutex_);
    if (!started_) {
        MMC_LOG_WARN("MmcClientDefault has not been started");
        return;
    }
    metaNetClient_->Stop();
    started_ = false;

}

const std::string& MmcClientDefault::Name() const
{
    return name_;
}

Result MmcClientDefault::Put(const char *key, mmc_buffer *buf, mmc_put_options &options, uint32_t flags)
{
    // todo 参数校验
    uint64_t blobSize = buf->type == 0 ? buf->dram.len : buf->hbm.width * buf->hbm.layerNum;
    AllocRequest request{key, {blobSize, 1, options.mediaType, RankId(options.policy), flags}};
    AllocResponse response;
    int16_t respRet = MMC_ERROR;
    MMC_LOG_ERROR_AND_RETURN_NOT_OK(metaNetClient_->SyncCall(request, response, respRet, timeOut_),
                                    "client " << name_ << " alloc " << key << " failed");
    MMC_LOG_ERROR_AND_RETURN_NOT_OK(respRet, "client " << name_ << " alloc " << key << " failed");

    for (uint8_t i = 0; i < response.numBlobs_; i++) {
        MMC_LOG_ERROR_AND_RETURN_NOT_OK(bmProxy_->Put(buf, response.blobs_[i].gva_, blobSize),
                                        "client " << name_ << " put " << key << " failed");
    }
    UpdateRequest updateRequest{MMC_WRITE_OK, key, 0, 0};
    Response updateResponse;
    MMC_LOG_ERROR_AND_RETURN_NOT_OK(metaNetClient_->SyncCall(updateRequest, updateResponse, respRet, timeOut_),
                                    "client " << name_ << " update " << key << " failed");
    MMC_LOG_ERROR_AND_RETURN_NOT_OK(respRet, "client " << name_ << " update " << key << " failed");
    MMC_LOG_ERROR_AND_RETURN_NOT_OK(updateResponse.ret_, "client " << name_ << " update " << key << " failed");

    return MMC_OK;
}

Result MmcClientDefault::Get(const char *key, mmc_buffer *buf, uint32_t flags)
{
    GetRequest request{key};
    AllocResponse response;
    int16_t respRet = MMC_ERROR;
    MMC_LOG_ERROR_AND_RETURN_NOT_OK(metaNetClient_->SyncCall(request, response, respRet, timeOut_),
                                    "client " << name_ << " get " << key << " failed");
    MMC_LOG_ERROR_AND_RETURN_NOT_OK(respRet, "client " << name_ << " get " << key << " failed");
    if (response.numBlobs_ > 0) {
        MMC_LOG_ERROR_AND_RETURN_NOT_OK(bmProxy_->Get(buf, response.blobs_[0].gva_),
                                        "client " << name_ << " put " << key << " failed");
    } else {
        MMC_LOG_ERROR("client " << name_ << " get " << key << " failed");
        return MMC_ERROR;
    }
    return MMC_OK;
}

mmc_location_t MmcClientDefault::GetLocation(const char* key, uint32_t flags)
{
    GetRequest request{key};
    AllocResponse response;
    int16_t respRet = MMC_ERROR;
    MMC_ASSERT_RETURN(metaNetClient_->SyncCall(request, response, respRet, timeOut_) == MMC_OK, {-1});
    MMC_ASSERT_RETURN(respRet == MMC_OK, {-1});
    return mmc_location_t();
}

Result MmcClientDefault::Remove(const char* key, uint32_t flags)
{
    RemoveRequest request{key};
    Response response;
    int16_t respRet;
    MMC_LOG_ERROR_AND_RETURN_NOT_OK(metaNetClient_->SyncCall(request, response, respRet, timeOut_),
                                    "client " << name_ << " remove " << key << " failed");
    MMC_LOG_ERROR_AND_RETURN_NOT_OK(respRet, "client " << name_ << " remove " << key << " failed");
    return 0;
}

}
}