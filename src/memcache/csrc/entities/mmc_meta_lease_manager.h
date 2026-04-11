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

#ifndef MF_HYBRID_MMC_META_LEASE_MANAGER_H
#define MF_HYBRID_MMC_META_LEASE_MANAGER_H

#include <unordered_set>

#include "mmc_logger.h"
#include "mmc_montotonic.h"
#include "mmc_ref.h"
#include "mmc_types.h"

namespace ock {
namespace mmc {

constexpr int RANK_ID_BIT_SHIFT = 32;
class MmcMetaLeaseManager : public MmcReferable {
public:
    Result Add(uint32_t id, uint32_t requestId, uint64_t ttl);
    Result Remove(uint32_t id, uint32_t requestId);
    Result Extend(uint64_t ttl);
    void Wait();
    inline uint32_t UseCount();
    inline uint64_t GenerateClientId(uint32_t rankId, uint32_t requestId);
    inline uint32_t RankId(uint64_t clientId);
    inline uint32_t RequestId(uint64_t clientId);

    friend std::ostream &operator<<(std::ostream &os, const MmcMetaLeaseManager &leaseMgr)
    {
        os << "lease={" << leaseMgr.lease_ << ",client:";
        for (const auto &c : leaseMgr.useClient) {
            os << c << ",";
        }
        os << "}";
        return os;
    }

private:
    uint64_t lease_{0}; /* lease of the memory object */
    std::unordered_set<uint64_t> useClient;
};

using MmcMetaLeaseManagerPtr = MmcRef<MmcMetaLeaseManager>;

inline uint32_t MmcMetaLeaseManager::UseCount()
{
    return useClient.size();
}

uint64_t MmcMetaLeaseManager::GenerateClientId(uint32_t rankId, uint32_t requestId)
{
    return (static_cast<uint64_t>(rankId) << RANK_ID_BIT_SHIFT) | requestId;
}
uint32_t MmcMetaLeaseManager::RankId(uint64_t clientId)
{
    return static_cast<uint32_t>(clientId >> RANK_ID_BIT_SHIFT); // 高32位
}
uint32_t MmcMetaLeaseManager::RequestId(uint64_t clientId)
{
    return static_cast<uint32_t>(clientId & 0xFFFFFFFF); // 低32位
}
} // namespace mmc
} // namespace ock

#endif // MF_HYBRID_MMC_META_LEASE_MANAGER_H