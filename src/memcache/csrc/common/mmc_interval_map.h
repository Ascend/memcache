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
#ifndef __MMC_INTERVAL_MAP_H__
#define __MMC_INTERVAL_MAP_H__

#include <map>
#include <optional>
#include <stdexcept>
#include <cstdint>
#include <string>
#include <iostream>

template<typename V = std::string, typename ValueEqual = std::equal_to<V>>
class MmcIntervalMap {
public:
    explicit MmcIntervalMap(ValueEqual eq = ValueEqual{}) : equalFn_(std::move(eq)) {}

    // 尝试添加 [start, start + size) → value
    // 返回是否成功添加（false 表示有重叠被拒绝）
    bool Add(uint64_t start, uint64_t size, V value)
    {
        if (size == 0) {
            return false;
        }
        uint64_t end = start + size;
        if (end < start) {
            return false;
        }

        if (HasOverlap(start, end)) {
            return false;
        }

        intervals_[start] = {end, std::move(value)};
        return true;
    }

    V* Query(uint64_t addr)
    {
        // 找到第一个 start > addr 的区间 → 前一个可能是包含 addr 的
        auto it = intervals_.upper_bound(addr);
        if (it == intervals_.begin()) {
            return nullptr;
        }
        --it;

        if (it->first <= addr && addr < it->second.first) {
            return &(it->second.second);
        }

        return nullptr;
    }

    // 范围查询：整个 [addr, addr+size) 是否被同一个值完全覆盖
    V* Query(uint64_t addr, uint64_t size)
    {
        if (size == 0) {
            return nullptr;
        }

        uint64_t end = addr + size;
        if (end < addr) {
            return nullptr;
        }

        // 找到第一个可能覆盖 addr 的区间（≤ addr 的最大 start）
        auto it = intervals_.upper_bound(addr);
        if (it == intervals_.begin()) {
            return nullptr; // 没有任何区间在 addr 之前
        }
        --it;

        // 如果当前区间甚至不覆盖 addr → 失败
        if (!(it->first <= addr && addr < it->second.first)) {
            return nullptr;
        }

        // 记录第一个区间的 value，作为基准
        V* common_value = &(it->second.second);

        // 从 addr 开始检查，直到覆盖到 end
        uint64_t covered_up_to = addr;

        while (covered_up_to < end) {
            // 当前区间必须包含 covered_up_to
            if (!(it->first <= covered_up_to && covered_up_to < it->second.first)) {
                return nullptr; // 有空洞
            }

            // value 必须相同
            if (!equalFn_(it->second.second, *common_value)) {
                return nullptr;
            }

            // 前进到当前区间的结束位置
            covered_up_to = std::max(covered_up_to, it->second.first);

            // 移动到下一个区间
            ++it;

            // 如果已经到达 map 末尾，但还没覆盖完 → 有空洞
            if (it == intervals_.end()) {
                return (covered_up_to >= end) ? common_value : nullptr;
            }
        }

        // 如果走完循环，covered_up_to >= end，且所有 value 相同
        return common_value;
    }

    // 1. 完整区间刪除：必須完全匹配某个已存在的 [start, start+size)
    // 若不存在完全相同的区间，或只有部分重叠 → 返回false
    bool Remove(uint64_t start, uint64_t size)
    {
        if (size == 0) {
            return false;
        }
        uint64_t end = start + size;
        if (end <= start) {
            return false;
        }

        auto it = intervals_.find(start);
        if (it == intervals_.end()) {
            return false;
        }

        if (it->second.first != end) {
            return false;
        }

        intervals_.erase(it);
        return true;
    }

    // 单点刪除：刪除唯一包含 addr 的区间
    // 若 addr 不属于任何区间，或 addr 同時落在多个区间（理论不可能）返回 false
    bool RemoveAt(uint64_t addr)
    {
        auto it = intervals_.upper_bound(addr);
        if (it == intervals_.begin()) {
            return false;
        }
        --it;

        if (!(it->first <= addr && addr < it->second.first)) {
            return false;
        }

        intervals_.erase(it);
        return true;
    }

    size_t Size() const
    {
        return intervals_.size();
    }

    bool Empty() const
    {
        return intervals_.empty();
    }

    void Clear()
    {
        intervals_.clear();
    }

private:
    // key = start, value = {end, mapped_value}
    std::map<uint64_t, std::pair<uint64_t, V>> intervals_;
    ValueEqual equalFn_;

    // 检查新区间 [start, end) 是否与已有区间重叠
    bool HasOverlap(uint64_t start, uint64_t end) const
    {
        // 找到第一个 >= start 的区间
        auto it = intervals_.lower_bound(start);
        // 检查前一个区间是否覆盖到 start
        if (it != intervals_.begin()) {
            auto prev = std::prev(it);
            if (prev->second.first > start) {
                return true; // 前一个区间右端 > 新区间起点 → 重叠
            }
        }

        // 检查后续区间是否与新区间有交集
        while (it != intervals_.end() && it->first < end) {
            if (it->first < end && start < it->second.first) {
                return true;
            }
            ++it;
        }

        return false;
    }
};

#endif