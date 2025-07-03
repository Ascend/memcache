/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2026. All rights reserved.
 */
#ifndef MEM_FABRIC_MMC_META_PROXY_IMPL_H
#define MEM_FABRIC_MMC_META_PROXY_IMPL_H

#include "mmc_meta_manager.h"
#include "mmc_meta_service.h"
#include "mmc_msg_client_meta.h"

namespace ock {
namespace mmc {

class MmcMetaMgrProxyImpl : public MmcMetaMgrProxy {
public:
    MmcMetaMgrProxyImpl(MmcMemPoolInitInfo mmcMemPoolInitInfo, uint64_t defaultTtl)
    {
        metaMangerPtr_ = MmcMakeRef<MmcMetaManager>(mmcMemPoolInitInfo, defaultTtl);
    };

    /**
     * @brief Alloc the global memeory space and create the meta object
     * @param key          [in] key of the meta object
     * @param metaInfo     [out] the meta object created
     */

    Result Alloc(const AllocRequest &allocReq, AllocResponse &allocResp)
    {
        MmcMemObjMetaPtr objMeta;
        Result ret = metaMangerPtr_->Alloc(allocReq.key_, allocReq.options_, objMeta);
        if (ret != MMC_OK) {
            MMC_LOG_ERROR("Meta Alloc Fail!");
            return MMC_ERROR;
        }
        allocResp.numBlobs_ = objMeta->NumBlobs();
        allocResp.prot_ = objMeta->Prot();
        allocResp.priority_ = objMeta->Priority();
        allocResp.lease_ = objMeta->Lease();

        std::vector<MmcMemBlobPtr> blobs = objMeta->GetBlobs();
        for (size_t i = 0; i < blobs.size(); i++) {
            allocResp.blobs_.push_back(blobs[i]->GetDesc());
        }

        // TODO: send a copy of the meta data to local service

        return MMC_OK;
    };

    Result UpdateState(const UpdateRequest &updateReq, UpdateResponse &upddateResp)
    {

        // const std::string &key, const MmcLocation &loc, const BlobActionResult &actRet
        MmcLocation loc{updateReq.rank_, updateReq.mediaType_};
        Result ret = metaMangerPtr_->UpdateState(updateReq.key_, loc, updateReq.actionResult_);
        upddateResp.ret_ = ret;
        if (ret != MMC_OK) {
            MMC_LOG_ERROR("Meta Update State Fail!");
            return MMC_ERROR;
        } else {
            return MMC_OK;
        }
    }

private:
    MmcMetaManagerPtr metaMangerPtr_;
};
using MmcMetaMgrProxyImplPtr = MmcRef<MmcMetaMgrProxyImpl>;

}  // namespace mmc
}  // namespace ock

#endif  // MEM_FABRIC_MMC_META_PROXY_IMPL_H
