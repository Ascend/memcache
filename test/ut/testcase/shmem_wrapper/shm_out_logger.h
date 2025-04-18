/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2026. All rights reserved.
 */
#ifndef SHMEM_SHM_OUT_LOGGER_H
#define SHMEM_SHM_OUT_LOGGER_H

#include <ctime>
#include <climits>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <iomanip>
#include <mutex>
#include <unistd.h>
#include <sstream>
#include <sys/time.h>
#include <sys/syscall.h>

namespace shm {
using ExternalLog = void (*)(int, const char *);

enum LogLevel : int {
    DEBUG_LEVEL = 0,
    INFO_LEVEL,
    WARN_LEVEL,
    ERROR_LEVEL,
    BUTT_LEVEL /* no use */
};

class ShmOutLogger {
public:
    static ShmOutLogger &Instance()
    {
        static ShmOutLogger gLogger;
        return gLogger;
    }

    inline void SetLogLevel(LogLevel level)
    {
        mLogLevel = level;
    }

    inline void SetExternLogFunc(ExternalLog func, bool forceUpdate = false)
    {
        if (mLogFunc == nullptr || forceUpdate) {
            mLogFunc = func;
        }
    }

    inline void Log(int level, const std::ostringstream &oss)
    {
        if (mLogFunc != nullptr) {
            mLogFunc(level, oss.str().c_str());
            return;
        }

        if (level < mLogLevel) {
            return;
        }

        struct timeval tv {};
        char strTime[24];

        gettimeofday(&tv, nullptr);
        time_t timeStamp = tv.tv_sec;
        struct tm localTime {};
        if (strftime(strTime, sizeof strTime, "%Y-%m-%d %H:%M:%S.", localtime_r(&timeStamp, &localTime)) != 0) {
            std::cout << strTime << std::setw(6) << std::setfill('0') << tv.tv_usec << " " << LogLevelDesc(level) << " "
                      << syscall(SYS_gettid) << " " << oss.str() << std::endl;
        } else {
            std::cout << " Invalid time " << LogLevelDesc(level) << " " << syscall(SYS_gettid) << " " << oss.str()
                      << std::endl;
        }
    }

    ShmOutLogger(const ShmOutLogger &) = delete;
    ShmOutLogger(ShmOutLogger &&) = delete;

    ~ShmOutLogger()
    {
        mLogFunc = nullptr;
    }

private:
    ShmOutLogger() = default;

    inline const std::string &LogLevelDesc(int level)
    {
        static std::string invalid = "invalid";
        if (level < DEBUG_LEVEL || level >= BUTT_LEVEL) {
            return invalid;
        }
        return mLogLevelDesc[level];
    }

private:
    const std::string mLogLevelDesc[BUTT_LEVEL] = {"debug", "info", "warn", "error"};

    LogLevel mLogLevel = INFO_LEVEL;
    ExternalLog mLogFunc = nullptr;
};
}  // namespace shm

#ifndef SHM_LOG_FILENAME_SHORT
#define SHM_LOG_FILENAME_SHORT (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#endif
#define SHM_OUT_LOG(LEVEL, ARGS)                                                       \
    do {                                                                               \
        std::ostringstream oss;                                                        \
        oss << "[SHMEM " << SHM_LOG_FILENAME_SHORT << ":" << __LINE__ << "] " << ARGS; \
        shm::ShmOutLogger::Instance().Log(LEVEL, oss);                                 \
    } while (0)

#define LOG_DEBUG(ARGS) SHM_OUT_LOG(shm::DEBUG_LEVEL, ARGS)
#define LOG_INFO(ARGS) SHM_OUT_LOG(shm::INFO_LEVEL, ARGS)
#define LOG_WARN(ARGS) SHM_OUT_LOG(shm::WARN_LEVEL, ARGS)
#define LOG_ERROR(ARGS) SHM_OUT_LOG(shm::ERROR_LEVEL, ARGS)

#define ASSERT_RETURN(ARGS, RET)              \
    do {                                         \
        if (__builtin_expect(!(ARGS), 0) != 0) { \
            LOG_ERROR("Assert " << #ARGS);       \
            return RET;                          \
        }                                        \
    } while (0)

#define ASSERT_RET_VOID(ARGS)                 \
    do {                                         \
        if (__builtin_expect(!(ARGS), 0) != 0) { \
            LOG_ERROR("Assert " << #ARGS);       \
            return;                              \
        }                                        \
    } while (0)

#define ASSERT_RETURN_NOLOG(ARGS, RET)        \
    do {                                         \
        if (__builtin_expect(!(ARGS), 0) != 0) { \
            return RET;                          \
        }                                        \
    } while (0)

#define ASSERT(ARGS)                          \
    do {                                         \
        if (__builtin_expect(!(ARGS), 0) != 0) { \
            LOG_ERROR("Assert " << #ARGS);       \
        }                                        \
    } while (0)


#endif  //SHMEM_SHM_OUT_LOGGER_H
