/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */
#ifndef MF_HYBRID_MMC_SMEM_BM_HELPER_H
#define MF_HYBRID_MMC_SMEM_BM_HELPER_H

#include <string>
#include "smem_bm_def.h"

namespace ock {
namespace mmc {
class MmcSmemBmHelper {
public:
    static inline smem_bm_data_op_type TransSmemBmDataOpType(const std::string &type)
    {
        if (type == "sdma") {
            return SMEMB_DATA_OP_SDMA;
        }
        if (type == "roce") {
            return SMEMB_DATA_OP_DEVICE_RDMA;
        }
        if (type == "tcp") {
            return SMEMB_DATA_OP_HOST_TCP;
        }
        return SMEMB_DATA_OP_BUTT;
    }
};

}
}
#endif // MF_HYBRID_MMC_SMEM_BM_HELPER_H
