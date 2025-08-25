/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */
#ifndef MF_HYBRID_HYBM_ENTITY_COMPOSE_H
#define MF_HYBRID_HYBM_ENTITY_COMPOSE_H

#include <map>
#include "hybm_common_include.h"
#include "hybm_device_mem_segment.h"
#include "hybm_data_operator.h"
#include "hybm_mem_segment.h"
#include "hybm_entity.h"

#include "hybm_transport_manager.h"

namespace ock {
namespace mf {
class HybmEntityCompose : public MemEntity {
public:
    explicit HybmEntityCompose(int32_t id) noexcept : id_(id), initialized(false) {};
    ~HybmEntityCompose() override = default;

    int32_t Initialize(const hybm_options *options) noexcept override;
    void UnInitialize() noexcept override;

    int32_t ReserveMemorySpace(void **reservedMem) noexcept override;

    void *GetReservedMemoryPtr(hybm_mem_type memType) noexcept override;

    int32_t UnReserveMemorySpace() noexcept override;

    int32_t AllocLocalMemory(uint64_t size, uint32_t flags, hybm_mem_slice_t &slice) noexcept override;
    void FreeLocalMemory(hybm_mem_slice_t slice, uint32_t flags) noexcept override;

    int32_t ExportExchangeInfo(hybm_exchange_info &desc, uint32_t flags) noexcept override;
    int32_t ExportExchangeInfo(hybm_mem_slice_t slice, hybm_exchange_info &desc, uint32_t flags) noexcept override;
    int32_t ImportExchangeInfo(const hybm_exchange_info *desc, uint32_t count, uint32_t flags) noexcept override;
    int32_t RemoveImported(const std::vector<uint32_t>& ranks) noexcept override;

    int32_t SetExtraContext(const void *context, uint32_t size) noexcept override;

    int32_t Mmap() noexcept override;
    void Unmap() noexcept override;

    bool CheckAddressInEntity(const void *ptr, uint64_t length) const noexcept override;
    int32_t CopyData(const void *src, void *dest, uint64_t length, hybm_data_copy_direction direction,
                     void *stream, uint32_t flags) noexcept override;
    int32_t CopyData2d(const void *src, uint64_t spitch, void *dest, uint64_t dpitch, uint64_t width, uint64_t height,
                       hybm_data_copy_direction direction, void *stream, uint32_t flags) noexcept override;
    int32_t ImportEntityExchangeInfo(const hybm_exchange_info desc[],
                                     uint32_t count, uint32_t flags) noexcept override;
    bool SdmaReaches(uint32_t remoteRank) const noexcept override;

private:
    static int CheckOptions(const hybm_options *options) noexcept;
    int UpdateHybmDeviceInfo(uint32_t extCtxSize) noexcept;
    void SetHybmDeviceInfo(HybmDeviceMeta &info);
    void CleanEntitySource();

    Result InitSegment();
    Result InitHbmSegment();
    Result InitDramSegment();
    Result InitTransManager();
    Result InitDataOperator();

    std::shared_ptr<MemSlice> FindMemSlice(hybm_mem_slice_t slice, std::shared_ptr<MemSegment> &segment_);
    void GetSegmentFromDirect(hybm_data_copy_direction direct, std::shared_ptr<MemSegment> &src,
                              std::shared_ptr<MemSegment> &dest);

private:
    bool initialized;
    const int32_t id_; /* id of the engine */
    hybm_options options_{};
    void *stream_{nullptr};
    void *deviceGva_{nullptr};
    std::shared_ptr<MemSlice> deviceSlice_{nullptr};
    std::shared_ptr<MemSegment> deviceSegment_{nullptr};
    void *hostGva_{nullptr};
    std::shared_ptr<MemSlice> hostSlice_{nullptr};
    std::shared_ptr<MemSegment> hostSegment_{nullptr};
    std::shared_ptr<DataOperator> dataOperator_{nullptr};
    bool transportPrepared_{false};
    transport::TransManagerPtr transportManager_;
    std::unordered_map<uint32_t, std::vector<transport::TransportMemoryKey>> importedMemories_;
};
}
}

#endif // MF_HYBRID_HYBM_ENTITY_COMPOSE_H
