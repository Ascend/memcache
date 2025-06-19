/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2026. All rights reserved.
 */
#include "mmc_mem_blob.h"

namespace ock {
namespace mmc {

MmcMemBlob::MmcMemBlob(uint32_t rank, uint64_t gva, uint32_t size, uint16_t mediaType)
    : rank_(rank), gva_(gva), size_(size), mediaType_(mediaType), nextBlob_(nullptr)
{
}

Result MmcMemBlob::UpdateState(BlobState newState, BlobState oldState)
{
    std::lock_guard guard(spinlock_);
    if (state_ == oldState) {
        state_ = newState;
        return MMC_OK;
    } else {
        return MMC_UNMATCHED_STATE;
    }
}

uint64_t MmcMemBlob::Gva() { return gva_; }

uint64_t MmcMemBlob::Size() { return size_; }

int16_t MmcMemBlob::MediaType() { return mediaType_; }

int32_t MmcMemBlob::Rank() { return rank_; }

BlobState MmcMemBlob::State()
{
    std::lock_guard guard(spinlock_);
    BlobState curState = state_;
    return curState;
}

MmcRef<MmcMemBlob> MmcMemBlob::Next()
{
    std::lock_guard guard(spinlock_);
    return nextBlob_;
}

Result MmcMemBlob::SetNext(MmcRef<MmcMemBlob> nextBlob)
{
    std::lock_guard guard(spinlock_);
    if (nextBlob_ == nullptr) {
        nextBlob_ = nextBlob;
        return MMC_OK;
    } else {
        return MMC_ERROR;
    }
}

} // namespace mmc
} // namespace ock