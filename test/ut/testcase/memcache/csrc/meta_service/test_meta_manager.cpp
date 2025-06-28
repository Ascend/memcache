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

class TestMmcMetaManager : public testing::Test {
public:
    TestMmcMetaManager();

    void SetUp() override;

    void TearDown() override;

protected:
};
TestMmcMetaManager::TestMmcMetaManager()
{
}

void TestMmcMetaManager::SetUp()
{
    cout << "this is Meta Manger TEST_F setup:" << std::endl;
}

void TestMmcMetaManager::TearDown()
{
    cout << "this is Meta Manager TEST_F teardown" << std::endl;
}

TEST_F(TestMmcMetaManager, Init)
{
    MmcMemPoolInitInfo poolInitInfo;
    MmcLocation loc{0, 0};
    MmcLocalMemlInitInfo locInfo{100, 1000};
    poolInitInfo[loc] = locInfo;

    uint64_t defaultTtl = 2000;

    MmcRef<MmcMetaManger> metaMng = MmcMakeRef<MmcMetaManger>(poolInitInfo, defaultTtl);
    ASSERT_TRUE(metaMng != nullptr);
}

TEST_F(TestMmcMetaManager, Alloc)
{
    MmcMemPoolInitInfo poolInitInfo;
    MmcLocation loc{0, 0};
    MmcLocalMemlInitInfo locInfo{0, 1000000};
    poolInitInfo[loc] = locInfo;
    uint64_t defaultTtl = 2000;
    MmcRef<MmcMetaManger> metaMng = MmcMakeRef<MmcMetaManger>(poolInitInfo, defaultTtl);

    AllocProperty allocReq{SIZE_32K, 1, 0, 0, 0};
    MmcMemObjMetaPtr objMeta;
    Result ret = metaMng->Alloc("test_string", allocReq, objMeta);
    ASSERT_TRUE(ret == MMC_OK);
    ASSERT_TRUE(objMeta->NumBlobs() == 1);
    ASSERT_TRUE(objMeta->Size() == SIZE_32K);
}
