/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023. All rights reserved.
 */

#ifndef MF_HYBRID_DEVICE_RDMA_COMMON_H
#define MF_HYBRID_DEVICE_RDMA_COMMON_H

#include <arpa/inet.h>
#include <ostream>
#include "hybm_define.h"
#include "hybm_transport_common.h"

namespace ock {
namespace mf {
namespace transport {
namespace device {

struct RegMemResult {
    uint32_t type{TT_HCCP};
    uint32_t reserved{0};
    uint64_t address;
    uint64_t size;
    void *mrHandle;
    uint32_t lkey;
    uint32_t rkey;

    RegMemResult() : address{0}, size{0}, mrHandle{nullptr}, lkey{0}, rkey{0} {}
    RegMemResult(uint64_t addr, uint64_t sz, void *hd, uint32_t lk, uint32_t rk)
        : address{addr},
          size{sz},
          mrHandle{hd},
          lkey{lk},
          rkey{rk}
    {
    }
};

union RegMemKeyUnion {
    TransportMemoryKey commonKey;
    RegMemResult deviceKey;
};

static inline std::ostream &operator<<(std::ostream &output, const RegMemResult &mr)
{
    output << "RegMemResult(size=" << mr.size << ", mrHandle=" << mr.mrHandle << ", lkey=" << mr.lkey
           << ", rkey=" << mr.rkey << ")";
    return output;
}

static inline std::ostream &operator<<(std::ostream &output, const HccpRaInitConfig &config)
{
    output << "HccpRaInitConfig(phyId=" << config.phyId << ", nicPosition=" << config.nicPosition
           << ", hdcType=" << config.hdcType << ")";
    return output;
}

static inline std::ostream &operator<<(std::ostream &output, const HccpRdevInitInfo &info)
{
    output << "HccpRdevInitInfo(mode=" << info.mode << ", notify=" << info.notifyType
           << ", enabled910aLite=" << info.enabled910aLite << ", disabledLiteThread=" << info.disabledLiteThread
           << ", enabled2mbLite=" << info.enabled2mbLite << ")";
    return output;
}

static inline std::ostream &operator<<(std::ostream &output, const HccpRdev &rdev)
{
    output << "HccpRdev(phyId=" << rdev.phyId << ", family=" << rdev.family
           << ", rdev.ip=" << inet_ntoa(rdev.localIp.addr) << ")";
    return output;
}

static inline std::ostream &operator<<(std::ostream &output, const ai_data_plane_wq &info)
{
    output << "ai_data_plane_wq(wqn=" << info.wqn << ", buff_addr=" << (void *)(ptrdiff_t)info.buf_addr
           << ", wqebb_size=" << info.wqebb_size << ", depth=" << info.depth
           << ", head=" << (void *)(ptrdiff_t)info.head_addr << ", tail=" << (void *)(ptrdiff_t)info.tail_addr
           << ", swdb_addr=" << (void *)(ptrdiff_t)info.swdb_addr << ", db_reg=" << info.db_reg << ")";
    return output;
}

static inline std::ostream &operator<<(std::ostream &output, const ai_data_plane_cq &info)
{
    output << "ai_data_plane_cq(cqn=" << info.cqn << ", buff_addr=" << (void *)(ptrdiff_t)info.buf_addr
           << ", cqe_size=" << info.cqe_size << ", depth=" << info.depth
           << ", head=" << (void *)(ptrdiff_t)info.head_addr << ", tail=" << (void *)(ptrdiff_t)info.tail_addr
           << ", swdb_addr=" << (void *)(ptrdiff_t)info.swdb_addr << ", db_reg=" << info.db_reg << ")";
    return output;
}

static inline std::ostream &operator<<(std::ostream &output, const HccpAiQpInfo &info)
{
    output << "HccpAiQpInfo(addr=" << (void *)(ptrdiff_t)info.aiQpAddr << ", sq_index=" << info.sqIndex
           << ", db_index=" << info.dbIndex << ", ai_scq_addr=" << (void *)(ptrdiff_t)info.ai_scq_addr
           << ", ai_rcq_addr" << (void *)(ptrdiff_t)info.ai_rcq_addr
           << ", data_plane_info:<sq=" << info.data_plane_info.sq << ", rq=" << info.data_plane_info.rq
           << ", scq=" << info.data_plane_info.scq << ", rcq=" << info.data_plane_info.rcq << ">)";
    return output;
}

static inline std::ostream &operator<<(std::ostream &output, const AiQpRMAWQ &info)
{
    output << "AiQpRMAWQ(wqn=" << info.wqn << ", buff_addr=" << (void *)(ptrdiff_t)info.bufAddr
           << ", wqe_size=" << info.wqeSize << ", depth=" << info.depth << ", head=" << (void *)(ptrdiff_t)info.headAddr
           << ", tail=" << (void *)(ptrdiff_t)info.tailAddr << ", db_mode=" << static_cast<int>(info.dbMode)
           << ", db_addr=" << (void *)(ptrdiff_t)info.dbAddr << ", sl=" << info.sl << ")";
    return output;
}

static inline std::ostream &operator<<(std::ostream &output, const AiQpRMACQ &info)
{
    output << "AiQpRMACQ(cqn=" << info.cqn << ", buff_addr=" << (void *)(ptrdiff_t)info.bufAddr
           << ", cqe_size=" << info.cqeSize << ", depth=" << info.depth << ", head=" << (void *)(ptrdiff_t)info.headAddr
           << ", tail=" << (void *)(ptrdiff_t)info.tailAddr << ", db_mode=" << static_cast<int>(info.dbMode)
           << ", db_addr=" << (void *)(ptrdiff_t)info.dbAddr << ")";
    return output;
}

static inline std::ostream &operator<<(std::ostream &output, const RdmaMemRegionInfo &info)
{
    output << "RdmaMemRegionInfo(size=" << info.size << ", addr=" << (void *)(ptrdiff_t)(info.addr)
           << ", lkey=" << info.lkey << ", rkey=" << info.rkey << ")";
    return output;
}

static inline std::string AiQpInfoToString(const AiQpRMAQueueInfo &info, uint32_t rankCount)
{
    std::stringstream ss;
    ss << "QiQpInfo(rankCount=" << rankCount << ", mq_count=" << info.count << ")={\n";
    for (auto i = 0U; i < rankCount; i++) {
        ss << "  rank" << i << "={\n";
        for (auto j = 0U; j < info.count; j++) {
            ss << "    qp" << j << "_info={\n";
            ss << "      sq=<" << (*(info.sq + i * info.count + j)) << ">\n";
            ss << "      rq=<" << (*(info.rq + i * info.count + j)) << ">\n";
            ss << "      scq=<" << (*(info.scq + i * info.count + j)) << ">\n";
            ss << "      rcq=<" << (*(info.rcq + i * info.count + j)) << ">\n";
            ss << "    }\n";
        }
        ss << "    MR-rank-" << i << "=<" << (*(info.mr + i)) << ">";
        ss << "  }\n";
    }
    ss << "}";
    return ss.str();
}

static inline std::ostream &operator<<(std::ostream &output, const HccpSocketConnectInfo &info)
{
    output << "HccpSocketConnectInfo(socketHandle=" << info.handle << ", remoteIp=" << inet_ntoa(info.remoteIp.addr)
           << ", port=" << info.port << ")";
    return output;
}
}
}
}
}

#endif  // MF_HYBRID_DEVICE_RDMA_COMMON_H
