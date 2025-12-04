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
#ifndef MEM_FABRIC_MMC_META_SERVICE_DEFAULT_H
#define MEM_FABRIC_MMC_META_SERVICE_DEFAULT_H

#include "mmc_def.h"
#include "mmc_global_allocator.h"
#include "mmc_meta_common.h"
#include "mmc_meta_net_server.h"
#include "mmc_meta_service.h"
#include "mmc_meta_mgr_proxy_default.h"
#include "smem_config_store.h"

namespace ock {
namespace mmc {
class MmcMetaServiceDefault : public MmcMetaService {
public:
    explicit MmcMetaServiceDefault(const std::string& name) : name_(name), options_{} {}

    Result Start(const mmc_meta_service_config_t &options) override;

    void Stop() override;

    Result BmRegister(uint32_t rank, std::vector<uint16_t> mediaType, std::vector<uint64_t> bm,
                      std::vector<uint64_t> capacity, std::map<std::string, MmcMemBlobDesc>& blobMap);

    Result BmUnregister(uint32_t rank, uint16_t mediaType);

    Result ClearResource(uint32_t rank);

    const std::string &Name() const override;

    const mmc_meta_service_config_t &Options() const override;

    const MmcMetaMgrProxyPtr& GetMetaMgrProxy() const override;

private:
    MetaNetServerPtr metaNetServer_;
    MmcMetaMgrProxyPtr metaMgrProxy_;
    MMCMetaBackUpMgrPtr metaBackUpMgrPtr_;

    std::mutex mutex_;
    bool started_ = false;
    std::string name_;
    mmc_meta_service_config_t options_;
    std::unordered_map<uint32_t, std::unordered_set<uint16_t>> rankMediaTypeMap_;
    ock::smem::StorePtr confStore_ = nullptr;
};
inline const std::string &MmcMetaServiceDefault::Name() const
{
    return name_;
}

inline const mmc_meta_service_config_t &MmcMetaServiceDefault::Options() const
{
    return options_;
}

inline const MmcMetaMgrProxyPtr& MmcMetaServiceDefault::GetMetaMgrProxy() const
{
    return metaMgrProxy_;
}

using MmcMetaServiceDefaultPtr = MmcRef<MmcMetaServiceDefault>;
}  // namespace mmc
}  // namespace ock

#endif  // MEM_FABRIC_MMC_META_SERVICE_DEFAULT_H