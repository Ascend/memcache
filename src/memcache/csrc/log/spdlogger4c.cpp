/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

#include "spdlogger.h"
#include "spdlogger4c.h"
namespace ock {
namespace mmc {
int SPDLOG_Init(const char* path, int minLogLevel, int rotationFileSize, int rotationFileCount)
{
    return ock::mmc::log::SpdLogger::GetInstance().Initialize(path, minLogLevel, rotationFileSize,
                                                              rotationFileCount);
}

int SPDLOG_AuditInit(const char* path, int rotationFileSize, int rotationFileCount)
{
    const int minLogLevel = 3;
    return ock::mmc::log::SpdLogger::GetAuditInstance().Initialize(path, minLogLevel, rotationFileSize,
                                                                   rotationFileCount);
}

void SPDLOG_LogMessage(int32_t level, const char* msg)
{
    ock::mmc::log::SpdLogger::GetInstance().LogMessage(level, msg);
}

void SPDLOG_AuditLogMessage(const char* msg)
{
    ock::mmc::log::SpdLogger::GetAuditInstance().AuditLogMessage(msg);
}

const char* SPDLOG_GetLastErrorMessage()
{
    return ock::mmc::log::SpdLogger::GetLastErrorMessage();
}

int SPDLOG_ResetLogLevel(int logLevel)
{
    return ock::mmc::log::SpdLogger::GetInstance().SetLogMinLevel(logLevel);
}
}
}