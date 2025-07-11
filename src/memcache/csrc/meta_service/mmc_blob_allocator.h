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

struct SpaceRange {
    const uint64_t offset_;
    const uint64_t size_;

    SpaceRange(uint64_t offset, uint64_t size) : offset_(offset), size_(size) {}
};

struct RangeSizeFirst {
    bool operator()(const SpaceRange &sr1, const SpaceRange &sr2) const
    {
        if (sr1.size_ != sr2.size_) {
            return sr1.size_ < sr2.size_;
        }
        return sr1.offset_ < sr2.offset_;
    }
};

class MmcBlobAllocator : public MmcReferable {
public:
    MmcBlobAllocator(uint32_t rank, uint16_t mediaType, uint64_t bm, uint64_t capacity)
        : rank_(rank), mediaType_(mediaType), bm_(bm), capacity_(capacity)
    {
        addressTree_[0] = capacity;
        sizeTree_.insert({0,capacity});
    }
    ~MmcBlobAllocator() {}
    bool CanAlloc(uint64_t blobSize);
    MmcMemBlobPtr Alloc(uint64_t blobSize);
    Result Release(MmcMemBlobPtr blob);

private:
    static uint64_t AllocSizeAlignUp(uint64_t size);

private:
    const uint32_t rank_;      /* rank id of the space */
    const uint16_t mediaType_; /* media type of the space */
    const uint64_t bm_;        /* bm address */
    const uint64_t capacity_;  /* capacity of the space */

    std::map<uint64_t, uint64_t> addressTree_;
    std::set<SpaceRange, RangeSizeFirst> sizeTree_;

    Spinlock spinlock_;
};

using MmcBlobAllocatorPtr = MmcRef<MmcBlobAllocator>;

} // namespace mmc
} // namespace ock

#endif // MEM_FABRIC_MMC_BLOB_ALLOCATOR_H