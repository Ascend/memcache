/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2026. All rights reserved.
 */
#ifndef MEM_FABRIC_MMC_CLIENT_DEFAULT_H
#define MEM_FABRIC_MMC_CLIENT_DEFAULT_H

#include "mmc_common_includes.h"
#include "mmc_meta_net_client.h"
#include "mmc_def.h"

namespace ock {
namespace mmc {
class MmcClientDefault : public MmcReferable {

public:
    explicit MmcClientDefault(const std::string inputName) : name_(inputName){}

    ~MmcClientDefault() override{}

    Result Start(const mmc_client_config_t& config);

    void Stop();

    const std::string &Name() const;

    Result Put(const char *key, mmc_buffer *buf, uint32_t flags);

    Result Get(const char *key, mmc_buffer *buf, uint32_t flags);

private:
    MetaNetClientPtr metaNetClient_;
    std::string name_;
};
using MmcClientDefaultPtr = MmcRef<MmcClientDefault>;
}
}

#endif //MEM_FABRIC_MMC_CLIENT_DEFAULT_H
