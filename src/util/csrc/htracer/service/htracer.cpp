/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

#include "htracer.h"
#include "htracer_service.h"

namespace ock {
namespace htracer {
bool HtracerManager::mEnable = true;

int32_t HTracerInit(const std::string &dumpDir)
{
    auto &service = HTracerService::GetInstance();
    if (service.StartUp(dumpDir) != RET_OK) {
        return RET_ERR;
    }
    return RET_OK;
}

void HTracerExit()
{
    auto &service = HTracerService::GetInstance();
    service.ShutDown();
}

std::string GetTraceInfo()
{
    auto &service = HTracerService::GetInstance();
    return service.GetTraceInfo();
}

void ClearTraceInfo()
{
    auto &service = HTracerService::GetInstance();
    return service.ClearTraceInfo();
}
}
}