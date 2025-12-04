/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 * MemCache_Hybrid is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
*/
#ifndef MEM_FABRIC_MMC_NET_LINK_ACC_H
#define MEM_FABRIC_MMC_NET_LINK_ACC_H

#include "mmc_net_engine.h"
#include "mmc_net_common_acc.h"

namespace ock {
namespace mmc {
class NetLinkAcc final : public NetLink {
public:
    NetLinkAcc(int32_t id, const TcpLinkPtr &tcpLink) : id_(id), tcpLink_(tcpLink) {}
    ~NetLinkAcc() override = default;

    int32_t Id() const override;
    const TcpLinkPtr &RealLink() const;

private:
    const int32_t id_;
    const TcpLinkPtr tcpLink_;
};

inline int32_t NetLinkAcc::Id() const
{
    return id_;
}
inline const TcpLinkPtr &NetLinkAcc::RealLink() const
{
    return tcpLink_;
}
}
}

#endif  // MEM_FABRIC_MMC_NET_LINK_ACC_H