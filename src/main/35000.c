/* 44 game functions. .text 0x80044800 (ROM 0x35000). Start boundary: GP (a
 * per-file gp split, Q65). */
#include "common.h"
#define INCLUDE_ASM_USE_MACRO_INC 1
#include "include_asm.h"
#include "bb2.h"
#include <psxsdk/libgte.h>

void gpu_OffsetBlockTexPrimStream();

void func_80044800(void) {
    SVECTOR var_0;
    MATRIX mat;
    Unk800A9CF8Entry *rec;
    Unk800A9CF8Entry *ent;
    s32 i;
    s32 var_5;
    s32 angle;
    s16 *var_7;
    s16 cos_val;
    s16 sin_val;
    s32 cz;
    s32 sz;
    s32 cx;
    s32 sx;
    s32 last;
    s32 var_15;
    void **list;

    rec = (Unk800A9CF8Entry *)D_800A9CF8.unkC;
    for (i = 0; i < D_800A9CF8.unk6; rec++, i++) {
        var_5 = rec->unk58;
        if (var_5 < 0)
            continue;
        if (var_5 < D_800A9CF8.unk2) {
            var_0.vx = 0;
            angle = rec->unk5C;
            var_0.vz = 0;
            var_7 = (s16 *)(D_800A9CF8.unk8 + var_5 * 12);
            var_0.vy = angle;
            math_RotMatrixZYX(&var_0, &mat);
            var_0.vx = *var_7++;
            var_0.vy = *var_7++;
            var_0.vz = *var_7++;
            rec->node.xf.rot.vx = *var_7++;
            rec->node.xf.rot.vy = *var_7++;
            rec->node.xf.rot.vz = *var_7++;
            rec->node.matrix_state = 0;
            node_UpdateWorldMatrix(&rec->node);
            MulMatrix2(&mat, &rec->node.xf.mat);
            cos_val = Judge[(angle + 0x400) & 0xFFF];
            cz = cos_val * var_0.vz;
            sin_val = Judge[angle & 0xFFF];
            sz = sin_val * var_0.vz;
            cx = cos_val * var_0.vx;
            sx = sin_val * var_0.vx;
            rec->node.xf.mat.t[0] += (sz + cx) >> 12;
            rec->node.xf.mat.t[1] += var_0.vy;
            rec->node.xf.mat.t[2] += (cz - sx) >> 12;
            var_5++;
            if (var_5 >= D_800A9CF8.unk2) {
                rec->unk60 = 0x1000;
            }
            rec->unk58 = var_5;
            if (D_800A9CF8.unk4 == 0x12) {
                /* FAKE: rec is reused (restored from ent below) for the paired
                 * func_8003F1D4 entry, an Unk800A6690Rec reached only
                 * through its node member (both records start with the node); a
                 * separate Unk80101DF0Record * local scores 70. */
                ent = rec;
                rec = (Unk800A9CF8Entry *)((Unk800A6690Rec *)D_800A9CF8.unk10 +
                                           i);
                last = D_800A9CF8.unk2 - 1;
                var_7 = (s16 *)(D_800A9CF8.unk8 + (var_5 + last) * 12);
                var_0.vx = *var_7++;
                var_0.vy = *var_7++;
                var_0.vz = *var_7++;
                rec->node.xf.rot.vx = *var_7++;
                rec->node.xf.rot.vy = *var_7++;
                rec->node.xf.rot.vz = *var_7++;
                rec->node.matrix_state = 0;
                node_UpdateWorldMatrix(&rec->node);
                rec->node.xf.mat.t[0] += var_0.vx;
                rec->node.xf.mat.t[1] += var_0.vy;
                rec->node.xf.mat.t[2] += var_0.vz;
                rec = ent;
            }
        } else {
            var_15 = rec->unk60 - 0x40;
            if (var_15 < 0) {
                var_15 = 0;
            }
            if (var_15 == 0 && D_800A9CF8.unk4 != 4) {
                rec->unk58 = -2;
            }
            rec->unk60 = var_15;
        }
        list = g_draw_queue_cursor;
        g_draw_queue_cursor = list + 1;
        *list = rec;
    }
}

void func_80044B30(s32 a0, s32 a1) {
    Unk800A9CF8Entry *p;

    if (a0 >= D_800A9CF8.unk6)
        return;

    p = (Unk800A9CF8Entry *)D_800A9CF8.unkC + a0;
    if (p->unk58 != -1)
        return;

    switch (D_800A9CF8.unk4) {
    case 4:
        if (a0 == 0) {
            a1 = 0x800;
        }
        if (a0 == 1) {
            a1 = 0;
        }
        break;
    case 0x12:
        a1 = 0;
        break;
    }
    p->unk5C = a1;
    p->unk58 = 0;
    /* FAKE: p is reused for the paired func_8003F1D4 entry, an
     * Unk800A6690Rec reached only through its node member (both records start
     * with the node); a separate Unk80101DF0Record * local scores 4. */
    p = (Unk800A9CF8Entry *)((Unk800A6690Rec *)D_800A9CF8.unk10 + a0);
    p->node.unk2 = 1;
    p->node.unk4 = D_800A9CF8.unk0;
    p->node.xf.rot.vy = a1;
    p->node.unk0 = 0;
    p->node.parent_node = 0;
    p->node.xf.rot.vx = 0;
    p->node.xf.rot.vz = 0;
    p->node.matrix_state = 1;
    D_800F66A0[p->node.rotation_order](&p->node.xf.rot, &p->node.xf.mat);
    if (D_800A9CF8.unk4 == 0x12) {
        p->node.work.t[0] = p->node.xf.mat.t[0];
        p->node.work.t[1] = p->node.xf.mat.t[1];
        p->node.work.t[2] = p->node.xf.mat.t[2];
    }
}

void func_80044C70(s32 a0) {
    reloc_RebaseBlock((s32)D_800A9CF8.unk0, a0);
    D_800A9CF8.unkC += a0;
    D_800A9CF8.unk8 += a0;
}

void func_80044CCC(s16 *a0, s16 *a1, s32 a2, s32 a3) {
    s32 sp18[3];
    s32 sp28[3];
    s32 angle;
    s32 var_3;

    sp18[1] = a0[0];
    angle = a0[1];
    var_3 = a0[2];
    sp18[0] = (rsin(angle) * var_3) >> 12;
    sp18[2] = (rcos(angle) * var_3) >> 12;
    sp18[1] = -sp18[1];
    sp18[2] = -sp18[2];

    sp28[1] = a1[0];
    angle = a1[1];
    var_3 = a1[2];
    sp28[0] = (rsin(angle) * var_3) >> 12;
    sp28[2] = (rcos(angle) * var_3) >> 12;
    sp28[1] = -sp28[1];
    sp28[2] = -sp28[2];

    LoadAverage12(sp18, sp28, 0x1000 - a2, a2, a3);
}

void math_LerpShortVectorsNegateYZ(s16 *a0, s16 *a1, s32 a2, s32 a3) {
    s32 sp18[3];
    s32 sp28[3];
    sp18[0] = a0[0];
    a0++;
    sp18[1] = -a0[0];
    sp18[2] = -a0[1];
    sp28[0] = a1[0];
    a1++;
    sp28[1] = -a1[0];
    sp28[2] = -a1[1];
    LoadAverage12(sp18, sp28, 0x1000 - a2, a2, a3);
}

s32 func_80044E64(void) { return 0x25; }

s32 func_80044E6C(void) { return 0x26; }

typedef struct {
    s16 start_sector;
    s16 length_sectors;
} NdataInfEntry;

extern NdataInfEntry g_ndata_inf[];

void ndata_Read(s32 a0, s32 a1) {
    cdrom_WaitIdle();
    cdrom_StartReadAt(
        0, a1, g_ndata_inf[a0].start_sector, g_ndata_inf[a0].length_sectors);
    cdrom_WaitIdle();
}

void func_80044ED8(s32 a0, s32 a1) {
    if (a0 >= 0x1F) {
        a0 -= 0x1B;
    }
    if (!ndata_ReadFromPack(a0, a1)) {
        ndata_Read(a0, a1);
    }
}

extern void ndata_Read(s32, s32);

void func_80044F30(s32 a0, s32 a1) { ndata_Read(a0 + 0x27, a1); }

void func_80044F50(s32 a0, s32 a1, s32 a2) {
    if (!a0) {
        ndata_Read(a1 + 0x83, a2);
    } else {
        ndata_Read(a1 + 0x10C, a2);
    }
}

void func_80044F80(s32 a0, s32 a1) { ndata_Read(a0 + 0x4D, a1); }

extern s32 D_800A3240;
extern char D_8001528C[];

s32 ndata_ReadChecked(s32 a0, s32 a1) {
    s32 size;
    s32 len;

    size = a1 - (s32)arena_GetBase();
    if (size >= 0 && size < arena_GetSize()) {
        if (D_800A3240 != 0) {
            size = g_ndata_inf[a0].length_sectors << 11;
        } else {
            size = 0;
        }
        len = arena_GetFreeSize();
        if (len < size) {
            printf(D_8001528C, a0, size - len);
            while (1) {
                func_800164F8();
            }
        }
    } else {
        size = g_ndata_inf[a0].length_sectors << 11;
    }
    ndata_Read(a0, a1);
    return size;
}

extern s32 arena_GetFreeSize(void);

s32 ndata_GetArenaRoomAfter(s32 a0) {
    s32 val = g_ndata_inf[a0].length_sectors << 11;
    return arena_GetFreeSize() - val;
}

/* Q65: this file's statics (.sbss), in address order. */
static s32 g_ndata_pack;
static s32 D_800A339C; /* not named by any code or data: size from the gap */
static s32 g_arena_free_ptr;
static s32 g_arena_free_size;
static s32 g_arena_peak;
static s32 g_arena_block_count;

void ndata_LoadPack(s32 a0, s32 a1) {
    ndata_Read(a0 + 0x25, a1);
    g_ndata_pack = a1;
    g_ndata_pack_loaded = 1;
}

extern s16 g_ndata_pack_index[];

s32 ndata_ReadFromPack(s32 a0, s32 a1) {
    s32 *a2_ptr;
    s32 v1;
    s32 *v0_ptr;
    if (g_ndata_pack_loaded == 0) {
        return 0;
    }
    a0 -= 0x1E;
    if ((u32)a0 >= 7) {
        return 0;
    }
    a0 = g_ndata_pack_index[a0];
    if (a0 == -1) {
        return 0;
    }
    a2_ptr = (s32 *)g_ndata_pack;
    if (*a2_ptr != 5) {
        g_ndata_pack_loaded = 0;
        return 0;
    }
    v0_ptr = (s32 *)(a0 * 4 + (s32)a2_ptr + 4);
    v1 = *v0_ptr;
    {
        s32 a0_new = (s32)a2_ptr + (((u32)v1 >> 2) << 2);
        s32 a2_val = v0_ptr[1] - v1;
        func_800520B8(a0_new, a1, a2_val);
    }
    return 1;
}

void ndata_InvalidatePack(void) { g_ndata_pack_loaded = 0; }

s32 ndata_IsPackLoaded(void) { return g_ndata_pack_loaded; }

void ndata_StartReadInf(void) { cdrom_StartReadAt(1, (s32)g_ndata_inf, 0, 2); }

void arena_Reset(void) {
    s32 i;
    for (i = 9; i >= 0; i--) {
        g_arena_blocks[i].id = -1;
    }
    g_arena_free_ptr = (s32)&g_arena;
    g_arena_free_size = 0x45000;
    g_arena_block_count = 0;
    g_arena_peak = 0;
    func_80049E1C();
}

void arena_UpdatePeak(s32 a0) {
    s32 v1;
    if (!a0) {
        a0 = g_arena_free_ptr;
    }
    a0 -= (s32)&g_arena;
    v1 = a0;
    if (a0 < g_arena_peak) {
        v1 = g_arena_peak;
    }
    g_arena_peak = v1;
    if (a0 > 0x44FFF) {
        func_80052C10();
    }
}

void arena_ShiftBlocks(s32 a0, s32 a1) {
    s32 sum;
    s32 i;
    s32 src;
    s32 dst;
    s32 count;

    sum = 0;
    src = g_arena_blocks[a0].unk4;
    count = g_arena_block_count;
    dst = src + a1;
    for (i = a0; i < count; i++) {
        sum += g_arena_blocks[i].amt;
    }
    if (sum != 0) {
        DrawSync(0);
        func_800520B8(src, dst, sum);
        for (i = a0; i < g_arena_block_count; i++) {
            g_arena_blocks[i].unk4 += a1;
            if (g_arena_blocks[i].fn != 0) {
                g_arena_blocks[i].fn(g_arena_blocks[i].id, a1);
            }
        }
    }
    g_arena_free_ptr += a1;
    g_arena_free_size -= a1;
}

/* Remove the table entry whose id == a0: undo its contribution via
   arena_ShiftBlocks(index + 1, -amt), shift the following records down one
   slot, clear the freed tail slot, decrement the count. */
void arena_Free(s32 a0) {
    s32 i;
    s32 j;
    s32 last;

    for (i = 0; i < g_arena_block_count; i++) {
        if (g_arena_blocks[i].id == a0) {
            arena_ShiftBlocks(i + 1, -g_arena_blocks[i].amt);
            if (i < g_arena_block_count - 1) {
                for (j = i + 1; j < g_arena_block_count; j++) {
                    g_arena_blocks[j - 1] = g_arena_blocks[j];
                }
            }
            last = g_arena_block_count - 1;
            g_arena_blocks[last].id = -1;
            g_arena_blocks[last].fn = 0;
            g_arena_block_count = last;
            return;
        }
    }
}

void arena_Resize(s32 a0, s32 a1) {
    s32 i;
    s32 diff;

    for (i = 0; i < g_arena_block_count; i++) {
        if (g_arena_blocks[i].id == a0) {
            diff = a1 - g_arena_blocks[i].amt;
            if (diff == 0)
                return;
            arena_ShiftBlocks(i + 1, diff);
            g_arena_blocks[i].amt = a1;
            return;
        }
    }
}

s32 *arena_OpenBlock(s32 a0) {
    Unk800EED10Entry *e;
    s32 idx;
    s32 ret;

    arena_Free(a0);
    idx = g_arena_block_count;
    e = &g_arena_blocks[idx];
    g_arena_block_count = idx + 1;
    ret = g_arena_free_ptr;
    e->unk4 = ret;
    e->id = a0;
    e->fn = 0;
    return (s32 *)ret;
}

void arena_SetBlockEnd(s32 a0, s32 a1) {
    s32 i = 0;
    s32 count = g_arena_block_count;
    Unk800EED10Entry *e;
    if (i >= count)
        goto not_found;
    {
        Unk800EED10Entry *p = g_arena_blocks;
        do {
            /* FAKE: cursor `p` and tested entry `e` are two names for one
               walk, both live as in the target; one pointer `e++`: score 7 */
            e = p;
            if (e->id == a0)
                goto found;
            i++;
            p = e + 1;
        } while (i < count);
    }
found:
    if (i < g_arena_block_count) {
        s32 amt = a1 - g_arena_free_ptr;
        e->amt = amt;
        g_arena_free_ptr += amt;
        g_arena_free_size -= amt;
        return;
    }
not_found:
    func_80052C10();
}

void arena_SetMoveCallback(s32 a0, s32 a1) {
    s32 i;
    s32 count = g_arena_block_count;
    for (i = 0; i < count; i++) {
        if (g_arena_blocks[i].id == a0) {
            g_arena_blocks[i].fn = (void (*)(s16, s32))a1;
            return;
        }
    }
}

void arena_ClearMoveCallback(s32 a0) {
    s32 i;
    s32 count = g_arena_block_count;
    for (i = 0; i < count; i++) {
        if (g_arena_blocks[i].id == a0) {
            g_arena_blocks[i].fn = 0;
            return;
        }
    }
}

s32 *arena_FindBlock(s32 arg0) {
    s32 i;
    s32 count = g_arena_block_count;
    for (i = 0; i < count; i++) {
        if (g_arena_blocks[i].id == arg0) {
            return (s32 *)&g_arena_blocks[i];
        }
    }
    return NULL;
}

s32 arena_GetBlockAddr(s32 a0) {
    s32 *v0 = arena_FindBlock(a0);
    if (v0) {
        return v0[1];
    }
    return 0;
}

void func_800457D4(void) {}

s32 arena_GetFreeSize(void) { return g_arena_free_size; }

s32 arena_GetMinFree(void) { return 0x45000 - g_arena_peak; }

s32 arena_GetFreePtr(void) { return g_arena_free_ptr; }

s32 arena_GetSize(void) { return 0x45000; }

void *arena_GetBase(void) { return &g_arena; }

void arena_Copy(s32 a0, s32 a1, s32 a2) {
    arena_UpdatePeak(a1 + a2);
    func_800520B8(a0, a1, a2);
}

extern void func_80044ED8(s32, s32);
extern void func_80045AA4(s32, s32);

Unk80045878Obj *func_80045878(s32 a0, s32 a1, s32 a2) {
    s32 s3;
    s32 *v0;
    Unk80045878Obj *s1;
    s32 s0;
    v0 = arena_FindBlock(a0);
    if (v0 != 0) {
        s1 = (Unk80045878Obj *)v0[1];
    } else {
        s1 = (Unk80045878Obj *)arena_OpenBlock(a0);
        arena_SetBlockEnd(a0, sizeof(Unk80045878Obj) + ((s32)s1));
        arena_UpdatePeak(0);
        arena_SetMoveCallback(a0, (s32)(&func_80045AA4));
        s1->unk_08 = -1;
        s1->load_status = 0;
    }
    s3 = a0 + 3;
    if (arena_FindBlock(s3) != 0) {
        softbody_FreeObject(s1);
    }
    if (((arena_FindBlock(s3) != 0) && (s1->unk_08 == a1)) &&
        (s1->load_status != (-2))) {
        s1->load_status = 0;
    } else {
        s1->unk_20 = a2;
        s0 = (s32)arena_OpenBlock(s3);
        s1->unk_1C = s0;
        if (a2 != 0) {
            func_80044ED8(a1, a2);
        } else {
            func_80044ED8(a1, s0);
            s0 = s0 + ((((u32)((s32 *)s0)[*((s32 *)s0)]) >> 2) << 2);
            arena_UpdatePeak(s0);
        }
        arena_SetBlockEnd(s3, s0);
        arena_SetMoveCallback(s3, (s32)(&func_80045AA4));
        s1->load_status = 1;
        s1->unk_24 = 0;
        s1->unk_00.word = 0;
    }
    s1->arena_id = a0;
    s1->unk_08 = a1;
    s1->unk_14 = a0;
    s1->unk_10 = a0;
    s1->unk_16 = a0 + 3;
    s1->unk_18.word = 0x8000;
    return s1;
}

void func_80045A28(s32 a0, s32 a1) {
    arena_Resize(a0 + 3, a1);
    arena_UpdatePeak(0);
}

extern void arena_ClearMoveCallback(s32);

void func_80045A50(s32 a0) {
    s32 a0p3 = a0 + 3;
    func_8005B644(a0);
    arena_ClearMoveCallback(a0p3);
    arena_ClearMoveCallback(a0);
    arena_Free(a0p3);
    arena_Free(a0);
}

/* Not the definition's spelling: snd_VabFakeOpen takes an s16 second
 * parameter, but the calls here pass it unextended (an s16 prototype adds
 * sll/sra in func_80045AA4, lh for lw in func_80046048). */
extern s32 snd_VabFakeOpen(s32, s32);

void func_80045AA4(s32 a0, s32 a1) {
    Unk80045878Obj *ptr;
    s32 idx;
    if (a0 < 3) {
        func_80041430(a0, a1);
        return;
    }
    idx = a0 - 3;
    reloc_RebaseBlock(idx, a1);
    reloc_RebaseBlock(a0, a1);
    ptr = (Unk80045878Obj *)arena_GetBlockAddr(idx);
    if (ptr == 0)
        return;
    ptr->unk_1C = ptr->unk_1C + a1;
    softbody_RebaseObject(ptr, a1);
    if ((ptr->unk_00.word >> 1) & 1) {
        /* FAKE: idx (a0 - 3) is reused for the sound index; a fresh local swaps
         * the s0 / s1 seats (score 8) */
        idx = 3 * ptr->arena_id + 1;
        snd_VabFakeOpen(a1, idx);
    }
}

extern s16 D_800993FC[];
extern void func_80044F50(s32, s32, s32);
extern void func_80046048(s32, s32);

void func_80045B68(s32 arg0, s32 arg1, s16 *arg2, s32 arg3) {
    s16 sp18[120];
    s16 sp108[32];
    s32 *p;
    s32 *var_3;
    s32 last;
    s32 prev;
    s32 var_6;
    s32 var_7;
    s32 n;
    s32 i;
    s32 y;
    s16 *q;
    s16 *r;
    u16 t;
    s32 *dst;

    p = arena_OpenBlock(6);
    if (arg3 != 0) {
        var_3 = (s32 *)arg3;
    } else {
        var_3 = p;
    }
    func_80044F50(arg0, arg1, (s32)var_3);

    last = (s32)var_3 + (((u32)var_3[1] >> 2) << 2);
    n = var_3[0];
    var_7 = (s32)var_3 + (((u32)var_3[n] >> 2) << 2);
    if (n >= 3) {
        prev = (s32)var_3 + (((u32)var_3[n - 1] >> 2) << 2);
    } else {
        prev = 0;
    }

    for (i = 0; i < 32; i++) {
        sp108[i] = -1;
    }

    q = arg2;
    t = *q++;
    i = 0;
    while ((s16)t != -2) {
        if ((s16)t >= 0) {
            sp108[D_800993FC[i]] = 1;
        }
        i++;
        t = *q++;
    }

    n = 0;
    for (i = 0; i < 32; i++) {
        if (sp108[i] >= 0) {
            switch (n) {
            case 0:
                gpu_LoadTimGroupSplitOffset(var_7, i, 0, 0, -0x180, 0xF0);
                break;
            case 1:
                gpu_LoadTimGroupSplitOffset(var_7, i, 0, 0x40, -0x180, 0xF1);
                break;
            case 2:
                gpu_LoadTimGroupSplitOffset(var_7, i, 0x80, 0, -0x160, 0xF0);
                break;
            case 3:
                gpu_LoadTimGroupSplitOffset(var_7, i, 0x80, 0x40, -0x160, 0xF1);
                break;
            }
            sp108[i] = n;
            n++;
        }
    }

    DrawSync(0);

    q = sp18;
    if (arg0 == 0) {
        r = arg2;
    } else {
        r = arg2 + 0x33;
    }
    while (*r != -2) {
        if (*r >= 0) {
            *q++ = 1;
            *q++ = 1;
        } else {
            *q++ = -1;
            *q++ = -1;
        }
        r++;
    }
    *q = -2;

    if (arg3 != 0) {
        dst = (s32 *)((s32)p + 4);
        var_6 = reloc_CopySelectedEntries(last, dst, sp18);
        reloc_RelocateBlock(dst, 6);
    } else {
        var_6 = reloc_KeepSelectedEntries((s32 *)last, sp18);
        reloc_RelocateBlock((s32 *)last, 6);
    }
    arena_UpdatePeak(var_6);

    q = sp18;
    y = 0;
    i = 0;
    while (*q != -2) {
        if (*q >= 0) {
            switch (sp108[D_800993FC[i]]) {
            case 0:
                gpu_OffsetBlockTexPrimStream(6, y * 2, 0, 0, -0x180, 0xE8);
                break;
            case 1:
                gpu_OffsetBlockTexPrimStream(6, y * 2, 0, 0x40, -0x180, 0xE9);
                break;
            case 2:
                gpu_OffsetBlockTexPrimStream(6, y * 2, 0x80, 0, -0x160, 0xE8);
                break;
            case 3:
                gpu_OffsetBlockTexPrimStream(
                    6, y * 2, 0x80, 0x40, -0x160, 0xE9);
                break;
            }
            y++;
        }
        q += 2;
        i++;
    }

    if (prev != 0) {
        *p = var_6 - (s32)p;
        var_6 = func_80049C24(prev, var_6);
        arena_UpdatePeak(var_6);
    } else {
        *p = 0;
    }
    arena_SetBlockEnd(6, var_6);
    arena_SetMoveCallback(6, (s32)&func_80046048);
}

void func_80046020(void) {
    arena_Free(6);
    func_8005B6AC();
}

void func_80046048(s32 a0, s32 a1) {
    s32 *s0;
    s32 count;
    reloc_RebaseBlock(6, a1);
    s0 = (s32 *)arena_GetBlockAddr(6);
    if (s0 == NULL) {
        return;
    }
    {
        s32 off = *s0;
        if (off == 0) {
            return;
        }
        s0 = (s32 *)((u8 *)s0 + off);
    }
    count = *s0;
    count = count - 1;
    if (count == -1) {
        return;
    }
    s0++;
    do {
        snd_VabFakeOpen(a1, *s0++);
        count--;
    } while (count != -1);
}

/* .data 0x800955E0..0x80099478 (32D04.c's D_80095588 precedes, 368E4.c's
 * D_80099478 follows), in address order: D_800955E0, the 899 NDATA asset path
 * pointers into d_15EC.c's pool (D_80010DEC, its strings in reverse address
 * order; no code reads them, docs/formats/NDATA.md); the NDATA.INF sector
 * table g_ndata_inf ({start, length} per file, read over from disc by
 * ndata_StartReadInf; trailing zero entries implicit), ndata_ReadFromPack's
 * pack index, the 0x1000-step sine table Judge (bb2.h), and func_80045B68's
 * index list D_800993FC. D_800955E0 sits between 32D04.c's .data and this
 * file's with no reorder either way; it is this file's by subsystem (the NDATA
 * file names, next to the NDATA file table; 899 is also g_ndata_inf's explicit
 * entry count, 760 packed + 139 {-1, -1}). Defined after every function, so
 * no body sees a definition where it saw an extern. */
extern const char D_80010DEC[];
const char *D_800955E0[899] = {
    D_80010DEC + 0x4490, D_80010DEC + 0x4480, D_80010DEC + 0x4470,
    D_80010DEC + 0x4460, D_80010DEC + 0x4450, D_80010DEC + 0x4440,
    D_80010DEC + 0x4430, D_80010DEC + 0x4420, D_80010DEC + 0x4410,
    D_80010DEC + 0x4400, D_80010DEC + 0x43F0, D_80010DEC + 0x43E0,
    D_80010DEC + 0x43D0, D_80010DEC + 0x43C0, D_80010DEC + 0x43B0,
    D_80010DEC + 0x43A0, D_80010DEC + 0x4390, D_80010DEC + 0x4380,
    D_80010DEC + 0x4370, D_80010DEC + 0x4360, D_80010DEC + 0x4350,
    D_80010DEC + 0x4340, D_80010DEC + 0x4330, D_80010DEC + 0x4320,
    D_80010DEC + 0x4310, D_80010DEC + 0x4300, D_80010DEC + 0x42F0,
    D_80010DEC + 0x42E0, D_80010DEC + 0x42D0, D_80010DEC + 0x42C0,
    D_80010DEC + 0x42B0, D_80010DEC + 0x42A0, D_80010DEC + 0x4290,
    D_80010DEC + 0x4280, D_80010DEC + 0x4270, D_80010DEC + 0x4260,
    D_80010DEC + 0x4250, D_80010DEC + 0x423C, D_80010DEC + 0x4228,
    D_80010DEC + 0x4218, D_80010DEC + 0x4208, D_80010DEC + 0x41F8,
    D_80010DEC + 0x41E8, D_80010DEC + 0x41D8, D_80010DEC + 0x41C8,
    D_80010DEC + 0x41B8, D_80010DEC + 0x41A8, D_80010DEC + 0x4198,
    D_80010DEC + 0x4188, D_80010DEC + 0x4178, D_80010DEC + 0x4168,
    D_80010DEC + 0x4158, D_80010DEC + 0x4148, D_80010DEC + 0x4138,
    D_80010DEC + 0x4128, D_80010DEC + 0x4118, D_80010DEC + 0x4108,
    D_80010DEC + 0x40F8, D_80010DEC + 0x40E8, D_80010DEC + 0x40D8,
    D_80010DEC + 0x40C8, D_80010DEC + 0x40B8, D_80010DEC + 0x40A8,
    D_80010DEC + 0x4098, D_80010DEC + 0x4088, D_80010DEC + 0x4078,
    D_80010DEC + 0x4068, D_80010DEC + 0x4058, D_80010DEC + 0x4048,
    D_80010DEC + 0x4038, D_80010DEC + 0x4028, D_80010DEC + 0x4018,
    D_80010DEC + 0x4008, D_80010DEC + 0x3FF8, D_80010DEC + 0x3FE8,
    D_80010DEC + 0x3FD8, D_80010DEC + 0x3FC8, D_80010DEC + 0x3FB8,
    D_80010DEC + 0x3FA8, D_80010DEC + 0x3F98, D_80010DEC + 0x3F88,
    D_80010DEC + 0x3F78, D_80010DEC + 0x3F68, D_80010DEC + 0x3F58,
    D_80010DEC + 0x3F48, D_80010DEC + 0x3F38, D_80010DEC + 0x3F28,
    D_80010DEC + 0x3F18, D_80010DEC + 0x3F08, D_80010DEC + 0x3EF8,
    D_80010DEC + 0x3EE8, D_80010DEC + 0x3ED8, D_80010DEC + 0x3EC8,
    D_80010DEC + 0x3EB8, D_80010DEC + 0x3EA8, D_80010DEC + 0x3E98,
    D_80010DEC + 0x3E88, D_80010DEC + 0x3E78, D_80010DEC + 0x3E68,
    D_80010DEC + 0x3E58, D_80010DEC + 0x3E48, D_80010DEC + 0x3E38,
    D_80010DEC + 0x3E28, D_80010DEC + 0x3E18, D_80010DEC + 0x3E04,
    D_80010DEC + 0x3DF0, D_80010DEC + 0x3DDC, D_80010DEC + 0x3DC8,
    D_80010DEC + 0x3DB4, D_80010DEC + 0x3DA0, D_80010DEC + 0x3D8C,
    D_80010DEC + 0x3D78, D_80010DEC + 0x3D64, D_80010DEC + 0x3D50,
    D_80010DEC + 0x3D3C, D_80010DEC + 0x3D28, D_80010DEC + 0x3D14,
    D_80010DEC + 0x3D00, D_80010DEC + 0x3CEC, D_80010DEC + 0x3CD8,
    D_80010DEC + 0x3CC4, D_80010DEC + 0x3CB0, D_80010DEC + 0x3C9C,
    D_80010DEC + 0x3C88, D_80010DEC + 0x3C74, D_80010DEC + 0x3C60,
    D_80010DEC + 0x3C4C, D_80010DEC + 0x3C38, D_80010DEC + 0x3C24,
    D_80010DEC + 0x3C10, D_80010DEC + 0x3BFC, D_80010DEC + 0x3BE8,
    D_80010DEC + 0x3BD4, D_80010DEC + 0x3BC0, D_80010DEC + 0x3BAC,
    D_80010DEC + 0x3B98, D_80010DEC + 0x3B84, D_80010DEC + 0x3B70,
    D_80010DEC + 0x3B5C, D_80010DEC + 0x3B48, D_80010DEC + 0x3B34,
    D_80010DEC + 0x3B20, D_80010DEC + 0x3B0C, D_80010DEC + 0x3AF8,
    D_80010DEC + 0x3AE4, D_80010DEC + 0x3AD0, D_80010DEC + 0x3ABC,
    D_80010DEC + 0x3AA8, D_80010DEC + 0x3A94, D_80010DEC + 0x3A80,
    D_80010DEC + 0x3A6C, D_80010DEC + 0x3A58, D_80010DEC + 0x3A44,
    D_80010DEC + 0x3A30, D_80010DEC + 0x3A1C, D_80010DEC + 0x3A08,
    D_80010DEC + 0x39F4, D_80010DEC + 0x39E0, D_80010DEC + 0x39CC,
    D_80010DEC + 0x39B8, D_80010DEC + 0x39A4, D_80010DEC + 0x3990,
    D_80010DEC + 0x397C, D_80010DEC + 0x3968, D_80010DEC + 0x3954,
    D_80010DEC + 0x3940, D_80010DEC + 0x392C, D_80010DEC + 0x3918,
    D_80010DEC + 0x3904, D_80010DEC + 0x38F0, D_80010DEC + 0x38DC,
    D_80010DEC + 0x38C8, D_80010DEC + 0x38B4, D_80010DEC + 0x38A0,
    D_80010DEC + 0x388C, D_80010DEC + 0x3878, D_80010DEC + 0x3864,
    D_80010DEC + 0x3850, D_80010DEC + 0x383C, D_80010DEC + 0x3828,
    D_80010DEC + 0x3814, D_80010DEC + 0x3800, D_80010DEC + 0x37EC,
    D_80010DEC + 0x37D8, D_80010DEC + 0x37C4, D_80010DEC + 0x37B0,
    D_80010DEC + 0x379C, D_80010DEC + 0x3788, D_80010DEC + 0x3774,
    D_80010DEC + 0x3760, D_80010DEC + 0x374C, D_80010DEC + 0x3738,
    D_80010DEC + 0x3724, D_80010DEC + 0x3710, D_80010DEC + 0x36FC,
    D_80010DEC + 0x36E8, D_80010DEC + 0x36D4, D_80010DEC + 0x36C0,
    D_80010DEC + 0x36AC, D_80010DEC + 0x3698, D_80010DEC + 0x3684,
    D_80010DEC + 0x3670, D_80010DEC + 0x365C, D_80010DEC + 0x3648,
    D_80010DEC + 0x3634, D_80010DEC + 0x3620, D_80010DEC + 0x360C,
    D_80010DEC + 0x35F8, D_80010DEC + 0x35E4, D_80010DEC + 0x35D0,
    D_80010DEC + 0x35BC, D_80010DEC + 0x35A8, D_80010DEC + 0x3594,
    D_80010DEC + 0x3580, D_80010DEC + 0x356C, D_80010DEC + 0x3558,
    D_80010DEC + 0x3544, D_80010DEC + 0x3530, D_80010DEC + 0x351C,
    D_80010DEC + 0x3508, D_80010DEC + 0x34F4, D_80010DEC + 0x34E0,
    D_80010DEC + 0x34CC, D_80010DEC + 0x34B8, D_80010DEC + 0x34A4,
    D_80010DEC + 0x3490, D_80010DEC + 0x347C, D_80010DEC + 0x3468,
    D_80010DEC + 0x3454, D_80010DEC + 0x3440, D_80010DEC + 0x342C,
    D_80010DEC + 0x3418, D_80010DEC + 0x3404, D_80010DEC + 0x33F0,
    D_80010DEC + 0x33DC, D_80010DEC + 0x33C8, D_80010DEC + 0x33B4,
    D_80010DEC + 0x33A0, D_80010DEC + 0x338C, D_80010DEC + 0x3378,
    D_80010DEC + 0x3364, D_80010DEC + 0x3350, D_80010DEC + 0x333C,
    D_80010DEC + 0x3328, D_80010DEC + 0x3314, D_80010DEC + 0x3300,
    D_80010DEC + 0x32EC, D_80010DEC + 0x32D8, D_80010DEC + 0x32C4,
    D_80010DEC + 0x32B0, D_80010DEC + 0x329C, D_80010DEC + 0x3288,
    D_80010DEC + 0x3274, D_80010DEC + 0x3260, D_80010DEC + 0x324C,
    D_80010DEC + 0x3238, D_80010DEC + 0x3224, D_80010DEC + 0x3210,
    D_80010DEC + 0x31FC, D_80010DEC + 0x31E8, D_80010DEC + 0x31D4,
    D_80010DEC + 0x31C0, D_80010DEC + 0x31AC, D_80010DEC + 0x3198,
    D_80010DEC + 0x3184, D_80010DEC + 0x3170, D_80010DEC + 0x315C,
    D_80010DEC + 0x3148, D_80010DEC + 0x3134, D_80010DEC + 0x3120,
    D_80010DEC + 0x310C, D_80010DEC + 0x30F8, D_80010DEC + 0x30E4,
    D_80010DEC + 0x30D0, D_80010DEC + 0x30BC, D_80010DEC + 0x30A8,
    D_80010DEC + 0x3094, D_80010DEC + 0x3080, D_80010DEC + 0x306C,
    D_80010DEC + 0x3058, D_80010DEC + 0x3044, D_80010DEC + 0x3030,
    D_80010DEC + 0x301C, D_80010DEC + 0x3008, D_80010DEC + 0x2FF4,
    D_80010DEC + 0x2FE0, D_80010DEC + 0x2FCC, D_80010DEC + 0x2FB8,
    D_80010DEC + 0x2FA4, D_80010DEC + 0x2F90, D_80010DEC + 0x2F7C,
    D_80010DEC + 0x2F68, D_80010DEC + 0x2F54, D_80010DEC + 0x2F40,
    D_80010DEC + 0x2F2C, D_80010DEC + 0x2F18, D_80010DEC + 0x2F04,
    D_80010DEC + 0x2EF0, D_80010DEC + 0x2EDC, D_80010DEC + 0x2EC8,
    D_80010DEC + 0x2EB4, D_80010DEC + 0x2EA0, D_80010DEC + 0x2E8C,
    D_80010DEC + 0x2E78, D_80010DEC + 0x2E64, D_80010DEC + 0x2E50,
    D_80010DEC + 0x2E3C, D_80010DEC + 0x2E28, D_80010DEC + 0x2E14,
    D_80010DEC + 0x2E00, D_80010DEC + 0x2DEC, D_80010DEC + 0x2DD8,
    D_80010DEC + 0x2DC4, D_80010DEC + 0x2DB0, D_80010DEC + 0x2D9C,
    D_80010DEC + 0x2D88, D_80010DEC + 0x2D74, D_80010DEC + 0x2D60,
    D_80010DEC + 0x2D4C, D_80010DEC + 0x2D38, D_80010DEC + 0x2D24,
    D_80010DEC + 0x2D10, D_80010DEC + 0x2CFC, D_80010DEC + 0x2CE8,
    D_80010DEC + 0x2CD4, D_80010DEC + 0x2CC0, D_80010DEC + 0x2CAC,
    D_80010DEC + 0x2C98, D_80010DEC + 0x2C84, D_80010DEC + 0x2C70,
    D_80010DEC + 0x2C5C, D_80010DEC + 0x2C48, D_80010DEC + 0x2C34,
    D_80010DEC + 0x2C20, D_80010DEC + 0x2C0C, D_80010DEC + 0x2BF8,
    D_80010DEC + 0x2BE4, D_80010DEC + 0x2BD0, D_80010DEC + 0x2BBC,
    D_80010DEC + 0x2BA8, D_80010DEC + 0x2B94, D_80010DEC + 0x2B80,
    D_80010DEC + 0x2B6C, D_80010DEC + 0x2B58, D_80010DEC + 0x2B44,
    D_80010DEC + 0x2B30, D_80010DEC + 0x2B1C, D_80010DEC + 0x2B08,
    D_80010DEC + 0x2AF4, D_80010DEC + 0x2AE0, D_80010DEC + 0x2ACC,
    D_80010DEC + 0x2AB8, D_80010DEC + 0x2AA4, D_80010DEC + 0x2A90,
    D_80010DEC + 0x2A7C, D_80010DEC + 0x2A68, D_80010DEC + 0x2A54,
    D_80010DEC + 0x2A40, D_80010DEC + 0x2A2C, D_80010DEC + 0x2A18,
    D_80010DEC + 0x2A04, D_80010DEC + 0x29F0, D_80010DEC + 0x29DC,
    D_80010DEC + 0x29C8, D_80010DEC + 0x29B4, D_80010DEC + 0x29A0,
    D_80010DEC + 0x298C, D_80010DEC + 0x2978, D_80010DEC + 0x2964,
    D_80010DEC + 0x2950, D_80010DEC + 0x293C, D_80010DEC + 0x2928,
    D_80010DEC + 0x2914, D_80010DEC + 0x2900, D_80010DEC + 0x28EC,
    D_80010DEC + 0x28D8, D_80010DEC + 0x28C4, D_80010DEC + 0x28B0,
    D_80010DEC + 0x289C, D_80010DEC + 0x2888, D_80010DEC + 0x2874,
    D_80010DEC + 0x2860, D_80010DEC + 0x284C, D_80010DEC + 0x2838,
    D_80010DEC + 0x2824, D_80010DEC + 0x2810, D_80010DEC + 0x27FC,
    D_80010DEC + 0x27E8, D_80010DEC + 0x27D4, D_80010DEC + 0x27C0,
    D_80010DEC + 0x27AC, D_80010DEC + 0x2798, D_80010DEC + 0x2784,
    D_80010DEC + 0x2770, D_80010DEC + 0x275C, D_80010DEC + 0x2748,
    D_80010DEC + 0x2734, D_80010DEC + 0x2720, D_80010DEC + 0x270C,
    D_80010DEC + 0x26F8, D_80010DEC + 0x26E4, D_80010DEC + 0x26D0,
    D_80010DEC + 0x26BC, D_80010DEC + 0x26A8, D_80010DEC + 0x2694,
    D_80010DEC + 0x2680, D_80010DEC + 0x266C, D_80010DEC + 0x2658,
    D_80010DEC + 0x2644, D_80010DEC + 0x2630, D_80010DEC + 0x261C,
    D_80010DEC + 0x2608, D_80010DEC + 0x25F4, D_80010DEC + 0x25E0,
    D_80010DEC + 0x25CC, D_80010DEC + 0x25B8, D_80010DEC + 0x25A4,
    D_80010DEC + 0x2590, D_80010DEC + 0x257C, D_80010DEC + 0x2568,
    D_80010DEC + 0x2554, D_80010DEC + 0x2540, D_80010DEC + 0x252C,
    D_80010DEC + 0x2518, D_80010DEC + 0x2504, D_80010DEC + 0x24F0,
    D_80010DEC + 0x24DC, D_80010DEC + 0x24C8, D_80010DEC + 0x24B4,
    D_80010DEC + 0x24A0, D_80010DEC + 0x248C, D_80010DEC + 0x2478,
    D_80010DEC + 0x2464, D_80010DEC + 0x2450, D_80010DEC + 0x243C,
    D_80010DEC + 0x2428, D_80010DEC + 0x2414, D_80010DEC + 0x2400,
    D_80010DEC + 0x23EC, D_80010DEC + 0x23D8, D_80010DEC + 0x23C4,
    D_80010DEC + 0x23B0, D_80010DEC + 0x239C, D_80010DEC + 0x2388,
    D_80010DEC + 0x2374, D_80010DEC + 0x2360, D_80010DEC + 0x234C,
    D_80010DEC + 0x2338, D_80010DEC + 0x2324, D_80010DEC + 0x2310,
    D_80010DEC + 0x22FC, D_80010DEC + 0x22E8, D_80010DEC + 0x22D4,
    D_80010DEC + 0x22C0, D_80010DEC + 0x22AC, D_80010DEC + 0x2298,
    D_80010DEC + 0x2284, D_80010DEC + 0x2270, D_80010DEC + 0x225C,
    D_80010DEC + 0x2248, D_80010DEC + 0x2234, D_80010DEC + 0x2220,
    D_80010DEC + 0x220C, D_80010DEC + 0x21F8, D_80010DEC + 0x21E4,
    D_80010DEC + 0x21D0, D_80010DEC + 0x21BC, D_80010DEC + 0x21A8,
    D_80010DEC + 0x2194, D_80010DEC + 0x2180, D_80010DEC + 0x216C,
    D_80010DEC + 0x2158, D_80010DEC + 0x2144, D_80010DEC + 0x2130,
    D_80010DEC + 0x211C, D_80010DEC + 0x2108, D_80010DEC + 0x20F4,
    D_80010DEC + 0x20E0, D_80010DEC + 0x20CC, D_80010DEC + 0x20B8,
    D_80010DEC + 0x20A4, D_80010DEC + 0x2090, D_80010DEC + 0x207C,
    D_80010DEC + 0x2068, D_80010DEC + 0x2054, D_80010DEC + 0x2040,
    D_80010DEC + 0x202C, D_80010DEC + 0x2018, D_80010DEC + 0x2004,
    D_80010DEC + 0x1FF0, D_80010DEC + 0x1FDC, D_80010DEC + 0x1FC8,
    D_80010DEC + 0x1FB4, D_80010DEC + 0x1FA0, D_80010DEC + 0x1F8C,
    D_80010DEC + 0x1F78, D_80010DEC + 0x1F64, D_80010DEC + 0x1F50,
    D_80010DEC + 0x1F3C, D_80010DEC + 0x1F28, D_80010DEC + 0x1F14,
    D_80010DEC + 0x1F00, D_80010DEC + 0x1EEC, D_80010DEC + 0x1ED8,
    D_80010DEC + 0x1EC4, D_80010DEC + 0x1EB0, D_80010DEC + 0x1E9C,
    D_80010DEC + 0x1E88, D_80010DEC + 0x1E74, D_80010DEC + 0x1E60,
    D_80010DEC + 0x1E4C, D_80010DEC + 0x1E38, D_80010DEC + 0x1E24,
    D_80010DEC + 0x1E10, D_80010DEC + 0x1DFC, D_80010DEC + 0x1DE8,
    D_80010DEC + 0x1DD4, D_80010DEC + 0x1DC0, D_80010DEC + 0x1DAC,
    D_80010DEC + 0x1D98, D_80010DEC + 0x1D84, D_80010DEC + 0x1D70,
    D_80010DEC + 0x1D5C, D_80010DEC + 0x1D48, D_80010DEC + 0x1D34,
    D_80010DEC + 0x1D20, D_80010DEC + 0x1D0C, D_80010DEC + 0x1CF8,
    D_80010DEC + 0x1CE4, D_80010DEC + 0x1CD0, D_80010DEC + 0x1CBC,
    D_80010DEC + 0x1CA8, D_80010DEC + 0x1C94, D_80010DEC + 0x1C80,
    D_80010DEC + 0x1C6C, D_80010DEC + 0x1C58, D_80010DEC + 0x1C44,
    D_80010DEC + 0x1C30, D_80010DEC + 0x1C1C, D_80010DEC + 0x1C08,
    D_80010DEC + 0x1BF4, D_80010DEC + 0x1BE0, D_80010DEC + 0x1BCC,
    D_80010DEC + 0x1BB8, D_80010DEC + 0x1BA4, D_80010DEC + 0x1B90,
    D_80010DEC + 0x1B7C, D_80010DEC + 0x1B68, D_80010DEC + 0x1B54,
    D_80010DEC + 0x1B40, D_80010DEC + 0x1B2C, D_80010DEC + 0x1B18,
    D_80010DEC + 0x1B04, D_80010DEC + 0x1AF0, D_80010DEC + 0x1ADC,
    D_80010DEC + 0x1AC8, D_80010DEC + 0x1AB4, D_80010DEC + 0x1AA0,
    D_80010DEC + 0x1A8C, D_80010DEC + 0x1A78, D_80010DEC + 0x1A64,
    D_80010DEC + 0x1A50, D_80010DEC + 0x1A3C, D_80010DEC + 0x1A28,
    D_80010DEC + 0x1A14, D_80010DEC + 0x1A00, D_80010DEC + 0x19EC,
    D_80010DEC + 0x19D8, D_80010DEC + 0x19C4, D_80010DEC + 0x19B0,
    D_80010DEC + 0x199C, D_80010DEC + 0x1988, D_80010DEC + 0x1974,
    D_80010DEC + 0x1960, D_80010DEC + 0x194C, D_80010DEC + 0x1938,
    D_80010DEC + 0x1924, D_80010DEC + 0x1910, D_80010DEC + 0x18FC,
    D_80010DEC + 0x18E8, D_80010DEC + 0x18D4, D_80010DEC + 0x18C0,
    D_80010DEC + 0x18AC, D_80010DEC + 0x1898, D_80010DEC + 0x1884,
    D_80010DEC + 0x1870, D_80010DEC + 0x185C, D_80010DEC + 0x1848,
    D_80010DEC + 0x1834, D_80010DEC + 0x1820, D_80010DEC + 0x180C,
    D_80010DEC + 0x17F8, D_80010DEC + 0x17E4, D_80010DEC + 0x17D0,
    D_80010DEC + 0x17BC, D_80010DEC + 0x17A8, D_80010DEC + 0x1794,
    D_80010DEC + 0x1780, D_80010DEC + 0x176C, D_80010DEC + 0x1758,
    D_80010DEC + 0x1744, D_80010DEC + 0x1730, D_80010DEC + 0x171C,
    D_80010DEC + 0x1708, D_80010DEC + 0x16F4, D_80010DEC + 0x16E0,
    D_80010DEC + 0x16CC, D_80010DEC + 0x16B8, D_80010DEC + 0x16A4,
    D_80010DEC + 0x1690, D_80010DEC + 0x167C, D_80010DEC + 0x1668,
    D_80010DEC + 0x1654, D_80010DEC + 0x1640, D_80010DEC + 0x162C,
    D_80010DEC + 0x1618, D_80010DEC + 0x1604, D_80010DEC + 0x15F0,
    D_80010DEC + 0x15DC, D_80010DEC + 0x15C8, D_80010DEC + 0x15B4,
    D_80010DEC + 0x15A0, D_80010DEC + 0x158C, D_80010DEC + 0x1578,
    D_80010DEC + 0x1564, D_80010DEC + 0x1550, D_80010DEC + 0x153C,
    D_80010DEC + 0x1528, D_80010DEC + 0x1514, D_80010DEC + 0x1500,
    D_80010DEC + 0x14EC, D_80010DEC + 0x14D8, D_80010DEC + 0x14C4,
    D_80010DEC + 0x14B0, D_80010DEC + 0x149C, D_80010DEC + 0x1488,
    D_80010DEC + 0x1474, D_80010DEC + 0x1460, D_80010DEC + 0x144C,
    D_80010DEC + 0x1438, D_80010DEC + 0x1424, D_80010DEC + 0x1410,
    D_80010DEC + 0x13FC, D_80010DEC + 0x13E8, D_80010DEC + 0x13D4,
    D_80010DEC + 0x13C0, D_80010DEC + 0x13AC, D_80010DEC + 0x1398,
    D_80010DEC + 0x1384, D_80010DEC + 0x1370, D_80010DEC + 0x135C,
    D_80010DEC + 0x1348, D_80010DEC + 0x1334, D_80010DEC + 0x1320,
    D_80010DEC + 0x130C, D_80010DEC + 0x12F8, D_80010DEC + 0x12E4,
    D_80010DEC + 0x12D0, D_80010DEC + 0x12BC, D_80010DEC + 0x12A8,
    D_80010DEC + 0x1294, D_80010DEC + 0x1280, D_80010DEC + 0x126C,
    D_80010DEC + 0x1258, D_80010DEC + 0x1244, D_80010DEC + 0x1230,
    D_80010DEC + 0x121C, D_80010DEC + 0x1208, D_80010DEC + 0x11F4,
    D_80010DEC + 0x11E0, D_80010DEC + 0x11CC, D_80010DEC + 0x11B8,
    D_80010DEC + 0x11A4, D_80010DEC + 0x1190, D_80010DEC + 0x117C,
    D_80010DEC + 0x1168, D_80010DEC + 0x1154, D_80010DEC + 0x1140,
    D_80010DEC + 0x112C, D_80010DEC + 0x1118, D_80010DEC + 0x1104,
    D_80010DEC + 0x10F0, D_80010DEC + 0x10DC, D_80010DEC + 0x10C8,
    D_80010DEC + 0x10B4, D_80010DEC + 0x10A0, D_80010DEC + 0x108C,
    D_80010DEC + 0x1078, D_80010DEC + 0x1064, D_80010DEC + 0x1050,
    D_80010DEC + 0x103C, D_80010DEC + 0x1028, D_80010DEC + 0x1014,
    D_80010DEC + 0x1000, D_80010DEC + 0xFEC,  D_80010DEC + 0xFD8,
    D_80010DEC + 0xFC4,  D_80010DEC + 0xFB0,  D_80010DEC + 0xF9C,
    D_80010DEC + 0xF88,  D_80010DEC + 0xF74,  D_80010DEC + 0xF60,
    D_80010DEC + 0xF4C,  D_80010DEC + 0xF38,  D_80010DEC + 0xF24,
    D_80010DEC + 0xF10,  D_80010DEC + 0xEFC,  D_80010DEC + 0xEE8,
    D_80010DEC + 0xED4,  D_80010DEC + 0xEC0,  D_80010DEC + 0xEAC,
    D_80010DEC + 0xE98,  D_80010DEC + 0xE84,  D_80010DEC + 0xE70,
    D_80010DEC + 0xE5C,  D_80010DEC + 0xE48,  D_80010DEC + 0xE34,
    D_80010DEC + 0xE20,  D_80010DEC + 0xE0C,  D_80010DEC + 0xDF8,
    D_80010DEC + 0xDE4,  D_80010DEC + 0xDD0,  D_80010DEC + 0xDBC,
    D_80010DEC + 0xDA8,  D_80010DEC + 0xD94,  D_80010DEC + 0xD80,
    D_80010DEC + 0xD6C,  D_80010DEC + 0xD58,  D_80010DEC + 0xD44,
    D_80010DEC + 0xD30,  D_80010DEC + 0xD1C,  D_80010DEC + 0xD08,
    D_80010DEC + 0xCF4,  D_80010DEC + 0xCE0,  D_80010DEC + 0xCCC,
    D_80010DEC + 0xCB8,  D_80010DEC + 0xCA4,  D_80010DEC + 0xC90,
    D_80010DEC + 0xC7C,  D_80010DEC + 0xC68,  D_80010DEC + 0xC54,
    D_80010DEC + 0xC40,  D_80010DEC + 0xC2C,  D_80010DEC + 0xC18,
    D_80010DEC + 0xC04,  D_80010DEC + 0xBF0,  D_80010DEC + 0xBDC,
    D_80010DEC + 0xBC8,  D_80010DEC + 0xBB4,  D_80010DEC + 0xBA0,
    D_80010DEC + 0xB8C,  D_80010DEC + 0xB78,  D_80010DEC + 0xB64,
    D_80010DEC + 0xB50,  D_80010DEC + 0xB3C,  D_80010DEC + 0xB28,
    D_80010DEC + 0xB14,  D_80010DEC + 0xB00,  D_80010DEC + 0xAEC,
    D_80010DEC + 0xAD8,  D_80010DEC + 0xAC4,  D_80010DEC + 0xAB0,
    D_80010DEC + 0xA9C,  D_80010DEC + 0xA88,  D_80010DEC + 0xA74,
    D_80010DEC + 0xA60,  D_80010DEC + 0xA4C,  D_80010DEC + 0xA38,
    D_80010DEC + 0xA24,  D_80010DEC + 0xA10,  D_80010DEC + 0x9FC,
    D_80010DEC + 0x9E8,  D_80010DEC + 0x9D4,  D_80010DEC + 0x9C0,
    D_80010DEC + 0x9AC,  D_80010DEC + 0x998,  D_80010DEC + 0x984,
    D_80010DEC + 0x970,  D_80010DEC + 0x95C,  D_80010DEC + 0x948,
    D_80010DEC + 0x934,  D_80010DEC + 0x920,  D_80010DEC + 0x90C,
    D_80010DEC + 0x8F8,  D_80010DEC + 0x8E4,  D_80010DEC + 0x8D0,
    D_80010DEC + 0x8BC,  D_80010DEC + 0x8A8,  D_80010DEC + 0x894,
    D_80010DEC + 0x880,  D_80010DEC + 0x86C,  D_80010DEC + 0x858,
    D_80010DEC + 0x844,  D_80010DEC + 0x830,  D_80010DEC + 0x81C,
    D_80010DEC + 0x808,  D_80010DEC + 0x7F4,  D_80010DEC + 0x7E0,
    D_80010DEC + 0x7CC,  D_80010DEC + 0x7B8,  D_80010DEC + 0x7A4,
    D_80010DEC + 0x790,  D_80010DEC + 0x77C,  D_80010DEC + 0x768,
    D_80010DEC + 0x754,  D_80010DEC + 0x740,  D_80010DEC + 0x72C,
    D_80010DEC + 0x718,  D_80010DEC + 0x704,  D_80010DEC + 0x6F0,
    D_80010DEC + 0x6DC,  D_80010DEC + 0x6C8,  D_80010DEC + 0x6B4,
    D_80010DEC + 0x6A0,  D_80010DEC + 0x68C,  D_80010DEC + 0x678,
    D_80010DEC + 0x664,  D_80010DEC + 0x650,  D_80010DEC + 0x63C,
    D_80010DEC + 0x628,  D_80010DEC + 0x614,  D_80010DEC + 0x600,
    D_80010DEC + 0x5EC,  D_80010DEC + 0x5D8,  D_80010DEC + 0x5C4,
    D_80010DEC + 0x5B0,  D_80010DEC + 0x59C,  D_80010DEC + 0x588,
    D_80010DEC + 0x574,  D_80010DEC + 0x560,  D_80010DEC + 0x54C,
    D_80010DEC + 0x538,  D_80010DEC + 0x524,  D_80010DEC + 0x510,
    D_80010DEC + 0x4FC,  D_80010DEC + 0x4E8,  D_80010DEC + 0x4D4,
    D_80010DEC + 0x4C0,  D_80010DEC + 0x4AC,  D_80010DEC + 0x498,
    D_80010DEC + 0x484,  D_80010DEC + 0x470,  D_80010DEC + 0x45C,
    D_80010DEC + 0x448,  D_80010DEC + 0x434,  D_80010DEC + 0x420,
    D_80010DEC + 0x40C,  D_80010DEC + 0x3F8,  D_80010DEC + 0x3E4,
    D_80010DEC + 0x3D0,  D_80010DEC + 0x3BC,  D_80010DEC + 0x3A8,
    D_80010DEC + 0x394,  D_80010DEC + 0x380,  D_80010DEC + 0x36C,
    D_80010DEC + 0x358,  D_80010DEC + 0x344,  D_80010DEC + 0x330,
    D_80010DEC + 0x31C,  D_80010DEC + 0x308,  D_80010DEC + 0x2F4,
    D_80010DEC + 0x2E0,  D_80010DEC + 0x2CC,  D_80010DEC + 0x2B8,
    D_80010DEC + 0x2A4,  D_80010DEC + 0x290,  D_80010DEC + 0x27C,
    D_80010DEC + 0x268,  D_80010DEC + 0x254,  D_80010DEC + 0x240,
    D_80010DEC + 0x22C,  D_80010DEC + 0x218,  D_80010DEC + 0x204,
    D_80010DEC + 0x1F0,  D_80010DEC + 0x1DC,  D_80010DEC + 0x1C8,
    D_80010DEC + 0x1B4,  D_80010DEC + 0x1A0,  D_80010DEC + 0x18C,
    D_80010DEC + 0x178,  D_80010DEC + 0x164,  D_80010DEC + 0x150,
    D_80010DEC + 0x13C,  D_80010DEC + 0x128,  D_80010DEC + 0x114,
    D_80010DEC + 0x100,  D_80010DEC + 0xEC,   D_80010DEC + 0xD8,
    D_80010DEC + 0xC4,   D_80010DEC + 0xB0,   D_80010DEC + 0x9C,
    D_80010DEC + 0x88,   D_80010DEC + 0x74,   D_80010DEC + 0x60,
    D_80010DEC + 0x4C,   D_80010DEC + 0x38,   D_80010DEC + 0x24,
    D_80010DEC + 0x10,   D_80010DEC};

NdataInfEntry g_ndata_inf[1024] = {
    {0, 73},      {73, 70},     {143, 73},    {216, 72},    {288, 71},
    {359, 73},    {432, 72},    {504, 68},    {572, 65},    {637, 74},
    {711, 61},    {772, 62},    {834, 77},    {911, 59},    {970, 70},
    {1040, 69},   {1109, 73},   {1182, 73},   {1255, 73},   {1328, 74},
    {1402, 71},   {1473, 70},   {1543, 69},   {1612, 65},   {1677, 68},
    {1745, 64},   {1809, 66},   {1875, 72},   {1947, 68},   {2015, 77},
    {2092, 82},   {2174, 19},   {2193, 26},   {2219, 23},   {2242, 25},
    {2267, 36},   {2303, 76},   {2379, 131},  {2510, 132},  {2642, 53},
    {2695, 68},   {2763, 69},   {2832, 77},   {2909, 77},   {2986, 68},
    {3054, 74},   {3128, 51},   {3179, 71},   {3250, 77},   {3327, 72},
    {3399, 49},   {3448, 53},   {3501, 70},   {3571, 59},   {3630, 50},
    {3680, 72},   {3752, 56},   {3808, 57},   {3865, 64},   {3929, 48},
    {3977, 60},   {4037, 68},   {4105, 48},   {4153, 60},   {4213, 87},
    {4300, 76},   {4376, 62},   {4438, 64},   {4502, 78},   {4580, 64},
    {4644, 69},   {4713, 53},   {4766, 64},   {4830, 85},   {4915, 50},
    {4965, 46},   {5011, 96},   {5107, 6},    {5113, 6},    {5119, 6},
    {5125, 6},    {5131, 6},    {5137, 6},    {5143, 6},    {5149, 6},
    {5155, 6},    {5161, 6},    {5167, 6},    {5173, 6},    {5179, 6},
    {5185, 6},    {5191, 6},    {5197, 6},    {5203, 6},    {5209, 6},
    {5215, 6},    {5221, 6},    {5227, 6},    {5233, 6},    {5239, 6},
    {5245, 6},    {5251, 6},    {5257, 6},    {5263, 6},    {5269, 6},
    {5275, 6},    {5281, 6},    {5287, 6},    {5293, 6},    {5299, 6},
    {5305, 6},    {5311, 6},    {5317, 6},    {5323, 6},    {5329, 6},
    {5335, 6},    {5341, 6},    {5347, 6},    {5353, 6},    {5359, 6},
    {5365, 6},    {5371, 6},    {5377, 6},    {5383, 6},    {5389, 6},
    {5395, 6},    {5401, 6},    {5407, 6},    {5413, 6},    {5419, 6},
    {5425, 6},    {5431, 91},   {5522, 122},  {5644, 121},  {5765, 118},
    {5883, 122},  {6005, 120},  {6125, 116},  {6241, 119},  {6360, 114},
    {6474, 118},  {6592, 112},  {6704, 115},  {6819, 112},  {6931, 115},
    {7046, 118},  {7164, 113},  {7277, 95},   {7372, 125},  {7497, 121},
    {7618, 126},  {7744, 124},  {7868, 120},  {7988, 123},  {8111, 118},
    {8229, 122},  {8351, 116},  {8467, 119},  {8586, 115},  {8701, 119},
    {8820, 122},  {8942, 117},  {9059, 93},   {9152, 120},  {9272, 124},
    {9396, 123},  {9519, 118},  {9637, 121},  {9758, 117},  {9875, 120},
    {9995, 114},  {10109, 118}, {10227, 114}, {10341, 118}, {10459, 121},
    {10580, 115}, {10695, 90},  {10785, 121}, {10906, 120}, {11026, 115},
    {11141, 118}, {11259, 113}, {11372, 117}, {11489, 111}, {11600, 115},
    {11715, 111}, {11826, 114}, {11940, 117}, {12057, 112}, {12169, 94},
    {12263, 124}, {12387, 119}, {12506, 122}, {12628, 118}, {12746, 122},
    {12868, 115}, {12983, 119}, {13102, 115}, {13217, 119}, {13336, 122},
    {13458, 116}, {13574, 93},  {13667, 118}, {13785, 121}, {13906, 116},
    {14022, 120}, {14142, 114}, {14256, 117}, {14373, 114}, {14487, 117},
    {14604, 120}, {14724, 115}, {14839, 88},  {14927, 116}, {15043, 112},
    {15155, 115}, {15270, 109}, {15379, 113}, {15492, 109}, {15601, 113},
    {15714, 116}, {15830, 110}, {15940, 91},  {16031, 115}, {16146, 119},
    {16265, 112}, {16377, 116}, {16493, 112}, {16605, 116}, {16721, 119},
    {16840, 113}, {16953, 87},  {17040, 114}, {17154, 108}, {17262, 111},
    {17373, 107}, {17480, 111}, {17591, 114}, {17705, 109}, {17814, 91},
    {17905, 111}, {18016, 115}, {18131, 111}, {18242, 115}, {18357, 118},
    {18475, 112}, {18587, 84},  {18671, 109}, {18780, 105}, {18885, 109},
    {18994, 112}, {19106, 106}, {19212, 88},  {19300, 109}, {19409, 112},
    {19521, 115}, {19636, 110}, {19746, 84},  {19830, 108}, {19938, 111},
    {20049, 106}, {20155, 88},  {20243, 115}, {20358, 110}, {20468, 91},
    {20559, 113}, {20672, 85},  {20757, 63},  {20820, 30},  {20850, 50},
    {20900, 50},  {20950, 50},  {21000, 50},  {21050, 50},  {21100, 44},
    {21144, 44},  {21188, 30},  {21218, 50},  {21268, 50},  {21318, 50},
    {21368, 50},  {21418, 44},  {21462, 44},  {21506, 30},  {21536, 50},
    {21586, 50},  {21636, 50},  {21686, 44},  {21730, 44},  {21774, 30},
    {21804, 50},  {21854, 50},  {21904, 44},  {21948, 44},  {21992, 30},
    {22022, 50},  {22072, 44},  {22116, 44},  {22160, 30},  {22190, 44},
    {22234, 44},  {22278, 24},  {22302, 38},  {22340, 24},  {22364, 10},
    {22374, 14},  {22388, 8},   {22396, 3},   {22399, 28},  {22427, 9},
    {22436, 1},   {22437, 3},   {22440, 3},   {22443, 4},   {22447, 6},
    {22453, 11},  {22464, 11},  {22475, 9},   {22484, 63},  {22547, 8},
    {22555, 4},   {22559, 3},   {22562, 5},   {22567, 6},   {22573, 9},
    {22582, 3},   {22585, 11},  {22596, 97},  {-1, -1},     {-1, -1},
    {-1, -1},     {-1, -1},     {-1, -1},     {-1, -1},     {-1, -1},
    {-1, -1},     {-1, -1},     {22693, 14},  {22707, 9},   {22716, 4},
    {22720, 25},  {22745, 8},   {22753, 2},   {22755, 3},   {22758, 3},
    {22761, 4},   {22765, 12},  {22777, 8},   {22785, 6},   {22791, 5},
    {22796, 55},  {22851, 10},  {22861, 5},   {22866, 2},   {22868, 4},
    {22872, 6},   {22878, 8},   {22886, 4},   {22890, 9},   {22899, 77},
    {22976, 68},  {-1, -1},     {-1, -1},     {-1, -1},     {-1, -1},
    {-1, -1},     {-1, -1},     {-1, -1},     {-1, -1},     {23044, 19},
    {23063, 7},   {23070, 5},   {23075, 30},  {23105, 9},   {23114, 2},
    {23116, 3},   {23119, 3},   {23122, 2},   {23124, 6},   {23130, 12},
    {23142, 11},  {23153, 13},  {23166, 51},  {23217, 9},   {23226, 3},
    {23229, 2},   {23231, 3},   {23234, 5},   {23239, 12},  {23251, 2},
    {23253, 13},  {23266, 130}, {-1, -1},     {-1, -1},     {-1, -1},
    {-1, -1},     {-1, -1},     {-1, -1},     {-1, -1},     {-1, -1},
    {-1, -1},     {-1, -1},     {23396, 8},   {23404, 12},  {23416, 3},
    {23419, 9},   {23428, 3},   {23431, 6},   {23437, 3},   {23440, 7},
    {23447, 11},  {23458, 39},  {23497, 6},   {23503, 3},   {23506, 3},
    {23509, 2},   {23511, 3},   {23514, 9},   {23523, 4},   {23527, 15},
    {23542, 62},  {23604, 56},  {-1, -1},     {-1, -1},     {-1, -1},
    {-1, -1},     {-1, -1},     {-1, -1},     {-1, -1},     {23660, 11},
    {23671, 5},   {23676, 1},   {23677, 52},  {23729, 12},  {23741, 4},
    {23745, 2},   {23747, 3},   {23750, 5},   {23755, 10},  {23765, 4},
    {23769, 10},  {23779, 4},   {23783, 26},  {23809, 8},   {23817, 2},
    {23819, 3},   {23822, 2},   {23824, 4},   {23828, 7},   {23835, 4},
    {23839, 10},  {23849, 63},  {23912, 33},  {-1, -1},     {-1, -1},
    {-1, -1},     {-1, -1},     {-1, -1},     {-1, -1},     {23945, 10},
    {23955, 9},   {23964, 3},   {23967, 10},  {23977, 3},   {23980, 6},
    {23986, 2},   {23988, 9},   {23997, 12},  {24009, 39},  {24048, 7},
    {24055, 2},   {24057, 3},   {24060, 3},   {24063, 4},   {24067, 9},
    {24076, 3},   {24079, 10},  {24089, 66},  {-1, -1},     {-1, -1},
    {-1, -1},     {-1, -1},     {-1, -1},     {-1, -1},     {-1, -1},
    {-1, -1},     {24155, 9},   {24164, 7},   {24171, 3},   {24174, 84},
    {24258, 7},   {24265, 2},   {24267, 3},   {24270, 2},   {24272, 3},
    {24275, 9},   {24284, 5},   {24289, 9},   {24298, 3},   {24301, 30},
    {24331, 9},   {24340, 4},   {24344, 4},   {24348, 3},   {24351, 5},
    {24356, 7},   {24363, 4},   {24367, 11},  {24378, 57},  {24435, 33},
    {-1, -1},     {-1, -1},     {-1, -1},     {-1, -1},     {-1, -1},
    {-1, -1},     {24468, 10},  {24478, 6},   {24484, 2},   {24486, 49},
    {24535, 9},   {24544, 3},   {24547, 3},   {24550, 5},   {24555, 2},
    {24557, 12},  {24569, 4},   {24573, 8},   {24581, 3},   {24584, 38},
    {24622, 7},   {24629, 3},   {24632, 1},   {24633, 2},   {24635, 1},
    {24636, 9},   {24645, 4},   {24649, 8},   {24657, 70},  {24727, 69},
    {-1, -1},     {-1, -1},     {-1, -1},     {-1, -1},     {-1, -1},
    {-1, -1},     {24796, 14},  {24810, 8},   {24818, 3},   {24821, 67},
    {24888, 10},  {24898, 3},   {24901, 4},   {24905, 3},   {24908, 5},
    {24913, 11},  {24924, 3},   {24927, 11},  {24938, 10},  {24948, 40},
    {24988, 8},   {24996, 4},   {25000, 3},   {25003, 2},   {25005, 4},
    {25009, 9},   {25018, 3},   {25021, 9},   {25030, 76},  {25106, 74},
    {-1, -1},     {-1, -1},     {-1, -1},     {-1, -1},     {-1, -1},
    {-1, -1},     {-1, -1},     {25180, 8},   {25188, 8},   {25196, 11},
    {25207, 3},   {25210, 34},  {25244, 7},   {25251, 3},   {25254, 3},
    {25257, 4},   {25261, 3},   {25264, 8},   {25272, 13},  {25285, 11},
    {25296, 6},   {25302, 80},  {25382, 10},  {25392, 4},   {25396, 3},
    {25399, 3},   {25402, 3},   {25405, 8},   {25413, 3},   {25416, 11},
    {25427, 9},   {25436, 67},  {25503, 47},  {25550, 29},  {25579, 22},
    {-1, -1},     {-1, -1},     {-1, -1},     {-1, -1},     {-1, -1},
    {-1, -1},     {-1, -1},     {25601, 6},   {25607, 9},   {25616, 8},
    {25624, 3},   {25627, 27},  {25654, 7},   {25661, 3},   {25664, 1},
    {25665, 5},   {25670, 3},   {25673, 7},   {25680, 11},  {25691, 7},
    {25698, 6},   {25704, 47},  {25751, 10},  {25761, 3},   {25764, 1},
    {25765, 5},   {25770, 3},   {25773, 9},   {25782, 3},   {25785, 7},
    {25792, 8},   {25800, 54},  {25854, 53},  {25907, 16},  {-1, -1},
    {-1, -1},     {-1, -1},     {-1, -1},     {-1, -1},     {-1, -1},
    {-1, -1},     {-1, -1},     {25923, 13},  {25936, 10},  {25946, 10},
    {25956, 2},   {25958, 46},  {26004, 7},   {26011, 4},   {26015, 2},
    {26017, 4},   {26021, 2},   {26023, 8},   {26031, 13},  {26044, 10},
    {26054, 6},   {26060, 68},  {26128, 7},   {26135, 2},   {26137, 5},
    {26142, 4},   {26146, 4},   {26150, 14},  {26164, 3},   {26167, 13},
    {26180, 11},  {26191, 61},  {26252, 47},  {26299, 13},  {-1, -1},
    {-1, -1},     {-1, -1},     {-1, -1},     {-1, -1},     {-1, -1},
    {-1, -1},     {-1, -1},     {26312, 10},  {26322, 9},   {26331, 11},
    {26342, 2},   {26344, 47},  {26391, 10},  {26401, 2},   {26403, 2},
    {26405, 3},   {26408, 3},   {26411, 11},  {26422, 14},  {26436, 12},
    {26448, 5},   {26453, 47},  {26500, 6},   {26506, 2},   {26508, 1},
    {26509, 4},   {26513, 3},   {26516, 8},   {26524, 4},   {26528, 14},
    {26542, 8},   {26550, 56},  {26606, 33},  {26639, 12},  {-1, -1},
    {-1, -1},     {-1, -1},     {-1, -1},     {-1, -1},     {-1, -1},
    {-1, -1},     {-1, -1},     {26651, 6},   {26657, 6},   {26663, 6},
    {26669, 2},   {26671, 43},  {26714, 9},   {26723, 3},   {26726, 3},
    {26729, 4},   {26733, 4},   {26737, 7},   {26744, 15},  {26759, 14},
    {26773, 9},   {26782, 28},  {26810, 7},   {26817, 4},   {26821, 3},
    {26824, 4},   {26828, 4},   {26832, 9},   {26841, 2},   {26843, 11},
    {26854, 8},   {26862, 51},  {26913, 41},  {26954, 12},  {-1, -1},
    {-1, -1},     {-1, -1},     {-1, -1},     {-1, -1},     {-1, -1},
    {-1, -1},     {-1, -1},     {26966, 10},  {26976, 12},  {26988, 8},
    {26996, 3},   {26999, 8},   {27007, 3},   {27010, 9},   {27019, 15},
    {27034, 9},   {27043, 4},   {27047, 50},  {27097, 8},   {27105, 4},
    {27109, 2},   {27111, 3},   {27114, 5},   {27119, 12},  {27131, 5},
    {27136, 10},  {27146, 10},  {27156, 56},  {27212, 42},  {27254, 18},
    {-1, -1},     {-1, -1},     {-1, -1},     {-1, -1},     {-1, -1},
    {-1, -1},     {-1, -1},     {-1, -1},     {27272, 10},  {27282, 7},
    {27289, 5},   {27294, 5},   {27299, 7},   {27306, 3},   {27309, 7},
    {27316, 11},  {27327, 10},  {27337, 6},   {27343, 41},  {27384, 7},
    {27391, 4},   {27395, 3},   {27398, 4},   {27402, 2},   {27404, 14},
    {27418, 3},   {27421, 9},   {27430, 9},   {27439, 55},  {27494, 33},
    {27527, 14},  {-1, -1},     {-1, -1},     {-1, -1},     {-1, -1},
    {-1, -1},     {-1, -1},     {-1, -1},     {-1, -1},     {27541, 8},
    {27549, 7},   {27556, 6},   {27562, 3},   {27565, 42},  {27607, 8},
    {27615, 4},   {27619, 2},   {27621, 3},   {27624, 3},   {27627, 13},
    {27640, 15},  {27655, 9},   {27664, 6},   {27670, 37},  {27707, 7},
    {27714, 3},   {27717, 3},   {27720, 3},   {27723, 6},   {27729, 10},
    {27739, 3},   {27742, 10},  {27752, 9},   {27761, 53},  {27814, 36},
    {27850, 10},  {-1, -1},     {-1, -1},     {-1, -1},     {-1, -1},
    {-1, -1},     {-1, -1},     {-1, -1},     {-1, -1},     {27860, 9},
    {27869, 10},  {27879, 10},  {27889, 2},   {27891, 60},  {27951, 6},
    {27957, 2},   {27959, 4},   {27963, 4},   {27967, 2},   {27969, 9},
    {27978, 14},  {27992, 10},  {28002, 2},   {28004, 38},  {28042, 9},
    {28051, 5},   {28056, 2},   {28058, 3},   {28061, 2},   {28063, 9},
    {28072, 4},   {28076, 10},  {28086, 8},   {28094, 57},  {28151, 40},
    {28191, 14},  {-1, -1},     {-1, -1},     {-1, -1},     {-1, -1},
    {-1, -1},     {-1, -1},     {-1, -1},     {-1, -1},     {28205, 19},
    {28224, 20},  {28244, 20},  {28264, 20},  {28284, 20},  {28304, 20},
    {28324, 7},   {28331, 7},   {28338, 7},   {28345, 7},   {28352, 5},
    {28357, 5},   {28362, 6},   {28368, 8},   {-1, -1}};
s16 g_ndata_pack_index[8] = {0, 1, 2, 3, 4, -1, 0, 0};
s16 Judge[0x1000] = {
    0,     6,     12,    18,    25,    31,    37,    43,    50,    56,    62,
    69,    75,    81,    87,    94,    100,   106,   113,   119,   125,   131,
    138,   144,   150,   157,   163,   169,   175,   182,   188,   194,   200,
    207,   213,   219,   226,   232,   238,   244,   251,   257,   263,   269,
    276,   282,   288,   295,   301,   307,   313,   320,   326,   332,   338,
    345,   351,   357,   363,   370,   376,   382,   388,   395,   401,   407,
    413,   420,   426,   432,   438,   445,   451,   457,   463,   470,   476,
    482,   488,   495,   501,   507,   513,   520,   526,   532,   538,   545,
    551,   557,   563,   569,   576,   582,   588,   594,   601,   607,   613,
    619,   625,   632,   638,   644,   650,   656,   663,   669,   675,   681,
    687,   694,   700,   706,   712,   718,   725,   731,   737,   743,   749,
    755,   762,   768,   774,   780,   786,   792,   799,   805,   811,   817,
    823,   829,   836,   842,   848,   854,   860,   866,   872,   879,   885,
    891,   897,   903,   909,   915,   921,   928,   934,   940,   946,   952,
    958,   964,   970,   976,   983,   989,   995,   1001,  1007,  1013,  1019,
    1025,  1031,  1037,  1043,  1050,  1056,  1062,  1068,  1074,  1080,  1086,
    1092,  1098,  1104,  1110,  1116,  1122,  1128,  1134,  1140,  1146,  1152,
    1158,  1164,  1170,  1176,  1182,  1189,  1195,  1201,  1207,  1213,  1219,
    1225,  1231,  1237,  1243,  1248,  1254,  1260,  1266,  1272,  1278,  1284,
    1290,  1296,  1302,  1308,  1314,  1320,  1326,  1332,  1338,  1344,  1350,
    1356,  1362,  1368,  1373,  1379,  1385,  1391,  1397,  1403,  1409,  1415,
    1421,  1427,  1433,  1438,  1444,  1450,  1456,  1462,  1468,  1474,  1479,
    1485,  1491,  1497,  1503,  1509,  1515,  1520,  1526,  1532,  1538,  1544,
    1550,  1555,  1561,  1567,  1573,  1579,  1584,  1590,  1596,  1602,  1608,
    1613,  1619,  1625,  1631,  1636,  1642,  1648,  1654,  1659,  1665,  1671,
    1677,  1682,  1688,  1694,  1699,  1705,  1711,  1717,  1722,  1728,  1734,
    1739,  1745,  1751,  1756,  1762,  1768,  1773,  1779,  1785,  1790,  1796,
    1802,  1807,  1813,  1819,  1824,  1830,  1835,  1841,  1847,  1852,  1858,
    1864,  1869,  1875,  1880,  1886,  1891,  1897,  1903,  1908,  1914,  1919,
    1925,  1930,  1936,  1941,  1947,  1952,  1958,  1964,  1969,  1975,  1980,
    1986,  1991,  1997,  2002,  2007,  2013,  2018,  2024,  2029,  2035,  2040,
    2046,  2051,  2057,  2062,  2067,  2073,  2078,  2084,  2089,  2094,  2100,
    2105,  2111,  2116,  2121,  2127,  2132,  2138,  2143,  2148,  2154,  2159,
    2164,  2170,  2175,  2180,  2186,  2191,  2196,  2201,  2207,  2212,  2217,
    2223,  2228,  2233,  2238,  2244,  2249,  2254,  2259,  2265,  2270,  2275,
    2280,  2286,  2291,  2296,  2301,  2306,  2312,  2317,  2322,  2327,  2332,
    2337,  2343,  2348,  2353,  2358,  2363,  2368,  2373,  2379,  2384,  2389,
    2394,  2399,  2404,  2409,  2414,  2419,  2424,  2429,  2434,  2439,  2445,
    2450,  2455,  2460,  2465,  2470,  2475,  2480,  2485,  2490,  2495,  2500,
    2505,  2510,  2515,  2519,  2524,  2529,  2534,  2539,  2544,  2549,  2554,
    2559,  2564,  2569,  2574,  2578,  2583,  2588,  2593,  2598,  2603,  2608,
    2613,  2617,  2622,  2627,  2632,  2637,  2641,  2646,  2651,  2656,  2661,
    2665,  2670,  2675,  2680,  2684,  2689,  2694,  2699,  2703,  2708,  2713,
    2717,  2722,  2727,  2732,  2736,  2741,  2746,  2750,  2755,  2760,  2764,
    2769,  2773,  2778,  2783,  2787,  2792,  2796,  2801,  2806,  2810,  2815,
    2819,  2824,  2828,  2833,  2837,  2842,  2847,  2851,  2856,  2860,  2865,
    2869,  2874,  2878,  2882,  2887,  2891,  2896,  2900,  2905,  2909,  2914,
    2918,  2922,  2927,  2931,  2936,  2940,  2944,  2949,  2953,  2957,  2962,
    2966,  2970,  2975,  2979,  2983,  2988,  2992,  2996,  3000,  3005,  3009,
    3013,  3018,  3022,  3026,  3030,  3034,  3039,  3043,  3047,  3051,  3055,
    3060,  3064,  3068,  3072,  3076,  3080,  3085,  3089,  3093,  3097,  3101,
    3105,  3109,  3113,  3117,  3121,  3126,  3130,  3134,  3138,  3142,  3146,
    3150,  3154,  3158,  3162,  3166,  3170,  3174,  3178,  3182,  3186,  3190,
    3193,  3197,  3201,  3205,  3209,  3213,  3217,  3221,  3225,  3229,  3232,
    3236,  3240,  3244,  3248,  3252,  3255,  3259,  3263,  3267,  3271,  3274,
    3278,  3282,  3286,  3289,  3293,  3297,  3301,  3304,  3308,  3312,  3315,
    3319,  3323,  3326,  3330,  3334,  3337,  3341,  3345,  3348,  3352,  3356,
    3359,  3363,  3366,  3370,  3373,  3377,  3381,  3384,  3388,  3391,  3395,
    3398,  3402,  3405,  3409,  3412,  3416,  3419,  3423,  3426,  3429,  3433,
    3436,  3440,  3443,  3447,  3450,  3453,  3457,  3460,  3463,  3467,  3470,
    3473,  3477,  3480,  3483,  3487,  3490,  3493,  3497,  3500,  3503,  3506,
    3510,  3513,  3516,  3519,  3522,  3526,  3529,  3532,  3535,  3538,  3541,
    3545,  3548,  3551,  3554,  3557,  3560,  3563,  3566,  3570,  3573,  3576,
    3579,  3582,  3585,  3588,  3591,  3594,  3597,  3600,  3603,  3606,  3609,
    3612,  3615,  3618,  3621,  3624,  3627,  3629,  3632,  3635,  3638,  3641,
    3644,  3647,  3650,  3652,  3655,  3658,  3661,  3664,  3667,  3669,  3672,
    3675,  3678,  3680,  3683,  3686,  3689,  3691,  3694,  3697,  3700,  3702,
    3705,  3708,  3710,  3713,  3716,  3718,  3721,  3723,  3726,  3729,  3731,
    3734,  3736,  3739,  3742,  3744,  3747,  3749,  3752,  3754,  3757,  3759,
    3762,  3764,  3767,  3769,  3772,  3774,  3776,  3779,  3781,  3784,  3786,
    3789,  3791,  3793,  3796,  3798,  3800,  3803,  3805,  3807,  3810,  3812,
    3814,  3816,  3819,  3821,  3823,  3826,  3828,  3830,  3832,  3834,  3837,
    3839,  3841,  3843,  3845,  3848,  3850,  3852,  3854,  3856,  3858,  3860,
    3862,  3864,  3867,  3869,  3871,  3873,  3875,  3877,  3879,  3881,  3883,
    3885,  3887,  3889,  3891,  3893,  3895,  3897,  3899,  3900,  3902,  3904,
    3906,  3908,  3910,  3912,  3914,  3915,  3917,  3919,  3921,  3923,  3925,
    3926,  3928,  3930,  3932,  3933,  3935,  3937,  3939,  3940,  3942,  3944,
    3945,  3947,  3949,  3950,  3952,  3954,  3955,  3957,  3959,  3960,  3962,
    3963,  3965,  3967,  3968,  3970,  3971,  3973,  3974,  3976,  3977,  3979,
    3980,  3982,  3983,  3985,  3986,  3988,  3989,  3990,  3992,  3993,  3995,
    3996,  3997,  3999,  4000,  4001,  4003,  4004,  4005,  4007,  4008,  4009,
    4011,  4012,  4013,  4014,  4016,  4017,  4018,  4019,  4020,  4022,  4023,
    4024,  4025,  4026,  4027,  4029,  4030,  4031,  4032,  4033,  4034,  4035,
    4036,  4037,  4038,  4039,  4040,  4041,  4042,  4043,  4044,  4045,  4046,
    4047,  4048,  4049,  4050,  4051,  4052,  4053,  4054,  4055,  4056,  4057,
    4057,  4058,  4059,  4060,  4061,  4062,  4062,  4063,  4064,  4065,  4065,
    4066,  4067,  4068,  4068,  4069,  4070,  4071,  4071,  4072,  4073,  4073,
    4074,  4075,  4075,  4076,  4076,  4077,  4078,  4078,  4079,  4079,  4080,
    4080,  4081,  4081,  4082,  4082,  4083,  4083,  4084,  4084,  4085,  4085,
    4086,  4086,  4087,  4087,  4087,  4088,  4088,  4089,  4089,  4089,  4090,
    4090,  4090,  4091,  4091,  4091,  4091,  4092,  4092,  4092,  4092,  4093,
    4093,  4093,  4093,  4094,  4094,  4094,  4094,  4094,  4094,  4095,  4095,
    4095,  4095,  4095,  4095,  4095,  4095,  4095,  4095,  4095,  4095,  4095,
    4095,  4096,  4095,  4095,  4095,  4095,  4095,  4095,  4095,  4095,  4095,
    4095,  4095,  4095,  4095,  4095,  4094,  4094,  4094,  4094,  4094,  4094,
    4093,  4093,  4093,  4093,  4092,  4092,  4092,  4092,  4091,  4091,  4091,
    4091,  4090,  4090,  4090,  4089,  4089,  4089,  4088,  4088,  4087,  4087,
    4087,  4086,  4086,  4085,  4085,  4084,  4084,  4083,  4083,  4082,  4082,
    4081,  4081,  4080,  4080,  4079,  4079,  4078,  4078,  4077,  4076,  4076,
    4075,  4075,  4074,  4073,  4073,  4072,  4071,  4071,  4070,  4069,  4068,
    4068,  4067,  4066,  4065,  4065,  4064,  4063,  4062,  4062,  4061,  4060,
    4059,  4058,  4057,  4057,  4056,  4055,  4054,  4053,  4052,  4051,  4050,
    4049,  4048,  4047,  4046,  4045,  4044,  4043,  4042,  4041,  4040,  4039,
    4038,  4037,  4036,  4035,  4034,  4033,  4032,  4031,  4030,  4029,  4027,
    4026,  4025,  4024,  4023,  4022,  4020,  4019,  4018,  4017,  4016,  4014,
    4013,  4012,  4011,  4009,  4008,  4007,  4005,  4004,  4003,  4001,  4000,
    3999,  3997,  3996,  3995,  3993,  3992,  3990,  3989,  3988,  3986,  3985,
    3983,  3982,  3980,  3979,  3977,  3976,  3974,  3973,  3971,  3970,  3968,
    3967,  3965,  3963,  3962,  3960,  3959,  3957,  3955,  3954,  3952,  3950,
    3949,  3947,  3945,  3944,  3942,  3940,  3939,  3937,  3935,  3933,  3932,
    3930,  3928,  3926,  3925,  3923,  3921,  3919,  3917,  3915,  3914,  3912,
    3910,  3908,  3906,  3904,  3902,  3900,  3899,  3897,  3895,  3893,  3891,
    3889,  3887,  3885,  3883,  3881,  3879,  3877,  3875,  3873,  3871,  3869,
    3867,  3864,  3862,  3860,  3858,  3856,  3854,  3852,  3850,  3848,  3845,
    3843,  3841,  3839,  3837,  3834,  3832,  3830,  3828,  3826,  3823,  3821,
    3819,  3816,  3814,  3812,  3810,  3807,  3805,  3803,  3800,  3798,  3796,
    3793,  3791,  3789,  3786,  3784,  3781,  3779,  3776,  3774,  3772,  3769,
    3767,  3764,  3762,  3759,  3757,  3754,  3752,  3749,  3747,  3744,  3742,
    3739,  3736,  3734,  3731,  3729,  3726,  3723,  3721,  3718,  3716,  3713,
    3710,  3708,  3705,  3702,  3700,  3697,  3694,  3691,  3689,  3686,  3683,
    3680,  3678,  3675,  3672,  3669,  3667,  3664,  3661,  3658,  3655,  3652,
    3650,  3647,  3644,  3641,  3638,  3635,  3632,  3629,  3627,  3624,  3621,
    3618,  3615,  3612,  3609,  3606,  3603,  3600,  3597,  3594,  3591,  3588,
    3585,  3582,  3579,  3576,  3573,  3570,  3566,  3563,  3560,  3557,  3554,
    3551,  3548,  3545,  3541,  3538,  3535,  3532,  3529,  3526,  3522,  3519,
    3516,  3513,  3510,  3506,  3503,  3500,  3497,  3493,  3490,  3487,  3483,
    3480,  3477,  3473,  3470,  3467,  3463,  3460,  3457,  3453,  3450,  3447,
    3443,  3440,  3436,  3433,  3429,  3426,  3423,  3419,  3416,  3412,  3409,
    3405,  3402,  3398,  3395,  3391,  3388,  3384,  3381,  3377,  3373,  3370,
    3366,  3363,  3359,  3356,  3352,  3348,  3345,  3341,  3337,  3334,  3330,
    3326,  3323,  3319,  3315,  3312,  3308,  3304,  3301,  3297,  3293,  3289,
    3286,  3282,  3278,  3274,  3271,  3267,  3263,  3259,  3255,  3252,  3248,
    3244,  3240,  3236,  3232,  3229,  3225,  3221,  3217,  3213,  3209,  3205,
    3201,  3197,  3193,  3190,  3186,  3182,  3178,  3174,  3170,  3166,  3162,
    3158,  3154,  3150,  3146,  3142,  3138,  3134,  3130,  3126,  3121,  3117,
    3113,  3109,  3105,  3101,  3097,  3093,  3089,  3085,  3080,  3076,  3072,
    3068,  3064,  3060,  3055,  3051,  3047,  3043,  3039,  3034,  3030,  3026,
    3022,  3018,  3013,  3009,  3005,  3000,  2996,  2992,  2988,  2983,  2979,
    2975,  2970,  2966,  2962,  2957,  2953,  2949,  2944,  2940,  2936,  2931,
    2927,  2922,  2918,  2914,  2909,  2905,  2900,  2896,  2891,  2887,  2882,
    2878,  2874,  2869,  2865,  2860,  2856,  2851,  2847,  2842,  2837,  2833,
    2828,  2824,  2819,  2815,  2810,  2806,  2801,  2796,  2792,  2787,  2783,
    2778,  2773,  2769,  2764,  2760,  2755,  2750,  2746,  2741,  2736,  2732,
    2727,  2722,  2717,  2713,  2708,  2703,  2699,  2694,  2689,  2684,  2680,
    2675,  2670,  2665,  2661,  2656,  2651,  2646,  2641,  2637,  2632,  2627,
    2622,  2617,  2613,  2608,  2603,  2598,  2593,  2588,  2583,  2578,  2574,
    2569,  2564,  2559,  2554,  2549,  2544,  2539,  2534,  2529,  2524,  2519,
    2515,  2510,  2505,  2500,  2495,  2490,  2485,  2480,  2475,  2470,  2465,
    2460,  2455,  2450,  2445,  2439,  2434,  2429,  2424,  2419,  2414,  2409,
    2404,  2399,  2394,  2389,  2384,  2379,  2373,  2368,  2363,  2358,  2353,
    2348,  2343,  2337,  2332,  2327,  2322,  2317,  2312,  2306,  2301,  2296,
    2291,  2286,  2280,  2275,  2270,  2265,  2259,  2254,  2249,  2244,  2238,
    2233,  2228,  2223,  2217,  2212,  2207,  2201,  2196,  2191,  2186,  2180,
    2175,  2170,  2164,  2159,  2154,  2148,  2143,  2138,  2132,  2127,  2121,
    2116,  2111,  2105,  2100,  2094,  2089,  2084,  2078,  2073,  2067,  2062,
    2057,  2051,  2046,  2040,  2035,  2029,  2024,  2018,  2013,  2007,  2002,
    1997,  1991,  1986,  1980,  1975,  1969,  1964,  1958,  1952,  1947,  1941,
    1936,  1930,  1925,  1919,  1914,  1908,  1903,  1897,  1891,  1886,  1880,
    1875,  1869,  1864,  1858,  1852,  1847,  1841,  1835,  1830,  1824,  1819,
    1813,  1807,  1802,  1796,  1790,  1785,  1779,  1773,  1768,  1762,  1756,
    1751,  1745,  1739,  1734,  1728,  1722,  1717,  1711,  1705,  1699,  1694,
    1688,  1682,  1677,  1671,  1665,  1659,  1654,  1648,  1642,  1636,  1631,
    1625,  1619,  1613,  1608,  1602,  1596,  1590,  1584,  1579,  1573,  1567,
    1561,  1555,  1550,  1544,  1538,  1532,  1526,  1520,  1515,  1509,  1503,
    1497,  1491,  1485,  1479,  1474,  1468,  1462,  1456,  1450,  1444,  1438,
    1433,  1427,  1421,  1415,  1409,  1403,  1397,  1391,  1385,  1379,  1373,
    1368,  1362,  1356,  1350,  1344,  1338,  1332,  1326,  1320,  1314,  1308,
    1302,  1296,  1290,  1284,  1278,  1272,  1266,  1260,  1254,  1248,  1243,
    1237,  1231,  1225,  1219,  1213,  1207,  1201,  1195,  1189,  1182,  1176,
    1170,  1164,  1158,  1152,  1146,  1140,  1134,  1128,  1122,  1116,  1110,
    1104,  1098,  1092,  1086,  1080,  1074,  1068,  1062,  1056,  1050,  1043,
    1037,  1031,  1025,  1019,  1013,  1007,  1001,  995,   989,   983,   976,
    970,   964,   958,   952,   946,   940,   934,   928,   921,   915,   909,
    903,   897,   891,   885,   879,   872,   866,   860,   854,   848,   842,
    836,   829,   823,   817,   811,   805,   799,   792,   786,   780,   774,
    768,   762,   755,   749,   743,   737,   731,   725,   718,   712,   706,
    700,   694,   687,   681,   675,   669,   663,   656,   650,   644,   638,
    632,   625,   619,   613,   607,   601,   594,   588,   582,   576,   569,
    563,   557,   551,   545,   538,   532,   526,   520,   513,   507,   501,
    495,   488,   482,   476,   470,   463,   457,   451,   445,   438,   432,
    426,   420,   413,   407,   401,   395,   388,   382,   376,   370,   363,
    357,   351,   345,   338,   332,   326,   320,   313,   307,   301,   295,
    288,   282,   276,   269,   263,   257,   251,   244,   238,   232,   226,
    219,   213,   207,   200,   194,   188,   182,   175,   169,   163,   157,
    150,   144,   138,   131,   125,   119,   113,   106,   100,   94,    87,
    81,    75,    69,    62,    56,    50,    43,    37,    31,    25,    18,
    12,    6,     0,     -6,    -12,   -18,   -25,   -31,   -37,   -43,   -50,
    -56,   -62,   -69,   -75,   -81,   -87,   -94,   -100,  -106,  -113,  -119,
    -125,  -131,  -138,  -144,  -150,  -157,  -163,  -169,  -175,  -182,  -188,
    -194,  -200,  -207,  -213,  -219,  -226,  -232,  -238,  -244,  -251,  -257,
    -263,  -269,  -276,  -282,  -288,  -295,  -301,  -307,  -313,  -320,  -326,
    -332,  -338,  -345,  -351,  -357,  -363,  -370,  -376,  -382,  -388,  -395,
    -401,  -407,  -413,  -420,  -426,  -432,  -438,  -445,  -451,  -457,  -463,
    -470,  -476,  -482,  -488,  -495,  -501,  -507,  -513,  -520,  -526,  -532,
    -538,  -545,  -551,  -557,  -563,  -569,  -576,  -582,  -588,  -594,  -601,
    -607,  -613,  -619,  -625,  -632,  -638,  -644,  -650,  -656,  -663,  -669,
    -675,  -681,  -687,  -694,  -700,  -706,  -712,  -718,  -725,  -731,  -737,
    -743,  -749,  -755,  -762,  -768,  -774,  -780,  -786,  -792,  -799,  -805,
    -811,  -817,  -823,  -829,  -836,  -842,  -848,  -854,  -860,  -866,  -872,
    -879,  -885,  -891,  -897,  -903,  -909,  -915,  -921,  -928,  -934,  -940,
    -946,  -952,  -958,  -964,  -970,  -976,  -983,  -989,  -995,  -1001, -1007,
    -1013, -1019, -1025, -1031, -1037, -1043, -1050, -1056, -1062, -1068, -1074,
    -1080, -1086, -1092, -1098, -1104, -1110, -1116, -1122, -1128, -1134, -1140,
    -1146, -1152, -1158, -1164, -1170, -1176, -1182, -1189, -1195, -1201, -1207,
    -1213, -1219, -1225, -1231, -1237, -1243, -1248, -1254, -1260, -1266, -1272,
    -1278, -1284, -1290, -1296, -1302, -1308, -1314, -1320, -1326, -1332, -1338,
    -1344, -1350, -1356, -1362, -1368, -1373, -1379, -1385, -1391, -1397, -1403,
    -1409, -1415, -1421, -1427, -1433, -1438, -1444, -1450, -1456, -1462, -1468,
    -1474, -1479, -1485, -1491, -1497, -1503, -1509, -1515, -1520, -1526, -1532,
    -1538, -1544, -1550, -1555, -1561, -1567, -1573, -1579, -1584, -1590, -1596,
    -1602, -1608, -1613, -1619, -1625, -1631, -1636, -1642, -1648, -1654, -1659,
    -1665, -1671, -1677, -1682, -1688, -1694, -1699, -1705, -1711, -1717, -1722,
    -1728, -1734, -1739, -1745, -1751, -1756, -1762, -1768, -1773, -1779, -1785,
    -1790, -1796, -1802, -1807, -1813, -1819, -1824, -1830, -1835, -1841, -1847,
    -1852, -1858, -1864, -1869, -1875, -1880, -1886, -1891, -1897, -1903, -1908,
    -1914, -1919, -1925, -1930, -1936, -1941, -1947, -1952, -1958, -1964, -1969,
    -1975, -1980, -1986, -1991, -1997, -2002, -2007, -2013, -2018, -2024, -2029,
    -2035, -2040, -2046, -2051, -2057, -2062, -2067, -2073, -2078, -2084, -2089,
    -2094, -2100, -2105, -2111, -2116, -2121, -2127, -2132, -2138, -2143, -2148,
    -2154, -2159, -2164, -2170, -2175, -2180, -2186, -2191, -2196, -2201, -2207,
    -2212, -2217, -2223, -2228, -2233, -2238, -2244, -2249, -2254, -2259, -2265,
    -2270, -2275, -2280, -2286, -2291, -2296, -2301, -2306, -2312, -2317, -2322,
    -2327, -2332, -2337, -2343, -2348, -2353, -2358, -2363, -2368, -2373, -2379,
    -2384, -2389, -2394, -2399, -2404, -2409, -2414, -2419, -2424, -2429, -2434,
    -2439, -2445, -2450, -2455, -2460, -2465, -2470, -2475, -2480, -2485, -2490,
    -2495, -2500, -2505, -2510, -2515, -2519, -2524, -2529, -2534, -2539, -2544,
    -2549, -2554, -2559, -2564, -2569, -2574, -2578, -2583, -2588, -2593, -2598,
    -2603, -2608, -2613, -2617, -2622, -2627, -2632, -2637, -2641, -2646, -2651,
    -2656, -2661, -2665, -2670, -2675, -2680, -2684, -2689, -2694, -2699, -2703,
    -2708, -2713, -2717, -2722, -2727, -2732, -2736, -2741, -2746, -2750, -2755,
    -2760, -2764, -2769, -2773, -2778, -2783, -2787, -2792, -2796, -2801, -2806,
    -2810, -2815, -2819, -2824, -2828, -2833, -2837, -2842, -2847, -2851, -2856,
    -2860, -2865, -2869, -2874, -2878, -2882, -2887, -2891, -2896, -2900, -2905,
    -2909, -2914, -2918, -2922, -2927, -2931, -2936, -2940, -2944, -2949, -2953,
    -2957, -2962, -2966, -2970, -2975, -2979, -2983, -2988, -2992, -2996, -3000,
    -3005, -3009, -3013, -3018, -3022, -3026, -3030, -3034, -3039, -3043, -3047,
    -3051, -3055, -3060, -3064, -3068, -3072, -3076, -3080, -3085, -3089, -3093,
    -3097, -3101, -3105, -3109, -3113, -3117, -3121, -3126, -3130, -3134, -3138,
    -3142, -3146, -3150, -3154, -3158, -3162, -3166, -3170, -3174, -3178, -3182,
    -3186, -3190, -3193, -3197, -3201, -3205, -3209, -3213, -3217, -3221, -3225,
    -3229, -3232, -3236, -3240, -3244, -3248, -3252, -3255, -3259, -3263, -3267,
    -3271, -3274, -3278, -3282, -3286, -3289, -3293, -3297, -3301, -3304, -3308,
    -3312, -3315, -3319, -3323, -3326, -3330, -3334, -3337, -3341, -3345, -3348,
    -3352, -3356, -3359, -3363, -3366, -3370, -3373, -3377, -3381, -3384, -3388,
    -3391, -3395, -3398, -3402, -3405, -3409, -3412, -3416, -3419, -3423, -3426,
    -3429, -3433, -3436, -3440, -3443, -3447, -3450, -3453, -3457, -3460, -3463,
    -3467, -3470, -3473, -3477, -3480, -3483, -3487, -3490, -3493, -3497, -3500,
    -3503, -3506, -3510, -3513, -3516, -3519, -3522, -3526, -3529, -3532, -3535,
    -3538, -3541, -3545, -3548, -3551, -3554, -3557, -3560, -3563, -3566, -3570,
    -3573, -3576, -3579, -3582, -3585, -3588, -3591, -3594, -3597, -3600, -3603,
    -3606, -3609, -3612, -3615, -3618, -3621, -3624, -3627, -3629, -3632, -3635,
    -3638, -3641, -3644, -3647, -3650, -3652, -3655, -3658, -3661, -3664, -3667,
    -3669, -3672, -3675, -3678, -3680, -3683, -3686, -3689, -3691, -3694, -3697,
    -3700, -3702, -3705, -3708, -3710, -3713, -3716, -3718, -3721, -3723, -3726,
    -3729, -3731, -3734, -3736, -3739, -3742, -3744, -3747, -3749, -3752, -3754,
    -3757, -3759, -3762, -3764, -3767, -3769, -3772, -3774, -3776, -3779, -3781,
    -3784, -3786, -3789, -3791, -3793, -3796, -3798, -3800, -3803, -3805, -3807,
    -3810, -3812, -3814, -3816, -3819, -3821, -3823, -3826, -3828, -3830, -3832,
    -3834, -3837, -3839, -3841, -3843, -3845, -3848, -3850, -3852, -3854, -3856,
    -3858, -3860, -3862, -3864, -3867, -3869, -3871, -3873, -3875, -3877, -3879,
    -3881, -3883, -3885, -3887, -3889, -3891, -3893, -3895, -3897, -3899, -3900,
    -3902, -3904, -3906, -3908, -3910, -3912, -3914, -3915, -3917, -3919, -3921,
    -3923, -3925, -3926, -3928, -3930, -3932, -3933, -3935, -3937, -3939, -3940,
    -3942, -3944, -3945, -3947, -3949, -3950, -3952, -3954, -3955, -3957, -3959,
    -3960, -3962, -3963, -3965, -3967, -3968, -3970, -3971, -3973, -3974, -3976,
    -3977, -3979, -3980, -3982, -3983, -3985, -3986, -3988, -3989, -3990, -3992,
    -3993, -3995, -3996, -3997, -3999, -4000, -4001, -4003, -4004, -4005, -4007,
    -4008, -4009, -4011, -4012, -4013, -4014, -4016, -4017, -4018, -4019, -4020,
    -4022, -4023, -4024, -4025, -4026, -4027, -4029, -4030, -4031, -4032, -4033,
    -4034, -4035, -4036, -4037, -4038, -4039, -4040, -4041, -4042, -4043, -4044,
    -4045, -4046, -4047, -4048, -4049, -4050, -4051, -4052, -4053, -4054, -4055,
    -4056, -4057, -4057, -4058, -4059, -4060, -4061, -4062, -4062, -4063, -4064,
    -4065, -4065, -4066, -4067, -4068, -4068, -4069, -4070, -4071, -4071, -4072,
    -4073, -4073, -4074, -4075, -4075, -4076, -4076, -4077, -4078, -4078, -4079,
    -4079, -4080, -4080, -4081, -4081, -4082, -4082, -4083, -4083, -4084, -4084,
    -4085, -4085, -4086, -4086, -4087, -4087, -4087, -4088, -4088, -4089, -4089,
    -4089, -4090, -4090, -4090, -4091, -4091, -4091, -4091, -4092, -4092, -4092,
    -4092, -4093, -4093, -4093, -4093, -4094, -4094, -4094, -4094, -4094, -4094,
    -4095, -4095, -4095, -4095, -4095, -4095, -4095, -4095, -4095, -4095, -4095,
    -4095, -4095, -4095, -4096, -4095, -4095, -4095, -4095, -4095, -4095, -4095,
    -4095, -4095, -4095, -4095, -4095, -4095, -4095, -4094, -4094, -4094, -4094,
    -4094, -4094, -4093, -4093, -4093, -4093, -4092, -4092, -4092, -4092, -4091,
    -4091, -4091, -4091, -4090, -4090, -4090, -4089, -4089, -4089, -4088, -4088,
    -4087, -4087, -4087, -4086, -4086, -4085, -4085, -4084, -4084, -4083, -4083,
    -4082, -4082, -4081, -4081, -4080, -4080, -4079, -4079, -4078, -4078, -4077,
    -4076, -4076, -4075, -4075, -4074, -4073, -4073, -4072, -4071, -4071, -4070,
    -4069, -4068, -4068, -4067, -4066, -4065, -4065, -4064, -4063, -4062, -4062,
    -4061, -4060, -4059, -4058, -4057, -4057, -4056, -4055, -4054, -4053, -4052,
    -4051, -4050, -4049, -4048, -4047, -4046, -4045, -4044, -4043, -4042, -4041,
    -4040, -4039, -4038, -4037, -4036, -4035, -4034, -4033, -4032, -4031, -4030,
    -4029, -4027, -4026, -4025, -4024, -4023, -4022, -4020, -4019, -4018, -4017,
    -4016, -4014, -4013, -4012, -4011, -4009, -4008, -4007, -4005, -4004, -4003,
    -4001, -4000, -3999, -3997, -3996, -3995, -3993, -3992, -3990, -3989, -3988,
    -3986, -3985, -3983, -3982, -3980, -3979, -3977, -3976, -3974, -3973, -3971,
    -3970, -3968, -3967, -3965, -3963, -3962, -3960, -3959, -3957, -3955, -3954,
    -3952, -3950, -3949, -3947, -3945, -3944, -3942, -3940, -3939, -3937, -3935,
    -3933, -3932, -3930, -3928, -3926, -3925, -3923, -3921, -3919, -3917, -3915,
    -3914, -3912, -3910, -3908, -3906, -3904, -3902, -3900, -3899, -3897, -3895,
    -3893, -3891, -3889, -3887, -3885, -3883, -3881, -3879, -3877, -3875, -3873,
    -3871, -3869, -3867, -3864, -3862, -3860, -3858, -3856, -3854, -3852, -3850,
    -3848, -3845, -3843, -3841, -3839, -3837, -3834, -3832, -3830, -3828, -3826,
    -3823, -3821, -3819, -3816, -3814, -3812, -3810, -3807, -3805, -3803, -3800,
    -3798, -3796, -3793, -3791, -3789, -3786, -3784, -3781, -3779, -3776, -3774,
    -3772, -3769, -3767, -3764, -3762, -3759, -3757, -3754, -3752, -3749, -3747,
    -3744, -3742, -3739, -3736, -3734, -3731, -3729, -3726, -3723, -3721, -3718,
    -3716, -3713, -3710, -3708, -3705, -3702, -3700, -3697, -3694, -3691, -3689,
    -3686, -3683, -3680, -3678, -3675, -3672, -3669, -3667, -3664, -3661, -3658,
    -3655, -3652, -3650, -3647, -3644, -3641, -3638, -3635, -3632, -3629, -3627,
    -3624, -3621, -3618, -3615, -3612, -3609, -3606, -3603, -3600, -3597, -3594,
    -3591, -3588, -3585, -3582, -3579, -3576, -3573, -3570, -3566, -3563, -3560,
    -3557, -3554, -3551, -3548, -3545, -3541, -3538, -3535, -3532, -3529, -3526,
    -3522, -3519, -3516, -3513, -3510, -3506, -3503, -3500, -3497, -3493, -3490,
    -3487, -3483, -3480, -3477, -3473, -3470, -3467, -3463, -3460, -3457, -3453,
    -3450, -3447, -3443, -3440, -3436, -3433, -3429, -3426, -3423, -3419, -3416,
    -3412, -3409, -3405, -3402, -3398, -3395, -3391, -3388, -3384, -3381, -3377,
    -3373, -3370, -3366, -3363, -3359, -3356, -3352, -3348, -3345, -3341, -3337,
    -3334, -3330, -3326, -3323, -3319, -3315, -3312, -3308, -3304, -3301, -3297,
    -3293, -3289, -3286, -3282, -3278, -3274, -3271, -3267, -3263, -3259, -3255,
    -3252, -3248, -3244, -3240, -3236, -3232, -3229, -3225, -3221, -3217, -3213,
    -3209, -3205, -3201, -3197, -3193, -3190, -3186, -3182, -3178, -3174, -3170,
    -3166, -3162, -3158, -3154, -3150, -3146, -3142, -3138, -3134, -3130, -3126,
    -3121, -3117, -3113, -3109, -3105, -3101, -3097, -3093, -3089, -3085, -3080,
    -3076, -3072, -3068, -3064, -3060, -3055, -3051, -3047, -3043, -3039, -3034,
    -3030, -3026, -3022, -3018, -3013, -3009, -3005, -3000, -2996, -2992, -2988,
    -2983, -2979, -2975, -2970, -2966, -2962, -2957, -2953, -2949, -2944, -2940,
    -2936, -2931, -2927, -2922, -2918, -2914, -2909, -2905, -2900, -2896, -2891,
    -2887, -2882, -2878, -2874, -2869, -2865, -2860, -2856, -2851, -2847, -2842,
    -2837, -2833, -2828, -2824, -2819, -2815, -2810, -2806, -2801, -2796, -2792,
    -2787, -2783, -2778, -2773, -2769, -2764, -2760, -2755, -2750, -2746, -2741,
    -2736, -2732, -2727, -2722, -2717, -2713, -2708, -2703, -2699, -2694, -2689,
    -2684, -2680, -2675, -2670, -2665, -2661, -2656, -2651, -2646, -2641, -2637,
    -2632, -2627, -2622, -2617, -2613, -2608, -2603, -2598, -2593, -2588, -2583,
    -2578, -2574, -2569, -2564, -2559, -2554, -2549, -2544, -2539, -2534, -2529,
    -2524, -2519, -2515, -2510, -2505, -2500, -2495, -2490, -2485, -2480, -2475,
    -2470, -2465, -2460, -2455, -2450, -2445, -2439, -2434, -2429, -2424, -2419,
    -2414, -2409, -2404, -2399, -2394, -2389, -2384, -2379, -2373, -2368, -2363,
    -2358, -2353, -2348, -2343, -2337, -2332, -2327, -2322, -2317, -2312, -2306,
    -2301, -2296, -2291, -2286, -2280, -2275, -2270, -2265, -2259, -2254, -2249,
    -2244, -2238, -2233, -2228, -2223, -2217, -2212, -2207, -2201, -2196, -2191,
    -2186, -2180, -2175, -2170, -2164, -2159, -2154, -2148, -2143, -2138, -2132,
    -2127, -2121, -2116, -2111, -2105, -2100, -2094, -2089, -2084, -2078, -2073,
    -2067, -2062, -2057, -2051, -2046, -2040, -2035, -2029, -2024, -2018, -2013,
    -2007, -2002, -1997, -1991, -1986, -1980, -1975, -1969, -1964, -1958, -1952,
    -1947, -1941, -1936, -1930, -1925, -1919, -1914, -1908, -1903, -1897, -1891,
    -1886, -1880, -1875, -1869, -1864, -1858, -1852, -1847, -1841, -1835, -1830,
    -1824, -1819, -1813, -1807, -1802, -1796, -1790, -1785, -1779, -1773, -1768,
    -1762, -1756, -1751, -1745, -1739, -1734, -1728, -1722, -1717, -1711, -1705,
    -1699, -1694, -1688, -1682, -1677, -1671, -1665, -1659, -1654, -1648, -1642,
    -1636, -1631, -1625, -1619, -1613, -1608, -1602, -1596, -1590, -1584, -1579,
    -1573, -1567, -1561, -1555, -1550, -1544, -1538, -1532, -1526, -1520, -1515,
    -1509, -1503, -1497, -1491, -1485, -1479, -1474, -1468, -1462, -1456, -1450,
    -1444, -1438, -1433, -1427, -1421, -1415, -1409, -1403, -1397, -1391, -1385,
    -1379, -1373, -1368, -1362, -1356, -1350, -1344, -1338, -1332, -1326, -1320,
    -1314, -1308, -1302, -1296, -1290, -1284, -1278, -1272, -1266, -1260, -1254,
    -1248, -1243, -1237, -1231, -1225, -1219, -1213, -1207, -1201, -1195, -1189,
    -1182, -1176, -1170, -1164, -1158, -1152, -1146, -1140, -1134, -1128, -1122,
    -1116, -1110, -1104, -1098, -1092, -1086, -1080, -1074, -1068, -1062, -1056,
    -1050, -1043, -1037, -1031, -1025, -1019, -1013, -1007, -1001, -995,  -989,
    -983,  -976,  -970,  -964,  -958,  -952,  -946,  -940,  -934,  -928,  -921,
    -915,  -909,  -903,  -897,  -891,  -885,  -879,  -872,  -866,  -860,  -854,
    -848,  -842,  -836,  -829,  -823,  -817,  -811,  -805,  -799,  -792,  -786,
    -780,  -774,  -768,  -762,  -755,  -749,  -743,  -737,  -731,  -725,  -718,
    -712,  -706,  -700,  -694,  -687,  -681,  -675,  -669,  -663,  -656,  -650,
    -644,  -638,  -632,  -625,  -619,  -613,  -607,  -601,  -594,  -588,  -582,
    -576,  -569,  -563,  -557,  -551,  -545,  -538,  -532,  -526,  -520,  -513,
    -507,  -501,  -495,  -488,  -482,  -476,  -470,  -463,  -457,  -451,  -445,
    -438,  -432,  -426,  -420,  -413,  -407,  -401,  -395,  -388,  -382,  -376,
    -370,  -363,  -357,  -351,  -345,  -338,  -332,  -326,  -320,  -313,  -307,
    -301,  -295,  -288,  -282,  -276,  -269,  -263,  -257,  -251,  -244,  -238,
    -232,  -226,  -219,  -213,  -207,  -200,  -194,  -188,  -182,  -175,  -169,
    -163,  -157,  -150,  -144,  -138,  -131,  -125,  -119,  -113,  -106,  -100,
    -94,   -87,   -81,   -75,   -69,   -62,   -56,   -50,   -43,   -37,   -31,
    -25,   -18,   -12,   -6};
s16 D_800993FC[62] = {
    0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15,
    16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 24, 0,  1,  0,  1,
    2,  3,  2,  3,  11, 11, 13, 13, 22, 22, 14, 14, 17, 17, 27, 28,
    29, 30, 31, 0,  1,  2,  3,  4,  5,  6,  0,  0,  0,  0};

/* Q65: this file's initialized small data (.sdata), in address order. */
s32 D_800A3240 = 1;
s32 g_ndata_pack_loaded = 0;
