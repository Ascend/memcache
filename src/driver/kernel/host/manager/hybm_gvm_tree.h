/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */
#ifndef MF_HYBRID_HYBM_GVM_TREE_H
#define MF_HYBRID_HYBM_GVM_TREE_H

#include "hybm_gvm_proc_info.h"

bool gvm_key_tree_insert(struct gvm_rbtree *rtree, struct gvm_node *data);
void gvm_key_tree_remove(struct gvm_rbtree *rtree, struct gvm_node *data, void (*release)(struct kref *kref));
struct gvm_node *gvm_key_tree_search_and_inc(struct gvm_rbtree *rtree, u64 key);

bool gvm_va_tree_cross(struct gvm_rbtree *rtree, u64 va, u64 size);
void *gvm_va_tree_get_first_and_inc(struct gvm_rbtree *rtree);
void *gvm_va_tree_search_and_inc(struct gvm_rbtree *rtree, u64 va);
bool gvm_va_tree_insert(struct gvm_rbtree *rtree, void *data);
void gvm_va_tree_remove_and_dec(struct gvm_rbtree *rtree, void *data, void (*release)(struct kref *kref));

#endif // MF_HYBRID_HYBM_GVM_TREE_H
