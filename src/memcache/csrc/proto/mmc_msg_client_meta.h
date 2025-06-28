/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2026. All rights reserved.
 */

#ifndef MF_HYBRID_MMC_MSG_CLIENT_META_H
#define MF_HYBRID_MMC_MSG_CLIENT_META_H
#include "mmc_msg_base.h"
#include "mmc_msg_packer.h"
#include "mmc_locality_strategy.h"
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

struct AllocRequest {
    MsgBase base_{0, ML_ALLOC_REQ, 0};
    std::string key_;
    AllocProperty prot_;
    Result Serialize(NetMsgPacker &packer) const
    {
        packer.Serialize(base_);
        packer.Serialize(key_);
        packer.Serialize(prot_);
        return MMC_OK;
    }
    Result Deserialize(NetMsgUnpacker &packer)
    {
        packer.Deserialize(base_);
        packer.Deserialize(key_);
        packer.Deserialize(prot_);
        return MMC_OK;
    }
};

}
}
#endif //MF_HYBRID_MMC_MSG_CLIENT_META_H
