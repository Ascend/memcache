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
#ifndef __MEMFABRIC_MMC_H__
#define __MEMFABRIC_MMC_H__

#include "stdint.h"
#ifdef __cplusplus
#include <string>
#endif

#include "mmc_def.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint32_t deviceId;
    bool initBm;
} mmc_init_config;

typedef struct {
    // Configuration file path
    // If set, the system will load configuration from this file
    char config_path[PATH_MAX_SIZE];

    // Meta service start-up url
    // K8s meta service cluster master-standby high availability scenario: ClusterIP address
    // Non-HA scenario: keep consistent with the same name configuration in mmc-meta.conf
    char meta_service_url[DISCOVERY_URL_SIZE];

    // Config store url, it will automatically modified to PodIP at Pod startup in HA scenario
    // Keep consistent with the same name configuration in mmc-meta.conf
    char config_store_url[DISCOVERY_URL_SIZE];

    // Log level: debug, info, warn, error
    char log_level[10];

    // The maximum supported rank count; once ranks are connected,
    // no further modifications are allowed, and a meta restart is required
    uint32_t world_size;

    // Data transfer protocol:
    // 'host_rdma': rdma over host; 'host_urma': rdma over host_ub;
    // 'host_tcp': tcp over host; 'device_rdma': rdma over device;
    // 'device_sdma': sdma over device
    // host_rdma, host_urma and host_tcp need hcom
    char protocol[PROTOCOL_SIZE];

    // If the protocol is host_rdma, the ip needs to be set as RDMA network card ip. Use 'show_gids' command to query it
    char hcom_url[DISCOVERY_URL_SIZE];

    // HBM/DRAM space usage, configuration type supports 134217728,
    // 2048KB/2048K, 200MB/200mb/200m, 2.5GB or 1TB, case-insensitive;
    // the maximum value is 1TB
    // The system automatically calculates and aligns upwards to 2MB
    // (host_rdma or host_tcp) or 1GB (device_sdma or device_rdma)
    // In A3 environment all protocol should be aligned to 1GB, otherwise mmc init will fail
    // After alignment, the HBM size and DRAM size cannot both be 0 at the same time
    char dram_size[64];
    char hbm_size[64];

    // The MAX size of `ock.mmc.local_service.dram.size` in all local processes
    char max_dram_size[64];

    // The MAX size of `ock.mmc.local_service.hbm.size` in all local processes
    char max_hbm_size[64];

    // The total retry duration (retry interval is 200ms) when client
    // requests meta service and the connection does not exist
    // Default value is 0, means no-retry and return immediately, value range [0, 600000]
    uint32_t client_retry_milliseconds;

    // Client request timeout in seconds, value range [1, 600]
    uint32_t client_timeout_seconds;

    // Read/write thread pool size, value range [1, 64]
    uint32_t read_thread_pool_size;
    uint32_t write_thread_pool_size;

    // Read/write aggregate, num range [1, 131072]
    bool aggregate_io;
    uint32_t aggregate_num;

    // UBS_IO enable
    bool ubs_io_enable;

    // The memory pool mode, optional: standard/expanded.
    // - standard: the capacity is limited 32TB.
    // - expanded: the capacity is limited 128TB.
    char memory_pool_mode[MEM_POOL_MODE_SIZE];

    // TLS configurations for metaservice
    bool tls_enable;
    char tls_ca_path[TLS_PATH_SIZE];
    char tls_ca_crl_path[TLS_PATH_SIZE];
    char tls_cert_path[TLS_PATH_SIZE];
    char tls_key_path[TLS_PATH_SIZE];
    char tls_key_pass_path[TLS_PATH_SIZE];
    char tls_package_path[TLS_PATH_SIZE];
    char tls_decrypter_path[TLS_PATH_SIZE];

    // TLS configurations for config_store
    bool config_store_tls_enable;
    char config_store_tls_ca_path[TLS_PATH_SIZE];
    char config_store_tls_ca_crl_path[TLS_PATH_SIZE];
    char config_store_tls_cert_path[TLS_PATH_SIZE];
    char config_store_tls_key_path[TLS_PATH_SIZE];
    char config_store_tls_key_pass_path[TLS_PATH_SIZE];
    char config_store_tls_package_path[TLS_PATH_SIZE];
    char config_store_tls_decrypter_path[TLS_PATH_SIZE];

    // HCOM TLS config
    bool hcom_tls_enable;
    char hcom_tls_ca_path[TLS_PATH_SIZE];
    char hcom_tls_ca_crl_path[TLS_PATH_SIZE];
    char hcom_tls_cert_path[TLS_PATH_SIZE];
    char hcom_tls_key_path[TLS_PATH_SIZE];
    char hcom_tls_key_pass_path[TLS_PATH_SIZE];
    char hcom_tls_decrypter_path[TLS_PATH_SIZE];
} local_config;

/**
 * @brief Setup local configuration before initialization
 * @param config              [in] local configuration @local_config_t
 * @return 0 if successful, non-zero otherwise
 */
int32_t mmc_setup(const local_config *config);

/**
 * @brief Initialize the memcache client and local service
 * @param config              [in] init confid @mmc_init_config
 * @return 0 if successful,
 */
int32_t mmc_init(const mmc_init_config *config);

/**
 * @brief Set external log function, user can set customized logger function,
 * in the customized logger function, user can use unified logger utility,
 * then the log message can be written into the same log file as caller's,
 * if it is not set, acc_links log message will be printed to stdout.
 *
 * level description:
 * 0 DEBUG,
 * 1 INFO,
 * 2 WARN,
 * 3 ERROR
 *
 * @param func             [in] external logger function
 * @return 0 if successful
 */
int32_t mmc_set_extern_logger(void (*func)(int level, const char *msg));

/**
 * @brief set log print level
 *
 * @param level            [in] log level, 0:debug 1:info 2:warn 3:error
 * @return 0 if successful
 */
int32_t mmc_set_log_level(int level);

/**
 * @brief Un-Initialize the smem running environment
 */
void mmc_uninit(void);

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus

/**
 * @brief Create a local_config object with built-in default values.
 * @return local_config    default-initialized local configuration
 */
local_config create_default_local_config();

/**
 * @brief Convert local configuration to a readable string.
 * @param config           [in] local configuration object
 * @return std::string     formatted configuration string
 */
std::string local_config_to_string(const local_config &config);

#endif

#endif //__MEMFABRIC_MMC_H__
