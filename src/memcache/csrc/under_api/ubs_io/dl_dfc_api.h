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

#ifndef MEM_FABRIC_MMC_DL_UBS_IO_API_H
#define MEM_FABRIC_MMC_DL_UBS_IO_API_H

#include <cstdint>
#include <cstddef>
#include <mutex>
#include <string>
#include "mmc_types.h"
#include "mmc_functions.h"

namespace ock {
namespace mmc {

// 定义 dfc.h 中的类型，因为不会包含 dfc.h
enum class kv_worker_mode {
    KV_CONVERGENCE,
    KV_SEPARATES
};

// 定义函数指针类型
using dfc_client_initFunc = int32_t (*)(kv_worker_mode, int32_t);
using dfc_putFunc = int32_t (*)(const char *, void *, size_t, uint32_t);
using dfc_getFunc = int32_t (*)(const char *, void *, size_t, uint32_t);
using dfc_existFunc = bool (*)(const char *, uint32_t);
using dfc_deleteFunc = int32_t (*)(const char *, uint32_t);
using dfc_get_lengthFunc = int32_t (*)(const char *, size_t *, uint32_t);
using dfc_batch_putFunc = int32_t (*)(const char **, uint32_t, void **, size_t *, int *, uint32_t);
using dfc_batch_getFunc = int32_t (*)(const char **, uint32_t, void **, size_t *, int *, uint32_t);
using dfc_batch_get_hbmFunc = int32_t (*)(const char **, uint32_t, void ***, size_t **,
                                            uint32_t, uint32_t, int *, uint32_t);
using dfc_batch_existFunc = int32_t (*)(const char **, uint32_t, bool *, uint32_t);
using dfc_batch_deleteFunc = int32_t (*)(const char **, uint32_t, int32_t *, uint32_t);
using dfc_batch_get_lengthFunc = int32_t (*)(const char **, uint32_t, size_t *, int32_t *, uint32_t);
using dfc_batch_free_addressFunc = int32_t (*)(void **, uint32_t);

class DlDfcApi {
public:
    static Result LoadLibrary();
    static void CleanupLibrary();

    static inline Result DfcClientInit(kv_worker_mode mode, int32_t deviceId)
    {
        if (pDfcClientInit == nullptr) {
            return MMC_NOT_INITIALIZED;
        }
        return pDfcClientInit(mode, deviceId);
    }

    static inline Result DfcPut(const char *key, void *buf, size_t length, uint32_t flags)
    {
        if (pDfcPut == nullptr) {
            return MMC_NOT_INITIALIZED;
        }
        return pDfcPut(key, buf, length, flags);
    }

    static inline Result DfcGet(const char *key, void *buf, size_t length, uint32_t flags)
    {
        if (pDfcGet == nullptr) {
            return MMC_NOT_INITIALIZED;
        }
        return pDfcGet(key, buf, length, flags);
    }

    static inline Result DfcExist(const char *key, uint32_t flags)
    {
        if (pDfcExist == nullptr) {
            return MMC_NOT_INITIALIZED;
        }
        return pDfcExist(key, flags);
    }

    static inline Result DfcDelete(const char *key, uint32_t flags)
    {
        if (pDfcDelete == nullptr) {
            return MMC_NOT_INITIALIZED;
        }
        return pDfcDelete(key, flags);
    }

    static inline Result DfcGetLength(const char *key, size_t *length, uint32_t flags)
    {
        if (pDfcGetLength == nullptr) {
            return MMC_NOT_INITIALIZED;
        }
        return pDfcGetLength(key, length, flags);
    }

    static inline Result DfcBatchPut(const char **keys, uint32_t keys_count, void **bufs, size_t *lengths,
                                     int *results, uint32_t flags)
    {
        if (pDfcBatchPut == nullptr) {
            return MMC_NOT_INITIALIZED;
        }
        return pDfcBatchPut(keys, keys_count, bufs, lengths, results, flags);
    }

    static inline Result DfcBatchGet(const char **keys, uint32_t keys_count, void **bufs, size_t *lengths,
                                     int *results, uint32_t flags)
    {
        if (pDfcBatchGet == nullptr) {
            return MMC_NOT_INITIALIZED;
        }
        return pDfcBatchGet(keys, keys_count, bufs, lengths, results, flags);
    }

    static inline Result DfcBatchGetWithHBM(const char **keys, uint32_t keys_count, void ***bufs, size_t **lengths,
                                     uint32_t lengthsRows, uint32_t lengthsCols, int *results, uint32_t flags)
    {
        if (pDfcBatchGetWithHBM == nullptr) {
            return MMC_NOT_INITIALIZED;
        }
        return pDfcBatchGetWithHBM(keys, keys_count, bufs, lengths, lengthsRows, lengthsCols, results, flags);
    }

    static inline Result DfcBatchExist(const char **keys, uint32_t keys_count, bool *results, uint32_t flags)
    {
        if (pDfcBatchExist == nullptr) {
            return MMC_NOT_INITIALIZED;
        }
        return pDfcBatchExist(keys, keys_count, results, flags);
    }

    static inline Result DfcBatchDelete(const char **keys, uint32_t keys_count, int32_t *results, uint32_t flags)
    {
        if (pDfcBatchDelete == nullptr) {
            return MMC_NOT_INITIALIZED;
        }
        return pDfcBatchDelete(keys, keys_count, results, flags);
    }

    static inline Result DfcBatchGetLength(const char **keys, uint32_t keys_count, size_t *lengths,
                                           int32_t *results, uint32_t flags)
    {
        if (pDfcBatchGetLength == nullptr) {
            return MMC_NOT_INITIALIZED;
        }
        return pDfcBatchGetLength(keys, keys_count, lengths, results, flags);
    }

    static inline Result DfcBatchFreeAddress(void **bufs, uint32_t keys_count)
    {
        if (pDfcBatchFreeAddress == nullptr) {
            return MMC_NOT_INITIALIZED;
        }
        return pDfcBatchFreeAddress(bufs, keys_count);
    }

private:
    static std::mutex gMutex;
    static bool gLoaded;
    static void *dfcHandle;
    static const std::string gDfcLibName;

    static dfc_client_initFunc pDfcClientInit;
    static dfc_putFunc pDfcPut;
    static dfc_getFunc pDfcGet;
    static dfc_existFunc pDfcExist;
    static dfc_deleteFunc pDfcDelete;
    static dfc_get_lengthFunc pDfcGetLength;
    static dfc_batch_putFunc pDfcBatchPut;
    static dfc_batch_getFunc pDfcBatchGet;
    static dfc_batch_get_hbmFunc pDfcBatchGetWithHBM;
    static dfc_batch_existFunc pDfcBatchExist;
    static dfc_batch_deleteFunc pDfcBatchDelete;
    static dfc_batch_get_lengthFunc pDfcBatchGetLength;
    static dfc_batch_free_addressFunc pDfcBatchFreeAddress;
};
}  // namespace mmc
}  // namespace ock

#endif  // MEM_FABRIC_MMC_DL_UBS_IO_API_H
