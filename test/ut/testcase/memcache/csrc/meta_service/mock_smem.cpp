/*
* Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 * MemFabric_Hybrid is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
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