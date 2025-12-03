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
#include "mmc_lookup_map.h"
#include "mmc_ref.h"
#include "gtest/gtest.h"
#include <iostream>
#include <string>
#include <thread>

using namespace testing;
using namespace std;
using namespace ock::mmc;

class TestLookupMap : public testing::Test {
public:
    TestLookupMap();

    void SetUp() override;

    void TearDown() override;

protected:
};
TestLookupMap::TestLookupMap() {}

void TestLookupMap::SetUp() {}

void TestLookupMap::TearDown() {}

TEST_F(TestLookupMap, EmptyMap) {
    MmcLookupMap<uint16_t, std::string, 3> emptyMap;
    EXPECT_EQ(emptyMap.begin(), emptyMap.end());
}