/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2026. All rights reserved.
 */
#ifndef MEM_FABRIC_MMC_MEM_BLOB_H
#define MEM_FABRIC_MMC_MEM_BLOB_H

#include "mmc_blob_state.h"
#include "mmc_ref.h"
#include "mmc_spinlock.h"

namespace ock {
namespace mmc {

class MmcMemBlob : public MmcReferable {
public:
    MmcMemBlob() = delete;
    inline MmcMemBlob(uint32_t rank, uint64_t gva, uint32_t size, uint16_t mediaType);

    inline Result UpdateState(BlobState newState, BlobState oldState);
    inline Result SetNext(MmcRef<MmcMemBlob> nextBlob);

    inline int32_t Rank();
    inline uint64_t Gva();
    inline uint64_t Size();
    inline int16_t MediaType();
    inline BlobState State();
    inline MmcRef<MmcMemBlob> Next();

private:
    const uint32_t rank_;
    const uint64_t gva_;
    const uint32_t size_;
    const uint16_t mediaType_;
    BlobState state_{BlobState::INIT};
    uint16_t prot_{0};

    MmcRef<MmcMemBlob> nextBlob_;

    Spinlock spinlock_;
};

using MmcMemBlobPtr = MmcRef<MmcMemBlob>;

} // namespace mmc
} // namespace ock

#endif // MEM_FABRIC_MMC_MEM_BLOB_H
