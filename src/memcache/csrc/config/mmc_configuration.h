/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2026. All rights reserved.
 */

#pragma once

#include <map>
#include <utility>
#include <vector>

#include "mmc_lock.h"
#include "mmc_ref.h"
#include "mmc_config_validator.h"
#include "mmc_config_convertor.h"
#include "mmc_config_const.h"

namespace ock {
namespace mmc {
constexpr uint32_t CONF_MUST = 1;

enum class ConfValueType {
    VINT = 0,
    VFLOAT = 1,
    VSTRING = 2,
    VBOOL = 3,
    VLONG = 4,
};

class Configuration;
using ConfigurationPtr = MmcRef<Configuration>;

class Configuration : public MmcReferable {
public:
    Configuration();
    ~Configuration() override;

    // forbid copy operation
    Configuration(const Configuration &) = delete;
    Configuration &operator = (const Configuration &) = delete;

    // forbid move operation
    Configuration(const Configuration &&) = delete;
    Configuration &operator = (const Configuration &&) = delete;

    static ConfigurationPtr FromFile(const std::string &filePath);
    static ConfigurationPtr GetInstance(const std::string &filePath);
    static ConfigurationPtr GetInstance();
    static void DestroyInstance();

    int32_t GetInt(const std::string &key, int32_t defaultValue = 0);
    float GetFloat(const std::string &key, float defaultValue = 0.0);
    std::string GetString(const std::string &key, const std::string &defaultValue = "");
    bool GetBool(const std::string &key, bool defaultValue = false);
    long GetLong(const std::string &key, long defaultValue = 0);
    std::string GetConvertedValue(const std::string &key);

    void Set(const std::string &key, int32_t value);
    void Set(const std::string &key, float value);
    void Set(const std::string &key, const std::string &value);
    void Set(const std::string &key, bool value);
    void Set(const std::string &key, long value);

    bool SetWithTypeAutoConvert(const std::string &key, const std::string &value);

    void AddIntConf(const std::pair<std::string, int> &pair, const ValidatorPtr &validator = nullptr,
        uint32_t flag = CONF_MUST);
    void AddStrConf(const std::pair<std::string, std::string> &pair, const ValidatorPtr &validator = nullptr,
        uint32_t flag = CONF_MUST);
    void AddConverter(const std::string &key, const ConverterPtr &converter);
    void AddPathConf(const std::pair<std::string, std::string> &pair, const ValidatorPtr &validator = nullptr,
        uint32_t flag = CONF_MUST);
    std::vector<std::string> Validate(bool isAuth = false, bool isTLS = false, bool isAuthor = false,
        bool isZKSecure = false);
    std::vector<std::string> ValidateDaemonConf();
    inline std::string GetConfigPath()
    {
        return mConfigPath;
    }

    inline void SetConfigPath(std::string filePath)
    {
        mConfigPath = std::move(filePath);
    }

    bool Initialized() const
    {
        return mInitialized;
    }

private:
    void SetValidator(const std::string &key, const ValidatorPtr &validator, uint32_t flag);

    void ValidateOneValueMap(std::vector<std::string> &errors,
        const std::map<std::string, ValidatorPtr> &valueValidator);

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

    void LoadDefault()
    {
        using namespace ConfConstant;
        AddStrConf(OCK_MMC_META_SERVICE_DISCOVERY_URL, VNoCheck::Create(), 0);
        AddIntConf(OCK_MMC_META_SERVICE_WORLD_SIZE,
            VIntRange::Create(OCK_MMC_META_SERVICE_WORLD_SIZE.first,
                MIN_META_SERVICE_WORLD_SIZE,MAX_META_SERVICE_WORLD_SIZE));
    }

private:
    static ConfigurationPtr gInstance;
    std::string mConfigPath;

    std::map<std::string, int32_t> mIntItems;
    std::map<std::string, float> mFloatItems;
    std::map<std::string, std::string> mStrItems;
    std::map<std::string, bool> mBoolItems;
    std::map<std::string, long> mLongItems;
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

} // namespace mmc
} // namespace ock