/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2026. All rights reserved.
 */

#ifndef MF_HYBRID_MMC_MSG_CLIENT_META_H
#define MF_HYBRID_MMC_MSG_CLIENT_META_H
#include "mmc_msg_base.h"
#include "mmc_msg_packer.h"
namespace ock {
namespace mmc {
struct PingMsg {
    MsgBase head;
    uint64_t num;
    Result Serialize(NetMsgPacker &packer) const
    {
        packer.Serialize(num);
        return MMC_OK;
    }
    Result Deserialize(NetMsgUnpacker &packer)
    {
        packer.Deserialize(num);
        return MMC_OK;
    }
};
}
}
#endif //MF_HYBRID_MMC_MSG_CLIENT_META_H
