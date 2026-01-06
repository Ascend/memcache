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
#include <gtest/gtest.h>

#include "smem_bm_api.h"

using namespace ock::mmc;

class MFSmemApiTest : public testing::Test {
public:
    static void SetUpTestCase() {}

    static void TearDownTestCase() {}

    void SetUp() override {}

    void TearDown() override {}
};

TEST_F(MFSmemApiTest, smem_bm_api_test)
{
    std::string outLibPath = OUT_LIB_PATH;
    ASSERT_EQ(MFSmemApi::LoadLibrary("") != 0, true);
    ASSERT_EQ(MFSmemApi::LoadLibrary(outLibPath + "/smem/lib64/") == 0, true);

    ASSERT_EQ(MFSmemApi::SmemInit(0) != 0, true);
}