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
#ifndef MMC_NET_LINK_H
#define MMC_NET_LINK_H

#include "mmc_common_includes.h"

namespace ock {
namespace mmc {
/**
 * Concurrent map link map
 */
template<typename LINK_PTR>
class NetLinkMap final : public MmcReferable {
public:
    /**
     * @brief Add link into this map
     *
     * @param id               [in] peer service id
     * @param link             [in] link
     * @return true if found, and hold the reference of link
     */
    bool Find(const uint32_t id, LINK_PTR &link)
    {
        auto bucket = id % gSubMapCount;
        {
            std::lock_guard<std::mutex> guard(mapMutex_[bucket]);
            auto iter = linkMaps_[bucket].find(id);
            if (iter != linkMaps_[bucket].end()) {
                link = iter->second;
                return true;
            }
        }
        return false;
    }

    /**
     * @brief Add a link into this map
     *
     * @param id             [in] peer service id
     * @param link           [in] link
     * @return true if added
     */
    bool Add(const uint32_t id, const LINK_PTR &link)
    {
        auto bucket = id % gSubMapCount;
        {
            std::lock_guard<std::mutex> guard(mapMutex_[bucket]);
            return linkMaps_[bucket].emplace(id, link).second;
        }
    }

    /**
     * @brief Remove a link from this map
     *
     * @param id             [in] peer service id
     * @return true if removed
     */
    bool Remove(const uint32_t id)
    {
        auto bucket = id % gSubMapCount;
        {
            std::lock_guard<std::mutex> guard(mapMutex_[bucket]);
            return linkMaps_[bucket].erase(id) != 0;
        }
    }

    /**
     * @brief Remove all links in this map
     */
    void Clear()
    {
        for (uint32_t bucket = 0; bucket < gSubMapCount; bucket++) {
            std::lock_guard<std::mutex> guard(mapMutex_[bucket]);
            linkMaps_[bucket].clear();
        }
    }

private:
    static constexpr uint32_t gSubMapCount = 7L;

private:
    std::mutex mapMutex_[gSubMapCount];
    std::map<uint32_t, LINK_PTR> linkMaps_[gSubMapCount];
};
} // namespace mmc
} // namespace ock

#endif // MMC_NET_LINK_H