/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2026. All rights reserved.
 */

#include <condition_variable>
#include <csignal>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <mutex>
#include <unistd.h>

#include "mmc_configuration.h"
#include "mmc_meta_service_default.h"

using namespace ock::mmc;

static std::mutex g_exitMtx;
static std::condition_variable g_exitCv;
static bool g_processExit = false;
static ConfigurationPtr g_config;

constexpr int ARGS_NUM = 2;

bool CheckIsRunning()
{
    const std::string filePath = "/tmp/mmc_meta_service";
    const std::string fileName = filePath + ".lock";
    const int fd = open(fileName.c_str(), O_WRONLY | O_CREAT, 0600);
    if (fd < 0) {
        std::cerr << "Open file " << fileName.c_str() << " failed, error message is " << strerror(errno) << "."
                  << std::endl;
        return true;
    }
    flock lock{};
    lock.l_type = F_WRLCK;
    lock.l_start = 0;
    lock.l_whence = SEEK_SET;
    lock.l_len = 0;
    const auto ret = fcntl(fd, F_SETLK, &lock);
    if (ret < 0) {
        std::cerr << "Fail to start mmc_meta_service, process lock file is locked." << std::endl;
        close(fd);
        return true;
    }
    return false;
}

void SignalInterruptHandler(int signal)
{
    std::cout << "Received exit signal[" << signal << "]" << std::endl;

    {
        std::unique_lock<std::mutex> lock(g_exitMtx);
        g_processExit = true;
    }
    g_exitCv.notify_all();
}

void RegisterSignal()
{
    sighandler_t oldIntHandler = signal(SIGINT, SignalInterruptHandler);
    if (oldIntHandler == SIG_ERR) {
        std::cerr << "Register SIGINT handler failed" << std::endl;
    }

    sighandler_t oldTermHandler = signal(SIGTERM, SignalInterruptHandler);
    if (oldTermHandler == SIG_ERR) {
        std::cerr << "Register SIGTERM handler failed" << std::endl;
    }
}

mmc_meta_service_config_t GetMetaServiceConfig() {
    mmc_meta_service_config_t config;
    const auto discoveryURL = g_config->GetString(
        ConfConstant::OCK_MMC_META_SERVICE_DISCOVERY_URL.first,
             ConfConstant::OCK_MMC_META_SERVICE_DISCOVERY_URL.second);
    strcpy(config.discoveryURL, discoveryURL.c_str());
    config.worldSize = g_config->GetInt(
        ConfConstant::OCK_MMC_META_SERVICE_WORLD_SIZE.first,
        ConfConstant::OCK_MMC_META_SERVICE_WORLD_SIZE.second);
    return config;
}

int LoadConfig(const std::string& confPath) {
    g_config = Configuration::GetInstance(confPath);
    if (g_config == nullptr) {
        std::cerr << "Configuration initialize failed." << std::endl;
        return -1;
    }
    const std::vector<std::string> validationError = g_config->ValidateDaemonConf();
    if (!validationError.empty()) {
        std::cerr << "Wrong configuration in file <" << confPath << ">, because of following mistakes:" << std::endl;
        for (auto &item : validationError) {
            std::cout << item << std::endl;
        }
        return -1;
    }
    return 0;
}

int main(int argc, char* argv[])
{
    if (argc != ARGS_NUM || argv == nullptr) {
        std::cerr << "Error, invalid parameters." << std::endl;
        return -1;
    }
    if (CheckIsRunning()) {
        std::cerr << "Error, meta service is already running." << std::endl;
        return -1;
    }
    if (LoadConfig(std::string(argv[1])) != 0) {
        std::cerr << "Error, failed to load config." << std::endl;
        return -1;
    }
    RegisterSignal();

    MmcMetaService *serviceDefault = new (std::nothrow)MmcMetaServiceDefault("meta_service");
    if (serviceDefault == nullptr || serviceDefault->Start(GetMetaServiceConfig()) != MMC_OK) {
        std::cerr << "Error, failed to start MmcMetaService." << std::endl;
        return -1;
    }

    std::unique_lock<std::mutex> lock(g_exitMtx);
    g_exitCv.wait(lock, []() { return g_processExit; });

    serviceDefault->Stop();
    return 0;
}
