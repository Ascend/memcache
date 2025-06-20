/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2026. All rights reserved.
 */
#include "mmc_local_service_default.h"

namespace ock {
namespace mmc {
MmcErrorCode MmcLocalServiceDefault::Start(mmc_local_service_config_t *config)
{
    return MMC_OBJECT_NOT_EXISTS;
}
void MmcLocalServiceDefault::Stop() {}
}
}