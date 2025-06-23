/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2026. All rights reserved.
 */
#ifndef MEM_FABRIC_MMC_BLOB_ALLOCATOR_H
#define MEM_FABRIC_MMC_BLOB_ALLOCATOR_H

#include "mmc_mem_blob.h"

namespace ock {
namespace mmc {

class MmcBlobAllocator : public MmcReferable {
public:
    MmcBlobAllocator(uint32_t rank, uint16_t mediaType, uint64_t bm, uint64_t capacity)
        : rank_(rank), mediaType_(mediaType), bm_(bm), capacity_(capacity) {};
    MmcMemBlobPtr Alloc(uint64_t blobSize) { return nullptr; };
    Result Free(MmcMemBlobPtr blob) { return MMC_OK; };

private:
    const uint32_t rank_;      /* rank id of the space */
    const uint16_t mediaType_; /* media type of the space */
    const uint64_t bm_;        /* bm address */
    const uint64_t capacity_;  /* capacity of the space */
};

using MmcBlobAllocatorPtr = MmcRef<MmcBlobAllocator>;

} // namespace mmc
} // namespace ock

#endif // MEM_FABRIC_MMC_BLOB_ALLOCATOR_H