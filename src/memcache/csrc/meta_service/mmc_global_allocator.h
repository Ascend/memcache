/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2026. All rights reserved.
 */
#ifndef MEM_FABRIC_MMC_GLOBAL_ALLOCATOR_H
#define MEM_FABRIC_MMC_GLOBAL_ALLOCATOR_H

#include "mmc_blob_allocator.h"
#include "mmc_locality_strategy.h"
#include "mmc_read_write_lock.h"

namespace ock {
namespace mmc {

using MmcMemPoolInitInfo = std::map<MmcLocation, MmcLocalMemlInitInfo>;

class MmcGlobalAllocator : public MmcReferable {
public:
    MmcGlobalAllocator() = default;

    Result Alloc(const AllocOptions &allocReq, std::vector<MmcMemBlobPtr> &blobs)
    {
        globalAllocLock_.LockRead();
        std::vector<MmcLocation> locations;
        Result ret = MmcLocalityStrategy::ArrangeLocality(allocators_, allocReq, locations);
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
        globalAllocLock_.UnlockRead();
        return MMC_OK;
    };

    Result Free(MmcMemBlobPtr blob)
    {
        globalAllocLock_.LockRead();
        MmcLocation location{blob->Rank(), blob->MediaType()};
        auto allocator = allocators_[location];
        Result ret = allocator->Free(blob);
        globalAllocLock_.UnlockRead();
        return ret;
    };

    Result Mount(const MmcLocation &loc, const MmcLocalMemlInitInfo &localMemInitInfo)
    {
        globalAllocLock_.LockWrite();
        auto iter = allocators_.find(loc);
        if (iter != allocators_.end()) {
            MMC_LOG_WARN("Cannot mount at the existing position!");
            return MMC_INVALID_PARAM;
        }
        allocators_[loc] =
            MmcMakeRef<MmcBlobAllocator>(loc.rank_, loc.mediaType_, localMemInitInfo.bm_, localMemInitInfo.capacity_);
        allocators_[loc]->Initialize();
        globalAllocLock_.UnlockWrite();
        return MMC_OK;
    }

    Result Unmount(const MmcLocation &loc)
    {
        globalAllocLock_.LockWrite();
        auto iter = allocators_.find(loc);
        if (iter != allocators_.end()) {
            MMC_LOG_WARN("Cannot find the given location in the mem pool!");
            return MMC_INVALID_PARAM;
        }
        allocators_.erase(iter);
        globalAllocLock_.UnlockWrite();
        return MMC_OK;
    }

private:
    MmcAllocators allocators_;
    ReadWriteLock globalAllocLock_;
};

using MmcGlobalAllocatorPtr = MmcRef<MmcGlobalAllocator>;

}  // namespace mmc
}  // namespace ock

#endif  // MEM_FABRIC_MMC_GLOBAL_ALLOCATOR_H