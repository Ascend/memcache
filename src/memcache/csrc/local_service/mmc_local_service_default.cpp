/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2026. All rights reserved.
 */
#include "mmc_local_service_default.h"
#include "mmc_meta_net_client.h"

namespace ock {
namespace mmc {
MmcLocalServiceDefault::~MmcLocalServiceDefault() {}
Result MmcLocalServiceDefault::Start(const mmc_local_service_config_t &config)
{
    MMC_LOG_INFO("Starting meta service " << name_);
    std::lock_guard<std::mutex> guard(mutex_);
    if (start_) {
        MMC_LOG_INFO("MetaService " << name_ << " already started");
        return MMC_OK;
    }
    options_ = config;
    metaNetClient_ = MmcMakeRef<MetaNetClient>(this, name_ + "_MetaClient").Get();
    MMC_ASSERT_RETURN(metaNetClient_.Get() != nullptr, MMC_NEW_OBJECT_FAILED);
    MMC_LOG_ERROR_AND_RETURN_NOT_OK(metaNetClient_->Start(), "Failed to start net server of local service " << name_);
    MMC_LOG_ERROR_AND_RETURN_NOT_OK(metaNetClient_->Connect(metaNetServerId_, config.metaServiceURL),
        "Failed to connect net server of local service " << name_);
    start_ = true;
    MMC_LOG_INFO("Started MetaService (" << name_ << ") at " << options_.discoveryURL);
    return MMC_OK;
}
void MmcLocalServiceDefault::Stop()
{
    metaNetClient_->Stop();
    MMC_LOG_INFO("Stop MetaService (" << name_ << ") at " << options_.discoveryURL);
}
}
}