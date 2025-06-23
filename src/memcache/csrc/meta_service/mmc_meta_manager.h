/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2026. All rights reserved.
 */
#ifndef MEM_FABRIC_MMC_META_MANAGER_H
#define MEM_FABRIC_MMC_META_MANAGER_H

#include "mmc_global_allocator.h"
#include "mmc_lookup_map.h"
#include "mmc_mem_obj_meta.h"
#include "mmc_meta_service.h"

namespace ock {
namespace mmc {

static const uint16_t NUM_BUCKETS = 128;

class MmcMetaManger : public MmcReferable {
public:
    MmcMetaManger(MmcMemPoolInitInfo mmcMemPoolInitInfo, uint64_t defaultTtl) : defaultTtl_(defaultTtl)
    {
        globalAllocator_ = MmcMakeRef<MmcGlobalAllocator>(mmcMemPoolInitInfo);
    };

    /**
     * @brief Get the metaInfo by key (filtered by DATA_READY state) & renew the lease
     * @param key          [in] key of the meta object
     * @param metaInfo     [out] metaInfo of the found meta object filltered by state
     */
    Result GetReadyBlobs(const std::string &key, MmcMetaInfoPtr &metaInfo);
    Result Alloc(const std::string &key, const AllocRequest &allocReq, MmcMemObjMetaPtr &objMeta);
    Result FreeStart(std::string key, uint32_t flags);

private:
    MmcLookupMap<std::string, MmcMemObjMetaPtr, NUM_BUCKETS> objMetaLookupMap_;
    MmcGlobalAllocatorPtr globalAllocator_;
    uint64_t defaultTtl_; /* defult ttl in miliseconds*/
};
} // namespace mmc
} // namespace ock

#endif // MEM_FABRIC_MMC_META_MANAGER_H
