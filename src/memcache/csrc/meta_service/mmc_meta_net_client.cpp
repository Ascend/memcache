/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2026. All rights reserved.
 */
#include "mmc_local_service_default.h"
#include "mmc_meta_net_client.h"
#include "mmc_msg_base.h"

namespace ock {
namespace mmc {
std::map<std::string, MmcRef<MetaNetClient>> MetaNetClientFactory::instances_;
std::mutex MetaNetClientFactory::instanceMutex_;

MetaNetClient::~MetaNetClient() {}
MetaNetClient::MetaNetClient(const std::string &serverUrl, const std::string inputName)
    : serverUrl_(serverUrl), name_(inputName)
{}

Result MetaNetClient::Start()
{
    std::lock_guard<std::mutex> guard(mutex_);
    if (started_) {
        MMC_LOG_INFO("MetaNetServer [" << name_ << "] already started");
        return MMC_OK;
    }

    /* init engine */
    NetEngineOptions options;
    options.name = name_;
    options.threadCount = 2;
    options.rankId = 0;
    options.startListener = false;

    NetEnginePtr client = NetEngine::Create();
    client->RegRequestReceivedHandler(LOCAL_META_OPCODE_REQ::ML_PING_REQ, nullptr);
    client->RegRequestReceivedHandler(LOCAL_META_OPCODE_REQ::ML_ALLOC_REQ, nullptr);

    /* start engine */
    MMC_ASSERT_RETURN(client->Start(options) == MMC_OK, MMC_NOT_STARTED);

    engine_ = client;
    started_ = true;
    MMC_LOG_INFO("initialize meta net server success [" << name_ << "]");
    return MMC_OK;
}

void MetaNetClient::Stop() {}

Result MetaNetClient::Connect(const std::string &url)
{
    NetEngineOptions options;
    NetEngineOptions::ExtractIpPortFromUrl(url, options);
    MMC_LOG_ERROR_AND_RETURN_NOT_OK(engine_->ConnectToPeer(peerId_, options.ip, options.port, link2Index_, false),
        "MetaNetClient Connect " << url << " failed");
    return MMC_OK;
}
}
}