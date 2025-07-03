/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2026. All rights reserved.
 */

#ifndef SMEM_MMC_LOCAL_MANAGER_H
#define SMEM_MMC_LOCAL_MANAGER_H
#include "acl/acl.h"
#include "mmc_common_includes.h"
#include "mmc_def.h"
#include "mmc_lookup_map.h"
#include "mmc_mem_obj_meta.h"
#include "smem.h"
#include "smem_shm.h"

namespace ock {
namespace mmc {
static const uint16_t NUM_BUCKETS_LOCAL = 2;

class MmcLocalHandler : public MmcReferable {
public:
    MmcLocalHandler(void *handle) : handle_(handle) {};

    /**
     * @brief Put the data
     * @param key          [in] key of the meta object
     * @param objMeta      [out] the meta object obtained
     */
    // const char *key, mmc_buffer *buf, uint32_t flags
    Result Put(const std::string &key, mmc_buffer *buf, MmcMemObjMetaPtr &objMeta)
    {
        // 1. put objMeta into local map and get the target blob
        objMetaLookupMap_.Insert(key, objMeta);

        std::vector<MmcMemBlobPtr> blobs = objMeta->GetBlobs();
        if (blobs.size() != 1U) {
            MMC_LOG_ERROR("More than one copy of blob in this position!");
            return MMC_ERROR;
        }
        MmcMemBlobPtr blob = blobs[0];

        // 2. Write data into mem pool

        Result writeRet = WriteData(buf, blob->Gva(), blob->Size());

        // 3. notify meta service (meta那边缺一个handle)

        MmcLocalServiceDefaultPtr localServiceDefaultPtr =
            Convert<MmcLocalService, MmcLocalServiceDefault>(localService_);

        UpdateRequest reqUpdate;
        reqUpdate.actionResult_ = writeRet;
        reqUpdate.key_ = "test";
        reqUpdate.rank_ = 0;
        reqUpdate.mediaType_ = 0;
        UpdateResponse resp;
        int16_t respRet;
        localServiceDefaultPtr->SyncCallMeta(reqUpdate, resp, respRet, 30);

        /* blobs[0]->UpdateState(writeRet); // 4. Update local meta object state*/
    };

private:
    Result WriteData(mmc_buffer *buf, const uint64_t &gva, const uint32_t &size)
    {
        // apply api smem_bm_copy
        return MMC_WRITE_OK; /*return MMC_WRITE_OK or MMC_WRITE_FAIL*/
    };

    MmcLookupMap<std::string, MmcMemObjMetaPtr, NUM_BUCKETS_LOCAL> objMetaLookupMap_;
    void *handle_;
    MmcLocalServicePtr localService_;
};
using MmcLocalHandler = MmcRef<MmcLocalHandler>;

}  // namespace mmc
}  // namespace ock

#endif  // SMEM_MMC_LOCAL_MANAGER_H