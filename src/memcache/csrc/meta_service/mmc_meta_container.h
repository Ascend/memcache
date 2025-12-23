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
#ifndef MF_HYBRID_MMC_META_CONTAINER_H
#define MF_HYBRID_MMC_META_CONTAINER_H

#include <memory>

#include "mmc_ref.h"

namespace ock {
namespace mmc {

template <typename Key, typename Value> class MmcMetaContainer : public MmcReferable {
public:
    virtual ~MmcMetaContainer() = default;
    virtual Result Insert(const Key &key, const Value &value) = 0;
    virtual Result Get(const Key &key, Value &value) = 0;
    virtual Result Erase(const Key &key) = 0;
    virtual Result Erase(const Key& key, Value& value) = 0;
    virtual Result EraseAll(std::function<void(const Key &, const Value &)> removeFunc) = 0;
    virtual void EraseIf(std::function<bool(const Key&, const Value&)> matchFunc) = 0;
    virtual void IterateIf(std::function<bool(const Key&, const Value&)> matchFunc,
                           std::map<Key, Value>& matchedValues) = 0;
    virtual void GetAllKeys(std::vector<Key>& keys) = 0;
    virtual Result Promote(const Key& key) = 0;
    virtual Result InsertLru(const Key& key, MediaType type) = 0;
    virtual void MultiLevelElimination(const uint16_t evictThresholdHigh, const uint16_t evictThresholdLow,
                                       const std::vector<MediaType>& needEvictList,
                                       const std::vector<uint16_t> &nowMemoryThresholds,
                                       std::function<EvictResult(const Key &, const Value &)> moveFunc) = 0;

    static MmcRef<MmcMetaContainer<Key, Value>> Create(std::function<MediaType(const Value &)> GetTypeFunc);
};
}  // namespace mmc
}  // namespace ock

#endif  // MF_HYBRID_MMC_META_CONTAINER_H