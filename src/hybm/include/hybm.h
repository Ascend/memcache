/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */
#ifndef MEM_FABRIC_HYBRID_HYBM_H
#define MEM_FABRIC_HYBRID_HYBM_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define HYBM_INIT_GVM_FLAG (1ULL << 1ULL) // Init the GVM module, enable to use Host DRAM

/**
 * @brief Initialize hybrid big memory library
 *
 * @param deviceId         [in] npu device id
 * @param flags            [in] optional flags
 * @return 0 if successful
 */
int32_t hybm_init(uint16_t deviceId, uint64_t flags);

/**
 * @brief UnInitialize hybrid big memory library
 */
void hybm_uninit(void);

/**
 * @brief Get error message by error code
 *
 * @param errCode          [in] error number returned by other functions
 * @return error string if the error code exists, null if the error is invalid
 */
const char *hybm_get_error_string(int32_t errCode);

#ifdef __cplusplus
}
#endif

#endif // MEM_FABRIC_HYBRID_HYBM_H
