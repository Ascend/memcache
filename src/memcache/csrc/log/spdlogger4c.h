/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */
#ifndef MEMORYFABRIC_SPDLOGGER_FOR_C_H
#define MEMORYFABRIC_SPDLOGGER_FOR_C_H
namespace ock {
namespace mmc {
using AuditEventType = enum {
    START_MF_SERVICE = 0,
    STOP_MF_SERVICE,
    CONNECT_ZOOKEEPER,
    DISCONNECT_ZOOKEEPER,
    RW_ZOOKEEPER,
    CLOSE_ZOOKEEPER,
};

using AuditResourceType = enum {
    BIG_MEMORY = 0,
    META_ZOOKEEPER,
};

/**
 * @brief initialize the normal ulog
 *
 * @param logType          - [IN] type of the ulog, could 0 or 1; 0: stdout, 1: file
 * @param minLogLevel      - [IN] min level of message, 0:trace, 1:debug, 2:info, 3:warn, 4:error, 5:critical
 * @param path             - [IN] full path of ulog file name
 * @param rotationFileSize - [IN] the max file size of a single rotation file
 * @param rotationFileSize - [IN] the max count of total rotated file
 *
 * @return 0 for success, non zero for failure
 */
int SPDLOG_Init(const char *path, int minLogLevel, int rotationFileSize, int rotationFileCount);

int SPDLOG_AuditInit(const char *path, int rotationFileSize, int rotationFileCount);

/**
 * @brief ulog a message into a normal ulog
 *
 * @param logLevel         - [IN] level of the message, 0-5
 * @param prefix           - [IN] format of the message
 *
 * @return 0 for success, non zero for failure
 */
void SPDLOG_LogMessage(int32_t level, const char *msg);

/**
 * @brief ulog a message into a audit ulog
 *
 * @param logLevel         - [IN] level of the message, 2:info, 3:warn, 4:error
 * @param prefix           - [IN] format of the message
 *
 * @return 0 for success, non zero for failure
 */
void SPDLOG_AuditLogMessage(const char *msg);

/**
 * @brief retrieve the last error message
 *
 * @return message
 */
const char *SPDLOG_GetLastErrorMessage();

int SPDLOG_ResetLogLevel(int logLevel);
}
}

#endif // MEMORYFABRIC_SPDLOGGER_FOR_C_H
