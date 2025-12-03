/*
* Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 * MemFabric_Hybrid is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
*/
#ifndef MEM_FABRIC_MMC_META_PROXY_H
#define MEM_FABRIC_MMC_META_PROXY_H

#include "mmc_msg_client_meta.h"

namespace ock {
namespace mmc {

/* interface */
class MmcMetaMgrProxy : public MmcReferable {
public:
    ~MmcMetaMgrProxy() override = default;

    virtual Result Start(uint64_t defaultTtl, uint16_t evictThresholdHigh, uint16_t evictThresholdLow) = 0;

    virtual void Stop() = 0;

    virtual Result Alloc(const AllocRequest &req, AllocResponse &resp) = 0;

    virtual Result BatchAlloc(const BatchAllocRequest &req, BatchAllocResponse &resp) = 0;

    virtual Result UpdateState(const UpdateRequest &req, Response &resp) = 0;

    virtual Result BatchUpdateState(const BatchUpdateRequest &req, BatchUpdateResponse &resp) = 0;

    virtual Result Get(const GetRequest &req, AllocResponse &resp) = 0;

    virtual Result Remove(const RemoveRequest &req, Response &resp) = 0;

    virtual Result Mount(const std::vector<MmcLocation>& loc, const std::vector<MmcLocalMemlInitInfo>& localMemInitInfo,
                         std::map<std::string, MmcMemBlobDesc>& blobMap) = 0;

    virtual Result Unmount(const MmcLocation &loc) = 0;

    virtual Result ExistKey(const IsExistRequest &req, IsExistResponse &resp) = 0;

    virtual Result BatchRemove(const BatchRemoveRequest &req, BatchRemoveResponse &resp) = 0;

    virtual Result BatchExistKey(const BatchIsExistRequest &req, BatchIsExistResponse &resp) = 0;

    virtual Result BatchGet(const BatchGetRequest &req, BatchAllocResponse &resp) = 0;

    virtual Result Query(const QueryRequest &req, QueryResponse &resp) = 0;

    virtual Result BatchQuery(const BatchQueryRequest &req, BatchQueryResponse &resp) = 0;
};
using MmcMetaMgrProxyPtr = MmcRef<MmcMetaMgrProxy>;
}  // namespace mmc
}  // namespace ock

#endif  // MEM_FABRIC_MMC_META_PROXY_H