/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2026. All rights reserved.
 */
#ifndef MEM_FABRIC_MMC_BLOB_ALLOCATOR_H
#define MEM_FABRIC_MMC_BLOB_ALLOCATOR_H

#include "mmc_mem_blob.h"
#include "mmc_spinlock.h"

namespace ock {
namespace mmc {

#define SIZE_32K (uint64_t)(32 * 1024)

typedef struct MmcBlock {
    uint32_t blockPos;
    struct MmcBlock *next;
} mmcBlock_t;

class MmcBlobAllocator : public MmcReferable {
public:
    MmcBlobAllocator(uint32_t rank, uint16_t mediaType, uint64_t bm, uint64_t capacity)
        : rank_(rank), mediaType_(mediaType), bm_(bm), capacity_(capacity) {};
    ~MmcBlobAllocator()
    {
        if (blocks_ != nullptr) {
            delete[] blocks_;
        }
    }
    Result Initialize();
    Result PreAlloc(uint64_t blobSize);
    MmcMemBlobPtr Alloc(uint64_t blobSize);
    Result Free(MmcMemBlobPtr blob);

private:
    const uint32_t rank_;      /* rank id of the space */
    const uint16_t mediaType_; /* media type of the space */
    const uint64_t bm_;        /* bm address */
    const uint64_t capacity_;  /* capacity of the space */

    uint64_t totalSize_ = 0;
    uint64_t usedSize_ = 0;
    uint32_t totalBlocks_ = 0;

    mmcBlock_t *blocks_ = nullptr;
    mmcBlock_t *head_ = nullptr;
    mmcBlock_t *tail_ = nullptr;

    Spinlock spinlock_;
};

using MmcBlobAllocatorPtr = MmcRef<MmcBlobAllocator>;

} // namespace mmc
} // namespace ock

#endif // MEM_FABRIC_MMC_BLOB_ALLOCATOR_H