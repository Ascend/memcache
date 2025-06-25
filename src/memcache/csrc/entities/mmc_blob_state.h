/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2026. All rights reserved.
 */
#ifndef MEMFABRIC_HYBRID_MMC_BLOB_STATE_H
#define MEMFABRIC_HYBRID_MMC_BLOB_STATE_H

#include "mmc_common_includes.h"

namespace ock {
namespace mmc {
/**
 * @brief State of blob
 */
enum BlobState : uint8_t {
    NONE,
    ALLOCATED,
    DATA_READY,
    REMOVING,
    FINAL,
};

/**
 * @brief Block action result, which a part of transition table
 */
enum BlobActionResult : uint8_t {
    MMC_REMOVE_START,
    MMC_REMOVE_OK,
    MMC_WRITE_OK,
    MMC_WRITE_FAIL,
    MMC_RECV_LOC_SIGN_WRITE_OK,
    MMC_RECV_LOC_SIGN_REMOVE_OK,
};

}  // namespace mmc
}  // namespace ock

#endif