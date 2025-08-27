/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

#ifndef HTRACE_SERVICE_H
#define HTRACE_SERVICE_H

#include <cstdint>
#include <memory>
#include <thread>
#include <condition_variable>
#include <mutex>
#include "common/htracer_info.h"

namespace ock {
namespace mf {
class HTracerService {
public:
    static HTracerService &GetInstance()
    {
        static HTracerService sevice;
        return sevice;
    }

    ~HTracerService()
    {
        if (mIsRunning) {
            ShutDown();
        }
    }

    int32_t StartUp(const std::string &dumpDir);
    void ShutDown();

    HTracerService(const HTracerService &) = delete;
    HTracerService& operator=(const HTracerService&) = delete;
    HTracerService(HTracerService &&) = delete;
    HTracerService& operator=(HTracerService &&) = delete;

    std::string GetTraceInfo();
    void ClearTraceInfo();

private:
    int GenerateTraceStream(std::stringstream &ss, bool needTotal = false);
    void DumpTraceInfos();
    void DumpTraceInfoPeriod();
    void WriteTraceInfo(std::stringstream &ss);
    void OverrideWrite(std::stringstream &ss);
    int PrepareDumpFile(const std::string &dumpDir);
    void CreateHeadLine();
    void StartDump();
    void GetString(HtracerInfo& traceInfo, bool needTotal, std::stringstream& ss, int& traceCount);

    HTracerService() = default;

    off_t writePos {0};
    std::string dumpFilePath;
    std::string headline;
    std::thread mDumpThread;
    std::condition_variable mDumpCond;
    std::mutex mDumpLock;
    bool mIsRunning {false};
};
}
}
#endif // HTRACE_SERVICE_H
