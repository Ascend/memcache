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
#include "mmc_meta_lease_manager.h"

#include <cstdint>
#include <limits>
#include <thread>

#include "mmc_montotonic.h"
#include "mmc_types.h"
#include "mmc_define.h"

namespace ock {
namespace mmc {
Result MmcMetaLeaseManager::Add(uint32_t id, uint32_t requestId, uint64_t ttl)
{
    MMC_LOG_DEBUG("MmcMetaLeaseManager ADD " << " id " << id << " requestId " << requestId << " ttl " << ttl);
    const uint64_t nowMs = ock::dagger::Monotonic::TimeUs() / 1000U;
    if (ttl > std::numeric_limits<uint64_t>::max() - nowMs) {
        return MMC_INVALID_PARAM;
    }
    lease_ = std::max(lease_, nowMs + ttl);
    useClient.insert(GenerateClientId(id, requestId));
    return MMC_OK;
}

Result MmcMetaLeaseManager::Remove(uint32_t id, uint32_t requestId)
{
    MMC_LOG_DEBUG("MmcMetaLeaseManager Remove id " << id << " requestId " << requestId);
    useClient.erase(GenerateClientId(id, requestId));
    return MMC_OK;
}

Result MmcMetaLeaseManager::Extend(uint64_t ttl)
{
    MMC_LOG_DEBUG("MmcMetaLeaseManager Extend " << " ttl " << ttl);
    const uint64_t nowMs = ock::dagger::Monotonic::TimeUs() / 1000U;
    if (ttl > std::numeric_limits<uint64_t>::max() - nowMs) {
        return MMC_INVALID_PARAM;
    }
    lease_ = std::max(lease_, nowMs + ttl);
    return MMC_OK;
}

void MmcMetaLeaseManager::Wait()
{
    while (!useClient.empty()) {
        if ((ock::dagger::Monotonic::TimeUs() / 1000ULL) >= lease_) {
            return;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(MMC_DATA_TTL_MS / 10ULL));
    }
}
} // namespace mmc
} // namespace ock