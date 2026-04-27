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
#include <sstream>
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
    EXPECT_NE(metricsResponse->body.find("memcache_alloc_successes_total"), std::string::npos);
    EXPECT_NE(metricsResponse->body.find("memcache_get_requests_total"), std::string::npos);
    EXPECT_NE(metricsResponse->body.find("memcache_get_successes_total"), std::string::npos);
    EXPECT_NE(metricsResponse->body.find("memcache_get_not_found_total"), std::string::npos);
    EXPECT_NE(metricsResponse->body.find("memcache_batch_get_requests_total"), std::string::npos);
    EXPECT_NE(metricsResponse->body.find("memcache_batch_get_successes_total"), std::string::npos);
    EXPECT_NE(metricsResponse->body.find("memcache_batch_get_not_found_total"), std::string::npos);
    EXPECT_NE(metricsResponse->body.find("memcache_stored_keys"), std::string::npos);
    EXPECT_NE(metricsResponse->body.find("memcache_segment_capacity_bytes"), std::string::npos);
    EXPECT_NE(metricsResponse->body.find("memcache_get_all_keys_requests_total"), std::string::npos);
    EXPECT_NE(metricsResponse->body.find("memcache_get_all_keys_successes_total"), std::string::npos);
    EXPECT_NE(metricsResponse->body.find("memcache_total_capacity_bytes"), std::string::npos);
    EXPECT_NE(metricsResponse->body.find("memcache_query_successes_total"), std::string::npos);
    EXPECT_NE(metricsResponse->body.find("memcache_query_not_found_total"), std::string::npos);
    EXPECT_NE(metricsResponse->body.find("memcache_batch_query_successes_total"), std::string::npos);
    EXPECT_NE(metricsResponse->body.find("memcache_batch_query_not_found_total"), std::string::npos);
    EXPECT_NE(metricsResponse->body.find("memcache_remove_successes_total"), std::string::npos);
    EXPECT_NE(metricsResponse->body.find("memcache_remove_not_found_total"), std::string::npos);
    EXPECT_NE(metricsResponse->body.find("memcache_batch_remove_successes_total"), std::string::npos);
    EXPECT_NE(metricsResponse->body.find("memcache_batch_remove_not_found_total"), std::string::npos);
    EXPECT_NE(metricsResponse->body.find("memcache_remove_all_successes_total"), std::string::npos);
    EXPECT_NE(metricsResponse->body.find("memcache_update_state_successes_total"), std::string::npos);
    EXPECT_NE(metricsResponse->body.find("memcache_update_state_not_found_total"), std::string::npos);
    EXPECT_NE(metricsResponse->body.find("memcache_batch_update_state_successes_total"), std::string::npos);
    EXPECT_NE(metricsResponse->body.find("memcache_batch_update_state_not_found_total"), std::string::npos);
    EXPECT_NE(metricsResponse->body.find("memcache_exist_key_successes_total"), std::string::npos);
    EXPECT_NE(metricsResponse->body.find("memcache_exist_key_not_found_total"), std::string::npos);
    EXPECT_NE(metricsResponse->body.find("memcache_batch_exist_key_successes_total"), std::string::npos);
    EXPECT_NE(metricsResponse->body.find("memcache_batch_exist_key_not_found_total"), std::string::npos);
    EXPECT_NE(metricsResponse->body.find("memcache_mount_successes_total"), std::string::npos);
    EXPECT_NE(metricsResponse->body.find("memcache_unmount_successes_total"), std::string::npos);

    const MmcMetaMetricSnapshot snapshot = MmcMetaMetricManager::GetInstance().GetSnapshot();
    std::ostringstream expectedSummary;
    expectedSummary
        << "keys=" << kHttpExpectedBlobCount << " evict=" << snapshot.evictCount << " hbm_used=" << SIZE_32K << "/"
        << kHttpSegmentCapacityBytes << " dram_used=" << kHttpZeroUsedBytes << "/" << kHttpSegmentCapacityBytes
        << " alloc_req=" << snapshot.allocRequestCount << " alloc_success=" << snapshot.allocSuccessCount
        << " alloc_fail=" << snapshot.allocFailureCount << " batch_alloc_req=" << snapshot.batchAllocRequestCount
        << " batch_alloc_success=" << snapshot.batchAllocSuccessCount
        << " batch_alloc_fail=" << snapshot.batchAllocFailureCount << " get_req=" << snapshot.getRequestCount
        << " get_success=" << snapshot.getSuccessCount << " get_fail=" << snapshot.getFailureCount
        << " get_not_found=" << snapshot.getNotFoundCount << " batch_get_req=" << snapshot.batchGetRequestCount
        << " batch_get_success=" << snapshot.batchGetSuccessCount << " batch_get_fail=" << snapshot.batchGetFailureCount
        << " batch_get_not_found=" << snapshot.batchGetNotFoundCount << " remove_req=" << snapshot.removeRequestCount
        << " remove_success=" << snapshot.removeSuccessCount << " remove_fail=" << snapshot.removeFailureCount
        << " remove_not_found=" << snapshot.removeNotFoundCount
        << " batch_remove_req=" << snapshot.batchRemoveRequestCount
        << " batch_remove_success=" << snapshot.batchRemoveSuccessCount
        << " batch_remove_fail=" << snapshot.batchRemoveFailureCount
        << " batch_remove_not_found=" << snapshot.batchRemoveNotFoundCount
        << " remove_all_req=" << snapshot.removeAllRequestCount
        << " remove_all_success=" << snapshot.removeAllSuccessCount
        << " remove_all_fail=" << snapshot.removeAllFailureCount
        << " update_state_req=" << snapshot.updateStateRequestCount
        << " update_state_success=" << snapshot.updateStateSuccessCount
        << " update_state_fail=" << snapshot.updateStateFailureCount
        << " update_state_not_found=" << snapshot.updateStateNotFoundCount
        << " batch_update_state_req=" << snapshot.batchUpdateStateRequestCount
        << " batch_update_state_success=" << snapshot.batchUpdateStateSuccessCount
        << " batch_update_state_fail=" << snapshot.batchUpdateStateFailureCount
        << " batch_update_state_not_found=" << snapshot.batchUpdateStateNotFoundCount
        << " query_req=" << snapshot.queryRequestCount << " query_success=" << snapshot.querySuccessCount
        << " query_fail=" << snapshot.queryFailureCount << " query_not_found=" << snapshot.queryNotFoundCount
        << " batch_query_req=" << snapshot.batchQueryRequestCount
        << " batch_query_success=" << snapshot.batchQuerySuccessCount
        << " batch_query_fail=" << snapshot.batchQueryFailureCount
        << " batch_query_not_found=" << snapshot.batchQueryNotFoundCount
        << " get_all_keys_req=" << snapshot.getAllKeysRequestCount
        << " get_all_keys_success=" << snapshot.getAllKeysSuccessCount
        << " get_all_keys_fail=" << snapshot.getAllKeysFailureCount
        << " exist_key_req=" << snapshot.existKeyRequestCount << " exist_key_success=" << snapshot.existKeySuccessCount
        << " exist_key_fail=" << snapshot.existKeyFailureCount
        << " exist_key_not_found=" << snapshot.existKeyNotFoundCount
        << " batch_exist_key_req=" << snapshot.batchExistKeyRequestCount
        << " batch_exist_key_success=" << snapshot.batchExistKeySuccessCount
        << " batch_exist_key_fail=" << snapshot.batchExistKeyFailureCount
        << " batch_exist_key_not_found=" << snapshot.batchExistKeyNotFoundCount
        << " mount_req=" << snapshot.mountRequestCount << " mount_success=" << snapshot.mountSuccessCount
        << " mount_fail=" << snapshot.mountFailureCount << " unmount_req=" << snapshot.unmountRequestCount
        << " unmount_success=" << snapshot.unmountSuccessCount << " unmount_fail=" << snapshot.unmountFailureCount;

    auto summaryResponse = WaitForResponse(client, "/metrics/summary");
    ASSERT_NE(summaryResponse, nullptr);
    EXPECT_NE(summaryResponse->body.find(expectedSummary.str()), std::string::npos);
}

TEST_F(MmcMetaServiceHttpTest, BusinessCountersIgnoreInternalEndpoints)
{
    constexpr uint64_t queryRequestDelta = 3;
    constexpr uint64_t querySuccessDelta = 2;
    constexpr uint64_t notFoundDelta = 1;
    constexpr uint64_t batchRequestDelta = 1;
    constexpr uint64_t getAllKeysRequestDelta = 1;

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
    EXPECT_EQ(afterBusinessSnapshot.queryRequestCount, beforeSnapshot.queryRequestCount + queryRequestDelta);
    EXPECT_EQ(afterBusinessSnapshot.querySuccessCount, beforeSnapshot.querySuccessCount + querySuccessDelta);
    EXPECT_EQ(afterBusinessSnapshot.queryFailureCount, beforeSnapshot.queryFailureCount);
    EXPECT_EQ(afterBusinessSnapshot.queryNotFoundCount, beforeSnapshot.queryNotFoundCount + notFoundDelta);
    EXPECT_EQ(afterBusinessSnapshot.batchQueryRequestCount, beforeSnapshot.batchQueryRequestCount + batchRequestDelta);
    EXPECT_EQ(afterBusinessSnapshot.batchQuerySuccessCount, beforeSnapshot.batchQuerySuccessCount + batchRequestDelta);
    EXPECT_EQ(afterBusinessSnapshot.batchQueryNotFoundCount, beforeSnapshot.batchQueryNotFoundCount);
    EXPECT_EQ(afterBusinessSnapshot.batchQueryFailureCount, beforeSnapshot.batchQueryFailureCount);
    EXPECT_EQ(afterBusinessSnapshot.getAllKeysRequestCount,
              beforeSnapshot.getAllKeysRequestCount + getAllKeysRequestDelta);
    EXPECT_EQ(afterBusinessSnapshot.getAllKeysSuccessCount,
              beforeSnapshot.getAllKeysSuccessCount + getAllKeysRequestDelta);
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
    EXPECT_EQ(afterInternalSnapshot.querySuccessCount, afterBusinessSnapshot.querySuccessCount);
    EXPECT_EQ(afterInternalSnapshot.queryNotFoundCount, afterBusinessSnapshot.queryNotFoundCount);
    EXPECT_EQ(afterInternalSnapshot.batchQueryRequestCount, afterBusinessSnapshot.batchQueryRequestCount);
    EXPECT_EQ(afterInternalSnapshot.batchQuerySuccessCount, afterBusinessSnapshot.batchQuerySuccessCount);
    EXPECT_EQ(afterInternalSnapshot.batchQueryNotFoundCount, afterBusinessSnapshot.batchQueryNotFoundCount);
    EXPECT_EQ(afterInternalSnapshot.batchQueryFailureCount, afterBusinessSnapshot.batchQueryFailureCount);
    EXPECT_EQ(afterInternalSnapshot.getAllKeysRequestCount, afterBusinessSnapshot.getAllKeysRequestCount);
    EXPECT_EQ(afterInternalSnapshot.getAllKeysSuccessCount, afterBusinessSnapshot.getAllKeysSuccessCount);
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

TEST_F(MmcMetaServiceHttpTest, ProxyDuplicateAllocCountersTrackFailures)
{
    constexpr uint64_t requestDelta = 1;
    constexpr size_t batchDuplicateItemCount = 1U;
    const AllocOptions duplicateAllocOptions(SIZE_32K, kHttpExpectedBlobCount, MEDIA_HBM, {kHttpRankId},
                                             kHttpAllocOffset);

    MmcMetaMetricSnapshot beforeSnapshot = MmcMetaMetricManager::GetInstance().GetSnapshot();

    AllocResponse duplicateAllocResponse;
    const AllocRequest duplicateAllocRequest(kHttpAllocKey, duplicateAllocOptions, GenerateOperateId(kHttpRankId));
    EXPECT_EQ(metaMgrProxy_->Alloc(duplicateAllocRequest, duplicateAllocResponse), MMC_DUPLICATED_OBJECT);

    MmcMetaMetricSnapshot afterSnapshot = MmcMetaMetricManager::GetInstance().GetSnapshot();
    EXPECT_EQ(afterSnapshot.allocRequestCount, beforeSnapshot.allocRequestCount + requestDelta);
    EXPECT_EQ(afterSnapshot.allocSuccessCount, beforeSnapshot.allocSuccessCount);
    EXPECT_EQ(afterSnapshot.allocFailureCount, beforeSnapshot.allocFailureCount + requestDelta);

    beforeSnapshot = afterSnapshot;
    const std::vector<std::string> batchKeys{kHttpAllocKey};
    const std::vector<AllocOptions> batchOptions{duplicateAllocOptions};
    BatchAllocResponse batchAllocResponse;
    const BatchAllocRequest batchAllocRequest(batchKeys, batchOptions, 0, GenerateOperateId(kHttpRankId));
    EXPECT_EQ(metaMgrProxy_->BatchAlloc(batchAllocRequest, batchAllocResponse), MMC_OK);
    ASSERT_EQ(batchAllocResponse.results_.size(), batchDuplicateItemCount);
    EXPECT_EQ(batchAllocResponse.results_.at(kHttpFirstItemIndex), MMC_DUPLICATED_OBJECT);

    afterSnapshot = MmcMetaMetricManager::GetInstance().GetSnapshot();
    EXPECT_EQ(afterSnapshot.batchAllocRequestCount, beforeSnapshot.batchAllocRequestCount + requestDelta);
    EXPECT_EQ(afterSnapshot.batchAllocSuccessCount, beforeSnapshot.batchAllocSuccessCount);
    EXPECT_EQ(afterSnapshot.batchAllocFailureCount, beforeSnapshot.batchAllocFailureCount + requestDelta);
    EXPECT_EQ(afterSnapshot.allocRequestCount, beforeSnapshot.allocRequestCount + requestDelta);
    EXPECT_EQ(afterSnapshot.allocSuccessCount, beforeSnapshot.allocSuccessCount);
    EXPECT_EQ(afterSnapshot.allocFailureCount, beforeSnapshot.allocFailureCount + requestDelta);
}

TEST_F(MmcMetaServiceHttpTest, ProxyGetCountersTrackAttempts)
{
    constexpr uint64_t getRequestDelta = 4;
    constexpr uint64_t getSuccessDelta = 2;
    constexpr uint64_t getNotFoundDelta = 2;
    constexpr uint64_t batchGetRequestDelta = 1;

    const MmcMetaMetricSnapshot beforeSnapshot = MmcMetaMetricManager::GetInstance().GetSnapshot();

    const std::string missingGetKey = "http-contract-missing-get-key";
    AllocResponse getResponse;
    const GetRequest getRequest(kHttpAllocKey, kHttpRankId, GenerateOperateId(kHttpRankId), true);
    EXPECT_EQ(metaMgrProxy_->Get(getRequest, getResponse), MMC_OK);

    AllocResponse missingGetResponse;
    const GetRequest missingGetRequest(missingGetKey, kHttpRankId, GenerateOperateId(kHttpRankId), true);
    EXPECT_EQ(metaMgrProxy_->Get(missingGetRequest, missingGetResponse), MMC_UNMATCHED_KEY);

    BatchAllocResponse batchGetResponse;
    const std::string missingBatchGetKey = "http-contract-missing-batch-get-key";
    const BatchGetRequest batchGetRequest({kHttpAllocKey, missingBatchGetKey}, kHttpRankId,
                                          GenerateOperateId(kHttpRankId));
    EXPECT_EQ(metaMgrProxy_->BatchGet(batchGetRequest, batchGetResponse), MMC_OK);
    ASSERT_EQ(batchGetResponse.results_.size(), kHttpExpectedSegmentCount);
    EXPECT_EQ(batchGetResponse.results_.at(kHttpFirstItemIndex), MMC_OK);
    EXPECT_EQ(batchGetResponse.results_.at(kHttpSecondItemIndex), MMC_UNMATCHED_KEY);

    const MmcMetaMetricSnapshot afterSnapshot = MmcMetaMetricManager::GetInstance().GetSnapshot();
    EXPECT_EQ(afterSnapshot.getRequestCount, beforeSnapshot.getRequestCount + getRequestDelta);
    EXPECT_EQ(afterSnapshot.getSuccessCount, beforeSnapshot.getSuccessCount + getSuccessDelta);
    EXPECT_EQ(afterSnapshot.getFailureCount, beforeSnapshot.getFailureCount);
    EXPECT_EQ(afterSnapshot.getNotFoundCount, beforeSnapshot.getNotFoundCount + getNotFoundDelta);
    EXPECT_EQ(afterSnapshot.batchGetRequestCount, beforeSnapshot.batchGetRequestCount + batchGetRequestDelta);
    EXPECT_EQ(afterSnapshot.batchGetSuccessCount, beforeSnapshot.batchGetSuccessCount + batchGetRequestDelta);
    EXPECT_EQ(afterSnapshot.batchGetNotFoundCount, beforeSnapshot.batchGetNotFoundCount);
    EXPECT_EQ(afterSnapshot.batchGetFailureCount, beforeSnapshot.batchGetFailureCount);
}

TEST_F(MmcMetaServiceHttpTest, ProxySingleOpsTrackCounters)
{
    constexpr uint64_t pairRequestDelta = 2;
    constexpr uint64_t successDelta = 1;
    constexpr uint64_t notFoundDelta = 1;
    constexpr uint64_t singleRequestDelta = 1;

    const std::string missingExistKey = "http-contract-missing-exist-key";
    const std::string missingQueryKey = "http-contract-missing-query-key";
    const std::string missingRemoveKey = "http-contract-missing-remove-key";
    const std::string missingUpdateKey = "http-contract-missing-update-key";

    MmcMetaMetricSnapshot beforeSnapshot = MmcMetaMetricManager::GetInstance().GetSnapshot();

    IsExistResponse existResponse;
    EXPECT_EQ(metaMgrProxy_->ExistKey(IsExistRequest(kHttpAllocKey), existResponse), MMC_OK);

    IsExistResponse missingExistResponse;
    EXPECT_EQ(metaMgrProxy_->ExistKey(IsExistRequest(missingExistKey), missingExistResponse), MMC_UNMATCHED_KEY);

    MmcMetaMetricSnapshot afterSnapshot = MmcMetaMetricManager::GetInstance().GetSnapshot();
    EXPECT_EQ(afterSnapshot.existKeyRequestCount, beforeSnapshot.existKeyRequestCount + pairRequestDelta);
    EXPECT_EQ(afterSnapshot.existKeySuccessCount, beforeSnapshot.existKeySuccessCount + successDelta);
    EXPECT_EQ(afterSnapshot.existKeyNotFoundCount, beforeSnapshot.existKeyNotFoundCount + notFoundDelta);
    EXPECT_EQ(afterSnapshot.existKeyFailureCount, beforeSnapshot.existKeyFailureCount);

    beforeSnapshot = afterSnapshot;
    QueryResponse queryResponse;
    EXPECT_EQ(metaMgrProxy_->Query(QueryRequest(kHttpAllocKey), queryResponse), MMC_OK);

    QueryResponse missingQueryResponse;
    EXPECT_EQ(metaMgrProxy_->Query(QueryRequest(missingQueryKey), missingQueryResponse), MMC_UNMATCHED_KEY);

    afterSnapshot = MmcMetaMetricManager::GetInstance().GetSnapshot();
    EXPECT_EQ(afterSnapshot.queryRequestCount, beforeSnapshot.queryRequestCount + pairRequestDelta);
    EXPECT_EQ(afterSnapshot.querySuccessCount, beforeSnapshot.querySuccessCount + successDelta);
    EXPECT_EQ(afterSnapshot.queryNotFoundCount, beforeSnapshot.queryNotFoundCount + notFoundDelta);
    EXPECT_EQ(afterSnapshot.queryFailureCount, beforeSnapshot.queryFailureCount);

    beforeSnapshot = afterSnapshot;
    const uint64_t updateOperateId = GenerateOperateId(kHttpRankId);
    AllocResponse updateGetResponse;
    EXPECT_EQ(metaMgrProxy_->Get(GetRequest(kHttpAllocKey, kHttpRankId, updateOperateId, true), updateGetResponse),
              MMC_OK);

    Response updateResponse;
    EXPECT_EQ(
        metaMgrProxy_->UpdateState(
            UpdateRequest(MMC_READ_FINISH, kHttpAllocKey, kHttpRankId, MEDIA_HBM, updateOperateId), updateResponse),
        MMC_OK);
    EXPECT_EQ(updateResponse.ret_, MMC_OK);

    Response missingUpdateResponse;
    EXPECT_EQ(metaMgrProxy_->UpdateState(UpdateRequest(MMC_READ_FINISH, missingUpdateKey, kHttpRankId, MEDIA_HBM,
                                                       GenerateOperateId(kHttpRankId)),
                                         missingUpdateResponse),
              MMC_OK);
    EXPECT_EQ(missingUpdateResponse.ret_, MMC_UNMATCHED_KEY);

    afterSnapshot = MmcMetaMetricManager::GetInstance().GetSnapshot();
    EXPECT_EQ(afterSnapshot.updateStateRequestCount, beforeSnapshot.updateStateRequestCount + pairRequestDelta);
    EXPECT_EQ(afterSnapshot.updateStateSuccessCount, beforeSnapshot.updateStateSuccessCount + successDelta);
    EXPECT_EQ(afterSnapshot.updateStateNotFoundCount, beforeSnapshot.updateStateNotFoundCount + notFoundDelta);
    EXPECT_EQ(afterSnapshot.updateStateFailureCount, beforeSnapshot.updateStateFailureCount);

    beforeSnapshot = afterSnapshot;
    Response removeResponse;
    EXPECT_EQ(metaMgrProxy_->Remove(RemoveRequest(missingRemoveKey), removeResponse), MMC_UNMATCHED_KEY);

    afterSnapshot = MmcMetaMetricManager::GetInstance().GetSnapshot();
    EXPECT_EQ(afterSnapshot.removeRequestCount, beforeSnapshot.removeRequestCount + singleRequestDelta);
    EXPECT_EQ(afterSnapshot.removeSuccessCount, beforeSnapshot.removeSuccessCount);
    EXPECT_EQ(afterSnapshot.removeNotFoundCount, beforeSnapshot.removeNotFoundCount + notFoundDelta);
    EXPECT_EQ(afterSnapshot.removeFailureCount, beforeSnapshot.removeFailureCount);

    beforeSnapshot = afterSnapshot;
    EXPECT_EQ(metaMgrProxy_->Remove(RemoveRequest(kHttpAllocKey), removeResponse), MMC_OK);

    afterSnapshot = MmcMetaMetricManager::GetInstance().GetSnapshot();
    EXPECT_EQ(afterSnapshot.removeRequestCount, beforeSnapshot.removeRequestCount + singleRequestDelta);
    EXPECT_EQ(afterSnapshot.removeSuccessCount, beforeSnapshot.removeSuccessCount + successDelta);
    EXPECT_EQ(afterSnapshot.removeNotFoundCount, beforeSnapshot.removeNotFoundCount);
    EXPECT_EQ(afterSnapshot.removeFailureCount, beforeSnapshot.removeFailureCount);
}

TEST_F(MmcMetaServiceHttpTest, ProxyBatchOpsTrackCounters)
{
    constexpr uint64_t batchSubcallDelta = 2;
    constexpr uint64_t successDelta = 1;
    constexpr uint64_t notFoundDelta = 1;
    constexpr uint64_t batchRequestDelta = 1;

    const std::string missingBatchExistKey = "http-contract-missing-batch-exist-key";
    const std::string missingBatchQueryKey = "http-contract-missing-batch-query-key";
    const std::string missingBatchRemoveKey = "http-contract-missing-batch-remove-key";

    MmcMetaMetricSnapshot beforeSnapshot = MmcMetaMetricManager::GetInstance().GetSnapshot();

    BatchIsExistResponse batchExistResponse;
    EXPECT_EQ(
        metaMgrProxy_->BatchExistKey(BatchIsExistRequest({kHttpAllocKey, missingBatchExistKey}), batchExistResponse),
        MMC_OK);
    ASSERT_EQ(batchExistResponse.results_.size(), kHttpExpectedSegmentCount);
    EXPECT_EQ(batchExistResponse.results_.at(kHttpFirstItemIndex), MMC_OK);
    EXPECT_EQ(batchExistResponse.results_.at(kHttpSecondItemIndex), MMC_UNMATCHED_KEY);

    MmcMetaMetricSnapshot afterSnapshot = MmcMetaMetricManager::GetInstance().GetSnapshot();
    EXPECT_EQ(afterSnapshot.existKeyRequestCount, beforeSnapshot.existKeyRequestCount + batchSubcallDelta);
    EXPECT_EQ(afterSnapshot.existKeySuccessCount, beforeSnapshot.existKeySuccessCount + successDelta);
    EXPECT_EQ(afterSnapshot.existKeyNotFoundCount, beforeSnapshot.existKeyNotFoundCount + notFoundDelta);
    EXPECT_EQ(afterSnapshot.existKeyFailureCount, beforeSnapshot.existKeyFailureCount);
    EXPECT_EQ(afterSnapshot.batchExistKeyRequestCount, beforeSnapshot.batchExistKeyRequestCount + batchRequestDelta);
    EXPECT_EQ(afterSnapshot.batchExistKeySuccessCount, beforeSnapshot.batchExistKeySuccessCount + successDelta);
    EXPECT_EQ(afterSnapshot.batchExistKeyNotFoundCount, beforeSnapshot.batchExistKeyNotFoundCount);
    EXPECT_EQ(afterSnapshot.batchExistKeyFailureCount, beforeSnapshot.batchExistKeyFailureCount);

    beforeSnapshot = afterSnapshot;
    BatchQueryResponse batchQueryResponse;
    EXPECT_EQ(metaMgrProxy_->BatchQuery(BatchQueryRequest({kHttpAllocKey, missingBatchQueryKey}), batchQueryResponse),
              MMC_OK);
    ASSERT_EQ(batchQueryResponse.batchQueryInfos_.size(), kHttpExpectedSegmentCount);
    EXPECT_TRUE(batchQueryResponse.batchQueryInfos_.at(kHttpFirstItemIndex).valid_);
    EXPECT_FALSE(batchQueryResponse.batchQueryInfos_.at(kHttpSecondItemIndex).valid_);

    afterSnapshot = MmcMetaMetricManager::GetInstance().GetSnapshot();
    EXPECT_EQ(afterSnapshot.queryRequestCount, beforeSnapshot.queryRequestCount + batchSubcallDelta);
    EXPECT_EQ(afterSnapshot.querySuccessCount, beforeSnapshot.querySuccessCount + successDelta);
    EXPECT_EQ(afterSnapshot.queryNotFoundCount, beforeSnapshot.queryNotFoundCount + notFoundDelta);
    EXPECT_EQ(afterSnapshot.queryFailureCount, beforeSnapshot.queryFailureCount);
    EXPECT_EQ(afterSnapshot.batchQueryRequestCount, beforeSnapshot.batchQueryRequestCount + batchRequestDelta);
    EXPECT_EQ(afterSnapshot.batchQuerySuccessCount, beforeSnapshot.batchQuerySuccessCount + successDelta);
    EXPECT_EQ(afterSnapshot.batchQueryNotFoundCount, beforeSnapshot.batchQueryNotFoundCount);
    EXPECT_EQ(afterSnapshot.batchQueryFailureCount, beforeSnapshot.batchQueryFailureCount);

    beforeSnapshot = afterSnapshot;
    BatchRemoveResponse batchRemoveResponse;
    EXPECT_EQ(
        metaMgrProxy_->BatchRemove(BatchRemoveRequest({kHttpAllocKey, missingBatchRemoveKey}), batchRemoveResponse),
        MMC_OK);
    ASSERT_EQ(batchRemoveResponse.results_.size(), kHttpExpectedSegmentCount);
    EXPECT_EQ(batchRemoveResponse.results_.at(kHttpFirstItemIndex), MMC_OK);
    EXPECT_EQ(batchRemoveResponse.results_.at(kHttpSecondItemIndex), MMC_UNMATCHED_KEY);

    afterSnapshot = MmcMetaMetricManager::GetInstance().GetSnapshot();
    EXPECT_EQ(afterSnapshot.removeRequestCount, beforeSnapshot.removeRequestCount + batchSubcallDelta);
    EXPECT_EQ(afterSnapshot.removeSuccessCount, beforeSnapshot.removeSuccessCount + successDelta);
    EXPECT_EQ(afterSnapshot.removeNotFoundCount, beforeSnapshot.removeNotFoundCount + notFoundDelta);
    EXPECT_EQ(afterSnapshot.removeFailureCount, beforeSnapshot.removeFailureCount);
    EXPECT_EQ(afterSnapshot.batchRemoveRequestCount, beforeSnapshot.batchRemoveRequestCount + batchRequestDelta);
    EXPECT_EQ(afterSnapshot.batchRemoveSuccessCount, beforeSnapshot.batchRemoveSuccessCount + successDelta);
    EXPECT_EQ(afterSnapshot.batchRemoveNotFoundCount, beforeSnapshot.batchRemoveNotFoundCount);
    EXPECT_EQ(afterSnapshot.batchRemoveFailureCount, beforeSnapshot.batchRemoveFailureCount);
}
