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
#ifndef __MEMFABRIC_HYBRID_MMC_THREAD_POOL_H__
#define __MEMFABRIC_HYBRID_MMC_THREAD_POOL_H__

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <future>
#include <stdexcept>
#include <type_traits>
#include <pthread.h>
#include <string>
#include "mmc_logger.h"
#include "mmc_types.h"
#include "mmc_ref.h"

namespace ock {
namespace mmc {
// 兼容 C++11/C++14
#if __cplusplus < 201703L
template<typename F, typename... Args>
using invoke_result_t = typename std::result_of<F(Args...)>::type;
#else
template<typename F, typename... Args>
using invoke_result_t = typename std::invoke_result<F, Args...>::type;
#endif

class MmcThreadPool  : public MmcReferable {
public:
    MmcThreadPool(std::string name, size_t numThreads) : mmcPoolName(name), numThreads(numThreads), stop(false) {}

    int32_t Start()
    {
        if (numThreads == 0 || numThreads > MMC_THREAD_POOL_MAX_THREADS) {
            MMC_LOG_ERROR("Number of threads must be greater than 0 and less than " << MMC_THREAD_POOL_MAX_THREADS);
            return MMC_ERROR;
        }
        for (size_t i = 0; i < numThreads; ++i) {
            workers.emplace_back([this] {
                while (true) {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lock(queueMutex);
                        queueCondition.wait(lock, [this] { return stop || !taskQueue.empty(); });
                        if (stop && taskQueue.empty()) {
                            break;
                        }
                        task = std::move(taskQueue.front());
                        taskQueue.pop();
                    }
                    task();
                }
                MMC_LOG_DEBUG("worker thread :" << std::this_thread::get_id() << " exit");
            });

            std::string threadName = mmcPoolName + std::to_string(i);
            int ret = pthread_setname_np(workers.back().native_handle(), threadName.c_str());
            if (ret != 0) {
                MMC_LOG_ERROR("set thread name failed, i:" << i << ", ret:" << ret);
            }
        }
        return MMC_OK;
    }

    template<typename F, typename... Args>
    auto Enqueue(F &&f, Args &&...args) -> std::future<invoke_result_t<F, Args...>>
    {
        using returnType = invoke_result_t<F, Args...>;
        auto func = std::make_shared<std::packaged_task<returnType()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...));
        std::future<returnType> future = func->get_future();
        {
            std::unique_lock<std::mutex> uniqueLock(queueMutex);
            if (stop) {
                MMC_LOG_ERROR("thread pool has stopped.");
                return std::future<returnType>{}; // 返回无效的 future
            }
            taskQueue.emplace([func]() { (*func)(); });
        }
        queueCondition.notify_one();
        return future;
    }

    void Destroy()
    {
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            stop = true;
        }
        queueCondition.notify_all();
        for (std::thread &worker : workers) {
            if (worker.joinable()) {
                worker.join();
            }
        }
    }

    ~MmcThreadPool() override
    {
        Destroy();
    }

    MmcThreadPool(const MmcThreadPool&) = delete;
    MmcThreadPool& operator=(const MmcThreadPool&) = delete;

private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> taskQueue;
    std::mutex queueMutex;
    std::condition_variable queueCondition;
    std::string mmcPoolName;
    size_t numThreads;
    bool stop;
};

using MmcThreadPoolPtr = MmcRef<MmcThreadPool>;
}  // namespace mmc
}  // namespace ock

#endif