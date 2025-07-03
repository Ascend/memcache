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
    metaNetClient_ = MetaNetClientFactory::GetInstance(this->options_.discoveryURL, "MetaClientCommon").Get();
    MMC_ASSERT_RETURN(metaNetClient_.Get() != nullptr, MMC_NEW_OBJECT_FAILED);
    if (!metaNetClient_->Status()) {
        MMC_LOG_ERROR_AND_RETURN_NOT_OK(metaNetClient_->Start(),
                                        "Failed to start net server of local service " << name_);
        MMC_LOG_ERROR_AND_RETURN_NOT_OK(metaNetClient_->Connect(config.discoveryURL),
                                        "Failed to connect net server of local service " << name_);
    }
    pid_ = getpid();

    start_ = true;
    MMC_LOG_INFO("Started LocalService (" << name_ << ") server " << options_.discoveryURL);
    return MMC_OK;
}
void MmcLocalServiceDefault::Stop()
{
    metaNetClient_->Stop();
    MMC_LOG_INFO("Stop LocalService (" << name_ << ") server " << options_.discoveryURL);
}
}
}