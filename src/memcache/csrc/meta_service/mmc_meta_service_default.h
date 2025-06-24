/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2026. All rights reserved.
 */
#ifndef MEM_FABRIC_MMC_META_SERVICE_DEFAULT_H
#define MEM_FABRIC_MMC_META_SERVICE_DEFAULT_H

#include "mmc_meta_local_net_server.h"
#include "mmc_meta_common.h"
#include "mmc_meta_service.h"
#include "mmc_def.h"


namespace ock {
namespace mmc {
class MmcMetaServiceDefault : public MmcMetaService {
public:
    explicit MmcMetaServiceDefault(const std::string &name) : name_(name) {}

    Result Start(const mmc_meta_service_config_t &options) override;

    void Stop() override;

    const std::string &Name() const override;

    const mmc_meta_service_config_t &Options() const override;

private:
    MetaNetServerPtr metaNetServer_;

    std::mutex mutex_;
    bool start_ = false;
    std::string name_;
    mmc_meta_service_config_t options_;
};
inline const std::string &MmcMetaServiceDefault::Name() const
{
    return name_;
}

inline const mmc_meta_service_config_t &MmcMetaServiceDefault::Options() const
{
    return options_;
}
}
}

#endif  //MEM_FABRIC_MMC_META_SERVICE_DEFAULT_H
