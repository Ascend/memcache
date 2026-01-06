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
#ifndef MEM_FABRIC_MMC_NET_COMMON_ACC_H
#define MEM_FABRIC_MMC_NET_COMMON_ACC_H

#include "acc_tcp_server.h"
#include "mmc_common_includes.h"
#include "mmc_net_link_map.h"

namespace ock {
namespace mmc {
using TcpServerOptions = ock::acc::AccTcpServerOptions;
using TcpServerPtr = ock::acc::AccTcpServerPtr;
using TcpServer = ock::acc::AccTcpServer;
using TcpLinkPtr = ock::acc::AccTcpLinkComplexPtr;
using TcpReqContext = ock::acc::AccTcpRequestContext;
using TcpMsgSentResult = ock::acc::AccMsgSentResult;
using TcpMsgHeader = ock::acc::AccMsgHeader;
using TcpDataBufPtr = ock::acc::AccDataBufferPtr;
using TcpConnReq = ock::acc::AccConnReq;
using TcpConnResp = ock::acc::AccConnResp;
using TcpNewReqHandler = ock::acc::AccNewReqHandler;
using TcpReqSentHandler = ock::acc::AccReqSentHandler;
using TcpLinkBrokenHandler = ock::acc::AccLinkBrokenHandler;
using TcpNewLinkHandler = ock::acc::AccNewLinkHandler;
using TcpTlsOption = ock::acc::AccTlsOption;

class NetLinkAcc;
class NetWaitHandler;
class NetContextStore;
class NetEngineAcc;

using NetWaitHandlerPtr = MmcRef<NetWaitHandler>;
using NetLinkAccPtr = MmcRef<NetLinkAcc>;
using NetContextStorePtr = MmcRef<NetContextStore>;
using NetEngineAccPtr = MmcRef<NetEngineAcc>;

using NetLinkMapAcc = NetLinkMap<NetLinkAccPtr>;
using NetLinkMapAccPtr = MmcRef<NetLinkMapAcc>;

constexpr int16_t MSG_TYPE_DATA = 0;
constexpr int16_t MSG_TYPE_CTRL = 1;
/**
 * @brief Seq number
 */
union NetSeqNo {
    struct {
        /* low address */
        uint32_t realSeq : 24; /* real seq no */
        uint32_t version : 6;  /* request version */
        uint32_t fromFlat : 1; /* allocated from flat or hash map */
        uint32_t isResp : 1;   /* request or reply, 0 for request, 1 for reply */
        /* high address */
    };
    uint32_t wholeSeq = 0;

    explicit NetSeqNo(uint32_t whole) : wholeSeq(whole) {}

    void SetValue(uint32_t flat, uint32_t ver, uint32_t seq);
    std::string ToString() const;
    bool IsResp() const;
};

/* inline function for NetSeqNo */
inline void NetSeqNo::SetValue(uint32_t flat, uint32_t ver, uint32_t seq)
{
    fromFlat = flat;
    version = ver;
    realSeq = seq;
}

inline std::string NetSeqNo::ToString() const
{
    std::ostringstream oss;
    oss << "NetSeqNo info=[wholeSeq: " << wholeSeq << ", isResp: " << isResp << ", fromFlat: " << fromFlat
        << ", version: " << version << ", realSeq: " << realSeq << "]";
    return oss.str();
}

inline bool NetSeqNo::IsResp() const
{
    return isResp == 1L;
}
} // namespace mmc
} // namespace ock

#endif // MEM_FABRIC_MMC_NET_COMMON_ACC_H