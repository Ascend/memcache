/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 */

#ifndef HTRACER_INFO_H
#define HTRACER_INFO_H

#include <atomic>
#include <string>
#include "htracer_utils.h"

namespace ock {
namespace htracer {
class HtracerInfo {
public:
    __always_inline void DelayBegin(std::string &tpName)
    {
        bool expectval = false;
        if (nameValid.compare_exchange_weak(expectval, true)) {
            name = tpName;
        }
        begin.fetch_add(1u, std::memory_order_relaxed);
    }

    __always_inline void DelayEnd(uint64_t diff, int32_t retCode)
    {
        if (retCode != 0) {
            badEnd.fetch_add(1u, std::memory_order_relaxed);
            return;
        }
        if (diff < min) {
            min.store(diff, std::memory_order_relaxed);
        }

        if (diff < previousMin) {
            previousMin.store(diff, std::memory_order_relaxed);
        }

        if (diff > max) {
            max.store(diff, std::memory_order_relaxed);
        }

        if (diff > previousMax) {
            previousMax.store(diff, std::memory_order_relaxed);
        }

        total.fetch_add(diff, std::memory_order_relaxed);
        goodEnd.fetch_add(1u, std::memory_order_relaxed);
    }

    __always_inline void Reset()
    {
        begin = 0;
        goodEnd = 0;
        badEnd = 0;
        min = UINT64_MAX;
        max = 0;
        total = 0;
    }

    __always_inline const std::string &GetName() const
    {
        return name;
    }

    __always_inline uint64_t GetBegin() const
    {
        return begin.load(std::memory_order_relaxed);
    }

    __always_inline uint64_t GetGoodEnd() const
    {
        return goodEnd.load(std::memory_order_relaxed);
    }

    __always_inline uint64_t GetBadEnd() const
    {
        return badEnd.load(std::memory_order_relaxed);
    }

    __always_inline uint64_t GetMin() const
    {
        return min.load(std::memory_order_relaxed);
    }

    __always_inline uint64_t GetMax() const
    {
        return max.load(std::memory_order_relaxed);
    }

    __always_inline uint64_t GetTotal() const
    {
        return total.load(std::memory_order_relaxed);
    }

    __always_inline bool NameValid() const
    {
        return nameValid;
    }

    void UpdatePreviousData()
    {
        previousBegin = begin.load(std::memory_order_relaxed);
        previousGoodEnd = goodEnd.load(std::memory_order_relaxed);
        previousBadEnd = badEnd.load(std::memory_order_relaxed);
        previousTotal = total.load(std::memory_order_relaxed);
        previousMin = UINT64_MAX;
        previousMax = 0;
    }

    std::string ToPeriodString()
    {
        auto beginGap = begin.load(std::memory_order_relaxed) - previousBegin;
        auto goodEndGap = goodEnd.load(std::memory_order_relaxed) - previousGoodEnd;
        auto badEndGap = badEnd.load(std::memory_order_relaxed) - previousBadEnd;
        auto totalGap = total.load(std::memory_order_relaxed) - previousTotal;
        auto minGap = previousMin.load(std::memory_order_relaxed);
        auto maxGap = previousMax.load(std::memory_order_relaxed);
        UpdatePreviousData();
        return HTracerUtils::FormatString(name, beginGap, goodEndGap, badEndGap, minGap, maxGap, totalGap);
    }

    std::string ToTotalString()
    {
        auto beginGap = begin.load(std::memory_order_relaxed);
        auto goodEndGap = goodEnd.load(std::memory_order_relaxed);
        auto badEndGap = badEnd.load(std::memory_order_relaxed);
        auto totalGap = total.load(std::memory_order_relaxed);
        auto minGap = min.load(std::memory_order_relaxed);
        auto maxGap = max.load(std::memory_order_relaxed);
        return HTracerUtils::FormatString(name, beginGap, goodEndGap, badEndGap, minGap, maxGap, totalGap);
    }

private:
    std::string name;
    std::atomic<bool> nameValid{ false };
    std::atomic_uint_fast64_t begin{ 0 };
    std::atomic_uint_fast64_t goodEnd{ 0 };
    std::atomic_uint_fast64_t badEnd{ 0 };
    std::atomic_uint_fast64_t min{ UINT64_MAX };
    std::atomic_uint_fast64_t max{ 0 };
    std::atomic_uint_fast64_t total{ 0 };

    uint64_t previousBegin{ 0 };
    uint64_t previousGoodEnd{ 0 };
    uint64_t previousBadEnd{ 0 };
    std::atomic_uint_fast64_t previousMin{ UINT64_MAX };
    std::atomic_uint_fast64_t previousMax{ 0 };
    uint64_t previousTotal{ 0 };
};
}
}
#endif // HTRACER_INFO_H
