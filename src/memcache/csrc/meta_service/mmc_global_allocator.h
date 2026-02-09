/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 * MemCache_Hybrid is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
*/
#ifndef MEM_FABRIC_MMC_GLOBAL_ALLOCATOR_H
#define MEM_FABRIC_MMC_GLOBAL_ALLOCATOR_H

#include "nlohmann/json.hpp"

#include "mmc_blob_allocator.h"
#include "mmc_locality_strategy.h"
#include "mmc_read_write_lock.h"

namespace ock {
namespace mmc {

constexpr int LEVEL_BASE = 100;

using MmcMemPoolInitInfo = std::map<MmcLocation, MmcLocalMemlInitInfo>;

inline std::ostream &operator<<(std::ostream &os, const std::vector<MmcMemBlobPtr> &vec)
{
    os << ", Rank[";
    for (size_t i = 0; i < vec.size(); ++i) {
        if (i > 0) {
            os << ", ";
        }
        os << vec[i]->Rank();
    }
    os << "]";
    return os;
}

class MmcGlobalAllocator : public MmcReferable {
public:
    MmcGlobalAllocator() = default;

    /**
     * @brief
     * blob分配策略实现
     * @param allocOpt
     * 1. preferredRank为空时（实际上至少有一个local rank），numBlobs可以是 <= MAX_BLOB_COPIES 的任意值
     * 2. preferredRank非空时，rank不能重复，且rank个数 <= numBlobs
     * 3. preferredRank大于1的时候，必定是强制分配场景，例如：
     *    replicaNum=4，preferredLocalServiceIDs=[0,1]场景，需要强制在0,1上分配两个副本，剩下2个副本按默认分配策略处理；且需要排除已分配过的rank列表
     * @param blobs
     * @return Result
     */
    Result Alloc(const AllocOptions &allocOpt, std::vector<MmcMemBlobPtr> &blobs)
    {
        std::unordered_set<uint32_t> uniqueRanks(allocOpt.preferredRank_.begin(), allocOpt.preferredRank_.end());
        if (allocOpt.numBlobs_ < allocOpt.preferredRank_.size() ||
            uniqueRanks.size() != allocOpt.preferredRank_.size()) {
            MMC_LOG_ERROR("Invalid alloc option: " << allocOpt);
            return MMC_ERROR;
        }

        AllocOptions allocReq = allocOpt;
        if (allocReq.mediaType_ == MEDIA_NONE) {
            allocReq.mediaType_ = static_cast<uint16_t>(GetTopLayerMediumType());
        }

        std::unordered_set<uint32_t> excludeRanks;
        globalAllocLock_.LockRead();
        // size=1 不一定是强制分配
        if (allocReq.preferredRank_.size() <= 1u) {
            auto ret = InnerAlloc(allocReq, blobs, excludeRanks);
            globalAllocLock_.UnlockRead();
            if (ret != MMC_OK) {
                Free(blobs);
                MMC_LOG_ERROR("Simple alloc failed, ret: " << ret << ", allocReq: " << allocReq);
            }
            return ret;
        }
        Result result{};
        for (const uint32_t rank : allocReq.preferredRank_) {
            AllocOptions tmpAllocReq = allocReq;
            tmpAllocReq.preferredRank_ = {rank};
            tmpAllocReq.numBlobs_ = 1u;
            tmpAllocReq.flags_ = static_cast<uint32_t>(ALLOC_FORCE_BY_RANK);
            if ((result = InnerAlloc(tmpAllocReq, blobs, excludeRanks)) != MMC_OK) {
                MMC_LOG_ERROR("Force alloc failed, result: " << result << ", allocReq: " << tmpAllocReq);
                globalAllocLock_.UnlockRead();
                Free(blobs);
                return result;
            }
        }
        excludeRanks.insert(allocReq.preferredRank_.begin(), allocReq.preferredRank_.end());
        if (allocReq.numBlobs_ - allocReq.preferredRank_.size() > 0) {
            AllocOptions tmpAllocReq = allocReq;
            tmpAllocReq.numBlobs_ = allocReq.numBlobs_ - allocReq.preferredRank_.size();
            tmpAllocReq.flags_ = static_cast<uint32_t>(ALLOC_RANDOM);
            if ((result = InnerAlloc(tmpAllocReq, blobs, excludeRanks)) != MMC_OK) {
                MMC_LOG_ERROR("Arrange alloc failed, result: " << result << ", allocReq: " << tmpAllocReq);
                globalAllocLock_.UnlockRead();
                Free(blobs);
                return result;
            }
        }
        globalAllocLock_.UnlockRead();
        if (blobs.size() != allocReq.numBlobs_) {
            MMC_LOG_ERROR("More alloc failed, " << allocReq << blobs);
            Free(blobs);
            return MMC_ERROR;
        }
        MMC_LOG_DEBUG("More alloc, " << allocReq << blobs);
        return MMC_OK;
    }

    void Free(std::vector<MmcMemBlobPtr> &blobs) noexcept
    {
        for (const auto &blob : blobs) {
            Free(blob);
        }
        blobs.clear();
    }

    Result Free(const MmcMemBlobPtr &blob)
    {
        if (blob == nullptr) {
            MMC_LOG_ERROR("Free blob failed, blob is nullptr");
            return MMC_INVALID_PARAM;
        }
        globalAllocLock_.LockRead();
        const MmcLocation location{blob->Rank(), static_cast<MediaType>(blob->Type())};
        const auto iter = allocators_.find(location);
        if (iter == allocators_.end()) {
            globalAllocLock_.UnlockRead();
            MMC_LOG_ERROR("Free blob failed, location not found, rank: " << location.rank_
                                                                         << ", mediaType: " << location.mediaType_);
            return MMC_INVALID_PARAM;
        }

        const auto &allocator = iter->second;
        if (allocator == nullptr) {
            globalAllocLock_.UnlockRead();
            MMC_LOG_ERROR("Free blob failed, allocator is nullptr");
            return MMC_ERROR;
        }
        Result ret = allocator->Release(blob);
        globalAllocLock_.UnlockRead();
        if (ret != MMC_OK) {
            MMC_LOG_ERROR("Free blob failed, blob: " << blob->GetDesc());
        }
        return ret;
    };

    Result Mount(const MmcLocation &loc, const MmcLocalMemlInitInfo &localMemInitInfo)
    {
        globalAllocLock_.LockWrite();
        auto iter = allocators_.find(loc);
        if (iter != allocators_.end()) {
            globalAllocLock_.UnlockWrite();
            MMC_LOG_INFO("not need mount at the existing position, loc: " << loc);
            return MMC_OK;
        }

        allocators_[loc] = MmcMakeRef<MmcBlobAllocator>(loc.rank_, loc.mediaType_, localMemInitInfo.bmAddr_,
                                                        localMemInitInfo.capacity_);

        MMC_LOG_INFO("Mount bm on " << loc << ", capacity:" << localMemInitInfo.capacity_ << "  successfully");
        globalAllocLock_.UnlockWrite();
        return MMC_OK;
    }

    Result Start(const MmcLocation &loc)
    {
        globalAllocLock_.LockRead();
        const auto iter = allocators_.find(loc);
        if (iter == allocators_.end()) {
            globalAllocLock_.UnlockRead();
            MMC_LOG_ERROR("location not found, rank: " << loc.rank_ << ", mediaType: " << loc.mediaType_);
            return MMC_INVALID_PARAM;
        }

        const auto &allocator = iter->second;
        if (allocator == nullptr) {
            globalAllocLock_.UnlockRead();
            MMC_LOG_ERROR("Start failed, allocator is nullptr");
            return MMC_ERROR;
        }
        allocator->Start();
        globalAllocLock_.UnlockRead();
        return MMC_OK;
    }

    Result Stop(const MmcLocation &loc)
    {
        globalAllocLock_.LockRead();
        const auto iter = allocators_.find(loc);
        if (iter == allocators_.end()) {
            globalAllocLock_.UnlockRead();
            MMC_LOG_ERROR("location not found, rank: " << loc.rank_ << ", mediaType: " << loc.mediaType_);
            return MMC_INVALID_PARAM;
        }

        const auto &allocator = iter->second;
        if (allocator == nullptr) {
            globalAllocLock_.UnlockRead();
            MMC_LOG_ERROR("Stop failed, allocator is nullptr");
            return MMC_ERROR;
        }
        allocator->Stop();
        MMC_LOG_INFO("Stop one bm successfully, loc: " << loc);
        globalAllocLock_.UnlockRead();
        return MMC_OK;
    }

    Result Unmount(const MmcLocation &loc)
    {
        globalAllocLock_.LockWrite();
        auto iter = allocators_.find(loc);
        if (iter == allocators_.end()) {
            globalAllocLock_.UnlockWrite();
            MMC_LOG_ERROR("Cannot find the given {rank:" << loc.rank_ << ", type:" << loc.mediaType_
                                                         << "} in the mem pool");
            return MMC_INVALID_PARAM;
        }
        if (iter->second == nullptr) {
            globalAllocLock_.UnlockWrite();
            MMC_LOG_ERROR("Unmount failed, allocator is nullptr");
            return MMC_ERROR;
        }
        if (!iter->second->CanUnmount()) {
            globalAllocLock_.UnlockWrite();
            MMC_LOG_ERROR("Cannot unmount the given {rank:" << loc.rank_ << ", type:" << loc.mediaType_
                                                            << "}  in the mem pool, space is in use");
            return MMC_INVALID_PARAM;
        }
        allocators_.erase(iter);
        MMC_LOG_DEBUG("Unmount one bm successfully, loc: " << loc);
        globalAllocLock_.UnlockWrite();
        return MMC_OK;
    }

    Result BuildFromBlobs(const MmcLocation &location, std::map<std::string, MmcMemBlobDesc> &blobMap)
    {
        globalAllocLock_.LockRead();
        const auto iter = allocators_.find(location);
        if (iter == allocators_.end()) {
            globalAllocLock_.UnlockRead();
            MMC_LOG_ERROR("Build from blobs failed, location not found, rank: " << location.rank_ << ", mediaType: "
                                                                                << location.mediaType_);
            return MMC_INVALID_PARAM;
        }

        const auto &allocator = iter->second;
        if (allocator == nullptr) {
            globalAllocLock_.UnlockRead();
            MMC_LOG_ERROR("BuildFromBlobs failed, allocator is nullptr");
            return MMC_ERROR;
        }
        Result ret = allocator->BuildFromBlobs(blobMap);
        globalAllocLock_.UnlockRead();
        return ret;
    }

    void GetUsedInfo(uint64_t (&totalSize)[MEDIA_NONE], uint64_t (&usedSize)[MEDIA_NONE])
    {
        globalAllocLock_.LockRead();
        for (auto &allocator : allocators_) {
            auto result = allocator.second->GetUsageInfo();
            totalSize[allocator.first.mediaType_] += result.first;
            usedSize[allocator.first.mediaType_] += result.second;
        }
        globalAllocLock_.UnlockRead();
    }

    uint64_t GetFreeSpace(MediaType type)
    {
        if (type == MEDIA_NONE) {
            return 0;
        }
        uint64_t totalSize[MEDIA_NONE] = {0};
        uint64_t usedSize[MEDIA_NONE] = {0};
        GetUsedInfo(totalSize, usedSize);
        return totalSize[type] - usedSize[type];
    }

    std::vector<MediaType> GetNeedEvictList(const uint64_t level, std::vector<uint16_t> &nowMemoryThresholds)
    {
        std::vector<MediaType> results;
        uint64_t totalSize[MEDIA_NONE] = {0};
        uint64_t usedSize[MEDIA_NONE] = {0};
        GetUsedInfo(totalSize, usedSize);
        // 从下层向上层遍历，先淘汰下层，后淘汰上层
        for (int i = MEDIA_NONE - 1; i >= 0; i--) {
            if (usedSize[i] > std::numeric_limits<uint64_t>::max() / LEVEL_BASE ||
                (level != 0 && (totalSize[i] > std::numeric_limits<uint64_t>::max() / level))) {
                MMC_LOG_ERROR("overflow: usedSize: " << usedSize[i] << ", LEVEL_BASE: " << LEVEL_BASE
                                                     << ", totalSize: " << totalSize[i] << ", level: " << level);
                continue;
            }
            if (usedSize[i] * LEVEL_BASE > totalSize[i] * level) {
                uint16_t nowMemoryThreshold = usedSize[i] * LEVEL_BASE / totalSize[i];
                results.push_back(static_cast<MediaType>(i));
                nowMemoryThresholds.push_back(nowMemoryThreshold);
                MMC_LOG_DEBUG("Medium " << static_cast<MediaType>(i) << " need evict, usedSize: " << usedSize[i]
                                        << ", totalSize: " << totalSize[i]);
            }
        }
        return results;
    }

    nlohmann::json GetAllSegmentInfo()
    {
        globalAllocLock_.LockRead();
        nlohmann::json segmentArray = nlohmann::json::array();
        for (const auto &[fst, snd] : allocators_) {
            const auto &allocator = snd;
            if (allocator == nullptr) {
                MMC_LOG_ERROR("allocator is nullptr");
                continue;
            }
            segmentArray.push_back(allocator->GetInfo());
        }
        globalAllocLock_.UnlockRead();
        return segmentArray;
    }

private:
    Result InnerAlloc(const AllocOptions &allocReq, std::vector<MmcMemBlobPtr> &blobs,
                      std::unordered_set<uint32_t> &excludeRanks) const
    {
        if (allocators_.empty()) {
            MMC_LOG_ERROR("Alloc allocators_ is empty");
            return MMC_ERROR;
        }
        Result ret;
        switch (allocReq.flags_) {
            case ALLOC_ARRANGE:
                ret = MmcLocalityStrategy::ArrangeLocality(allocators_, allocReq, blobs, excludeRanks);
                break;

            case ALLOC_FORCE_BY_RANK:
                ret = MmcLocalityStrategy::ForceAssign(allocators_, allocReq, blobs);
                break;

            case ALLOC_RANDOM:
                ret = MmcLocalityStrategy::RandomAssign(allocators_, allocReq, blobs, excludeRanks);
                break;

            default:
                ret = MmcLocalityStrategy::ArrangeLocality(allocators_, allocReq, blobs, excludeRanks);
                break;
        }
        return ret;
    }

    MediaType GetTopLayerMediumType()
    {
        MediaType result = MEDIA_NONE;
        globalAllocLock_.LockRead();
        for (const auto &[location, allocator] : allocators_) {
            result = location.mediaType_;
            break;
        }
        globalAllocLock_.UnlockRead();
        return result;
    }

    MmcAllocators allocators_;
    ReadWriteLock globalAllocLock_;
};

using MmcGlobalAllocatorPtr = MmcRef<MmcGlobalAllocator>;

} // namespace mmc
} // namespace ock

#endif // MEM_FABRIC_MMC_GLOBAL_ALLOCATOR_H