/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

#ifndef HTRACER_H
#define HTRACER_H

#include "htracer_def.h"
#include "common/htracer_monotonic.h"
#include "manager/htracer_manager.h"

namespace ock {
namespace htracer {

const std::string DEFAULT_DUMP_DIR = "/var/log/mxc/memfabric_hybrid";

int32_t HTracerInit(const std::string &dumpDir = DEFAULT_DUMP_DIR);
void HTracerExit();

#define TP_DELAY_BEGIN(TP_ID)                                                   \
    uint64_t tsBegin##TP_ID = ock::utils::Monotonic::TimeNs();                  \
    if (ock::htracer::HtracerManager::IsEnable()) {                             \
        ock::htracer::HtracerManager::DelayBegin(TP_ID, #TP_ID);                \
    }

#define TP_DELAY_END(TP_ID, RET_CODE)                                           \
    if (ock::htracer::HtracerManager::IsEnable()) {                             \
        uint64_t tsEnd##TP_ID = ock::utils::Monotonic::TimeNs();                \
        uint64_t tpDiff##TP_ID = tsEnd##TP_ID - tsBegin##TP_ID;                 \
        ock::htracer::HtracerManager::DelayEnd(TP_ID, tpDiff##TP_ID, RET_CODE); \
    }

#define TP_TRACE_DELAY_BEGIN(TP_ID, P_U64_TIME_NS)           \
    if (ock::htracer::HtracerManager::IsEnable()) {    \
        ock::htracer::HtracerManager::DelayBegin(TP_ID, #TP_ID);               \
        (*(P_U64_TIME_NS)) = ock::utils::Monotonic::TimeNs(); \
    }

#define TP_TRACE_DELAY_END(TP_ID, U64_TIME_NS, RET_CODE)                                                 \
    if (ock::htracer::HtracerManager::IsEnable()) {                                                \
        ock::htracer::HtracerManager::DelayEnd(TP_ID, (ock::utils::Monotonic::TimeNs() - (U64_TIME_NS)), RET_CODE); \
    }

#define TP_TRACE_DELAY(TP_ID, U64_DIFF_TIME_NS, RET_CODE)                  \
    if (ock::htracer::HtracerManager::IsEnable()) {                  \
        ock::htracer::HtracerManager::DelayBegin(TP_ID, #TP_ID);                             \
        ock::htracer::HtracerManager::DelayEnd(TP_ID, (U64_DIFF_TIME_NS), RET_CODE); \
    }

#define TP_CURRENT_TIME_NS ock::utils::Monotonic::TimeNs()
}
}
#endif // HTRACER_H
