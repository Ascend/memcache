/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2026. All rights reserved.
 */
#include "mmc_client_default.h"
#include "mmc_msg_client_meta.h"
#include "mmc_mem_obj_meta.h"

namespace ock {
namespace mmc {
Result MmcClientDefault::Start(const mmc_client_config_t& config)
{
    metaNetClient_ = MetaNetClientFactory::GetInstance(config.discoveryURL, "MetaClientCommon").Get();
    MMC_ASSERT_RETURN(metaNetClient_.Get() != nullptr, MMC_NEW_OBJECT_FAILED);
    if (!metaNetClient_->Status()) {
        MMC_LOG_ERROR_AND_RETURN_NOT_OK(metaNetClient_->Start(),
                                        "Failed to start net server of local service " << name_);
        MMC_LOG_ERROR_AND_RETURN_NOT_OK(metaNetClient_->Connect(config.discoveryURL),
                                        "Failed to connect net server of local service " << name_);
    }
    MMC_ASSERT_RETURN(metaNetClient_.Get() != nullptr, MMC_NOT_CONNET_META);
    return MMC_OK;
}

void MmcClientDefault::Stop()
{
}

const std::string& MmcClientDefault::Name() const
{
    return name_;
}

Result MmcClientDefault::Put(const char *key, mmc_buffer *buf, uint32_t flags)
{
    uint32_t rank = 0;
    AllocRequest reqAlloc;
    int16_t respRet;
    reqAlloc.key_ = key;
    reqAlloc.prot_ = {buf->hbm.width * buf->hbm.layerCount, 1, static_cast<uint16_t>(buf->type), rank, 0};
    MmcMemObjMetaPtr objMeta = MmcMakeRef<MmcMemObjMeta>();
    metaNetClient_->SyncCall(reqAlloc, *objMeta.Get(), respRet, 30);
//    localClient_->
    //bm_copy()
    return 0;
}

Result MmcClientDefault::Get(const char *key, mmc_buffer *buf, uint32_t flags)
{
    return 0;
}
}
}