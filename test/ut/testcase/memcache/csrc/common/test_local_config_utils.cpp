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

#include <string>

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "common/mmc_functions.h"
#include "mmc.h"

using namespace testing;
using ock::mmc::SafeCopy;

class TestLocalConfigUtils : public testing::Test {
public:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(TestLocalConfigUtils, CreateDefaultLocalConfigReturnsExpectedDefaults)
{
    const local_config config = create_default_local_config();

    EXPECT_STREQ(config.config_path, "");
    EXPECT_STREQ(config.meta_service_url, "tcp://127.0.0.1:5000");
}

TEST_F(TestLocalConfigUtils, LocalConfigToStringReturnsExpectedFormat)
{
    local_config config = create_default_local_config();
    SafeCopy("tcp://10.0.0.1:5000", config.meta_service_url, sizeof(config.meta_service_url));
    SafeCopy("tcp://10.0.0.2:6000", config.config_store_url, sizeof(config.config_store_url));

    const std::string expected = "LocalConfig {\n"
                                 "  meta_service_url: tcp://10.0.0.1:5000\n"
                                 "  config_store_url: tcp://10.0.0.2:6000\n";

    EXPECT_THAT(local_config_to_string(config), ::testing::StartsWith(expected.substr(0, 50UL)));
}