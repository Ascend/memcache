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

#ifndef MF_HYBRID_MMC_BLOB_COMMON_H
#define MF_HYBRID_MMC_BLOB_COMMON_H
#include <iostream>
#include "mmc_ref.h"
namespace ock {
namespace mmc {
class MmcMemBlob;
using MmcMemBlobPtr = MmcRef<MmcMemBlob>;

struct MmcMemBlobDesc {
    uint64_t size_ = 0;               /* data size of the blob */
    uint64_t gva_ = UINT64_MAX;       /* global virtual address */
    uint32_t rank_ = UINT32_MAX;      /* rank id of the blob located */
    uint16_t mediaType_ = UINT16_MAX; /* media type where blob located */

    MmcMemBlobDesc() = default;
    MmcMemBlobDesc(const uint32_t &rank, const uint64_t &gva, const uint64_t &size, const uint16_t &mediaType)
        : rank_(rank), size_(size), gva_(gva), mediaType_(mediaType)
    {
    }

    friend bool operator==(const MmcMemBlobDesc &lhs, const MmcMemBlobDesc &rhs)
    {
        return lhs.size_ == rhs.size_ && lhs.gva_ == rhs.gva_ && lhs.rank_ == rhs.rank_ &&
               lhs.mediaType_ == rhs.mediaType_;
    }

    friend bool operator!=(const MmcMemBlobDesc &lhs, const MmcMemBlobDesc &rhs)
    {
        return !(lhs == rhs);
    }

    friend std::ostream& operator<<(std::ostream& os, const MmcMemBlobDesc& blob)
    {
        os << "blob{size=" << blob.size_ << ",gva=" << blob.gva_ << ",rank=" << blob.rank_
           << ",media=" << blob.mediaType_ << "}";
        return os;
    }
};
}
}
#endif // MF_HYBRID_MMC_BLOB_COMMON_H