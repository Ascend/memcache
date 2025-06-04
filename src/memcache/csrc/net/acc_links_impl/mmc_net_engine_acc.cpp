/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2026. All rights reserved.
 */
#include "mmc_net_engine_acc.h"

namespace ock {
namespace mmc {
Result NetEngineAcc::Start(const NetEngineOptions &options)
{
    return MMC_OK;
}

Result NetEngineAcc::Stop()
{
    return MMC_OK;
}

Result NetEngineAcc::ConnectToPeer(const uint16_t &peerId, const std::string &peerIp, uint16_t port,
                                   NetLinkPtr &newLink, bool isForce)
{
    return MMC_OK;
}

Result NetEngineAcc::Call(const uint16_t &targetId, int32_t timeoutInSecond)
{
    return MMC_OK;
}

Result NetEngineAcc::RegRequestReceivedHandler(uint16_t opCode, const NetReqReceivedHandler &h)
{
    return MMC_OK;
}
Result NetEngineAcc::RegRequestSentHandler(uint16_t opCode, const NetReqSentHandler &h)
{
    return MMC_OK;
}
Result NetEngineAcc::RegNewLinkHandler(uint16_t opCode, const NetNewLinkHandler &h)
{
    return MMC_OK;
}
Result NetEngineAcc::RegLinkBrokenHandler(uint16_t opCode, const NetLinkBrokenHandler &h)
{
    return MMC_OK;
}
}
}
