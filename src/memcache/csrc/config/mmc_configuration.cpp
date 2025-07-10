/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2026. All rights reserved.
 */

#include "mmc_configuration.h"

#include <iostream>

#include "mmc_kv_parser.h"

namespace ock {
namespace mmc {
ConfigurationPtr Configuration::gInstance;
Configuration::Configuration()
{
    LoadConfigurations();
}

Configuration::~Configuration()
{
    for (auto validator : mValueValidator) {
        validator.second.Set(nullptr);
    }
    for (auto converter : mValueConverter) {
        converter.second.Set(nullptr);
    }
    mValueValidator.clear();
    mValueConverter.clear();
}

ConfigurationPtr Configuration::FromFile(const std::string &filePath)
{
    auto *conf = new (std::nothrow) Configuration();
    if (conf == nullptr) {
        return nullptr;
    }
    if (!conf->Initialized()) {
        SAFE_DELETE(conf);
        return nullptr;
    }
    auto *kvParser = new (std::nothrow) KVParser();
    if (kvParser == nullptr) {
        SAFE_DELETE(conf);
        return nullptr;
    }
    if (RESULT_FAIL(kvParser->FromFile(filePath))) {
        SAFE_DELETE(kvParser);
        SAFE_DELETE(conf);
        return nullptr;
    }

    uint32_t size = kvParser->Size();
    static const int MAX_CONF_ITEM_COUNT = 100;
    if (size > MAX_CONF_ITEM_COUNT) {
        SAFE_DELETE(kvParser);
        SAFE_DELETE(conf);
        return nullptr;
    }
    for (uint32_t i = 0; i < size; i++) {
        std::string key;
        std::string value;
        kvParser->GetI(i, key, value);
        if (!conf->SetWithTypeAutoConvert(key, value)) {
            SAFE_DELETE(kvParser);
            SAFE_DELETE(conf);
            return nullptr;
        }
    }

    conf->SetConfigPath(filePath);
    if (!kvParser->CheckSet(conf->mMustKeys)) {
        SAFE_DELETE(kvParser);
        SAFE_DELETE(conf);
        return nullptr;
    }
    SAFE_DELETE(kvParser);
    return conf;
}

ConfigurationPtr Configuration::GetInstance(const std::string &filePath)
{
    if (gInstance.Get() == nullptr) {
        gInstance = FromFile(filePath);
    }
    return gInstance;
}

ConfigurationPtr Configuration::GetInstance()
{
    return gInstance;
}

void Configuration::DestroyInstance()
{
    gInstance.Set(nullptr);
}


int32_t Configuration::GetInt(const std::string &key, int32_t defaultValue)
{
    GUARD(&mLock, mLock);
    auto iter = mIntItems.find(key);
    if (iter != mIntItems.end()) {
        return iter->second;
    }
    return defaultValue;
}

std::string Configuration::GetString(const std::string &key, const std::string &defaultValue)
{
    GUARD(&mLock, mLock);
    auto iter = mStrItems.find(key);
    if (iter != mStrItems.end()) {
        return iter->second;
    }
    return defaultValue;
}

std::string Configuration::GetConvertedValue(const std::string &key)
{
    GUARD(&mLock, mLock);
    auto iter = mValueTypes.find(key);
    if (iter == mValueTypes.end()) {
        return "";
    }
    ConfValueType valueType = iter->second;
    std::string value = std::string();
    switch (valueType) {
        case ConfValueType::VSTRING: {
            auto iterStr = mStrItems.find(key);
            value = iterStr != mStrItems.end() ? iterStr->second : std::string();
            break;
        }
        case ConfValueType::VBOOL: {
            auto iterBool = mBoolItems.find(key);
            value = iterBool != mBoolItems.end() ? std::to_string(iterBool->second) : std::string();
            break;
        }
        case ConfValueType::VLONG: {
            auto iterLong = mLongItems.find(key);
            value = iterLong != mLongItems.end() ? std::to_string(iterLong->second) : std::string();
            break;
        }
        case ConfValueType::VINT: {
            auto iterInt = mIntItems.find(key);
            value = iterInt != mIntItems.end() ? std::to_string(iterInt->second) : std::string();
            break;
        }
        case ConfValueType::VFLOAT: {
            auto iterFloat = mFloatItems.find(key);
            value = iterFloat != mFloatItems.end() ? std::to_string(iterFloat->second) : std::string();
            break;
        }
        default:;
    }
    auto converterIter = mValueConverter.find(key);
    if (converterIter == mValueConverter.end() || converterIter->second == nullptr) {
        return value;
    }
    return converterIter->second->Convert(value);
}

void Configuration::Set(const std::string &key, int32_t value)
{
    GUARD(&mLock, mLock);
    if (mIntItems.count(key) > 0) {
        mIntItems.at(key) = value;
    }
}

void Configuration::Set(const std::string &key, float value)
{
    GUARD(&mLock, mLock);
    if (mFloatItems.count(key) > 0) {
        mFloatItems.at(key) = value;
    }
}

void Configuration::Set(const std::string &key, const std::string &value)
{
    GUARD(&mLock, mLock);
    if (mStrItems.count(key) > 0) {
        mStrItems.at(key) = value;
    }
}

void Configuration::Set(const std::string &key, bool value)
{
    GUARD(&mLock, mLock);
    if (mBoolItems.count(key) > 0) {
        mBoolItems.at(key) = value;
    }
}

void Configuration::Set(const std::string &key, long value)
{
    GUARD(&mLock, mLock);
    if (mLongItems.count(key) > 0) {
        mLongItems.at(key) = value;
    }
}

bool Configuration::SetWithTypeAutoConvert(const std::string &key, const std::string &value)
{
    GUARD(&mLock, mLock);
    auto iter = mValueTypes.find(key);
    auto iterIgnored = std::find(mInvalidSetConfs.begin(), mInvalidSetConfs.end(), key);
    if (iter == mValueTypes.end() || iterIgnored != mInvalidSetConfs.end()) {
        std::cerr << "Invalid key <" << key << ">." << std::endl;
        return false;
    }
    ConfValueType valueType = iter->second;
    if (valueType == ConfValueType::VINT) {
        long tmp = 0;
        if (!OckStol(value, tmp) || tmp > INT32_MAX || tmp < INT32_MIN) {
            std::cerr << "<" << key << "> was empty or in wrong type, it should be a int number." << std::endl;
            return false;
        }
        if (mIntItems.count(key) > 0) {
            mIntItems.at(key) = static_cast<int32_t>(tmp);
        }
    } else if (valueType == ConfValueType::VFLOAT) {
        if (!OckStof(value, mFloatItems.at(key))) {
            std::cerr << "<" << key << "> was empty or in wrong type, it should be a float number." << std::endl;
            return false;
        }
    } else if (valueType == ConfValueType::VSTRING) {
        if (mStrItems.count(key) > 0) {
            std::string tempValue = value;
            if (find(mPathConfs.begin(), mPathConfs.end(), key) != mPathConfs.end() &&
                !GetRealPath(tempValue)) { // 简化路径为绝对路径
                std::cerr << "Simplify <" << key << "> to absolute path failed." << std::endl;
                return false;
            }
            mStrItems.at(key) = tempValue;
        }
    } else if (valueType == ConfValueType::VBOOL) {
        bool b = false;
        if (IsBool(value, b)) {
            std::cerr << "<" << key << "> should represent a bool value." << std::endl;
            return false;
        }
        if (mBoolItems.count(key) > 0) {
            mBoolItems.at(key) = b;
        }
    } else if (valueType == ConfValueType::VLONG) {
        if (!OckStol(value, mLongItems.at(key))) {
            std::cerr << "<" << key << "> was empty or in wrong type, it should be a long number." << std::endl;
            return false;
        }
    }
    return true;
}

void Configuration::SetValidator(const std::string &key, const ValidatorPtr &validator, uint32_t flag)
{
    if (validator == nullptr) {
        std::string errorInfo = "The validator of <" + key + "> create failed, maybe out of memory.";
        mLoadDefaultErrors.push_back(errorInfo);
        return;
    }
    if (mValueValidator.find(key) == mValueValidator.end()) {
        mValueValidator.insert(std::make_pair(key, validator));
    } else {
        mValueValidator.at(key) = validator;
    }
    if (flag & CONF_MUST) {
        mMustKeys.push_back(key);
    }
}

void Configuration::AddIntConf(const std::pair<std::string, int> &pair, const ValidatorPtr &validator, uint32_t flag)
{
    mIntItems.insert(pair);
    mValueTypes.insert(std::make_pair(pair.first, ConfValueType::VINT));
    SetValidator(pair.first, validator, flag);
}

void Configuration::AddStrConf(const std::pair<std::string, std::string> &pair, const ValidatorPtr &validator,
    uint32_t flag)
{
    mStrItems.insert(pair);
    mValueTypes.insert(std::make_pair(pair.first, ConfValueType::VSTRING));
    SetValidator(pair.first, validator, flag);
}

void Configuration::AddPathConf(const std::pair<std::string, std::string> &pair, const ValidatorPtr &validator,
    uint32_t flag)
{
    AddStrConf(pair, validator, flag);
    mPathConfs.push_back(pair.first);
}

void Configuration::AddConverter(const std::string &key, const ConverterPtr &converter)
{
    if (converter == nullptr) {
        std::string errorInfo = "The converter of <" + key + "> create failed, maybe out of memory.";
        mLoadDefaultErrors.push_back(errorInfo);
        return;
    }
    if (mValueConverter.find(key) == mValueConverter.end()) {
        mValueConverter.insert(std::make_pair(key, converter));
    } else {
        mValueConverter.at(key) = converter;
    }
}

void Configuration::ValidateOneType(const std::string &key, const ValidatorPtr &validator,
    std::vector<std::string> &errors, ConfValueType &vType)
{
    if (validator == nullptr) {
        errors.push_back("Failed to validate <" + key + ">, validator is NULL");
        return;
    }
    switch (vType) {
        case ConfValueType::VSTRING: {
            auto valueIterStr = mStrItems.find(key);
            if (valueIterStr == mStrItems.end()) {
                errors.push_back("Failed to find <" + key + "> in string value map, which should not happen.");
                break;
            }
            AddValidateError(validator, errors, valueIterStr);
            break;
        }
        case ConfValueType::VFLOAT: {
            auto valueIterFloat = mFloatItems.find(key);
            if (valueIterFloat == mFloatItems.end()) {
                errors.push_back("Failed to find <" + key + "> in float value map, which should not happen.");
                break;
            }
            AddValidateError(validator, errors, valueIterFloat);
            break;
        }
        case ConfValueType::VINT: {
            auto valueIterInt = mIntItems.find(key);
            if (valueIterInt == mIntItems.end()) {
                errors.push_back("Failed to find <" + key + "> in int value map, which should not happen.");
                break;
            }
            AddValidateError(validator, errors, valueIterInt);
            break;
        }
        case ConfValueType::VLONG: {
            auto valueIterLong = mLongItems.find(key);
            if (valueIterLong == mLongItems.end()) {
                errors.push_back("Failed to find <" + key + "> in long value map, which should not happen.");
                break;
            }
            AddValidateError(validator, errors, valueIterLong);
            break;
        }
        default:;
    }
}

void Configuration::ValidateOneValueMap(std::vector<std::string> &errors,
    const std::map<std::string, ValidatorPtr> &valueValidator)
{
    for (auto &item : valueValidator) {
        auto valueValidatorPtr = item.second.Get();
        if (valueValidatorPtr == nullptr) {
            errors.push_back("The validator of <" + item.first + "> is null, skip.");
            continue;
        }
        // initialize, if failed then skip it
        if (!(valueValidatorPtr->Initialize())) {
            errors.push_back(valueValidatorPtr->ErrorMessage());
            continue;
        }

        // firstly find the value type
        auto typeIter = mValueTypes.find(item.first);
        if (typeIter == mValueTypes.end()) {
            errors.push_back("Failed to find <" + item.first + "> in type map, which should not happen.");
            continue;
        }

        ValidateOneType(item.first, item.second, errors, typeIter->second);
    }
}

void Configuration::ValidateItem(const std::string &itemKey, std::vector<std::string> &errors)
{
    auto validatorIter = mValueValidator.find(itemKey);
    auto typeIter = mValueTypes.find(itemKey);
    if (validatorIter == mValueValidator.end()) {
        errors.push_back("Failed to find <" + itemKey + "> in Validator map, which should not happen.");
        return;
    }
    if (typeIter == mValueTypes.end()) {
        errors.push_back("Failed to find <" + itemKey + "> in type map, which should not happen.");
        return;
    }
    ValidateOneType(validatorIter->first, validatorIter->second, errors, typeIter->second);
}

std::vector<std::string> Configuration::ValidateDaemonConf()
{
    using namespace ConfConstant;
    std::vector<std::string> errors;
    for (auto validate : mValueValidator) {
        if (validate.second == nullptr) {
            std::string errorInfo = "The validator of <" + validate.first + "> create failed, maybe out of memory.";
            errors.push_back(errorInfo);
            continue;
        }
        ValidateItem(validate.first, errors);
    }
    return errors;
}


std::vector<std::string> Configuration::Validate(bool isAuth, bool isTLS, bool isAuthor, bool isZKSecure)
{
    std::vector<std::string> errors;
    ValidateOneValueMap(errors, mValueValidator);

    return errors;
}

void Configuration::LoadConfigurations()
{
    mLoadDefaultErrors.clear();
    mInitialized = false;
    LoadDefault();
    if (!mLoadDefaultErrors.empty()) {
        for (auto &errorInfo : mLoadDefaultErrors) {
            std::cerr << errorInfo << std::endl;
        }
        mLoadDefaultErrors.clear();
        return;
    }
    mLoadDefaultErrors.clear();
    mInitialized = true;
}
} // namespace common
} // namespace ock