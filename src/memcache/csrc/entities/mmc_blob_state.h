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
#ifndef MEMFABRIC_HYBRID_MMC_BLOB_STATE_H
#define MEMFABRIC_HYBRID_MMC_BLOB_STATE_H

#include <utility>

#include "functional"
#include "mmc_common_includes.h"
#include "mmc_meta_lease_manager.h"
namespace ock {
namespace mmc {

/**
* @brief State of blob
*/
using BlobLeaseFunction = std::function<Result(MmcMetaLeaseManager &leaseMgr, uint32_t rankId, uint32_t requestId)>;

/**
 state machine:
NONE -----> ALLOCATED -----------
                |               |
                |               |
                |               |
            READABLE -----> REMOVING -----> NONE
*/
enum BlobState : uint8_t {
    ALLOCATED,
    READABLE,
    REMOVING,
    NONE,
};

inline std::ostream& operator<<(std::ostream& os, BlobState type)
{
    switch (type) {
        case ALLOCATED: os << "ALLOCATED"; break;
        case READABLE: os << "READABLE"; break;
        case REMOVING: os << "REMOVING"; break;
        default: os << "NONE"; break;
    }
    return os;
}

struct BlobStateAction {
    BlobState state_ = NONE;
    BlobLeaseFunction action_ = nullptr;
    BlobStateAction(BlobState state, BlobLeaseFunction action) : state_(state), action_(std::move(action)) {}
    BlobStateAction() = default;
};

/**
 * @brief Block action result, which a part of transition table
 */
enum BlobActionResult : uint8_t {
    MMC_ALLOCATED_OK,  // alloc complete

    MMC_WRITE_OK,
    MMC_WRITE_FAIL,

    MMC_READ_START,
    MMC_READ_FINISH,

    MMC_REMOVE_START
};

using StateTransTable = std::unordered_map<BlobState, std::unordered_map<Result, BlobStateAction>>;

class BlobStateMachine : public MmcReferable {
public:
    static StateTransTable GetGlobalTransTable();
};

}  // namespace mmc
}  // namespace ock

#endif