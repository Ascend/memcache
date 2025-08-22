/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */
#include "hybm_gvm_s2a.h"

#include "hybm_gvm_agent_msg.h"
#include "hybm_gvm_log.h"
#include "hybm_gvm_symbol_get.h"

static int hybm_gvm_agent_msg_send(u32 devid, struct hybm_gvm_agent_msg *msg, u32 in_data_len, u32 out_data_len,
                                   u32 *real_out_len)
{
    hybm_gvm_debug("hybm_gvm_agent_msg_send, type:%d", msg->type);
    return devdrv_common_msg_send(devid, (void *)msg, in_data_len, out_data_len, real_out_len,
                                  DEVDRV_COMMON_MSG_PROFILE);
}

int hybm_gvm_to_agent_init(u32 devid, u32 *pasid, u32 *svspid)
{
    struct hybm_gvm_agent_msg msg;
    u32 out_len;
    int ret;

    msg.type = HYBM_GVM_AGENT_MSG_INIT;
    msg.valid = HYBM_GVM_S2A_MSG_SEND_MAGIC;
    msg.result = 0;
    msg.init.hostpid = (current->tgid);
    msg.init.pasid = 0;
    msg.init.svspid = 0;

    ret = hybm_gvm_agent_msg_send(devid, &msg, sizeof(msg), sizeof(msg), &out_len);
    if (ret != 0 || out_len != sizeof(msg) || msg.valid != HYBM_GVM_S2A_MSG_RCV_MAGIC || msg.result != 0) {
        hybm_gvm_err("init msg send fail. (ret=%d; result=%d; valid=0x%x; devid=%u; size=%u)", ret, msg.result,
                     msg.valid, devid, out_len);
        return -EBUSY;
    }

    *pasid = msg.init.pasid;
    *svspid = msg.init.svspid;
    return 0;
}

int hybm_gvm_to_agent_mmap(u32 devid, u32 pasid, u64 va, u64 pa, u64 size)
{
    struct hybm_gvm_agent_msg msg;
    u32 out_len;
    int ret;

    msg.type = HYBM_GVM_AGENT_MSG_MAP;
    msg.valid = HYBM_GVM_S2A_MSG_SEND_MAGIC;
    msg.result = 0;
    msg.mmap.va = va;
    msg.mmap.pa = pa;
    msg.mmap.size = size;
    msg.mmap.pasid = pasid;

    ret = hybm_gvm_agent_msg_send(devid, &msg, sizeof(msg), sizeof(msg), &out_len);
    if (ret != 0 || out_len != sizeof(msg) || msg.valid != HYBM_GVM_S2A_MSG_RCV_MAGIC || msg.result != 0) {
        hybm_gvm_err("map msg send fail. (ret=%d; result=%d; valid=0x%x; devid=%u; size=%u)", ret, msg.result,
                     msg.valid, devid, out_len);
        return -EBUSY;
    }
    return 0;
}

int hybm_gvm_to_agent_unmap(u32 devid, u32 pasid, u64 va, u64 size, u64 page_size)
{
    struct hybm_gvm_agent_msg msg;
    u32 out_len;
    int ret;

    msg.type = HYBM_GVM_AGENT_MSG_UNMAP;
    msg.valid = HYBM_GVM_S2A_MSG_SEND_MAGIC;
    msg.result = 0;
    msg.unmap.va = va;
    msg.unmap.pasid = pasid;
    msg.unmap.size = size;
    msg.unmap.page_size = page_size;

    ret = hybm_gvm_agent_msg_send(devid, &msg, sizeof(msg), sizeof(msg), &out_len);
    if (ret != 0 || out_len != sizeof(msg) || msg.valid != HYBM_GVM_S2A_MSG_RCV_MAGIC || msg.result != 0) {
        hybm_gvm_err("unmap msg send fail. (ret=%d; result=%d; valid=0x%x; devid=%u; size=%u)", ret, msg.result,
                     msg.valid, devid, out_len);
        return -EBUSY;
    }
    return 0;
}

int hybm_gvm_to_agent_fetch(u32 devid, u32 pasid, u64 va, u64 size)
{
    struct hybm_gvm_agent_msg msg;
    u32 out_len;
    int ret;

    msg.type = HYBM_GVM_AGENT_MSG_FETCH;
    msg.valid = HYBM_GVM_S2A_MSG_SEND_MAGIC;
    msg.result = 0;
    msg.fetch.va = va;
    msg.fetch.size = size;
    msg.fetch.hostpid = (current->tgid);
    msg.fetch.pasid = pasid;

    ret = hybm_gvm_agent_msg_send(devid, &msg, sizeof(msg), sizeof(msg), &out_len);
    if (ret != 0 || out_len != sizeof(msg) || msg.valid != HYBM_GVM_S2A_MSG_RCV_MAGIC || msg.result != 0) {
        hybm_gvm_err("fetch msg send fail. (ret=%d; result=%d; valid=0x%x; devid=%u; size=%u)", ret, msg.result,
                     msg.valid, devid, out_len);
        return -EBUSY;
    }
    return 0;
}

int hybm_gvm_to_agent_alloc(u32 devid, u32 pasid, u64 pa, u64 size, u64 *va)
{
    struct hybm_gvm_agent_msg msg;
    u32 out_len;
    int ret;

    msg.type = HYBM_GVM_AGENT_MSG_MAP;
    msg.valid = HYBM_GVM_S2A_MSG_SEND_MAGIC;
    msg.result = 0;
    msg.mmap.va = 0ULL;
    msg.mmap.pa = pa;
    msg.mmap.size = size;
    msg.mmap.pasid = pasid;

    ret = hybm_gvm_agent_msg_send(devid, &msg, sizeof(msg), sizeof(msg), &out_len);
    if (ret != 0 || out_len != sizeof(msg) || msg.valid != HYBM_GVM_S2A_MSG_RCV_MAGIC || msg.result != 0) {
        hybm_gvm_err("alloc msg send fail. (ret=%d; result=%d; valid=0x%x; devid=%u; size=%u)", ret, msg.result,
                     msg.valid, devid, out_len);
        return -EBUSY;
    }
    *va = msg.mmap.va;
    return 0;
}
