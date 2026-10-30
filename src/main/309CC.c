/* 12 game functions, among them gpu_AddDrawMove. .text 0x800401CC (ROM
 * 0x309CC). Start boundary: G8 (cc1 -G8 by proof). */
#include "common.h"
#define INCLUDE_ASM_USE_MACRO_INC 1
#include "include_asm.h"
#include "bb2.h"

/* Unprototyped: slot_FinalizeObjectLoadSoundTexturesAndArena passes arena_id *
 * 3 + 1 as an int; the s16 prototype snd.c defines it with narrows that
 * argument before the call. */
extern s32 snd_LoadVabPack();
extern s32 g_gpu_draw_move_bank;
extern u16 g_gpu_draw_move_table[];
extern u8 g_gpu_asset_draw_move_masks[];

/* Q65: this file's statics (.sbss, allocated per file in link order by
 * PSYLINK), in address order. */
/* Cursor in the 10-packet bank
 * g_gpu_draw_move_packets[g_gpu_presented_frame_count & 1]. */
static DR_MOVE *g_gpu_draw_move_cursor;

void gpu_AddDrawMove(s32 slot_index, s32 move_index) {
    s32 bank_index;
    RECT buf;
    u16 *tbl;
    s16 u, v;

    bank_index = g_gpu_presented_frame_count & 1;
    if (bank_index != g_gpu_draw_move_bank) {
        g_gpu_draw_move_cursor = g_gpu_draw_move_packets[bank_index];
        g_gpu_draw_move_bank = bank_index;
    }
    if (g_gpu_draw_move_cursor !=
        g_gpu_draw_move_packets[g_gpu_draw_move_bank] + 10) {
        tbl = g_gpu_draw_move_table + move_index * 6;
        buf.x = *tbl++;
        buf.y = *tbl++;
        buf.w = *tbl++;
        buf.h = *tbl++;
        u = *tbl++;
        v = *tbl;
        if (slot_index != 0) {
            buf.x = buf.x + 0x80;
            u = u + 0x80;
        }
        SetDrawMove(g_gpu_draw_move_cursor, &buf, u, v);
        addPrim(&g_gpu_ot_offset_ptr[0x3FFC / 4], g_gpu_draw_move_cursor);
        g_gpu_draw_move_cursor++;
    }
}

void gpu_QueueSlotDrawMovesForSelectedSet(s32 slot_index, s32 draw_move_set) {
    Unk80045878Obj *ptr;
    s32 mask;
    s32 i;

    ptr = slot_GetObject(slot_index);
    if (ptr != 0) {
        switch (draw_move_set) {
        case 0:
            mask = 0x05;
            break;
        case 1:
            mask = 0x09;
            break;
        case 2:
            mask = 0x11;
            break;
        case 3:
            mask = 0x06;
            break;
        case 4:
            mask = 0x0A;
            break;
        case 5:
            mask = 0x12;
            break;
        case 6:
            gpu_AddDrawMove(slot_index, 5);
            gpu_AddDrawMove(slot_index, 6);
            mask = 0;
            break;
        }
        mask = mask & g_gpu_asset_draw_move_masks[ptr->asset_index];
        i = 0;
        do {
            if (mask & 1) {
                gpu_AddDrawMove(slot_index, 4 - i);
            }
            i++;
            mask >>= 1;
        } while (i < 5);
    }
}

void node_AppendProjectedNodeToList(
    Unk80045878Node *a0, Unk80045878Node *node_cursor, s16 block_id) {
    while (node_cursor->node.reloc_entry_index != -1) {
        node_cursor++;
    }
    node_cursor->node.reloc_entry_index = 2;
    node_cursor->node.node_kind = 3;
    node_cursor->node.primitive_table_select = 0;
    node_cursor->node.parent_node = &a0[6].node;
    node_cursor->node.matrix_state = 1;
    node_cursor->node.rotation_order = 0;
    node_cursor->node.light_setup_index = 0;
    node_cursor->node.reloc_block_id = block_id;
    node_cursor->unk58 = 0;
    node_cursor[1].node.reloc_entry_index = -1;
}

Unk80045878Node *node_GetSlotNodeById(s32 slot_index, s32 node_id) {
    Unk80045878Obj *base = slot_GetObject(slot_index);
    return base->node_by_id[node_id];
}

void node_SetWordOnAllListedNodes(Unk80045878Node *node_cursor, s32 a1) {
    if (node_cursor->node.reloc_entry_index == -1) {
        return;
    }
    do {
        node_cursor->unk58 = a1;
        node_cursor++;
    } while (node_cursor->node.reloc_entry_index != -1);
}

void slot_ClearObjectPointersAndTextureRequests(void) {
    s32 i;
    for (i = 0; i < 3; i++) {
        g_slot_objects[i] = 0;
        g_slot_tim_group_requests[i] = 0;
    }
}

extern void slot_FinalizeObjectLoadSoundTexturesAndArena(Unk80045878Obj *);
extern void node_InitObjectHierarchyFromTemplates(Unk80045878Obj *);
extern void node_BuildIdToNodeMapWithDuplicates(Unk80045878Obj *);
/* Not the definition's spelling: snd_VabFakeOpen takes an s16 second parameter,
 * but this call passes it unextended (an s16 prototype adds sll/sra). */
extern s32 snd_VabFakeOpen(s32, s32);

Unk80045878Obj *slot_LoadObjectBuildNodesSoftbodyAndFogLevels(
    s32 slot_index, s32 a1, s32 a2) {
    Unk80045878Obj *ptr;
    ptr = slot_OpenObjectBlockFreeSoftbodyAndLoadOrReuseAssetBlock(
        slot_index, a1, a2);
    g_slot_objects[slot_index] = ptr;
    slot_FinalizeObjectLoadSoundTexturesAndArena(ptr);
    node_InitObjectHierarchyFromTemplates(ptr);
    node_BuildIdToNodeMapWithDuplicates(ptr);
    softbody_SetupObject(ptr, 1);
    softbody_RequestRebuild(ptr);
    node_BuildProjectedNodeList(ptr);
    gpu_BuildFogBlockLevels();
    return ptr;
}

void slot_FinalizeObjectLoadSoundTexturesAndArena(Unk80045878Obj *slot_object) {
    s32 *block_base;
    s32 *tim_groups;
    s32 count;
    s32 *ptr;
    s32 off;

    if (slot_object->load_status == 0) {
        return;
    }

    slot_object->load_status = 0;

    if (slot_object->staging_buffer != 0) {
        block_base = (s32 *)slot_object->staging_buffer;
    } else {
        block_base = (s32 *)slot_object->asset_block;
    }

    count = block_base[0];
    tim_groups =
        (s32 *)((s32)block_base + (((u32)block_base[count] >> 2) << 2));

    if (ndata_IsPackLoaded() == 0)
        goto call_b644;
    if (slot_object->arena_id == 1)
        goto after_b644;

call_b644:
    snd_StopAllVoicesAndCloseSlotVab(slot_object->arena_id);

after_b644:
    if (count < 6)
        goto simple;

    {
        ptr =
            (s32 *)((s32)block_base + (((u32)block_base[count - 1] >> 2) << 2));
        ptr = (s32 *)((s32)ptr +
                      snd_LoadVabPack(
                          ptr, slot_object->arena_id * 3 + 1,
                          (s32 *)((s32)block_base +
                                  (((u32)block_base[count + 1] >> 2) << 2))));

        if (ptr == 0) {
            sys_StoreZeroPastScratchpadOnFailedCheck();
        }

        slot_object->load_flags.word |= 2;
        goto after_select;
    }

simple:
    ptr = tim_groups;

after_select:
    off = (s32)ptr - (s32)block_base;

    if (slot_object->staging_buffer != 0) {
        mem_MoveBlockRoundedUpToWords(
            slot_object->staging_buffer, slot_object->asset_block, off);
        block_base = (s32 *)slot_object->asset_block;
        if ((slot_object->load_flags.word >> 1) & 1) {
            snd_VabFakeOpen((s32)block_base - slot_object->staging_buffer,
                            slot_object->arena_id * 3 + 1);
        }
    }

    {
        s32 *block_1 =
            (s32 *)((s32)block_base + (((u32)block_base[1] >> 2) << 2));
        s32 *block_4;
        block_4 = (s32 *)((s32)block_base + (((u32)block_base[4] >> 2) << 2));
        slot_object->softbody_params =
            (s32)block_base + (((u32)block_base[3] >> 2) << 2);
        reloc_RelocateBlock(block_1, slot_object->transform_block_id);
        reloc_RelocateBlock(block_4, slot_object->projected_block_id);
    }

    {
        if (slot_object->arena_id == 1)
            goto case_1;
        if (slot_object->arena_id >= 2)
            goto done_cases;
        if (slot_object->arena_id != 0)
            goto done_cases;

        gpu_LoadTimGroup((u32 *)tim_groups, 0);
        if (gpu_GetFogClutObject() != 0)
            goto done_cases;
        gpu_OffsetBlockTexPrimStreams(
            slot_object->transform_block_id, 0, 0, -0x140, 0xE8);
        gpu_LoadTimGroupSplitOffset((u32 *)tim_groups, 0, 0, 0, -0x140, 0xF0);
        goto done_cases;

    case_1:
        gpu_LoadTimGroupOffset((u32 *)tim_groups, 0, 0x80, 0);
        if (gpu_GetFogClutObject() != 1)
            goto case_1_else;
        gpu_OffsetBlockTexPrimStreams(
            slot_object->transform_block_id, 0x80, 0, -0x140, 0xE8);
        gpu_LoadTimGroupSplitOffset(
            (u32 *)tim_groups, 0, 0x80, 0, -0x140, 0xF0);
        goto case_1_done;

    case_1_else:
        gpu_OffsetBlockTexPrimStreamsByPages(
            slot_object->transform_block_id, 2, 0, 2, 0);

    case_1_done:
        gpu_BackUpFixedSlotVramRowsAndRecordAssetIndex(slot_object);
    }

done_cases:
    DrawSync(0);
    gpu_LoadRequestedAvailableTimGroupsForSlot(
        slot_object->arena_id, slot_object->asset_index,
        g_slot_tim_group_requests[slot_object->arena_id], (s32)tim_groups);

    {
        slot_object->load_flags.word =
            (slot_object->load_flags.word & 0xFFE0FFFF) |
            ((g_slot_tim_group_requests[slot_object->arena_id] & 0x1F) << 16);
        g_slot_tim_group_requests[slot_object->arena_id] = 0;
    }

    DrawSync(0);
    slot_object->softbody_block = (void *)(slot_object->asset_block + off);
    slot_ResizeAssetBlockAndUpdatePeak(slot_object->arena_id, off);
}

/* The 21 node-template records at 0x80094B96 follow an s16 -1 at 0x80094B94
 * that no code reads. gcc word-aligns every top-level array and struct, so no
 * object of this file can start at 0x80094B96: the -1 and the records are one
 * word-aligned table, g_node_hierarchy_templates (it ends at
 * g_object_scale_factors). */
typedef struct {
    s16 unk_00;
    Unk80094B96Rec recs[21];
} Unk80094B94Table;

extern Unk80094B94Table g_node_hierarchy_templates;

void node_InitObjectHierarchyFromTemplates(Unk80045878Obj *slot_object) {
    s16 *tbl;
    s32 count;
    s16 scale_index;

    tbl = g_object_scale_factors;
    count = 0;
    while (*tbl++ != -1) {
        count++;
    }

    scale_index = slot_object->asset_index;
    if (scale_index < count) {
        slot_object->position_scale = g_object_scale_factors[scale_index];
    } else {
        slot_object->position_scale = 0x1000;
    }

    {
        Unk80045878Node *nodes = slot_object->transform_nodes;
        Unk80045878Node *node_cursor = slot_object->transform_nodes;
        s32 i = 0;

        do {
            s16 parent_node_index;
            node_cursor->node.node_kind = 0;
            node_cursor->node.primitive_table_select = 0;
            node_cursor->node.rotation_order = 0;
            node_cursor->node.reloc_entry_index = -1;
            parent_node_index = g_node_hierarchy_templates.recs[i].parent_index;
            if (parent_node_index != -1) {
                node_cursor->node.parent_node = &nodes[parent_node_index].node;
            } else {
                node_cursor->node.parent_node = 0;
            }
            node_cursor->node.work.t[0] =
                (g_node_hierarchy_templates.recs[i].node_translation[0] *
                 slot_object->position_scale) >>
                12;
            node_cursor->node.work.t[1] =
                (g_node_hierarchy_templates.recs[i].node_translation[1] *
                 slot_object->position_scale) >>
                12;
            node_cursor->node.work.t[2] =
                (g_node_hierarchy_templates.recs[i].node_translation[2] *
                 slot_object->position_scale) >>
                12;
            node_cursor->node.xf.rot.vx = 0;
            node_cursor->node.xf.rot.vy = 0;
            node_cursor->node.xf.rot.vz = 0;
            node_cursor->node.matrix_state = 0;
            node_cursor->node.light_setup_index =
                slot_object->node_light_setup_index;
            node_cursor->node.reloc_block_id = slot_object->transform_block_id;
            node_cursor++;
            i++;
        } while (i < 21);
    }

    node_RefreshObjectMatrixPointerTables(slot_object);
}

void node_RefreshObjectMatrixPointerTables(Unk80045878Obj *slot_object) {
    s32 matrix_ptr_index;
    Unk80045878Node *n;

    matrix_ptr_index = 0;
    slot_object->unk_18F4[0] = &slot_object->transform_nodes[6].node.xf.mat;
    slot_object->unk_18F4[1] = &slot_object->transform_nodes[5].node.xf.mat;
    slot_object->unk_18F4[2] = &slot_object->transform_nodes[4].node.xf.mat;
    slot_object->unk_18F4[3] = &slot_object->transform_nodes[10].node.xf.mat;
    slot_object->unk_18F4[4] = &slot_object->transform_nodes[9].node.xf.mat;
    slot_object->unk_18F4[5] = &slot_object->transform_nodes[8].node.xf.mat;
    slot_object->unk_18F4[6] = &slot_object->transform_nodes[3].node.xf.mat;
    slot_object->unk_18F4[7] = &slot_object->transform_nodes[2].node.xf.mat;
    slot_object->unk_18F4[8] = &slot_object->transform_nodes[17].node.xf.mat;
    slot_object->unk_18F4[9] = &slot_object->transform_nodes[16].node.xf.mat;
    slot_object->unk_18F4[10] = &slot_object->transform_nodes[14].node.xf.mat;
    slot_object->unk_18F4[11] = &slot_object->transform_nodes[13].node.xf.mat;
    slot_object->unk_18F4[12] = &slot_object->transform_nodes[12].node.xf.mat;
    slot_object->unk_18F4[13] = &slot_object->transform_nodes[1].node.xf.mat;
    slot_object->unk_18F4[14] = &slot_object->transform_nodes[7].node.xf.mat;
    slot_object->unk_18F4[15] = &slot_object->transform_nodes[11].node.xf.mat;
    slot_object->unk_18F4[16] = &slot_object->transform_nodes[18].node.xf.mat;
    slot_object->unk_18F4[17] = &slot_object->transform_nodes[15].node.xf.mat;
    slot_object->unk_18F4[18] = &slot_object->transform_nodes[19].node.xf.mat;
    slot_object->unk_18F4[19] = &slot_object->transform_nodes[20].node.xf.mat;
    n = &slot_object->transform_nodes[1];
    do {
        slot_object->node_matrix_ptrs[matrix_ptr_index] = &n->node.xf.mat;
        n++;
        matrix_ptr_index += 1;
    } while (matrix_ptr_index < 0x14);
}

void node_BuildIdToNodeMapWithDuplicates(Unk80045878Obj *slot_object) {
    s32 node_used[18];
    Unk80045878Node *duplicate_cursor;
    Unk80045878Node *mapped_nodes;
    s32 *block_base;
    u16 *pair_cursor;
    s32 i;

    duplicate_cursor = slot_object->duplicate_nodes;
    mapped_nodes = &slot_object->transform_nodes[1];
    block_base = (s32 *)slot_object->asset_block;
    pair_cursor = (u16 *)((u8 *)block_base + block_base[2]);

    i = 0x11;
    do {
        node_used[i] = 0;
        i--;
    } while (i >= 0);
    i = 0x13;
    do {
        slot_object->node_by_id[i] = 0;
        i--;
    } while (i >= 0);

    while (*pair_cursor != 0xFFFF) {
        s32 id = *pair_cursor++;
        s32 node_index = *pair_cursor++;

        if (node_used[node_index] == 0) {
            mapped_nodes[node_index].node.reloc_entry_index = id;
            node_used[node_index] = 1;
            slot_object->node_by_id[id] = &mapped_nodes[node_index];
        } else {
            *duplicate_cursor = mapped_nodes[node_index];
            duplicate_cursor->node.reloc_entry_index = id;
            duplicate_cursor->unk58 = (s32)&mapped_nodes[node_index];
            slot_object->node_by_id[id] = duplicate_cursor;
            duplicate_cursor++;
        }
    }
    duplicate_cursor->unk58 = 0;
}

void node_BuildProjectedNodeList(Unk80045878Obj *slot_object) {
    s32 i;
    Unk80045878Node *node_cursor;
    /* Once-written named intermediate (no-new-park-categories entry 6):
     * parents[i].node is transform_nodes[i + 1].node, used as parent_node.
     * &slot_object->transform_nodes[i + 1].node in place: score 3. */
    Unk80045878Node *parents;
    /* Once-written per iteration named intermediate (entry 6): id points at
     * record i's projected_entry_index, tested for -1 and stored into
     * node_cursor->node.reloc_entry_index.
     * g_node_hierarchy_templates.recs[i].projected_entry_index read in place:
     * score 5. */
    s16 *id;

    node_cursor = slot_object->projected_nodes;
    parents = &slot_object->transform_nodes[1];
    for (i = 0; i < 18; i++) {
        id = &g_node_hierarchy_templates.recs[i].projected_entry_index;
        if (*id != -1) {
            node_cursor->node.reloc_entry_index = *id;
            node_cursor->node.node_kind = 3;
            node_cursor->node.primitive_table_select = 0;
            node_cursor->node.rotation_order = 0;
            node_cursor->node.parent_node = &parents[i].node;
            node_cursor->node.matrix_state = 1;
            node_cursor->node.light_setup_index = 0;
            node_cursor->node.reloc_block_id = slot_object->projected_block_id;
            node_cursor->unk58 = 0;
            node_cursor++;
        }
    }
    node_cursor->node.reloc_entry_index = -1;
}

/* .data 0x80094AF4..0x80094CFC, this file's whole .data (d_80DF8.c's
 * g_softbody_face_record_strides precedes it, 31548.c's
 * g_node_pose_rotation_indices follows), in address order. Defined after every
 * function, so no body sees a definition where it saw an extern.
 * g_gpu_draw_move_table: gpu_AddDrawMove's seven 6-entry rows. */
u16 g_gpu_draw_move_table[42] = {
    0x0290, 0x0000, 0x0010, 0x0020, 0x0280, 0x0000, 0x02A0, 0x0000, 0x0010,
    0x0020, 0x0280, 0x0000, 0x02B0, 0x0000, 0x0010, 0x0020, 0x0280, 0x0000,
    0x0290, 0x0020, 0x0010, 0x0028, 0x0280, 0x0020, 0x02A0, 0x0020, 0x0010,
    0x0028, 0x0280, 0x0020, 0x02C0, 0x0040, 0x0010, 0x0020, 0x0280, 0x0000,
    0x02D0, 0x0040, 0x0010, 0x0028, 0x0280, 0x0020,
};

/* gpu_QueueSlotDrawMovesForSelectedSet's per-model masks, indexed by
 * asset_index. */
u8 g_gpu_asset_draw_move_masks[64] = {
    0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F,
    0x1C, 0x1F, 0x1F, 0x00, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F,
    0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x00, 0x1F, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x1C, 0x1F, 0x1F, 0x1F, 0x00, 0x1F,
};

/* Per arena_id: slot_ClearObjectPointersAndTextureRequests clears the three
 * slots. */
s32 g_slot_tim_group_requests[3] = {-1, -1, -1};

Unk80094B94Table g_node_hierarchy_templates = {
    -1,
    {
        {-1, {0, 0, 0}, -1},   {0, {0, 0, 0}, 0},        {1, {0, 10, 0}, 1},
        {2, {468, 0, 0}, -1},  {2, {289, 0, -100}, 2},   {4, {140, 0, 0}, 3},
        {5, {262, 0, 0}, 4},   {6, {211, 0, -3}, -1},    {2, {289, 0, 100}, 5},
        {8, {140, 0, 0}, 6},   {9, {262, 0, 0}, 7},      {10, {211, 0, -2}, 8},
        {1, {0, 10, 0}, 9},    {12, {180, 0, 110}, 10},  {13, {390, 0, 0}, 11},
        {14, {441, 0, 0}, 12}, {12, {180, 0, -109}, 13}, {16, {390, 0, 0}, 14},
        {17, {444, 0, 0}, -1}, {1, {0, 0, 0}, -1},       {1, {0, 0, 0}, 0},
    },
};

/* g_object_scale_factors, the position scales
 * node_InitObjectHierarchyFromTemplates counts up to the -1 at [70] and indexes
 * (9F9C.c reads it as size_scale); three zero entries follow the -1. */
u16 g_object_scale_factors[74] = {
    0x107A, 0x1000, 0x0F5C, 0x107A, 0x1000, 0x0F5C, 0x1000, 0x1000, 0x1000,
    0x1028, 0x10A3, 0x1000, 0x1000, 0x1000, 0x1000, 0x1051, 0x1000, 0x0F85,
    0x1051, 0x1000, 0x0F85, 0x1000, 0x1000, 0x1051, 0x0F0A, 0x1000, 0x1051,
    0x1000, 0x107A, 0x1000, 0x1000, 0x107A, 0x1000, 0x0F5C, 0x1000, 0x1000,
    0x1000, 0x1028, 0x10A3, 0x1000, 0x1000, 0x1051, 0x1000, 0x0F85, 0x1000,
    0x1000, 0x1051, 0x0F0A, 0x1000, 0x1051, 0x1000, 0x1000, 0x1000, 0x1000,
    0x1000, 0x1000, 0x1000, 0x1000, 0x1000, 0x1000, 0x1000, 0x1000, 0x1000,
    0x1000, 0x1000, 0x1000, 0x1000, 0x1000, 0x1000, 0x1000, 0xFFFF, 0x0000,
    0x0000, 0x0000,
};

/* Q65: this file's initialized small data (.sdata), in address order; values
 * from the original EXE. */
s32 g_gpu_draw_move_bank = -1;
