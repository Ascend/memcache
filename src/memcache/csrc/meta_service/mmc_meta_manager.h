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

static const uint16_t NUM_BUCKETS = 8;

class MmcMetaManger : public MmcReferable {
public:
    MmcMetaManger(MmcMemPoolInitInfo mmcMemPoolInitInfo, uint64_t defaultTtl) : defaultTtl_(defaultTtl)
    {
        globalAllocator_ = MmcMakeRef<MmcGlobalAllocator>(mmcMemPoolInitInfo);
    };

    /**
     * @brief Get the meta object and extend the lease
     * @param key          [in] key of the meta object
     * @param objMeta      [out] the meta object obtained
     */
    Result Get(const std::string &key, MmcMemObjMetaPtr &objMeta);

    /**
     * @brief Alloc the global memeory space and create the meta object
     * @param key          [in] key of the meta object
     * @param metaInfo     [out] the meta object created
     */

    Result Alloc(const std::string &key, const AllocProperty &allocReq, MmcMemObjMetaPtr &objMeta);

    /**
     * @brief Get the blobs by key and the filter, and renew the lease
     * @param key          [in] key of the meta object
     * @param filter       [in] filter used to choose the blobs
     * @param blobs        [out] the blobs found by the key and filter
     */
    Result GetBlobs(const std::string &key, const MmcBlobFilterPtr &filter, std::vector<MmcMemBlobPtr> &blobs);

private:
    MmcLookupMap<std::string, MmcMemObjMetaPtr, NUM_BUCKETS> objMetaLookupMap_;
    MmcGlobalAllocatorPtr globalAllocator_;
    uint64_t defaultTtl_; /* defult ttl in miliseconds*/
};
using MmcMetaMangerPtr = MmcRef<MmcMetaManger>;
}  // namespace mmc
}  // namespace ock

#endif  // MEM_FABRIC_MMC_META_MANAGER_H
