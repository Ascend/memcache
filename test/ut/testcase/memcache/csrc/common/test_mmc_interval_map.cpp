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

#include <iostream>
#include <thread>
#include <vector>
#include "gtest/gtest.h"
#include "mmc_ref.h"
#include "mmc_interval_map.h"

using namespace testing;
using namespace std;
using namespace ock::mmc;

class TestMmcIntervalMap : public testing::Test {
public:
    TestMmcIntervalMap() {};

    void SetUp() override;

    void TearDown() override;

protected:
};

void TestMmcIntervalMap::SetUp() {}

void TestMmcIntervalMap::TearDown() {}

TEST(TestMmcIntervalMap, BasicInsertAndQuery)
{
    MmcIntervalMap<std::string> im;

    im.Add(100, 50, "code");   // [100, 150)
    im.Add(200, 80, "data");   // [200, 280)
    im.Add(300, 100, "stack"); // [300, 400)

    EXPECT_EQ(im.Query(90), nullptr);
    EXPECT_EQ(*im.Query(100), "code");
    EXPECT_EQ(*im.Query(149), "code");
    EXPECT_EQ(im.Query(150), nullptr);
    EXPECT_EQ(im.Query(199), nullptr);
    EXPECT_EQ(*im.Query(200), "data");
    EXPECT_EQ(*im.Query(279), "data");
    EXPECT_EQ(im.Query(280), nullptr);
    EXPECT_EQ(*im.Query(350), "stack");
    EXPECT_EQ(*im.Query(399), "stack");
    EXPECT_EQ(im.Query(400), nullptr);
    EXPECT_EQ(im.Query(1000), nullptr);
}

// ------------------------------------------------------------------------
// 测试组：重叠拒绝 - 各种重叠情况
// ------------------------------------------------------------------------
TEST(TestMmcIntervalMap, OverlapRejection)
{
    MmcIntervalMap<std::string> im;

    im.Add(100, 100, "A"); // [100, 200)

    // 完全包含已有区间
    EXPECT_EQ(im.Add(120, 60, "B"), false); // [120,180)

    // 与已有区间左侧重叠
    EXPECT_EQ(im.Add(80, 50, "C"), false); // [80,130)

    // 与已有区间右侧重叠
    EXPECT_EQ(im.Add(180, 50, "D"), false); // [180,230)

    // 刚好起点重叠（起点相同）
    EXPECT_EQ(im.Add(100, 30, "E"), false); // [100,130)

    // 刚好终点重叠（end 相同）
    EXPECT_EQ(im.Add(150, 50, "F"), false); // [150,200)

    // 相邻但不重叠（应该允许）
    ASSERT_TRUE(im.Add(200, 50, "G")); // [200,250)
    ASSERT_TRUE(im.Add(0, 100, "H"));  // [0,100)

    // 确认相邻区间独立存在，不合并
    auto q = im.Query(199);
    EXPECT_EQ(*q, "A");

    q = im.Query(200);
    EXPECT_EQ(*q, "G");

    q = im.Query(0);
    EXPECT_EQ(*q, "H");
}

// ------------------------------------------------------------------------
// 测试组：边界、空区间、零大小、极端地址
// ------------------------------------------------------------------------
TEST(TestMmcIntervalMap, EdgeCasesAndInvalidInputs)
{
    MmcIntervalMap<std::string> im;

    // 零大小区间应被拒绝
    EXPECT_FALSE(im.Add(500, 0, "zero"));

    // 非常大的地址（接近 uint64_t 边界）
    ASSERT_FALSE(im.Add(0xFFFFFFFFFFFFFF00ULL, 0x100, "high"));  // 翻转应该失败
    auto q = im.Query(0xFFFFFFFFFFFFFF50ULL);
    EXPECT_EQ(q, nullptr);

    // 负数地址（如果你的实现允许 signed → unsigned 转换，这里测试 uint64_t 语义）
    // uint64_t 下 -1 会变成很大值，通常不应允许负数地址
    // 如果你想明确禁止，可以在 Add 里加检查
    // 这里假设允许大值
    ASSERT_TRUE(im.Add(0x8000000000000000ULL, 0x1000, "kernel"));

    // 查询边界点
    im.Add(1000, 1, "single");  // [1000, 1001)
    EXPECT_EQ(im.Query(999), nullptr);
    EXPECT_EQ(*im.Query(1000), "single");
    EXPECT_EQ(im.Query(1001), nullptr);

    // 空 map
    MmcIntervalMap<std::string> empty_map;
    EXPECT_EQ(empty_map.Query(0), nullptr);
    EXPECT_EQ(empty_map.Query(123456789012345ULL), nullptr);
}

// ------------------------------------------------------------------------
// 測試組：範圍查詢基本正確性
// ------------------------------------------------------------------------
TEST(TestMmcIntervalMap, SingleIntervalFullyCovered)
{
    MmcIntervalMap<std::string> im;
    im.Add(100, 200, "regionA"); // [100, 300)

    EXPECT_EQ(*im.Query(120, 40), "regionA");  // 完全在內部
    EXPECT_EQ(*im.Query(100, 200), "regionA"); // 從起點開始
    EXPECT_EQ(*im.Query(299, 1), "regionA");   // 最後一個位元組
    EXPECT_EQ(*im.Query(100, 200), "regionA"); // 正好整段

    EXPECT_EQ(im.Query(99, 10), nullptr);  // 左邊超出
    EXPECT_EQ(im.Query(250, 60), nullptr); // 右邊超出
    EXPECT_EQ(im.Query(120, 0), nullptr);  // size=0
}

// ------------------------------------------------------------------------
// 測試組：連續相同值的多個區間應視為同一塊
// ------------------------------------------------------------------------
TEST(TestMmcIntervalMap, MultipleAdjacentSameValue)
{
    MmcIntervalMap<std::string> im;
    im.Add(0, 100, "code");   // [0,100)
    im.Add(100, 150, "code"); // [100,250)
    im.Add(250, 50, "code");  // [250,300)

    EXPECT_EQ(*im.Query(0, 300), "code");  // 完整三段
    EXPECT_EQ(*im.Query(50, 180), "code"); // 跨越前兩段
    EXPECT_EQ(*im.Query(240, 40), "code"); // 跨越最後兩段的交界

    EXPECT_EQ(im.Query(0, 301), nullptr); // 多出一個位元組
    EXPECT_EQ(*im.Query(90, 180), "code");      // 從第一段中間到第三段中間
}

// ------------------------------------------------------------------------
// 測試組：值不同或有空洞時應拒絕
// ------------------------------------------------------------------------
TEST(TestMmcIntervalMap, DifferentValueOrGapShouldFail)
{
    MmcIntervalMap<std::string> im;
    im.Add(0, 100, "A");   // [0,100)
    im.Add(100, 100, "A"); // [100,200)
    im.Add(250, 100, "A"); // [250,350)   ← 中間有空洞 200~250
    im.Add(350, 100, "B"); // [350,450)

    // 值相同但有空洞
    EXPECT_EQ(im.Query(50, 250), nullptr);  // 中间一段[200, 250]是空洞
    EXPECT_EQ(im.Query(180, 120), nullptr); // 中间一段[200, 250]是空洞

    // 值不同
    EXPECT_EQ(im.Query(180, 200), nullptr); // 有空洞且值不同
    EXPECT_EQ(im.Query(300, 100), nullptr); //  A → B

    // 部分覆蓋
    EXPECT_EQ(*im.Query(260, 50), "A"); // [260,310) 只覆蓋一部分 A
}

// ------------------------------------------------------------------------
// 測試組：邊界與極端情況
// ------------------------------------------------------------------------
TEST(TestMmcIntervalMap, BoundaryAndExtremeCases)
{
    MmcIntervalMap<std::string> im;
    im.Add(0, 1, "single");             // [0,1)
    im.Add(1000000000000ULL, 1, "big"); // 大地址單點

    // 單點區間
    EXPECT_EQ(*im.Query(0, 1), "single");
    EXPECT_EQ(im.Query(0, 2), nullptr);

    // 大地址
    uint64_t base = 1000000000000ULL;
    EXPECT_EQ(*im.Query(base, 1), "big");
    EXPECT_EQ(im.Query(base - 1, 1), nullptr);
    EXPECT_EQ(im.Query(base, 2), nullptr);

    // size 很大但實際只覆蓋一點
    EXPECT_EQ(im.Query(base, 10000000000ULL), nullptr);

    // 空 map
    MmcIntervalMap<std::string> empty;
    EXPECT_EQ(empty.Query(0, 100), nullptr);
    EXPECT_EQ(empty.Query(123456789012345ULL, 1), nullptr);

    // size = 0
    EXPECT_EQ(im.Query(500, 0), nullptr);
}

// ------------------------------------------------------------------------
// 測試組：相鄰但值不同（不應合併）
// ------------------------------------------------------------------------
TEST(TestMmcIntervalMap, AdjacentButDifferentValue)
{
    MmcIntervalMap<std::string> im;
    im.Add(0, 100, "left");
    im.Add(100, 100, "right");

    EXPECT_EQ(im.Query(50, 100), nullptr); // 跨越 left → right，值不同
    EXPECT_EQ(*im.Query(0, 100), "left");
    EXPECT_EQ(*im.Query(100, 100), "right");
}

TEST(IntervalMapDeleteTest, RemoveExactMatch)
{
    MmcIntervalMap<std::string> im;
    im.Add(1000, 4096, "page1");
    im.Add(8192, 8192, "page2");

    EXPECT_TRUE(im.Remove(1000, 4096));
    EXPECT_EQ(im.Query(1000), nullptr);
    EXPECT_EQ(im.Query(5000), nullptr);

    EXPECT_FALSE(im.Remove(1000, 4096)); // 已刪除
}

TEST(IntervalMapDeleteTest, RemoveExactOnly)
{
    MmcIntervalMap<std::string> im;
    im.Add(0, 100, "A");
    im.Add(100, 200, "B");

    // 部分重疊應失敗
    EXPECT_FALSE(im.Remove(50, 100)); // 與 A 重疊但不完整
    EXPECT_FALSE(im.Remove(0, 150));  // 跨越 A 和 B
    EXPECT_FALSE(im.Remove(0, 50));   // 只刪除 A 一半

    EXPECT_TRUE(im.Remove(0, 100));
    EXPECT_TRUE(im.Remove(100, 200));
}

TEST(IntervalMapDeleteTest, RemoveAt)
{
    MmcIntervalMap<std::string> im;
    im.Add(200, 300, "region");

    EXPECT_TRUE(im.RemoveAt(250));
    EXPECT_EQ(im.Query(250), nullptr);
    EXPECT_EQ(im.Query(199), nullptr);
    EXPECT_EQ(im.Query(500), nullptr);

    EXPECT_FALSE(im.RemoveAt(250)); // 已刪除
    EXPECT_FALSE(im.RemoveAt(150)); // 本來就不存在
}