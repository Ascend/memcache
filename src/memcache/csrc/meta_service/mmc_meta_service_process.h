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
#ifndef MMC_META_SERVICE_PROCESS_H
#define MMC_META_SERVICE_PROCESS_H

#include "mmc_leader_election.h"
#include "mmc_configuration.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstack-usage="
#pragma GCC diagnostic ignored "-Wfloat-equal"
#include "mmc_http_server.h"
#pragma GCC diagnostic pop

#include "mmc_meta_service.h"

namespace ock {
namespace mmc {

class MmcMetaServiceProcess {
public:
    static MmcMetaServiceProcess &getInstance()
    {
        static MmcMetaServiceProcess meta;
        return meta;
    }
    MmcMetaServiceProcess() = default;
    ~MmcMetaServiceProcess() = default;
    MmcMetaServiceProcess(const MmcMetaServiceProcess &) = delete;
    MmcMetaServiceProcess &operator=(const MmcMetaServiceProcess &) = delete;

    int MainForExecutable();
    int MainForPython();

private:
    static bool CheckIsRunning();
    int LoadConfig();
    static void RegisterSignal();
    static void SignalInterruptHandler(const int signal);
    static int InitLogger(const mmc_meta_service_config_t &options);
    int StartHttpServer();
    void Exit();

    mmc_meta_service_config_t config_{};
    MmcMetaService *metaService_{};
    MmcMetaServiceLeaderElection *leaderElection_{};
    MmcHttpServer *httpServer_{};
};

} // namespace mmc
} // namespace ock

#endif