/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
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
