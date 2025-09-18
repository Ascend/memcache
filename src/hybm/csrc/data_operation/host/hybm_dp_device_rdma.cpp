/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023. All rights reserved.
 */
#include "hybm_dp_device_rdma.h"

#include <sys/mman.h>
#include <cstdint>

#include "dl_acl_api.h"
#include "hybm_def.h"
#include "hybm_define.h"
#include "hybm_logger.h"
#include "hybm_types.h"

namespace {
constexpr uint64_t RDMA_SWAP_SPACE_SIZE = 1024 * 1024 * 128;
}

namespace ock {
namespace mf {
DataOpDeviceRDMA::DataOpDeviceRDMA(uint32_t rankId, void *stm, std::shared_ptr<transport::TransportManager> tm) noexcept
    : rankId_{rankId}, stream_(stm), transportManager_{std::move(tm)}
{}

int32_t DataOpDeviceRDMA::Initialize() noexcept
{
    auto ret = AllocSwapMemory();
    if (ret != BM_OK) {
        return ret;
    }
    transport::TransportMemoryRegion input;
    input.addr = reinterpret_cast<uint64_t>(rdmaSwapBaseAddr_);
    input.size = RDMA_SWAP_SPACE_SIZE;
    input.flags = transport::REG_MR_FLAG_DRAM;
    if (transportManager_ != nullptr) {
        auto ret = transportManager_->RegisterMemoryRegion(input);
        if (ret != BM_OK) {
            BM_LOG_ERROR("Failed to register rdma swap memory, size: " << RDMA_SWAP_SPACE_SIZE);
            FreeSwapMemory();
            return BM_MALLOC_FAILED;
        }
    }
    rdmaSwapMemoryAllocator_ = std::make_shared<RbtreeRangePool>((uint8_t *)rdmaSwapBaseAddr_, RDMA_SWAP_SPACE_SIZE);
    return BM_OK;
}

void DataOpDeviceRDMA::UnInitialize() noexcept
{
    FreeSwapMemory();
}

int32_t DataOpDeviceRDMA::AllocSwapMemory()
{
    void *ptr = nullptr;
    auto ret = DlAclApi::AclrtMallocHost(&ptr, RDMA_SWAP_SPACE_SIZE);
    if (ret != 0) {
        BM_LOG_ERROR("Failed to AclrtMallocHost rdma swap memory, size: " << RDMA_SWAP_SPACE_SIZE);
        return BM_MALLOC_FAILED;
    }
    rdmaSwapBaseAddr_ = ptr;
    return BM_OK;
}

void DataOpDeviceRDMA::FreeSwapMemory()
{
    if (rdmaSwapBaseAddr_ != nullptr) {
        auto ret = DlAclApi::AclrtFreeHost(rdmaSwapBaseAddr_);
        if (ret != 0) {
            BM_LOG_ERROR("Failed to AclrtFreeHost swap memory, ret: " << ret);
        }
        rdmaSwapBaseAddr_ = nullptr;
    }
}

DataOpDeviceRDMA::~DataOpDeviceRDMA()
{
    FreeSwapMemory();
}

int32_t DataOpDeviceRDMA::DataCopy(const void *srcVA, void *destVA, uint64_t length, hybm_data_copy_direction direction,
                                   const ock::mf::ExtOptions &options) noexcept
{
    int ret;
    BM_LOG_DEBUG("[DataOpDeviceRDMA::DataCopy] srcVA=" << srcVA << ", destVA=" << destVA << ", length=" << length
                                                       << ", direction=" << direction);
    switch (direction) {
        case HYBM_LOCAL_HOST_TO_GLOBAL_HOST:
            ret = CopyLH2GH(srcVA, destVA, length, options);
            break;
        case HYBM_LOCAL_HOST_TO_GLOBAL_DEVICE:
            ret = CopyLH2GD(srcVA, destVA, length, options);
            break;
        case HYBM_LOCAL_DEVICE_TO_GLOBAL_HOST:
            ret = CopyLD2GH(srcVA, destVA, length, options);
            break;
        case HYBM_LOCAL_DEVICE_TO_GLOBAL_DEVICE:
            ret = CopyLD2GD(srcVA, destVA, length, options);
            break;
        case HYBM_GLOBAL_DEVICE_TO_GLOBAL_DEVICE:
            ret = CopyGD2GD(srcVA, destVA, length, options);
            break;
        case HYBM_GLOBAL_DEVICE_TO_GLOBAL_HOST:
            ret = CopyGD2GH(srcVA, destVA, length, options);
            break;
        case HYBM_GLOBAL_HOST_TO_GLOBAL_DEVICE:
            ret = CopyGH2GD(srcVA, destVA, length, options);
            break;
        case HYBM_GLOBAL_HOST_TO_GLOBAL_HOST:
            ret = CopyGH2GH(srcVA, destVA, length, options);
            break;
        case HYBM_GLOBAL_HOST_TO_LOCAL_HOST:
            ret = CopyGH2LH(srcVA, destVA, length, options);
            break;
        case HYBM_GLOBAL_DEVICE_TO_LOCAL_HOST:
            ret = CopyGD2LH(srcVA, destVA, length, options);
            break;
        case HYBM_GLOBAL_HOST_TO_LOCAL_DEVICE:
            ret = CopyGH2LD(srcVA, destVA, length, options);
            break;
        case HYBM_GLOBAL_DEVICE_TO_LOCAL_DEVICE:
            ret = CopyGD2LD(srcVA, destVA, length, options);
            break;
        default:
            BM_LOG_ERROR("data copy invalid direction: " << direction);
            ret = BM_INVALID_PARAM;
    }
    return ret;
}

int32_t DataOpDeviceRDMA::CopyLH2LH(const void *srcVA, void *destVA, uint64_t length,
                                    const ExtOptions &options) noexcept
{
    BM_LOG_DEBUG("[CopyLH2LH] srcVA=" << srcVA << ", destVA=" << destVA << ", length=" << length);
    auto ret = DlAclApi::AclrtMemcpy(destVA, length, srcVA, length, ACL_MEMCPY_HOST_TO_HOST);
    if (ret != BM_OK) {
        BM_LOG_ERROR("[CopyLH2LH] AclrtMemcpy failed, ret: " << ret);
        return BM_DL_FUNCTION_FAILED;
    }
    return BM_OK;
}
int32_t DataOpDeviceRDMA::CopyLD2LD(const void *srcVA, void *destVA, uint64_t length,
                                    const ExtOptions &options) noexcept
{
    BM_LOG_DEBUG("[CopyLD2LD] srcVA=" << srcVA << ", destVA=" << destVA << ", length=" << length);
    auto ret = DlAclApi::AclrtMemcpy(destVA, length, srcVA, length, ACL_MEMCPY_DEVICE_TO_DEVICE);
    if (ret != BM_OK) {
        BM_LOG_ERROR("[CopyLD2LD] AclrtMemcpy failed, ret: " << ret);
        return BM_DL_FUNCTION_FAILED;
    }
    return BM_OK;
}

int32_t DataOpDeviceRDMA::CopyLH2LD(const void *srcVA, void *destVA, uint64_t length,
                                    const ExtOptions &options) noexcept
{
    BM_LOG_DEBUG("[CopyLH2LD] srcVA=" << srcVA << ", destVA=" << destVA << ", length=" << length);
    auto ret = DlAclApi::AclrtMemcpy(destVA, length, srcVA, length, ACL_MEMCPY_HOST_TO_DEVICE);
    if (ret != BM_OK) {
        BM_LOG_ERROR("[CopyLH2LD] AclrtMemcpy failed, ret: " << ret);
        return BM_DL_FUNCTION_FAILED;
    }
    return BM_OK;
}

int32_t DataOpDeviceRDMA::CopyLD2LH(const void *srcVA, void *destVA, uint64_t length,
                                    const ExtOptions &options) noexcept
{
    BM_LOG_DEBUG("[CopyLD2LH] srcVA=" << srcVA << ", destVA=" << destVA << ", length=" << length);
    auto ret = DlAclApi::AclrtMemcpy(destVA, length, srcVA, length, ACL_MEMCPY_DEVICE_TO_HOST);
    if (ret != BM_OK) {
        BM_LOG_ERROR("[CopyLD2LH] AclrtMemcpy failed, ret: " << ret);
        return BM_DL_FUNCTION_FAILED;
    }
    return BM_OK;
}

int32_t DataOpDeviceRDMA::CopyLH2GH(const void *srcVA, void *destVA, uint64_t length,
                                    const ExtOptions &options) noexcept
{
    BM_LOG_DEBUG("[CopyLH2GH] srcVA=" << srcVA << ", destVA=" << destVA << ", length=" << length);
    int32_t ret;
    if (options.destRankId == rankId_) {
        ret = CopyLH2LH(srcVA, destVA, length, options);
        BM_ASSERT_LOG_AND_RETURN(ret == BM_OK, "[CopyLH2GH] Failed to copy src to dest", ret);
    } else {
        auto tmpRdmaMemory = rdmaSwapMemoryAllocator_->Allocate(length);
        auto tmpHost = tmpRdmaMemory.Address();
        BM_ASSERT_LOG_AND_RETURN(tmpHost != nullptr, "[CopyLH2GH] Failed to malloc temp buffer", BM_MALLOC_FAILED);
        ret = CopyLH2LH(srcVA, tmpHost, length, options);
        BM_ASSERT_LOG_AND_RETURN(ret == BM_OK, "[CopyLH2GH] Failed to copy src to tmp", ret);
        ret = CopyRDMA(tmpHost, destVA, length, options);
        BM_ASSERT_LOG_AND_RETURN(ret == BM_OK, "[CopyLH2GH] Failed to copy tmp to dest", ret);
    }
    return ret;
}

int32_t DataOpDeviceRDMA::CopyLH2GD(const void *srcVA, void *destVA, uint64_t length,
                                    const ExtOptions &options) noexcept
{
    BM_LOG_DEBUG("[CopyLH2GD] srcVA=" << srcVA << ", destVA=" << destVA << ", length=" << length);
    int32_t ret;
    if (options.destRankId == rankId_) {
        ret = CopyLH2LD(srcVA, destVA, length, options);
        BM_ASSERT_LOG_AND_RETURN(ret == BM_OK, "[CopyLH2GD] Failed to copy src to dest", ret);
    } else {
        auto tmpRdmaMemory = rdmaSwapMemoryAllocator_->Allocate(length);
        auto tmpHost = tmpRdmaMemory.Address();
        BM_ASSERT_LOG_AND_RETURN(tmpHost != nullptr, "[CopyLH2GD] Failed to malloc temp buffer", BM_MALLOC_FAILED);
        ret = CopyLH2LH(srcVA, tmpHost, length, options);
        BM_ASSERT_LOG_AND_RETURN(ret == BM_OK, "[CopyLH2GD] Failed to copy src to tmp", ret);
        ret = CopyRDMA(tmpHost, destVA, length, options);
        BM_ASSERT_LOG_AND_RETURN(ret == BM_OK, "[CopyLH2GD] Failed to copy tmp to dest", ret);
    }
    return ret;
}

int32_t DataOpDeviceRDMA::CopyLD2GH(const void *srcVA, void *destVA, uint64_t length,
                                    const ExtOptions &options) noexcept
{
    BM_LOG_DEBUG("[CopyLD2GH] srcVA=" << srcVA << ", destVA=" << destVA << ", length=" << length);
    int32_t ret;
    if (options.destRankId == rankId_) {
        ret = CopyLD2LH(srcVA, destVA, length, options);
        BM_ASSERT_LOG_AND_RETURN(ret == BM_OK, "[CopyLD2GH] Failed to copy src to dest", ret);
    } else {
        auto tmpRdmaMemory = rdmaSwapMemoryAllocator_->Allocate(length);
        auto tmpHost = tmpRdmaMemory.Address();
        BM_ASSERT_LOG_AND_RETURN(tmpHost != nullptr, "[CopyLD2GH] Failed to malloc temp buffer", BM_MALLOC_FAILED);
        ret = CopyLD2LH(srcVA, tmpHost, length, options);
        BM_ASSERT_LOG_AND_RETURN(ret == BM_OK, "[CopyLD2GH] Failed to copy src to tmp", ret);
        ret = CopyRDMA(tmpHost, destVA, length, options);
        BM_ASSERT_LOG_AND_RETURN(ret == BM_OK, "[CopyLD2GH] Failed to copy tmp to dest", ret);
    }
    return ret;
}

int32_t DataOpDeviceRDMA::CopyLD2GD(const void *srcVA, void *destVA, uint64_t length,
                                    const ExtOptions &options) noexcept
{
    BM_LOG_DEBUG("[CopyLD2GD] srcVA=" << srcVA << ", destVA=" << destVA << ", length=" << length);
    int32_t ret;
    if (options.destRankId == rankId_) {
        ret = CopyLD2LD(srcVA, destVA, length, options);
        BM_ASSERT_LOG_AND_RETURN(ret == BM_OK, "[CopyLD2GD] Failed to copy src to dest", ret);
    } else {
        auto tmpRdmaMemory = rdmaSwapMemoryAllocator_->Allocate(length);
        auto tmpHost = tmpRdmaMemory.Address();
        BM_ASSERT_LOG_AND_RETURN(tmpHost != nullptr, "[CopyLD2GD] Failed to malloc temp buffer", BM_MALLOC_FAILED);
        ret = CopyLD2LH(srcVA, tmpHost, length, options);
        BM_ASSERT_LOG_AND_RETURN(ret == BM_OK, "[CopyLD2GD] Failed to copy src to tmp", ret);
        ret = CopyRDMA(tmpHost, destVA, length, options);
        BM_ASSERT_LOG_AND_RETURN(ret == BM_OK, "[CopyLD2GD] Failed to copy tmp to dest", ret);
    }
    return ret;
}

int32_t DataOpDeviceRDMA::CopyRDMA(const void *srcVA, void *destVA, uint64_t length,
                                   const ock::mf::ExtOptions &options) noexcept
{
    BM_LOG_DEBUG("[CopyRDMA] srcVA=" << srcVA << ", destVA=" << destVA << ", length=" << length);
    auto src = (uint64_t)(ptrdiff_t)srcVA;
    auto dest = (uint64_t)(ptrdiff_t)destVA;
    int ret;
    if (options.srcRankId == rankId_) {
        ret = transportManager_->WriteRemote(options.destRankId, src, dest, length);
        BM_ASSERT_LOG_AND_RETURN(ret == BM_OK, "[CopyRDMA] Failed to write src to dest", ret);
    } else if (options.destRankId == rankId_) {
        ret = transportManager_->ReadRemote(options.srcRankId, dest, src, length);
        BM_ASSERT_LOG_AND_RETURN(ret == BM_OK, "[CopyRDMA] Failed to read src to dest", ret);
    } else {
        BM_LOG_ERROR("[CopyRDMA] invalid param, local rank:" << rankId_ << ", srcId: " << options.srcRankId
                                                             << ", dstId: " << options.destRankId);
        return BM_INVALID_PARAM;
    }
    return ret;
}

int32_t DataOpDeviceRDMA::CopyGH2GH(const void *srcVA, void *destVA, uint64_t length,
                                    const ock::mf::ExtOptions &options) noexcept
{
    BM_LOG_DEBUG("[CopyGH2GH] srcVA=" << srcVA << ", destVA=" << destVA << ", length=" << length);
    int ret;
    if (options.srcRankId == rankId_ && options.destRankId == rankId_) {
        ret = CopyLH2LH(srcVA, destVA, length, options);
        BM_ASSERT_LOG_AND_RETURN(ret == BM_OK, "[CopyGH2GH] Failed to copy src to dest", ret);
    } else {
        ret = CopyRDMA(srcVA, destVA, length, options);
        BM_ASSERT_LOG_AND_RETURN(ret == BM_OK, "[CopyGH2GH] Failed to rdma src to dest", ret);
    }
    return ret;
}

int32_t DataOpDeviceRDMA::CopyGD2GH(const void *srcVA, void *destVA, uint64_t length,
                                    const ock::mf::ExtOptions &options) noexcept
{
    BM_LOG_DEBUG("[CopyGD2GH] srcVA=" << srcVA << ", destVA=" << destVA << ", length=" << length);
    int ret;
    if (options.srcRankId == rankId_ && options.destRankId == rankId_) {
        ret = CopyLD2LH(srcVA, destVA, length, options);
        BM_ASSERT_LOG_AND_RETURN(ret == BM_OK, "[CopyGD2GH] Failed to copy src to dest", ret);
    } else {
        ret = CopyRDMA(srcVA, destVA, length, options);
        BM_ASSERT_LOG_AND_RETURN(ret == BM_OK, "[CopyGD2GH] Failed to rdma src to dest", ret);
    }
    return ret;
}
int32_t DataOpDeviceRDMA::CopyGH2GD(const void *srcVA, void *destVA, uint64_t length,
                                    const ock::mf::ExtOptions &options) noexcept
{
    BM_LOG_DEBUG("[CopyGH2GD] srcVA=" << srcVA << ", destVA=" << destVA << ", length=" << length);
    int ret;
    if (options.srcRankId == rankId_ && options.destRankId == rankId_) {
        ret = CopyLH2LD(srcVA, destVA, length, options);
        BM_ASSERT_LOG_AND_RETURN(ret == BM_OK, "[CopyGH2GD] Failed to copy src to dest", ret);
    } else {
        ret = CopyRDMA(srcVA, destVA, length, options);
        BM_ASSERT_LOG_AND_RETURN(ret == BM_OK, "[CopyGH2GD] Failed to rdma src to dest", ret);
    }
    return ret;
}
int32_t DataOpDeviceRDMA::CopyGD2GD(const void *srcVA, void *destVA, uint64_t length,
                                    const ock::mf::ExtOptions &options) noexcept
{
    BM_LOG_DEBUG("[CopyGD2GD] srcVA=" << srcVA << ", destVA=" << destVA << ", length=" << length);
    int ret;
    if (options.srcRankId == rankId_ && options.destRankId == rankId_) {
        ret = CopyLD2LD(srcVA, destVA, length, options);
        BM_ASSERT_LOG_AND_RETURN(ret == BM_OK, "[CopyGD2GD] Failed to copy src to dest", ret);
    } else {
        ret = CopyRDMA(srcVA, destVA, length, options);
        BM_ASSERT_LOG_AND_RETURN(ret == BM_OK, "[CopyGD2GD] Failed to rdma src to dest", ret);
    }
    return ret;
}

int32_t DataOpDeviceRDMA::CopyGH2LH(const void *srcVA, void *destVA, uint64_t length,
                                    const ExtOptions &options) noexcept
{
    BM_LOG_DEBUG("[CopyGH2LH] srcVA=" << srcVA << ", destVA=" << destVA << ", length=" << length);
    int32_t ret;
    if (options.srcRankId == rankId_) {
        ret = CopyLH2LH(srcVA, destVA, length, options);
        BM_ASSERT_LOG_AND_RETURN(ret == BM_OK, "[CopyGH2LH] Failed to copy src to dest", ret);
    } else {
        auto tmpRdmaMemory = rdmaSwapMemoryAllocator_->Allocate(length);
        auto tmpHost = tmpRdmaMemory.Address();
        BM_ASSERT_LOG_AND_RETURN(tmpHost != nullptr, "[CopyGH2LH] Failed to malloc temp buffer", BM_MALLOC_FAILED);
        ret = CopyRDMA(srcVA, tmpHost, length, options);
        BM_ASSERT_LOG_AND_RETURN(ret == BM_OK, "[CopyGH2LH] Failed to copy src to tmp", ret);
        ret = CopyLH2LH(tmpHost, destVA, length, options);
        BM_ASSERT_LOG_AND_RETURN(ret == BM_OK, "[CopyGH2LH] Failed to copy tmp to dest", ret);
    }
    return ret;
}

int32_t DataOpDeviceRDMA::CopyGD2LH(const void *srcVA, void *destVA, uint64_t length,
                                    const ExtOptions &options) noexcept
{
    BM_LOG_DEBUG("[CopyGD2LH] srcVA=" << srcVA << ", destVA=" << destVA << ", length=" << length);
    int32_t ret;
    if (options.srcRankId == rankId_) {
        ret = CopyLD2LH(srcVA, destVA, length, options);
        BM_ASSERT_LOG_AND_RETURN(ret == BM_OK, "[CopyGD2LH] Failed to copy src to dest", ret);
    } else {
        auto tmpRdmaMemory = rdmaSwapMemoryAllocator_->Allocate(length);
        auto tmpHost = tmpRdmaMemory.Address();
        BM_ASSERT_LOG_AND_RETURN(tmpHost != nullptr, "[CopyGD2LH] Failed to malloc temp buffer", BM_MALLOC_FAILED);
        ret = CopyRDMA(srcVA, tmpHost, length, options);
        BM_ASSERT_LOG_AND_RETURN(ret == BM_OK, "[CopyGD2LH] Failed to copy src to tmp", ret);
        ret = CopyLH2LH(tmpHost, destVA, length, options);
        BM_ASSERT_LOG_AND_RETURN(ret == BM_OK, "[CopyGD2LH] Failed to copy tmp to dest", ret);
    }
    return ret;
}

int32_t DataOpDeviceRDMA::CopyGH2LD(const void *srcVA, void *destVA, uint64_t length,
                                    const ExtOptions &options) noexcept
{
    BM_LOG_DEBUG("[CopyGH2LD] srcVA=" << srcVA << ", destVA=" << destVA << ", length=" << length);
    int32_t ret;
    if (options.srcRankId == rankId_) {
        ret = CopyLH2LD(srcVA, destVA, length, options);
        BM_ASSERT_LOG_AND_RETURN(ret == BM_OK, "[CopyGH2LD] Failed to copy src to dest", ret);
    } else {
        auto tmpRdmaMemory = rdmaSwapMemoryAllocator_->Allocate(length);
        auto tmpHost = tmpRdmaMemory.Address();
        BM_ASSERT_LOG_AND_RETURN(tmpHost != nullptr, "[CopyGH2LD] Failed to malloc temp buffer", BM_MALLOC_FAILED);
        ret = CopyRDMA(srcVA, tmpHost, length, options);
        BM_ASSERT_LOG_AND_RETURN(ret == BM_OK, "[CopyGH2LD] Failed to copy src to tmp", ret);
        ret = CopyLH2LD(tmpHost, destVA, length, options);
        BM_ASSERT_LOG_AND_RETURN(ret == BM_OK, "[CopyGH2LD] Failed to copy tmp to dest", ret);
    }
    return ret;
}

int32_t DataOpDeviceRDMA::CopyGD2LD(const void *srcVA, void *destVA, uint64_t length,
                                    const ExtOptions &options) noexcept
{
    BM_LOG_DEBUG("[CopyGD2LD] srcVA=" << srcVA << ", destVA=" << destVA << ", length=" << length);
    int32_t ret;
    if (options.srcRankId == rankId_) {
        ret = CopyLD2LD(srcVA, destVA, length, options);
        BM_ASSERT_LOG_AND_RETURN(ret == BM_OK, "[CopyGD2LD] Failed to copy src to dest", ret);
    } else {
        auto tmpRdmaMemory = rdmaSwapMemoryAllocator_->Allocate(length);
        auto tmpHost = tmpRdmaMemory.Address();
        BM_ASSERT_LOG_AND_RETURN(tmpHost != nullptr, "[CopyGD2LD] Failed to malloc temp buffer", BM_MALLOC_FAILED);
        ret = CopyRDMA(srcVA, tmpHost, length, options);
        BM_ASSERT_LOG_AND_RETURN(ret == BM_OK, "[CopyGD2LD] Failed to copy src to tmp", ret);
        ret = CopyLH2LD(tmpHost, destVA, length, options);
        BM_ASSERT_LOG_AND_RETURN(ret == BM_OK, "[CopyGD2LD] Failed to copy tmp to dest", ret);
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
    // Since DataOpDeviceRDMA::DataCopyAsync is not supported, Wait should do nothing for now.
    return BM_OK;
}

int32_t DataOpDeviceRDMA::BatchDataCopy(hybm_batch_copy_params &params, hybm_data_copy_direction direction,
                                        const ExtOptions &options) noexcept
{
    BM_LOG_ERROR("DataOpDeviceRDMA::Wait Not Supported!");
    return BM_ERROR;
}
} // namespace mf
} // namespace ock
