/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2026. All rights reserved.
 */
#ifndef MEM_FABRIC_MMC_LOCAL_SERVICE_H
#define MEM_FABRIC_MMC_LOCAL_SERVICE_H

#include "mmc_common_includes.h"
#include "mmc_def.h"

namespace ock {
namespace mmc {
class MmcLocalService : public MmcReferable {
public:
  virtual MmcErrorCode Start(mmc_local_service_config_t *config) = 0;
  virtual void Stop() = 0;
};
using MmcLocalServicePtr = MmcRef<MmcLocalService>;
} // namespace mmc
} // namespace ock

#endif //MEM_FABRIC_MMC_LOCAL_SERVICE_H
