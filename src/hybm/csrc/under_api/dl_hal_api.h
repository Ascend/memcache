/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023. All rights reserved.
 */

#ifndef MF_HYBM_CORE_DL_HAL_API_H
#define MF_HYBM_CORE_DL_HAL_API_H

#include <cstddef>
#include <mutex>

#include "dl_hal_api_def.h"
#include "hybm_types.h"

namespace ock {
namespace mf {
using halSvmModuleAllocedSizeIncFunc = void (*)(void *, uint32_t, uint32_t, uint64_t);
using halDevmmVirtAllocMemFromBaseFunc = uint64_t (*)(void *, size_t, uint32_t, uint64_t);
using halDevmmIoctlEnableHeapFunc = int32_t (*)(uint32_t, uint32_t, uint32_t, uint64_t, uint32_t);
using halDevmmGetHeapListByTypeFunc = int32_t (*)(void *, void *, void *);
using halDevmmVirtSetHeapIdleFunc = int32_t (*)(void *, void *);
using halDevmmVirtDestroyHeapFunc = int32_t (*)(void *, void *, bool);
using halDevmmVirtGetHeapMgmtFunc = void *(*)(void);
using halDevmmIoctlFreePagesFunc = int32_t (*)(uint64_t);
using halDevmmVaToHeapIdxFunc = uint32_t (*)(const void *, uint64_t);
using halDevmmVirtGetHeapFromQueueFunc = void *(*)(void *, uint32_t, size_t);
using halDevmmVirtNormalHeapUpdateInfoFunc = void (*)(void *, void *, void *, void *, uint64_t);
using halDevmmVaToHeapFunc = void *(*)(uint64_t);

using halSqTaskSendFunc = int (*)(uint32_t, halTaskSendInfo *);
using halCqReportRecvFunc = int (*)(uint32_t, halReportRecvInfo *);
using halSqCqAllocateFunc = int (*)(uint32_t, halSqCqInputInfo *, halSqCqOutputInfo *);
using halSqCqFreeFunc = int (*)(uint32_t, halSqCqFreeInfo *);
using halResourceIdAllocFunc = int (*)(uint32_t, struct halResourceIdInputInfo *, struct halResourceIdOutputInfo *);
using halResourceIdFreeFunc = int (*)(uint32_t, struct halResourceIdInputInfo *);
using halGetSsidFunc = int (*)(uint32_t, uint32_t *);
using halResourceConfigFunc = int (*)(uint32_t, struct halResourceIdInputInfo *, struct halResourceConfigInfo *);
using halSqCqQueryFunc = int (*)(uint32_t devId, struct halSqCqQueryInfo *info);
using halHostRegisterFunc = int (*)(void *, uint64_t, uint32_t, uint32_t, void **);
using halHostUnregisterFunc = int (*)(void *, uint32_t);
using drvNotifyIdAddrOffsetFunc = int (*)(uint32_t, struct drvNotifyInfo *);

class DlHalApi {
public:
    static Result LoadLibrary();
    static void CleanupLibrary();

    static inline void HalSvmModuleAllocedSizeInc(void *type, uint32_t devid, uint32_t moduleId, uint64_t size)
    {
        return pSvmModuleAllocedSizeInc(type, devid, moduleId, size);
    }

    static inline uint64_t HalDevmmVirtAllocMemFromBase(void *mgmt, size_t size, uint32_t advise, uint64_t allocPtr)
    {
        return pDevmmVirtAllocMemFromBase(mgmt, size, advise, allocPtr);
    }

    static inline Result HalDevmmIoctlEnableHeap(uint32_t heapIdx, uint32_t heapType, uint32_t subType,
                                                 uint64_t heapSize, uint32_t heapListType)
    {
        return pDevmmIoctlEnableHeap(heapIdx, heapType, subType, heapSize, heapListType);
    }

    static inline Result HalDevmmGetHeapListByType(void *mgmt, void *heapType, void *heapList)
    {
        return pDevmmGetHeapListByType(mgmt, heapType, heapList);
    }

    static inline Result HalDevmmVirtSetHeapIdle(void *mgmt, void *heap)
    {
        return pDevmmVirtSetHeapIdle(mgmt, heap);
    }

    static inline Result HalDevmmVirtDestroyHeap(void *mgmt, void *heap, bool needDec)
    {
        return pDevmmVirtDestroyHeap(mgmt, heap, needDec);
    }

    static inline void *HalDevmmVirtGetHeapMgmt(void)
    {
        return pDevmmVirtGetHeapMgmt();
    }

    static inline Result HalDevmmIoctlFreePages(uint64_t ptr)
    {
        return pDevmmIoctlFreePages(ptr);
    }

    static inline uint32_t HalDevmmVaToHeapIdx(void *mgmt, uint64_t va)
    {
        return pDevmmVaToHeapIdx(mgmt, va);
    }

    static inline void *HalDevmmVirtGetHeapFromQueue(void *mgmt, uint32_t heapIdx, size_t heapSize)
    {
        return pDevmmVirtGetHeapFromQueue(mgmt, heapIdx, heapSize);
    }

    static inline void HalDevmmVirtNormalHeapUpdateInfo(void *mgmt, void *heap, void *type, void *ops, uint64_t size)
    {
        return pDevmmVirtNormalHeapUpdateInfo(mgmt, heap, type, ops, size);
    }

    static inline void *HalDevmmVaToHeap(uint64_t ptr)
    {
        return pDevmmVaToHeap(ptr);
    }

    static inline int32_t GetDevmmFd(void)
    {
        return *pHalDevmmFd;
    }

    static inline int HalSqTaskSend(uint32_t devId, struct halTaskSendInfo *info)
    {
        return pHalSqTaskSend(devId, info);
    }

    static inline int HalCqReportRecv(uint32_t devId, struct halReportRecvInfo *info)
    {
        return pHalCqReportRecv(devId, info);
    }

    static inline int HalSqCqAllocate(uint32_t devId, struct halSqCqInputInfo *in, struct halSqCqOutputInfo *out)
    {
        return pHalSqCqAllocate(devId, in, out);
    }

    static inline int HalSqCqFree(uint32_t devId, struct halSqCqFreeInfo *info)
    {
        return pHalSqCqFree(devId, info);
    }

    static inline int HalResourceIdAlloc(uint32_t devId, struct halResourceIdInputInfo *in,
                                         struct halResourceIdOutputInfo *out)
    {
        return pHalResourceIdAlloc(devId, in, out);
    }

    static inline int HalResourceIdFree(uint32_t devId, struct halResourceIdInputInfo *in)
    {
        return pHalResourceIdFree(devId, in);
    }

    static inline int HalGetSsid(uint32_t devId, uint32_t *ssid)
    {
        return pHalGetSsid(devId, ssid);
    }

    static inline int HalResourceConfig(uint32_t devId, struct halResourceIdInputInfo *in,
                                        struct halResourceConfigInfo *para)
    {
        return pHalResourceConfig(devId, in, para);
    }

    static inline int HalSqCqQuery(uint32_t devId, struct halSqCqQueryInfo *info)
    {
        return pHalSqCqQuery(devId, info);
    }

    static inline int HalHostRegister(void *srcPtr, uint64_t size, uint32_t flag, uint32_t devid, void **dstPtr)
    {
        return pHalHostRegister(srcPtr, size, flag, devid, dstPtr);
    }

    static inline int HalHostUnregister(void *srcPtr, uint32_t devid)
    {
        return pHalHostUnregister(srcPtr, devid);
    }

    static inline int DrvNotifyIdAddrOffset(uint32_t deviceId, struct drvNotifyInfo *drvInfo)
    {
        return pDrvNotifyIdAddrOffset(deviceId, drvInfo);
    }

private:
    static std::mutex gMutex;
    static bool gLoaded;
    static void *halHandle;
    static const char *gAscendHalLibName;

    static halSvmModuleAllocedSizeIncFunc pSvmModuleAllocedSizeInc;
    static halDevmmVirtAllocMemFromBaseFunc pDevmmVirtAllocMemFromBase;
    static halDevmmIoctlEnableHeapFunc pDevmmIoctlEnableHeap;
    static halDevmmGetHeapListByTypeFunc pDevmmGetHeapListByType;
    static halDevmmVirtSetHeapIdleFunc pDevmmVirtSetHeapIdle;
    static halDevmmVirtDestroyHeapFunc pDevmmVirtDestroyHeap;
    static halDevmmVirtGetHeapMgmtFunc pDevmmVirtGetHeapMgmt;
    static halDevmmIoctlFreePagesFunc pDevmmIoctlFreePages;
    static halDevmmVaToHeapIdxFunc pDevmmVaToHeapIdx;
    static halDevmmVirtGetHeapFromQueueFunc pDevmmVirtGetHeapFromQueue;
    static halDevmmVirtNormalHeapUpdateInfoFunc pDevmmVirtNormalHeapUpdateInfo;
    static halDevmmVaToHeapFunc pDevmmVaToHeap;
    static int *pHalDevmmFd;

    static halSqTaskSendFunc pHalSqTaskSend;
    static halCqReportRecvFunc pHalCqReportRecv;
    static halSqCqAllocateFunc pHalSqCqAllocate;
    static halSqCqFreeFunc pHalSqCqFree;
    static halResourceIdAllocFunc pHalResourceIdAlloc;
    static halResourceIdFreeFunc pHalResourceIdFree;
    static halGetSsidFunc pHalGetSsid;
    static halResourceConfigFunc pHalResourceConfig;
    static halSqCqQueryFunc pHalSqCqQuery;
    static halHostRegisterFunc pHalHostRegister;
    static halHostUnregisterFunc pHalHostUnregister;
    static drvNotifyIdAddrOffsetFunc pDrvNotifyIdAddrOffset;
};

}
}

#endif // MF_HYBM_CORE_DL_HAL_API_H
