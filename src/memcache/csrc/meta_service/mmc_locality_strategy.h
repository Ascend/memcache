/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2026. All rights reserved.
 */
#ifndef MEM_FABRIC_MMC_LOCALITY_STRATEGY_H
#define MEM_FABRIC_MMC_LOCALITY_STRATEGY_H

#include "mmc_mem_blob.h"
#include <vector>

namespace ock {
namespace mmc {

struct AllocRequest {
    uint64_t blobSize_{0};
    uint32_t numBlobs_{0};
    uint16_t mediaType_{0};
    uint32_t priorityRank_{0};
    uint32_t flags_{0};
};

using MmcLocation = std::pair<uint32_t, uint16_t>;

struct MmcLocalMemCurInfo {
    uint64_t capacity_;
};

using MmcMemPoolCurInfo = std::map<MmcLocation, MmcLocalMemCurInfo>;

class MmcLocalityStrategy : public MmcReferable {
public:
    static Result ArrangeLocality(const MmcMemPoolCurInfo &memPoolCurInfo, const AllocRequest &allocReq,
                                  std::vector<MmcLocation> &locations)
    {
        return MMC_OK;
    };
};

} // namespace mmc
} // namespace ock

#endif // MEM_FABRIC_MMC_LOCALITY_STRATEGY_H