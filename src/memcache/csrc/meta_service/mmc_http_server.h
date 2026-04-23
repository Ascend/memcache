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

#ifndef MEM_HTTP_SERVER_H
#define MEM_HTTP_SERVER_H

#include <thread>
#include <atomic>

#include "httplib.h"

#include "mmc_rest_api_facade.h"

namespace ock {
namespace mmc {

class MmcHttpServer {
public:
    MmcHttpServer(const std::string &host, const uint16_t port, const MmcRestApiFacadePtr &restApiFacade)
        : host_(host), port_(port), restApiFacade_(restApiFacade)
    {
        RegisterUrls();
    }

    ~MmcHttpServer();

    bool Start();

    void Stop();

    bool IsRunning() const
    {
        return running_.load();
    }

    MmcHttpServer(const MmcHttpServer &) = delete;
    MmcHttpServer &operator=(const MmcHttpServer &) = delete;

private:
    void RegisterUrls();
    void RegisterHealthCheckEndpoint();
    void RegisterDataManagementEndpoints();
    void RegisterSegmentManagementEndpoints();
    void RegisterDrainJobEndpoints();
    void RegisterMetricsEndpoint();

    std::atomic<bool> running_{false};
    std::string host_;
    uint16_t port_;
    MmcRestApiFacadePtr restApiFacade_;
    httplib::Server server_{};
    std::thread serverThread_;
};

} // namespace mmc
} // namespace ock

#endif
