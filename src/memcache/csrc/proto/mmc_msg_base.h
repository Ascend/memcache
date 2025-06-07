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
};

enum MsgId : int16_t {};
}
}

#endif  //MEMFABRIC_MMC_MSG_BASE_H
