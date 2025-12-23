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
#ifndef MEM_FABRIC_MMC_CONFIG_CONST_H
#define MEM_FABRIC_MMC_CONFIG_CONST_H

#include <utility>

namespace ock {
namespace mmc {
namespace ConfConstant {
// add configuration here with default values
constexpr auto OCK_MMC_META_SERVICE_URL = std::make_pair("ock.mmc.meta_service_url", "tcp://127.0.0.1:5000");
constexpr auto OCK_MMC_META_SERVICE_CONFIG_STORE_URL = std::make_pair("ock.mmc.meta_service.config_store_url",
                                                                      "tcp://127.0.0.1:6000");
constexpr auto OCK_MMC_META_SERVICE_HTTP_URL = std::make_pair("ock.mmc.meta_service.metrics_url", "127.0.0.1:8000");
constexpr auto OCK_MMC_META_HA_ENABLE = std::make_pair("ock.mmc.meta.ha.enable", false);
constexpr auto OKC_MMC_EVICT_THRESHOLD_HIGH = std::make_pair("ock.mmc.evict_threshold_high", 70);
constexpr auto OKC_MMC_EVICT_THRESHOLD_LOW = std::make_pair("ock.mmc.evict_threshold_low", 60);
constexpr auto OCK_MMC_LOG_LEVEL = std::make_pair("ock.mmc.log_level", "info");
constexpr auto OCK_MMC_LOG_PATH = std::make_pair("ock.mmc.log_path", ".");
constexpr auto OCK_MMC_LOG_ROTATION_FILE_SIZE = std::make_pair("ock.mmc.log_rotation_file_size", 20);
constexpr auto OCK_MMC_LOG_ROTATION_FILE_COUNT = std::make_pair("ock.mmc.log_rotation_file_count", 50);

constexpr auto OCK_MMC_TLS_ENABLE = std::make_pair("ock.mmc.tls.enable", true);
constexpr auto OCK_MMC_TLS_CA_PATH = std::make_pair("ock.mmc.tls.ca.path", "");
constexpr auto OCK_MMC_TLS_CRL_PATH = std::make_pair("ock.mmc.tls.ca.crl.path", "");
constexpr auto OCK_MMC_TLS_CERT_PATH = std::make_pair("ock.mmc.tls.cert.path", "");
constexpr auto OCK_MMC_TLS_KEY_PATH = std::make_pair("ock.mmc.tls.key.path", "");
constexpr auto OCK_MMC_TLS_KEY_PASS_PATH = std::make_pair("ock.mmc.tls.key.pass.path", "");
constexpr auto OCK_MMC_TLS_PACKAGE_PATH = std::make_pair("ock.mmc.tls.package.path", "");
constexpr auto OCK_MMC_TLS_DECRYPTER_PATH = std::make_pair("ock.mmc.tls.decrypter.path", "");
constexpr auto OCK_MMC_CS_TLS_ENABLE = std::make_pair("ock.mmc.config_store.tls.enable", true);
constexpr auto OCK_MMC_CS_TLS_CA_PATH = std::make_pair("ock.mmc.config_store.tls.ca.path", "");
constexpr auto OCK_MMC_CS_TLS_CRL_PATH = std::make_pair("ock.mmc.config_store.tls.ca.crl.path", "");
constexpr auto OCK_MMC_CS_TLS_CERT_PATH = std::make_pair("ock.mmc.config_store.tls.cert.path", "");
constexpr auto OCK_MMC_CS_TLS_KEY_PATH = std::make_pair("ock.mmc.config_store.tls.key.path", "");
constexpr auto OCK_MMC_CS_TLS_KEY_PASS_PATH = std::make_pair("ock.mmc.config_store.tls.key.pass.path", "");
constexpr auto OCK_MMC_CS_TLS_PACKAGE_PATH = std::make_pair("ock.mmc.config_store.tls.package.path", "");
constexpr auto OCK_MMC_CS_TLS_DECRYPTER_PATH = std::make_pair("ock.mmc.config_store.tls.decrypter.path", "");

constexpr auto OKC_MMC_LOCAL_SERVICE_WORLD_SIZE = std::make_pair("ock.mmc.local_service.world_size", 1);
constexpr auto OKC_MMC_LOCAL_SERVICE_BM_IP_PORT = std::make_pair("ock.mmc.local_service.config_store_url",
                                                                 "tcp://127.0.0.1:6000");
constexpr auto OKC_MMC_LOCAL_SERVICE_PROTOCOL = std::make_pair("ock.mmc.local_service.protocol", "device_sdma");
constexpr auto OKC_MMC_LOCAL_SERVICE_DRAM_SIZE = std::make_pair("ock.mmc.local_service.dram.size", "128MB");
constexpr auto OKC_MMC_LOCAL_SERVICE_HBM_SIZE = std::make_pair("ock.mmc.local_service.hbm.size", "2097152");
constexpr auto OKC_MMC_LOCAL_SERVICE_BM_HCOM_URL = std::make_pair("ock.mmc.local_service.hcom_url",
                                                                  "tcp://127.0.0.1:7000");
constexpr auto OCK_MMC_HCOM_TLS_ENABLE = std::make_pair("ock.mmc.local_service.hcom.tls.enable", true);
constexpr auto OCK_MMC_HCOM_TLS_CA_PATH = std::make_pair("ock.mmc.local_service.hcom.tls.ca.path", "");
constexpr auto OCK_MMC_HCOM_TLS_CRL_PATH = std::make_pair("ock.mmc.local_service.hcom.tls.ca.crl.path", "");
constexpr auto OCK_MMC_HCOM_TLS_CERT_PATH = std::make_pair("ock.mmc.local_service.hcom.tls.cert.path", "");
constexpr auto OCK_MMC_HCOM_TLS_KEY_PATH = std::make_pair("ock.mmc.local_service.hcom.tls.key.path", "");
constexpr auto OCK_MMC_HCOM_TLS_KEY_PASS_PATH = std::make_pair("ock.mmc.local_service.hcom.tls.key.pass.path", "");
constexpr auto OCK_MMC_HCOM_TLS_DECRYPTER_PATH = std::make_pair("ock.mmc.local_service.hcom.tls.decrypter.path", "");

constexpr auto OKC_MMC_CLIENT_RETRY_MILLISECONDS = std::make_pair("ock.mmc.client.retry_milliseconds", 0);
constexpr auto OCK_MMC_CLIENT_TIMEOUT_SECONDS = std::make_pair("ock.mmc.client.timeout.seconds", 60);
constexpr auto OCK_MMC_CLIENT_READ_THREAD_POOL_SIZE = std::make_pair("ock.mmc.client.read_thread_pool.size", 32);
constexpr auto OCK_MMC_CLIENT_WRITE_THREAD_POOL_SIZE = std::make_pair("ock.mmc.client.write_thread_pool.size", 4);
}

constexpr int MIN_LOG_ROTATION_FILE_SIZE = 1;
constexpr int MAX_LOG_ROTATION_FILE_SIZE = 500;

constexpr int MIN_LOG_ROTATION_FILE_COUNT = 1;
constexpr int MAX_LOG_ROTATION_FILE_COUNT = 50;

constexpr int MIN_DEVICE_ID = 0;
constexpr int MAX_DEVICE_ID = 383;

constexpr int MIN_WORLD_SIZE = 1;
constexpr int MAX_WORLD_SIZE = 1024;

constexpr int MIN_EVICT_THRESHOLD = 1;
constexpr int MAX_EVICT_THRESHOLD = 100;

constexpr int MIN_RETRY_MS = 0;
constexpr int MAX_RETRY_MS = 600000;

constexpr int MIN_TIMEOUT_SEC = 1;
constexpr int MAX_TIMEOUT_SEC = 600;

constexpr int MIN_THREAD_POOL_SIZE = 1;
constexpr int MAX_THREAD_POOL_SIZE = 64;

constexpr uint64_t MAX_DRAM_SIZE = 1024ULL * 1024ULL * 1024ULL * 1024ULL;  // 1TB
constexpr uint64_t MAX_HBM_SIZE = 1024ULL * 1024ULL * 1024ULL * 1024ULL;  // 1TB

constexpr uint64_t KB_MEM_BYTES = 1024ULL;
constexpr uint64_t MB_MEM_BYTES = 1024ULL * 1024ULL;
constexpr uint64_t GB_MEM_BYTES = 1024ULL * 1024ULL * 1024ULL;
constexpr uint64_t TB_MEM_BYTES = 1024ULL * 1024ULL * 1024ULL * 1024ULL;

constexpr int MB_NUM = 1024 * 1024;
constexpr uint64_t MEM_2MB_BYTES = 2ULL * 1024ULL * 1024ULL;
constexpr uint64_t MEM_128MB_BYTES = 128ULL * 1024ULL * 1024ULL;

constexpr unsigned long PATH_MAX_LEN = 1023;

}  // namespace mmc
}  // namespace ock

#endif