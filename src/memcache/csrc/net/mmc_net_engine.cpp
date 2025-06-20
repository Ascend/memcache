/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2026. All rights reserved.
 */
#include "mmc_ref.h"
#include "mmc_net_engine.h"
#include "mmc_net_engine_acc.h"

namespace ock {
namespace mmc {
NetEnginePtr NetEngine::Create()
{
    // TODO
    MMC_LOG_INFO("NetEngine Create");
    return  Convert<NetEngineAcc, NetEngine>(MmcMakeRef<NetEngineAcc>());
}

}
}
