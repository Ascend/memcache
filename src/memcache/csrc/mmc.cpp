/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2026. All rights reserved.
 */
#include "mmc.h"
#include "mmc_common_includes.h"

using namespace ock::mmc;

MMC_API int32_t mmc_init(uint32_t flags)
{
    return MMC_OK;
}

MMC_API int32_t mmc_set_extern_logger(void (*func)(int level, const char *msg))
{
    return MMC_OK;
}

MMC_API int32_t mmc_set_log_level(int level)
{
    return MMC_OK;
}

MMC_API void mmc_uninit()
{
    return;
}

MMC_API const char *mmc_get_last_err_msg()
{
    return NULL;
}

MMC_API const char *mmc_get_and_clear_last_err_msg()
{
    return NULL;
}