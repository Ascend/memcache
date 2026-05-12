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

using ubsio_client_initFunc = int32_t (*)(int32_t);
using ubsio_putFunc = int32_t (*)(const char *, void *, size_t, uint32_t);
using ubsio_getFunc = int32_t (*)(const char *, void *, size_t, uint32_t);
using ubsio_existFunc = bool (*)(const char *, uint32_t);
using ubsio_deleteFunc = int32_t (*)(const char *, uint32_t);
using ubsio_get_lengthFunc = int32_t (*)(const char *, size_t *, uint32_t);
using ubsio_batch_putFunc = int32_t (*)(const char **, uint32_t, void **, size_t *, int *, uint32_t);
using ubsio_batch_getFunc = int32_t (*)(const char **, uint32_t, void **, size_t *, int *, uint32_t);
using ubsio_batch_get_hbmFunc = int32_t (*)(const char **, uint32_t, void ***, size_t **,
                                            uint32_t, uint32_t, int *, uint32_t);
using ubsio_batch_existFunc = int32_t (*)(const char **, uint32_t, bool *, uint32_t);
using ubsio_batch_deleteFunc = int32_t (*)(const char **, uint32_t, int32_t *, uint32_t);
using ubsio_batch_get_lengthFunc = int32_t (*)(const char **, uint32_t, size_t *, int32_t *, uint32_t);
using ubsio_batch_free_addressFunc = int32_t (*)(void **, uint32_t);

class DlUbsioApi {
public:
    static Result LoadLibrary();
    static void CleanupLibrary();

    static inline Result UbsioClientInit(int32_t deviceId)
    {
        if (pUbsioClientInit == nullptr) {
            return MMC_NOT_INITIALIZED;
        }
        return pUbsioClientInit(deviceId);
    }

    static inline Result UbsioPut(const char *key, void *buf, size_t length, uint32_t flags)
    {
        if (pUbsioPut == nullptr) {
            return MMC_NOT_INITIALIZED;
        }
        return pUbsioPut(key, buf, length, flags);
    }

    static inline Result UbsioGet(const char *key, void *buf, size_t length, uint32_t flags)
    {
        if (pUbsioGet == nullptr) {
            return MMC_NOT_INITIALIZED;
        }
        return pUbsioGet(key, buf, length, flags);
    }

    static inline Result UbsioExist(const char *key, uint32_t flags)
    {
        if (pUbsioExist == nullptr) {
            return MMC_NOT_INITIALIZED;
        }
        return pUbsioExist(key, flags);
    }

    static inline Result UbsioDelete(const char *key, uint32_t flags)
    {
        if (pUbsioDelete == nullptr) {
            return MMC_NOT_INITIALIZED;
        }
        return pUbsioDelete(key, flags);
    }

    static inline Result UbsioGetLength(const char *key, size_t *length, uint32_t flags)
    {
        if (pUbsioGetLength == nullptr) {
            return MMC_NOT_INITIALIZED;
        }
        return pUbsioGetLength(key, length, flags);
    }

    static inline Result UbsioBatchPut(const char **keys, uint32_t keys_count, void **bufs, size_t *lengths,
                                     int *results, uint32_t flags)
    {
        if (pUbsioBatchPut == nullptr) {
            return MMC_NOT_INITIALIZED;
        }
        return pUbsioBatchPut(keys, keys_count, bufs, lengths, results, flags);
    }

    static inline Result UbsioBatchGet(const char **keys, uint32_t keys_count, void **bufs, size_t *lengths,
                                     int *results, uint32_t flags)
    {
        if (pUbsioBatchGet == nullptr) {
            return MMC_NOT_INITIALIZED;
        }
        return pUbsioBatchGet(keys, keys_count, bufs, lengths, results, flags);
    }

    static inline Result UbsioBatchGetWithHBM(const char **keys, uint32_t keys_count, void ***bufs, size_t **lengths,
                                     uint32_t lengthsRows, uint32_t lengthsCols, int *results, uint32_t flags)
    {
        if (pUbsioBatchGetWithHBM == nullptr) {
            return MMC_NOT_INITIALIZED;
        }
        return pUbsioBatchGetWithHBM(keys, keys_count, bufs, lengths, lengthsRows, lengthsCols, results, flags);
    }

    static inline Result UbsioBatchExist(const char **keys, uint32_t keys_count, bool *results, uint32_t flags)
    {
        if (pUbsioBatchExist == nullptr) {
            return MMC_NOT_INITIALIZED;
        }
        return pUbsioBatchExist(keys, keys_count, results, flags);
    }

    static inline Result UbsioBatchDelete(const char **keys, uint32_t keys_count, int32_t *results, uint32_t flags)
    {
        if (pUbsioBatchDelete == nullptr) {
            return MMC_NOT_INITIALIZED;
        }
        return pUbsioBatchDelete(keys, keys_count, results, flags);
    }

    static inline Result UbsioBatchGetLength(const char **keys, uint32_t keys_count, size_t *lengths,
                                           int32_t *results, uint32_t flags)
    {
        if (pUbsioBatchGetLength == nullptr) {
            return MMC_NOT_INITIALIZED;
        }
        return pUbsioBatchGetLength(keys, keys_count, lengths, results, flags);
    }

    static inline Result UbsioBatchFreeAddress(void **bufs, uint32_t keys_count)
    {
        if (pUbsioBatchFreeAddress == nullptr) {
            return MMC_NOT_INITIALIZED;
        }
        return pUbsioBatchFreeAddress(bufs, keys_count);
    }

private:
    static std::mutex gMutex;
    static bool gLoaded;
    static void *ubsioHandle;
    static const std::string gUbsioLibName;

    static ubsio_client_initFunc pUbsioClientInit;
    static ubsio_putFunc pUbsioPut;
    static ubsio_getFunc pUbsioGet;
    static ubsio_existFunc pUbsioExist;
    static ubsio_deleteFunc pUbsioDelete;
    static ubsio_get_lengthFunc pUbsioGetLength;
    static ubsio_batch_putFunc pUbsioBatchPut;
    static ubsio_batch_getFunc pUbsioBatchGet;
    static ubsio_batch_get_hbmFunc pUbsioBatchGetWithHBM;
    static ubsio_batch_existFunc pUbsioBatchExist;
    static ubsio_batch_deleteFunc pUbsioBatchDelete;
    static ubsio_batch_get_lengthFunc pUbsioBatchGetLength;
    static ubsio_batch_free_addressFunc pUbsioBatchFreeAddress;
};
}  // namespace mmc
}  // namespace ock

#endif  // MEM_FABRIC_MMC_DL_UBS_IO_API_H
