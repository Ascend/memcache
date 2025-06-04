/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2026. All rights reserved.
 */
#ifndef MEM_FABRIC_HYBRID_MMC_DEFINE_H
#define MEM_FABRIC_HYBRID_MMC_DEFINE_H

namespace ock {
namespace mmc {
// macro for gcc optimization for prediction of if/else
#ifndef LIKELY
#define LIKELY(x) (__builtin_expect(!!(x), 1) != 0)
#endif

#ifndef UNLIKELY
#define UNLIKELY(x) (__builtin_expect(!!(x), 0) != 0)
#endif

#define MMC_LOG_AND_SET_LAST_ERROR(msg)  \
    do {                                \
        std::stringstream tmpStr;       \
        tmpStr << msg;                  \
        MOLastError::Set(tmpStr.str()); \
        MMC_LOG_ERROR(tmpStr.str());     \
    } while (0)

#define MMC_SET_LAST_ERROR(msg)          \
    do {                                \
        std::stringstream tmpStr;       \
        tmpStr << msg;                  \
        MOLastError::Set(tmpStr.str()); \
    } while (0)

#define MMC_COUT_AND_SET_LAST_ERROR(msg) \
    do {                                \
        std::stringstream tmpStr;       \
        tmpStr << msg;                  \
        MOLastError::Set(tmpStr.str()); \
        std::cout << msg << std::endl;  \
    } while (0)

// if expression is false, print error
#define MMC_PARAM_VALIDATE(expression, msg, returnValue) \
    do {                                                \
        if (!(expression)) {                             \
            MMC_SET_LAST_ERROR(msg);                     \
            MMC_LOG_ERROR(msg);                          \
            return returnValue;                         \
        }                                               \
    } while (0)

#define MMC_API __attribute__((visibility("default")))

}  // namespace smem
}  // namespace ock

#endif  //MEM_FABRIC_HYBRID_MMC_DEFINE_H
