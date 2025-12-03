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
#ifndef MEM_FABRIC_MMC_LOCAL_SERVICE_H
#define MEM_FABRIC_MMC_LOCAL_SERVICE_H

#include "mmc_common_includes.h"
#include "mmc_def.h"

namespace ock {
namespace mmc {
class MmcLocalService : public MmcReferable {
public:
    virtual Result Start(const mmc_local_service_config_t &config) = 0;

    virtual void Stop() = 0;

    virtual const std::string &Name() const = 0;

    virtual const mmc_local_service_config_t &Options() const = 0;
};
} // namespace mmc
} // namespace ock

#endif // MEM_FABRIC_MMC_LOCAL_SERVICE_H