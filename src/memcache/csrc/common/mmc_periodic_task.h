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
#ifndef MEM_FABRIC_MMC_PERIODIC_TASK_H
#define MEM_FABRIC_MMC_PERIODIC_TASK_H

#include <algorithm>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <functional>
#include <limits>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include "mmc_logger.h"

namespace ock {
namespace mmc {

class MmcPeriodicTask {
public:
    using Task = std::function<void()>;

    MmcPeriodicTask() = default;
    ~MmcPeriodicTask()
    {
        Stop();
    }

    MmcPeriodicTask(const MmcPeriodicTask &) = delete;
    MmcPeriodicTask &operator=(const MmcPeriodicTask &) = delete;

    bool RegisterTask(std::string name, uint32_t intervalSeconds, Task task)
    {
        if (intervalSeconds == 0 || !task) {
            MMC_LOG_ERROR("Failed to register periodic task, invalid param: name=" << name
                           << ", intervalSeconds=" << intervalSeconds << ", task=" << (task ? "set" : "null"));
            return false;
        }

        const auto now = std::chrono::steady_clock::now();
        {
            std::lock_guard<std::mutex> lock(mutex_);
            const auto iter = std::find_if(tasks_.begin(), tasks_.end(), [&name](const TaskEntry &entry) {
                return entry.name == name;
            });
            if (iter != tasks_.end()) {
                MMC_LOG_WARN("Periodic task already exists, update config: " << name);
                iter->interval = std::chrono::seconds(intervalSeconds);
                iter->task = std::move(task);
                iter->nextRun = now + iter->interval;
            } else {
                tasks_.push_back(TaskEntry{std::move(name), std::chrono::seconds(intervalSeconds), std::move(task),
                                           now + std::chrono::seconds(intervalSeconds)});
            }
        }
        cv_.notify_all();
        return true;
    }

    bool Start()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (running_.load()) {
            MMC_LOG_WARN("Periodic task scheduler already running");
            return true;
        }

        stop_.store(false);
        running_.store(true);
        MMC_LOG_INFO("Starting periodic task scheduler, taskCount=" << tasks_.size());
        worker_ = std::thread([this]() { Run(); });
        return true;
    }

    void Stop()
    {
        std::thread workerToJoin;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (!running_.load()) {
                return;
            }
            stop_.store(true);
            workerToJoin = std::move(worker_);
        }
        cv_.notify_all();
        if (workerToJoin.joinable()) {
            workerToJoin.join();
        }
        running_.store(false);
        MMC_LOG_INFO("Periodic task scheduler stopped");
    }

    bool IsRunning() const
    {
        return running_.load();
    }

private:
    struct TaskEntry {
        std::string name;
        std::chrono::seconds interval{0};
        Task task;
        std::chrono::steady_clock::time_point nextRun;
    };

    void Run()
    {
        MMC_LOG_INFO("Periodic task scheduler worker entered");
        while (!stop_.load()) {
            std::vector<TaskEntry> dueTasks;
            auto nextWake = std::chrono::steady_clock::time_point::max();
            const auto now = std::chrono::steady_clock::now();

            {
                std::unique_lock<std::mutex> lock(mutex_);
                if (tasks_.empty()) {
                    cv_.wait(lock, [this]() { return stop_.load() || !tasks_.empty(); });
                    continue;
                }

                const auto current = std::chrono::steady_clock::now();
                for (auto &entry : tasks_) {
                    if (entry.nextRun <= current) {
                        dueTasks.push_back(entry);
                        auto roundsBehind = static_cast<int64_t>((current - entry.nextRun) / entry.interval) + 1;
                        if (roundsBehind < 1) {
                            roundsBehind = 1;
                        }
                        entry.nextRun += entry.interval * roundsBehind;
                    }
                    if (entry.nextRun < nextWake) {
                        nextWake = entry.nextRun;
                    }
                }

                if (dueTasks.empty()) {
                    cv_.wait_until(lock, nextWake);
                    continue;
                }
            }

            for (const auto &entry : dueTasks) {
                try {
                    entry.task();
                } catch (const std::exception &e) {
                    MMC_LOG_ERROR("Periodic task " << entry.name << " failed: " << e.what());
                } catch (...) {
                    MMC_LOG_ERROR("Periodic task " << entry.name << " failed with unknown exception");
                }
            }
        }
        MMC_LOG_INFO("Periodic task scheduler worker exiting");
    }

private:
    std::vector<TaskEntry> tasks_;
    std::thread worker_;
    std::mutex mutex_;
    std::condition_variable cv_;
    std::atomic<bool> running_{false};
    std::atomic<bool> stop_{false};
};

} // namespace mmc
} // namespace ock

#endif
