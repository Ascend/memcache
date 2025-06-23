/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2026. All rights reserved.
 */
#ifndef MEM_FABRIC_MMC_GLOBAL_ALLOCATOR_H
#define MEM_FABRIC_MMC_GLOBAL_ALLOCATOR_H

#include "mmc_blob_allocator.h"
#include "mmc_locality_strategy.h"

namespace ock {
namespace mmc {

struct MmcLocalMemlInitInfo {
    uint64_t bm_;
    uint64_t capacity_;
};

using MmcMemPoolInitInfo = std::map<std::pair<uint32_t, uint16_t>, MmcLocalMemlInitInfo>;
using MmcAllocators = std::map<std::pair<uint32_t, uint16_t>, MmcBlobAllocatorPtr>;

class MmcGlobalAllocator : public MmcReferable {
public:
    MmcGlobalAllocator(MmcMemPoolInitInfo mmcMemPoolInitInfo) : mmcMemPoolInitInfo_(mmcMemPoolInitInfo)
    {
        for (const auto &localInfo : mmcMemPoolInitInfo_) {
            mmcMemPoolCurInfo_[localInfo.first].capacity_ = localInfo.second.capacity_;
            allocators_[localInfo.first] = MmcMakeRef<MmcBlobAllocator>(
                localInfo.first.first, localInfo.first.second, localInfo.second.bm_, localInfo.second.capacity_);
        }
    };
    Result Alloc(const AllocRequest &allocReq, std::vector<MmcMemBlobPtr> &blobs)
    {
        // todo: need to deal with the locks
        std::vector<std::pair<uint32_t, uint16_t>> locations;
        Result ret = MmcLocalityStrategy::ArrangeLocality(mmcMemPoolCurInfo_, allocReq, locations);
        if (ret != MMC_OK) {
            return MMC_ERROR;
        }
        for (auto loc : locations) {
            MmcMemBlobPtr blob = allocators_[loc]->Alloc(allocReq.blobSize_);
            if (blob != nullptr) {
                blobs.push_back(blob);
            } else {
                return MMC_ERROR;
            }
        }
        return MMC_OK;
    };
    Result Free(MmcMemBlobPtr blob)
    {
        // todo: need to deal with the locks
        MmcLocation location(blob->Rank(), blob->MediaType());
        auto allocator = allocators_[location];
        return allocator->Free(blob);
    };

private:
    Result UpdateMemPoolInfo() { return MMC_OK; };

    MmcMemPoolInitInfo mmcMemPoolInitInfo_;
    MmcMemPoolCurInfo mmcMemPoolCurInfo_;
    MmcAllocators allocators_;
};

using MmcGlobalAllocatorPtr = MmcRef<MmcGlobalAllocator>;

} // namespace mmc
} // namespace ock

#endif // MEM_FABRIC_MMC_GLOBAL_ALLOCATOR_H