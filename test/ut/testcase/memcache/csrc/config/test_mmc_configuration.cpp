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
#include "gtest/gtest.h"

#define private public
#include "mmc_configuration.h"
#include "mmc_def.h"
#undef private

using namespace testing;
using namespace std;
using namespace ock::mmc;

class TestMmcConfiguration : public testing::Test {
public:
    TestMmcConfiguration();

    void SetUp() override;

    void TearDown() override;

protected:
};
TestMmcConfiguration::TestMmcConfiguration() = default;

void TestMmcConfiguration::SetUp() {}

void TestMmcConfiguration::TearDown() {}

static local_config CreateLocalConfigWithCurrentDefaults()
{
    local_config config{};
    SafeCopy("tcp://127.0.0.1:5000", config.meta_service_url, sizeof(config.meta_service_url));
    SafeCopy("tcp://127.0.0.1:6000", config.config_store_url, sizeof(config.config_store_url));
    SafeCopy("info", config.log_level, sizeof(config.log_level));
    config.world_size = 256UL;
    SafeCopy("host_rdma", config.protocol, sizeof(config.protocol));
    SafeCopy("tcp://127.0.0.1:7000", config.hcom_url, sizeof(config.hcom_url));
    SafeCopy("1GB", config.dram_size, sizeof(config.dram_size));
    SafeCopy("0", config.hbm_size, sizeof(config.hbm_size));
    SafeCopy("64GB", config.max_dram_size, sizeof(config.max_dram_size));
    SafeCopy("0", config.max_hbm_size, sizeof(config.max_hbm_size));
    config.client_retry_milliseconds = 0;
    config.client_timeout_seconds = 60UL;
    config.read_thread_pool_size = 32UL;
    config.write_thread_pool_size = 4UL;
    config.aggregate_io = true;
    config.aggregate_num = 122UL;
    config.ubs_io_enable = false;
    SafeCopy("standard", config.memory_pool_mode, sizeof(config.memory_pool_mode));
    config.tls_enable = false;
    config.config_store_tls_enable = false;
    config.hcom_tls_enable = false;
    return config;
}

TEST_F(TestMmcConfiguration, ValidateTLSConfigTest)
{
    mmc_tls_config tlsConfig{};
    tlsConfig.tlsEnable = true;
    // 1. caPath is empty
    auto ret = Configuration::ValidateTLSConfig(tlsConfig);
    ASSERT_NE(ret, MMC_OK);

    // 2. certPath is empty
    SafeCopy("./", tlsConfig.caPath, TLS_PATH_SIZE);
    ret = Configuration::ValidateTLSConfig(tlsConfig);
    ASSERT_NE(ret, MMC_OK);

    // 3. keyPath is empty
    SafeCopy("./", tlsConfig.certPath, TLS_PATH_SIZE);
    ret = Configuration::ValidateTLSConfig(tlsConfig);
    ASSERT_NE(ret, MMC_OK);

    // 4. crlPath is error
    SafeCopy("./", tlsConfig.keyPath, TLS_PATH_SIZE);
    SafeCopy("nonPath", tlsConfig.crlPath, TLS_PATH_SIZE);
    ret = Configuration::ValidateTLSConfig(tlsConfig);
    ASSERT_NE(ret, MMC_OK);

    // 4. keyPassPath is error
    SafeCopy("./", tlsConfig.crlPath, TLS_PATH_SIZE);
    SafeCopy("nonPath", tlsConfig.keyPassPath, TLS_PATH_SIZE);
    ret = Configuration::ValidateTLSConfig(tlsConfig);
    ASSERT_NE(ret, MMC_OK);

    // 5. packagePath is error
    SafeCopy("./", tlsConfig.keyPassPath, TLS_PATH_SIZE);
    SafeCopy("nonPath", tlsConfig.packagePath, TLS_PATH_SIZE);
    ret = Configuration::ValidateTLSConfig(tlsConfig);
    ASSERT_NE(ret, MMC_OK);

    // 6. decrypterLibPath is error
    SafeCopy("./", tlsConfig.packagePath, TLS_PATH_SIZE);
    SafeCopy("nonPath", tlsConfig.decrypterLibPath, TLS_PATH_SIZE);
    ret = Configuration::ValidateTLSConfig(tlsConfig);
    ASSERT_NE(ret, MMC_OK);

    SafeCopy("./", tlsConfig.decrypterLibPath, TLS_PATH_SIZE);
    ret = Configuration::ValidateTLSConfig(tlsConfig);
    ASSERT_EQ(ret, MMC_OK);
}

TEST_F(TestMmcConfiguration, GetLogPathTest)
{
    Configuration configuration;

    auto logPath = configuration.GetLogPath("");
    ASSERT_NE(logPath, "");

    logPath = configuration.GetLogPath("/");
    ASSERT_EQ(logPath, "/");

    logPath = configuration.GetLogPath("./");
    ASSERT_NE(logPath, "/");
}

TEST_F(TestMmcConfiguration, ValidateLocalServiceConfigTest)
{
    // 测试设备类型协议（1GB对齐）
    mmc_local_service_config_t deviceConfig{};
    deviceConfig.logLevel = INFO_LEVEL;
    deviceConfig.accTlsConfig.tlsEnable = false;
    deviceConfig.hcomTlsConfig.tlsEnable = false;
    deviceConfig.configStoreTlsConfig.tlsEnable = false;

    // 测试device_rdma协议，1GB向上对齐
    SafeCopy("device_rdma", deviceConfig.dataOpType, PROTOCOL_SIZE);
    deviceConfig.localDRAMSize = 1536 * 1024 * 1024;       // 1.5GB，应该对齐到2GB
    deviceConfig.localMaxDRAMSize = 3584ULL * 1024 * 1024; // 3.5GB，应该对齐到4GB
    deviceConfig.localHBMSize = 2048ULL * 1024 * 1024;     // 2GB，已经对齐
    deviceConfig.localMaxHBMSize = 4096ULL * 1024 * 1024;  // 4GB，已经对齐

    auto ret = ClientConfig::ValidateLocalServiceConfig(deviceConfig);
    ASSERT_EQ(ret, MMC_OK);
    ASSERT_EQ(deviceConfig.localDRAMSize, 2ULL * 1024 * 1024 * 1024);    // 2GB
    ASSERT_EQ(deviceConfig.localMaxDRAMSize, 4ULL * 1024 * 1024 * 1024); // 4GB
    ASSERT_EQ(deviceConfig.localHBMSize, 2ULL * 1024 * 1024 * 1024);     // 2GB
    ASSERT_EQ(deviceConfig.localMaxHBMSize, 4ULL * 1024 * 1024 * 1024);  // 4GB

    // 测试device_sdma协议，1GB向上对齐
    SafeCopy("device_sdma", deviceConfig.dataOpType, PROTOCOL_SIZE);
    deviceConfig.localDRAMSize = 512ULL * 1024 * 1024;     // 512MB，应该对齐到1GB
    deviceConfig.localMaxDRAMSize = 1024ULL * 1024 * 1024; // 1GB，已经对齐
    deviceConfig.localHBMSize = 128ULL * 1024 * 1024;      // 128MB，应该对齐到1GB
    deviceConfig.localMaxHBMSize = 2048ULL * 1024 * 1024;  // 2GB，已经对齐

    ret = ClientConfig::ValidateLocalServiceConfig(deviceConfig);
    ASSERT_EQ(ret, MMC_OK);
    ASSERT_EQ(deviceConfig.localDRAMSize, 1ULL * 1024 * 1024 * 1024);    // 1GB
    ASSERT_EQ(deviceConfig.localMaxDRAMSize, 1ULL * 1024 * 1024 * 1024); // 1GB
    ASSERT_EQ(deviceConfig.localHBMSize, 1ULL * 1024 * 1024 * 1024);     // 1GB
    ASSERT_EQ(deviceConfig.localMaxHBMSize, 2ULL * 1024 * 1024 * 1024);  // 2GB

    // 测试主机类型协议（2MB对齐）
    mmc_local_service_config_t hostConfig{};
    hostConfig.logLevel = INFO_LEVEL;
    hostConfig.accTlsConfig.tlsEnable = false;
    hostConfig.hcomTlsConfig.tlsEnable = false;
    hostConfig.configStoreTlsConfig.tlsEnable = false;

    // 测试host_rdma协议，2MB向上对齐
    SafeCopy("host_rdma", hostConfig.dataOpType, PROTOCOL_SIZE);
    hostConfig.localDRAMSize = 3ULL * 1024 * 1024;    // 3MB，应该对齐到4MB
    hostConfig.localMaxDRAMSize = 5ULL * 1024 * 1024; // 5MB，应该对齐到6MB
    hostConfig.localHBMSize = 2ULL * 1024 * 1024;     // 2MB，已经对齐
    hostConfig.localMaxHBMSize = 4ULL * 1024 * 1024;  // 4MB，已经对齐

    ret = ClientConfig::ValidateLocalServiceConfig(hostConfig);
    ASSERT_EQ(ret, MMC_OK);
    ASSERT_EQ(hostConfig.localDRAMSize, 4ULL * 1024 * 1024);    // 4MB
    ASSERT_EQ(hostConfig.localMaxDRAMSize, 6ULL * 1024 * 1024); // 6MB
    ASSERT_EQ(hostConfig.localHBMSize, 2ULL * 1024 * 1024);     // 2MB
    ASSERT_EQ(hostConfig.localMaxHBMSize, 4ULL * 1024 * 1024);  // 4MB

    // 测试host_tcp协议，2MB向上对齐
    SafeCopy("host_tcp", hostConfig.dataOpType, PROTOCOL_SIZE);
    hostConfig.localDRAMSize = 1ULL * 1024 * 1024;    // 1MB，应该对齐到2MB
    hostConfig.localMaxDRAMSize = 3ULL * 1024 * 1024; // 3MB，应该对齐到4MB
    hostConfig.localHBMSize = 512ULL * 1024;          // 512KB，应该对齐到2MB
    hostConfig.localMaxHBMSize = 1536ULL * 1024;      // 1.5MB，应该对齐到2MB

    ret = ClientConfig::ValidateLocalServiceConfig(hostConfig);
    ASSERT_EQ(ret, MMC_OK);
    ASSERT_EQ(hostConfig.localDRAMSize, 2ULL * 1024 * 1024);    // 2MB
    ASSERT_EQ(hostConfig.localMaxDRAMSize, 4ULL * 1024 * 1024); // 4MB
    ASSERT_EQ(hostConfig.localHBMSize, 2ULL * 1024 * 1024);     // 2MB
    ASSERT_EQ(hostConfig.localMaxHBMSize, 2ULL * 1024 * 1024);  // 2MB

    // 测试边界情况：配置为0
    mmc_local_service_config_t zeroConfig{};
    zeroConfig.logLevel = INFO_LEVEL;
    zeroConfig.accTlsConfig.tlsEnable = false;
    zeroConfig.hcomTlsConfig.tlsEnable = false;
    zeroConfig.configStoreTlsConfig.tlsEnable = false;
    SafeCopy("host_rdma", zeroConfig.dataOpType, PROTOCOL_SIZE);
    zeroConfig.localDRAMSize = 0;
    zeroConfig.localMaxDRAMSize = 0;
    zeroConfig.localHBMSize = 0;
    zeroConfig.localMaxHBMSize = 0;

    ret = ClientConfig::ValidateLocalServiceConfig(zeroConfig);
    ASSERT_EQ(ret, MMC_INVALID_PARAM); // 两者都为0应该返回错误

    // 测试一个为0，一个不为0的情况
    zeroConfig.localDRAMSize = 2ULL * 1024 * 1024;    // 2MB
    zeroConfig.localMaxDRAMSize = 2ULL * 1024 * 1024; // 2MB
    zeroConfig.localHBMSize = 0;
    zeroConfig.localMaxHBMSize = 0;

    ret = ClientConfig::ValidateLocalServiceConfig(zeroConfig);
    ASSERT_EQ(ret, MMC_OK);
    ASSERT_EQ(zeroConfig.localDRAMSize, 2ULL * 1024 * 1024); // 2MB
    ASSERT_EQ(zeroConfig.localHBMSize, 0);                   // 保持为0
}

TEST_F(TestMmcConfiguration, SetTest)
{
    Configuration configuration;
    std::string testKey = "testKey";

    configuration.Set(testKey, 1);
    ASSERT_EQ(configuration.GetInt({testKey.c_str(), 0}), 0);
    configuration.AddIntConf({testKey, 0});
    configuration.Set(testKey, 1);
    ASSERT_EQ(configuration.GetInt({testKey.c_str(), 0}), 1);

    configuration.Set(testKey, 1.0f);
    ASSERT_EQ(configuration.GetFloat({testKey.c_str(), 0.0f}), 0.0f);
    configuration.mFloatItems[testKey] = 0.0;
    configuration.Set(testKey, 1.0f);
    ASSERT_EQ(configuration.GetFloat({testKey.c_str(), 0.0f}), 1.0f);

    configuration.Set(testKey, testKey);
    ASSERT_EQ(configuration.GetString({testKey.c_str(), ""}), "");
    configuration.AddStrConf({testKey, ""});
    configuration.Set(testKey, testKey);
    ASSERT_EQ(configuration.GetString({testKey.c_str(), testKey.c_str()}), testKey);

    configuration.Set(testKey, true);
    ASSERT_EQ(configuration.GetBool({testKey.c_str(), false}), false);
    configuration.AddBoolConf({testKey, false});
    configuration.Set(testKey, true);
    ASSERT_EQ(configuration.GetBool({testKey.c_str(), false}), true);

    configuration.Set(testKey, 1LU);
    ASSERT_EQ(configuration.GetUInt64({testKey.c_str(), 0LU}), 0LU);
    configuration.AddUInt64Conf({testKey, 0LU});
    configuration.Set(testKey, 1LU);
    ASSERT_EQ(configuration.GetUInt64({testKey.c_str(), 0LU}), 1LU);
}

TEST_F(TestMmcConfiguration, ParseMemSizeTest)
{
    Configuration configuration;
    auto ret = configuration.ParseMemSize("");
    ASSERT_EQ(ret, UINT64_MAX);

    ret = configuration.ParseMemSize("aB");
    ASSERT_EQ(ret, UINT64_MAX);

    ret = configuration.ParseMemSize("1B");
    ASSERT_EQ(ret, 1);

    ret = configuration.ParseMemSize("1KB");
    ASSERT_EQ(ret, KB_MEM_BYTES);

    ret = configuration.ParseMemSize("1MB");
    ASSERT_EQ(ret, MB_MEM_BYTES);

    ret = configuration.ParseMemSize("1GB");
    ASSERT_EQ(ret, GB_MEM_BYTES);

    ret = configuration.ParseMemSize("1TB");
    ASSERT_EQ(ret, TB_MEM_BYTES);
}

TEST_F(TestMmcConfiguration, SetupWithErrorHandlingTest)
{
    ClientConfig clientConfig;
    auto ret = clientConfig.Setup(nullptr);
    ASSERT_FALSE(ret);

    auto config = CreateLocalConfigWithCurrentDefaults();
    SafeCopy("/non/exist/path.conf", config.config_path, sizeof(config.config_path));
    ret = clientConfig.Setup(&config);
    ASSERT_FALSE(ret);
}

TEST_F(TestMmcConfiguration, SetupWithFullConfigTest)
{
    ClientConfig clientConfig;
    auto config = CreateLocalConfigWithCurrentDefaults();
    SafeCopy("tcp://127.0.0.1:5001", config.meta_service_url, sizeof(config.meta_service_url));
    SafeCopy("tcp://127.0.0.1:6001", config.config_store_url, sizeof(config.config_store_url));
    SafeCopy("debug", config.log_level, sizeof(config.log_level));
    config.world_size = 128UL;
    SafeCopy("device_rdma", config.protocol, sizeof(config.protocol));
    SafeCopy("tcp://127.0.0.1:7001", config.hcom_url, sizeof(config.hcom_url));
    SafeCopy("2GB", config.dram_size, sizeof(config.dram_size));
    SafeCopy("4GB", config.hbm_size, sizeof(config.hbm_size));
    SafeCopy("128GB", config.max_dram_size, sizeof(config.max_dram_size));
    SafeCopy("64GB", config.max_hbm_size, sizeof(config.max_hbm_size));
    config.client_retry_milliseconds = 5000UL;
    config.client_timeout_seconds = 120UL;
    config.read_thread_pool_size = 16UL;
    config.write_thread_pool_size = 8UL;
    config.aggregate_io = true;
    config.aggregate_num = 256UL;
    config.ubs_io_enable = true;
    SafeCopy("expanded", config.memory_pool_mode, sizeof(config.memory_pool_mode));

    config.tls_enable = true;
    SafeCopy("/etc/ssl/ca.pem", config.tls_ca_path, sizeof(config.tls_ca_path));
    SafeCopy("/etc/ssl/cert.pem", config.tls_cert_path, sizeof(config.tls_cert_path));
    SafeCopy("/etc/ssl/key.pem", config.tls_key_path, sizeof(config.tls_key_path));
    config.config_store_tls_enable = true;
    SafeCopy("/etc/ssl/cs_ca.pem", config.config_store_tls_ca_path, sizeof(config.config_store_tls_ca_path));
    config.hcom_tls_enable = true;
    SafeCopy("/etc/ssl/hcom_cert.pem", config.hcom_tls_cert_path, sizeof(config.hcom_tls_cert_path));

    const auto ret = clientConfig.Setup(&config);
    ASSERT_TRUE(ret);

    ASSERT_EQ(clientConfig.GetString(ConfConstant::OCK_MMC_META_SERVICE_URL), "tcp://127.0.0.1:5001");
    ASSERT_EQ(clientConfig.GetString(ConfConstant::OCK_MMC_META_SERVICE_CONFIG_STORE_URL), "tcp://127.0.0.1:6001");
    ASSERT_EQ(clientConfig.GetString(ConfConstant::OCK_MMC_LOG_LEVEL), "debug");
    ASSERT_EQ(clientConfig.GetInt(ConfConstant::OKC_MMC_LOCAL_SERVICE_WORLD_SIZE), 128UL);
    ASSERT_EQ(clientConfig.GetString(ConfConstant::OKC_MMC_LOCAL_SERVICE_PROTOCOL), "device_rdma");
    ASSERT_EQ(clientConfig.GetString(ConfConstant::OKC_MMC_LOCAL_SERVICE_BM_HCOM_URL), "tcp://127.0.0.1:7001");
    ASSERT_EQ(clientConfig.GetString(ConfConstant::OKC_MMC_LOCAL_SERVICE_DRAM_SIZE), "2GB");
    ASSERT_EQ(clientConfig.GetString(ConfConstant::OKC_MMC_LOCAL_SERVICE_HBM_SIZE), "4GB");
    ASSERT_EQ(clientConfig.GetString(ConfConstant::OKC_MMC_LOCAL_SERVICE_MAX_DRAM_SIZE), "128GB");
    ASSERT_EQ(clientConfig.GetString(ConfConstant::OKC_MMC_LOCAL_SERVICE_MAX_HBM_SIZE), "64GB");
    ASSERT_EQ(clientConfig.GetInt(ConfConstant::OKC_MMC_CLIENT_RETRY_MILLISECONDS), 5000UL);
    ASSERT_EQ(clientConfig.GetInt(ConfConstant::OCK_MMC_CLIENT_TIMEOUT_SECONDS), 120UL);
    ASSERT_EQ(clientConfig.GetInt(ConfConstant::OCK_MMC_CLIENT_READ_THREAD_POOL_SIZE), 16UL);
    ASSERT_EQ(clientConfig.GetInt(ConfConstant::OCK_MMC_CLIENT_WRITE_THREAD_POOL_SIZE), 8UL);
    ASSERT_EQ(clientConfig.GetBool(ConfConstant::OCK_MMC_CLIENT_AGGREGATE_IO), true);
    ASSERT_EQ(clientConfig.GetInt(ConfConstant::OCK_MMC_CLIENT_AGGREGATE_NUM), 256UL);
    ASSERT_EQ(clientConfig.GetBool(ConfConstant::OCK_MMC_UBS_IO_ENABLE), true);
    ASSERT_EQ(clientConfig.GetString(ConfConstant::OKC_MMC_LOCAL_SERVICE_MEMORY_POOL_MODE), "expanded");

    ASSERT_EQ(clientConfig.GetBool(ConfConstant::OCK_MMC_TLS_ENABLE), true);
    ASSERT_EQ(clientConfig.GetString(ConfConstant::OCK_MMC_TLS_CA_PATH), "/etc/ssl/ca.pem");
    ASSERT_EQ(clientConfig.GetString(ConfConstant::OCK_MMC_TLS_CERT_PATH), "/etc/ssl/cert.pem");
    ASSERT_EQ(clientConfig.GetString(ConfConstant::OCK_MMC_TLS_KEY_PATH), "/etc/ssl/key.pem");
    ASSERT_EQ(clientConfig.GetBool(ConfConstant::OCK_MMC_CS_TLS_ENABLE), true);
    ASSERT_EQ(clientConfig.GetString(ConfConstant::OCK_MMC_CS_TLS_CA_PATH), "/etc/ssl/cs_ca.pem");
    ASSERT_EQ(clientConfig.GetBool(ConfConstant::OCK_MMC_HCOM_TLS_ENABLE), true);
    ASSERT_EQ(clientConfig.GetString(ConfConstant::OCK_MMC_HCOM_TLS_CERT_PATH), "/etc/ssl/hcom_cert.pem");
}

TEST_F(TestMmcConfiguration, SetupWithBoundaryValuesTest)
{
    ClientConfig clientConfig;
    auto config = CreateLocalConfigWithCurrentDefaults();

    config.client_retry_milliseconds = MIN_RETRY_MS;
    config.client_timeout_seconds = MIN_TIMEOUT_SEC;
    config.read_thread_pool_size = MIN_THREAD_POOL_SIZE;
    config.write_thread_pool_size = MIN_THREAD_POOL_SIZE;
    config.aggregate_num = 1UL;

    auto ret = clientConfig.Setup(&config);
    ASSERT_TRUE(ret);

    ASSERT_EQ(clientConfig.GetInt(ConfConstant::OKC_MMC_CLIENT_RETRY_MILLISECONDS), MIN_RETRY_MS);
    ASSERT_EQ(clientConfig.GetInt(ConfConstant::OCK_MMC_CLIENT_TIMEOUT_SECONDS), MIN_TIMEOUT_SEC);
    ASSERT_EQ(clientConfig.GetInt(ConfConstant::OCK_MMC_CLIENT_READ_THREAD_POOL_SIZE), MIN_THREAD_POOL_SIZE);
    ASSERT_EQ(clientConfig.GetInt(ConfConstant::OCK_MMC_CLIENT_WRITE_THREAD_POOL_SIZE), MIN_THREAD_POOL_SIZE);
    ASSERT_EQ(clientConfig.GetInt(ConfConstant::OCK_MMC_CLIENT_AGGREGATE_NUM), 1UL);

    ClientConfig clientConfigMax;
    config.client_retry_milliseconds = MAX_RETRY_MS;
    config.client_timeout_seconds = MAX_TIMEOUT_SEC;
    config.read_thread_pool_size = MAX_THREAD_POOL_SIZE;
    config.write_thread_pool_size = MAX_THREAD_POOL_SIZE;
    config.aggregate_num = MAX_AGGREGATE_NUM;
    config.world_size = MAX_WORLD_SIZE;

    ret = clientConfigMax.Setup(&config);
    ASSERT_TRUE(ret);

    ASSERT_EQ(clientConfigMax.GetInt(ConfConstant::OKC_MMC_CLIENT_RETRY_MILLISECONDS), MAX_RETRY_MS);
    ASSERT_EQ(clientConfigMax.GetInt(ConfConstant::OCK_MMC_CLIENT_TIMEOUT_SECONDS), MAX_TIMEOUT_SEC);
    ASSERT_EQ(clientConfigMax.GetInt(ConfConstant::OCK_MMC_CLIENT_READ_THREAD_POOL_SIZE), MAX_THREAD_POOL_SIZE);
    ASSERT_EQ(clientConfigMax.GetInt(ConfConstant::OCK_MMC_CLIENT_WRITE_THREAD_POOL_SIZE), MAX_THREAD_POOL_SIZE);
    ASSERT_EQ(clientConfigMax.GetInt(ConfConstant::OCK_MMC_CLIENT_AGGREGATE_NUM), MAX_AGGREGATE_NUM);
    ASSERT_EQ(clientConfigMax.GetInt(ConfConstant::OKC_MMC_LOCAL_SERVICE_WORLD_SIZE), MAX_WORLD_SIZE);
}
