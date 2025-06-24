/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2026. All rights reserved.
 */

#ifndef SMEM_MMC_LOCAL_META_NET_CLIENT_H
#define SMEM_MMC_LOCAL_META_NET_CLIENT_H
#include "mmc_local_common.h"
#include "mmc_net_engine.h"
namespace ock {
namespace mmc {
class MetaNetClient : public MmcReferable {
public:
    explicit MetaNetClient(const MmcLocalServicePtr &localServiceDefaultPtr, const std::string inputName = "");

    ~MetaNetClient() override;

    Result Start();

    void Stop();

    Result Connect(uint16_t peerId, const std::string &url);

    template <typename REQ, typename RESP>
    Result SyncCall(uint32_t peerId, const REQ &req, RESP &resp, int16_t &userResult, int32_t timeoutInSecond)
    {
        return engine_->Call(peerId, req, resp, userResult, timeoutInSecond);
    }

private:
    NetEnginePtr engine_;
    MmcLocalServicePtr localService_;
    NetLinkPtr link2Index_ = nullptr;

    /* not hot used variables */
    std::mutex mutex_;
    bool started_ = false;
    std::string name_;
};
using MetaNetClientPtr = MmcRef<MetaNetClient>;
}
}
#endif // SMEM_MMC_LOCAL_META_NET_CLIENT_H
