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
    local_config config = create_default_local_config();

    EXPECT_STREQ(config.config_path, "");
    EXPECT_STREQ(config.meta_service_url, "tcp://127.0.0.1:5000");
    EXPECT_STREQ(config.config_store_url, "tcp://127.0.0.1:6000");
    EXPECT_STREQ(config.log_level, "info");
    EXPECT_EQ(config.world_size, 256u);
    EXPECT_STREQ(config.protocol, "host_rdma");
    EXPECT_STREQ(config.hcom_url, "tcp://127.0.0.1:7000");
    EXPECT_STREQ(config.dram_size, "1GB");
    EXPECT_STREQ(config.hbm_size, "0");
    EXPECT_STREQ(config.max_dram_size, "64GB");
    EXPECT_STREQ(config.max_hbm_size, "0");
    EXPECT_EQ(config.client_retry_milliseconds, 0u);
    EXPECT_EQ(config.client_timeout_seconds, 60u);
    EXPECT_EQ(config.read_thread_pool_size, 32u);
    EXPECT_EQ(config.write_thread_pool_size, 4u);
    EXPECT_TRUE(config.aggregate_io);
    EXPECT_EQ(config.aggregate_num, 122u);
    EXPECT_FALSE(config.ubs_io_enable);

    EXPECT_FALSE(config.tls_enable);
    EXPECT_STREQ(config.tls_ca_path, "");
    EXPECT_STREQ(config.tls_ca_crl_path, "");
    EXPECT_STREQ(config.tls_cert_path, "");
    EXPECT_STREQ(config.tls_key_path, "");
    EXPECT_STREQ(config.tls_key_pass_path, "");
    EXPECT_STREQ(config.tls_package_path, "");
    EXPECT_STREQ(config.tls_decrypter_path, "");

    EXPECT_FALSE(config.config_store_tls_enable);
    EXPECT_STREQ(config.config_store_tls_ca_path, "");
    EXPECT_STREQ(config.config_store_tls_ca_crl_path, "");
    EXPECT_STREQ(config.config_store_tls_cert_path, "");
    EXPECT_STREQ(config.config_store_tls_key_path, "");
    EXPECT_STREQ(config.config_store_tls_key_pass_path, "");
    EXPECT_STREQ(config.config_store_tls_package_path, "");
    EXPECT_STREQ(config.config_store_tls_decrypter_path, "");

    EXPECT_FALSE(config.hcom_tls_enable);
    EXPECT_STREQ(config.hcom_tls_ca_path, "");
    EXPECT_STREQ(config.hcom_tls_ca_crl_path, "");
    EXPECT_STREQ(config.hcom_tls_cert_path, "");
    EXPECT_STREQ(config.hcom_tls_key_path, "");
    EXPECT_STREQ(config.hcom_tls_key_pass_path, "");
    EXPECT_STREQ(config.hcom_tls_decrypter_path, "");
}

TEST_F(TestLocalConfigUtils, LocalConfigToStringReturnsExpectedFormat)
{
    local_config config{};
    SafeCopy("/tmp/local.conf", config.config_path, sizeof(config.config_path));
    SafeCopy("tcp://10.0.0.1:5000", config.meta_service_url, sizeof(config.meta_service_url));
    SafeCopy("tcp://10.0.0.2:6000", config.config_store_url, sizeof(config.config_store_url));
    SafeCopy("debug", config.log_level, sizeof(config.log_level));
    config.world_size = 8UL;
    SafeCopy("device_rdma", config.protocol, sizeof(config.protocol));
    SafeCopy("tcp://10.0.0.3:7000", config.hcom_url, sizeof(config.hcom_url));
    SafeCopy("2GB", config.dram_size, sizeof(config.dram_size));
    SafeCopy("4GB", config.hbm_size, sizeof(config.hbm_size));
    SafeCopy("128GB", config.max_dram_size, sizeof(config.max_dram_size));
    SafeCopy("256GB", config.max_hbm_size, sizeof(config.max_hbm_size));
    config.client_retry_milliseconds = 1234UL;
    config.client_timeout_seconds = 56UL;
    config.read_thread_pool_size = 12UL;
    config.write_thread_pool_size = 34UL;
    config.aggregate_io = false;
    config.aggregate_num = 78UL;
    config.ubs_io_enable = true;
    config.tls_enable = true;
    SafeCopy("/tls/ca.pem", config.tls_ca_path, sizeof(config.tls_ca_path));
    SafeCopy("/tls/ca.crl", config.tls_ca_crl_path, sizeof(config.tls_ca_crl_path));
    SafeCopy("/tls/cert.pem", config.tls_cert_path, sizeof(config.tls_cert_path));
    SafeCopy("/tls/key.pem", config.tls_key_path, sizeof(config.tls_key_path));
    SafeCopy("/tls/key.pass", config.tls_key_pass_path, sizeof(config.tls_key_pass_path));
    SafeCopy("/tls/pkg", config.tls_package_path, sizeof(config.tls_package_path));
    SafeCopy("/tls/dec.so", config.tls_decrypter_path, sizeof(config.tls_decrypter_path));
    config.config_store_tls_enable = true;
    SafeCopy("/cs/ca.pem", config.config_store_tls_ca_path, sizeof(config.config_store_tls_ca_path));
    SafeCopy("/cs/ca.crl", config.config_store_tls_ca_crl_path, sizeof(config.config_store_tls_ca_crl_path));
    SafeCopy("/cs/cert.pem", config.config_store_tls_cert_path, sizeof(config.config_store_tls_cert_path));
    SafeCopy("/cs/key.pem", config.config_store_tls_key_path, sizeof(config.config_store_tls_key_path));
    SafeCopy("/cs/key.pass", config.config_store_tls_key_pass_path, sizeof(config.config_store_tls_key_pass_path));
    SafeCopy("/cs/pkg", config.config_store_tls_package_path, sizeof(config.config_store_tls_package_path));
    SafeCopy("/cs/dec.so", config.config_store_tls_decrypter_path, sizeof(config.config_store_tls_decrypter_path));
    config.hcom_tls_enable = true;
    SafeCopy("/hcom/ca.pem", config.hcom_tls_ca_path, sizeof(config.hcom_tls_ca_path));
    SafeCopy("/hcom/ca.crl", config.hcom_tls_ca_crl_path, sizeof(config.hcom_tls_ca_crl_path));
    SafeCopy("/hcom/cert.pem", config.hcom_tls_cert_path, sizeof(config.hcom_tls_cert_path));
    SafeCopy("/hcom/key.pem", config.hcom_tls_key_path, sizeof(config.hcom_tls_key_path));
    SafeCopy("/hcom/key.pass", config.hcom_tls_key_pass_path, sizeof(config.hcom_tls_key_pass_path));
    SafeCopy("/hcom/dec.so", config.hcom_tls_decrypter_path, sizeof(config.hcom_tls_decrypter_path));

    const std::string expected = "LocalConfig {\n"
                                 "  meta_service_url: tcp://10.0.0.1:5000\n"
                                 "  config_store_url: tcp://10.0.0.2:6000\n"
                                 "  log_level: debug\n"
                                 "  world_size: 8\n"
                                 "  protocol: device_rdma\n"
                                 "  hcom_url: tcp://10.0.0.3:7000\n"
                                 "  dram_size: 2GB\n"
                                 "  hbm_size: 4GB\n"
                                 "  max_dram_size: 128GB\n"
                                 "  max_hbm_size: 256GB\n"
                                 "  client_retry_milliseconds: 1234\n"
                                 "  client_timeout_seconds: 56\n"
                                 "  read_thread_pool_size: 12\n"
                                 "  write_thread_pool_size: 34\n"
                                 "  aggregate_io: false\n"
                                 "  aggregate_num: 78\n"
                                 "  ubs_io_enable: true\n"
                                 "  tls_enable: true\n"
                                 "  tls_ca_path: /tls/ca.pem\n"
                                 "  tls_ca_crl_path: /tls/ca.crl\n"
                                 "  tls_cert_path: /tls/cert.pem\n"
                                 "  tls_key_path: /tls/key.pem\n"
                                 "  tls_key_pass_path: /tls/key.pass\n"
                                 "  tls_package_path: /tls/pkg\n"
                                 "  tls_decrypter_path: /tls/dec.so\n"
                                 "  config_store_tls_enable: true\n"
                                 "  config_store_tls_ca_path: /cs/ca.pem\n"
                                 "  config_store_tls_ca_crl_path: /cs/ca.crl\n"
                                 "  config_store_tls_cert_path: /cs/cert.pem\n"
                                 "  config_store_tls_key_path: /cs/key.pem\n"
                                 "  config_store_tls_key_pass_path: /cs/key.pass\n"
                                 "  config_store_tls_package_path: /cs/pkg\n"
                                 "  config_store_tls_decrypter_path: /cs/dec.so\n"
                                 "  hcom_tls_enable: true\n"
                                 "  hcom_tls_ca_path: /hcom/ca.pem\n"
                                 "  hcom_tls_ca_crl_path: /hcom/ca.crl\n"
                                 "  hcom_tls_cert_path: /hcom/cert.pem\n"
                                 "  hcom_tls_key_path: /hcom/key.pem\n"
                                 "  hcom_tls_key_pass_path: /hcom/key.pass\n"
                                 "  hcom_tls_decrypter_path: /hcom/dec.so\n"
                                 "}";

    EXPECT_EQ(local_config_to_string(config), expected);
}
