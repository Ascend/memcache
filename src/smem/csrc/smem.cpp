/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */
#include "smem.h"

#include <atomic>

#include "smem_common_includes.h"
#include "smem_version.h"
#include "hybm.h"
#include "smem_store_factory.h"

namespace {
bool g_smemInited = false;
}

SMEM_API int32_t smem_init(uint32_t flags)
{
    using namespace ock::smem;
    g_smemInited = true;
    SM_LOG_INFO("smem init successfully, " << LIB_VERSION);
    return SM_OK;
}

SMEM_API int32_t smem_create_config_store(const char *storeUrl)
{
    static std::atomic<uint32_t> callNum = { 0 };

    if (storeUrl == nullptr) {
        SM_LOG_ERROR("input store URL is null.");
        return ock::smem::SM_INVALID_PARAM;
    }

    ock::smem::UrlExtraction extraction;
    auto ret = extraction.ExtractIpPortFromUrl(storeUrl);
    if (ret != 0) {
        SM_LOG_ERROR("input store URL invalid.");
        return ock::smem::SM_INVALID_PARAM;
    }

    auto store = ock::smem::StoreFactory::CreateStoreServer(extraction.ip, extraction.port);
    if (store == nullptr) {
        SM_LOG_ERROR("create store server failed with URL.");
        return ock::smem::SM_ERROR;
    }

    if (callNum.fetch_add(1U) == 0) {
        pthread_atfork(
            []() {}, // 父进程 fork 前：释放锁等资源
            []() {}, // 父进程 fork 后：无特殊操作
            []() {
                ock::smem::StoreFactory::DestroyStoreAll(true);
            });
    }

    return ock::smem::SM_OK;
}

SMEM_API void smem_uninit()
{
    g_smemInited = false;
}

SMEM_API int32_t smem_set_extern_logger(void (*fun)(int, const char *))
{
    using namespace ock::smem;
    SM_VALIDATE_RETURN(fun != nullptr, "set extern logger failed, invalid func which is NULL", SM_INVALID_PARAM);

    /* set my out logger */
    ock::mf::OutLogger::Instance().SetExternalLogFunction(fun, true);
    return SM_OK;
}

SMEM_API int32_t smem_set_log_level(int level)
{
    using namespace ock::smem;
    SM_VALIDATE_RETURN(ock::mf::OutLogger::ValidateLevel(level),
                       "set log level failed, invalid param, level should be 0~3", SM_INVALID_PARAM);

    /* set my logger's level */
    ock::mf::OutLogger::Instance().SetLogLevel(static_cast<ock::mf::LogLevel>(level));
    return SM_OK;
}

SMEM_API const char *smem_get_last_err_msg()
{
    return ock::smem::SmLastError::GetAndClear(false);
}

SMEM_API const char *smem_get_and_clear_last_err_msg()
{
    return ock::smem::SmLastError::GetAndClear(true);
}