/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2026. All rights reserved.
 */
#ifndef MF_HYBRID_HOST_HCOM_HELPER_H
#define MF_HYBRID_HOST_HCOM_HELPER_H

#include <string>
#include <cstdint>

#include "hybm_types.h"

namespace ock {
namespace mf {
namespace transport {
namespace host {

class HostHcomHelper {
public:
    static Result AnalysisNic(const std::string &nic, std::string &protocol, std::string &ipStr, int32_t &port);

private:
    static Result AnalysisNicWithMask(const std::string &nic, std::string &protocol, std::string &ip, int32_t &port);

    static Result SelectLocalIpByIpMask(const std::string &ipStr, const int32_t &mask, std::string &localIp);
};
}
}
}
}
#endif // MF_HYBRID_HOST_HCOM_HELPER_H
