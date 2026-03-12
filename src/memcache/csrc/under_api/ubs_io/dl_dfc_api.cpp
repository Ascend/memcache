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
#include "dl_dfc_api.h"

namespace ock {
namespace mmc {
bool DlDfcApi::gLoaded = false;
std::mutex DlDfcApi::gMutex;
void *DlDfcApi::dfcHandle = nullptr;
const std::string DlDfcApi::gDfcLibName = "libdfc_client.so";

dfc_client_initFunc DlDfcApi::pDfcClientInit = nullptr;
dfc_putFunc DlDfcApi::pDfcPut = nullptr;
dfc_getFunc DlDfcApi::pDfcGet = nullptr;
dfc_existFunc DlDfcApi::pDfcExist = nullptr;
dfc_deleteFunc DlDfcApi::pDfcDelete = nullptr;
dfc_get_lengthFunc DlDfcApi::pDfcGetLength = nullptr;
dfc_batch_putFunc DlDfcApi::pDfcBatchPut = nullptr;
dfc_batch_getFunc DlDfcApi::pDfcBatchGet = nullptr;
dfc_batch_existFunc DlDfcApi::pDfcBatchExist = nullptr;
dfc_batch_deleteFunc DlDfcApi::pDfcBatchDelete = nullptr;
dfc_batch_get_lengthFunc DlDfcApi::pDfcBatchGetLength = nullptr;
dfc_batch_free_addressFunc DlDfcApi::pDfcBatchFreeAddress = nullptr;

Result DlDfcApi::LoadLibrary()
{
    std::lock_guard<std::mutex> guard(gMutex);
    if (gLoaded) {
        return MMC_OK;
    }

    /* dlopen library */
    dfcHandle = dlopen(gDfcLibName.c_str(), RTLD_LAZY | RTLD_GLOBAL);
    if (dfcHandle == nullptr) {
        MMC_LOG_ERROR("Failed to open library [" << gDfcLibName << "], error: " << dlerror());
        return MMC_ERROR;
    }

    /* load sym */
    DL_LOAD_SYM(pDfcClientInit, dfc_client_initFunc, dfcHandle, "DfcClientInit");
    DL_LOAD_SYM(pDfcPut, dfc_putFunc, dfcHandle, "DfcPut");
    DL_LOAD_SYM(pDfcGet, dfc_getFunc, dfcHandle, "DfcGet");
    DL_LOAD_SYM(pDfcExist, dfc_existFunc, dfcHandle, "DfcExist");
    DL_LOAD_SYM(pDfcDelete, dfc_deleteFunc, dfcHandle, "DfcDelete");
    DL_LOAD_SYM(pDfcGetLength, dfc_get_lengthFunc, dfcHandle, "DfcGetLength");
    DL_LOAD_SYM(pDfcBatchPut, dfc_batch_putFunc, dfcHandle,  "DfcBatchPut");
    DL_LOAD_SYM(pDfcBatchGet, dfc_batch_getFunc, dfcHandle,  "DfcBatchGet");
    DL_LOAD_SYM(pDfcBatchExist, dfc_batch_existFunc, dfcHandle,  "DfcBatchExist");
    DL_LOAD_SYM(pDfcBatchDelete, dfc_batch_deleteFunc, dfcHandle,  "DfcBatchDelete");
    DL_LOAD_SYM(pDfcBatchGetLength, dfc_batch_get_lengthFunc, dfcHandle,  "DfcBatchGetLength");
    DL_LOAD_SYM(pDfcBatchFreeAddress, dfc_batch_free_addressFunc, dfcHandle,  "DfcBatchFreeAddress");

    gLoaded = true;
    return MMC_OK;
}

void DlDfcApi::CleanupLibrary()
{
    std::lock_guard<std::mutex> guard(gMutex);
    if (!gLoaded) {
        return;
    }

    pDfcClientInit = nullptr;
    pDfcPut = nullptr;
    pDfcGet = nullptr;
    pDfcExist = nullptr;
    pDfcDelete = nullptr;
    pDfcGetLength = nullptr;
    pDfcBatchPut = nullptr;
    pDfcBatchGet = nullptr;
    pDfcBatchExist = nullptr;
    pDfcBatchDelete = nullptr;
    pDfcBatchGetLength = nullptr;
    pDfcBatchFreeAddress = nullptr;

    if (dfcHandle != nullptr) {
        dlclose(dfcHandle);
        dfcHandle = nullptr;
    }
    gLoaded = false;
}
}  // namespace mmc
}  // namespace ock
