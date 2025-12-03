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
#ifndef __MEMFABRIC_MMC_H__
#define __MEMFABRIC_MMC_H__

#include "stdint.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint32_t deviceId;
} mmc_init_config;

/**
 * @brief Initialize the memcache client and local service
 * @param config              [in] init confid @mmc_init_config
 * @return 0 if successful,
 */
int32_t mmc_init(const mmc_init_config *config);

/**
 * @brief Set external log function, user can set customized logger function,
 * in the customized logger function, user can use unified logger utility,
 * then the log message can be written into the same log file as caller's,
 * if it is not set, acc_links log message will be printed to stdout.
 *
 * level description:
 * 0 DEBUG,
 * 1 INFO,
 * 2 WARN,
 * 3 ERROR
 *
 * @param func             [in] external logger function
 * @return 0 if successful
 */
int32_t mmc_set_extern_logger(void (*func)(int level, const char *msg));

/**
 * @brief set log print level
 *
 * @param level            [in] log level, 0:debug 1:info 2:warn 3:error
 * @return 0 if successful
 */
int32_t mmc_set_log_level(int level);

/**
 * @brief Un-Initialize the smem running environment
 */
void mmc_uninit(void);

#ifdef __cplusplus
}
#endif


#endif //__MEMFABRIC_MMC_H__