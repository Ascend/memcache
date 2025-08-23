/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */
#ifndef MF_HYBRID_HTRACER_DEF_H
#define MF_HYBRID_HTRACER_DEF_H

#include <string>

namespace ock {
namespace mf {
const std::string DEFAULT_DUMP_DIR = "/var/log/mxc/memfabric_hybrid";
int32_t HTracerInit(const std::string& dumpDir = DEFAULT_DUMP_DIR);
void HTracerExit();
}  // namespace mf
}  // namespace ock

#define TRACE_ID(SERVICE_ID_, INNER_ID_) ((SERVICE_ID_) << 16 | ((INNER_ID_) & 0xFFFF))

enum TRACE_MODULE {
    MF_HYBM = 0,
    MF_SMEM,
    MF_MMC,
};

enum MF_HYBM_MOD {
    MF_HYBM_START = TRACE_ID(MF_HYBM, 0U),
    MF_HYBM_COPY_DATA_LOCAL_HOST_TO_GLOBAL_HOST,
};

enum MF_SMEM_MOD {
    MF_SMEM_START = TRACE_ID(MF_SMEM, 0U),
    MF_SMEM_BM_CREATE_ENTITY,
};

enum MF_MMC_MOD {
    MF_MMC_START = TRACE_ID(MF_MMC, 0U),
    MF_MMC_LOCAL_PUT,
    MF_MMC_LOCAL_BATCH_PUT,
    MF_MMC_LOCAL_GET,
    MF_MMC_LOCAL_BATCH_GET,
    MF_MMC_LOCAL_UPDATE,
    MF_MMC_LOCAL_BATCH_UPDATE,

    MF_MMC_META_PUT,
    MF_MMC_META_BATCH_PUT,
    MF_MMC_META_GET,
    MF_MMC_META_BATCH_GET,
};

#endif // MF_HYBRID_HTRACER_DEF_H
