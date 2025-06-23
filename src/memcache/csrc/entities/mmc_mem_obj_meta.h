/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2026. All rights reserved.
 */
#ifndef MEM_FABRIC_MMC_MEM_OBJ_META_H
#define MEM_FABRIC_MMC_MEM_OBJ_META_H

#include "mmc_mem_blob.h"
#include "mmc_ref.h"
#include "mmc_spinlock.h"
#include <vector>

namespace ock {
namespace mmc {

static const uint16_t MAX_NUM_BLOB_CHAINS = 5; // to make sure MmcMemObjMeta <= 64 bytes

struct MmcBlobInfo {
    const uint32_t rank_{0};      /* rank id of the blob located */
    const uint64_t gva_{0};       /* global virtual address */
    const uint16_t mediaType_{0}; /* media type where blob located */
    uint16_t prot_{0};            /* prot, i.e. access */

    MmcBlobInfo(uint32_t rank, uint64_t gva, uint16_t mediaType) : rank_(rank), gva_(gva), mediaType_(mediaType) {}
};

struct MmcMetaInfo : public MmcReferable {
    std::vector<MmcBlobInfo> blobInfoList_{};
    BlobState state_{BlobState::INIT}; /* state of the blob */
    uint32_t size_{0};                 /* byteSize of each blob */
    uint8_t priority_{0};              /* priority of the memory object, used for eviction */
    uint64_t lease_{0};                /* lease of the memory object */
    uint16_t prot_{0};                 /* prot of the mem object, i.e. accessibility */

    MmcMetaInfo(std::vector<MmcBlobInfo> &blobInfoList, BlobState state, uint32_t size, uint8_t priority,
                uint64_t lease, uint16_t prot)
        : blobInfoList_(blobInfoList), state_(state), size_(size), priority_(priority), lease_(lease), prot_(prot)
    {
    }
};

using MmcMetaInfoPtr = MmcRef<MmcMetaInfo>;

class MmcMemObjMeta : public MmcReferable {
public:
    MmcMemObjMeta() = default;
    ~MmcMemObjMeta() override = default;

    /**
     * @brief Add blob info into the mem object,
     * for replicated blob on different rank ids
     *
     * @param blob         [in] blob pointer to be added
     */
    void AddBlob(const MmcMemBlobPtr &blob);

    /**
     * @brief Remove a blob from the mem object
     *
     * @param blob         [in] blob pointer to be removed
     * @return 0 if removed
     */
    Result RemoveBlob(const MmcMemBlobPtr &blob);

    /**
     * @brief Extend the lease
     * @param ttl          [in] time of live in ms
     */
    void ExtendLease(uint64_t ttl);

    /**
     * @brief Check if the lease already expired
     * @return true if expired
     */
    bool IsLeaseExpired();

    /**
     * @brief Get the number of blobs
     * @return number of blobs
     */
    uint16_t NumBlobs();

    /**
     * @brief Get brief meta info based on state
     * @return brief meta info
     */
    MmcMetaInfoPtr GetMetaInfo(BlobState state = DEFAULT);

    /**
     * @brief Get blobs based on state
     * @return blobs with the given state （if DEFAULT，do not filter by state)
     */
    std::vector<MmcMemBlobPtr> GetBlobs(BlobState state = DEFAULT);

private:
    /* make sure the size of this class is 64 bytes */
    uint16_t prot_{0};                         /* prot of the mem object, i.e. accessibility */
    uint8_t priority_{0};                      /* priority of the memory object, used for eviction */
    uint8_t numBlobs_{0};                      /* number of blob that the memory object, i.e. replica count */
    MmcMemBlobPtr blobs_[MAX_NUM_BLOB_CHAINS]; /* pointers of blobs */
    uint64_t lease_{0};                        /* lease of the memory object */
    uint32_t size_{0};                         /* byteSize of each blob */
    Spinlock spinlock_;                        /* 4 bytes */
};

using MmcMemObjMetaPtr = MmcRef<MmcMemObjMeta>;

inline void MmcMemObjMeta::ExtendLease(const uint64_t ttl)
{
    using namespace std::chrono;
    const uint64_t nowMs = duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();

    std::lock_guard<Spinlock> guard(spinlock_);
    lease_ = std::max(lease_, nowMs + ttl);
}

inline bool MmcMemObjMeta::IsLeaseExpired()
{
    using namespace std::chrono;
    const uint64_t nowMs = duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();

    std::lock_guard<Spinlock> guard(spinlock_);
    return (lease_ < nowMs);
}

inline uint16_t MmcMemObjMeta::NumBlobs()
{
    std::lock_guard<Spinlock> guard(spinlock_);
    return numBlobs_;
}

} // namespace mmc
} // namespace ock

#endif // MEM_FABRIC_MMC_MEM_OBJ_META_H
