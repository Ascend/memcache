/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2026. All rights reserved.
 */
#ifndef MEM_FABRIC_MMC_CLIENT_DEFAULT_H
#define MEM_FABRIC_MMC_CLIENT_DEFAULT_H

#include "mmc_common_includes.h"

namespace ock {
namespace mmc {
class MmcClientDefault : public MmcReferable {};
using MmcClientDefaultPtr = MmcRef<MmcClientDefault>;
}
}

#endif  //MEM_FABRIC_MMC_CLIENT_DEFAULT_H
