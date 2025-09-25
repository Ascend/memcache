/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023. All rights reserved.
 */

#ifndef MF_HYBRID_BIPARTITE_RANKS_QP_MANAGER_H
#define MF_HYBRID_BIPARTITE_RANKS_QP_MANAGER_H
#include <atomic>
#include <thread>
#include <list>
#include <mutex>
#include <condition_variable>
#include <unordered_set>
#include "device_qp_manager.h"
#include "bipartite_ranks_qp_def.h"

namespace ock {
namespace mf {
namespace transport {
namespace device {

class BipartiteRanksQpManager : public DeviceQpManager {
public:
    BipartiteRanksQpManager(uint32_t deviceId, uint32_t rankId, uint32_t rankCount, sockaddr_in devNet,
                          bool server) noexcept;
    ~BipartiteRanksQpManager() noexcept override;

    int SetRemoteRankInfo(const std::unordered_map<uint32_t, ConnectRankInfo> &ranks) noexcept override;
    int Startup(void *rdma) noexcept override;
    void Shutdown() noexcept override;
    UserQpInfo *GetQpHandleWithRankId(uint32_t rankId) noexcept override;
    void PutQpHandle(UserQpInfo *qp) const noexcept override;

private:
    void BackgroundProcess() noexcept;
    int ProcessServerAddWhitelistTask() noexcept;
    int ProcessClientConnectSocketTask() noexcept;
    int ProcessQueryConnectionStateTask() noexcept;
    int ProcessConnectQpTask() noexcept;
    int ProcessQueryQpStateTask() noexcept;
    void CloseServices() noexcept;

    void GenDiffInfoChangeRanks(const std::unordered_map<uint32_t, ConnectRankInfo> &last,
                                std::unordered_map<uint32_t, sockaddr_in> &addedRanks) noexcept;
    void GenTaskFromChangeRanks(const std::unordered_map<uint32_t, sockaddr_in> &addedRanks) noexcept;

private:
    void *rdmaHandle_{nullptr};
    std::unordered_map<uint32_t, ConnectRankInfo> currentRanksInfo_;
    MemoryRegionMap currentLocalMrs_;
    std::atomic<bool> managerRunning_{false};
    std::mutex mutex_;
    std::condition_variable cond_;
    std::shared_ptr<std::thread> backGroundThread_;
    ConnectionTasks connectionTasks_;
    std::unordered_map<uint32_t, ConnectionChannel> connections_;
    std::vector<ConnectionChannel *> connectionView_;
    std::vector<UserQpInfo> userQpInfo_;
};
}
}
}
}

#endif  // MF_HYBRID_BIPARTITE_RANKS_QP_MANAGER_H
