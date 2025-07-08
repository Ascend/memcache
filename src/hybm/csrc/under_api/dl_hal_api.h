/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023. All rights reserved.
 */

#ifndef MF_HYBM_CORE_DL_HAL_API_H
#define MF_HYBM_CORE_DL_HAL_API_H

#include "hybm_common_include.h"

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
};

}
}

#endif  // MF_HYBM_CORE_DL_HAL_API_H
