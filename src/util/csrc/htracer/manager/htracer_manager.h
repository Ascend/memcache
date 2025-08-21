/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

#ifndef HTRACER_MANAGER_H
#define HTRACER_MANAGER_H

#include <cstdint>
#include <string>
#include <functional>
#include "common/htracer_info.h"

namespace ock {
namespace mf {
constexpr int32_t MAX_SERVICE_NUM = 64;
constexpr int32_t MAX_INNER_ID_NUM = 1024;

class HtracerManager {
public:
    static __always_inline HtracerInfo **GetTracePoints()
    {
        static HtracerInfo **tracePoints = CreateInstance();
        return tracePoints;
    }

    static __always_inline HtracerInfo *GetPointPtr(int32_t tpId)
    {
        auto tracePoints = GetTracePoints();
        if (tracePoints == nullptr) {
            return nullptr;
        }
        uint32_t serviceId = GetServiceId(tpId);
        uint32_t innerId = GetInnerId(tpId);
        if (serviceId > MAX_SERVICE_NUM || innerId > MAX_INNER_ID_NUM) {
            return nullptr;
        }
        return &tracePoints[serviceId][innerId];
    }

    static __always_inline void DelayBegin(int32_t tpId, std::string tpName)
    {
        auto ptr = GetPointPtr(tpId);
        if (ptr == nullptr) {
            return;
        }
        ptr->DelayBegin(tpName);
    }

    static __always_inline void DelayEnd(int32_t tpId, const uint64_t &diff, int32_t retCode)
    {
        auto ptr = GetPointPtr(tpId);
        if (ptr == nullptr) {
            return;
        }
        ptr->DelayEnd(diff, retCode);
    }

    static __always_inline bool IsEnable()
    {
        return mEnable;
    }

private:
    static __always_inline HtracerInfo **CreateInstance()
    {
        auto **instance = new (std::nothrow) HtracerInfo *[MAX_SERVICE_NUM];
        if (instance == nullptr) {
            return nullptr;
        } else {
            for (int i = 0; i < MAX_SERVICE_NUM; ++i) {
                instance[i] = nullptr;
            }
        }

        int ret = 0;
        uint16_t i = 0;
        for (i = 0; i < MAX_SERVICE_NUM; ++i) {
            instance[i] = new (std::nothrow) HtracerInfo[MAX_INNER_ID_NUM];
            if (instance[i] == nullptr) {
                ret = -1;
                break;
            }
        }

        if (ret != 0) {
            for (uint16_t j = 0; j < i; ++j) {
                delete instance[j];
            }
            delete[] instance;
            return nullptr;
        }
        return instance;
    }

    static __always_inline uint32_t GetServiceId(uint32_t tpId)
    {
        return ((tpId >> 16) & 0xFFFF);
    }

    static __always_inline uint32_t GetInnerId(uint32_t tpId)
    {
        return (tpId & 0xFFFF);
    }

private:
    static bool mEnable;
};
}
}
#endif // HTRACER_MANAGER_H
