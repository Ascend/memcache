/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2026. All rights reserved.
 */
#ifndef __MEMFABRIC_MOBS_DEF_H__
#define __MEMFABRIC_MOBS_DEF_H__

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void *mobs_meta_service_t;
typedef void *mobs_local_service_t;
typedef void *mobs_client_t;

typedef struct {
    char discoveryURL[1024]; /* composed by schema and url, e.g. tcp:// or etcd:// or zk:// */
} mobs_meta_service_config_t;

typedef struct {
    char discoveryURL[1024];
} mobs_local_service_config_t;

typedef struct {
    char discoveryURL[1024];
} mobs_client_config_t;

typedef struct {
    uint64_t addr : 60;
    uint64_t type : 4;  // 0 dram, 1 hbm
    union {
        struct {
            uint64_t dpitch : 48;
            uint64_t layerOffset : 16;
            uint32_t width;  // max 4GB
            uint16_t layerNum;
            uint16_t layerCount;
        } hbm;
        struct {
            uint64_t offset;
            uint64_t len;
        } dram;
    };
} mobs_buffer;

typedef struct {
    int32_t xx;  // TODO
} mobs_location_t;

#ifdef __cplusplus
}
#endif

#endif  //__MEMFABRIC_MOBS_DEF_H__
