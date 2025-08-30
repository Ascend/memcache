/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */

#include "hybm_gvm_ioctl.h"
#include "hybm_gvm_cmd.h"
#include "hybm_gvm_proc.h"
#include "hybm_gvm_log.h"

int hybm_gvm_dispatch_ioctl(struct file *file, u32 cmd, struct hybm_gvm_ioctl_arg *buffer)
{
    struct hybm_gvm_process *gvm_proc = NULL;
    u32 cmd_id = _IOC_NR(cmd);
    int ret;

    if (file->private_data == NULL) {
        hybm_gvm_err("gvm not initialized, private_data is NULL.");
        return -EINVAL;
    }

    if (cmd_id >= HYBM_GVM_CMD_MAX_CMD || gvm_ioctl_handlers[cmd_id].ioctl_handler == NULL) {
        hybm_gvm_err("Cmd not support. (cmd=0x%x; cmd_id=0x%x)", cmd, cmd_id);
        return -EOPNOTSUPP;
    }

    if (cmd == HYBM_GVM_CMD_PROC_CREATE) { // 初始化操作未创建gvm_proc,不加锁
        return gvm_ioctl_handlers[cmd_id].ioctl_handler(file, NULL, buffer);
    }

    gvm_proc = (struct hybm_gvm_process *)(((struct gvm_private_data *)file->private_data)->process);
    if (gvm_proc == NULL || gvm_proc->initialized != true) {
        hybm_gvm_err("gvm_proc has not been initialized. cmd:%x create_cmd:%lx", cmd, HYBM_GVM_CMD_PROC_CREATE);
        return -EINVAL;
    }

    kref_get(&gvm_proc->ref);
    ret = gvm_ioctl_handlers[cmd_id].ioctl_handler(file, gvm_proc, buffer);
    kref_put(&gvm_proc->ref, hybm_proc_ref_release);
    return ret;
}
