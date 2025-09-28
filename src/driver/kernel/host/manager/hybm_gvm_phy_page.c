/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */
#include "hybm_gvm_phy_page.h"

#include <linux/nodemask.h>
#include <linux/slab.h>

#include "hybm_gvm_log.h"
#include "hybm_gvm_proc_info.h"
#include "hybm_gvm_symbol_get.h"

u64 hybm_gvm_alloc_pg(void)
{
    int num = num_online_nodes();
    int i;
    u64 addr;

    for (i = 0; i < num; i++) {
        addr = (uint64_t)obmm_alloc(i, HYBM_GVM_PAGE_SIZE, 0);
        if (addr != 0) {
            break;
        }
    }

    hybm_gvm_debug("alloc pa valid:%d, numa:%d", addr != 0, i);
    return addr;
}

int hybm_gvm_free_pg(u64 addr)
{
    obmm_free((void *)addr, 0);

    hybm_gvm_debug("free pa");
    return 0;
}
