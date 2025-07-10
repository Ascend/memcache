/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2026. All rights reserved.
 */
#pragma once

#include <utility>

namespace ock {
namespace mmc {
namespace ConfConstant {
// add configuration here with default values
constexpr auto OCK_MMC_META_SERVICE_DISCOVERY_URL = std::make_pair("ock.mmc.meta_service.discovery_url", "tcp://127.0.0.1:5000");
constexpr auto OCK_MMC_META_SERVICE_WORLD_SIZE = std::make_pair("ock.mmc.meta_service.world_size", 1);

constexpr int MIN_META_SERVICE_WORLD_SIZE = 1;
constexpr int MAX_META_SERVICE_WORLD_SIZE = 64;
}
} // namespace mmc
} // namespace ock