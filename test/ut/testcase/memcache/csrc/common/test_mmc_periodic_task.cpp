/*
* Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 * MemCache_Hybrid is licensed under Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN AS IS BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
*/

#include <atomic>
#include <chrono>
#include <stdexcept>
#include <thread>

#include "gtest/gtest.h"
#include "common/mmc_periodic_task.h"

using namespace ock::mmc;

class TestMmcPeriodicTask : public testing::Test {
public:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(TestMmcPeriodicTask, RegisterTaskReturnsFalseWhenIntervalIsZero)
{
    MmcPeriodicTask scheduler;
    EXPECT_FALSE(scheduler.RegisterTask("zero_interval", 0, []() {}));
    EXPECT_FALSE(scheduler.IsRunning());
}

TEST_F(TestMmcPeriodicTask, RegisterTaskReturnsFalseWhenTaskIsEmpty)
{
    MmcPeriodicTask scheduler;
    MmcPeriodicTask::Task emptyTask;
    EXPECT_FALSE(scheduler.RegisterTask("empty_task", 1, emptyTask));
    EXPECT_FALSE(scheduler.IsRunning());
}

TEST_F(TestMmcPeriodicTask, StartWithoutRegisteredTasksSucceeds)
{
    MmcPeriodicTask scheduler;
    EXPECT_TRUE(scheduler.Start());
    EXPECT_TRUE(scheduler.IsRunning());

    scheduler.Stop();
    EXPECT_FALSE(scheduler.IsRunning());
}

TEST_F(TestMmcPeriodicTask, StartTwiceReturnsTrueWhenAlreadyRunning)
{
    MmcPeriodicTask scheduler;
    std::atomic<int> counter{0};
    ASSERT_TRUE(scheduler.RegisterTask("double_start_task", 1, [&counter]() { ++counter; }));

    EXPECT_TRUE(scheduler.Start());
    EXPECT_TRUE(scheduler.IsRunning());
    EXPECT_TRUE(scheduler.Start());

    scheduler.Stop();
    EXPECT_FALSE(scheduler.IsRunning());
}

TEST_F(TestMmcPeriodicTask, StartAndStopWorkCorrectly)
{
    MmcPeriodicTask scheduler;
    std::atomic<int> counter{0};
    ASSERT_TRUE(scheduler.RegisterTask("count_task", 1, [&counter]() { ++counter; }));
    EXPECT_TRUE(scheduler.Start());
    EXPECT_TRUE(scheduler.IsRunning());

    for (int i = 0; i < 20UL && counter.load() == 0; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100UL));
    }

    scheduler.Stop();
    EXPECT_FALSE(scheduler.IsRunning());
    EXPECT_GE(counter.load(), 1);
}

TEST_F(TestMmcPeriodicTask, RegisterSameNameUpdatesTaskAndInterval)
{
    MmcPeriodicTask scheduler;
    std::atomic<int> oldTaskCounter{0};
    std::atomic<int> newTaskCounter{0};

    ASSERT_TRUE(scheduler.RegisterTask("dup_task", 1, [&oldTaskCounter]() { ++oldTaskCounter; }));
    ASSERT_TRUE(scheduler.RegisterTask("dup_task", 1, [&newTaskCounter]() { ++newTaskCounter; }));
    ASSERT_TRUE(scheduler.Start());

    for (int i = 0; i < 20UL && newTaskCounter.load() == 0; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100UL));
    }

    scheduler.Stop();
    EXPECT_EQ(oldTaskCounter.load(), 0);
    EXPECT_GE(newTaskCounter.load(), 1);
}

TEST_F(TestMmcPeriodicTask, TaskExceptionDoesNotStopScheduler)
{
    MmcPeriodicTask scheduler;
    std::atomic<int> counter{0};
    ASSERT_TRUE(scheduler.RegisterTask("throw_task", 1, [&counter]() {
        ++counter;
        if (counter.load() == 1) {
            throw std::runtime_error("expected");
        }
    }));
    ASSERT_TRUE(scheduler.Start());

    for (int i = 0; i < 30UL && counter.load() < 2UL; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100UL));
    }

    scheduler.Stop();
    EXPECT_FALSE(scheduler.IsRunning());
    EXPECT_GE(counter.load(), 2UL);
}

TEST_F(TestMmcPeriodicTask, UnknownExceptionDoesNotStopScheduler)
{
    MmcPeriodicTask scheduler;
    std::atomic<int> counter{0};
    ASSERT_TRUE(scheduler.RegisterTask("unknown_throw_task", 1, [&counter]() {
        ++counter;
        if (counter.load() == 1) {
            throw 1;
        }
    }));
    ASSERT_TRUE(scheduler.Start());

    for (int i = 0; i < 30UL && counter.load() < 2UL; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100UL));
    }

    scheduler.Stop();
    EXPECT_FALSE(scheduler.IsRunning());
    EXPECT_GE(counter.load(), 2UL);
}

TEST_F(TestMmcPeriodicTask, StopWithoutStartIsSafe)
{
    MmcPeriodicTask scheduler;
    scheduler.Stop();
    EXPECT_FALSE(scheduler.IsRunning());
}

TEST_F(TestMmcPeriodicTask, RegisterTaskWhileRunningCanWakeWorkerAndExecute)
{
    MmcPeriodicTask scheduler;
    std::atomic<int> counter{0};

    ASSERT_TRUE(scheduler.RegisterTask("slow_task", 10UL, []() {}));
    ASSERT_TRUE(scheduler.Start());
    ASSERT_TRUE(scheduler.RegisterTask("fast_task", 1, [&counter]() { ++counter; }));

    for (int i = 0; i < 20UL && counter.load() == 0; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100UL));
    }

    scheduler.Stop();
    EXPECT_GE(counter.load(), 1);
}

TEST_F(TestMmcPeriodicTask, MultipleDueTasksExecuteInRegistrationOrder)
{
    MmcPeriodicTask scheduler;
    std::atomic<int> step{0};
    std::atomic<int> firstOrder{0};
    std::atomic<int> secondOrder{0};

    ASSERT_TRUE(scheduler.RegisterTask("first_task", 1, [&step, &firstOrder]() {
        firstOrder.store(++step);
    }));
    ASSERT_TRUE(scheduler.RegisterTask("second_task", 1, [&step, &secondOrder]() {
        secondOrder.store(++step);
    }));
    ASSERT_TRUE(scheduler.Start());

    for (int i = 0; i < 20UL && secondOrder.load() == 0; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100UL));
    }

    scheduler.Stop();
    EXPECT_EQ(firstOrder.load(), 1);
    EXPECT_EQ(secondOrder.load(), 2UL);
}
