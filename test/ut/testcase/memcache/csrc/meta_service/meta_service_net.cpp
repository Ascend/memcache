/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include <iostream>
#include "gtest/gtest.h"
#include "mmc_ref.h"
#include "mmc_meta_service_default.h"
#include "mmc_local_service_default.h"
#include "mmc_msg_client_meta.h"
#include "mmc_locality_strategy.h"
#include "mmc_mem_obj_meta.h"

using namespace testing;
using namespace std;
using namespace ock::mmc;

class TestMmcMetaService : public testing::Test {
public:
    TestMmcMetaService();

    void SetUp() override;

    void TearDown() override;

protected:
};
TestMmcMetaService::TestMmcMetaService() {}

void TestMmcMetaService::SetUp()
{
    cout << "this is NetEngine TEST_F setup:";
}

void TestMmcMetaService::TearDown()
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

TEST_F(TestMmcMetaService, Init)
{
    std::string metaUrl = "tcp://127.0.0.1:5678";
    mmc_meta_service_config_t metaServiceConfig;
    UrlStringToChar(metaUrl, metaServiceConfig.discoveryURL);
    auto metaServiceDefault = MmcMakeRef<MmcMetaServiceDefault>("testMetaService");
    MmcMetaServicePtr metaServicePtr = Convert<MmcMetaServiceDefault, MmcMetaService>(metaServiceDefault);
    ASSERT_TRUE(metaServicePtr->Start(metaServiceConfig) == MMC_OK);

    mmc_local_service_config_t localServiceConfig;
    UrlStringToChar(metaUrl, localServiceConfig.discoveryURL);
    auto localServiceDefault = MmcMakeRef<MmcLocalServiceDefault>("testLocalService");
    MmcLocalServicePtr localServicePtr = Convert<MmcLocalServiceDefault, MmcLocalService>(localServiceDefault);
    ASSERT_TRUE(localServicePtr->Start(localServiceConfig) == MMC_OK);

    PingMsg req;
    req.msgId = ML_PING_REQ;
    req.num = 123;
    PingMsg resp;
    int16_t respRet;
    ASSERT_TRUE(localServiceDefault->SyncCallMeta(req, resp, respRet, 30) == MMC_OK);
    ASSERT_TRUE(respRet == MMC_OK);


    AllocRequest reqAlloc;
    reqAlloc.key_ = "test";
    reqAlloc.options_ = {SIZE_32K, 1, 0, 0, 0};
    AllocResponse respAlloc;
    ASSERT_TRUE(localServiceDefault->SyncCallMeta(reqAlloc, respAlloc, respRet, 30) == MMC_OK);
    ASSERT_TRUE(respRet == MMC_OK);
    ASSERT_TRUE(respAlloc.numBlobs_ == 1);
    ASSERT_TRUE(respAlloc.blobs_[0].size_ == SIZE_32K);
    metaServicePtr->Stop();
    localServicePtr->Stop();
}
