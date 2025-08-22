/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */
#ifndef MF_HYBRID_HYBM_GVM_PHY_PAGE_H
#define MF_HYBRID_HYBM_GVM_PHY_PAGE_H

#include <linux/types.h>

u64 hybm_gvm_alloc_pg(void);
int hybm_gvm_free_pg(u64 addr);

#endif // MF_HYBRID_HYBM_GVM_PHY_PAGE_H
