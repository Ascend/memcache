/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2026. All rights reserved.
 */
#include "hybm_logger.h"
#include "../../under_api/dl_acl_api.h"
#include "htracer.h"
#include "hybm_data_operator_sdma.h"

namespace ock {
namespace mf {

HostDataOpSDMA::HostDataOpSDMA(void *stm) noexcept : stream_{stm} {}

int32_t HostDataOpSDMA::DataCopy(const void *srcVA, void *destVA, uint64_t length, hybm_data_copy_direction direction,
                                 const ExtOptions &options) noexcept
{
    int ret;
    switch (direction) {
        case HYBM_LOCAL_DEVICE_TO_GLOBAL_DEVICE: {
            TP_DELAY_BEGIN(TP_HYBM_SDMA_LD_TO_GD);
            ret = CopyLD2GD(destVA, srcVA, length, options.stream);
            TP_DELAY_END(TP_HYBM_SDMA_LD_TO_GD, ret);
            break;
        }
        case HYBM_GLOBAL_DEVICE_TO_LOCAL_DEVICE: {
            TP_DELAY_BEGIN(TP_HYBM_SDMA_GD_TO_LD);
            ret = CopyGD2LD(destVA, srcVA, length, options.stream);
            TP_DELAY_END(TP_HYBM_SDMA_GD_TO_LD, ret);
            break;
        }
        case HYBM_LOCAL_HOST_TO_GLOBAL_DEVICE: {
            TP_DELAY_BEGIN(TP_HYBM_SDMA_LH_TO_GD);
            ret = CopyLH2GD(destVA, srcVA, length, options.stream);
            TP_DELAY_END(TP_HYBM_SDMA_LH_TO_GD, ret);
            break;
        }
        case HYBM_GLOBAL_DEVICE_TO_LOCAL_HOST: {
            TP_DELAY_BEGIN(TP_HYBM_SDMA_GD_TO_LH);
            ret = CopyGD2LH(destVA, srcVA, length, options.stream);
            TP_DELAY_END(TP_HYBM_SDMA_GD_TO_LH, ret);
            break;
        }
        case HYBM_GLOBAL_DEVICE_TO_GLOBAL_DEVICE: {
            TP_DELAY_BEGIN(TP_HYBM_SDMA_GD_TO_GD);
            ret = CopyGD2GD(destVA, srcVA, length, options.stream);
            TP_DELAY_END(TP_HYBM_SDMA_GD_TO_GD, ret);
            break;
        }
        case HYBM_GLOBAL_DEVICE_TO_GLOBAL_HOST: {
            TP_DELAY_BEGIN(TP_HYBM_SDMA_GD_TO_GH);
            ret = CopyGD2GH(destVA, srcVA, length, options.stream);
            TP_DELAY_END(TP_HYBM_SDMA_GD_TO_GH, ret);
            break;
        }
        case HYBM_GLOBAL_HOST_TO_GLOBAL_DEVICE: {
            TP_DELAY_BEGIN(TP_HYBM_SDMA_GH_TO_GD);
            ret = CopyGH2GD(destVA, srcVA, length, options.stream);
            TP_DELAY_END(TP_HYBM_SDMA_GH_TO_GD, ret);
            break;
        }
        case HYBM_GLOBAL_HOST_TO_GLOBAL_HOST: {
            TP_DELAY_BEGIN(TP_HYBM_SDMA_GH_TO_GD);
            ret = CopyGH2GH(destVA, srcVA, length, options.stream);
            TP_DELAY_END(TP_HYBM_SDMA_GH_TO_GD, ret);
            break;
        }
        case HYBM_LOCAL_DEVICE_TO_GLOBAL_HOST: {
            TP_DELAY_BEGIN(TP_HYBM_SDMA_LD_TO_GH);
            ret = CopyLD2GH(destVA, srcVA, length, options.stream);
            TP_DELAY_END(TP_HYBM_SDMA_LD_TO_GH, ret);
            break;
        }
        case HYBM_LOCAL_HOST_TO_GLOBAL_HOST: {
            TP_DELAY_BEGIN(TP_HYBM_SDMA_LH_TO_GH);
            ret = CopyLH2GH(destVA, srcVA, length, options.stream);
            TP_DELAY_END(TP_HYBM_SDMA_LH_TO_GH, ret);
            break;
        }
        case HYBM_GLOBAL_HOST_TO_LOCAL_HOST: {
            TP_DELAY_BEGIN(TP_HYBM_SDMA_GH_TO_LH);
            ret = CopyGH2LH(destVA, srcVA, length, options.stream);
            TP_DELAY_END(TP_HYBM_SDMA_GH_TO_LH, ret);
            break;
        }
        case HYBM_GLOBAL_HOST_TO_LOCAL_DEVICE: {
            TP_DELAY_BEGIN(TP_HYBM_SDMA_GH_TO_LD);
            ret = CopyGH2LD(destVA, srcVA, length, options.stream);
            TP_DELAY_END(TP_HYBM_SDMA_GH_TO_LD, ret);
            break;
        }
        default:
            BM_LOG_ERROR("data copy invalid direction: " << direction);
            ret = BM_INVALID_PARAM;
    }
    return ret;
}

int HostDataOpSDMA::CopyLH2GD(void *gvaAddr, const void *hostAddr, size_t count, void *stream) noexcept
{
    void *copyDevice;
    auto ret = DlAclApi::AclrtMalloc(&copyDevice, count, 0);
    if (ret != 0) {
        BM_LOG_ERROR("allocate temp copy memory on local device failed: " << ret);
        return BM_DL_FUNCTION_FAILED;
    }

    ret = DlAclApi::AclrtMemcpy(copyDevice, count, hostAddr, count, ACL_MEMCPY_HOST_TO_DEVICE);
    if (ret != 0) {
        BM_LOG_ERROR("copy host data to temp copy memory on local device failed: " << ret);
        DlAclApi::AclrtFree(copyDevice);
        return BM_DL_FUNCTION_FAILED;
    }

    auto result = CopyLD2GD(gvaAddr, copyDevice, count, stream);
    if (result != BM_OK) {
        DlAclApi::AclrtFree(copyDevice);
        return result;
    }

    DlAclApi::AclrtFree(copyDevice);
    return BM_OK;
}

int HostDataOpSDMA::CopyGD2GD(void* gvaAddr, const void* deviceAddr, size_t count, void* stream) noexcept
{
    return CopyLD2GD(gvaAddr, deviceAddr, count, stream);
}

int HostDataOpSDMA::CopyLD2GD(void *gvaAddr, const void *deviceAddr, size_t count, void *stream) noexcept
{
    void *st = stream_;
    if (stream != nullptr) {
        st = stream;
    }

    auto ret = DlAclApi::AclrtMemcpyAsync(gvaAddr, count, deviceAddr, count, ACL_MEMCPY_DEVICE_TO_DEVICE, st);
    if (ret != 0) {
        BM_LOG_ERROR("copy memory on local device to GVA failed: " << ret << " stream:" << st);
        return BM_DL_FUNCTION_FAILED;
    }

    ret = DlAclApi::AclrtSynchronizeStream(st);
    if (ret != 0) {
        BM_LOG_ERROR("aclrtSynchronizeStream failed: " << ret << " stream:" << st);
        return BM_DL_FUNCTION_FAILED;
    }

    return BM_OK;
}

int HostDataOpSDMA::CopyGD2LD(void *deviceAddr, const void *gvaAddr, size_t count, void *stream) noexcept
{
    void *st = stream_;
    if (stream != nullptr) {
        st = stream;
    }

    auto ret = DlAclApi::AclrtMemcpyAsync(deviceAddr, count, gvaAddr, count, ACL_MEMCPY_DEVICE_TO_DEVICE, st);
    if (ret != 0) {
        BM_LOG_ERROR("copy memory on GVA to local device failed: " << ret << " stream:" << st);
        return BM_DL_FUNCTION_FAILED;
    }

    ret = DlAclApi::AclrtSynchronizeStream(st);
    if (ret != 0) {
        BM_LOG_ERROR("aclrtSynchronizeStream failed: " << ret << " stream:" << st);
        return BM_DL_FUNCTION_FAILED;
    }

    return BM_OK;
}

int HostDataOpSDMA::CopyGD2LH(void *hostAddr, const void *gvaAddr, size_t count, void *stream) noexcept
{
    void *copyDevice;
    auto ret = DlAclApi::AclrtMalloc(&copyDevice, count, 0);
    if (ret != 0) {
        BM_LOG_ERROR("allocate temp copy memory on local device failed: " << ret);
        return BM_DL_FUNCTION_FAILED;
    }

    auto result = CopyGD2LD(copyDevice, gvaAddr, count, stream);
    if (result != BM_OK) {
        DlAclApi::AclrtFree(copyDevice);
        return result;
    }

    ret = DlAclApi::AclrtMemcpy(hostAddr, count, copyDevice, count, ACL_MEMCPY_DEVICE_TO_HOST);
    if (ret != 0) {
        BM_LOG_ERROR("copy data on temp DEVICE to GVA failed: " << ret);
        DlAclApi::AclrtFree(copyDevice);
        return BM_DL_FUNCTION_FAILED;
    }

    DlAclApi::AclrtFree(copyDevice);
    return BM_OK;
}

int HostDataOpSDMA::CopyLH2GD2d(void* gvaAddr, uint64_t dpitch, const void* hostAddr, uint64_t spitch, size_t width,
                                uint64_t height, void* stream) noexcept
{
    void *copyDevice;
    auto ret = DlAclApi::AclrtMalloc(&copyDevice, width * height, 0);
    if (ret != 0) {
        BM_LOG_ERROR("allocate temp copy memory on local device failed: " << ret);
        return BM_DL_FUNCTION_FAILED;
    }

    ret = DlAclApi::AclrtMemcpy2d(copyDevice, width, hostAddr, spitch, width, height, ACL_MEMCPY_HOST_TO_DEVICE);
    if (ret != 0) {
        BM_LOG_ERROR("copy2d host data to temp copy memory on local device failed: " << ret
                     << " spitch: " << spitch << " dpitch: " << width << " width: " << width << " height:" << height);
        DlAclApi::AclrtFree(copyDevice);
        return BM_DL_FUNCTION_FAILED;
    }

    auto result = CopyLD2GD2d(gvaAddr, dpitch, copyDevice, width, width, height, stream);
    if (result != BM_OK) {
        DlAclApi::AclrtFree(copyDevice);
        return result;
    }

    DlAclApi::AclrtFree(copyDevice);
    return BM_OK;
}

int HostDataOpSDMA::CopyLD2GD2d(void *gvaAddr, uint64_t dpitch, const void *deviceAddr, uint64_t spitch,
                   size_t width, uint64_t height, void *stream) noexcept
{
    void *st = stream_;
    if (stream != nullptr) {
        st = stream;
    }

    int ret = BM_OK;
    for (uint64_t i = 0; i < height; ++i) {
        void *dstAddr = reinterpret_cast<void *>((uint64_t) gvaAddr + i * dpitch);
        void *srcAddr = reinterpret_cast<void *>((uint64_t) deviceAddr + i * spitch);
        auto asyncRet = DlAclApi::AclrtMemcpyAsync(dstAddr, width, srcAddr, width,
                                                   ACL_MEMCPY_DEVICE_TO_DEVICE, st);
        if (asyncRet != 0) {
            BM_LOG_ERROR("copy2d memory on gva to device failed:: " << asyncRet
                         << " dpitch: " << dpitch << " spitch: " << spitch << " width: " << width
                         << " height:" << height << " stream:" << st);
            ret = asyncRet;
        }
    }

    int syncRet  = DlAclApi::AclrtSynchronizeStream(st);
    if (syncRet != 0) {
        BM_LOG_ERROR("aclrtSynchronizeStream failed: " << syncRet << " stream:" << st);
        ret = syncRet;
    }
    return ret;
}

int HostDataOpSDMA::CopyGD2LH2d(void *hostAddr, uint64_t dpitch, const void *gvaAddr, uint64_t spitch,
                 size_t width, uint64_t height, void *stream) noexcept
{
    void *copyDevice;
    auto ret = DlAclApi::AclrtMalloc(&copyDevice, width * height, 0);
    if (ret != 0) {
        BM_LOG_ERROR("allocate temp copy memory on local device failed: " << ret);
        return BM_DL_FUNCTION_FAILED;
    }

    auto result = CopyGD2LD2d(copyDevice, width, gvaAddr, spitch, width, height, stream);
    if (result != BM_OK) {
        DlAclApi::AclrtFree(copyDevice);
        return result;
    }

    ret = DlAclApi::AclrtMemcpy2d(hostAddr, dpitch, copyDevice, width, width, height, ACL_MEMCPY_DEVICE_TO_HOST);
    if (ret != 0) {
        BM_LOG_ERROR("copy data on temp DEVICE to GVA failed: " << ret
                     << " spitch: " << spitch << " width: " << width << " height:" << height);
        DlAclApi::AclrtFree(copyDevice);
        return BM_DL_FUNCTION_FAILED;
    }

    DlAclApi::AclrtFree(copyDevice);
    return BM_OK;
}

int HostDataOpSDMA::CopyGD2LD2d(void *deviceAddr, uint64_t dpitch, const void *gvaAddr, uint64_t spitch,
                   size_t width, uint64_t height, void *stream) noexcept
{
    void *st = stream_;
    if (stream != nullptr) {
        st = stream;
    }

    int ret = BM_OK;
    for (uint64_t i = 0; i < height; ++i) {
        void *dstAddr = reinterpret_cast<void *>((uint64_t) deviceAddr + i * dpitch);
        void *srcAddr = reinterpret_cast<void *>((uint64_t) gvaAddr + i * spitch);
        auto asyncRet = DlAclApi::AclrtMemcpyAsync(dstAddr, width, srcAddr, width,
                                                   ACL_MEMCPY_DEVICE_TO_DEVICE, st);
        if (asyncRet != 0) {
            BM_LOG_ERROR("copy2d memory on gva to device failed:: " << asyncRet
                         << " spitch: " << spitch << " dpitch: " << dpitch << " width: " << width
                         << " height:" << height << " stream:" << st);
            ret = asyncRet;
        }
    }

    int syncRet  = DlAclApi::AclrtSynchronizeStream(st);
    if (syncRet != 0) {
        BM_LOG_ERROR("aclrtSynchronizeStream failed: " << syncRet << " stream:" << st);
        ret = syncRet;
    }
    return ret;
}

int HostDataOpSDMA::DataCopy2d(const void *srcVA, uint64_t spitch, void *destVA, uint64_t dpitch,
                               uint64_t width, uint64_t height, hybm_data_copy_direction direction,
                               const ExtOptions &options) noexcept
{
    int ret;
    switch (direction) {
        case HYBM_LOCAL_DEVICE_TO_GLOBAL_DEVICE:
            ret = CopyLD2GD2d(destVA, dpitch, srcVA, spitch, width, height, options.stream);
            break;
        case HYBM_GLOBAL_DEVICE_TO_LOCAL_DEVICE:
            ret = CopyGD2LD2d(destVA, dpitch, srcVA, spitch, width, height, options.stream);
            break;
        case HYBM_LOCAL_HOST_TO_GLOBAL_DEVICE:
            ret = CopyLH2GD2d(destVA, dpitch, srcVA, spitch, width, height, options.stream);
            break;
        case HYBM_GLOBAL_DEVICE_TO_LOCAL_HOST:
            ret = CopyGD2LH2d(destVA, dpitch, srcVA, spitch, width, height, options.stream);
            break;
        case HYBM_GLOBAL_DEVICE_TO_GLOBAL_DEVICE:
            ret = CopyLD2GD2d(destVA, dpitch, srcVA, spitch, width, height, options.stream);
            break;

        default:
            BM_LOG_ERROR("data copy invalid direction: " << direction);
            ret = BM_INVALID_PARAM;
    }
    return ret;
}

int32_t HostDataOpSDMA::DataCopyAsync(const void* srcVA, void* destVA, uint64_t length,
                                      hybm_data_copy_direction direction, const ExtOptions &options) noexcept
{
    BM_LOG_ERROR("not supported data copy async!");
    return BM_ERROR;
}

int32_t HostDataOpSDMA::Wait(int32_t waitId) noexcept
{
    BM_LOG_ERROR("not supported data copy wait!");
    return BM_ERROR;
}

int32_t HostDataOpSDMA::Initialized() noexcept
{
    return 0;
}

void HostDataOpSDMA::UnInitialized() noexcept
{

}

int HostDataOpSDMA::CopyGD2GH(void *destVA, const void *srcVA, uint64_t length, void *stream) noexcept
{
    // HBM池拷贝到HOST池
    return 0;
}

int HostDataOpSDMA::CopyGH2GD(void *destVA, const void *srcVA, uint64_t length, void *stream) noexcept
{
    // HOST池到HBM池的拷贝
    return 0;
}

int HostDataOpSDMA::CopyGH2GH(void *destVA, const void *srcVA, uint64_t length, void *stream) noexcept
{
    return 0;
}

int HostDataOpSDMA::CopyLD2GH(void *destVA, const void *srcVA, uint64_t length, void *stream) noexcept
{
    // local device到dram池的拷贝
    return 0;
}

int HostDataOpSDMA::CopyLH2GH(void *destVA, const void *srcVA, uint64_t length, void *stream) noexcept
{
    // local host到dram池的拷贝
    return 0;
}

int HostDataOpSDMA::CopyGH2LD(void *destVA, const void *srcVA, uint64_t length, void *stream) noexcept
{
    // dram池的拷贝到local device
    return 0;
}

int HostDataOpSDMA::CopyGH2LH(void *destVA, const void *srcVA, uint64_t length, void *stream) noexcept
{
    // dram池的拷贝到local host
    return 0;
}

}
}
