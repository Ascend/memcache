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
#include <iostream>
#include "gtest/gtest.h"
#include "mmc_def.h"
#include "mmc_service.h"
#include "mmc_client.h"
#include "mmc_mem_blob.h"
#include "mmc_blob_allocator.h"
#include "mmc_types.h"

using namespace testing;
using namespace std;
using namespace ock::mmc;

static const uint32_t UT_READ_POOL_NUM = 32U;
static const uint32_t UT_WRITE_POOL_NUM = 4U;

class TestUbsIoEnabled : public testing::Test {
public:
    TestUbsIoEnabled();

    void SetUp() override;

    void TearDown() override;

protected:
    std::shared_ptr<ock::mmc::MmcBlobAllocator> allocator;
};

TestUbsIoEnabled::TestUbsIoEnabled() {}

void TestUbsIoEnabled::SetUp()
{
    cout << "TestUbsIoEnabled SetUp" << endl;
}

void TestUbsIoEnabled::TearDown()
{
    cout << "TestUbsIoEnabled TearDown" << endl;
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
            arr[i] = -base;
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
        if (arr1[i] != arr2[i])
            return false;
    }
    return true;
}

TEST_F(TestUbsIoEnabled, ClientInitWithUbsIoEnabled)
{
    mmc_client_config_t clientConfig{};
    clientConfig.logLevel = INFO_LEVEL;
    clientConfig.ubsIoEnable = true;
    clientConfig.readThreadPoolNum = UT_READ_POOL_NUM;
    clientConfig.writeThreadPoolNum = UT_WRITE_POOL_NUM;
    int32_t ret = mmcc_init(&clientConfig);
    ASSERT_EQ(ret, ock::mmc::MMC_OK);

    mmcc_uninit();
}

TEST_F(TestUbsIoEnabled, PutAndGetWithUbsIoFallback)
{
    std::string metaUrl = "tcp://127.0.0.1:5970";
    std::string bmUrl = "tcp://127.0.0.1:5982";
    std::string hcomUrl = "tcp://127.0.0.1:5983";

    mmc_meta_service_config_t metaServiceConfig{};
    metaServiceConfig.logLevel = INFO_LEVEL;
    metaServiceConfig.logRotationFileSize = 2 * 1024 * 1024;
    metaServiceConfig.logRotationFileCount = 20;
    metaServiceConfig.accTlsConfig.tlsEnable = false;
    metaServiceConfig.evictThresholdHigh = 80;
    metaServiceConfig.evictThresholdLow = 60;
    metaServiceConfig.haEnable = false;
    metaServiceConfig.ubsIoEnable = true;
    UrlStringToChar(metaUrl, metaServiceConfig.discoveryURL);
    UrlStringToChar(bmUrl, metaServiceConfig.configStoreURL);
    mmc_meta_service_t meta_service = mmcs_meta_service_start(&metaServiceConfig);
    ASSERT_TRUE(meta_service != nullptr);

    uint64_t totalSize = SIZE_32K * 10;

    mmc_local_service_config_t localServiceConfig = {
        "", 0, 0,  1, "",      "", 0,  "device_sdma", totalSize, totalSize, totalSize, totalSize,
        0, {}, 0, nullptr, {}, {}, false};
    localServiceConfig.logLevel = INFO_LEVEL;
    localServiceConfig.accTlsConfig.tlsEnable = false;
    UrlStringToChar(metaUrl, localServiceConfig.discoveryURL);
    UrlStringToChar(bmUrl, localServiceConfig.bmIpPort);
    UrlStringToChar(hcomUrl, localServiceConfig.bmHcomUrl);
    mmc_local_service_t local_service = mmcs_local_service_start(&localServiceConfig);
    ASSERT_TRUE(local_service != nullptr);

    mmc_client_config_t clientConfig;
    clientConfig.logLevel = INFO_LEVEL;
    clientConfig.tlsConfig.tlsEnable = false;
    clientConfig.rankId = 0;
    clientConfig.ubsIoEnable = true;
    clientConfig.readThreadPoolNum = UT_READ_POOL_NUM;
    clientConfig.writeThreadPoolNum = UT_WRITE_POOL_NUM;
    UrlStringToChar(metaUrl, clientConfig.discoveryURL);
    int32_t ret = mmcc_init(&clientConfig);
    ASSERT_EQ(ret, 0);

    void *hostSrc = malloc(SIZE_32K);
    void *hostDest = malloc(SIZE_32K);
    GenerateData(hostSrc, 1);

    mmc_buffer buffer;
    buffer.addr = (uint64_t)hostSrc;
    buffer.type = 0;
    buffer.offset = 0;
    buffer.len = SIZE_32K;

    mmc_put_options options{0, NATIVE_AFFINITY, 1, {}};
    std::fill_n(options.preferredLocalServiceIDs, MAX_BLOB_COPIES, -1);
    std::string key = "ubs_io_test_key_1";
    ret = mmcc_put(key.c_str(), &buffer, options, 0);
    EXPECT_EQ(ret, 0);

    mmc_buffer readBuffer;
    readBuffer.addr = (uint64_t)hostDest;
    readBuffer.type = 0;
    readBuffer.offset = 0;
    readBuffer.len = SIZE_32K;

    ret = mmcc_get(key.c_str(), &readBuffer, 0);
    EXPECT_EQ(ret, 0);

    ret = mmcc_remove(key.c_str(), 0);
    EXPECT_EQ(ret, 0);

    free(hostSrc);
    free(hostDest);
    sleep(1);
    mmcs_local_service_stop(local_service);
    mmcc_uninit();
    mmcs_meta_service_stop(meta_service);
}

TEST_F(TestUbsIoEnabled, BatchGetWithUbsIoFallback)
{
    std::string metaUrl = "tcp://127.0.0.1:5971";
    std::string bmUrl = "tcp://127.0.0.1:5984";
    std::string hcomUrl = "tcp://127.0.0.1:5985";

    mmc_meta_service_config_t metaServiceConfig{};
    metaServiceConfig.logLevel = INFO_LEVEL;
    metaServiceConfig.logRotationFileSize = 2 * 1024 * 1024;
    metaServiceConfig.logRotationFileCount = 20;
    metaServiceConfig.accTlsConfig.tlsEnable = false;
    metaServiceConfig.evictThresholdHigh = 80;
    metaServiceConfig.evictThresholdLow = 60;
    metaServiceConfig.haEnable = false;
    metaServiceConfig.ubsIoEnable = true;
    UrlStringToChar(metaUrl, metaServiceConfig.discoveryURL);
    UrlStringToChar(bmUrl, metaServiceConfig.configStoreURL);
    mmc_meta_service_t meta_service = mmcs_meta_service_start(&metaServiceConfig);
    ASSERT_TRUE(meta_service != nullptr);

    uint64_t totalSize = SIZE_32K * 10;

    mmc_local_service_config_t localServiceConfig = {
        "", 0, 0,  1, "",      "", 0,  "device_sdma", totalSize, totalSize, totalSize, totalSize,
        0, {}, 0, nullptr, {}, {}, false};
    localServiceConfig.logLevel = INFO_LEVEL;
    localServiceConfig.accTlsConfig.tlsEnable = false;
    UrlStringToChar(metaUrl, localServiceConfig.discoveryURL);
    UrlStringToChar(bmUrl, localServiceConfig.bmIpPort);
    UrlStringToChar(hcomUrl, localServiceConfig.bmHcomUrl);
    mmc_local_service_t local_service = mmcs_local_service_start(&localServiceConfig);
    ASSERT_TRUE(local_service != nullptr);

    mmc_client_config_t clientConfig;
    clientConfig.logLevel = INFO_LEVEL;
    clientConfig.tlsConfig.tlsEnable = false;
    clientConfig.rankId = 0;
    clientConfig.ubsIoEnable = true;
    clientConfig.readThreadPoolNum = UT_READ_POOL_NUM;
    clientConfig.writeThreadPoolNum = UT_WRITE_POOL_NUM;
    UrlStringToChar(metaUrl, clientConfig.discoveryURL);
    int32_t ret = mmcc_init(&clientConfig);
    ASSERT_EQ(ret, 0);

    const uint32_t keyCount = 3;
    const char *keys[keyCount] = {"batch_ubs_key1", "batch_ubs_key2", "batch_ubs_key3"};
    void *srcData[keyCount];
    void *destData[keyCount];
    mmc_buffer srcBufs[keyCount];
    mmc_buffer destBufs[keyCount];

    for (uint32_t i = 0; i < keyCount; i++) {
        srcData[i] = malloc(SIZE_32K);
        destData[i] = calloc(1, SIZE_32K);
        GenerateData(srcData[i], i + 1);
        srcBufs[i].addr = (uint64_t)srcData[i];
        srcBufs[i].type = 0;
        srcBufs[i].offset = 0;
        srcBufs[i].len = SIZE_32K;
        destBufs[i].addr = (uint64_t)destData[i];
        destBufs[i].type = 0;
        destBufs[i].offset = 0;
        destBufs[i].len = SIZE_32K;
    }

    mmc_put_options options{0, NATIVE_AFFINITY, 1, {}};
    std::fill_n(options.preferredLocalServiceIDs, MAX_BLOB_COPIES, -1);
    std::vector<int> putResults(keyCount, -1);
    ret = mmcc_batch_put(keys, keyCount, srcBufs, options, 0, putResults.data());
    EXPECT_EQ(ret, 0);
    for (uint32_t i = 0; i < keyCount; i++) {
        EXPECT_EQ(putResults[i], 0);
    }

    std::vector<int> getResults(keyCount, -1);
    ret = mmcc_batch_get(keys, keyCount, destBufs, 0, getResults.data());
    EXPECT_EQ(ret, 0);

    for (uint32_t i = 0; i < keyCount; i++) {
        EXPECT_EQ(getResults[i], 0);
    }

    std::vector<int> removeResults(keyCount, -1);
    ret = mmcc_batch_remove(keys, keyCount, removeResults.data(), 0);
    EXPECT_EQ(ret, 0);

    for (uint32_t i = 0; i < keyCount; i++) {
        free(srcData[i]);
        free(destData[i]);
    }

    sleep(1);
    mmcs_local_service_stop(local_service);
    mmcc_uninit();
    mmcs_meta_service_stop(meta_service);
}

TEST_F(TestUbsIoEnabled, ExistOperationsWithUbsIo)
{
    std::string metaUrl = "tcp://127.0.0.1:5972";
    std::string bmUrl = "tcp://127.0.0.1:5986";
    std::string hcomUrl = "tcp://127.0.0.1:5987";

    mmc_meta_service_config_t metaServiceConfig{};
    metaServiceConfig.logLevel = INFO_LEVEL;
    metaServiceConfig.logRotationFileSize = 2 * 1024 * 1024;
    metaServiceConfig.logRotationFileCount = 20;
    metaServiceConfig.accTlsConfig.tlsEnable = false;
    metaServiceConfig.evictThresholdHigh = 80;
    metaServiceConfig.evictThresholdLow = 60;
    metaServiceConfig.haEnable = false;
    metaServiceConfig.ubsIoEnable = true;
    UrlStringToChar(metaUrl, metaServiceConfig.discoveryURL);
    UrlStringToChar(bmUrl, metaServiceConfig.configStoreURL);
    mmc_meta_service_t meta_service = mmcs_meta_service_start(&metaServiceConfig);
    ASSERT_TRUE(meta_service != nullptr);

    uint64_t totalSize = SIZE_32K * 10;

    mmc_local_service_config_t localServiceConfig = {
        "", 0, 0,  1, "",      "", 0,  "device_sdma", totalSize, totalSize, totalSize, totalSize,
        0, {}, 0, nullptr, {}, {}, false};
    localServiceConfig.logLevel = INFO_LEVEL;
    localServiceConfig.accTlsConfig.tlsEnable = false;
    UrlStringToChar(metaUrl, localServiceConfig.discoveryURL);
    UrlStringToChar(bmUrl, localServiceConfig.bmIpPort);
    UrlStringToChar(hcomUrl, localServiceConfig.bmHcomUrl);
    mmc_local_service_t local_service = mmcs_local_service_start(&localServiceConfig);
    ASSERT_TRUE(local_service != nullptr);

    mmc_client_config_t clientConfig;
    clientConfig.logLevel = INFO_LEVEL;
    clientConfig.tlsConfig.tlsEnable = false;
    clientConfig.rankId = 0;
    clientConfig.ubsIoEnable = true;
    clientConfig.readThreadPoolNum = UT_READ_POOL_NUM;
    clientConfig.writeThreadPoolNum = UT_WRITE_POOL_NUM;
    UrlStringToChar(metaUrl, clientConfig.discoveryURL);
    int32_t ret = mmcc_init(&clientConfig);
    ASSERT_EQ(ret, 0);

    void *hostSrc = malloc(SIZE_32K);
    GenerateData(hostSrc, 1);

    mmc_buffer buffer;
    buffer.addr = (uint64_t)hostSrc;
    buffer.type = 0;
    buffer.offset = 0;
    buffer.len = SIZE_32K;

    mmc_put_options options{0, NATIVE_AFFINITY, 1, {}};
    std::fill_n(options.preferredLocalServiceIDs, MAX_BLOB_COPIES, -1);
    std::string key = "exist_ubs_test_key";
    ret = mmcc_put(key.c_str(), &buffer, options, 0);
    EXPECT_EQ(ret, 0);

    ret = mmcc_exist(key.c_str(), 0);
    EXPECT_EQ(ret, MMC_OK);

    const char *keys[] = {key.c_str(), "non_existent_ubs_key"};
    int32_t existResults[2] = {0};
    ret = mmcc_batch_exist(keys, 2, existResults, 0);
    EXPECT_EQ(ret, 0);
    EXPECT_EQ(existResults[0], MMC_OK);

    ret = mmcc_remove(key.c_str(), 0);
    EXPECT_EQ(ret, 0);

    free(hostSrc);
    sleep(1);
    mmcs_local_service_stop(local_service);
    mmcc_uninit();
    mmcs_meta_service_stop(meta_service);
}

TEST_F(TestUbsIoEnabled, QueryOperationsWithUbsIo)
{
    std::string metaUrl = "tcp://127.0.0.1:5973";
    std::string bmUrl = "tcp://127.0.0.1:5988";
    std::string hcomUrl = "tcp://127.0.0.1:5989";

    mmc_meta_service_config_t metaServiceConfig{};
    metaServiceConfig.logLevel = INFO_LEVEL;
    metaServiceConfig.logRotationFileSize = 2 * 1024 * 1024;
    metaServiceConfig.logRotationFileCount = 20;
    metaServiceConfig.accTlsConfig.tlsEnable = false;
    metaServiceConfig.evictThresholdHigh = 80;
    metaServiceConfig.evictThresholdLow = 60;
    metaServiceConfig.haEnable = false;
    metaServiceConfig.ubsIoEnable = true;
    UrlStringToChar(metaUrl, metaServiceConfig.discoveryURL);
    UrlStringToChar(bmUrl, metaServiceConfig.configStoreURL);
    mmc_meta_service_t meta_service = mmcs_meta_service_start(&metaServiceConfig);
    ASSERT_TRUE(meta_service != nullptr);

    uint64_t totalSize = SIZE_32K * 10;

    mmc_local_service_config_t localServiceConfig = {
        "", 0, 0,  1, "",      "", 0,  "device_sdma", totalSize, totalSize, totalSize, totalSize,
        0, {}, 0, nullptr, {}, {}, false};
    localServiceConfig.logLevel = INFO_LEVEL;
    localServiceConfig.accTlsConfig.tlsEnable = false;
    UrlStringToChar(metaUrl, localServiceConfig.discoveryURL);
    UrlStringToChar(bmUrl, localServiceConfig.bmIpPort);
    UrlStringToChar(hcomUrl, localServiceConfig.bmHcomUrl);
    mmc_local_service_t local_service = mmcs_local_service_start(&localServiceConfig);
    ASSERT_TRUE(local_service != nullptr);

    mmc_client_config_t clientConfig;
    clientConfig.logLevel = INFO_LEVEL;
    clientConfig.tlsConfig.tlsEnable = false;
    clientConfig.rankId = 0;
    clientConfig.ubsIoEnable = true;
    clientConfig.readThreadPoolNum = UT_READ_POOL_NUM;
    clientConfig.writeThreadPoolNum = UT_WRITE_POOL_NUM;
    UrlStringToChar(metaUrl, clientConfig.discoveryURL);
    int32_t ret = mmcc_init(&clientConfig);
    ASSERT_EQ(ret, 0);

    void *hostSrc = malloc(SIZE_32K);
    GenerateData(hostSrc, 1);

    mmc_buffer buffer;
    buffer.addr = (uint64_t)hostSrc;
    buffer.type = 0;
    buffer.offset = 0;
    buffer.len = SIZE_32K;

    mmc_put_options options{0, NATIVE_AFFINITY, 1, {}};
    std::fill_n(options.preferredLocalServiceIDs, MAX_BLOB_COPIES, -1);
    std::string key = "query_ubs_test_key";
    ret = mmcc_put(key.c_str(), &buffer, options, 0);
    EXPECT_EQ(ret, 0);

    mmc_data_info info;
    ret = mmcc_query(key.c_str(), &info, 0);
    EXPECT_EQ(ret, 0);
    EXPECT_TRUE(info.valid);
    EXPECT_EQ(info.size, SIZE_32K);

    const char *keys[] = {key.c_str()};
    mmc_data_info infos[1];
    ret = mmcc_batch_query(keys, 1, infos, 0);
    EXPECT_EQ(ret, 0);
    EXPECT_TRUE(infos[0].valid);

    ret = mmcc_remove(key.c_str(), 0);
    EXPECT_EQ(ret, 0);

    free(hostSrc);
    sleep(1);
    mmcs_local_service_stop(local_service);
    mmcc_uninit();
    mmcs_meta_service_stop(meta_service);
}

TEST_F(TestUbsIoEnabled, UbsIoFallbackWhenMemcacheFull)
{
    std::string metaUrl = "tcp://127.0.0.1:5974";
    std::string bmUrl = "tcp://127.0.0.1:5990";
    std::string hcomUrl = "tcp://127.0.0.1:5991";

    mmc_meta_service_config_t metaServiceConfig{};
    metaServiceConfig.logLevel = INFO_LEVEL;
    metaServiceConfig.logRotationFileSize = 2 * 1024 * 1024;
    metaServiceConfig.logRotationFileCount = 20;
    metaServiceConfig.accTlsConfig.tlsEnable = false;
    metaServiceConfig.evictThresholdHigh = 80;
    metaServiceConfig.evictThresholdLow = 60;
    metaServiceConfig.haEnable = false;
    metaServiceConfig.ubsIoEnable = true;
    UrlStringToChar(metaUrl, metaServiceConfig.discoveryURL);
    UrlStringToChar(bmUrl, metaServiceConfig.configStoreURL);
    mmc_meta_service_t meta_service = mmcs_meta_service_start(&metaServiceConfig);
    ASSERT_TRUE(meta_service != nullptr);

    uint64_t totalSize = SIZE_32K * 2;

    mmc_local_service_config_t localServiceConfig = {
        "", 0, 0,  1, "",      "", 0,  "device_sdma", totalSize, totalSize, totalSize, totalSize,
        0, {}, 0, nullptr, {}, {}, false};
    localServiceConfig.logLevel = INFO_LEVEL;
    localServiceConfig.accTlsConfig.tlsEnable = false;
    UrlStringToChar(metaUrl, localServiceConfig.discoveryURL);
    UrlStringToChar(bmUrl, localServiceConfig.bmIpPort);
    UrlStringToChar(hcomUrl, localServiceConfig.bmHcomUrl);
    mmc_local_service_t local_service = mmcs_local_service_start(&localServiceConfig);
    ASSERT_TRUE(local_service != nullptr);

    mmc_client_config_t clientConfig;
    clientConfig.logLevel = INFO_LEVEL;
    clientConfig.tlsConfig.tlsEnable = false;
    clientConfig.rankId = 0;
    clientConfig.ubsIoEnable = true;
    clientConfig.readThreadPoolNum = UT_READ_POOL_NUM;
    clientConfig.writeThreadPoolNum = UT_WRITE_POOL_NUM;
    UrlStringToChar(metaUrl, clientConfig.discoveryURL);
    int32_t ret = mmcc_init(&clientConfig);
    ASSERT_EQ(ret, 0);

    void *hostSrc = malloc(SIZE_32K);
    void *hostDest = malloc(SIZE_32K);
    GenerateData(hostSrc, 1);

    mmc_buffer buffer;
    buffer.addr = (uint64_t)hostSrc;
    buffer.type = 0;
    buffer.offset = 0;
    buffer.len = SIZE_32K;

    mmc_put_options options{0, NATIVE_AFFINITY, 1, {}};
    std::fill_n(options.preferredLocalServiceIDs, MAX_BLOB_COPIES, -1);

    std::vector<std::string> keys;
    for (int i = 0; i < 5; i++) {
        std::string key = "overflow_ubs_key_" + std::to_string(i);
        ret = mmcc_put(key.c_str(), &buffer, options, 0);
        keys.emplace_back(key);
        usleep(1000 * 100);
    }

    mmc_buffer readBuffer;
    readBuffer.addr = (uint64_t)hostDest;
    readBuffer.type = 0;
    readBuffer.offset = 0;
    readBuffer.len = SIZE_32K;

    for (const auto &key : keys) {
        ret = mmcc_get(key.c_str(), &readBuffer, 0);
        mmcc_remove(key.c_str(), 0);
    }

    free(hostSrc);
    free(hostDest);
    sleep(1);
    mmcs_local_service_stop(local_service);
    mmcc_uninit();
    mmcs_meta_service_stop(meta_service);
}

TEST_F(TestUbsIoEnabled, UbsIoDisabledCompare)
{
    std::string metaUrl = "tcp://127.0.0.1:5975";
    std::string bmUrl = "tcp://127.0.0.1:5992";
    std::string hcomUrl = "tcp://127.0.0.1:5993";

    mmc_meta_service_config_t metaServiceConfig{};
    metaServiceConfig.logLevel = INFO_LEVEL;
    metaServiceConfig.logRotationFileSize = 2 * 1024 * 1024;
    metaServiceConfig.logRotationFileCount = 20;
    metaServiceConfig.accTlsConfig.tlsEnable = false;
    metaServiceConfig.evictThresholdHigh = 80;
    metaServiceConfig.evictThresholdLow = 60;
    metaServiceConfig.haEnable = false;
    metaServiceConfig.ubsIoEnable = false;
    UrlStringToChar(metaUrl, metaServiceConfig.discoveryURL);
    UrlStringToChar(bmUrl, metaServiceConfig.configStoreURL);
    mmc_meta_service_t meta_service = mmcs_meta_service_start(&metaServiceConfig);
    ASSERT_TRUE(meta_service != nullptr);

    uint64_t totalSize = SIZE_32K * 10;

    mmc_local_service_config_t localServiceConfig = {
        "", 0, 0,  1, "",      "", 0,  "device_sdma", totalSize, totalSize, totalSize, totalSize,
        0, {}, 0, nullptr, {}, {}, false};
    localServiceConfig.logLevel = INFO_LEVEL;
    localServiceConfig.accTlsConfig.tlsEnable = false;
    UrlStringToChar(metaUrl, localServiceConfig.discoveryURL);
    UrlStringToChar(bmUrl, localServiceConfig.bmIpPort);
    UrlStringToChar(hcomUrl, localServiceConfig.bmHcomUrl);
    mmc_local_service_t local_service = mmcs_local_service_start(&localServiceConfig);
    ASSERT_TRUE(local_service != nullptr);

    mmc_client_config_t clientConfig;
    clientConfig.logLevel = INFO_LEVEL;
    clientConfig.tlsConfig.tlsEnable = false;
    clientConfig.rankId = 0;
    clientConfig.ubsIoEnable = false;
    clientConfig.readThreadPoolNum = UT_READ_POOL_NUM;
    clientConfig.writeThreadPoolNum = UT_WRITE_POOL_NUM;
    UrlStringToChar(metaUrl, clientConfig.discoveryURL);
    int32_t ret = mmcc_init(&clientConfig);
    ASSERT_EQ(ret, 0);

    void *hostSrc = malloc(SIZE_32K);
    void *hostDest = malloc(SIZE_32K);
    GenerateData(hostSrc, 1);

    mmc_buffer buffer;
    buffer.addr = (uint64_t)hostSrc;
    buffer.type = 0;
    buffer.offset = 0;
    buffer.len = SIZE_32K;

    mmc_put_options options{0, NATIVE_AFFINITY, 1, {}};
    std::fill_n(options.preferredLocalServiceIDs, MAX_BLOB_COPIES, -1);
    std::string key = "no_ubs_test_key";
    ret = mmcc_put(key.c_str(), &buffer, options, 0);
    EXPECT_EQ(ret, 0);

    mmc_buffer readBuffer;
    readBuffer.addr = (uint64_t)hostDest;
    readBuffer.type = 0;
    readBuffer.offset = 0;
    readBuffer.len = SIZE_32K;

    ret = mmcc_get(key.c_str(), &readBuffer, 0);
    EXPECT_EQ(ret, 0);

    ret = mmcc_remove(key.c_str(), 0);
    EXPECT_EQ(ret, 0);

    free(hostSrc);
    free(hostDest);
    sleep(1);
    mmcs_local_service_stop(local_service);
    mmcc_uninit();
    mmcs_meta_service_stop(meta_service);
}
