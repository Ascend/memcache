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
#include <cstdlib>
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

TEST_F(MFSmemApiTest, resolve_lib_dir_and_load)
{
    const std::string libDir = MFSmemApi::ResolveLibDir();
    ASSERT_FALSE(libDir.empty());
    ASSERT_EQ(MFSmemApi::LoadLibrary(libDir), MMC_OK);
    ASSERT_EQ(MFSmemApi::LoadLibrary(libDir), MMC_OK);
}

TEST_F(MFSmemApiTest, resolve_lib_dir_fails_when_lib_missing)
{
    const char *savedPath = std::getenv("MEMFABRIC_HYBRID_EXTEND_LIB_PATH");
    ASSERT_NE(savedPath, nullptr);

    setenv("MEMFABRIC_HYBRID_EXTEND_LIB_PATH", "/tmp/mf_smem_not_exist/lib64", 1);
    const std::string resolved = MFSmemApi::ResolveLibDir();
    setenv("MEMFABRIC_HYBRID_EXTEND_LIB_PATH", savedPath, 1);

    ASSERT_TRUE(resolved.empty());
}

TEST_F(MFSmemApiTest, reject_empty_lib_dir)
{
    ASSERT_EQ(MFSmemApi::LoadLibrary(""), MMC_INVALID_PARAM);
}

TEST_F(MFSmemApiTest, cleanup_and_reload)
{
    const std::string libDir = MFSmemApi::ResolveLibDir();
    ASSERT_FALSE(libDir.empty());
    ASSERT_EQ(MFSmemApi::LoadLibrary(libDir), MMC_OK);

    MFSmemApi::CleanupLibrary();
    ASSERT_EQ(MFSmemApi::LoadLibrary(libDir), MMC_OK);

    MFSmemApi::CleanupLibrary();
    MFSmemApi::CleanupLibrary();
}
