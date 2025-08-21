/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */
#ifndef MF_HYBRID_HYBM_GVM_PHY_PAGE_H
#define MF_HYBRID_HYBM_GVM_PHY_PAGE_H

#include <linux/spinlock.h>
#include <linux/types.h>

struct hybm_gvm_pg_list {
    u64 *arr;
    u32 start;
    u32 end;
    u32 size;
};

struct hybm_gvm_pg_manager {
    int initialized;
    u64 pg_start;
    u64 pg_size;

    spinlock_t lock;
    struct hybm_gvm_pg_list pg_list;
};

int hybm_gvm_pg_init(void);
void hybm_gvm_pg_uninit(void);
u64 hybm_gvm_alloc_pg(void);
int hybm_gvm_free_pg(u64 addr);

#endif // MF_HYBRID_HYBM_GVM_PHY_PAGE_H
