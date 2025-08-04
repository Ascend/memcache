/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include "mmc_meta_manager.h"
#include "mmc_ref.h"
#include "gtest/gtest.h"
#include <iostream>

using namespace testing;
using namespace std;
using namespace ock::mmc;

class TestLocalHandler : public testing::Test {
public:
    TestLocalHandler();

    void SetUp() override;

    void TearDown() override;

protected:
};
TestLocalHandler::TestLocalHandler() {}

void TestLocalHandler::SetUp()
{
    cout << "this is Meta Manger TEST_F setup:" << std::endl;
}

void TestLocalHandler::TearDown()
{
    cout << "this is Meta Manager TEST_F teardown" << std::endl;
}

TEST_F(TestLocalHandler, Init)
{
    MmcLocation loc{0, MEDIA_DRAM};
    MmcLocalMemlInitInfo locInfo{100, 1000};

    uint64_t defaultTtl = 2000;
    MmcRef<MmcMetaManager> metaMng = MmcMakeRef<MmcMetaManager>(defaultTtl, 70, 60);
    metaMng->Start();
    metaMng->Mount(loc, locInfo);
    ASSERT_TRUE(metaMng != nullptr);
    metaMng->Stop();
}

TEST_F(TestLocalHandler, Alloc)
{
    MmcLocation loc{0, MEDIA_DRAM};
    MmcLocalMemlInitInfo locInfo{0, 1000000};

    uint64_t defaultTtl = 2000;
    MmcRef<MmcMetaManager> metaMng = MmcMakeRef<MmcMetaManager>(defaultTtl, 70, 60);
    metaMng->Start();
    metaMng->Mount(loc, locInfo);

    AllocOptions allocReq{SIZE_32K, 1, 0, 0, 0};  // blobSize, numBlobs, mediaType, preferredRank, flags
    MmcMemObjMetaPtr objMeta;
    Result ret = metaMng->Alloc("test_string", allocReq, 1, objMeta);
    ASSERT_TRUE(ret == MMC_OK);
    ASSERT_TRUE(objMeta->NumBlobs() == 1);
    ASSERT_TRUE(objMeta->Size() == SIZE_32K);

    metaMng->UpdateState("test_string", loc, 0, 1, MMC_WRITE_OK);

    ret = metaMng->Remove("test_string");
    ASSERT_TRUE(ret == MMC_OK);
    metaMng->Stop();
}
