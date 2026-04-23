/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2026. All rights reserved.
 * MemCache_Hybrid is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 */

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <chrono>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include "gtest/gtest.h"
#include "httplib.h"
#include "nlohmann/json.hpp"

#include "mmc_blob_allocator.h"
#include "mmc_meta_common.h"
#include "mmc_http_server.h"
#include "mmc_meta_metric_manager.h"
#include "mmc_meta_service.h"
#include "mmc_msg_client_meta.h"
#include "mmc_ref.h"
#include "mmc_rest_api_facade.h"

using namespace testing;
using namespace std;
using namespace ock::mmc;

constexpr char kHttpMetaKey[] = "http-contract-metadata";
constexpr char kHttpAllocKey[] = "http-contract-key";
constexpr char kHttpLocalHost[] = "127.0.0.1";
constexpr char kHttpTcpUrlPrefix[] = "tcp://127.0.0.1:";
constexpr char kHttpMetadataValue[] = "metadata-value-1";
constexpr char kHttpHbmSegmentName[] = "rank-0-hbm";
constexpr char kHttpDramSegmentName[] = "rank-0-dram";
constexpr uint64_t kHttpSegmentCapacityBytes = 65536;
constexpr uint32_t kHttpRankId = 0;
constexpr int kHttpStartRetryCount = 20;
constexpr uint16_t kHttpTestPortBase = 30000;
constexpr uint16_t kHttpTestPortStride = 20;
constexpr int kHttpWaitRetryCount = 50;
constexpr int kHttpWaitIntervalMs = 20;
constexpr int kHttpLogRotationFileCount = 20;
constexpr int kHttpEvictThresholdHigh = 70;
constexpr int kHttpEvictThresholdLow = 60;
constexpr size_t kBytesPerKilobyte = 1024;
constexpr size_t kKilobytesPerMegabyte = 1024;
constexpr size_t kHttpLogRotationFileSizeMb = 2;
constexpr size_t kHttpLogRotationFileSize =
    kHttpLogRotationFileSizeMb * kKilobytesPerMegabyte * kBytesPerKilobyte;
constexpr pid_t kHttpPidModuloBase = 1000;
constexpr int kHttpCandidatePortAttemptStride = 3;
constexpr int kHttpDefaultSocketProtocol = 0;
constexpr uint16_t kHttpInvalidPort = 0;
constexpr in_port_t kHttpEphemeralPort = 0;
constexpr int kHttpDiscoveryPortOffset = 0;
constexpr int kHttpConfigStorePortOffset = 1;
constexpr int kHttpServerPortOffset = 2;
constexpr uint32_t kHttpInitInfoRankId = 0;
constexpr int kHttpAllocOffset = 0;
constexpr uint64_t kHttpExpectedBlobCount = 1;
constexpr size_t kHttpExpectedBlobCountSize = 1U;
constexpr size_t kHttpExpectedSegmentCount = 2U;
constexpr size_t kHttpFirstItemIndex = 0U;
constexpr size_t kHttpSecondItemIndex = 1U;
constexpr int kHttpSegmentStatusOk = 1;
constexpr uint64_t kHttpZeroUsedBytes = 0U;
constexpr double kHttpNpuUsageRatioExpected = 0.5;
constexpr double kHttpCpuUsageRatioExpected = 0.0;

uint16_t BuildCandidatePort(int attempt, int offset)
{
    return static_cast<uint16_t>(kHttpTestPortBase + (getpid() % kHttpPidModuloBase) * kHttpTestPortStride +
                                 attempt * kHttpCandidatePortAttemptStride + offset);
}

class MmcMetaServiceHttpTest : public testing::Test {
public:
    MmcMetaServiceHttpTest() = default;

    void SetUp() override;
    void TearDown() override;

protected:
    static uint16_t GetFreePort();
    static void CopyUrl(const std::string &url, char *dest);
    static std::shared_ptr<httplib::Response> WaitForResponse(httplib::Client &client, const std::string &path);

    void StartService();
    void MountSegments();
    void PrepareAllocatedKey();
    void StartHttpServer();

protected:
    uint16_t discoveryPort_{kHttpInvalidPort};
    uint16_t configStorePort_{kHttpInvalidPort};
    uint16_t httpPort_{kHttpInvalidPort};
    mmc_meta_service_config_t metaServiceConfig_{};
    MmcMetaServicePtr metaService_;
    MmcMetaMgrProxyPtr metaMgrProxy_;
    MmcMetaManagerPtr metaManager_;
    MmcRestApiFacadePtr restApiFacade_;
    std::unique_ptr<MmcHttpServer> httpServer_;
};

uint16_t MmcMetaServiceHttpTest::GetFreePort()
{
    const int socketFd = ::socket(AF_INET, SOCK_STREAM, kHttpDefaultSocketProtocol);
    if (socketFd < 0) {
        return kHttpInvalidPort;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    addr.sin_port = kHttpEphemeralPort;
    if (::bind(socketFd, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) != 0) {
        ::close(socketFd);
        return kHttpInvalidPort;
    }

    socklen_t addrLen = sizeof(addr);
    if (::getsockname(socketFd, reinterpret_cast<sockaddr *>(&addr), &addrLen) != 0) {
        ::close(socketFd);
        return kHttpInvalidPort;
    }

    ::close(socketFd);
    return ntohs(addr.sin_port);
}

void MmcMetaServiceHttpTest::CopyUrl(const std::string &url, char *dest)
{
    for (size_t i = 0; i < url.size(); ++i) {
        dest[i] = url[i];
    }
    dest[url.size()] = '\0';
}

std::shared_ptr<httplib::Response> MmcMetaServiceHttpTest::WaitForResponse(httplib::Client &client,
                                                                           const std::string &path)
{
    for (int i = 0; i < kHttpWaitRetryCount; ++i) {
        auto response = client.Get(path.c_str());
        if (response && response->status == httplib::OK_200) {
            return std::make_shared<httplib::Response>(*response);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(kHttpWaitIntervalMs));
    }
    return nullptr;
}

void MmcMetaServiceHttpTest::StartService()
{
    for (int attempt = 0; attempt < kHttpStartRetryCount; ++attempt) {
        discoveryPort_ = BuildCandidatePort(attempt, kHttpDiscoveryPortOffset);
        configStorePort_ = BuildCandidatePort(attempt, kHttpConfigStorePortOffset);

        metaService_ = MmcMakeRef<MmcMetaService>("httpMetaService");
        ASSERT_NE(metaService_, nullptr);

        metaServiceConfig_ = {};
        metaServiceConfig_.logLevel = INFO_LEVEL;
        metaServiceConfig_.logRotationFileSize = kHttpLogRotationFileSize;
        metaServiceConfig_.logRotationFileCount = kHttpLogRotationFileCount;
        metaServiceConfig_.evictThresholdHigh = kHttpEvictThresholdHigh;
        metaServiceConfig_.evictThresholdLow = kHttpEvictThresholdLow;
        metaServiceConfig_.haEnable = false;
        metaServiceConfig_.accTlsConfig.tlsEnable = false;
        metaServiceConfig_.ubsIoEnable = false;

        const std::string discoveryUrl = std::string(kHttpTcpUrlPrefix) + std::to_string(discoveryPort_);
        const std::string configStoreUrl = std::string(kHttpTcpUrlPrefix) + std::to_string(configStorePort_);
        CopyUrl(discoveryUrl, metaServiceConfig_.discoveryURL);
        CopyUrl(configStoreUrl, metaServiceConfig_.configStoreURL);

        if (metaService_->Start(metaServiceConfig_) == MMC_OK) {
            metaMgrProxy_ = metaService_->GetMetaMgrProxy();
            ASSERT_NE(metaMgrProxy_, nullptr);
            metaManager_ = metaMgrProxy_->GetMetaManager();
            ASSERT_NE(metaManager_, nullptr);
            return;
        }

        if (metaService_ != nullptr) {
            metaService_->Stop();
        }
        metaService_ = nullptr;
        metaMgrProxy_ = nullptr;
        metaManager_ = nullptr;
    }

    FAIL() << "Failed to start meta service after retries";
}

void MmcMetaServiceHttpTest::MountSegments()
{
    std::vector<MmcLocation> locations;
    locations.emplace_back(kHttpRankId, MEDIA_HBM);
    locations.emplace_back(kHttpRankId, MEDIA_DRAM);

    std::vector<MmcLocalMemlInitInfo> initInfos;
    initInfos.push_back(MmcLocalMemlInitInfo{kHttpInitInfoRankId, kHttpSegmentCapacityBytes});
    initInfos.push_back(MmcLocalMemlInitInfo{kHttpInitInfoRankId, kHttpSegmentCapacityBytes});

    std::map<std::string, MmcMemBlobDesc> blobMap;
    ASSERT_EQ(metaManager_->Mount(locations, initInfos, blobMap), MMC_OK);
}

void MmcMetaServiceHttpTest::PrepareAllocatedKey()
{
    AllocRequest allocRequest(kHttpAllocKey,
                              AllocOptions(SIZE_32K, kHttpExpectedBlobCount, MEDIA_HBM, {kHttpRankId},
                                           kHttpAllocOffset),
                              GenerateOperateId(kHttpRankId));
    AllocResponse allocResponse;
    ASSERT_EQ(metaMgrProxy_->Alloc(allocRequest, allocResponse), MMC_OK);
    ASSERT_EQ(allocResponse.numBlobs_, kHttpExpectedBlobCount);
    ASSERT_EQ(allocResponse.blobs_.size(), kHttpExpectedBlobCountSize);

    const auto &allocatedBlob = allocResponse.blobs_[kHttpFirstItemIndex];
    UpdateRequest updateRequest(MMC_WRITE_OK, kHttpAllocKey, allocatedBlob.rank_, allocatedBlob.mediaType_,
                                allocRequest.operateId_);
    Response updateResponse;
    ASSERT_EQ(metaMgrProxy_->UpdateState(updateRequest, updateResponse), MMC_OK);
    ASSERT_EQ(updateResponse.ret_, MMC_OK);
}

void MmcMetaServiceHttpTest::StartHttpServer()
{
    restApiFacade_ = MmcMakeRef<MmcRestApiFacade>(metaService_.Get(), metaMgrProxy_, nullptr);
    ASSERT_NE(restApiFacade_, nullptr);

    for (int attempt = 0; attempt < kHttpStartRetryCount; ++attempt) {
        httpPort_ = BuildCandidatePort(attempt, kHttpServerPortOffset);

        httpServer_ = std::make_unique<MmcHttpServer>(kHttpLocalHost, httpPort_, restApiFacade_);
        ASSERT_TRUE(httpServer_->Start());

        httplib::Client client(kHttpLocalHost, httpPort_);
        if (WaitForResponse(client, "/health") != nullptr) {
            return;
        }

        httpServer_->Stop();
        httpServer_.reset();
    }

    FAIL() << "Failed to start HTTP server after retries";
}

void MmcMetaServiceHttpTest::SetUp()
{
    StartService();
    MountSegments();
    PrepareAllocatedKey();
    StartHttpServer();
}

void MmcMetaServiceHttpTest::TearDown()
{
    if (httpServer_ != nullptr) {
        httpServer_->Stop();
        httpServer_.reset();
    }
    if (metaService_ != nullptr) {
        metaService_->Stop();
    }
}

TEST_F(MmcMetaServiceHttpTest, RoutesContract)
{
    httplib::Client client(kHttpLocalHost, httpPort_);

    auto healthResponse = WaitForResponse(client, "/health");
    ASSERT_NE(healthResponse, nullptr);
    ASSERT_EQ(healthResponse->status, httplib::OK_200);
    auto healthJson = nlohmann::json::parse(healthResponse->body);
    EXPECT_EQ(healthJson.at("status"), "ok");
    EXPECT_EQ(healthJson.at("role"), "unknown");
    EXPECT_EQ(healthJson.at("ha_state"), "unknown");
    EXPECT_TRUE(healthJson.at("service_ready").get<bool>());

    auto roleResponse = WaitForResponse(client, "/role");
    ASSERT_NE(roleResponse, nullptr);
    EXPECT_EQ(roleResponse->body, "unknown");

    auto haStatusResponse = WaitForResponse(client, "/ha_status");
    ASSERT_NE(haStatusResponse, nullptr);
    EXPECT_EQ(haStatusResponse->body, "unknown");

    auto leaderResponse = WaitForResponse(client, "/leader");
    ASSERT_NE(leaderResponse, nullptr);
    auto leaderJson = nlohmann::json::parse(leaderResponse->body);
    EXPECT_FALSE(leaderJson.at("present").get<bool>());

    auto putResponse =
        client.Put((std::string("/metadata?key=") + kHttpMetaKey).c_str(), kHttpMetadataValue, "text/plain");
    ASSERT_NE(putResponse, nullptr);
    EXPECT_EQ(putResponse->body, "metadata updated");

    auto getMetadataResponse = WaitForResponse(client, std::string("/metadata?key=") + kHttpMetaKey);
    ASSERT_NE(getMetadataResponse, nullptr);
    EXPECT_EQ(getMetadataResponse->body, kHttpMetadataValue);

    auto queryKeyResponse = WaitForResponse(client, std::string("/query_key?key=") + kHttpAllocKey);
    ASSERT_NE(queryKeyResponse, nullptr);
    auto queryKeyJson = nlohmann::json::parse(queryKeyResponse->body);
    EXPECT_EQ(queryKeyJson.at("key"), kHttpAllocKey);
    EXPECT_TRUE(queryKeyJson.at("valid").get<bool>());
    EXPECT_EQ(queryKeyJson.at("size").get<uint64_t>(), SIZE_32K);
    EXPECT_EQ(queryKeyJson.at("numBlobs").get<uint64_t>(), kHttpExpectedBlobCount);
    EXPECT_EQ(queryKeyJson.at("blobs").size(), kHttpExpectedBlobCountSize);
    EXPECT_EQ(queryKeyJson.at("blobs").at(kHttpFirstItemIndex).at("rank").get<uint32_t>(), kHttpRankId);
    EXPECT_EQ(queryKeyJson.at("blobs").at(kHttpFirstItemIndex).at("medium"), "HBM");

    auto batchQueryResponse =
        WaitForResponse(client, std::string("/batch_query_keys?keys=") + kHttpAllocKey + ",missing-key");
    ASSERT_NE(batchQueryResponse, nullptr);
    auto batchQueryJson = nlohmann::json::parse(batchQueryResponse->body);
    EXPECT_TRUE(batchQueryJson.at("success").get<bool>());
    ASSERT_EQ(batchQueryJson.at("data").size(), kHttpExpectedSegmentCount);
    EXPECT_EQ(batchQueryJson.at("data").at(kHttpFirstItemIndex).at("key"), kHttpAllocKey);
    EXPECT_TRUE(batchQueryJson.at("data").at(kHttpFirstItemIndex).at("valid").get<bool>());
    EXPECT_FALSE(batchQueryJson.at("data").at(kHttpSecondItemIndex).at("valid").get<bool>());

    auto allKeysResponse = WaitForResponse(client, "/get_all_keys");
    ASSERT_NE(allKeysResponse, nullptr);
    EXPECT_EQ(allKeysResponse->body, std::string(kHttpAllocKey) + "\n");

    auto allSegmentsResponse = WaitForResponse(client, "/get_all_segments");
    ASSERT_NE(allSegmentsResponse, nullptr);
    EXPECT_EQ(allSegmentsResponse->body, std::string(kHttpHbmSegmentName) + "\n" + kHttpDramSegmentName + "\n");

    auto querySegmentResponse = WaitForResponse(client, std::string("/query_segment?segment=") + kHttpHbmSegmentName);
    ASSERT_NE(querySegmentResponse, nullptr);
    auto querySegmentJson = nlohmann::json::parse(querySegmentResponse->body);
    EXPECT_EQ(querySegmentJson.at("segment"), kHttpHbmSegmentName);
    EXPECT_EQ(querySegmentJson.at("medium"), "HBM");
    EXPECT_EQ(querySegmentJson.at("total_bytes").get<uint64_t>(), kHttpSegmentCapacityBytes);
    EXPECT_EQ(querySegmentJson.at("used_bytes").get<uint64_t>(), SIZE_32K);
    EXPECT_EQ(querySegmentJson.at("remaining_bytes").get<uint64_t>(), kHttpSegmentCapacityBytes - SIZE_32K);

    auto segmentStatusResponse =
        WaitForResponse(client, std::string("/api/v1/segments/status?segment=") + kHttpHbmSegmentName);
    ASSERT_NE(segmentStatusResponse, nullptr);
    auto segmentStatusJson = nlohmann::json::parse(segmentStatusResponse->body);
    EXPECT_TRUE(segmentStatusJson.at("success").get<bool>());
    EXPECT_EQ(segmentStatusJson.at("segment"), kHttpHbmSegmentName);
    EXPECT_EQ(segmentStatusJson.at("status").get<int>(), kHttpSegmentStatusOk);
    EXPECT_EQ(segmentStatusJson.at("status_name"), "OK");

    auto capacityUsageResponse = WaitForResponse(client, "/api/v1/capacity/usage");
    ASSERT_NE(capacityUsageResponse, nullptr);
    auto capacityUsageJson = nlohmann::json::parse(capacityUsageResponse->body);
    EXPECT_TRUE(capacityUsageJson.at("data_source_ready").get<bool>());
    EXPECT_FALSE(capacityUsageJson.at("degraded").get<bool>());
    EXPECT_EQ(capacityUsageJson.at("npu").at("total_bytes").get<uint64_t>(), kHttpSegmentCapacityBytes);
    EXPECT_EQ(capacityUsageJson.at("npu").at("used_bytes").get<uint64_t>(), SIZE_32K);
    EXPECT_EQ(capacityUsageJson.at("npu").at("free_bytes").get<uint64_t>(), kHttpSegmentCapacityBytes - SIZE_32K);
    EXPECT_DOUBLE_EQ(capacityUsageJson.at("npu").at("usage_ratio").get<double>(), kHttpNpuUsageRatioExpected);
    EXPECT_EQ(capacityUsageJson.at("cpu").at("total_bytes").get<uint64_t>(), kHttpSegmentCapacityBytes);
    EXPECT_EQ(capacityUsageJson.at("cpu").at("used_bytes").get<uint64_t>(), kHttpZeroUsedBytes);
    EXPECT_EQ(capacityUsageJson.at("cpu").at("free_bytes").get<uint64_t>(), kHttpSegmentCapacityBytes);
    EXPECT_DOUBLE_EQ(capacityUsageJson.at("cpu").at("usage_ratio").get<double>(), kHttpCpuUsageRatioExpected);

    auto segmentRemainingResponse = WaitForResponse(client, "/api/v1/capacity/segment_remaining");
    ASSERT_NE(segmentRemainingResponse, nullptr);
    auto segmentRemainingJson = nlohmann::json::parse(segmentRemainingResponse->body);
    EXPECT_TRUE(segmentRemainingJson.at("data_source_ready").get<bool>());
    EXPECT_FALSE(segmentRemainingJson.at("degraded").get<bool>());
    ASSERT_EQ(segmentRemainingJson.at("segments").size(), kHttpExpectedSegmentCount);
    EXPECT_EQ(segmentRemainingJson.at("segments").at(kHttpFirstItemIndex).at("segment_name"), kHttpHbmSegmentName);
    EXPECT_EQ(segmentRemainingJson.at("segments").at(kHttpSecondItemIndex).at("segment_name"),
              kHttpDramSegmentName);

    auto drainJobResponse = client.Post("/api/v1/drain_jobs", "{}", "application/json");
    ASSERT_NE(drainJobResponse, nullptr);
    auto drainJobJson = nlohmann::json::parse(drainJobResponse->body);
    EXPECT_FALSE(drainJobJson.at("success").get<bool>());
    EXPECT_EQ(drainJobJson.at("error_message"), "Not supported");

    auto deleteMetadataResponse = client.Delete((std::string("/metadata?key=") + kHttpMetaKey).c_str());
    ASSERT_NE(deleteMetadataResponse, nullptr);
    EXPECT_EQ(deleteMetadataResponse->body, "metadata deleted");

    auto missingMetadataResponse = WaitForResponse(client, std::string("/metadata?key=") + kHttpMetaKey);
    ASSERT_NE(missingMetadataResponse, nullptr);
    auto missingMetadataJson = nlohmann::json::parse(missingMetadataResponse->body);
    EXPECT_FALSE(missingMetadataJson.at("success").get<bool>());
    EXPECT_EQ(missingMetadataJson.at("error_message"), "Metadata key not found");
}

TEST_F(MmcMetaServiceHttpTest, MetricsContract)
{
    httplib::Client client(kHttpLocalHost, httpPort_);

    auto metricsResponse = WaitForResponse(client, "/metrics");
    ASSERT_NE(metricsResponse, nullptr);
    EXPECT_NE(metricsResponse->body.find("memcache_alloc_requests_total"), std::string::npos);
    EXPECT_NE(metricsResponse->body.find("memcache_get_requests_total"), std::string::npos);
    EXPECT_NE(metricsResponse->body.find("memcache_batch_get_requests_total"), std::string::npos);
    EXPECT_NE(metricsResponse->body.find("memcache_stored_keys"), std::string::npos);
    EXPECT_NE(metricsResponse->body.find("memcache_segment_capacity_bytes"), std::string::npos);
    EXPECT_NE(metricsResponse->body.find("memcache_get_all_keys_requests_total"), std::string::npos);
    EXPECT_NE(metricsResponse->body.find("memcache_total_capacity_bytes"), std::string::npos);

    const MmcMetaMetricSnapshot snapshot = MmcMetaMetricManager::GetInstance().GetSnapshot();
    const std::string expectedSummary =
        std::string("keys=") + std::to_string(kHttpExpectedBlobCount) +
        " evict=" + std::to_string(snapshot.evictCount) +
        " hbm_used=" + std::to_string(SIZE_32K) + "/" + std::to_string(kHttpSegmentCapacityBytes) +
        " dram_used=" + std::to_string(kHttpZeroUsedBytes) + "/" + std::to_string(kHttpSegmentCapacityBytes) +
        " alloc_req=" + std::to_string(snapshot.allocRequestCount) +
        " alloc_fail=" + std::to_string(snapshot.allocFailureCount) +
        " batch_alloc_req=" + std::to_string(snapshot.batchAllocRequestCount) +
        " batch_alloc_fail=" + std::to_string(snapshot.batchAllocFailureCount) +
        " get_req=" + std::to_string(snapshot.getRequestCount) +
        " get_fail=" + std::to_string(snapshot.getFailureCount) +
        " batch_get_req=" + std::to_string(snapshot.batchGetRequestCount) +
        " batch_get_fail=" + std::to_string(snapshot.batchGetFailureCount) +
        " remove_req=" + std::to_string(snapshot.removeRequestCount) +
        " remove_fail=" + std::to_string(snapshot.removeFailureCount) +
        " batch_remove_req=" + std::to_string(snapshot.batchRemoveRequestCount) +
        " batch_remove_fail=" + std::to_string(snapshot.batchRemoveFailureCount) +
        " remove_all_req=" + std::to_string(snapshot.removeAllRequestCount) +
        " remove_all_fail=" + std::to_string(snapshot.removeAllFailureCount) +
        " update_state_req=" + std::to_string(snapshot.updateStateRequestCount) +
        " update_state_fail=" + std::to_string(snapshot.updateStateFailureCount) +
        " batch_update_state_req=" + std::to_string(snapshot.batchUpdateStateRequestCount) +
        " batch_update_state_fail=" + std::to_string(snapshot.batchUpdateStateFailureCount) +
        " query_req=" + std::to_string(snapshot.queryRequestCount) +
        " query_fail=" + std::to_string(snapshot.queryFailureCount) +
        " batch_query_req=" + std::to_string(snapshot.batchQueryRequestCount) +
        " batch_query_fail=" + std::to_string(snapshot.batchQueryFailureCount) +
        " get_all_keys_req=" + std::to_string(snapshot.getAllKeysRequestCount) +
        " get_all_keys_fail=" + std::to_string(snapshot.getAllKeysFailureCount) +
        " exist_key_req=" + std::to_string(snapshot.existKeyRequestCount) +
        " exist_key_fail=" + std::to_string(snapshot.existKeyFailureCount) +
        " batch_exist_key_req=" + std::to_string(snapshot.batchExistKeyRequestCount) +
        " batch_exist_key_fail=" + std::to_string(snapshot.batchExistKeyFailureCount) +
        " mount_req=" + std::to_string(snapshot.mountRequestCount) +
        " mount_fail=" + std::to_string(snapshot.mountFailureCount) +
        " unmount_req=" + std::to_string(snapshot.unmountRequestCount) +
        " unmount_fail=" + std::to_string(snapshot.unmountFailureCount);

    auto summaryResponse = WaitForResponse(client, "/metrics/summary");
    ASSERT_NE(summaryResponse, nullptr);
    EXPECT_NE(summaryResponse->body.find(expectedSummary), std::string::npos);
}

TEST_F(MmcMetaServiceHttpTest, BusinessCountersIgnoreInternalEndpoints)
{
    httplib::Client client(kHttpLocalHost, httpPort_);
    MmcMetaMetricSnapshot beforeSnapshot = MmcMetaMetricManager::GetInstance().GetSnapshot();

    auto queryKeyResponse = WaitForResponse(client, std::string("/query_key?key=") + kHttpAllocKey);
    ASSERT_NE(queryKeyResponse, nullptr);
    auto batchQueryResponse =
        WaitForResponse(client, std::string("/batch_query_keys?keys=") + kHttpAllocKey + ",missing-key");
    ASSERT_NE(batchQueryResponse, nullptr);
    auto allKeysResponse = WaitForResponse(client, "/get_all_keys");
    ASSERT_NE(allKeysResponse, nullptr);
    auto allSegmentsResponse = WaitForResponse(client, "/get_all_segments");
    ASSERT_NE(allSegmentsResponse, nullptr);
    auto querySegmentResponse = WaitForResponse(client, std::string("/query_segment?segment=") + kHttpHbmSegmentName);
    ASSERT_NE(querySegmentResponse, nullptr);

    MmcMetaMetricSnapshot afterBusinessSnapshot = MmcMetaMetricManager::GetInstance().GetSnapshot();
    EXPECT_EQ(afterBusinessSnapshot.queryRequestCount, beforeSnapshot.queryRequestCount + 1);
    EXPECT_EQ(afterBusinessSnapshot.queryFailureCount, beforeSnapshot.queryFailureCount);
    EXPECT_EQ(afterBusinessSnapshot.batchQueryRequestCount, beforeSnapshot.batchQueryRequestCount + 1);
    EXPECT_EQ(afterBusinessSnapshot.batchQueryFailureCount, beforeSnapshot.batchQueryFailureCount);
    EXPECT_EQ(afterBusinessSnapshot.getAllKeysRequestCount, beforeSnapshot.getAllKeysRequestCount + 1);
    EXPECT_EQ(afterBusinessSnapshot.getAllKeysFailureCount, beforeSnapshot.getAllKeysFailureCount);

    auto segmentStatusResponse =
        WaitForResponse(client, std::string("/api/v1/segments/status?segment=") + kHttpHbmSegmentName);
    ASSERT_NE(segmentStatusResponse, nullptr);
    auto capacityUsageResponse = WaitForResponse(client, "/api/v1/capacity/usage");
    ASSERT_NE(capacityUsageResponse, nullptr);
    auto segmentRemainingResponse = WaitForResponse(client, "/api/v1/capacity/segment_remaining");
    ASSERT_NE(segmentRemainingResponse, nullptr);
    auto metricsResponse = WaitForResponse(client, "/metrics");
    ASSERT_NE(metricsResponse, nullptr);
    auto summaryResponse = WaitForResponse(client, "/metrics/summary");
    ASSERT_NE(summaryResponse, nullptr);
    auto ptracerResponse = WaitForResponse(client, "/metrics/ptracer");
    ASSERT_NE(ptracerResponse, nullptr);
    auto healthResponse = WaitForResponse(client, "/health");
    ASSERT_NE(healthResponse, nullptr);
    auto roleResponse = WaitForResponse(client, "/role");
    ASSERT_NE(roleResponse, nullptr);
    auto haStatusResponse = WaitForResponse(client, "/ha_status");
    ASSERT_NE(haStatusResponse, nullptr);
    auto leaderResponse = WaitForResponse(client, "/leader");
    ASSERT_NE(leaderResponse, nullptr);

    MmcMetaMetricSnapshot afterInternalSnapshot = MmcMetaMetricManager::GetInstance().GetSnapshot();
    EXPECT_EQ(afterInternalSnapshot.queryRequestCount, afterBusinessSnapshot.queryRequestCount);
    EXPECT_EQ(afterInternalSnapshot.queryFailureCount, afterBusinessSnapshot.queryFailureCount);
    EXPECT_EQ(afterInternalSnapshot.batchQueryRequestCount, afterBusinessSnapshot.batchQueryRequestCount);
    EXPECT_EQ(afterInternalSnapshot.batchQueryFailureCount, afterBusinessSnapshot.batchQueryFailureCount);
    EXPECT_EQ(afterInternalSnapshot.getAllKeysRequestCount, afterBusinessSnapshot.getAllKeysRequestCount);
    EXPECT_EQ(afterInternalSnapshot.getAllKeysFailureCount, afterBusinessSnapshot.getAllKeysFailureCount);
}

TEST_F(MmcMetaServiceHttpTest, BatchQueryKeysIgnoresTrailingComma)
{
    httplib::Client client(kHttpLocalHost, httpPort_);

    // Trailing comma: "http-contract-key," should resolve to exactly one key, not two.
    auto response = WaitForResponse(client, std::string("/batch_query_keys?keys=") + kHttpAllocKey + ",");
    ASSERT_NE(response, nullptr);
    ASSERT_EQ(response->status, httplib::OK_200);
    auto json = nlohmann::json::parse(response->body);
    EXPECT_TRUE(json.at("success").get<bool>());
    ASSERT_EQ(json.at("data").size(), kHttpExpectedBlobCountSize)
        << "Trailing comma should not produce an extra empty-key entry";
    EXPECT_EQ(json.at("data").at(kHttpFirstItemIndex).at("key"), kHttpAllocKey);
    EXPECT_TRUE(json.at("data").at(kHttpFirstItemIndex).at("valid").get<bool>());
}

TEST_F(MmcMetaServiceHttpTest, ProxyGetCountersTrackAttempts)
{
    const MmcMetaMetricSnapshot beforeSnapshot = MmcMetaMetricManager::GetInstance().GetSnapshot();

    AllocResponse getResponse;
    const GetRequest getRequest(kHttpAllocKey, kHttpRankId, GenerateOperateId(kHttpRankId), true);
    EXPECT_EQ(metaMgrProxy_->Get(getRequest, getResponse), MMC_OK);

    AllocResponse missingGetResponse;
    const GetRequest missingGetRequest("missing-key", kHttpRankId, GenerateOperateId(kHttpRankId), true);
    EXPECT_NE(metaMgrProxy_->Get(missingGetRequest, missingGetResponse), MMC_OK);

    BatchAllocResponse batchGetResponse;
    const BatchGetRequest batchGetRequest({kHttpAllocKey, "missing-key"}, kHttpRankId, GenerateOperateId(kHttpRankId));
    EXPECT_EQ(metaMgrProxy_->BatchGet(batchGetRequest, batchGetResponse), MMC_OK);
    ASSERT_EQ(batchGetResponse.results_.size(), kHttpExpectedSegmentCount);
    EXPECT_EQ(batchGetResponse.results_.at(kHttpFirstItemIndex), MMC_OK);
    EXPECT_NE(batchGetResponse.results_.at(kHttpSecondItemIndex), MMC_OK);

    const MmcMetaMetricSnapshot afterSnapshot = MmcMetaMetricManager::GetInstance().GetSnapshot();
    constexpr uint64_t kHttpGetAttemptCount = 2;
    constexpr uint64_t kHttpGetFailureCount = 1;
    EXPECT_EQ(afterSnapshot.getRequestCount, beforeSnapshot.getRequestCount + kHttpGetAttemptCount);
    EXPECT_EQ(afterSnapshot.getFailureCount, beforeSnapshot.getFailureCount + kHttpGetFailureCount);
    EXPECT_EQ(afterSnapshot.batchGetRequestCount, beforeSnapshot.batchGetRequestCount + 1);
    EXPECT_EQ(afterSnapshot.batchGetFailureCount, beforeSnapshot.batchGetFailureCount);
}
