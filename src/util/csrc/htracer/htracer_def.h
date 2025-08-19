/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */
#ifndef MF_HYBRID_HTRACER_DEF_H
#define MF_HYBRID_HTRACER_DEF_H

namespace ock {
namespace htracer {

#define TRACE_ID(SERVICE_ID_, INNER_ID_) ((SERVICE_ID_) << 16 | ((INNER_ID_) & 0xFFFF))

enum HResult {
    RET_OK = 0,
    RET_ERR = -1,
    RET_INVALID_PARAM = -2,
};

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
}
}
#endif // MF_HYBRID_HTRACER_DEF_H
