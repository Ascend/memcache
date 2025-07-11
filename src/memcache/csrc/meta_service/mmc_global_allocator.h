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
        Result ret = MmcLocalityStrategy::ArrangeLocality(allocators_, allocReq, blobs);
        globalAllocLock_.UnlockRead();
        return ret;
    };

    Result Free(MmcMemBlobPtr blob)
    {
        globalAllocLock_.LockRead();
        MmcLocation location{blob->Rank(), blob->MediaType()};
        if (allocators_.find(location) == allocators_.end()) {
            globalAllocLock_.UnlockRead();
            return MMC_ERROR;
        }
        auto allocator = allocators_[location];
        Result ret = allocator->Release(blob);
        globalAllocLock_.UnlockRead();
        return ret;
    };

    Result Mount(const MmcLocation &loc, const MmcLocalMemlInitInfo &localMemInitInfo)
    {
        globalAllocLock_.LockWrite();
        auto iter = allocators_.find(loc);
        if (iter != allocators_.end()) {
            MMC_LOG_WARN("Cannot mount at the existing position!");
            globalAllocLock_.UnlockWrite();
            return MMC_INVALID_PARAM;
        }
        allocators_[loc] =
            MmcMakeRef<MmcBlobAllocator>(loc.rank_, loc.mediaType_, localMemInitInfo.bm_, localMemInitInfo.capacity_);
        globalAllocLock_.UnlockWrite();
        return MMC_OK;
    }

    Result Unmount(const MmcLocation &loc)
    {
        globalAllocLock_.LockWrite();
        auto iter = allocators_.find(loc);
        if (iter == allocators_.end()) {
            MMC_LOG_WARN("Cannot find the given location in the mem pool!");
            globalAllocLock_.UnlockWrite();
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