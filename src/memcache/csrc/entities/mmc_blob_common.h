/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#ifndef MF_HYBRID_MMC_BLOB_COMMON_H
#define MF_HYBRID_MMC_BLOB_COMMON_H
#include "mmc_ref.h"
namespace ock {
namespace mmc {
class MmcMemBlob;
using MmcMemBlobPtr = MmcRef<MmcMemBlob>;

struct MmcMemBlobDesc {
    uint32_t rank_ = UINT32_MAX;        /* rank id of the blob located */
    uint64_t size_ = 0;                 /* data size of the blob */
    uint64_t gva_ = UINT64_MAX;         /* global virtual address */
    uint16_t mediaType_ = UINT16_MAX;   /* media type where blob located */

    MmcMemBlobDesc() = default;
    MmcMemBlobDesc(const uint32_t &rank, const uint64_t &gva, const uint32_t &size, const uint16_t &mediaType)
        : rank_(rank), size_(size), gva_(gva), mediaType_(mediaType)
    {
    }
};
}
}
#endif // MF_HYBRID_MMC_BLOB_COMMON_H
