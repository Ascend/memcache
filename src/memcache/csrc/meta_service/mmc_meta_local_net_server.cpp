/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2026. All rights reserved.
 */
#include "mmc_meta_service.h"
#include "mmc_msg_base.h"
#include "mmc_meta_local_net_server.h"
#include "mmc_msg_client_meta.h"

namespace ock {
namespace mmc {
MetaNetServer::MetaNetServer(const MmcMetaServicePtr &metaService, const std::string inputName)
    : metaService_(metaService), name_(inputName)
{}
MetaNetServer::~MetaNetServer() {}
Result ock::mmc::MetaNetServer::Start()
{
    std::lock_guard<std::mutex> guard(mutex_);
    if (started_) {
        MMC_LOG_INFO("MetaNetServer [" << name_ << "] already started");
        return MMC_OK;
    }

    MMC_ASSERT_RETURN(metaService_.Get() != nullptr, MMC_INVALID_PARAM);

    /* init engine */
    NetEngineOptions options;
    std::string url{ metaService_->Options().discoveryURL };
    NetEngineOptions::ExtractIpPortFromUrl(url, options);
    options.name = name_;
    options.threadCount = 2;
    options.rankId = 0;
    options.startListener = true;

    NetEnginePtr server = NetEngine::Create();
    server->RegRequestReceivedHandler(META_LOCAL_OPCODE_REQ::ML_ALLOC_REQ,
        std::bind(&MetaNetServer::HandleAlloc, this, std::placeholders::_1));
    server->RegRequestReceivedHandler(META_LOCAL_OPCODE_REQ::ML_PING_REQ,
                                      std::bind(&MetaNetServer::HandlePing, this, std::placeholders::_1));
    server->RegNewLinkHandler(std::bind(&MetaNetServer::HandleNewLink, this, std::placeholders::_1));

    /* start engine */
    MMC_ASSERT_RETURN(server->Start(options) == MMC_OK, MMC_NOT_STARTED);

    engine_ = server;
    started_ = true;
    MMC_LOG_INFO("initialize meta net server success [" << name_ << "]");
    return MMC_OK;
}

Result MetaNetServer::HandleAlloc(const NetContextPtr &context)
{
    return MMC_OK;
}

Result MetaNetServer::HandlePing(const NetContextPtr &context)
{
    std::string str{(char *)context->Data(), context->DataLen()};
    NetMsgUnpacker unpacker(str);
    PingMsg req;
    req.Deserialize(unpacker);
    MMC_LOG_INFO("HandlePing num " << req.num);

    NetMsgPacker packer;
    PingMsg recv;
    recv.Serialize(packer);
    std::string serializedData = packer.String();
    uint32_t retSize = serializedData.length();
    return context->Reply(0, const_cast<char* >(serializedData.c_str()), retSize);
}

Result MetaNetServer::HandleNewLink(const NetLinkPtr &link)
{
    MMC_LOG_INFO("new link " << name_);
    return MMC_OK;
}

void MetaNetServer::Stop()
{
    engine_->Stop();
}
}
}
