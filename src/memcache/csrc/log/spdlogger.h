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

#ifndef MEMORYFABRIC_SPDLOGGER_FOR_H
#define MEMORYFABRIC_SPDLOGGER_FOR_H

#include <cstdint>
#include <string>
#include <sstream>
#include <chrono>
#include <memory>
#include <spdlog/common.h>
#include <spdlog/spdlog.h>

namespace ock::mmc::log {
enum class LogLevel {
    TRACE    = 0,
    DEBUG    = 1,
    INFO     = 2,
    WARN     = 3,
    ERROR    = 4,
    CRITICAL = 5,
    LOG_LEVEL_MAX,
};
class SpdLogger {
public:
    SpdLogger() = default;

    ~SpdLogger() = default;

    static SpdLogger& GetInstance()
    {
        static SpdLogger instance;
        return instance;
    }

    static SpdLogger& GetAuditInstance()
    {
        static SpdLogger instance;
        return instance;
    }

    int Initialize(const std::string &path, int minLogLevel, int rotationFileSize, int rotationFileCount);
    int SetLogMinLevel(int minLevel);
    void LogMessage(int level, const char *message);
    void AuditLogMessage(const char *message);
    static const char *GetLastErrorMessage();
    void Flush(void);

private:
    static int ValidateParams(int minLogLevel, const std::string& path, int rotationFileSize,
        int rotationFileCount);

    static void BeforeOpenCallback(const std::string& filename);
    static void AfterOpenCallback(const std::string& filename, std::FILE *file_stream);
    static void AfterCloseCallback(const std::string& filename);

    std::mutex mutex_;
    bool started_ = false;
    std::shared_ptr<spdlog::logger> mSPDLogger;
    std::string mFilePath;
    int mRotationFileSize = 0;
    int mRotationFileCount = 0;
    bool mDebugEnabled = false;
    static thread_local std::string gLastErrorMessage;
};
}
#endif // MEMORYFABRIC_SPDLOGGER_FOR_H