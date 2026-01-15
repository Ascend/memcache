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

#ifndef SMEM_MMC_META_NET_SERVER_H
#define SMEM_MMC_META_NET_SERVER_H
#include "mmc_meta_common.h"
#include "mmc_net_engine.h"

namespace ock {
namespace mmc {
class MetaNetServer : public MmcReferable {
public:
    explicit MetaNetServer(const MmcMetaServicePtr &metaService, const std::string inputName = "");

    ~MetaNetServer() override;

    Result Start(NetEngineOptions &options);

    void Stop();

    template<typename REQ, typename RESP>
    Result SyncCall(uint32_t rankId, const REQ &req, RESP &resp, int32_t timeoutInSecond)
    {
        return engine_->Call(rankId, req.msgId, req, resp, timeoutInSecond);
    }

private:
    Result HandleNewLink(const NetLinkPtr &link);

    Result HandleLinkBroken(const NetLinkPtr &link);
    /* message handle function */
    Result HandleAlloc(const NetContextPtr &context);

    Result HandleBatchAlloc(const NetContextPtr &context);

    Result HandleBmRegister(const NetContextPtr &context);

    Result HandleBmUnregister(const NetContextPtr &context);

    Result HandlePing(const NetContextPtr &context);

    Result HandleUpdate(const NetContextPtr &context);

    Result HandleBatchUpdate(const NetContextPtr &context);

    Result HandleGet(const NetContextPtr &context);

    Result HandleBatchGet(const NetContextPtr &context);

    Result HandleRemove(const NetContextPtr &context);

    Result HandleBatchRemove(const NetContextPtr &context);

    Result HandleRemoveAll(const NetContextPtr &context);

    Result HandleIsExist(const NetContextPtr &context);

    Result HandleBatchIsExist(const NetContextPtr &context);

    Result HandleQuery(const NetContextPtr &context);

    Result HandleBatchQuery(const NetContextPtr &context);

private:
    NetEnginePtr engine_;
    MmcMetaServicePtr metaService_;

    /* not hot used variables */
    std::mutex mutex_;
    bool started_ = false;
    std::string name_;
};
using MetaNetServerPtr = MmcRef<MetaNetServer>;
} // namespace mmc
} // namespace ock
#endif // SMEM_MMC_META_NET_SERVER_H