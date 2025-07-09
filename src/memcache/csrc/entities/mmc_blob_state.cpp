/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2026. All rights reserved.
 */
#include "mmc_blob_state.h"

namespace ock {
namespace mmc {
/**
 * @brief tuple struct of transition table of state machine
 */
struct StateTransitionItem {
    BlobState curState;
    Result retCode;
    BlobState nextState;
};

/**
 * @brief State transition table of mem object meta in meta service
 */
StateTransitionItem g_metaStateTransItemTable[]{
    {ALLOCATED, MMC_WRITE_OK, DATA_READY},
    {ALLOCATED, MMC_REMOVE_START, REMOVING},   // data may be transfering at local; can only start remove
    {DATA_READY, MMC_REMOVE_START, REMOVING},  // data may be transfering at local; can only start remove
    {REMOVING, MMC_REMOVE_OK, FINAL},
};

/**
 * @brief State transition table of mem object meta in local service
 */
StateTransitionItem g_localStateTransItemTable[]{
    {ALLOCATED, MMC_WRITE_OK, DATA_READY},
    {ALLOCATED, MMC_WRITE_FAIL, ALLOCATED},
    {DATA_READY, MMC_REMOVE_OK, FINAL},
    {ALLOCATED, MMC_REMOVE_OK, FINAL},
};

StateTransTable BlobStateMachine::GetGlobalTransTable()
{
    StateTransTable table;
    for (size_t i = 0; i < sizeof(g_metaStateTransItemTable) / sizeof(StateTransitionItem); i++) {
        table[g_metaStateTransItemTable[i].curState][g_metaStateTransItemTable[i].retCode] =
            g_metaStateTransItemTable[i].nextState;
    }
    return table;
}

}  // namespace mmc
}  // namespace ock
