/* 12 game functions, among them gpu_AddDrawMove. .text 0x800401CC (ROM
 * 0x309CC). Start boundary: G8 (cc1 -G8 by proof). */
#include "common.h"
#define INCLUDE_ASM_USE_MACRO_INC 1
#include "include_asm.h"
#include "bb2.h"

/* Unprototyped: func_80040594 passes arena_id * 3 + 1 as an int; the s16
 * prototype 4BC3C defines it with narrows that argument before the call. */
extern s32 snd_LoadVabPack();
extern s32 g_gpu_draw_move_bank;
extern u16 g_gpu_draw_move_table[];
extern u8 D_80094B48[];

/* Q65: this file's statics (.sbss, allocated per file in link order by
 * PSYLINK), in address order. */
/* Cursor in the 10-packet bank g_gpu_draw_move_packets[D_800A36AC & 1]. */
static DR_MOVE *g_gpu_draw_move_cursor;

void gpu_AddDrawMove(s32 a0, s32 a1) {
    s32 var_0;
    RECT buf;
    u16 *tbl;
    s16 u, v;

    var_0 = D_800A36AC & 1;
    if (var_0 != g_gpu_draw_move_bank) {
        g_gpu_draw_move_cursor = g_gpu_draw_move_packets[var_0];
        g_gpu_draw_move_bank = var_0;
    }
    if (g_gpu_draw_move_cursor !=
        g_gpu_draw_move_packets[g_gpu_draw_move_bank] + 10) {
        tbl = g_gpu_draw_move_table + a1 * 6;
        buf.x = *tbl++;
        buf.y = *tbl++;
        buf.w = *tbl++;
        buf.h = *tbl++;
        u = *tbl++;
        v = *tbl;
        if (a0 != 0) {
            buf.x = buf.x + 0x80;
            u = u + 0x80;
        }
        SetDrawMove(g_gpu_draw_move_cursor, &buf, u, v);
        addPrim(&D_800A378C[0x3FFC / 4], g_gpu_draw_move_cursor);
        g_gpu_draw_move_cursor++;
    }
}

void func_80040304(s32 a0, s32 a1) {
    Unk80045878Obj *ptr;
    s32 mask;
    s32 i;

    ptr = func_8004153C(a0);
    if (ptr != 0) {
        switch (a1) {
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
            gpu_AddDrawMove(a0, 5);
            gpu_AddDrawMove(a0, 6);
            mask = 0;
            break;
        }
        mask = mask & D_80094B48[ptr->unk_08];
        i = 0;
        do {
            if (mask & 1) {
                gpu_AddDrawMove(a0, 4 - i);
            }
            i++;
            mask >>= 1;
        } while (i < 5);
    }
}

void func_80040400(Unk80045878Node *a0, Unk80045878Node *a1, s16 a2) {
    while (a1->node.unk2 != -1) {
        a1++;
    }
    a1->node.unk2 = 2;
    a1->node.unk0 = 3;
    a1->node.unk1 = 0;
    a1->node.parent_node = &a0[6].node;
    a1->node.matrix_state = 1;
    a1->node.rotation_order = 0;
    a1->node.light_setup_index = 0;
    a1->node.unk4 = a2;
    a1->unk58 = 0;
    a1[1].node.unk2 = -1;
}

Unk80045878Node *func_8004046C(s32 a0, s32 a1) {
    Unk80045878Obj *base = func_8004153C(a0);
    return base->node_by_id[a1];
}

void func_800404A0(Unk80045878Node *a0, s32 a1) {
    if (a0->node.unk2 == -1) {
        return;
    }
    do {
        a0->unk58 = a1;
        a0++;
    } while (a0->node.unk2 != -1);
}

void func_800404D8(void) {
    s32 i;
    for (i = 0; i < 3; i++) {
        D_800A9A10[i] = 0;
        D_80094B88[i] = 0;
    }
}

extern void func_80040594(Unk80045878Obj *);
extern void func_800408F8(Unk80045878Obj *);
extern void func_80040B44(Unk80045878Obj *);
/* Not the definition's spelling: snd_VabFakeOpen takes an s16 second parameter,
 * but this call passes it unextended (an s16 prototype adds sll/sra). */
extern s32 snd_VabFakeOpen(s32, s32);

Unk80045878Obj *func_80040510(s32 a0, s32 a1, s32 a2) {
    Unk80045878Obj *ptr;
    ptr = func_80045878(a0, a1, a2);
    D_800A9A10[a0] = ptr;
    func_80040594(ptr);
    func_800408F8(ptr);
    func_80040B44(ptr);
    softbody_SetupObject(ptr, 1);
    softbody_RequestRebuild(ptr);
    func_80040CB8(ptr);
    func_8003E120();
    return ptr;
}

void func_80040594(Unk80045878Obj *a0) {
    s32 *var_0;
    s32 *var_1;
    s32 count;
    s32 *ptr;
    s32 off;

    if (a0->load_status == 0) {
        return;
    }

    a0->load_status = 0;

    if (a0->unk_20 != 0) {
        var_0 = (s32 *)a0->unk_20;
    } else {
        var_0 = (s32 *)a0->unk_1C;
    }

    count = var_0[0];
    var_1 = (s32 *)((s32)var_0 + (((u32)var_0[count] >> 2) << 2));

    if (ndata_IsPackLoaded() == 0)
        goto call_b644;
    if (a0->arena_id == 1)
        goto after_b644;

call_b644:
    func_8005B644(a0->arena_id);

after_b644:
    if (count < 6)
        goto simple;

    {
        ptr = (s32 *)((s32)var_0 + (((u32)var_0[count - 1] >> 2) << 2));
        ptr = (s32 *)((s32)ptr +
                      snd_LoadVabPack(
                          ptr, a0->arena_id * 3 + 1,
                          (s32 *)((s32)var_0 +
                                  (((u32)var_0[count + 1] >> 2) << 2))));

        if (ptr == 0) {
            func_80052C10();
        }

        a0->unk_00.word |= 2;
        goto after_select;
    }

simple:
    ptr = var_1;

after_select:
    off = (s32)ptr - (s32)var_0;

    if (a0->unk_20 != 0) {
        func_800520B8(a0->unk_20, a0->unk_1C, off);
        var_0 = (s32 *)a0->unk_1C;
        if ((a0->unk_00.word >> 1) & 1) {
            snd_VabFakeOpen((s32)var_0 - a0->unk_20, a0->arena_id * 3 + 1);
        }
    }

    {
        s32 *block_1 = (s32 *)((s32)var_0 + (((u32)var_0[1] >> 2) << 2));
        s32 *block_4;
        block_4 = (s32 *)((s32)var_0 + (((u32)var_0[4] >> 2) << 2));
        a0->softbody_params = (s32)var_0 + (((u32)var_0[3] >> 2) << 2);
        reloc_RelocateBlock(block_1, a0->unk_14);
        reloc_RelocateBlock(block_4, a0->unk_16);
    }

    {
        if (a0->arena_id == 1)
            goto case_1;
        if (a0->arena_id >= 2)
            goto done_cases;
        if (a0->arena_id != 0)
            goto done_cases;

        gpu_LoadTimGroup((u32 *)var_1, 0);
        if (gpu_GetFogClutObject() != 0)
            goto done_cases;
        gpu_OffsetBlockTexPrimStreams(a0->unk_14, 0, 0, -0x140, 0xE8);
        gpu_LoadTimGroupSplitOffset((u32 *)var_1, 0, 0, 0, -0x140, 0xF0);
        goto done_cases;

    case_1:
        gpu_LoadTimGroupOffset((u32 *)var_1, 0, 0x80, 0);
        if (gpu_GetFogClutObject() != 1)
            goto case_1_else;
        gpu_OffsetBlockTexPrimStreams(a0->unk_14, 0x80, 0, -0x140, 0xE8);
        gpu_LoadTimGroupSplitOffset((u32 *)var_1, 0, 0x80, 0, -0x140, 0xF0);
        goto case_1_done;

    case_1_else:
        gpu_OffsetBlockTexPrimStreamsByPages(a0->unk_14, 2, 0, 2, 0);

    case_1_done:
        func_80041AC8(a0);
    }

done_cases:
    DrawSync(0);
    func_80041988(
        a0->arena_id, a0->unk_08, D_80094B88[a0->arena_id], (s32)var_1);

    {
        a0->unk_00.word = (a0->unk_00.word & 0xFFE0FFFF) |
                          ((D_80094B88[a0->arena_id] & 0x1F) << 16);
        D_80094B88[a0->arena_id] = 0;
    }

    DrawSync(0);
    a0->unk_24 = (void *)(a0->unk_1C + off);
    func_80045A28(a0->arena_id, off);
}

/* The 21 node-template records at 0x80094B96 follow an s16 -1 at 0x80094B94
 * that no code reads. gcc word-aligns every top-level array and struct, so no
 * object of this file can start at 0x80094B96: the -1 and the records are one
 * word-aligned table, D_80094B94 (it ends at D_80094C68). */
typedef struct {
    s16 unk_00;
    Unk80094B96Rec recs[21];
} Unk80094B94Table;

extern Unk80094B94Table D_80094B94;

void func_800408F8(Unk80045878Obj *a0) {
    s16 *tbl;
    s32 count;
    s16 idx;

    tbl = D_80094C68;
    count = 0;
    while (*tbl++ != -1) {
        count++;
    }

    idx = a0->unk_08;
    if (idx < count) {
        a0->position_scale = D_80094C68[idx];
    } else {
        a0->position_scale = 0x1000;
    }

    {
        Unk80045878Node *nodes = a0->transform_nodes;
        Unk80045878Node *p = a0->transform_nodes;
        s32 i = 0;

        do {
            s16 v1;
            p->node.unk0 = 0;
            p->node.unk1 = 0;
            p->node.rotation_order = 0;
            p->node.unk2 = -1;
            v1 = D_80094B94.recs[i].unk_00;
            if (v1 != -1) {
                p->node.parent_node = &nodes[v1].node;
            } else {
                p->node.parent_node = 0;
            }
            p->node.work.t[0] =
                (D_80094B94.recs[i].unk_02[0] * a0->position_scale) >> 12;
            p->node.work.t[1] =
                (D_80094B94.recs[i].unk_02[1] * a0->position_scale) >> 12;
            p->node.work.t[2] =
                (D_80094B94.recs[i].unk_02[2] * a0->position_scale) >> 12;
            p->node.xf.rot.vx = 0;
            p->node.xf.rot.vy = 0;
            p->node.xf.rot.vz = 0;
            p->node.matrix_state = 0;
            p->node.light_setup_index = a0->unk_10;
            p->node.unk4 = a0->unk_14;
            p++;
            i++;
        } while (i < 21);
    }

    func_80040A78(a0);
}

void func_80040A78(Unk80045878Obj *arg0) {
    s32 var_a1;
    Unk80045878Node *n;

    var_a1 = 0;
    arg0->unk_18F4[0] = &arg0->transform_nodes[6].node.xf.mat;
    arg0->unk_18F4[1] = &arg0->transform_nodes[5].node.xf.mat;
    arg0->unk_18F4[2] = &arg0->transform_nodes[4].node.xf.mat;
    arg0->unk_18F4[3] = &arg0->transform_nodes[10].node.xf.mat;
    arg0->unk_18F4[4] = &arg0->transform_nodes[9].node.xf.mat;
    arg0->unk_18F4[5] = &arg0->transform_nodes[8].node.xf.mat;
    arg0->unk_18F4[6] = &arg0->transform_nodes[3].node.xf.mat;
    arg0->unk_18F4[7] = &arg0->transform_nodes[2].node.xf.mat;
    arg0->unk_18F4[8] = &arg0->transform_nodes[17].node.xf.mat;
    arg0->unk_18F4[9] = &arg0->transform_nodes[16].node.xf.mat;
    arg0->unk_18F4[10] = &arg0->transform_nodes[14].node.xf.mat;
    arg0->unk_18F4[11] = &arg0->transform_nodes[13].node.xf.mat;
    arg0->unk_18F4[12] = &arg0->transform_nodes[12].node.xf.mat;
    arg0->unk_18F4[13] = &arg0->transform_nodes[1].node.xf.mat;
    arg0->unk_18F4[14] = &arg0->transform_nodes[7].node.xf.mat;
    arg0->unk_18F4[15] = &arg0->transform_nodes[11].node.xf.mat;
    arg0->unk_18F4[16] = &arg0->transform_nodes[18].node.xf.mat;
    arg0->unk_18F4[17] = &arg0->transform_nodes[15].node.xf.mat;
    arg0->unk_18F4[18] = &arg0->transform_nodes[19].node.xf.mat;
    arg0->unk_18F4[19] = &arg0->transform_nodes[20].node.xf.mat;
    n = &arg0->transform_nodes[1];
    do {
        arg0->node_matrix_ptrs[var_a1] = &n->node.xf.mat;
        n++;
        var_a1 += 1;
    } while (var_a1 < 0x14);
}

void func_80040B44(Unk80045878Obj *arg0) {
    s32 var_0[18];
    Unk80045878Node *t5;
    Unk80045878Node *t7;
    s32 *v1;
    u16 *t3;
    s32 i;

    t5 = arg0->duplicate_nodes;
    t7 = &arg0->transform_nodes[1];
    v1 = (s32 *)arg0->unk_1C;
    t3 = (u16 *)((u8 *)v1 + v1[2]);

    i = 0x11;
    do {
        var_0[i] = 0;
        i--;
    } while (i >= 0);
    i = 0x13;
    do {
        arg0->node_by_id[i] = 0;
        i--;
    } while (i >= 0);

    while (*t3 != 0xFFFF) {
        s32 id = *t3++;
        s32 a3 = *t3++;

        if (var_0[a3] == 0) {
            t7[a3].node.unk2 = id;
            var_0[a3] = 1;
            arg0->node_by_id[id] = &t7[a3];
        } else {
            *t5 = t7[a3];
            t5->node.unk2 = id;
            t5->unk58 = (s32)&t7[a3];
            arg0->node_by_id[id] = t5;
            t5++;
        }
    }
    t5->unk58 = 0;
}

void func_80040CB8(Unk80045878Obj *arg0) {
    s32 i;
    Unk80045878Node *p;
    /* Once-written named intermediate (no-new-park-categories entry 6):
     * parents[i].node is transform_nodes[i + 1].node, used as parent_node.
     * &arg0->transform_nodes[i + 1].node in place: score 3. */
    Unk80045878Node *parents;
    /* Once-written per iteration named intermediate (entry 6): id points at
     * record i's unk_08, tested for -1 and stored into p->node.unk2.
     * D_80094B94.recs[i].unk_08 read in place: score 5. */
    s16 *id;

    p = arg0->projected_nodes;
    parents = &arg0->transform_nodes[1];
    for (i = 0; i < 18; i++) {
        id = &D_80094B94.recs[i].unk_08;
        if (*id != -1) {
            p->node.unk2 = *id;
            p->node.unk0 = 3;
            p->node.unk1 = 0;
            p->node.rotation_order = 0;
            p->node.parent_node = &parents[i].node;
            p->node.matrix_state = 1;
            p->node.light_setup_index = 0;
            p->node.unk4 = arg0->unk_16;
            p->unk58 = 0;
            p++;
        }
    }
    p->node.unk2 = -1;
}

/* .data 0x80094AF4..0x80094CFC, this file's whole .data (d_80DF8.c's
 * D_80094AEC precedes it, 31548.c's D_80094CFC follows), in address order.
 * Defined after every function, so no body sees a definition where it saw an
 * extern. g_gpu_draw_move_table: gpu_AddDrawMove's seven 6-entry rows. */
u16 g_gpu_draw_move_table[42] = {
    0x0290, 0x0000, 0x0010, 0x0020, 0x0280, 0x0000, 0x02A0, 0x0000, 0x0010,
    0x0020, 0x0280, 0x0000, 0x02B0, 0x0000, 0x0010, 0x0020, 0x0280, 0x0000,
    0x0290, 0x0020, 0x0010, 0x0028, 0x0280, 0x0020, 0x02A0, 0x0020, 0x0010,
    0x0028, 0x0280, 0x0020, 0x02C0, 0x0040, 0x0010, 0x0020, 0x0280, 0x0000,
    0x02D0, 0x0040, 0x0010, 0x0028, 0x0280, 0x0020,
};

/* func_80040304's per-model masks, indexed by unk_08. */
u8 D_80094B48[64] = {
    0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F,
    0x1C, 0x1F, 0x1F, 0x00, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F,
    0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x00, 0x1F, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x1C, 0x1F, 0x1F, 0x1F, 0x00, 0x1F,
};

/* Per arena_id: func_800404D8 clears the three slots. */
s32 D_80094B88[3] = {-1, -1, -1};

Unk80094B94Table D_80094B94 = {
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

/* D_80094C68, the position scales
 * func_800408F8 counts up to the -1 at [70] and indexes (9F9C.c reads it as
 * size_scale); three zero entries follow the -1. */
u16 D_80094C68[74] = {
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
