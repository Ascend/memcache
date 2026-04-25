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
#include "mmc_meta_manager.h"
#include "mmc_bm_proxy.h"
#include "mmc_ref.h"
#include "smem_bm.h"
#include "gtest/gtest.h"
#include <iostream>
#include <memory>

extern "C" const smem_bm_create_option_t *MockSmemBmGetLastCreate2Option();
extern "C" void MockSmemBmResetLastCreate2Option();

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
    MmcRef<MmcMetaManager> metaMng = MmcMakeRef<MmcMetaManager>(defaultTtl, 70, 60, false);
    metaMng->Start();
    std::map<std::string, MmcMemBlobDesc> blobMap;
    metaMng->Mount(loc, locInfo, blobMap);
    ASSERT_TRUE(metaMng != nullptr);
    metaMng->Stop();
}

TEST_F(TestLocalHandler, Alloc)
{
    MmcLocation loc{0, MEDIA_DRAM};
    MmcLocalMemlInitInfo locInfo{0, 1000000};

    uint64_t defaultTtl = 2000;
    MmcRef<MmcMetaManager> metaMng = MmcMakeRef<MmcMetaManager>(defaultTtl, 70, 60, false);
    metaMng->Start();
    std::map<std::string, MmcMemBlobDesc> blobMap;
    metaMng->Mount(loc, locInfo, blobMap);

    AllocOptions allocReq{SIZE_32K, 1, MEDIA_DRAM, {0}, 0}; // blobSize, numBlobs, mediaType, preferredRank, flags
    MmcMemMetaDesc objMeta;
    Result ret = metaMng->Alloc("test_string", allocReq, 1, objMeta);
    ASSERT_TRUE(ret == MMC_OK);
    ASSERT_TRUE(objMeta.NumBlobs() == 1);
    ASSERT_TRUE(objMeta.Size() == SIZE_32K);

    metaMng->UpdateState("test_string", loc, MMC_WRITE_OK, 1);

    ret = metaMng->Remove("test_string");
    ASSERT_TRUE(ret == MMC_OK);
    metaMng->Stop();
}

class TestMmcBmProxy : public testing::Test {
protected:
    void SetUp() override
    {
        proxy = MmcMakeRef<MmcBmProxy>("test_proxy");

        initConfig = {.deviceId = 0,
                      .worldSize = 4,
                      .ipPort = "127.0.0.1:5000",
                      .hcomUrl = "tcp://127.0.0.1:5001",
                      .logLevel = INFO_LEVEL,
                      .logFunc = nullptr,
                      .flags = 0,
                      .hcomTlsConfig = {},
                      .storeTlsConfig = {}};

        createConfig = {.id = 12345,
                        .memberSize = 4,
                        .dataOpType = "device_sdma",
                        .localDRAMSize = 0,
                        .localMaxDRAMSize = 0,
                        .localHBMSize = 1024 * 1024 * 2,
                        .localMaxHBMSize = 0,
                        .flags = 0};

        oneDimBuffer = {.addr = 0x1000, .type = 0, .offset = 0, .len = 1024};
    }

    Result InitBmWithConfig()
    {
        return proxy->InitBm(initConfig, createConfig);
    }

    MmcRef<MmcBmProxy> proxy;
    mmc_bm_init_config_t initConfig;
    mmc_bm_create_config_t createConfig;
    mmc_buffer oneDimBuffer;
};

TEST_F(TestMmcBmProxy, InitBm_Success)
{
    Result ret = InitBmWithConfig();
    EXPECT_EQ(ret, MMC_OK);
}

TEST_F(TestMmcBmProxy, InitBm_AlreadyStarted)
{
    InitBmWithConfig();
    Result ret = InitBmWithConfig();
    EXPECT_EQ(ret, MMC_OK);
}

TEST_F(TestMmcBmProxy, InitBm_InvalidOpType)
{
    createConfig.dataOpType = "invalid_type";
    Result ret = InitBmWithConfig();
    EXPECT_EQ(ret, MMC_ERROR);
}

// 总容量 <= 32TB 时不启用 56 位 GVA。
TEST_F(TestMmcBmProxy, InitBm_BelowThreshold_NotEnable56BitsGva)
{
    MockSmemBmResetLastCreate2Option();
    // (localMaxDRAMSize + localMaxHBMSize) * worldSize = (1TB + 0) * 4 = 4TB < 32TB
    createConfig.localDRAMSize = 1ULL << 30ULL;
    createConfig.localMaxDRAMSize = 1ULL << 40ULL;
    createConfig.localHBMSize = 0;
    createConfig.localMaxHBMSize = 0;
    Result ret = InitBmWithConfig();
    EXPECT_EQ(ret, MMC_OK);

    const smem_bm_create_option_t *captured = MockSmemBmGetLastCreate2Option();
    ASSERT_NE(captured, nullptr);
    EXPECT_FALSE(captured->enable56BitsGva);
}

// (localMaxDRAMSize + localMaxHBMSize) * worldSize > 32TB 时自动启用 56 位 GVA
// （mmc 适配层职责，业务层无感）。
TEST_F(TestMmcBmProxy, InitBm_AboveThreshold_AutoEnable56BitsGva)
{
    MockSmemBmResetLastCreate2Option();
    // 9TB * 4 = 36TB > 32TB
    createConfig.localMaxDRAMSize = 9ULL << 40ULL;
    createConfig.localMaxHBMSize = 0;
    Result ret = InitBmWithConfig();
    EXPECT_EQ(ret, MMC_OK);

    const smem_bm_create_option_t *captured = MockSmemBmGetLastCreate2Option();
    ASSERT_NE(captured, nullptr);
    EXPECT_TRUE(captured->enable56BitsGva);
}

// 阈值边界：恰好等于 32TB 时不启用（严格大于阈值才启用）。
TEST_F(TestMmcBmProxy, InitBm_AtThreshold_NotEnable56BitsGva)
{
    MockSmemBmResetLastCreate2Option();
    // 8TB * 4 = 32TB（不大于阈值）
    createConfig.localMaxDRAMSize = 8ULL << 40ULL;
    createConfig.localMaxHBMSize = 0;
    Result ret = InitBmWithConfig();
    EXPECT_EQ(ret, MMC_OK);

    const smem_bm_create_option_t *captured = MockSmemBmGetLastCreate2Option();
    ASSERT_NE(captured, nullptr);
    EXPECT_FALSE(captured->enable56BitsGva);
}

TEST_F(TestMmcBmProxy, DestroyBm_Success)
{
    InitBmWithConfig();
    proxy->DestroyBm();
    EXPECT_EQ(proxy->GetGva(MEDIA_HBM), 0UL);
}

TEST_F(TestMmcBmProxy, DestroyBm_NotStarted)
{
    proxy->DestroyBm();
    EXPECT_EQ(proxy->GetGva(MEDIA_HBM), 0UL);
}

TEST_F(TestMmcBmProxy, Put_OneDimSuccess)
{
    InitBmWithConfig();
    Result ret = proxy->Put(&oneDimBuffer, 0x2000, 1024);
    EXPECT_EQ(ret, MMC_OK);
}

TEST_F(TestMmcBmProxy, Put_OneDimSizeExceed)
{
    InitBmWithConfig();
    oneDimBuffer.len = 2048;
    Result ret = proxy->Put(&oneDimBuffer, 0x2000, 1024);
    EXPECT_EQ(ret, MMC_ERROR);
}

TEST_F(TestMmcBmProxy, Put_NullBuffer)
{
    InitBmWithConfig();
    Result ret = proxy->Put(nullptr, 0x2000, 1024);
    EXPECT_EQ(ret, MMC_ERROR);
}

TEST_F(TestMmcBmProxy, Get_OneDimSuccess)
{
    InitBmWithConfig();
    Result ret = proxy->Get(&oneDimBuffer, 0x2000, 1024);
    EXPECT_EQ(ret, MMC_OK);
}

TEST_F(TestMmcBmProxy, Get_OneDimSizeMismatch)
{
    InitBmWithConfig();
    oneDimBuffer.len = 2048;
    Result ret = proxy->Get(&oneDimBuffer, 0x2000, 1024);
    EXPECT_EQ(ret, MMC_ERROR);
}

TEST_F(TestMmcBmProxy, Get_NullBuffer)
{
    InitBmWithConfig();
    Result ret = proxy->Get(nullptr, 0x2000, 1024);
    EXPECT_EQ(ret, MMC_ERROR);
}

TEST_F(TestMmcBmProxy, Get_NotInitialized)
{
    Result ret = proxy->Get(&oneDimBuffer, 0x2000, 1024);
    EXPECT_EQ(ret, MMC_ERROR);
}

TEST_F(TestMmcBmProxy, Put_NotInitialized)
{
    Result ret = proxy->Put(&oneDimBuffer, 0x2000, 1024);
    EXPECT_EQ(ret, MMC_ERROR);
}

TEST(MmcBmProxyFactory, GetInstance)
{
    auto proxy1 = MmcBmProxyFactory::GetInstance("proxy1");
    auto proxy2 = MmcBmProxyFactory::GetInstance("proxy2");
    auto proxy1_again = MmcBmProxyFactory::GetInstance("proxy1");

    EXPECT_NE(proxy1.Get(), nullptr);
    EXPECT_NE(proxy2.Get(), nullptr);
    EXPECT_EQ(proxy1.Get(), proxy1_again.Get());
}
