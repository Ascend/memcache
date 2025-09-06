/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023. All rights reserved.
 */
#ifndef MF_HYBM_CORE_HYBM_DATA_OP_H
#define MF_HYBM_CORE_HYBM_DATA_OP_H

#include <stddef.h>
#include <stdint.h>
#include "hybm_def.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ASYNC_COPY_FLAG (1UL << (0))
/**
 * @brief copies <i>count</i> bytes from memory area <i>src</i> to memory area <i>dest</i>.
 * @param e                [in] entity created by hybm_create_entity
 * @param src              [in] pointer to copy source memory area.
 * @param dest             [in] pointer to copy destination memory area.
 * @param count            [in] copy memory size in bytes.
 * @param direction        [in] copy direction.
 * @param stream           [in] copy used stream (use default stream if stream == NULL)
 * @param flags            [in] optional flags, default value 0.
 * @return 0 if reserved successful
 */
int32_t hybm_data_copy(hybm_entity_t e, const void *src, void *dest, size_t count, hybm_data_copy_direction direction,
                       void *stream, uint32_t flags);

/**
 * @brief copies <i>count</i> bytes from memory area <i>src</i> to memory area <i>dest</i>.
 * @param e                [in] entity created by hybm_create_entity
 * @param src              [in] pointer to copy source memory area.
 * @param spitch           [in] pitch of source memory
 * @param dest             [in] pointer to copy destination memory area.
 * @param dpitch           [in] pitch of destination memory
 * @param width            [in] width of matrix transfer
 * @param height           [in] height of matrix transfer
 * @param direction        [in] copy direction.
 * @param stream           [in] copy used stream (use default stream if stream == NULL)
 * @param flags            [in] optional flags, default value 0.
 * @return 0 if reserved successful
 */
int32_t hybm_data_copy_2d(hybm_entity_t e, const void *src, uint64_t spitch,
                          void *dest, uint64_t dpitch, uint64_t width, uint64_t height,
                          hybm_data_copy_direction direction, void *stream, uint32_t flags);

/**
 * @brief wait data_copy_op_async finish .
 * @param e                [in] entity created by hybm_create_entity
 * @return 0 if reserved successful
 */
int32_t hybm_wait(hybm_entity_t e);

/**
 * @brief batch copy data bytes from memory area <i>sources</i> to memory area <i>destinations</i>.
 * @param e                       [in] entity created by hybm_create_entity
 * @param params.sources          [in] array of pointers to copy sources memory area.
 * @param params.destinations     [in] array of pointer to copy destinations memory area.
 * @param params.dataSizes        [in] array of copy memory sizes in bytes.
 * @param params.batchSize        [in] array size for <i>sources</>, <i>destinations</i> and <i>counts</>.
 * @param direction               [in] copy direction.
 * @param stream                  [in] copy used stream (use default stream if stream == NULL)
 * @param flags                   [in] optional flags, default value 0.
 * @return 0 if successful
 */
int32_t hybm_data_batch_copy(hybm_entity_t e, hybm_batch_copy_params* params,
                             hybm_data_copy_direction direction, void *stream, uint32_t flags);

#ifdef __cplusplus
}
#endif

#endif  // MF_HYBM_CORE_HYBM_DATA_OP_H
