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

#ifndef MMC_META_SERVICE_HA_H
#define MMC_META_SERVICE_HA_H

#include <string>
#include <mutex>
#include <thread>
#include <atomic>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#include <pybind11/stl.h>
#pragma GCC diagnostic pop

#include "mmc_types.h"

namespace ock {
namespace mmc {

#pragma GCC visibility push(default)
class MmcMetaServiceLeaderElection {
public:
    explicit MmcMetaServiceLeaderElection(
        const std::string &name, const std::string &pod, const std::string &ns, const std::string &lease);

    ~MmcMetaServiceLeaderElection();

    Result Start(const mmc_meta_service_config_t &options);

    void Stop();

private:
    void ElectionLoop();
    void CheckLeaderStatus();
    void RenewLoop();
    void OnStartLeading();
    void OnStopLeading();

    std::mutex mutex_;
    std::thread electionThread_;
    std::thread renewThread_;
    std::atomic<bool> running_{false};
    std::atomic<bool> isLeader_{false};
    std::string podName_;
    std::string leaseName_;
    std::string ns_;
    std::string name_;
    pybind11::object leaderElection_ = pybind11::none();
};
#pragma GCC visibility pop

}  // namespace mmc
}  // namespace ock

#endif // MMC_META_SERVICE_HA_H