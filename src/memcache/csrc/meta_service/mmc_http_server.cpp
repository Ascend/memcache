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

#include <sstream>

#include "nlohmann/json.hpp"

#include "mmc_logger.h"
#include "mmc_meta_metric_manager.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstack-usage="
#pragma GCC diagnostic ignored "-Wfloat-equal"
#include "mmc_http_server.h"
#pragma GCC diagnostic pop

constexpr int HTTP_INIT_WAIT_MILLISECONDS = 100;

namespace ock {
namespace mmc {

MmcHttpServer::~MmcHttpServer()
{
    Stop();
}

void MmcHttpServer::RegisterUrls()
{
    RegisterHealthCheckEndpoint();
    RegisterDataManagementEndpoints();
    RegisterSegmentManagementEndpoints();
    RegisterMetricsEndpoint();
}

void MmcHttpServer::RegisterHealthCheckEndpoint()
{
    server_.Get("/health", [](const httplib::Request &, httplib::Response &res) {
        res.status = httplib::OK_200;
        res.set_content("OK", "text/plain");
    });
}

void MmcHttpServer::RegisterDataManagementEndpoints()
{
    server_.Get("/get_all_keys", [this](const httplib::Request &, httplib::Response &res) {
        if (metaMetaManager_ == nullptr) {
            MMC_LOG_ERROR("metaMetaManager_ is nullptr");
            res.status = httplib::InternalServerError_500;
            res.set_content("Internal server error", "text/plain");
            return;
        }

        std::vector<std::string> keys;
        if (const auto ret = metaMetaManager_->GetAllKeys(keys); ret != MMC_OK) {
            MMC_LOG_ERROR("Failed to get all keys from meta manager, ret=" << ret);
            res.status = httplib::InternalServerError_500;
            res.set_content("Internal server error", "text/plain");
            return;
        }

        std::ostringstream oss;
        for (const auto &key : keys) {
            oss << key << "\n";
        }

        res.status = httplib::OK_200;
        res.set_content(oss.str(), "text/plain");
    });

    server_.Get("/query_key", [this](const httplib::Request &req, httplib::Response &res) {
        const auto key_it = req.params.find("key");
        if (key_it == req.params.end()) {
            res.status = httplib::BadRequest_400;
            res.set_content("Missing 'key' parameter", "text/plain");
            return;
        }
        const std::string &key = key_it->second;

        if (metaMetaManager_ == nullptr) {
            MMC_LOG_ERROR("metaMetaManager_ is nullptr");
            res.status = httplib::InternalServerError_500;
            res.set_content("Internal server error", "text/plain");
            return;
        }

        MemObjQueryInfo queryInfo;
        if (const Result ret = metaMetaManager_->Query(key, queryInfo); ret != MMC_OK) {
            MMC_LOG_ERROR("Failed to query key: " << key << ", ret=" << ret);
            if (ret == MMC_UNMATCHED_KEY) {
                res.status = httplib::NotFound_404;
                res.set_content("Key not found", "text/plain");
            } else {
                res.status = httplib::InternalServerError_500;
                res.set_content("Internal server error", "text/plain");
            }
            return;
        }

        res.status = httplib::OK_200;
        res.set_content(queryInfo.toJson(key).dump(), "application/json");
    });
}

void MmcHttpServer::RegisterSegmentManagementEndpoints()
{
    server_.Get("/get_all_segments", [this](const httplib::Request &, httplib::Response &res) {
        if (metaMetaManager_ == nullptr) {
            MMC_LOG_ERROR("metaMetaManager_ is nullptr");
            res.status = httplib::InternalServerError_500;
            res.set_content("Internal server error", "text/plain");
            return;
        }

        const auto result = metaMetaManager_->GetAllSegmentInfo();

        res.status = httplib::OK_200;
        res.set_content(result.dump(), "application/json");
    });
}

void MmcHttpServer::RegisterMetricsEndpoint()
{
    server_.Get("/metrics", [this](const httplib::Request &, httplib::Response &res) {
        const auto result = MmcMetaMetricManager::GetPrometheusSummary();

        res.status = httplib::OK_200;
        res.set_content(result, "text/plain");
    });

    server_.Get("/metrics/summary", [this](const httplib::Request &, httplib::Response &res) {
        const auto result = MmcMetaMetricManager::GetInstance().GetSummary();

        res.status = httplib::OK_200;
        res.set_content(result, "text/plain");
    });
}

bool MmcHttpServer::Start()
{
    if (running_) {
        return true;
    }

    serverThread_ = std::thread([this]() { server_.listen(host_, port_); });

    std::this_thread::sleep_for(std::chrono::milliseconds(HTTP_INIT_WAIT_MILLISECONDS));
    running_ = true;
    MMC_LOG_INFO("HTTP server started on " << host_ << ":" << port_);

    return true;
}

void MmcHttpServer::Stop()
{
    if (!running_) {
        return;
    }

    server_.stop();
    if (serverThread_.joinable()) {
        serverThread_.join();
    }
    running_ = false;
    MMC_LOG_INFO("HTTP server stopped");
}

} // namespace mmc
} // namespace ock
