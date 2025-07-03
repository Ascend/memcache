/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2026. All rights reserved.
 */

#ifndef SMEM_MMC_META_NET_CLIENT_H
#define SMEM_MMC_META_NET_CLIENT_H
#include "mmc_local_common.h"
#include "mmc_net_engine.h"
namespace ock {
namespace mmc {
class MetaNetClient : public MmcReferable {
public:
    explicit MetaNetClient(const std::string &serverUrl, const std::string inputName = "");

    ~MetaNetClient() override;

    Result Start();

    void Stop();

    Result Connect(const std::string &url);

    template <typename REQ, typename RESP>
    Result SyncCall(const REQ &req, RESP &resp, int16_t &userResult, int32_t timeoutInSecond)
    {
        return engine_->Call(peerId_, req, resp, userResult, timeoutInSecond);
    }

    inline bool Status();

private:
    NetEnginePtr engine_;
    std::string serverUrl_;
    NetLinkPtr link2Index_ = nullptr;
    const uint16_t peerId_ = 0;

    /* not hot used variables */
    std::mutex mutex_;
    bool started_ = false;
    std::string name_;
};

inline bool MetaNetClient::Status()
{
    std::lock_guard<std::mutex> ld(mutex_);
    return started_;
}

using MetaNetClientPtr = MmcRef<MetaNetClient>;

class MetaNetClientFactory: public MmcReferable {
public:
    static MmcRef<MetaNetClient> GetInstance(const std::string &serverUrl, const std::string inputName = "")
    {
        std::lock_guard<std::mutex> lock(instanceMutex_);
        std::string key = serverUrl + inputName;
        auto it = instances_.find(key);
        if (it == instances_.end()) {
            MmcRef<MetaNetClient> instance = new (std::nothrow)MetaNetClient(serverUrl, inputName);
            instances_[key] = instance;
            return instance;
        }
        return it->second;
    }

private:
    static std::map<std::string, MmcRef<MetaNetClient>> instances_;
    static std::mutex instanceMutex_;
};
}
}
#endif // SMEM_MMC_META_NET_CLIENT_H
