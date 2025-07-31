/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include "smem.h"

int32_t smem_init(uint32_t flags)
{
    return 0;
}

int32_t smem_set_extern_logger(void (*func)(int level, const char *msg))
{
    return 0;
}

int32_t smem_set_log_level(int level)
{
    return 0;
}

void smem_uninit()
{
    return;
}

const char *smem_get_last_err_msg()
{
    return "";
}

const char *smem_get_and_clear_last_err_msg()
{
    return "";
}