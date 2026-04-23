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

#include <chrono>
#include <ctime>
#include <sstream>

#include "nlohmann/json.hpp"

#include "mmc_logger.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstack-usage="
#pragma GCC diagnostic ignored "-Wfloat-equal"
#include "mmc_http_server.h"
#pragma GCC diagnostic pop

constexpr int HTTP_INIT_WAIT_MILLISECONDS = 100;
constexpr char kContentTypeJsonUtf8[] = "application/json; charset=utf-8";
constexpr char kContentTypeTextUtf8[] = "text/plain; charset=utf-8";
constexpr char kContentTypePrometheus[] = "text/plain; version=0.0.4";
constexpr char kMetadataUpdatedText[] = "metadata updated";
constexpr char kMetadataDeletedText[] = "metadata deleted";
constexpr char kErrorNotSupported[] = "Not supported";
constexpr char kErrorInternalServer[] = "Internal server error";
constexpr char kErrorKeyNotFound[] = "Key not found";
constexpr char kErrorSegmentNotFound[] = "Segment not found";
constexpr char kErrorMetadataNotFound[] = "Metadata key not found";

namespace {

uint64_t CurrentTimestamp()
{
    return static_cast<uint64_t>(std::time(nullptr));
}

std::string BuildMissingParameterMessage(const std::string &paramName)
{
    return "Missing '" + paramName + "' parameter";
}

void ReplyTextOk(httplib::Response &res, const std::string &body, const char *contentType = kContentTypeTextUtf8)
{
    res.status = httplib::OK_200;
    res.set_content(body, contentType);
}

void ReplyJsonOk(httplib::Response &res, const nlohmann::json &body)
{
    res.status = httplib::OK_200;
    res.set_content(body.dump(), kContentTypeJsonUtf8);
}

void ReplyJsonError200(httplib::Response &res, const std::string &message)
{
    nlohmann::json errorBody;
    errorBody["success"] = false;
    errorBody["error_message"] = message;
    errorBody["timestamp"] = CurrentTimestamp();
    ReplyJsonOk(res, errorBody);
}

bool GetRequiredParam(const httplib::Request &req, const std::string &paramName, std::string &value,
                      httplib::Response &res)
{
    const auto paramIt = req.params.find(paramName);
    if (paramIt == req.params.end()) {
        ReplyJsonError200(res, BuildMissingParameterMessage(paramName));
        return false;
    }
    value = paramIt->second;
    return true;
}

void SplitCommaSeparated(const std::string &source, std::vector<std::string> &items)
{
    items.clear();
    size_t start = 0;
    while (start <= source.size()) {
        const size_t end = source.find(',', start);
        if (end == std::string::npos) {
            std::string item = source.substr(start);
            if (!item.empty()) {
                items.push_back(std::move(item));
            }
            return;
        }
        std::string item = source.substr(start, end - start);
        if (!item.empty()) {
            items.push_back(std::move(item));
        }
        start = end + 1;
    }
}

} // namespace

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
    RegisterDrainJobEndpoints();
    RegisterMetricsEndpoint();
}

void MmcHttpServer::RegisterHealthCheckEndpoint()
{
    server_.Get("/health", [this](const httplib::Request &, httplib::Response &res) {
        if (restApiFacade_ == nullptr) {
            ReplyJsonError200(res, kErrorInternalServer);
            return;
        }
        ReplyJsonOk(res, restApiFacade_->BuildHealth(IsRunning()));
    });

    server_.Get("/role", [this](const httplib::Request &, httplib::Response &res) {
        if (restApiFacade_ == nullptr) {
            ReplyJsonError200(res, kErrorInternalServer);
            return;
        }
        ReplyTextOk(res, restApiFacade_->GetRole());
    });

    server_.Get("/ha_status", [this](const httplib::Request &, httplib::Response &res) {
        if (restApiFacade_ == nullptr) {
            ReplyJsonError200(res, kErrorInternalServer);
            return;
        }
        ReplyTextOk(res, restApiFacade_->GetHaStatus());
    });

    server_.Get("/leader", [this](const httplib::Request &, httplib::Response &res) {
        if (restApiFacade_ == nullptr) {
            ReplyJsonError200(res, kErrorInternalServer);
            return;
        }
        ReplyJsonOk(res, restApiFacade_->BuildLeader());
    });
}

void MmcHttpServer::RegisterDataManagementEndpoints()
{
    server_.Get("/metadata", [this](const httplib::Request &req, httplib::Response &res) {
        if (restApiFacade_ == nullptr) {
            ReplyJsonError200(res, kErrorInternalServer);
            return;
        }

        std::string key;
        if (!GetRequiredParam(req, "key", key, res)) {
            return;
        }

        std::string value;
        const Result ret = restApiFacade_->GetMetadata(key, value);
        if (ret != MMC_OK) {
            const std::string message =
                ret == ock::smem::StoreErrorCode::NOT_EXIST ? kErrorMetadataNotFound : kErrorInternalServer;
            ReplyJsonError200(res, message);
            return;
        }
        ReplyTextOk(res, value);
    });

    server_.Put("/metadata", [this](const httplib::Request &req, httplib::Response &res) {
        if (restApiFacade_ == nullptr) {
            ReplyJsonError200(res, kErrorInternalServer);
            return;
        }

        std::string key;
        if (!GetRequiredParam(req, "key", key, res)) {
            return;
        }

        const Result ret = restApiFacade_->PutMetadata(key, req.body);
        if (ret != MMC_OK) {
            ReplyJsonError200(res, kErrorInternalServer);
            return;
        }
        ReplyTextOk(res, kMetadataUpdatedText);
    });

    server_.Delete("/metadata", [this](const httplib::Request &req, httplib::Response &res) {
        if (restApiFacade_ == nullptr) {
            ReplyJsonError200(res, kErrorInternalServer);
            return;
        }

        std::string key;
        if (!GetRequiredParam(req, "key", key, res)) {
            return;
        }

        const Result ret = restApiFacade_->DeleteMetadata(key);
        if (ret != MMC_OK) {
            const std::string message =
                ret == ock::smem::StoreErrorCode::NOT_EXIST ? kErrorMetadataNotFound : kErrorInternalServer;
            ReplyJsonError200(res, message);
            return;
        }
        ReplyTextOk(res, kMetadataDeletedText);
    });

    server_.Get("/get_all_keys", [this](const httplib::Request &, httplib::Response &res) {
        if (restApiFacade_ == nullptr) {
            ReplyJsonError200(res, kErrorInternalServer);
            return;
        }

        std::string result;
        const Result ret = restApiFacade_->GetAllKeysText(result);
        if (ret != MMC_OK) {
            ReplyJsonError200(res, kErrorInternalServer);
            return;
        }
        ReplyTextOk(res, result);
    });

    server_.Get("/query_key", [this](const httplib::Request &req, httplib::Response &res) {
        if (restApiFacade_ == nullptr) {
            ReplyJsonError200(res, kErrorInternalServer);
            return;
        }

        std::string key;
        if (!GetRequiredParam(req, "key", key, res)) {
            return;
        }

        nlohmann::json result;
        const Result ret = restApiFacade_->QueryKey(key, result);
        if (ret != MMC_OK) {
            const std::string message = ret == MMC_UNMATCHED_KEY ? kErrorKeyNotFound : kErrorInternalServer;
            ReplyJsonError200(res, message);
            return;
        }
        ReplyJsonOk(res, result);
    });

    server_.Get("/batch_query_keys", [this](const httplib::Request &req, httplib::Response &res) {
        if (restApiFacade_ == nullptr) {
            ReplyJsonError200(res, kErrorInternalServer);
            return;
        }

        std::string keyString;
        if (!GetRequiredParam(req, "keys", keyString, res)) {
            return;
        }

        std::vector<std::string> keys;
        SplitCommaSeparated(keyString, keys);
        nlohmann::json result;
        const Result ret = restApiFacade_->BatchQueryKeys(keys, result);
        if (ret != MMC_OK) {
            ReplyJsonError200(res, kErrorInternalServer);
            return;
        }
        ReplyJsonOk(res, result);
    });
}

void MmcHttpServer::RegisterSegmentManagementEndpoints()
{
    server_.Get("/get_all_segments", [this](const httplib::Request &, httplib::Response &res) {
        if (restApiFacade_ == nullptr) {
            ReplyJsonError200(res, kErrorInternalServer);
            return;
        }

        std::string result;
        const Result ret = restApiFacade_->GetAllSegmentsText(result);
        if (ret != MMC_OK) {
            ReplyJsonError200(res, kErrorInternalServer);
            return;
        }
        ReplyTextOk(res, result);
    });

    server_.Get("/query_segment", [this](const httplib::Request &req, httplib::Response &res) {
        if (restApiFacade_ == nullptr) {
            ReplyJsonError200(res, kErrorInternalServer);
            return;
        }

        std::string segmentId;
        if (!GetRequiredParam(req, "segment", segmentId, res)) {
            return;
        }

        RestSegmentSnapshot segment;
        const Result ret = restApiFacade_->QuerySegment(segmentId, segment);
        if (ret != MMC_OK) {
            const std::string message = ret == MMC_UNMATCHED_KEY ? kErrorSegmentNotFound : kErrorInternalServer;
            ReplyJsonError200(res, message);
            return;
        }
        ReplyJsonOk(res, restApiFacade_->BuildSegment(segment));
    });

    server_.Get("/api/v1/segments/status", [this](const httplib::Request &req, httplib::Response &res) {
        if (restApiFacade_ == nullptr) {
            ReplyJsonError200(res, kErrorInternalServer);
            return;
        }

        std::string segmentId;
        if (!GetRequiredParam(req, "segment", segmentId, res)) {
            return;
        }

        nlohmann::json result;
        const Result ret = restApiFacade_->BuildSegmentStatus(segmentId, result);
        if (ret != MMC_OK) {
            const std::string message = ret == MMC_UNMATCHED_KEY ? kErrorSegmentNotFound : kErrorInternalServer;
            ReplyJsonError200(res, message);
            return;
        }
        ReplyJsonOk(res, result);
    });

    server_.Get("/api/v1/capacity/usage", [this](const httplib::Request &, httplib::Response &res) {
        if (restApiFacade_ == nullptr) {
            ReplyJsonError200(res, kErrorInternalServer);
            return;
        }

        nlohmann::json result;
        const Result ret = restApiFacade_->BuildCapacityUsage(result);
        if (ret != MMC_OK) {
            ReplyJsonError200(res, kErrorInternalServer);
            return;
        }
        ReplyJsonOk(res, result);
    });

    server_.Get("/api/v1/capacity/segment_remaining", [this](const httplib::Request &, httplib::Response &res) {
        if (restApiFacade_ == nullptr) {
            ReplyJsonError200(res, kErrorInternalServer);
            return;
        }

        nlohmann::json result;
        const Result ret = restApiFacade_->BuildSegmentRemaining(result);
        if (ret != MMC_OK) {
            ReplyJsonError200(res, kErrorInternalServer);
            return;
        }
        ReplyJsonOk(res, result);
    });
}

void MmcHttpServer::RegisterDrainJobEndpoints()
{
    auto notSupportedHandler = [](const httplib::Request &, httplib::Response &res) {
        ReplyJsonError200(res, kErrorNotSupported);
    };
    server_.Post("/api/v1/drain_jobs", notSupportedHandler);
    server_.Get("/api/v1/drain_jobs/query", notSupportedHandler);
    server_.Post("/api/v1/drain_jobs/cancel", notSupportedHandler);
}

void MmcHttpServer::RegisterMetricsEndpoint()
{
    server_.Get("/metrics", [this](const httplib::Request &, httplib::Response &res) {
        if (restApiFacade_ == nullptr) {
            ReplyJsonError200(res, kErrorInternalServer);
            return;
        }

        std::string result;
        const Result ret = restApiFacade_->BuildPrometheusMetrics(IsRunning(), result);
        if (ret != MMC_OK) {
            ReplyJsonError200(res, kErrorInternalServer);
            return;
        }
        ReplyTextOk(res, result, kContentTypePrometheus);
    });

    server_.Get("/metrics/summary", [this](const httplib::Request &, httplib::Response &res) {
        if (restApiFacade_ == nullptr) {
            ReplyJsonError200(res, kErrorInternalServer);
            return;
        }

        std::string result;
        const Result ret = restApiFacade_->BuildMetricsSummary(IsRunning(), result);
        if (ret != MMC_OK) {
            ReplyJsonError200(res, kErrorInternalServer);
            return;
        }
        ReplyTextOk(res, result);
    });

    server_.Get("/metrics/ptracer", [this](const httplib::Request &, httplib::Response &res) {
        if (restApiFacade_ == nullptr) {
            ReplyJsonError200(res, kErrorInternalServer);
            return;
        }

        std::string result;
        const Result ret = restApiFacade_->GetPtracerText(result);
        if (ret != MMC_OK) {
            ReplyJsonError200(res, kErrorInternalServer);
            return;
        }
        ReplyTextOk(res, result);
    });

    server_.Get("/api/v1/analysis/alloc_free_latency", [this](const httplib::Request &, httplib::Response &res) {
        if (restApiFacade_ == nullptr) {
            ReplyJsonError200(res, kErrorInternalServer);
            return;
        }

        std::string result;
        const Result ret = restApiFacade_->GetAllocFreeLatencyText(result);
        if (ret != MMC_OK) {
            ReplyJsonError200(res, kErrorInternalServer);
            return;
        }
        ReplyTextOk(res, result);
    });
}

bool MmcHttpServer::Start()
{
    if (running_.load()) {
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
    if (!running_.load()) {
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
