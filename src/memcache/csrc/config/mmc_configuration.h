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

#pragma once

#include <map>
#include <utility>
#include <vector>
#include <cctype>

#include "mmc_lock.h"
#include "mmc_ref.h"
#include "mmc_config_validator.h"
#include "mmc_config_convertor.h"
#include "mmc_config_const.h"
#include "mmc_def.h"
#include "mmc_logger.h"
#include "mmc_types.h"
#include "mmc_last_error.h"
#include "smem_bm_def.h"
#include "common/mmc_functions.h"

namespace ock {
namespace mmc {
constexpr uint32_t CONF_MUST = 1;
constexpr uint64_t DRAM_SIZE_ALIGNMENT = 2097152; // 2MB
constexpr uint64_t HBM_SIZE_ALIGNMENT = 2097152; // 2MB

const std::string BOOL_ENUM_STR = "false||true";
const std::string LOG_LEVEL_ENUM_STR = "debug||info||warn||error";
const std::string LOCAL_SERVER_PROTOCAL_ENUM_STR = "host_rdma||host_urma||host_tcp||device_rdma||device_sdma";

// 定义单位与字节的转换关系
enum class MemUnit {
    B,
    KB,
    MB,
    GB,
    TB,
    UNKNOWN
};

enum class ConfValueType {
    VINT = 0,
    VFLOAT = 1,
    VSTRING = 2,
    VBOOL = 3,
    VUINT64 = 4,
};

void StringToUpper(std::string &str);

class Configuration;
using ConfigurationPtr = MmcRef<Configuration>;

class Configuration : public MmcReferable {
public:
    Configuration() = default;
    ~Configuration() override;

    // forbid copy operation
    Configuration(const Configuration &) = delete;
    Configuration &operator = (const Configuration &) = delete;

    // forbid move operation
    Configuration(const Configuration &&) = delete;
    Configuration &operator = (const Configuration &&) = delete;

    bool LoadFromFile(const std::string &filePath);

    int32_t GetInt(const std::pair<const char *, int32_t> &item);
    float GetFloat(const std::pair<const char *, float> &item);
    std::string GetString(const std::pair<const char *, const char *> &item);
    bool GetBool(const std::pair<const char *, bool> &item);
    uint64_t GetUInt64(const std::pair<const char *, uint64_t> &item);
    uint64_t GetUInt64(const char *key, uint64_t defaultValue);

    void Set(const std::string &key, int32_t value);
    void Set(const std::string &key, float value);
    void Set(const std::string &key, const std::string &value);
    void Set(const std::string &key, bool value);
    void Set(const std::string &key, uint64_t value);

    bool SetWithTypeAutoConvert(const std::string &key, const std::string &value);

    void AddIntConf(const std::pair<std::string, int> &pair, const ValidatorPtr &validator = nullptr,
        uint32_t flag = CONF_MUST);
    void AddStrConf(const std::pair<std::string, std::string> &pair, const ValidatorPtr &validator = nullptr,
        uint32_t flag = CONF_MUST);
    void AddBoolConf(const std::pair<std::string, bool> &pair, const ValidatorPtr &validator = nullptr,
        uint32_t flag = CONF_MUST);
    void AddUInt64Conf(const std::pair<std::string, uint64_t> &pair, const ValidatorPtr &validator = nullptr,
        uint32_t flag = CONF_MUST);
    void AddConverter(const std::string &key, const ConverterPtr &converter);
    void AddPathConf(const std::pair<std::string, std::string> &pair, const ValidatorPtr &validator = nullptr,
        uint32_t flag = CONF_MUST);
    std::vector<std::string> ValidateConf();
    void GetAccTlsConfig(mmc_tls_config &tlsConfig);
    void GetHcomTlsConfig(mmc_tls_config &tlsConfig);
    void GetConfigStoreTlsConfig(mmc_tls_config &tlsConfig);

    static int ValidateTLSConfig(const mmc_tls_config &tlsConfig);

    const std::string GetBinDir();
    const std::string GetLogPath(const std::string &logPath);
    static int ValidateLogPathConfig(const std::string &logPath);

    bool Initialized() const
    {
        return mInitialized;
    }

private:
    bool SetWithStrAutoConvert(const std::string &key, const std::string &value);
    uint64_t ParseMemSize(const std::string &memStr);
    MemUnit ParseMemUnit(const std::string& unit);

    void SetValidator(const std::string &key, const ValidatorPtr &validator, uint32_t flag);

    template <class T>
    static void AddValidateError(const ValidatorPtr &validator, std::vector<std::string> &errors, const T &iter)
    {
        if (validator == nullptr) {
            errors.push_back("Failed to validate <" + iter->first + ">, validator is NULL");
            return;
        }
        if (!(validator->Validate(iter->second))) {
            errors.push_back(validator->ErrorMessage());
        }
    }
    void ValidateOneType(const std::string &key, const ValidatorPtr &validator,
        std::vector<std::string> &errors, ConfValueType &vType);

    void ValidateItem(const std::string &itemKey, std::vector<std::string> &errors);

    void LoadConfigurations();

    virtual void LoadDefault() {}

    std::string mConfigPath;

    std::map<std::string, int32_t> mIntItems;
    std::map<std::string, float> mFloatItems;
    std::map<std::string, std::string> mStrItems;
    std::map<std::string, bool> mBoolItems;
    std::map<std::string, uint64_t> mUInt64Items;
    std::map<std::string, std::string> mAllItems;

    std::map<std::string, ConfValueType> mValueTypes;
    std::map<std::string, ValidatorPtr> mValueValidator;
    std::map<std::string, ConverterPtr> mValueConverter;

    std::vector<std::pair<std::string, std::string>> mServiceList;
    std::vector<std::string> mMustKeys;
    std::vector<std::string> mLoadDefaultErrors;

    std::vector<std::string> mPathConfs;
    std::vector<std::string> mExceptPrintConfs;
    std::vector<std::string> mInvalidSetConfs;

    bool mInitialized = false;
    Lock mLock;
};

class MetaServiceConfig final : public Configuration {
public:
    void LoadDefault() override
    {
        using namespace ConfConstant;
        AddStrConf(OCK_MMC_META_SERVICE_URL, VNoCheck::Create(), 0);
        AddStrConf(OCK_MMC_META_SERVICE_CONFIG_STORE_URL, VNoCheck::Create(), 0);
        AddStrConf(OCK_MMC_META_SERVICE_HTTP_URL, VNoCheck::Create(), 0);
        AddBoolConf(OCK_MMC_META_HA_ENABLE, VStrEnum::Create(OCK_MMC_META_HA_ENABLE.first, BOOL_ENUM_STR), 0);
        AddStrConf(OCK_MMC_LOG_LEVEL, VStrEnum::Create(OCK_MMC_LOG_LEVEL.first, LOG_LEVEL_ENUM_STR), 0);
        AddStrConf(OCK_MMC_LOG_PATH, VStrLength::Create(OCK_MMC_LOG_PATH.first, PATH_MAX_LEN), 0);
        AddIntConf(OCK_MMC_LOG_ROTATION_FILE_SIZE, VIntRange::Create(OCK_MMC_LOG_ROTATION_FILE_SIZE.first,
            MIN_LOG_ROTATION_FILE_SIZE, MAX_LOG_ROTATION_FILE_SIZE), 0);
        AddIntConf(OCK_MMC_LOG_ROTATION_FILE_COUNT, VIntRange::Create(OCK_MMC_LOG_ROTATION_FILE_COUNT.first,
            MIN_LOG_ROTATION_FILE_COUNT, MAX_LOG_ROTATION_FILE_COUNT), 0);
        AddIntConf(OKC_MMC_EVICT_THRESHOLD_HIGH,
            VIntRange::Create(OKC_MMC_EVICT_THRESHOLD_HIGH.first, MIN_EVICT_THRESHOLD, MAX_EVICT_THRESHOLD), 0);
        AddIntConf(OKC_MMC_EVICT_THRESHOLD_LOW,
            VIntRange::Create(OKC_MMC_EVICT_THRESHOLD_LOW.first, MIN_EVICT_THRESHOLD, MAX_EVICT_THRESHOLD - 1), 0);

        AddBoolConf(OCK_MMC_TLS_ENABLE, VStrEnum::Create(OCK_MMC_TLS_ENABLE.first, BOOL_ENUM_STR), 0);
        AddStrConf(OCK_MMC_TLS_CA_PATH, VStrLength::Create(OCK_MMC_TLS_CA_PATH.first, TLS_PATH_MAX_LEN), 0);
        AddStrConf(OCK_MMC_TLS_CRL_PATH, VStrLength::Create(OCK_MMC_TLS_CRL_PATH.first, TLS_PATH_MAX_LEN), 0);
        AddStrConf(OCK_MMC_TLS_CERT_PATH, VStrLength::Create(OCK_MMC_TLS_CERT_PATH.first, TLS_PATH_MAX_LEN), 0);
        AddStrConf(OCK_MMC_TLS_KEY_PATH, VStrLength::Create(OCK_MMC_TLS_KEY_PATH.first, TLS_PATH_MAX_LEN), 0);
        AddStrConf(OCK_MMC_TLS_KEY_PASS_PATH, VStrLength::Create(OCK_MMC_TLS_KEY_PASS_PATH.first, TLS_PATH_MAX_LEN), 0);
        AddStrConf(OCK_MMC_TLS_PACKAGE_PATH, VStrLength::Create(OCK_MMC_TLS_PACKAGE_PATH.first, TLS_PATH_MAX_LEN), 0);
        AddStrConf(OCK_MMC_TLS_DECRYPTER_PATH,
            VStrLength::Create(OCK_MMC_TLS_DECRYPTER_PATH.first, TLS_PATH_MAX_LEN), 0);

        AddBoolConf(OCK_MMC_CS_TLS_ENABLE, VStrEnum::Create(OCK_MMC_CS_TLS_ENABLE.first, BOOL_ENUM_STR), 0);
        AddStrConf(OCK_MMC_CS_TLS_CA_PATH, VStrLength::Create(OCK_MMC_CS_TLS_CA_PATH.first, TLS_PATH_MAX_LEN), 0);
        AddStrConf(OCK_MMC_CS_TLS_CRL_PATH, VStrLength::Create(OCK_MMC_CS_TLS_CRL_PATH.first, TLS_PATH_MAX_LEN), 0);
        AddStrConf(OCK_MMC_CS_TLS_CERT_PATH, VStrLength::Create(OCK_MMC_CS_TLS_CERT_PATH.first, TLS_PATH_MAX_LEN), 0);
        AddStrConf(OCK_MMC_CS_TLS_KEY_PATH, VStrLength::Create(OCK_MMC_CS_TLS_KEY_PATH.first, TLS_PATH_MAX_LEN), 0);
        AddStrConf(OCK_MMC_CS_TLS_KEY_PASS_PATH,
            VStrLength::Create(OCK_MMC_CS_TLS_KEY_PASS_PATH.first, TLS_PATH_MAX_LEN), 0);
        AddStrConf(OCK_MMC_CS_TLS_PACKAGE_PATH,
            VStrLength::Create(OCK_MMC_CS_TLS_PACKAGE_PATH.first, TLS_PATH_MAX_LEN), 0);
        AddStrConf(OCK_MMC_CS_TLS_DECRYPTER_PATH,
            VStrLength::Create(OCK_MMC_CS_TLS_DECRYPTER_PATH.first, TLS_PATH_MAX_LEN), 0);
    }

    void GetMetaServiceConfig(mmc_meta_service_config_t &config)
    {
        SafeCopy(GetString(ConfConstant::OCK_MMC_META_SERVICE_URL),
            config.discoveryURL, DISCOVERY_URL_SIZE);
        SafeCopy(GetString(ConfConstant::OCK_MMC_META_SERVICE_CONFIG_STORE_URL),
            config.configStoreURL, DISCOVERY_URL_SIZE);
        SafeCopy(GetString(ConfConstant::OCK_MMC_META_SERVICE_HTTP_URL),
            config.httpURL, DISCOVERY_URL_SIZE);

        config.haEnable = GetBool(ConfConstant::OCK_MMC_META_HA_ENABLE);
        std::string logLevelStr = GetString(ConfConstant::OCK_MMC_LOG_LEVEL);
        StringToUpper(logLevelStr);
        config.logLevel = MmcOutLogger::Instance().GetLogLevel(logLevelStr);

        SafeCopy(GetLogPath(GetString(ConfConstant::OCK_MMC_LOG_PATH)), config.logPath, PATH_MAX_SIZE);

        config.evictThresholdHigh = GetInt(ConfConstant::OKC_MMC_EVICT_THRESHOLD_HIGH);
        config.evictThresholdLow = GetInt(ConfConstant::OKC_MMC_EVICT_THRESHOLD_LOW);
        config.logRotationFileSize = GetInt(ConfConstant::OCK_MMC_LOG_ROTATION_FILE_SIZE) * MB_NUM;
        config.logRotationFileCount = GetInt(ConfConstant::OCK_MMC_LOG_ROTATION_FILE_COUNT);
        GetAccTlsConfig(config.accTlsConfig);
        GetConfigStoreTlsConfig(config.configStoreTlsConfig);
    }
};

class ClientConfig final: public Configuration {
public:
    void LoadDefault() override
    {
        using namespace ConfConstant;
        AddStrConf(OCK_MMC_META_SERVICE_URL, VNoCheck::Create(), 0);
        AddStrConf(OCK_MMC_LOG_LEVEL, VStrEnum::Create(OCK_MMC_LOG_LEVEL.first, LOG_LEVEL_ENUM_STR), 0);

        AddBoolConf(OCK_MMC_TLS_ENABLE, VStrEnum::Create(OCK_MMC_TLS_ENABLE.first, BOOL_ENUM_STR), 0);
        AddStrConf(OCK_MMC_TLS_CA_PATH, VStrLength::Create(OCK_MMC_TLS_CA_PATH.first, TLS_PATH_MAX_LEN), 0);
        AddStrConf(OCK_MMC_TLS_CRL_PATH, VStrLength::Create(OCK_MMC_TLS_CRL_PATH.first, TLS_PATH_MAX_LEN), 0);
        AddStrConf(OCK_MMC_TLS_CERT_PATH, VStrLength::Create(OCK_MMC_TLS_CERT_PATH.first, TLS_PATH_MAX_LEN), 0);
        AddStrConf(OCK_MMC_TLS_KEY_PATH, VStrLength::Create(OCK_MMC_TLS_KEY_PATH.first, TLS_PATH_MAX_LEN), 0);
        AddStrConf(OCK_MMC_TLS_KEY_PASS_PATH, VStrLength::Create(OCK_MMC_TLS_KEY_PASS_PATH.first, TLS_PATH_MAX_LEN), 0);
        AddStrConf(OCK_MMC_TLS_PACKAGE_PATH, VStrLength::Create(OCK_MMC_TLS_PACKAGE_PATH.first, TLS_PATH_MAX_LEN), 0);
        AddStrConf(OCK_MMC_TLS_DECRYPTER_PATH,
            VStrLength::Create(OCK_MMC_TLS_DECRYPTER_PATH.first, TLS_PATH_MAX_LEN), 0);

        AddBoolConf(OCK_MMC_CS_TLS_ENABLE, VStrEnum::Create(OCK_MMC_CS_TLS_ENABLE.first, BOOL_ENUM_STR), 0);
        AddStrConf(OCK_MMC_CS_TLS_CA_PATH, VStrLength::Create(OCK_MMC_CS_TLS_CA_PATH.first, TLS_PATH_MAX_LEN), 0);
        AddStrConf(OCK_MMC_CS_TLS_CRL_PATH, VStrLength::Create(OCK_MMC_CS_TLS_CRL_PATH.first, TLS_PATH_MAX_LEN), 0);
        AddStrConf(OCK_MMC_CS_TLS_CERT_PATH, VStrLength::Create(OCK_MMC_CS_TLS_CERT_PATH.first, TLS_PATH_MAX_LEN), 0);
        AddStrConf(OCK_MMC_CS_TLS_KEY_PATH, VStrLength::Create(OCK_MMC_CS_TLS_KEY_PATH.first, TLS_PATH_MAX_LEN), 0);
        AddStrConf(OCK_MMC_CS_TLS_KEY_PASS_PATH,
            VStrLength::Create(OCK_MMC_CS_TLS_KEY_PASS_PATH.first, TLS_PATH_MAX_LEN), 0);
        AddStrConf(OCK_MMC_CS_TLS_PACKAGE_PATH,
            VStrLength::Create(OCK_MMC_CS_TLS_PACKAGE_PATH.first, TLS_PATH_MAX_LEN), 0);
        AddStrConf(OCK_MMC_CS_TLS_DECRYPTER_PATH,
            VStrLength::Create(OCK_MMC_CS_TLS_DECRYPTER_PATH.first, TLS_PATH_MAX_LEN), 0);

        AddIntConf(OKC_MMC_LOCAL_SERVICE_WORLD_SIZE,
            VIntRange::Create(OKC_MMC_LOCAL_SERVICE_WORLD_SIZE.first, MIN_WORLD_SIZE, MAX_WORLD_SIZE), 0);
        AddStrConf(OKC_MMC_LOCAL_SERVICE_BM_IP_PORT, VNoCheck::Create(), 0);
        AddStrConf(OKC_MMC_LOCAL_SERVICE_PROTOCOL,
            VStrEnum::Create(OKC_MMC_LOCAL_SERVICE_PROTOCOL.first, LOCAL_SERVER_PROTOCAL_ENUM_STR), 1); // REQUIED
        AddStrConf(OKC_MMC_LOCAL_SERVICE_DRAM_SIZE, VNoCheck::Create(), 1); // REQUIED
        AddStrConf(OKC_MMC_LOCAL_SERVICE_HBM_SIZE, VNoCheck::Create(), 1); // REQUIED
        
        // HCOM TLS config
        AddStrConf(OKC_MMC_LOCAL_SERVICE_BM_HCOM_URL, VNoCheck::Create(), 0);
        AddBoolConf(OCK_MMC_HCOM_TLS_ENABLE, VStrEnum::Create(OCK_MMC_HCOM_TLS_ENABLE.first, BOOL_ENUM_STR), 0);
        AddStrConf(OCK_MMC_HCOM_TLS_CA_PATH,
            VStrLength::Create(OCK_MMC_HCOM_TLS_CA_PATH.first, TLS_PATH_MAX_LEN), 0);
        AddStrConf(OCK_MMC_HCOM_TLS_CRL_PATH,
            VStrLength::Create(OCK_MMC_HCOM_TLS_CRL_PATH.first, TLS_PATH_MAX_LEN), 0);
        AddStrConf(OCK_MMC_HCOM_TLS_CERT_PATH,
            VStrLength::Create(OCK_MMC_HCOM_TLS_CERT_PATH.first, TLS_PATH_MAX_LEN), 0);
        AddStrConf(OCK_MMC_HCOM_TLS_KEY_PATH,
            VStrLength::Create(OCK_MMC_HCOM_TLS_KEY_PATH.first, TLS_PATH_MAX_LEN), 0);
        AddStrConf(OCK_MMC_HCOM_TLS_KEY_PASS_PATH,
            VStrLength::Create(OCK_MMC_HCOM_TLS_KEY_PASS_PATH.first, TLS_PATH_MAX_LEN), 0);
        AddStrConf(OCK_MMC_HCOM_TLS_DECRYPTER_PATH,
            VStrLength::Create(OCK_MMC_HCOM_TLS_DECRYPTER_PATH.first, TLS_PATH_MAX_LEN), 0);
        
        AddIntConf(OKC_MMC_CLIENT_RETRY_MILLISECONDS,
                   VIntRange::Create(OKC_MMC_CLIENT_RETRY_MILLISECONDS.first, MIN_RETRY_MS, MAX_RETRY_MS), 0);
        AddIntConf(OCK_MMC_CLIENT_TIMEOUT_SECONDS,
            VIntRange::Create(OCK_MMC_CLIENT_TIMEOUT_SECONDS.first, MIN_TIMEOUT_SEC, MAX_TIMEOUT_SEC), 0);
        AddIntConf(OCK_MMC_CLIENT_READ_THREAD_POOL_SIZE,
            VIntRange::Create(OCK_MMC_CLIENT_READ_THREAD_POOL_SIZE.first,
            MIN_THREAD_POOL_SIZE, MAX_THREAD_POOL_SIZE), 0);
        AddBoolConf(OCK_MMC_CLIENT_AGGREGATE_IO, VStrEnum::Create(OCK_MMC_CLIENT_AGGREGATE_IO.first, BOOL_ENUM_STR), 0);
        AddIntConf(OCK_MMC_CLIENT_WRITE_THREAD_POOL_SIZE,
            VIntRange::Create(OCK_MMC_CLIENT_WRITE_THREAD_POOL_SIZE.first,
            MIN_THREAD_POOL_SIZE, MAX_THREAD_POOL_SIZE), 0);
    }

    void GetLocalServiceConfig(mmc_local_service_config_t &config)
    {
        SafeCopy(GetString(ConfConstant::OCK_MMC_META_SERVICE_URL), config.discoveryURL, DISCOVERY_URL_SIZE);

        config.worldSize = static_cast<uint32_t>(GetInt(ConfConstant::OKC_MMC_LOCAL_SERVICE_WORLD_SIZE));
        SafeCopy(GetString(ConfConstant::OKC_MMC_LOCAL_SERVICE_BM_IP_PORT), config.bmIpPort, DISCOVERY_URL_SIZE);
        SafeCopy(GetString(ConfConstant::OKC_MMC_LOCAL_SERVICE_BM_HCOM_URL), config.bmHcomUrl, DISCOVERY_URL_SIZE);
        config.createId = 0;
        SafeCopy(GetString(ConfConstant::OKC_MMC_LOCAL_SERVICE_PROTOCOL), config.dataOpType, PROTOCOL_SIZE);
        config.localDRAMSize = GetUInt64(ConfConstant::OKC_MMC_LOCAL_SERVICE_DRAM_SIZE.first, MEM_128MB_BYTES);
        config.localHBMSize = GetUInt64(ConfConstant::OKC_MMC_LOCAL_SERVICE_HBM_SIZE.first, MEM_2MB_BYTES);
        auto protocol = std::string(config.dataOpType);
        std::string logLevelStr = GetString(ConfConstant::OCK_MMC_LOG_LEVEL);
        StringToUpper(logLevelStr);
        config.logLevel = MmcOutLogger::Instance().GetLogLevel(logLevelStr);
        GetAccTlsConfig(config.accTlsConfig);
        GetHcomTlsConfig(config.hcomTlsConfig);
        GetConfigStoreTlsConfig(config.configStoreTlsConfig);
    }

    void GetClientConfig(mmc_client_config_t &config)
    {
        SafeCopy(GetString(ConfConstant::OCK_MMC_META_SERVICE_URL), config.discoveryURL, DISCOVERY_URL_SIZE);

        config.rpcRetryTimeOut = static_cast<uint32_t>(GetInt(ConfConstant::OKC_MMC_CLIENT_RETRY_MILLISECONDS));
        config.timeOut = static_cast<uint32_t>(GetInt(ConfConstant::OCK_MMC_CLIENT_TIMEOUT_SECONDS));
        config.readThreadPoolNum = static_cast<uint32_t>(GetInt(ConfConstant::OCK_MMC_CLIENT_READ_THREAD_POOL_SIZE));
        config.aggregateIO = GetBool(ConfConstant::OCK_MMC_CLIENT_AGGREGATE_IO);
        config.writeThreadPoolNum = static_cast<uint32_t>(GetInt(ConfConstant::OCK_MMC_CLIENT_WRITE_THREAD_POOL_SIZE));
        std::string logLevelStr = GetString(ConfConstant::OCK_MMC_LOG_LEVEL);
        StringToUpper(logLevelStr);
        config.logLevel = MmcOutLogger::Instance().GetLogLevel(logLevelStr);
        GetAccTlsConfig(config.tlsConfig);
    }

    static Result ValidateLocalServiceConfig(mmc_local_service_config_t &config)
    {
        if (config.localDRAMSize > MAX_DRAM_SIZE) {
            MMC_LOG_ERROR("After alignment 2MB, DRAM size (" << config.localDRAMSize << ") exceeds 1TB");
            return MMC_INVALID_PARAM;
        }

        if (config.localHBMSize > MAX_HBM_SIZE) {
            MMC_LOG_ERROR("After alignment 2MB, HBM size (" << config.localHBMSize << ") exceeds 1TB");
            return MMC_INVALID_PARAM;
        }

        config.localDRAMSize = (config.localDRAMSize / DRAM_SIZE_ALIGNMENT) * DRAM_SIZE_ALIGNMENT;
        config.localHBMSize = (config.localHBMSize / DRAM_SIZE_ALIGNMENT) * DRAM_SIZE_ALIGNMENT;

        MMC_LOG_INFO("After alignment 2MB, DRAM size is " << config.localDRAMSize);
        MMC_LOG_INFO("After alignment 2MB, HBM size is " << config.localHBMSize);

        if (config.localDRAMSize == 0 && config.localHBMSize == 0) {
            MMC_LOG_ERROR("After alignment 2MB, DRAM size and HBM size cannot be 0 at the same time");
            return MMC_INVALID_PARAM;
        }

        MMC_VALIDATE_RETURN(ValidateTLSConfig(config.accTlsConfig) == MMC_OK,
            "Invalid acc_link TLS config", MMC_INVALID_PARAM);
        MMC_VALIDATE_RETURN(ValidateTLSConfig(config.hcomTlsConfig) == MMC_OK,
            "Invalid hcom TLS config", MMC_INVALID_PARAM);
        MMC_VALIDATE_RETURN(ValidateTLSConfig(config.configStoreTlsConfig) == MMC_OK,
            "Invalid config store TLS config", MMC_INVALID_PARAM);

        return MMC_OK;
    }
};

} // namespace mmc
} // namespace ock