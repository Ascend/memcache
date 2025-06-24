/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2026. All rights reserved.
 */
#ifndef MEM_FABRIC_MMC_META_SERVICE_H
#define MEM_FABRIC_MMC_META_SERVICE_H

#include "mmc_common_includes.h"
#include "mmc_def.h"

namespace ock {
namespace mmc {
class MmcMetaService : public MmcReferable {
public:
    virtual Result Start(const mmc_meta_service_config_t &options) = 0;

    virtual void Stop() = 0;

    virtual const std::string &Name() const = 0;

    virtual const mmc_meta_service_config_t &Options() const = 0;
};
using MmcMetaServicePtr = MmcRef<MmcMetaService>;
}
}

#endif  //MEM_FABRIC_MMC_META_SERVICE_H