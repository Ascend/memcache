#include "mmc_mem_obj_meta.h"
#include <algorithm>
#include <chrono>

namespace ock {
namespace mmc {

// TODO 添加blob时要检查size一致
void MmcMemObjMeta::AddBlob(const MmcMemBlobPtr &blob)
{
    std::lock_guard<Spinlock> guard(spinlock_);
    if (numBlobs_ < MAX_NUM_BLOB_CHAINS) {
        for (size_t i = 0; i < MAX_NUM_BLOB_CHAINS; ++i) {
            if (blobs_[i] == nullptr) {
                blobs_[i] = blob;
                numBlobs_++;
                break;
            }
        }
    } else {
        MmcMemBlobPtr last = blobs_[MAX_NUM_BLOB_CHAINS - 1];
        while (last->Next() != nullptr) {
            last = last->Next();
        }
        last->Next(blob);
        numBlobs_++;
    }
}

Result MmcMemObjMeta::RemoveBlob(const MmcMemBlobPtr &blob)
{
    if (blob == nullptr) {
        return MMC_INVALID_PARAM;
    }

    std::lock_guard<Spinlock> guard(spinlock_);
    for (size_t i = 0; i < MAX_NUM_BLOB_CHAINS; ++i) {
        if (blobs_[i] == blob) {
            blobs_[i] = nullptr;
            numBlobs_--;
            return MMC_OK;
        }
    }
    MmcMemBlobPtr last = blobs_[MAX_NUM_BLOB_CHAINS - 1];
    while (last != nullptr) {
        last = last->Next();
        if (last == blob) {
            last = nullptr;
            numBlobs_--;
            return MMC_OK;
        }
    }
    return MMC_ERROR;
}

MmcMetaInfoPtr MmcMemObjMeta::GetMetaInfo(BlobState state)
{
    std::lock_guard<Spinlock> guard(spinlock_);
    std::vector<MmcBlobInfo> blobInfoList;
    for (size_t i = 0; i < MAX_NUM_BLOB_CHAINS; ++i) {
        auto curBlob = blobs_[i];
        while (curBlob != nullptr && curBlob->State() == state) {
            blobInfoList.emplace_back(curBlob->Rank(), curBlob->Gva(), curBlob->MediaType());
            curBlob = curBlob->Next();
        }
    }
    if (blobInfoList.empty()) {
        return nullptr;
    } else {
        MmcMetaInfoPtr metaInfo = MmcMakeRef<MmcMetaInfo>(blobInfoList, state, size_, priority_, lease_, prot_);
        /*         metaInfo->blobInfoList_ = blobInfoList;
                metaInfo->state_ = state;
                metaInfo->size_ = size_;
                metaInfo->priority_ = priority_;
                metaInfo->prot_ = prot_; */
        return metaInfo;
    }
}

std::vector<MmcMemBlobPtr> MmcMemObjMeta::GetBlobs(BlobState state)
{
    std::vector<MmcMemBlobPtr> blobs;
    for (size_t i = 0; i < MAX_NUM_BLOB_CHAINS; ++i) {
        auto curBlob = blobs_[i];
        while (curBlob != nullptr && (state == DEFAULT || curBlob->State() == state)) {
            blobs.push_back(curBlob);
        }
    }
    return blobs;
}

} // namespace mmc
} // namespace ock