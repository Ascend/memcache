/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2026. All rights reserved.
 */
#include "mmc_client.h"
#include "mmc_common_includes.h"

using namespace ock::mmc;

MMC_API int32_t mmcc_init(mmc_client_config_t *config)
{
    return MMC_OK;
}

MMC_API void mmcc_uninit()
{
    return;
}

MMC_API int32_t mmcc_put(const char *key, mmc_buffer *buf, uint32_t flags)
{
    return MMC_OK;
}

MMC_API int32_t mmcc_get(const char *key, mmc_buffer *buf, uint32_t flags)
{
    return MMC_OK;
}

MMC_API mmc_location_t mmcc_get_location(const char *key, uint32_t flags)
{
    return {};
}

MMC_API int32_t mmcc_remove(const char *key, uint32_t flags)
{
    return MMC_OK;
}