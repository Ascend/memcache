/*
Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#ifndef MEMORYFABRIC_TRACER_H
#define MEMORYFABRIC_TRACER_H

#include <stdint.h>
#include "htracer_def.h"

#ifdef __cplusplus
extern "C" {
#endif
typedef struct HTRACE_INTF_S {
    bool (*IsEnable)();
    void (*DelayBegin)(uint32_t tpId, const char* tpName);
    void (*DelayEnd)(uint32_t tpId, const uint64_t diff, int32_t retCode);
    uint64_t (*GetCurrentTimeNs)();
} HTRACE_INTF;

extern HTRACE_INTF g_traceIntf;

#define TP_DELAY_BEGIN(TP_ID)                             \
    uint64_t tpBegin##TP_ID = 0;                          \
    if (g_traceIntf.IsEnable && g_traceIntf.IsEnable()) { \
        g_traceIntf.DelayBegin(TP_ID, #TP_ID);            \
        tpBegin##TP_ID = g_traceIntf.GetCurrentTimeNs();  \
    }

#define TP_DELAY_END(TP_ID, RET_CODE)                                                                   \
    if (g_traceIntf.IsEnable && g_traceIntf.IsEnable()) {                                               \
        g_traceIntf.DelayEnd(TP_ID, g_traceIntf.GetCurrentTimeNs() - tpBegin##TP_ID, RET_CODE); \
    }

#define TP_TRACE_DELAY_BEGIN(TP_ID, P_U64_TIME_NS)           \
    if (g_traceIntf.IsEnable && g_traceIntf.IsEnable()) {    \
        g_traceIntf.DelayBegin(TP_ID, #TP_ID);               \
        (*(P_U64_TIME_NS)) = g_traceIntf.GetCurrentTimeNs(); \
    }

#define TP_TRACE_DELAY_END(TP_ID, U64_TIME_NS, RET_CODE)                                                 \
    if (g_traceIntf.IsEnable && g_traceIntf.IsEnable()) {                                                \
        g_traceIntf.DelayEnd(TP_ID, (g_traceIntf.GetCurrentTimeNs() - (U64_TIME_NS)), RET_CODE); \
    }

#define TP_TRACE_DELAY(TP_ID, U64_DIFF_TIME_NS, RET_CODE)                  \
    if (g_traceIntf.IsEnable && g_traceIntf.IsEnable()) {                  \
        g_traceIntf.DelayBegin(TP_ID, #TP_ID);                             \
        g_traceIntf.DelayEnd(TP_ID, (U64_DIFF_TIME_NS), RET_CODE); \
    }

#define TP_CURRENT_TIME_NS (g_traceIntf.GetCurrentTimeNs ? g_traceIntf.GetCurrentTimeNs() : 0)

#ifdef __cplusplus
}
#endif

#endif  // MEMORYFABRIC_TRACER_H
