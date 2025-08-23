/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

#include "htracer.h"
#include "htracer_service.h"
#include "manager/htracer_manager.h"
#include "common/htracer_monotonic.h"
#include "common/htracer_utils.h"

#ifdef __cplusplus
extern "C" {
#endif

static bool trace_enable();
static volatile bool g_traceInit = false;
HTRACE_INTF g_traceIntf = {trace_enable, nullptr, nullptr, nullptr};

#ifdef ENABLE_HTRACE
static bool g_enableHtrace = true;
#else
static bool g_enableHtrace = false;
#endif

static bool trace_enable()
{
    return g_traceInit && g_enableHtrace;
}

static void delay_begin(uint32_t tpId, const char* tpName)
{
    ock::mf::HtracerManager::DelayBegin(tpId, tpName);
}

static void delay_end(uint32_t tpId, uint64_t ns, int32_t retCode)
{
    ock::mf::HtracerManager::DelayEnd(tpId, ns, retCode);
}

static uint64_t time_ns()
{
    return ock::utils::Monotonic::TimeNs();
}

#ifdef __cplusplus
}
#endif

namespace ock {
namespace mf {

static std::mutex g_mutex;

int32_t HTracerInit(const std::string &dumpDir)
{
    std::lock_guard<std::mutex> lk{g_mutex};
    if (!g_enableHtrace) {
        return 0;
    }

    if (g_traceInit) {
        return 0;
    }

    auto &service = HTracerService::GetInstance();
    if (service.StartUp(dumpDir) != 0) {
        return -1;
    }

    g_traceIntf.DelayBegin = delay_begin;
    g_traceIntf.DelayEnd = delay_end;
    g_traceIntf.GetCurrentTimeNs = time_ns;
    g_traceInit = true;
    return 0;
}

void HTracerExit()
{
    std::lock_guard<std::mutex> lk{g_mutex};
    if (!g_enableHtrace) {
        return;
    }

    if (!g_traceInit) {
        return;
    }
    auto &service = HTracerService::GetInstance();
    service.ShutDown();
    g_traceInit = false;
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
}  // namespace mf
}  // namespace ock