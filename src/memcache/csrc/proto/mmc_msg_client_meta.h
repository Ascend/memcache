/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2026. All rights reserved.
 */

#ifndef MF_HYBRID_MMC_MSG_CLIENT_META_H
#define MF_HYBRID_MMC_MSG_CLIENT_META_H
#include "mmc_msg_base.h"
namespace ock {
namespace mmc {
struct PingMsg {
    MsgBase head;
    uint64_t num;
};
}
}
#endif //MF_HYBRID_MMC_MSG_CLIENT_META_H
