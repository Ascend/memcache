/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2026. All rights reserved.
 */
#ifndef MEM_FABRIC_MMC_NET_ENGINE_H
#define MEM_FABRIC_MMC_NET_ENGINE_H

#include <functional>

#include "mmc_net_common.h"

namespace ock {
namespace mmc {

using NetReqReceivedHandler = std::function<int32_t(NetContext &ctx)>;
using NetReqSentHandler = std::function<int32_t(NetContext &ctx)>;
using NetNewLinkHandler = std::function<int32_t(const NetLinkPtr &link)>;
using NetLinkBrokenHandler = std::function<int32_t(const NetLinkPtr &link)>;

class NetContext {};

class NetLink : public MmcReferable {
public:
};

class NetEngine : public MmcReferable {
public:
    static NetEnginePtr Create();

public:
    ~NetEngine() override = default;

    virtual Result Start(const NetEngineOptions &options) = 0;
    virtual Result Stop() = 0;

    virtual Result ConnectToPeer(const uint16_t &peerId, const std::string &peerIp, uint16_t port, NetLinkPtr &newLink,
                                 bool isForce) = 0;

    virtual Result Call(const uint16_t &targetId, int32_t timeoutInSecond) = 0;

    virtual Result RegRequestReceivedHandler(uint16_t opCode, const NetReqReceivedHandler &h) = 0;
    virtual Result RegRequestSentHandler(uint16_t opCode, const NetReqSentHandler &h) = 0;
    virtual Result RegNewLinkHandler(uint16_t opCode, const NetNewLinkHandler &h) = 0;
    virtual Result RegLinkBrokenHandler(uint16_t opCode, const NetLinkBrokenHandler &h) = 0;
};
}
}

#endif  //MEM_FABRIC_MMC_NET_ENGINE_H
