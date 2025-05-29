/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2026. All rights reserved.
 */
#ifndef __MEMFABRIC_MOBS_CLIENT_H__
#define __MEMFABRIC_MOBS_CLIENT_H__

#include "mobs.h"
#include "mobs_def.h"

#ifdef __cplusplus
extern "C" {
#endif

int32_t mobsc_init(mobs_client_config_t *config);

void mobsc_uninit();

int32_t mobsc_put(const char *key, mobs_buffer *buf, uint32_t flags);

int32_t mobsc_get(const char *key, mobs_buffer *buf, uint32_t flags);

mobs_location_t mobsc_get_location(const char *key, uint32_t flags);

int32_t mobsc_remove(const char *key, uint32_t flags);

#ifdef __cplusplus
}
#endif

#endif //__MEMFABRIC_MOBS_CLIENT_H__
