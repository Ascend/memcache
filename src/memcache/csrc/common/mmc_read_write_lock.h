/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2026. All rights reserved.
 */
#ifndef MEMFABRIC_HYBRID_MMC_READWRITELOCK_H
#define MEMFABRIC_HYBRID_MMC_READWRITELOCK_H

#include <condition_variable>
#include <memory>
#include <mutex>

namespace ock {
namespace mmc {
class ReadWriteLock {
public:
    ReadWriteLock() = default;

    void LockRead()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        while (isWriting_) {
            cv_.wait(lock);
        }
        numReaders_++;
    }

    void UnlockRead()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        numReaders_--;
        if (numReaders_ == 0) {
            cv_.notify_one();  // wake up the write thread
        }
    }

    void LockWrite()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        while (isWriting_ || numReaders_ > 0) {
            cv_.wait(lock);
        }
        isWriting_ = true;
    }

    void UnlockWrite()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        isWriting_ = false;
        cv_.notify_all();  // wake up all the read threads
    }

private:
    std::mutex mutex_;
    std::condition_variable cv_;
    uint16_t numReaders_{0};  // number of current reade threads
    bool isWriting_{false};   // whether currently have write threads
};

}  // namespace mmc
}  // namespace ock

#endif  // MEMFABRIC_HYBRID_MMC_READWRITELOCK_H