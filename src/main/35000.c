/* 44 game functions. .text 0x80044800 (ROM 0x35000). Start boundary: GP (a
 * per-file gp split, Q65). */
#include "common.h"
#define INCLUDE_ASM_USE_MACRO_INC 1
#include "include_asm.h"
#include "bb2.h"
#include <psxsdk/libgte.h>

void func_800433E4();

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
    func_80044100((s32)D_800A9CF8.unk0, a0);
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
        s32 cur = g_arena_free_ptr;
        s32 rest = g_arena_free_size;
        /* FAKE: the dead parameter a0 reused for the new amt, computed into
           $a0 as in the target; a fresh local: score 17 */
        a0 = a1 - cur;
        cur += a0;
        rest -= a0;
        e->amt = a0;
        g_arena_free_ptr = cur;
        g_arena_free_size = rest;
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
    func_80044100(idx, a1);
    func_80044100(a0, a1);
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
        var_6 = func_80044378(last, dst, sp18);
        func_80044010(dst, 6);
    } else {
        var_6 = func_8004428C((s32 *)last, sp18);
        func_80044010((s32 *)last, 6);
    }
    arena_UpdatePeak(var_6);

    q = sp18;
    y = 0;
    i = 0;
    while (*q != -2) {
        if (*q >= 0) {
            switch (sp108[D_800993FC[i]]) {
            case 0:
                func_800433E4(6, y * 2, 0, 0, -0x180, 0xE8);
                break;
            case 1:
                func_800433E4(6, y * 2, 0, 0x40, -0x180, 0xE9);
                break;
            case 2:
                func_800433E4(6, y * 2, 0x80, 0, -0x160, 0xE8);
                break;
            case 3:
                func_800433E4(6, y * 2, 0x80, 0x40, -0x160, 0xE9);
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
    func_80044100(6, a1);
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

/* Q65: this file's initialized small data (.sdata), in address order. */
s32 D_800A3240 = 1;
s32 g_ndata_pack_loaded = 0;
