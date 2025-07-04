/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2026. All rights reserved.
 */

#include "mmc_blob_allocator.h"

namespace ock {
namespace mmc {

Result MmcBlobAllocator::Initialize()
{
    totalSize_ = (capacity_ / SIZE_32K) * SIZE_32K;
    totalBlocks_ = totalSize_ / SIZE_32K;
    blocks_ = new mmcBlock_t[totalBlocks_];
    if (blocks_ == nullptr) {
        return MMC_MALLOC_FAILED;
    }
    for (uint32_t i = 0; i < totalBlocks_; i++) {
        blocks_[i].blockPos = i;
        if (i == totalBlocks_ - 1) {
            blocks_[i].next = nullptr;
        } else {
            blocks_[i].next = &blocks_[i + 1];
        }
    }
    head_ = &blocks_[0];
    tail_ = &blocks_[totalBlocks_ - 1];

    return MMC_OK;
}

Result MmcBlobAllocator::PreAlloc(uint64_t blobSize)
{
    std::lock_guard<Spinlock> guard(spinlock_);
    if (blobSize > SIZE_32K) {
        return MMC_ERROR;
    }
    if (totalSize_ == usedSize_) {
        return MMC_NOT_ENOUGH_MEMORY;
    }
    usedSize_ += SIZE_32K;
    return MMC_OK;
}

MmcMemBlobPtr MmcBlobAllocator::Alloc(uint64_t blobSize)
{
    std::lock_guard<Spinlock> guard(spinlock_);
    if (blobSize > SIZE_32K) {
        return nullptr;
    }
    mmcBlock_t *cur = head_;
    if (head_ == tail_) {
        head_ = nullptr;
        tail_ = nullptr;
    } else {
        head_ = head_->next;
    }
    cur->next = nullptr;
    // 空间清零要在这做吗
    return MmcMakeRef<MmcMemBlob>(rank_, bm_ + cur->blockPos * SIZE_32K, SIZE_32K, mediaType_, ALLOCATED);
}

Result MmcBlobAllocator::Free(MmcMemBlobPtr blob)
{
    std::lock_guard<Spinlock> guard(spinlock_);
    if (blob == nullptr) {
        return MMC_ERROR;
    }
    if (blob->Size() > SIZE_32K) {
        return MMC_ERROR;
    }
    if (bm_ > blob->Gva() || bm_ + totalSize_ < blob->Gva() + blob->Size()) {
        return MMC_ERROR;
    }

    if (tail_ == nullptr) {
        head_ = &blocks_[(blob->Gva() - bm_) / SIZE_32K];
        tail_ = &blocks_[(blob->Gva() - bm_) / SIZE_32K];
    } else {
        tail_->next = &blocks_[(blob->Gva() - bm_) / SIZE_32K];
        tail_ = tail_->next;
    }
    tail_->next = nullptr;
    usedSize_ -= SIZE_32K;
    return MMC_OK;
}
}
}