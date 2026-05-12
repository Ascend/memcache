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
#include <dlfcn.h>
#include <cstdint>
#include <string>
#include "mmc_logger.h"
#include "dl_ubsio_api.h"

namespace ock {
namespace mmc {
bool DlUbsioApi::gLoaded = false;
std::mutex DlUbsioApi::gMutex;
void *DlUbsioApi::ubsioHandle = nullptr;
const std::string DlUbsioApi::gUbsioLibName = "libubsio_kvc.so";

ubsio_client_initFunc DlUbsioApi::pUbsioClientInit = nullptr;
ubsio_putFunc DlUbsioApi::pUbsioPut = nullptr;
ubsio_getFunc DlUbsioApi::pUbsioGet = nullptr;
ubsio_existFunc DlUbsioApi::pUbsioExist = nullptr;
ubsio_deleteFunc DlUbsioApi::pUbsioDelete = nullptr;
ubsio_get_lengthFunc DlUbsioApi::pUbsioGetLength = nullptr;
ubsio_batch_putFunc DlUbsioApi::pUbsioBatchPut = nullptr;
ubsio_batch_getFunc DlUbsioApi::pUbsioBatchGet = nullptr;
ubsio_batch_get_hbmFunc DlUbsioApi::pUbsioBatchGetWithHBM = nullptr;
ubsio_batch_existFunc DlUbsioApi::pUbsioBatchExist = nullptr;
ubsio_batch_deleteFunc DlUbsioApi::pUbsioBatchDelete = nullptr;
ubsio_batch_get_lengthFunc DlUbsioApi::pUbsioBatchGetLength = nullptr;
ubsio_batch_free_addressFunc DlUbsioApi::pUbsioBatchFreeAddress = nullptr;

Result DlUbsioApi::LoadLibrary()
{
    std::lock_guard<std::mutex> guard(gMutex);
    if (gLoaded) {
        return MMC_OK;
    }

    /* dlopen library */
    ubsioHandle = dlopen(gUbsioLibName.c_str(), RTLD_LAZY | RTLD_GLOBAL);
    if (ubsioHandle == nullptr) {
        MMC_LOG_ERROR("Failed to open library [" << gUbsioLibName << "], error: " << dlerror());
        return MMC_ERROR;
    }

    /* load sym */
    DL_LOAD_SYM(pUbsioClientInit, ubsio_client_initFunc, ubsioHandle, "UbsioKvCacheInit");
    DL_LOAD_SYM(pUbsioPut, ubsio_putFunc, ubsioHandle, "UbsioKvCachePut");
    DL_LOAD_SYM(pUbsioGet, ubsio_getFunc, ubsioHandle, "UbsioKvCacheGet");
    DL_LOAD_SYM(pUbsioExist, ubsio_existFunc, ubsioHandle, "UbsioKvCacheExist");
    DL_LOAD_SYM(pUbsioDelete, ubsio_deleteFunc, ubsioHandle, "UbsioKvCacheDelete");
    DL_LOAD_SYM(pUbsioGetLength, ubsio_get_lengthFunc, ubsioHandle, "UbsioKvCacheGetLength");
    DL_LOAD_SYM(pUbsioBatchPut, ubsio_batch_putFunc, ubsioHandle,  "UbsioKvCacheBatchPut");
    DL_LOAD_SYM(pUbsioBatchGet, ubsio_batch_getFunc, ubsioHandle,  "UbsioKvCacheBatchGet");
    DL_LOAD_SYM(pUbsioBatchGetWithHBM, ubsio_batch_get_hbmFunc, ubsioHandle,  "UbsioKvCacheBatchGetDirect");
    DL_LOAD_SYM(pUbsioBatchExist, ubsio_batch_existFunc, ubsioHandle,  "UbsioKvCacheBatchExist");
    DL_LOAD_SYM(pUbsioBatchDelete, ubsio_batch_deleteFunc, ubsioHandle,  "UbsioKvCacheBatchDelete");
    DL_LOAD_SYM(pUbsioBatchGetLength, ubsio_batch_get_lengthFunc, ubsioHandle,  "UbsioKvCacheBatchGetLength");
    DL_LOAD_SYM(pUbsioBatchFreeAddress, ubsio_batch_free_addressFunc, ubsioHandle,  "UbsioKvCacheBatchFree");

    gLoaded = true;
    return MMC_OK;
}

void DlUbsioApi::CleanupLibrary()
{
    std::lock_guard<std::mutex> guard(gMutex);
    if (!gLoaded) {
        return;
    }

    pUbsioClientInit = nullptr;
    pUbsioPut = nullptr;
    pUbsioGet = nullptr;
    pUbsioExist = nullptr;
    pUbsioDelete = nullptr;
    pUbsioGetLength = nullptr;
    pUbsioBatchPut = nullptr;
    pUbsioBatchGet = nullptr;
    pUbsioBatchGetWithHBM = nullptr;
    pUbsioBatchExist = nullptr;
    pUbsioBatchDelete = nullptr;
    pUbsioBatchGetLength = nullptr;
    pUbsioBatchFreeAddress = nullptr;

    if (ubsioHandle != nullptr) {
        dlclose(ubsioHandle);
        ubsioHandle = nullptr;
    }
    gLoaded = false;
}
}  // namespace mmc
}  // namespace ock
