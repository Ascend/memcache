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

#include "mmc_logger.h"


extern "C" {
void mmc_logger(int level, const char *msg)
{
    switch (level) {
        case ock::mmc::DEBUG_LEVEL: MMC_LOG_DEBUG(msg); break;
        case ock::mmc::INFO_LEVEL: MMC_LOG_INFO(msg); break;
        case ock::mmc::WARN_LEVEL: MMC_LOG_WARN(msg); break;
        case ock::mmc::ERROR_LEVEL: MMC_LOG_ERROR(msg); break;
        default: break;
    }
}
}