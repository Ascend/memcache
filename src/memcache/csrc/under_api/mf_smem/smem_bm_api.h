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
#ifndef __SMEM_HYBM_CORE_API_H__
#define __SMEM_HYBM_CORE_API_H__

#include "smem_bm_def.h"
#include "mmc_common_includes.h"

namespace ock {
namespace mmc {

using smemInitFunc = int32_t (*)(uint32_t);
using smemUnInitFunc = void (*)();
using smemSetExternLoggerFunc = int32_t (*)(void (*)(int level, const char *));
using smemSetLogLevelFunc = int32_t (*)(int);
using smemGetLastErrMsgFunc = const char *(*)();
using smemGetAndClearLastErrMsgFunc = const char *(*)();

using smemBmConfigInitFunc = int32_t (*)(smem_bm_config_t *);
using smemBmInitFunc = int32_t (*)(const char *, uint32_t, uint16_t, const smem_bm_config_t *);
using smemBmUnInitFunc = void (*)(uint32_t);
using smemBmGetRankIdFunc = uint32_t (*)();
using smemBmCreate2Func = smem_bm_t (*)(uint32_t, const smem_bm_create_option_t *);
using smemBmDestroyFunc = void (*)(smem_bm_t);
using smemBmJoinFunc = int32_t (*)(smem_bm_t, uint32_t);
using smemBmPtrByMemTypeFunc = void *(*)(smem_bm_t, smem_bm_mem_type_t, uint16_t);
using smemBmGetLocalMemSizeByMemTypeFunc = uint64_t (*)(smem_bm_t, smem_bm_mem_type_t);
using smemBmCopyFunc = int32_t (*)(smem_bm_t, smem_copy_params_t *, smem_bm_copy_type_t, uint32_t);
using smemBmCopyBatchFunc = int32_t (*)(smem_bm_t, smem_batch_copy_params_t *, smem_bm_copy_type_t, uint32_t);
using smemBmRegisterUserMemFunc = int32_t (*)(smem_bm_t, uint64_t, uint64_t);
using smemBmUnregisterUserMemFunc = int32_t (*)(smem_bm_t, uint64_t);
using smemBmWaitFunc = int32_t (*)(smem_bm_t);

class MFSmemApi {
public:
    static std::string ResolveLibDir();

    static Result LoadLibrary(const std::string &libDirPath);
    static void CleanupLibrary();

    static int32_t SmemInit(uint32_t flags)
    {
        return gSmemInit(flags);
    }

    static int32_t SmemSetExternLogger(void (*func)(int level, const char *msg))
    {
        return gSmemSetExternLogger(func);
    }

    static int32_t SmemSetLogLevel(int level)
    {
        return gSmemSetLogLevel(level);
    }

    static void SmemUninit()
    {
        gSmemUnInit();
    }

    static const char *SmemGetLastErrMsg()
    {
        return gSemGetLastErrMsg();
    }

    static const char *SmemGetAndClearLastErrMsg()
    {
        return gSmemGetAndClearLastErrMsg();
    }

    static int32_t SmemBmConfigInit(smem_bm_config_t *config)
    {
        return gSmemBmConfigInit(config);
    }

    static int32_t SmemBmInit(const char *storeURL, uint32_t worldSize, uint16_t deviceId,
                              const smem_bm_config_t *config)
    {
        return gSmemBmInit(storeURL, worldSize, deviceId, config);
    }

    static void SmemBmUninit(uint32_t flags)
    {
        gSmemBmUnInit(flags);
    }

    static uint32_t SmemBmGetRankId(void)
    {
        return gSmemBmGetRankId();
    }

    static smem_bm_t SmemBmCreate2(uint32_t id, const smem_bm_create_option_t *option)
    {
        return gSmemBmCreate2(id, option);
    }

    static void SmemBmDestroy(smem_bm_t handle)
    {
        gSmemBmDestroy(handle);
    }

    static int32_t SmemBmJoin(smem_bm_t handle, uint32_t flags)
    {
        return gSmemBmJoin(handle, flags);
    }

    static void *SmemBmPtrByMemType(smem_bm_t handle, smem_bm_mem_type_t memType, uint16_t peerRankId)
    {
        return gSmemBmPtrByMemType(handle, memType, peerRankId);
    }

    static uint64_t SmemBmGetLocalMemSizeByMemType(smem_bm_t handle, smem_bm_mem_type_t memType)
    {
        return gSmemBmGetLocalMemSizeByMemType(handle, memType);
    }

    static int32_t SmemBmCopy(smem_bm_t handle, smem_copy_params_t *params, smem_bm_copy_type_t type, uint32_t flags)
    {
        return gSmemBmCopy(handle, params, type, flags);
    }

    static int32_t SmemBmCopyBatch(smem_bm_t handle, smem_batch_copy_params_t *params, smem_bm_copy_type_t type,
                                   uint32_t flags)
    {
        return gSmemBmCopyBatch(handle, params, type, flags);
    }

    static int32_t SmemBmRegisterUserMem(smem_bm_t handle, uint64_t addr, uint64_t size)
    {
        return gSmemBmRegisterUserMem(handle, addr, size);
    }

    static int32_t SmemBmUnregisterUserMem(smem_bm_t handle, uint64_t addr)
    {
        return gSmemBmUnregisterUserMem(handle, addr);
    }

    static int32_t SmemBmWait(smem_bm_t handle)
    {
        return gSmemBmWait(handle);
    }

private:
    static Result LoadAllSymbols();
    static Result LoadSymbol(const char *symbolName, void **target);
    static void ClearAllSymbols();

private:
    static std::mutex gMutex;
    static bool gLoaded;
    static void *gSmemHandle;
    static const char *gSmemLibName;

    static smemInitFunc gSmemInit;
    static smemUnInitFunc gSmemUnInit;
    static smemSetExternLoggerFunc gSmemSetExternLogger;
    static smemSetLogLevelFunc gSmemSetLogLevel;
    static smemGetLastErrMsgFunc gSemGetLastErrMsg;
    static smemGetAndClearLastErrMsgFunc gSmemGetAndClearLastErrMsg;

    static smemBmConfigInitFunc gSmemBmConfigInit;
    static smemBmInitFunc gSmemBmInit;
    static smemBmUnInitFunc gSmemBmUnInit;
    static smemBmGetRankIdFunc gSmemBmGetRankId;
    static smemBmCreate2Func gSmemBmCreate2;
    static smemBmDestroyFunc gSmemBmDestroy;
    static smemBmJoinFunc gSmemBmJoin;
    static smemBmPtrByMemTypeFunc gSmemBmPtrByMemType;
    static smemBmGetLocalMemSizeByMemTypeFunc gSmemBmGetLocalMemSizeByMemType;
    static smemBmCopyFunc gSmemBmCopy;
    static smemBmCopyBatchFunc gSmemBmCopyBatch;
    static smemBmRegisterUserMemFunc gSmemBmRegisterUserMem;
    static smemBmUnregisterUserMemFunc gSmemBmUnregisterUserMem;
    static smemBmWaitFunc gSmemBmWait;
};
} // namespace mmc
} // namespace ock

#endif // __SMEM_HYBM_CORE_API_H__
