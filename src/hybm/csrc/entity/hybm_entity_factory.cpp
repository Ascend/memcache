/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2026. All rights reserved.
 */
#include "hybm_entity_factory.h"
#include "hybm_entity_compose.h"
#include "hybm_entity_default.h"

namespace ock {
namespace mf {
MemEntityPtr MemEntityFactory::GetOrCreateEngine(uint16_t id,  const hybm_options *options, uint32_t flags)
{
    std::lock_guard<std::mutex> guard(enginesMutex_);
    auto iter = engines_.find(id);
    if (iter != engines_.end()) {
        return iter->second;
    }

    MemEntityPtr engine = nullptr;
    switch (options->bmType) {
        case HYBM_TYPE_HBM_DRAM_HOST_INITIATE:
            engine = std::make_shared<HybmEntityCompose>(id);
            break;
        default:
            engine = std::make_shared<MemEntityDefault>(id);
            break;
    }
    /* create new engine */
    engines_.emplace(id, engine);
    enginesFromAddress_.emplace(engine.get(), id);
    return engine;
}

bool MemEntityFactory::RemoveEngine(hybm_entity_t entity)
{
    std::lock_guard<std::mutex> guard(enginesMutex_);
    auto pos = enginesFromAddress_.find(entity);
    if (pos == enginesFromAddress_.end()) {
        return false;
    }

    auto id = pos->second;
    enginesFromAddress_.erase(pos);
    engines_.erase(id);
    return true;
}
}
}