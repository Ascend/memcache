/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2026. All rights reserved.
 */
#ifndef MEMFABRIC_HYBRID_MMC_LOOKUP_MAP_H
#define MEMFABRIC_HYBRID_MMC_LOOKUP_MAP_H

#include "mmc_spinlock.h"
#include "mmc_types.h"
#include <unordered_map>

namespace ock {
namespace mmc {

template <typename Key, typename Value, uint32_t numBuckets>
class MmcLookupMap {
    static_assert(numBuckets > 0, "numBuckets must be positive");

public:
    /**
     * @brief Insert a value into this concurrent hash map
     *
     * @param key          [in] key of value to be inserted
     * @param value        [in] value to be inserted
     * @return 0 if insert successfully
     */
    Result Insert(const Key &key, const Value &value)
    {
        std::size_t index = GetIndex(key);
        std::lock_guard<std::mutex> guard(locks_[index]);
        auto ret = buckets_[index].emplace(key, value);
        if (ret.second) {
            return MMC_OK;
        }
        return MMC_ERROR;
    }

    /**
     * @brief Find the value by key from the concurrent hash map
     *
     * @param key          [in] key to be found
     * @param value        [in/out] value found
     * @return 0 if found
     */
    Result Find(const Key &key, Value &value)
    {
        std::size_t index = GetIndex(key);
        std::lock_guard<std::mutex> guard(locks_[index]);
        auto iter = buckets_[index].find(key);
        if (iter != buckets_[index].end()) {
            value = iter->second;
            return MMC_OK;
        }
        return MMC_ERROR;
    }

    /**
     * @brief Erase the value by key
     *
     * @param key          [in] the key of value to be erased
     * @return 0 if erase successfully
     */
    Result Erase(const Key &key)
    {
        std::size_t index = GetIndex(key);
        std::lock_guard<std::mutex> guard(locks_[index]);
        if (buckets_[index].erase(key) > 0) {
            return MMC_OK;
        }
        return MMC_ERROR;
    }

private:
    std::hash<Key> keyHasher;

    std::size_t GetIndex(Key key) const
    {
        return keyHasher(key) % numBuckets;
    }

private:
    std::unordered_map<Key, Value> buckets_[numBuckets];
    std::mutex locks_[numBuckets];
};
}  // namespace mmc
}  // namespace ock

#endif  // MEMFABRIC_HYBRID_MMC_LOOKUP_MAP_H