/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include <iostream>
#include "gtest/gtest.h"
#include "mmc_service.h"
#include "mmc_client.h"

using namespace testing;
using namespace std;

class TestMmcServiceInterface : public testing::Test {
public:
    TestMmcServiceInterface();

    void SetUp() override;

    void TearDown() override;

protected:
};
TestMmcServiceInterface::TestMmcServiceInterface() {}

void TestMmcServiceInterface::SetUp()
{
    cout << "this is NetEngine TEST_F setup:";
}

void TestMmcServiceInterface::TearDown()
{
    cout << "this is NetEngine TEST_F teardown";
}

static void UrlStringToChar(std::string &urlString, char *urlChar)
{
    for (uint32_t i = 0; i < urlString.length(); i++) {
        urlChar[i] = urlString.at(i);
    }
    urlChar[urlString.length()] = '\0';
}

TEST_F(TestMmcServiceInterface, metaServiceStart)
{
    std::string metaUrl = "tcp://127.0.0.1:5668";
    std::string localUrl = "";
    mmc_meta_service_config_t metaServiceConfig;
    UrlStringToChar(metaUrl, metaServiceConfig.discoveryURL);
    mmc_meta_service_t meta_service = mmcs_meta_service_start(&metaServiceConfig);
    ASSERT_TRUE(meta_service != nullptr);

    mmc_local_service_config_t localServiceConfig;
    UrlStringToChar(metaUrl, localServiceConfig.discoveryURL);
    mmc_meta_service_t local_service = mmcs_local_service_start(&localServiceConfig);
    ASSERT_TRUE(local_service != nullptr);

    mmc_client_config_t clientConfig;
    UrlStringToChar(metaUrl, clientConfig.discoveryURL);
    int32_t ret = mmcc_init(&clientConfig);
    ASSERT_TRUE(ret == 0);
    std::string test = "test";
    mmc_buffer buffer;
    buffer.addr = 0;
    buffer.type = 0;
    buffer.hbm.width = 32;
    buffer.hbm.layerCount = 61;

    ret = mmcc_put(test.c_str(), &buffer, 0);
    ASSERT_TRUE(ret == 0);
}
