/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2026. All rights reserved.
 */
#ifndef MEM_FABRIC_MMC_MEM_OBJ_META_H
#define MEM_FABRIC_MMC_MEM_OBJ_META_H

#include "mmc_mem_blob.h"
#include "mmc_ref.h"
#include "mmc_spinlock.h"

namespace ock {
namespace mmc {

static const uint16_t MAX_NUM_BLOB_CHAINS = 5; // to make sure MmcMemObjMeta <= 64 bytes

class MmcMemObjMeta : public MmcReferable {
public:
    inline MmcMemObjMeta() = default;
    inline void ExtendLease(uint64_t ttl);
    inline uint16_t NumBlobs();
    inline bool IsLeaseExpired();
    void AddBlob(MmcMemBlobPtr blob);
    Result RemoveBlob(MmcMemBlobPtr blob);

private:
    uint8_t numBlobs_ {0};
    uint8_t priority_ {0};
    uint16_t prot_ {0};
    MmcMemBlobPtr blobs_[MAX_NUM_BLOB_CHAINS];
    uint64_t lease_ {0};
    uint32_t size_{0};  // byteSize of each blob

    Spinlock spinlock_;  // 4 bytes
};

using MmcMemBlobPtr = MmcRef<MmcMemBlob>;

} // namespace mmc
} // namespace ock

#endif // MEM_FABRIC_MMC_MEM_OBJ_META_H
