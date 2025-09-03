/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023. All rights reserved.
 */
#include "hybm_transport_manager.h"

#include "hybm_logger.h"
#include "host_hcom_transport_manager.h"
#include "device_rdma_transport_manager.h"
#include "compose_transport_manager.h"

using namespace ock::mf;
using namespace ock::mf::transport;

std::shared_ptr<TransportManager> TransportManager::Create(TransportType type)
{
    switch (type) {
        case TT_HCOM:
            return host::HcomTransportManager::GetInstance();
        case TT_HCCP:
            return std::make_shared<device::RdmaTransportManager>();
        case TT_COMPOSE:
            return std::make_shared<ComposeTransportManager>();
        default:
            BM_LOG_ERROR("Invalid trans type: " << type);
            return nullptr;
    }
}

const void *TransportManager::GetQpInfo() const
{
    BM_LOG_DEBUG("Not Implement GetQpInfo()");
    return nullptr;
}
