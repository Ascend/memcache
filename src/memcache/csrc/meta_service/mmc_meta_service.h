/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2026. All rights reserved.
 */
#ifndef MEM_FABRIC_MMC_META_SERVICE_H
#define MEM_FABRIC_MMC_META_SERVICE_H

#include "mmc_common_includes.h"

namespace ock {
namespace mmc {
class MmcMetaService : public MmcReferable {
public:
};
using MmcMetaServicePtr = MmcRef<MmcMetaService>;
}
}

#endif  //MEM_FABRIC_MMC_META_SERVICE_H