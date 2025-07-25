/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2026. All rights reserved.
 */
#pragma once
#include <cstdlib>


namespace ock::mmc {
    const char* MMC_META_CONF_PATH = std::getenv("MMC_META_CONFIG_PATH");
    const char* MMC_LOCAL_CONF_PATH = std::getenv("MMC_LOCAL_CONFIG_PATH");
}
