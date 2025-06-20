/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2026. All rights reserved.
 */
#ifndef MMC_NET_CTX_ACC_H
#define MMC_NET_CTX_ACC_H

#include "mmc_net_engine.h"
#include "mmc_net_common_acc.h"

namespace ock {
namespace mmc {
class NetContextAcc : public NetContext {
public:
    explicit NetContextAcc(const TcpReqContext &ctx) : realContext(ctx)
    {

    }
    int32_t Reply(int16_t responseCode, char *respData, uint32_t &respDataLen)
    {
        /* step2: copy data */
        auto dataBuf = MmcMakeRef<ock::acc::AccDataBuffer>(respDataLen);
        MMC_ASSERT_RETURN(dataBuf.Get() != nullptr, MMC_NEW_OBJECT_FAILED);
        MMC_ASSERT_RETURN(dataBuf->AllocIfNeed(), MMC_NEW_OBJECT_FAILED);
        memcpy(dataBuf->DataPtrVoid(), static_cast<void *>(const_cast<char *>(respData)),
               respDataLen);
        dataBuf->SetDataSize(respDataLen);
        return realContext.Reply(responseCode, dataBuf.Get());
    }

    uint32_t SeqNo()
    {
        return realContext.Header().seqNo;
    }

    int16_t OpCode()
    {
        return realContext.Header().result;
    }

    int16_t SrcRankId()
    {
        return 0;
    }

    uint32_t DataLen()
    {
        return realContext.DataLen();
    }

    void *Data()
    {
        return realContext.DataPtr();
    }
private:
    const TcpReqContext &realContext;
};
using NetContextAccPtr = MmcRef<NetContextAcc>;
}
}

#endif //MMC_NET_CTX_ACC_H
