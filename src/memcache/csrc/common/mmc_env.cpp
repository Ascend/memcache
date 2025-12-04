/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 * MemCache_Hybrid is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
*/
#include "mmc_env.h"

#include <string>

#include "mmc_functions.h"

namespace ock {
namespace mmc {
std::string MMC_META_CONF_PATH = SafeGetEnv("MMC_META_CONFIG_PATH");
std::string MMC_LOCAL_CONF_PATH = SafeGetEnv("MMC_LOCAL_CONFIG_PATH");
std::string META_POD_NAME = SafeGetEnv("META_POD_NAME");
std::string META_NAMESPACE = SafeGetEnv("META_NAMESPACE");
std::string META_LEASE_NAME = SafeGetEnv("META_LEASE_NAME");
} // namespace mmc
} // namespace ock