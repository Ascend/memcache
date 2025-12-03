/*
* Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 * MemFabric_Hybrid is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
*/
#ifndef __MEMFABRIC_MMC_SERVICE_H__
#define __MEMFABRIC_MMC_SERVICE_H__

#include "mmc_def.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Start meta service of Distributed Memory Cache, which is global
 *
 * @param config           [in] config of meta service
 * @return handle of meta service if successful, NULL if failed
 */
mmc_meta_service_t mmcs_meta_service_start(mmc_meta_service_config_t *config);

/**
 * @brief Stop meta service of Distributed Memory Cache
 *
 * @param handle           [in] meta service handle, created by <i>mobss_meta_service_start</i>
 */
void mmcs_meta_service_stop(mmc_meta_service_t handle);

/**
 * @brief Start local service of Distributed Memory Cache, which take charges of management
 * of memory object locally
 *
 * @param config           [in] config of local service
 * @return handle of local service if successful, NULL if failed
 */
mmc_local_service_t mmcs_local_service_start(mmc_local_service_config_t *config);

/**
 * @brief Stop local service of Distributed Memory Cache
 *
 * @param handle           [in] local service handle, created by <i>mobss_local_service_start</i>
 */
void mmcs_local_service_stop(mmc_local_service_t handle);

#ifdef __cplusplus
}
#endif

#endif // __MEMFABRIC_MMC_SERVICE_H__