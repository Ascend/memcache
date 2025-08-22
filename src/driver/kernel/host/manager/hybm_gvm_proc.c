/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
 */
#include "hybm_gvm_proc.h"

#include <linux/mm.h>
#include <linux/slab.h>

#include "hybm_gvm_ioctl.h"
#include "hybm_gvm_log.h"
#include "hybm_gvm_p2p.h"
#include "hybm_gvm_phy_page.h"
#include "hybm_gvm_s2a.h"
#include "hybm_gvm_symbol_get.h"

#define GVM_NODE_FLAG_ALLOCATED 1ULL
#define GVM_NODE_FLAG_REMOTE    2ULL

#define GVM_KEY_SDID_OFFSET    32
#define GVM_SDID_SERVER_OFFSET 22
#define GVM_GLOBAL_SERVER_NUM  48
#define GVM_GLOBAL_PA_MASK     (0xffffffffffffULL) // 256T (2^48)

struct hybm_gvm_process *g_proc_list[HYBM_MAX_DEVICE_NUM] = {NULL};

static inline bool gvm_check_flag(u64 *flag, u64 shift)
{
    return (((*flag) >> shift) & 1ULL);
}

static inline void gvm_set_flag(u64 *flag, u64 shift)
{
    (*flag) |= (1ULL << shift);
}

static inline void gvm_clear_flag(u64 *flag, u64 shift)
{
    (*flag) &= (~(1ULL << shift));
}

static inline u32 gvm_get_devid_by_sdid(u32 sdid)
{
    return (sdid & (((u32)1 << UDEVID_BIT_LEN) - 1));
}

static inline u64 gvm_generate_key(u32 sdid, u32 id)
{
    return (((u64)sdid << GVM_KEY_SDID_OFFSET) | id);
}

static inline u32 gvm_get_sdid_by_key(u64 key)
{
    return (key >> GVM_KEY_SDID_OFFSET);
}

static inline u32 gvm_get_server_id_by_sdid(u32 sdid)
{
    return (sdid >> GVM_SDID_SERVER_OFFSET);
}

static u64 g_global_pa_offset[48] = {
    [4] = 0xb76e00000000ULL,
    [5] = 0xb99580000000ULL,
};

static u64 gvm_get_local_pa_from_global(u64 pa, u32 sdid)
{
    u32 server_id = gvm_get_server_id_by_sdid(sdid);
    u64 offset;
    if (server_id >= GVM_GLOBAL_SERVER_NUM) {
        hybm_gvm_err("translate failed, server id is invalid(%u)", server_id);
        return 0ULL;
    }

    offset = g_global_pa_offset[server_id];
    if (offset == 0ULL) {
        hybm_gvm_err("translate failed, server id is not support(%u)", server_id);
        return 0ULL;
    }

    return (pa + offset) & GVM_GLOBAL_PA_MASK;
}

static u64 gvm_get_global_pa_from_local(u64 pa, u32 sdid)
{
    u32 server_id = gvm_get_server_id_by_sdid(sdid);
    u64 offset;
    if (server_id >= GVM_GLOBAL_SERVER_NUM) {
        hybm_gvm_err("translate failed, server id is invalid(%u)", server_id);
        return 0ULL;
    }

    offset = g_global_pa_offset[server_id];
    if (offset == 0ULL) {
        hybm_gvm_err("translate failed, server id is not support(%u)", server_id);
        return 0ULL;
    }

    return (pa - offset) & GVM_GLOBAL_PA_MASK;
}

static bool gvm_tree_insert(struct rb_root *root, struct gvm_node *data)
{
    struct rb_node **new = &(root->rb_node);
    struct rb_node *parent = NULL;
    while (*new) {
        struct gvm_node *tmp = container_of(*new, struct gvm_node, tree_node);
        parent = *new;
        if (data->shm_key < tmp->shm_key) {
            new = &((*new)->rb_left);
        } else if (data->shm_key > tmp->shm_key) {
            new = &((*new)->rb_right);
        } else {
            return false;
        }
    }
    /* Add new node and rebalance tree. */
    rb_link_node(&data->tree_node, parent, new);
    rb_insert_color(&data->tree_node, root);
    return true;
}

static void gvm_tree_remove(struct rb_root *root, struct gvm_node *data)
{
    rb_erase(&data->tree_node, root);
}

static struct gvm_node *gvm_tree_search(struct rb_root *root, u64 key)
{
    struct rb_node *node = root->rb_node;
    while (node) {
        struct gvm_node *data = container_of(node, struct gvm_node, tree_node);
        if (key < data->shm_key) {
            node = node->rb_left;
        } else if (key > data->shm_key) {
            node = node->rb_right;
        } else {
            return data;
        }
    }
    return NULL;
}
