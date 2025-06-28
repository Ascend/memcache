/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2026. All rights reserved.
 */
#ifndef MEMFABRIC_MMC_MSG_BASE_H
#define MEMFABRIC_MMC_MSG_BASE_H

#include "mmc_common_includes.h"

namespace ock {
namespace mmc {
struct MsgBase {
    int16_t msgVer = 0;
    int16_t msgId = -1;
    uint32_t destRankId = 0;
    MsgBase(){}
    MsgBase(int16_t ver, int16_t op, uint32_t dst): msgVer(ver), msgId(op), destRankId(dst){}
};

enum MsgId : int16_t {
    PING_MSG = 0,
};

enum META_LOCAL_OPCODE_REQ : int16_t {
    ML_PING_REQ = 0,
    ML_ALLOC_REQ = 1,
};
}
}

#endif  //MEMFABRIC_MMC_MSG_BASE_H
