/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
 */
#ifndef MF_HYBRID_HYBM_OP_DRAM_SDMA_H
#define MF_HYBRID_HYBM_OP_DRAM_SDMA_H

#include "hybm_data_operator.h"
#include "hybm_stream.h"

namespace ock {
namespace mf {
class HostDramSdma : public DataOperator {
public:
    ~HostDramSdma() override;

    int32_t Initialized() noexcept override;
    void UnInitialized() noexcept override;

    int32_t DataCopy(const void *srcVA, void *destVA, uint64_t length, hybm_data_copy_direction direction,
                     const ExtOptions &options) noexcept override;
    int32_t DataCopy2d(const void *srcVA, uint64_t spitch, void *destVA, uint64_t dpitch, uint64_t width,
                       uint64_t height, hybm_data_copy_direction direction,
                       const ExtOptions &options) noexcept override;
    int32_t DataCopyAsync(const void *srcVA, void *destVA, uint64_t length, hybm_data_copy_direction direction,
                          const ExtOptions &options) noexcept override;

    int32_t Wait(int32_t waitId) noexcept override;

private:
    int CopyGH2GH(void *destVA, const void *srcVA, size_t count) noexcept;

private:
    bool inited_ = false;
    HybmStreamPtr stream_ = nullptr;
};
} // namespace mf
} // namespace ock

#endif // MF_HYBRID_HYBM_OP_DRAM_SDMA_H
