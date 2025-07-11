/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include "mmc_read_write_lock.h"
#include "mmc_ref.h"
#include "gtest/gtest.h"
#include <iostream>
#include <thread>
#include <vector>

using namespace testing;
using namespace std;
using namespace ock::mmc;

class TestLocks : public testing::Test {
public:
    TestLocks();

    void SetUp() override;

    void TearDown() override;

protected:
};
TestLocks::TestLocks() {}

void TestLocks::SetUp() {}

void TestLocks::TearDown() {}

TEST_F(TestLocks, SingleThreadLorckOrder)
{
    ReadWriteLock rwLock;
    int sharedData = 0;
    {
        WriteLock write(rwLock);
        sharedData = 7;
        ASSERT_TRUE(sharedData == 7);
    }
    {
        ReadLock read(rwLock);
        ASSERT_TRUE(sharedData == 7);
    }
}

TEST_F(TestLocks, MultiReaders)
{
    ReadWriteLock rwLock;
    std::atomic<uint16_t> readerCount(0);
    const uint16_t numThreads = 5;

    auto reader = [&] {
        ReadLock lock(rwLock);
        ++readerCount;
        std::cout << "readerCount:" << readerCount << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    };

    std::vector<std::thread> threads;
    for (size_t i = 0; i < numThreads; ++i) {
        threads.emplace_back(reader);
    }

    for (auto &t : threads) {
        t.join();
    }

    ASSERT_TRUE(readerCount == numThreads);
}

TEST_F(TestLocks, WriteLock)
{
    ReadWriteLock rwLock;
    std::atomic<bool> writerOccupied(false);
    std::atomic<bool> readerBlocked(true);

    std::thread writer([&] {
        WriteLock lock(rwLock);
        writerOccupied = true;
        std::cout << "writerOccupied" << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(30));
    });

    while (!writerOccupied) {
        std::this_thread::yield();
    }

    std::thread reader([&] {
        std::cout << "before get ReadLock, readerBlocked:" << readerBlocked << std::endl;
        ReadLock lock(rwLock);
        readerBlocked = false;
        std::cout << "after get ReadLock, readerBlocked:" << readerBlocked << std::endl;
    });

    reader.join();
    writer.join();

    ASSERT_TRUE(readerBlocked == false);
}