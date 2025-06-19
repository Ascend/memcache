#include "mmc_mem_obj_meta.h"
#include <algorithm>
#include <chrono>

namespace ock {
namespace mmc {

void MmcMemObjMeta::ExtendLease(uint64_t ttl)
{
    std::lock_guard guard(spinlock_);
    auto now = std::chrono::steady_clock::now();
    uint64_t nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    lease_ = std::max(lease_, nowMs + ttl);
}

bool MmcMemObjMeta::IsLeaseExpired()
{
    std::lock_guard guard(spinlock_);
    auto now = std::chrono::steady_clock::now();
    uint64_t nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    bool ret = lease_ < nowMs;
    return ret;
}

uint16_t MmcMemObjMeta::NumBlobs()
{
    std::lock_guard guard(spinlock_);
    auto ret = numBlobs_;
    return ret;
}

void MmcMemObjMeta::AddBlob(MmcMemBlobPtr blob)
{
    std::lock_guard guard(spinlock_);
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
        last->SetNext(blob);
        numBlobs_++;
    }
}

Result MmcMemObjMeta::RemoveBlob(MmcMemBlobPtr blob)
{
    if (blob == nullptr) {
        return MMC_INVALID_PARAM;
    }

    std::lock_guard guard(spinlock_);
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

} // namespace mmc
} // namespace ock