/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include <iostream>
#include "gtest/gtest.h"
#include "mmc_service.h"
#include "mmc_client.h"
#include "mmc_blob_allocator.h"

using namespace testing;
using namespace std;

class TestMmcServiceError : public testing::Test {
public:
    TestMmcServiceError();

    void SetUp() override;

    void TearDown() override;

protected:
};
TestMmcServiceError::TestMmcServiceError() {}

void TestMmcServiceError::SetUp()
{
    cout << "this is NetEngine TEST_F setup:";
}

void TestMmcServiceError::TearDown()
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

static void GenerateData(void *ptr, int32_t rank)
{
    int32_t *arr = (int32_t *)ptr;
    static int32_t mod = INT16_MAX;
    int32_t base = rank;
    for (uint32_t i = 0; i < SIZE_32K / sizeof(int); i++) {
        base = (base * 23 + 17) % mod;
        if ((i + rank) % 3 == 0) {
            arr[i] = -base; // 构造三分之一的负数
        } else {
            arr[i] = base;
        }
    }
}

static bool CheckData(void *base, void *ptr)
{
    int32_t *arr1 = (int32_t *)base;
    int32_t *arr2 = (int32_t *)ptr;
    for (uint32_t i = 0; i < SIZE_32K / sizeof(int); i++) {
        if (arr1[i] != arr2[i]) return false;
    }
    return true;
}

TEST_F(TestMmcServiceError, metaService)
{
    std::string metaUrl = "tcp://127.0.0.1:5000";
    std::string bmUrl = "tcp://127.0.0.1:5881";
    std::string hcomUrl = "tcp://127.0.0.1:5882";
    std::string localUrl = "";

//    mmc_meta_service_config_t metaServiceConfig;
//    metaServiceConfig.tlsConfig.tlsEnable = false;
//    UrlStringToChar(metaUrl, metaServiceConfig.discoveryURL);
//    mmc_meta_service_t meta_service = mmcs_meta_service_start(&metaServiceConfig);
//    ASSERT_TRUE(meta_service != nullptr);

    mmc_local_service_config_t localServiceConfig = {"", 0, 0, 1, bmUrl, hcomUrl, 0, 0, "sdma", 0, 104857600, 0};
    UrlStringToChar(metaUrl, localServiceConfig.discoveryURL);
    mmc_meta_service_t local_service = mmcs_local_service_start(&localServiceConfig);
    ASSERT_TRUE(local_service != nullptr);

    mmc_client_config_t clientConfig;
    clientConfig.rankId = 0;
    UrlStringToChar(metaUrl, clientConfig.discoveryURL);
    int32_t ret = mmcc_init(&clientConfig);
    ASSERT_TRUE(ret == 0);
    sleep(1000);
    mmcs_local_service_stop(local_service);
    mmcc_uninit();
   // mmcs_meta_service_stop(meta_service);
}
