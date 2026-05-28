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
#include <cstdlib>
#include <dlfcn.h>
#include "mmc_functions.h"
#include "smem_bm_api.h"

namespace ock {
namespace mmc {

namespace {
constexpr const char *kMfExtendLibPathEnv = "MEMFABRIC_HYBRID_EXTEND_LIB_PATH";

struct SmemSymbolEntry {
    void **target;
    const char *name;
};
} // namespace

bool MFSmemApi::gLoaded = false;
std::mutex MFSmemApi::gMutex;
void *MFSmemApi::gSmemHandle = nullptr;
const char *MFSmemApi::gSmemLibName = "libmf_smem.so";

smemInitFunc MFSmemApi::gSmemInit = nullptr;
smemUnInitFunc MFSmemApi::gSmemUnInit = nullptr;
smemSetExternLoggerFunc MFSmemApi::gSmemSetExternLogger = nullptr;
smemSetLogLevelFunc MFSmemApi::gSmemSetLogLevel = nullptr;
smemGetLastErrMsgFunc MFSmemApi::gSemGetLastErrMsg = nullptr;
smemGetAndClearLastErrMsgFunc MFSmemApi::gSmemGetAndClearLastErrMsg = nullptr;

smemBmConfigInitFunc MFSmemApi::gSmemBmConfigInit = nullptr;
smemBmInitFunc MFSmemApi::gSmemBmInit = nullptr;
smemBmUnInitFunc MFSmemApi::gSmemBmUnInit = nullptr;
smemBmGetRankIdFunc MFSmemApi::gSmemBmGetRankId = nullptr;
smemBmCreate2Func MFSmemApi::gSmemBmCreate2 = nullptr;
smemBmDestroyFunc MFSmemApi::gSmemBmDestroy = nullptr;
smemBmJoinFunc MFSmemApi::gSmemBmJoin = nullptr;
smemBmPtrByMemTypeFunc MFSmemApi::gSmemBmPtrByMemType = nullptr;
smemBmGetLocalMemSizeByMemTypeFunc MFSmemApi::gSmemBmGetLocalMemSizeByMemType = nullptr;
smemBmCopyFunc MFSmemApi::gSmemBmCopy = nullptr;
smemBmCopyBatchFunc MFSmemApi::gSmemBmCopyBatch = nullptr;
smemBmRegisterUserMemFunc MFSmemApi::gSmemBmRegisterUserMem = nullptr;
smemBmUnregisterUserMemFunc MFSmemApi::gSmemBmUnregisterUserMem = nullptr;
smemBmWaitFunc MFSmemApi::gSmemBmWait = nullptr;

std::string MFSmemApi::ResolveLibDir()
{
    const auto libDir = SafeGetEnv(kMfExtendLibPathEnv);
    if (libDir.empty()) {
        MMC_LOG_ERROR("MFSmemApi: env " << kMfExtendLibPathEnv << " is not set; source memfabric_hybrid set_env.sh");
        return {};
    }

    std::string realPath;
    if (Func::LibraryRealPath(libDir, std::string(gSmemLibName), realPath) != MMC_OK) {
        MMC_LOG_ERROR("MFSmemApi: " << gSmemLibName << " not found under " << libDir << "; check "
                                    << kMfExtendLibPathEnv);
        return {};
    }

    MMC_LOG_INFO("MFSmemApi: resolved " << gSmemLibName << " lib dir to " << libDir);
    return libDir;
}

Result MFSmemApi::LoadAllSymbols()
{
    static const SmemSymbolEntry kSmemSymbols[] = {
        {reinterpret_cast<void **>(&gSmemInit), "smem_init"},
        {reinterpret_cast<void **>(&gSmemUnInit), "smem_uninit"},
        {reinterpret_cast<void **>(&gSmemSetExternLogger), "smem_set_extern_logger"},
        {reinterpret_cast<void **>(&gSmemSetLogLevel), "smem_set_log_level"},
        {reinterpret_cast<void **>(&gSemGetLastErrMsg), "smem_get_last_err_msg"},
        {reinterpret_cast<void **>(&gSmemGetAndClearLastErrMsg), "smem_get_and_clear_last_err_msg"},
        {reinterpret_cast<void **>(&gSmemBmConfigInit), "smem_bm_config_init"},
        {reinterpret_cast<void **>(&gSmemBmInit), "smem_bm_init"},
        {reinterpret_cast<void **>(&gSmemBmUnInit), "smem_bm_uninit"},
        {reinterpret_cast<void **>(&gSmemBmGetRankId), "smem_bm_get_rank_id"},
        {reinterpret_cast<void **>(&gSmemBmCreate2), "smem_bm_create2"},
        {reinterpret_cast<void **>(&gSmemBmDestroy), "smem_bm_destroy"},
        {reinterpret_cast<void **>(&gSmemBmJoin), "smem_bm_join"},
        {reinterpret_cast<void **>(&gSmemBmPtrByMemType), "smem_bm_ptr_by_mem_type"},
        {reinterpret_cast<void **>(&gSmemBmGetLocalMemSizeByMemType), "smem_bm_get_local_mem_size_by_mem_type"},
        {reinterpret_cast<void **>(&gSmemBmCopy), "smem_bm_copy"},
        {reinterpret_cast<void **>(&gSmemBmCopyBatch), "smem_bm_copy_batch"},
        {reinterpret_cast<void **>(&gSmemBmRegisterUserMem), "smem_bm_register_user_mem"},
        {reinterpret_cast<void **>(&gSmemBmUnregisterUserMem), "smem_bm_unregister_user_mem"},
        {reinterpret_cast<void **>(&gSmemBmWait), "smem_bm_wait"},
    };

    for (const auto &symbol : kSmemSymbols) {
        if (LoadSymbol(symbol.name, symbol.target) != MMC_OK) {
            return MMC_ERROR;
        }
    }
    return MMC_OK;
}

Result MFSmemApi::LoadSymbol(const char *symbolName, void **target)
{
    dlerror();
    void *sym = dlsym(gSmemHandle, symbolName);
    const char *err = dlerror();
    if (sym == nullptr) {
        MMC_LOG_ERROR("MFSmemApi dlsym failed, symbol: " << symbolName << ", lib: " << gSmemLibName <<
                        ", error: " << (err != nullptr ? err : "unknown"));
        return MMC_ERROR;
    }
    *target = sym;
    return MMC_OK;
}

Result MFSmemApi::LoadLibrary(const std::string &libDirPath)
{
    std::lock_guard<std::mutex> guard(gMutex);
    if (libDirPath.empty()) {
        MMC_LOG_ERROR("MFSmemApi LoadLibrary: libDirPath is empty; set env " << kMfExtendLibPathEnv);
        return MMC_INVALID_PARAM;
    }

    if (gLoaded) {
        return MMC_OK;
    }

    MMC_LOG_INFO("MFSmemApi LoadLibrary: dlopen " << gSmemLibName << " from " << libDirPath);
    std::string realPath;
    auto ret = Func::LibraryRealPath(libDirPath, std::string(gSmemLibName), realPath);
    if (ret != MMC_OK) {
        MMC_LOG_ERROR("MFSmemApi LoadLibrary: resolve " << gSmemLibName << " under dir " << libDirPath
                                                        << " failed, ret: " << ret);
        return ret;
    }

    gSmemHandle = dlopen(realPath.c_str(), RTLD_NOW | RTLD_LOCAL);
    if (gSmemHandle == nullptr) {
        MMC_LOG_ERROR("MFSmemApi LoadLibrary: dlopen failed, path: " << realPath << ", error: " << dlerror());
        return MMC_ERROR;
    }

    if (LoadAllSymbols() != MMC_OK) {
        MMC_LOG_ERROR("MFSmemApi LoadLibrary: dlsym smem_bm symbols failed, path: " << realPath);
        dlclose(gSmemHandle);
        gSmemHandle = nullptr;
        return MMC_ERROR;
    }

    gLoaded = true;
    MMC_LOG_INFO("MFSmemApi LoadLibrary: loaded 20 smem_bm symbols from " << realPath);
    return MMC_OK;
}

void MFSmemApi::ClearAllSymbols()
{
    gSmemInit = nullptr;
    gSmemUnInit = nullptr;
    gSmemSetExternLogger = nullptr;
    gSmemSetLogLevel = nullptr;
    gSemGetLastErrMsg = nullptr;
    gSmemGetAndClearLastErrMsg = nullptr;
    gSmemBmConfigInit = nullptr;
    gSmemBmInit = nullptr;
    gSmemBmUnInit = nullptr;
    gSmemBmGetRankId = nullptr;
    gSmemBmCreate2 = nullptr;
    gSmemBmDestroy = nullptr;
    gSmemBmJoin = nullptr;
    gSmemBmPtrByMemType = nullptr;
    gSmemBmGetLocalMemSizeByMemType = nullptr;
    gSmemBmCopy = nullptr;
    gSmemBmCopyBatch = nullptr;
    gSmemBmRegisterUserMem = nullptr;
    gSmemBmUnregisterUserMem = nullptr;
    gSmemBmWait = nullptr;
}

void MFSmemApi::CleanupLibrary()
{
    std::lock_guard<std::mutex> guard(gMutex);
    if (!gLoaded) {
        return;
    }

    ClearAllSymbols();
    if (gSmemHandle != nullptr) {
        dlclose(gSmemHandle);
        gSmemHandle = nullptr;
    }
    gLoaded = false;
    MMC_LOG_INFO("MFSmemApi CleanupLibrary: unloaded " << gSmemLibName);
}
} // namespace mmc
} // namespace ock
