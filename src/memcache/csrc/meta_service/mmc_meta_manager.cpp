/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2026. All rights reserved.
 */
#include "mmc_meta_manager.h"

namespace ock {
namespace mmc {

Result MmcMetaManger::Get(const std::string &key, MmcMemObjMetaPtr &objMeta)
{
    auto ret = objMetaLookupMap_.Find(key, objMeta);
    if (ret == MMC_OK) {
        objMeta->ExtendLease(1000U);
    }
    return ret;
}

Result MmcMetaManger::Alloc(const std::string &key, const AllocProperty &allocReq, MmcMemObjMetaPtr &objMeta)
{
    objMeta = MmcMakeRef<MmcMemObjMeta>();
    std::vector<MmcMemBlobPtr> blobs;

    Result ret = globalAllocator_->Alloc(allocReq, blobs);
    if (ret == MMC_ERROR) {
        return MMC_ERROR;
    }

    for (auto &blob : blobs) {
        objMeta->AddBlob(blob);
    }
    objMetaLookupMap_.Insert(key, objMeta);
    objMeta->ExtendLease(1000U);
    return MMC_OK;
}

Result MmcMetaManger::GetBlobs(const std::string &key, const MmcBlobFilterPtr &filter,
                               std::vector<MmcMemBlobPtr> &blobs)
{
    MmcMemObjMetaPtr objMeta;
    Result ret = objMetaLookupMap_.Find(key, objMeta);
    blobs = objMeta->GetBlobs(filter);
    if (ret == MMC_OK && !blobs.empty()) {
        objMeta->ExtendLease(1000U);
        return MMC_OK;
    } else {
        return MMC_ERROR;
    }
}

}  // namespace mmc
}  // namespace ock