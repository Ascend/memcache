/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2026. All rights reserved.
 */
#include "mmc_meta_manager.h"

namespace ock {
namespace mmc {

Result MmcMetaManager::Get(const std::string &key, MmcMemObjMetaPtr &objMeta)
{
    auto ret = objMetaLookupMap_.Find(key, objMeta);
    if (ret == MMC_OK) {
        objMeta->ExtendLease(1000U);
    }
    return ret;
}

// TODO: Check threshold， if above， try to remove to free space
// TODO: 检测不能是相同的key
Result MmcMetaManager::Alloc(const std::string &key, const AllocOptions &allocOpt, MmcMemObjMetaPtr &objMeta)
{
    objMeta = MmcMakeRef<MmcMemObjMeta>();
    std::vector<MmcMemBlobPtr> blobs;

    Result ret = globalAllocator_->Alloc(allocOpt, blobs);
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

Result MmcMetaManager::GetBlobs(const std::string &key, const MmcBlobFilterPtr &filter,
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

Result MmcMetaManager::UpdateState(const std::string &key, const MmcLocation &loc, const BlobActionResult &actRet)
{
    std::vector<MmcMemBlobPtr> blobs;
    MmcBlobFilterPtr filter = MmcMakeRef<MmcBlobFilter>(loc.rank_, loc.mediaType_, NONE);
    if (GetBlobs(key, filter, blobs) != MMC_OK) {
        MMC_LOG_ERROR("UpdateState: Cannot find blobs!");
        return MMC_UNMATCHED_KEY;
    }
    if (blobs.size() != 1) {
        MMC_LOG_ERROR("UpdateState: More than one blob in one position!");
        return MMC_ERROR;
    }
    Result ret = blobs[0]->UpdateState(actRet);
    if (ret != MMC_OK) {
        MMC_LOG_WARN("UpdateState Fail!");
        return ret;
    }
    return MMC_OK;
}

Result MmcMetaManager::Remove(const std::string &key)
{
    MmcMemObjMetaPtr objMeta;
    auto ret = objMetaLookupMap_.Find(key, objMeta);
    if (ret != MMC_OK) {
        return MMC_UNMATCHED_KEY;
    }
    if (objMeta->IsLeaseExpired()) {
        std::vector<MmcMemBlobPtr> blobs = objMeta->GetBlobs();
        for (size_t i = 0; i < blobs.size(); i++) {
            Result ret = globalAllocator_->Free(blobs[i]);
            if (ret != MMC_OK) {
                MMC_LOG_ERROR("Error in free blobs!");
                return ret;
            }
        }
        ret = objMeta->RemoveBlobs();
        objMetaLookupMap_.Erase(key);
        objMeta = nullptr;
        return MMC_OK;
    } else {
        return MMC_LEASE_NOT_EXPIRED;
    }
}

}  // namespace mmc
}  // namespace ock