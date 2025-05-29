/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2026. All rights reserved.
 */
#include "mobs_client.h"
#include "mobs_common_includes.h"

using namespace ock::mobs;

MOBS_API int32_t mobsc_init(mobs_client_config_t *config)
{
    return MO_OK;
}

MOBS_API void mobsc_uninit()
{
    return;
}

MOBS_API int32_t mobsc_put(const char *key, mobs_buffer *buf, uint32_t flags)
{
    return MO_OK;
}

MOBS_API int32_t mobsc_get(const char *key, mobs_buffer *buf, uint32_t flags)
{
    return MO_OK;
}

MOBS_API mobs_location_t mobsc_get_location(const char *key, uint32_t flags)
{
    return {};
}

MOBS_API int32_t mobsc_remove(const char *key, uint32_t flags)
{
    return MO_OK;
}