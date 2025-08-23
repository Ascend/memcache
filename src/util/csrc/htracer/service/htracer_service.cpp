/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

#include <functional>
#include <fstream>
#include <utility>
#include <unistd.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <iostream>
#include <sstream>
#include "htracer.h"
#include "htracer_monotonic.h"
#include "htracer_utils.h"
#include "htracer_manager.h"
#include "htracer_service.h"

namespace ock {
namespace mf {
constexpr int DUMP_PERIOD = 10;
constexpr size_t MAX_DUMP_SIZE = 10 * 1024 * 1024;

int32_t HTracerService::StartUp(const std::string &dumpDir)
{
    auto tracePoints = HtracerManager::GetTracePoints();
    if (tracePoints == nullptr) {
        return -1;
    }

    if (PrepareDumpFile(dumpDir) != 0) {
        return -1;
    }

    StartDump();
    return 0;
}

void HTracerService::ShutDown()
{
    {
        std::unique_lock<std::mutex> lock(mDumpLock);
        mIsRunning = false;
        mDumpCond.notify_all();
    }
    mDumpThread.join();
}

void HTracerService::OverrideWrite(std::stringstream &ss)
{
    std::ifstream infile(dumpFilePath);
    if (!infile) {
        return;
    }
    infile.seekg(0, std::ios_base::beg);
    std::stringstream outInfo;
    std::string line;
    if (writePos > 0) {
        auto bufferLen = writePos + 1;
        std::vector<char> readBuffer(bufferLen);
        infile.read(readBuffer.data(), bufferLen);
        outInfo.write(readBuffer.data(), static_cast<int64_t>(readBuffer.size()));
    }

    int32_t lineCount = 0;
    while (std::getline(ss, line)) {
        outInfo << line << std::endl;
        lineCount++;
    }

    writePos = outInfo.tellp();
    int32_t jumpCount = 0;
    while (std::getline(infile, line)) {
        if (++jumpCount < lineCount) {
            continue;
        }
        if (line == headline) {
            outInfo << headline << std::endl;
            break;
        }
    }
    outInfo << infile.rdbuf();
    infile.close();

    std::ofstream outfile;
    outfile.open(dumpFilePath, std::ios::out | std::ios::trunc);
    if (!outfile.is_open()) {
        return;
    }
    outfile << outInfo.str();
    outfile.flush();
    outfile.close();
}

void HTracerService::WriteTraceInfo(std::stringstream &ss)
{
    size_t filesize = 0;
    struct stat statbuf {};
    int ret = stat(dumpFilePath.c_str(), &statbuf);
    if (ret != 0) {
        if (errno != ENOENT) {
            return;
        }
        filesize = 0;
    } else {
        filesize = static_cast<size_t>(statbuf.st_size);
    }
    if (filesize + static_cast<size_t>(ss.tellp()) > MAX_DUMP_SIZE) {
        if (static_cast<size_t>(writePos + ss.tellp()) > MAX_DUMP_SIZE) {
            writePos = 0;
        }
        return OverrideWrite(ss);
    }
    auto mode = std::ios::out | std::ios::app;
    std::ofstream dump;
    dump.open(dumpFilePath, mode);
    if (!dump.is_open()) {
        return;
    }
    dump << ss.str();
    writePos = dump.tellp();
    dump.flush();
    dump.close();
}

void HTracerService::GetString(HtracerInfo& traceInfo, bool needTotal, std::stringstream& ss, int& traceCount)
{
    if (!traceInfo.Valid()) {
        return;
    }

    if (headline.size() == 0) {
        CreateHeadLine();
        ss << headline << std::endl;
    }

    std::string currentTime = HTracerUtils::CurrentTime();
    if (needTotal) {
        ss << currentTime << traceInfo.ToTotalString() << std::endl;
    } else {
        ss << currentTime << traceInfo.ToPeriodString() << std::endl;
    }
    traceCount++;
}

int HTracerService::GenerateTraceStream(std::stringstream& ss, bool needTotal)
{
    auto tracePoints = HtracerManager::GetTracePoints();
    if (tracePoints == nullptr) {
        return 0;
    }
    int traceCount = 0;
    for (int i = 0; i < MAX_SERVICE_NUM; ++i) {
        for (int j = 0; j < MAX_INNER_ID_NUM; ++j) {
            auto& traceInfo = tracePoints[i][j];
            GetString(traceInfo, needTotal, ss, traceCount);
        }
    }
    return traceCount;
}

void HTracerService::DumpTraceInfos()
{
    std::stringstream ss;
    int traceCount = GenerateTraceStream(ss);
    if (traceCount <= 0) {
        return;
    }
    WriteTraceInfo(ss);
}

void HTracerService::DumpTraceInfoPeriod()
{
    std::unique_lock<std::mutex> lock(mDumpLock);
    while (mIsRunning) {
        mDumpCond.wait_for(lock, std::chrono::seconds(DUMP_PERIOD));
        DumpTraceInfos();
    }
}

int HTracerService::PrepareDumpFile(const std::string &dumpDir)
{
    if (dumpDir.empty()) {
        return -1;
    }
    std::string dumpFileDir = dumpDir;
    if (dumpFileDir.back() != '/') {
        dumpFileDir += "/";
    }
    int32_t ret = HTracerUtils::CreateDirectory(dumpFileDir);
    if (ret != 0) {
        return -1;
    }
    dumpFilePath = dumpFileDir + "htrace_" + std::to_string(getpid()) + ".dat";

    int fd = open(dumpFilePath.c_str(), O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP);
    if (fd < 0) {
        return -1;
    }
    close(fd);
    return 0;
}

void HTracerService::CreateHeadLine()
{
    std::stringstream ss;
    ss << HTracerUtils::HeaderString();
    headline = ss.str();
}

void HTracerService::StartDump()
{
    std::unique_lock<std::mutex> lock(mDumpLock);
    if (mIsRunning) {
        return;
    }
    mIsRunning = true;
    mDumpThread = std::thread(&HTracerService::DumpTraceInfoPeriod, this);
    pthread_setname_np(mDumpThread.native_handle(), "htracerDump");
}

std::string HTracerService::GetTraceInfo()
{
    std::stringstream ss;
    GenerateTraceStream(ss, true);
    return ss.str();
}

void HTracerService::ClearTraceInfo()
{
    auto tracePoints = HtracerManager::GetTracePoints();
    if (tracePoints == nullptr) {
        return;
    }
    for (int i = 0; i < MAX_SERVICE_NUM; ++i) {
        for (int j = 0; j < MAX_INNER_ID_NUM; ++j) {
            auto &traceInfo = tracePoints[i][j];
            if (traceInfo.Valid()) {
                traceInfo.Reset();
            }
        }
    }
}
}
}