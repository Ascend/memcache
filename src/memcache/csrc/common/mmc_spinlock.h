/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2026. All rights reserved.
 */
#ifndef MEMFABRIC_HYBRID_MMC_SPINLOCK_H
#define MEMFABRIC_HYBRID_MMC_SPINLOCK_H

#include <atomic>
#include <mutex>

namespace ock {
namespace mmc {

class Spinlock {
public:
    void lock()
    {
        while (lock_.exchange(1, std::memory_order_acquire)) {
        }
    }
    void unlock() { lock_.store(0, std::memory_order_release); }

private:
    std::atomic<uint32_t> lock_{0}; // 4 bytes atomic variable
};

} // namespace mmc
} // namespace ock

#endif // MEMFABRIC_HYBRID_MMC_SPINLOCK_H