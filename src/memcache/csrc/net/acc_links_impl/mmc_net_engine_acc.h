/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 * MemCache_Hybrid is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
*/
#ifndef MEM_FABRIC_MMC_NET_ENGINE_ACC_LINKS_H
#define MEM_FABRIC_MMC_NET_ENGINE_ACC_LINKS_H

#include "mmc_net_engine.h"
#include "mmc_net_common_acc.h"
#include "mmc_thread_pool.h"
#include "mmc_net_ctx_store.h"

namespace ock {
namespace mmc {
class NetEngineAcc final : public NetEngine {
public:
    ~NetEngineAcc() override;

    Result Start(const NetEngineOptions &options) override;
    void Stop() override;

    Result ConnectToPeer(uint32_t peerId, const std::string &peerIp, uint16_t port, NetLinkPtr &newLink,
                         bool isForce) override;

    Result Call(uint32_t targetId, int16_t opCode, const char *reqData, uint32_t reqDataLen, char **respData,
                uint32_t &respDataLen, int32_t timeoutInSecond) override;

    Result Send(uint32_t peerId, const char *reqData, uint32_t reqDataLen, int32_t timeoutInSecond) override;

private:
    static Result VerifyOptions(const NetEngineOptions &options);
    Result Initialize(const NetEngineOptions &options);
    void UnInitialize();
    Result StartInner();
    Result StopInner();

    /* callback function of tcp server */
    Result RegisterTcpServerHandler();
    Result HandleNewLink(const TcpConnReq &req, const TcpLinkPtr &link) const;
    Result HandleNeqRequest(const TcpReqContext &context);
    Result HandleMsgSent(TcpMsgSentResult result, const TcpMsgHeader &header, const TcpDataBufPtr &cbCtx);
    Result HandleLinkBroken(const TcpLinkPtr &link) const;
    Result HandleAllRequests4Response(const TcpReqContext &context);

    Result RegisterDecryptHandler(const std::string &decryptLibPath) const;

private:
    /* hot used variables */
    TcpServerPtr server_;
    NetLinkMapAccPtr peerLinkMap_;
    NetContextStorePtr ctxStore_;

    /* not hot used variables */
    NetEngineOptions options_{};
    bool started_ = false;
    bool inited_ = false;
    std::mutex connectMutex_;
    int32_t timeoutInSecond_ = 0;
    MmcThreadPoolPtr threadPool_;
};
} // namespace mmc
} // namespace ock

#endif // MEM_FABRIC_MMC_NET_ENGINE_ACC_LINKS_H