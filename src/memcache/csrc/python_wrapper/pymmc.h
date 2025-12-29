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
#ifndef MMC_PYMMC_H
#define MMC_PYMMC_H

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#pragma GCC diagnostic pop

#include <csignal>
#include <mutex>
#include <string>
#include <unordered_set>
#include <sstream>

#include "mmc_def.h"
#include "mmc_types.h"
#include "mmcache.h"
#include "mmcache_store.h"

using namespace ock::mmc;


#endif