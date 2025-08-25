/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
 */
#include "hybm_op_dram_sdma.h"

#include "hybm_logger.h"
#include "hybm_stream.h"

namespace ock {
namespace mf {

HostDramSdma::~HostDramSdma()
{
    HostDramSdma::UnInitialized();
}

int32_t HostDramSdma::Initialized() noexcept
{
    if (inited_) {
        return BM_OK;
    }

    uint32_t devId = HybmGetInitDeviceId();
    stream_ = std::make_shared<HybmStream>(devId, 0, 0);
    BM_ASSERT_RETURN(stream_ != nullptr, BM_MALLOC_FAILED);

    auto ret = stream_->Initialize();
    if (ret != 0) {
        BM_LOG_ERROR("create stream failed, dev:" << devId << " ret:" << ret);
        stream_ = nullptr;
        return BM_ERROR;
    }

    inited_ = true;
    return BM_OK;
}

void HostDramSdma::UnInitialized() noexcept
{
    if (!inited_) {
        return;
    }

    if (stream_ != nullptr) {
        stream_->Destroy();
        stream_ = nullptr;
    }
    inited_ = false;
}

int32_t HostDramSdma::DataCopy(const void *srcVA, void *destVA, uint64_t length, hybm_data_copy_direction direction,
                               const ExtOptions &options) noexcept
{
    BM_ASSERT_RETURN(inited_, BM_NOT_INITIALIZED);
    int ret;
    switch (direction) {
        case HYBM_GLOBAL_HOST_TO_GLOBAL_HOST:
            ret = CopyGH2GH(destVA, srcVA, length);
            break;

        default:
            BM_LOG_ERROR("data copy invalid direction: " << direction);
            ret = BM_INVALID_PARAM;
    }
    return ret;
}

int HostDramSdma::CopyGH2GH(void *destVA, const void *srcVA, size_t count) noexcept
{
    StreamTask task;
    task.type = STREAM_TASK_TYPE_SDMA;
    rtStarsMemcpyAsyncSqe_t *const sqe = &(task.sqe.memcpyAsyncSqe);
    sqe->header.type = RT_STARS_SQE_TYPE_SDMA;
    sqe->header.ie = RT_STARS_SQE_INT_DIR_NO;
    sqe->header.pre_p = RT_STARS_SQE_INT_DIR_NO;
    sqe->header.wr_cqe = 0U; // TODO: cann里面为0,无异常时不写cqe
    sqe->header.rt_stream_id = stream_->GetId();
    sqe->header.task_id = 0;

    sqe->kernelCredit = RT_STARS_DEFAULT_KERNEL_CREDIT;
    sqe->ptrMode = 0;
    sqe->opcode = 0U;

    sqe->src_streamid = 0U; // get sid and ssid from sq, leave 0 here
    sqe->dst_streamid = 0U;
    sqe->src_sub_streamid = 0U;
    sqe->dstSubStreamId = 0U;
    sqe->ie2 = 0U;
    sqe->sssv = 1U;
    sqe->dssv = 1U;
    sqe->sns = 1U;
    sqe->dns = 1U;
    sqe->qos = 6U;
    sqe->sro = 0U;
    sqe->dro = 0U;
    sqe->partid = 0U;
    sqe->mpam = 0U;

    sqe->res3 = 0U;
    sqe->res4 = 0U;
    sqe->res5 = 0U;
    sqe->res6 = 0U;

    sqe->d2dOffsetFlag = 0U;
    sqe->srcOffsetLow = 0U;
    sqe->dstOffsetLow = 0U;
    sqe->srcOffsetHigh = 0U;
    sqe->dstOffsetHigh = 0U;

    sqe->length = count;
    sqe->src_addr_low =
        static_cast<uint32_t>(static_cast<uint64_t>(reinterpret_cast<uintptr_t>(srcVA)) & 0x00000000FFFFFFFFU);
    sqe->src_addr_high = static_cast<uint32_t>(
        (static_cast<uint64_t>(reinterpret_cast<uintptr_t>(srcVA)) & 0xFFFFFFFF00000000U) >> UINT32_BIT_NUM);
    sqe->dst_addr_low =
        static_cast<uint32_t>(static_cast<uint64_t>(reinterpret_cast<uintptr_t>(destVA)) & 0x00000000FFFFFFFFU);
    sqe->dst_addr_high = static_cast<uint32_t>(
        (static_cast<uint64_t>(reinterpret_cast<uintptr_t>(destVA)) & 0xFFFFFFFF00000000U) >> UINT32_BIT_NUM);

    auto ret = stream_->SubmitTasks(task);
    BM_ASSERT_RETURN(ret == 0, BM_ERROR);

    ret = stream_->Synchronize();
    BM_ASSERT_RETURN(ret == 0, BM_ERROR);
    return BM_OK;
}

int HostDramSdma::DataCopy2d(const void *srcVA, uint64_t spitch, void *destVA, uint64_t dpitch, uint64_t width,
                             uint64_t height, hybm_data_copy_direction direction, const ExtOptions &options) noexcept
{
    BM_ASSERT_RETURN(inited_, BM_NOT_INITIALIZED);
    BM_LOG_ERROR("not support data copy 2d by dram sdma!");
    return BM_ERROR;
}

int32_t HostDramSdma::DataCopyAsync(const void *srcVA, void *destVA, uint64_t length,
                                    hybm_data_copy_direction direction, const ExtOptions &options) noexcept
{
    BM_ASSERT_RETURN(inited_, BM_NOT_INITIALIZED);
    BM_LOG_ERROR("not supported data copy async!");
    return BM_ERROR;
}

int32_t HostDramSdma::Wait(int32_t waitId) noexcept
{
    BM_ASSERT_RETURN(inited_, BM_NOT_INITIALIZED);
    BM_LOG_ERROR("not supported data copy wait!");
    return BM_ERROR;
}

} // namespace mf
} // namespace ock
