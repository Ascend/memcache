/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023. All rights reserved.
 */
#include "hybm_dp_device_rdma.h"

namespace ock {
namespace mf {
DataOpDeviceRDMA::DataOpDeviceRDMA(uint32_t rankId, std::shared_ptr<transport::TransportManager> tm) noexcept
    : rankId_{rankId}, transportManager_{std::move(tm)}
{}

int32_t DataOpDeviceRDMA::Initialize() noexcept
{
    return BM_OK;
}

void DataOpDeviceRDMA::UnInitialize() noexcept {}

int32_t DataOpDeviceRDMA::DataCopy(const void *srcVA, void *destVA, uint64_t length, hybm_data_copy_direction direction,
                                   const ock::mf::ExtOptions &options) noexcept
{
    auto src = (uint64_t)(ptrdiff_t)srcVA;
    auto dest = (uint64_t)(ptrdiff_t)destVA;
    int ret;
    if (options.srcRankId == rankId_) {
        ret = transportManager_->WriteRemote(options.destRankId, src, dest, length);
    } else if (options.destRankId == rankId_) {
        ret = transportManager_->ReadRemote(options.srcRankId, src, dest, length);
    } else {
        BM_LOG_ERROR("local rank:" << rankId_ << ", srcId: " << options.srcRankId << ", dstId: " << options.destRankId);
        return BM_INVALID_PARAM;
    }

    if (ret != BM_OK) {
        BM_LOG_ERROR("transport copy data failed: " << ret);
    }

    return ret;
}

int32_t DataOpDeviceRDMA::DataCopy2d(const void *srcVA, uint64_t spitch, void *destVA, uint64_t dpitch, uint64_t width,
                                     uint64_t height, hybm_data_copy_direction direction,
                                     const ock::mf::ExtOptions &options) noexcept
{
    BM_LOG_ERROR("DataOpDeviceRDMA::DataCopy2d Not Supported!");
    return BM_ERROR;
}

int32_t DataOpDeviceRDMA::DataCopyAsync(const void *srcVA, void *destVA, uint64_t length,
                                        hybm_data_copy_direction direction, const ExtOptions &options) noexcept
{
    BM_LOG_ERROR("DataOpDeviceRDMA::DataCopyAsync Not Supported!");
    return BM_ERROR;
}

int32_t DataOpDeviceRDMA::Wait(int32_t waitId) noexcept
{
    BM_LOG_ERROR("DataOpDeviceRDMA::Wait Not Supported!");
    return BM_ERROR;
}

int32_t DataOpDeviceRDMA::BatchDataCopy(hybm_batch_copy_params &params, hybm_data_copy_direction direction,
                                        const ExtOptions &options) noexcept
{
    BM_LOG_ERROR("DataOpDeviceRDMA::Wait Not Supported!");
    return BM_ERROR;
}
}  // namespace mf
}  // namespace ock