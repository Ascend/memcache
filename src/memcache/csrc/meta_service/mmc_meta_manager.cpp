/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2026. All rights reserved.
 */
#include "mmc_meta_manager.h"

namespace ock {
namespace mmc {

Result MmcMetaManger::GetReadyBlobs(const std::string &key, MmcMetaInfoPtr &metaInfo)
{
    auto tmpMetaObj = MmcMakeRef<MmcMemObjMeta>();
    Result ret = objMetaLookupMap_.Find(key, tmpMetaObj);
    if (ret != MMC_OK) {
        return MMC_ERROR;
    }
    metaInfo = tmpMetaObj->GetMetaInfo(DATA_READY);
    if (metaInfo != nullptr) {
        tmpMetaObj->ExtendLease(defaultTtl_);
        return MMC_OK;
    } else {
        return MMC_ERROR;
    }
}

Result MmcMetaManger::Alloc(const std::string &key, const AllocRequest &allocReq, MmcMemObjMetaPtr &objMeta)
{
    std::vector<MmcMemBlobPtr> blobs;
    Result ret = globalAllocator_->Alloc(allocReq, blobs);
    if (ret == MMC_ERROR) {
        return MMC_ALLOC_FAIL;
    }
    for (auto &blob : blobs) {
        objMeta->AddBlob(blob);
    }
    objMetaLookupMap_.Insert(key, objMeta);
    return MMC_ALLOC_OK;
}
Result MmcMetaManger::FreeStart(std::string key, uint32_t flags)
{
    MmcMemObjMetaPtr memObj;
    Result ret = objMetaLookupMap_.Find(key, memObj);
    if (!ret || !memObj->IsLeaseExpired()) {
        return MMC_ERROR;
    }
    std::vector<MmcMemBlobPtr> blobs = memObj->GetBlobs(DEFAULT);
    for (MmcMemBlobPtr blob : blobs) {
        // TODO: 发消息让local侧进行删除
        // TODO: 我感觉meta侧，WRITING和COPYING状态既然可能有延迟，就不要了
        blob->UpdateState(REMOVING, DEFAULT);
    }
    return MMC_OK;
};

} // namespace mmc
} // namespace ock