/* 48 game functions: the row menu (menu_SetEnabledRows, func_80068F70,
 * func_8006B578, ..., gpu_InitPrimBufferPair). .text 0x80068ECC (ROM 0x596CC).
 * Start boundary: SEAM (owner ruling 2026-10-17; Codex seam #99): no direct
 * call or static crosses the cut; 51268.c's .sbss ends at D_800A34F4 and these
 * functions own D_800A34F8..D_800A352C; .rodata 0x80015988+0x28
 * (func_8006B578's table, D_800159A0) follows 51268.o's. */
#define INCLUDE_ASM_USE_MACRO_INC 1
#include "common.h"
#include "include_asm.h"
#include "bb2.h"
#include <psxsdk/libgte.h>
#include <psxsdk/inline_c.h>
#include <psxsdk/gtemac.h>

/* Q65: this file's statics (.sbss), in address order. */
static s32 D_800A34F8;
static Unk800A34FCRec *D_800A34FC;
static s32 D_800A3500;
/* not named by any code or data: size from the gap */
static s32 D_800A3504[2];
static s16 D_800A350C[2];
static s16 D_800A3510[2];
static s32 D_800A3514;
static s32 D_800A3518;
static Unk8006E49CRec *D_800A351C;
static Unk8006E49CRec *D_800A3520;
static Unk8009BD24Block *D_800A3524;
static s16 D_800A3528;
static s32 D_800A352C;

extern void AddPrim(void *, void *);
extern s32 snd_LoadVabPack(Unk8005C2A8Pack *, s16, s32);
/* Declared unprototyped: the callers below pass an s32 where the definition
 * takes a pointer, and func_80069250 calls func_80069E18 with 2 of its 1 args.
 */
s32 func_8006919C();
void func_80069E18();
extern void gpu_LoadVramBottomStrip();

void menu_SetEnabledRows(s32 arg0) {
    s32 *p = &D_8009BC04.rows_on;
    s32 v = *p;
    v &= ~0x1;
    v |= arg0 & 0x1;
    v &= ~0x2;
    v |= arg0 & 0x2;
    v &= ~0x4;
    v |= arg0 & 0x4;
    v &= ~0x8;
    v |= (((u32)arg0 >> 4) & 1) << 3;
    v &= ~0x10;
    v |= (((u32)arg0 >> 5) & 1) << 4;
    v &= ~0x20;
    v |= (((u32)arg0 >> 6) & 1) << 5;
    v &= ~0x40;
    v |= (arg0 << 3) & 0x40;
    v &= ~0x80;
    v |= arg0 & 0x80;
    *p = v;
}

extern s32 D_800A372C;
extern u8 D_800A32C0[8];

s32 func_80068F70(s32 arg0, Unk8009BD24Block *arg1) {
    RECT buf;
    s32 temp_s0;
    s32 var_2;
    Unk800A34FCRec *v0_e49c;

    D_800A3500 = arg0;
    D_800A351C = (Unk8006E49CRec *)arg0;
    temp_s0 = arg0 + 0x58;
    D_800A3500 = temp_s0;
    func_80046AA0();
    func_8006E950(2, (Unk8006E950Head *)D_800A3500);
    D_800A372C = D_800A3500;
    var_2 = func_8006919C(D_800A3500);
    D_800A3500 = var_2;
    v0_e49c = (Unk800A34FCRec *)gpu_InitPrimBufferPair(var_2, D_800A351C);
    v0_e49c->unk_24 = (Unk8006919CRec *)temp_s0;
    D_800A3500 = (s32)v0_e49c;
    D_800A34FC = v0_e49c;
    D_800A3500 = (s32)(v0_e49c + 1);
    {
        s32 var_4 = -0x10;
        s32 var_5;
        s32 flags = D_800A34F8;
        var_5 = D_8009BC04.rows_on;

        flags &= var_4;
        D_800A34F8 = flags;
        if (!((u32)var_5 & 1)) {
            u32 mask;
            s32 var_8;
            mask = (u32)-0x10;
            var_8 = D_8009BC04.rows_on;
            do {
                s32 next;
                s32 lo;
                flags = D_800A34F8;
                lo = flags;
                lo &= 0xF;
                if (lo >= 7) {
                    D_800A34F8 = flags & mask;
                    break;
                }
                next = lo + 1;
                flags = (flags & mask) | (next & 0xF);
                D_800A34F8 = flags;
                flags &= 0xF;
                flags = (u32)var_8 >> flags;
                flags &= 1;
            } while (!flags);
        }
        {
            /* FAKE: one s16 * holder per stored work-block field, set before
             * the D_800A3524 store; plain `*p` stores keep the D_800A3528 and
             * D_800A34F8 stores in place. Member stores through a holder: score
             * 28; straight through D_800A34FC: 39. */
            s16 *p_2A;
            s16 *p_28;
            s16 *p_0E;
            s16 *p_0C;
            s16 *p_10;
            s16 *p_12;
            s32 value;

            value = 5;
            /* FAKE: block fence keeps li v0,5 at the join-block head so
               reorg steals it into all three incoming jump delay slots; without
               it: score 4 */
            do {
            } while (0);
            p_2A = &D_800A34FC->unk_28.half[1];
            p_28 = &D_800A34FC->unk_28.half[0];
            p_0E = &D_800A34FC->unk_0C[1];
            p_0C = &D_800A34FC->unk_0C[0];
            p_10 = &D_800A34FC->unk_10;
            p_12 = &D_800A34FC->unk_12;
            D_800A3524 = arg1;
            /* FAKE: sched fence keeps the D_800A3524 store adjacent to the
               D_800A34FC load instead of sinking below the zero-stores; without
               it: score 2 */
            do {
            } while (0);
            D_800A3518 = 0;
            D_800A3528 = 0;
            *p_2A = value;
            *p_28 = value;
            flags = D_800A34F8 & ~0x1C00;
            D_800A3510[1] = 0;
            D_800A3510[0] = 0;
            D_800A350C[1] = 0;
            D_800A350C[0] = 0;
            flags |= 0x1000;
            D_800A34F8 = flags;
            *p_12 = 0;
            *p_10 = 0;
            *p_0E = 0;
            *p_0C = 0;
            __builtin_memcpy(&buf, D_800A32C0, 8);
            DrawSync(0);
            MoveImage(&buf, 0x3C0, 0x1FE);
            DrawSync(0);
        }
    }
    D_800A34FC->unk_30 = D_800A3524->unk20_0;
    return 1;
}

Unk8006E49CRec *func_80069120(s32 a0) {
    Unk8009BD24Block *v0 = D_800A3524;
    Unk800A34FCRec *v1 = D_800A34FC;
    if (v1->unk_30 != v0->unk20_0) {
        gpu_LoadVramBottomStrip(D_800A372C);
    }
    v0 = D_800A3524;
    v1 = D_800A34FC;
    v1->unk_30 = v0->unk20_0;
    return &D_800A351C[a0];
}

s32 func_8006919C(Unk8006919CRec *a0) {
    s32 i = 0;
    Unk8009B0E0Record ***p = &a0->unk_14;
    do {
        reloc_RelocateOffsetList((s32 *)a0, (s32)*p);
        p++;
        i++;
    } while (i < 12);
    snd_LoadVabPack(a0->unk_00.unk_00, 1, a0->unk_00.unk_04);
    return a0->unk_00.unk_04;
}

void reloc_RelocateOffsetList(s32 *a0, s32 a1) {
    s32 *p = (s32 *)a1;
    if (!*p)
        return;
    while (*p) {
        if (*p != -1) {
            *p = *p + (s32)a0;
        }
        p++;
    }
}

extern void func_8006E390();

s32 func_80069250(s32 arg0, s32 arg1) {
    s32 sp10[10];
    D_800A3514 = 0;
    func_8006E390(sp10, &D_800A3518);
    func_80069E18(sp10, 0);
    if ((arg1 & 0x400040) != 0) {
        snd_QueueSound(1, 0x7F, 0x7F);
        return 1;
    }
    return 0;
}

extern u32 D_800A32D0[2];

s32 func_800692C0(u32 *arg0, s32 arg1, s16 *arg2, s16 *arg3) {
    s32 one;
    s32 sum;
    s32 i;
    s32 a3_off;
    u32 maskA;
    u32 maskB;
    u32 v;
    s16 sval;

    sum = 0;
    i = 0;
    arg1 <<= 4;
    one = 1;
    /* FAKE: do{}while(0) around the loop seats sum in $t2 and the reduced
     * i * 16 in $t1 (without it: score 7); the `one` constant-holder schedules
     * `li $t6,1` at the preheader (the literal: score 2). */
    do {
        a3_off = 0;
        do {
            /* FAKE: byte-offset step (arg3 += a3_off with an element count
             * scores 3) */
            arg3 = (s16 *)((u8 *)arg3 + a3_off);
            maskB = D_800A32D0[i] << arg1;
            maskA = D_800A32C8[i] << arg1;
            if (*arg2 == 0) {
                v = *arg0;
                if (v & maskA) {
                    *arg3 = one;
                } else if (v & maskB) {
                    *arg3 = -1;
                }
            } else {
                v = *arg0;
                if (v & maskA) {
                    *arg2 = 6;
                } else if (v & maskB) {
                    *arg3 = -6;
                }
                sval = *arg2;
                if (sval >= 6) {
                    sum += one << (i * 16);
                    *arg3 = 0;
                    *arg2 = 0;
                } else if (sval < -5) {
                    sum += 2 << (i * 16);
                    *arg3 = 0;
                    *arg2 = 0;
                }
            }
            a3_off += 2;
            i++;
            *arg2 = (u16)*arg2 + (u16)*arg3;
            arg2++;
        } while (i < 2);
    } while (0);
    return sum;
}

s32 menu_UpdateRowMenu(s32 arg0, s32 arg1) {
    extern void func_80069AE4(s32 *, s32, GpuDb *);
    extern void func_8006A880(u8 *, u16 *, s32);
    extern void func_80069F80(s32 *, s32);
    extern void func_8006A1A0(s32 *, s32);
    /* FAKE: frame layout. func_8006E390 fills words 1..9 (siblings: s32
     * sp10[10]); the folded `arg1` word sits at word 12 and the
     * saved row mask at word 14 only to reproduce the target's 0x60 frame
     * (sp+0x40 / sp+0x48); separate locals give the wrong frame: score 11. */
    s32 var_0[15];
    s32 result;
    GpuDb *var_2;
    var_2 = &g_gpu_db[D_800A36AC & 1];
    func_8006E390(var_0, &D_800A3518);
    var_0[12] = (arg1 & 0xFFFF) | ((u32)arg1 >> 16);
    result =
        func_800692C0((u32 *)&var_0[12], 0, D_800A34FC->unk_0C, D_800A350C);
    switch (result >> 16) {
    case 1:
        do {
            s32 flags = D_800A34F8;
            s32 index;
            D_800A34F8 = (flags & ~0xF) | (index = ((flags & 0xF) + 1) & 0xF);
            if (index >= 8) {
                D_800A34F8 &= ~0xF;
            }
        } while (!(((u32)D_8009BC04.rows_on >> (D_800A34F8 & 0xF)) & 1));
        D_800A3514 = 0;
        D_800A3524->unk20_3 = 0;
        break;
    case 2:
        do {
            s32 flags = D_800A34F8;
            s32 index = flags & 0xF;
            if (index == 0) {
                D_800A34F8 = (flags & ~0xF) | 7;
            } else {
                D_800A34F8 = (flags & ~0xF) | ((index - 1) & 0xF);
            }
        } while (!(((u32)D_8009BC04.rows_on >> (D_800A34F8 & 0xF)) & 1));
        D_800A3514 = 0;
        D_800A3524->unk20_3 = 0;
        break;
    }
    if (((result & 0xFF) < 3) && ((result & 0xFF) != 0) &&
        (D_8009BC04.opt[D_800A34F8 & 0xF].mode == 3)) {
        D_800A3524->unk20_3 ^= 1;
        snd_QueueSound(0, 0x7F, 0x7F);
    }
    if (D_8009BC04.opt[D_800A34F8 & 0xF].mode == 2) {
        D_800A3524->unk20_3 = 1;
    } else if (D_8009BC04.opt[D_800A34F8 & 0xF].mode == 1) {
        D_800A3524->unk20_3 = 0;
    }
    if ((D_800A34F8 & 0xF) != 7) {
        if (((D_800A34F8 & 0xF) - 6) >= 0) {
            var_0[14] = D_8009BC04.unk4 & 0x80;
            D_8009BC04.unk4 = var_0[14] | (0x1F << ((D_800A34F8 & 0xF) - 5));
        } else {
            D_8009BC04.unk4 = 0x9F;
        }
    }
    func_80069AE4(var_0, 0, var_2);
    func_8006A880(
        (u8 *)var_0, (u16 *)var_2, D_8009BC04.opt[D_800A34F8 & 0xF].mode);
    func_80069F80(var_0, D_8009BC04.opt[D_800A34F8 & 0xF].mode);
    func_8006A1A0(var_0, D_8009BC04.opt[D_800A34F8 & 0xF].mode);
    if (arg1 & 0x40) {
        switch (D_8009BC04.opt[D_800A34F8 & 0xF].state) {
        case 4:
        case 5:
            if (g_pad_state.valid[1] == 0) {
                snd_QueueSound(2, 0x7F, 0x7F);
                snd_FlushSoundQueue();
                goto cancel;
            }
        }
        goto accept;
    }
    if (arg1 & 0x400000) {
        switch (D_8009BC04.opt[D_800A34F8 & 0xF].state) {
        case 0:
        case 1:
        case 2:
        case 3:
        case 6:
            goto reject;
        }
        if (g_pad_state.valid[0] == 0) {
        reject:
            snd_QueueSound(2, 0x7F, 0x7F);
            goto cancel;
        }
    accept:
        snd_QueueSound(1, 0x7F, 0x7F);
        if (D_8009BC04.opt[D_800A34F8 & 0xF].state != 7) {
            snd_CloseVab1();
        }
        return D_8009BC04.opt[D_800A34F8 & 0xF].state;
    }
    if (arg1 & 0x100010) {
        snd_QueueSound(2, 0x7F, 0x7F);
        return -2;
    }
cancel:
    if (arg1 & 0x50005000) {
        snd_QueueSound(0, 0x7F, 0x7F);
    }
    return -1;
}

void func_80069898(s32 *arg0, u16 *arg1, s32 arg2) {
    TILE *p = (TILE *)arg0[6];

    SetTile(p);
    p->r0 = 0xFF;
    p->g0 = 0xFF;
    p->b0 = 0xFF;
    p->r0 = 0x80;
    p->g0 = 0x80;
    p->b0 = 0x80;
    p->x0 = arg1[0];
    p->y0 = arg1[1];
    p->w = arg1[2];
    p->h = arg1[3];
    SetSemiTrans(p, 0);
    AddPrim(g_gpu_ot_ptr + arg2, p);
    p++;

    SetTile(p);
    p->r0 = 0x80;
    p->g0 = 0x80;
    p->b0 = 0x80;
    p->x0 = arg1[0];
    p->y0 = arg1[1] - 1;
    p->w = arg1[2];
    p->h = arg1[3];
    SetSemiTrans(p, 1);
    AddPrim(g_gpu_ot_ptr + arg2, p);
    p++;

    SetTile(p);
    p->r0 = 0x40;
    p->g0 = 0x40;
    p->b0 = 0x40;
    p->x0 = arg1[0];
    p->y0 = arg1[1] - 2;
    p->w = arg1[2];
    p->h = arg1[3];
    SetSemiTrans(p, 1);
    AddPrim(g_gpu_ot_ptr + arg2, p);
    p++;

    arg0[6] = (s32)p;
}

void func_80069A30(void *arg0) {
    TILE *a0 = arg0;
    Unk8009BD24Block *p = func_80077D00();
    s32 v0;
    if (p->unk20_0) {
        v0 = 0x22;
        a0->r0 = v0;
        a0->g0 = v0;
    } else {
        v0 = 0x4C;
        a0->r0 = v0;
        a0->g0 = v0;
        v0 = 0x6C;
    }
    a0->b0 = v0;
}

void func_80069A8C(void *arg0) {
    POLY_F4 *a0 = arg0;
    Unk8009BD24Block *p = func_80077D00();
    s32 v0;
    if (p->unk20_0) {
        v0 = 8;
        a0->r0 = v0;
        a0->g0 = v0;
    } else {
        v0 = 0x31;
        a0->r0 = 0;
        a0->g0 = 0;
    }
    a0->b0 = v0;
}

void func_80069AE4(s32 *arg0, s32 mode, GpuDb *arg2) {
    TILE *p;
    POLY_F4 *poly;
    Unk8009B0E0Record **qbase;
    Unk8009B0E0Record **q;
    s32 i;
    Unk8007352CRec s;

    p = (TILE *)arg0[6];

    if (mode == 2) {
        SetTile(p);
        func_80069A30(p);
        p->x0 = 0x4E;
        p->y0 = 0x30;
        p->w = 0xCC;
        p->h = 0xB0;
        SetSemiTrans(p, 1);
        AddPrim(g_gpu_ot_ptr + 0x11, p);
        p++;
        SetTile(p);
        func_80069A30(p);
        p->x0 = 0x166;
        p->y0 = 0x30;
        p->w = 0xCC;
        p->h = 0xB0;
        SetSemiTrans(p, 1);
        AddPrim(g_gpu_ot_ptr + 0x11, p);
        p++;
    } else if (mode == 1) {
        SetTile(p);
        func_80069A30(p);
        p->x0 = 0x3F;
        p->y0 = 0x30;
        p->w = 0x202;
        p->h = 0xB0;
        SetSemiTrans(p, 1);
        AddPrim(g_gpu_ot_ptr + 0x11, p);
        p++;
    } else {
        SetTile(p);
        func_80069A30(p);
        p->x0 = 0x130;
        p->y0 = 0x3A;
        p->w = 0x126;
        p->h = 0xAB;
        SetSemiTrans(p, 1);
        AddPrim(g_gpu_ot_ptr + 0x11, p);
        p++;
    }
    arg0[6] = (s32)p;
    s.ot_idx = 0x12;
    s.has_color = 0;
    s.semi = 0;
    qbase = ((Unk8006919CRec *)arg0[1])->unk_34;
    s.x = 0;
    s.y = 0;
    q = qbase;
    i = 0;
    do {
        s.header = *q;
        s.table = s.header->cells;
        s.sprt_out = arg0[5];
        arg0[5] = gpu_AddSpriteCells(&s);
        q++;
        i++;
    } while (i < 3);

    {
        s.header = qbase[0];
        SetDrawMode(
            (DR_MODE *)arg0[7], 1, 0, gpu_GetSheetTPage(s.header, 0), 0);
    }
    AddPrim(g_gpu_ot_ptr + 0x12, (DR_MODE *)arg0[7]);

    poly = (POLY_F4 *)arg0[3];
    arg0[7] += 0xC;
    SetPolyF4(poly);
    func_80069A8C(poly);
    poly->x0 = 0;
    poly->y0 = 0xB9;
    poly->x1 = 0x122;
    poly->y1 = 0;
    poly->x2 = 0;
    poly->y2 = 0xEF;
    poly->x3 = 0x122;
    poly->y3 = 0xEF;
    SetSemiTrans(poly, 0);
    AddPrim(g_gpu_ot_ptr + 0x13, poly);
    poly++;

    SetPolyF4(poly);
    func_80069A8C(poly);
    poly->x0 = 0x15E;
    poly->x1 = 0x27F;
    poly->y0 = 0;
    poly->y1 = 0;
    poly->x2 = 0x15E;
    poly->y2 = 0xEF;
    poly->x3 = 0x27F;
    poly->y3 = 0x36;
    SetSemiTrans(poly, 0);
    AddPrim(g_gpu_ot_ptr + 0x13, poly);
    poly++;

    SetPolyF4(poly);
    func_80069A8C(poly);
    poly->x0 = 0x122;
    poly->y0 = 0;
    poly->x1 = 0x15E;
    poly->y1 = 0;
    poly->x2 = 0x122;
    poly->y2 = 0xEF;
    poly->x3 = 0x15E;
    poly->y3 = 0xEF;
    SetSemiTrans(poly, 0);
    AddPrim(g_gpu_ot_ptr + 0x13, poly);
    poly++;

    arg0[3] = (s32)poly;
}

void func_80069E18(s32 arg0) {
    TILE *tile;
    Unk8009B0E0Record **ptr;
    Unk8007352CRec s;
    /* FAKE: single-use holder of the cell table ahead of the descriptor's table
     * store; stored directly: score 8. */
    Unk8009B400Record *p1;

    tile = (TILE *)*(s32 *)(arg0 + 0x18);
    SetTile(tile);
    tile->r0 = 0xFF;
    tile->g0 = 0xFF;
    tile->b0 = 0xFF;
    tile->w = 0x280;
    tile->x0 = 0;
    tile->y0 = 0;
    tile->h = 0xF0;
    SetSemiTrans(tile, 0);
    AddPrim(g_gpu_ot_ptr + 0x14, tile);
    *(s32 *)(arg0 + 0x18) = (s32)(tile + 1);

    ptr = (*(Unk8006919CRec **)(arg0 + 4))->unk_14;
    s.ot_idx = 0x10;
    s.semi = 0;
    s.x = 0;
    s.y = 0;
    s.has_color = 0;

    s.header = ptr[0];
    SetDrawMode((DR_MODE *)*(s32 *)(arg0 + 0x1C), 1, 0,
                gpu_GetSheetTPage(s.header, 0), 0);
    AddPrim(g_gpu_ot_ptr + 0x11, (DR_MODE *)*(s32 *)(arg0 + 0x1C));
    *(s32 *)(arg0 + 0x1C) = *(s32 *)(arg0 + 0x1C) + 0xC;

    p1 = s.header->cells;
    s.table = p1;
    s.sprt_out = *(s32 *)(arg0 + 0x14);
    *(s32 *)(arg0 + 0x14) = gpu_AddSpriteCells(&s);

    s.header = ptr[1];
    p1 = s.header->cells;
    s.table = p1;
    s.sprt_out = *(s32 *)(arg0 + 0x14);
    *(s32 *)(arg0 + 0x14) = gpu_AddSpriteCells(&s);

    s.header = ptr[2];
    p1 = s.header->cells;
    s.table = p1;
    s.sprt_out = *(s32 *)(arg0 + 0x14);
    *(s32 *)(arg0 + 0x14) = gpu_AddSpriteCells(&s);
}

/* FAKE: frame layout (oversized live object): the descriptor plus an
 * unwritten tail, so the locals region reaches the target's frame. The plain
 * descriptor scores 12 (func_80069F80) / 14 (func_8006A1A0). */
typedef struct {
    Unk8007352CRec fr00;
    s32 tail[4];
} Unk80069F80Rec;

void func_80069F80(s32 *arg0, s32 arg1) {
    /* FAKE: frame layout - `s` is the live descriptor (its address goes to
     * gpu_AddScaledSpriteQuads / gpu_AddSpriteCells); tail[0..3] is unwritten
     * padding that brings the locals to the target's 0x70 frame (the 0x2C
     * descriptor alone gives 0x60; dead-vars-local-array OVERSIZED-LOCALS). The
     * plain descriptor: score 12. */
    Unk80069F80Rec s;
    Unk8009B0E0Record **ptr;
    s32 x0;
    s32 c;
    /* FAKE: single-use holder of the cell table ahead of the descriptor's table
     * store; stored directly: score 8. */
    Unk8009B400Record *var_4;

    if (arg1 & 2) {
        ptr = ((Unk8006919CRec *)arg0[1])->unk_1C;
        s.fr00.header = ptr[0];
        if (arg1 & 1) {
            s.fr00.x = 0x9C;
            s.fr00.has_color = 1;
        } else {
            s.fr00.x = 0x4E;
            s.fr00.has_color = 0;
        }
        x0 = s.fr00.x;
        if (D_800A3524->unk20_3) {
            if (arg1 & 1) {
                s.fr00.x = x0 + D_800A34FC->unk_0C[0];
                c = ((rsin((D_800A3514 & 0x1F) << 7) * 47) >> 12) - 0x80;
                s.fr00.col_b = c;
                s.fr00.col_g = c;
                s.fr00.col_r = c;
            }
            s.fr00.y = 0;
            s.fr00.scale_y = 0x100;
            s.fr00.scale_x = 0x100;
        } else {
            s.fr00.col_b = 0x70;
            s.fr00.col_g = 0x70;
            s.fr00.col_r = 0x70;
            s.fr00.scale_y = 0x80;
            s.fr00.scale_x = 0x80;
            s.fr00.y = 0xA;
        }
        s.fr00.semi = 0;
        s.fr00.ot_idx = 3;
        var_4 = s.fr00.header->cells;
        s.fr00.table = var_4;
        s.fr00.ft4_out = arg0[2];
        arg0[2] = gpu_AddScaledSpriteQuads(&s.fr00, 0);
        s.fr00.y = 0;
        s.fr00.x = x0;
        s.fr00.has_color = 0;
        s.fr00.header = ptr[1];
        var_4 = s.fr00.header->cells;
        s.fr00.table = var_4;
        s.fr00.sprt_out = arg0[5];
        arg0[5] = gpu_AddSpriteCells(&s.fr00);
        if (arg1 & 1) {
            s.fr00.ot_idx = 2;
            s.fr00.semi = 0;
            s.fr00.header = ptr[6];
            s.fr00.x = 0;
            s.fr00.y = 0;
            s.fr00.has_color = 1;
            s.fr00.table = &s.fr00.header->cells[1];
            s.fr00.sprt_out = arg0[5];
            arg0[5] = gpu_AddSpriteCells(&s.fr00);
        }
        SetDrawMode(
            (DR_MODE *)arg0[7], 1, 0, gpu_GetSheetTPage(s.fr00.header, 0), 0);
        AddPrim(g_gpu_ot_ptr + 3, (DR_MODE *)arg0[7]);
        arg0[7] += 0xC;
    }
}

void func_8006A1A0(s32 *arg0, s32 arg1) {
    /* FAKE: frame layout - `s` is the live descriptor (its address goes to
     * gpu_AddScaledSpriteQuads / gpu_AddSpriteCells); tail[0..3] is unwritten
     * padding that brings the locals to the target's 0x70 frame (the 0x2C
     * descriptor alone gives 0x60; dead-vars-local-array OVERSIZED-LOCALS). The
     * plain descriptor: score 14. */
    Unk80069F80Rec s;
    Unk8009B0E0Record **ptr;
    s32 x0;
    s32 c;
    /* FAKE: single-use holder of the cell table ahead of the descriptor's table
     * store; stored directly: score 8. */
    Unk8009B400Record *var_4;

    if (arg1 & 1) {
        ptr = ((Unk8006919CRec *)arg0[1])->unk_1C;
        s.fr00.header = ptr[2];
        if (arg1 & 2) {
            s.fr00.x = -1;
            s.fr00.has_color = 1;
        } else {
            s.fr00.x = 0x4E;
            s.fr00.has_color = 0;
        }
        x0 = s.fr00.x;
        if (!(D_800A3524->unk20_3)) {
            if (arg1 & 2) {
                s.fr00.x = x0 + D_800A34FC->unk_0C[0];
                c = ((rsin((D_800A3514 & 0x1F) << 7) * 47) >> 12) - 0x80;
                s.fr00.col_b = c;
                s.fr00.col_g = c;
                s.fr00.col_r = c;
            }
            s.fr00.y = 0;
            s.fr00.scale_y = 0x100;
            s.fr00.scale_x = 0x100;
        } else {
            s.fr00.x = x0 + 0x32;
            s.fr00.col_b = 0x70;
            s.fr00.col_g = 0x70;
            s.fr00.col_r = 0x70;
            s.fr00.scale_y = 0x80;
            s.fr00.scale_x = 0x80;
            s.fr00.y = 0xA;
        }
        s.fr00.semi = 0;
        s.fr00.ot_idx = 2;
        var_4 = s.fr00.header->cells;
        s.fr00.table = var_4;
        s.fr00.ft4_out = arg0[2];
        arg0[2] = gpu_AddScaledSpriteQuads(&s.fr00, 0);
        s.fr00.y = 0;
        s.fr00.x = x0;
        s.fr00.has_color = 0;
        s.fr00.header = ptr[3];
        var_4 = s.fr00.header->cells;
        s.fr00.table = var_4;
        s.fr00.sprt_out = arg0[5];
        arg0[5] = gpu_AddSpriteCells(&s.fr00);
        if (arg1 & 2) {
            s.fr00.ot_idx = 2;
            s.fr00.semi = 0;
            s.fr00.header = ptr[6];
            s.fr00.x = 0;
            s.fr00.y = 0;
            s.fr00.has_color = 1;
            var_4 = s.fr00.header->cells;
            s.fr00.table = var_4;
            s.fr00.sprt_out = arg0[5];
            arg0[5] = gpu_AddSpriteCells(&s.fr00);
        }
        SetDrawMode(
            (DR_MODE *)arg0[7], 1, 0, gpu_GetSheetTPage(s.fr00.header, 0), 0);
        AddPrim(g_gpu_ot_ptr + 2, (DR_MODE *)arg0[7]);
        arg0[7] += 0xC;
    }
}

void func_8006A3CC(s32 *arg0, Unk8007352CRec *arg1) {
    arg1->header = ((Unk8006919CRec *)arg0[1])->unk_1C[4];
    arg1->x = 0;
    arg1->y = 0;
    arg1->has_color = 0;
    arg1->semi = 0;
    arg1->ot_idx = 1;
    arg1->table = arg1->header->cells;
    arg1->sprt_out = arg0[5];
    arg0[5] = gpu_AddSpriteCells(arg1);
    SetDrawMode(
        (DR_MODE *)arg0[7], 1, 0, gpu_GetSheetTPage(arg1->header, 0), 0);
    AddPrim(g_gpu_ot_ptr + 1, (DR_MODE *)arg0[7]);
    arg0[7] += 0xC;
}

void func_8006A494(s32 *arg0, Unk8007352CRec *arg1) {
    arg1->header = ((Unk8006919CRec *)arg0[1])->unk_1C[9];
    arg1->x = 0;
    arg1->y = 0;
    arg1->has_color = 0;
    arg1->semi = 0;
    arg1->ot_idx = 1;
    arg1->scale_x = 0x100;
    arg1->scale_y = 0x100;
    arg1->table = arg1->header->cells;
    arg1->ft4_out = arg0[2];
    arg0[2] = gpu_AddScaledSpriteQuads(arg1, 0);
    SetDrawMode(
        (DR_MODE *)arg0[7], 1, 0, gpu_GetSheetTPage(arg1->header, 0), 0);
    AddPrim(g_gpu_ot_ptr + 1, (DR_MODE *)arg0[7]);
    arg0[7] += 0xC;
}

void func_8006A564(u8 *arg0, Unk8007352CRec *arg1, s32 arg2) {
    TILE *tile;
    Unk8009B0E0Record **tbl;
    s32 s4;

    tile = (TILE *)*(s32 *)(arg0 + 0x18);
    SetTile(tile);
    if ((D_800A34F8 & 0xF) == arg2) {
        setRGB0(tile, arg1->col_r, 0, arg1->col_b);
        s4 = 0;
    } else {
        setRGB0(tile, 0x50, 0x50, 0x50);
        s4 = 0x20;
    }
    tile->x0 = 0x5F;
    tile->y0 = arg1->y + 0xF;
    tile->w = arg1->x + 0x19;
    tile->h = 1;
    SetSemiTrans(tile, arg1->semi);
    AddPrim(g_gpu_ot_ptr + arg1->ot_idx, tile);
    tile++;

    SetTile(tile);
    if ((D_800A34F8 & 0xF) == arg2) {
        setRGB0(tile, arg1->col_r, 0, arg1->col_b);
    } else {
        setRGB0(tile, 0x20, 0x20, 0x20);
    }
    tile->x0 = arg1->x;
    tile->y0 = arg1->y + 0xE;
    tile->w = 0x78;
    tile->h = 1;
    SetSemiTrans(tile, 1);
    AddPrim(g_gpu_ot_ptr + arg1->ot_idx, tile);
    tile++;

    SetTile(tile);
    if ((D_800A34F8 & 0xF) == arg2) {
        setRGB0(tile, arg1->col_r >> 1, 0, arg1->col_b >> 1);
    } else {
        setRGB0(tile, 0x10, 0x10, 0x10);
    }
    tile->x0 = arg1->x + 0x40;
    tile->y0 = arg1->y + 0xD;
    tile->w = 0x38;
    tile->h = 1;
    SetSemiTrans(tile, 1);
    AddPrim(g_gpu_ot_ptr + arg1->ot_idx, tile);

    tile = tile + 1;
    *(s32 *)(arg0 + 0x18) = (s32)tile;
    tbl = (*(Unk8006919CRec **)(arg0 + 4))->unk_1C;
    {
        /* FAKE: single-use holder of the cell table ahead of the descriptor's
         * table store; stored directly: score 7. */
        Unk8009B400Record *v1;
        /* FAKE: the header store through a pointer to the member: stored as
           arg1->header the member store is scheduled past the D_800A34F8 load
           (MEM_IN_STRUCT_P); score 7. */
        Unk8009B0E0Record **var_7 = &arg1->header;

        *var_7 = tbl[10];
        if ((D_800A34F8 & 0xF) == arg2) {
            arg1->col_g = 0;
            arg1->col_r = arg1->col_r >> 1;
            arg1->col_b = arg1->col_b >> 1;
        } else {
            arg1->col_b = 0x28;
            arg1->col_g = 0x28;
            arg1->col_r = 0x28;
        }

        arg1->x = 0;
        v1 = arg1->header->cells;
        arg1->y += 0xF;
        arg1->table = v1;

        arg1->sprt_out = *(s32 *)(arg0 + 0x14);
        *(s32 *)(arg0 + 0x14) = gpu_AddSpriteCells(arg1);

        arg1->header = tbl[11];
        v1 = arg1->header->cells;
        arg1->table = v1;
        arg1->sprt_out = *(s32 *)(arg0 + 0x14);
        *(s32 *)(arg0 + 0x14) = gpu_AddSpriteCells(arg1);
    }

    SetDrawMode((DR_MODE *)*(s32 *)(arg0 + 0x1C), 1, 0,
                gpu_GetSheetTPage(arg1->header, s4), 0);
    AddPrim(g_gpu_ot_ptr + arg1->ot_idx, (DR_MODE *)*(s32 *)(arg0 + 0x1C));
    *(s32 *)(arg0 + 0x1C) = *(s32 *)(arg0 + 0x1C) + 0xC;
}

/* Draws the eight-row list screen from the MOD.BIN resource (ctx+4): the row
 * sheets (cursor row flashing, rows switched on in D_8009BC04.rows_on lit), a
 * counter frame, row 7, the frame icons and a TILE. arg0 = the words
 * func_8006E390 fills (+0x8 POLY_FT4 cursor, +0x14 SPRT cursor,
 * +0x18 TILE cursor, +0x1C DR_MODE cursor, +0x20 DR_AREA cursor,
 * +0x24 DR_OFFSET cursor), accessed as byte offsets like func_8006A564;
 * arg1 = the display environment's clip RECT and ofs[2]; arg2 (passed by
 * menu_UpdateRowMenu) is unused. */
void func_8006A880(u8 *arg0, u16 *arg1, s32 arg2) {
    Unk8007352CRec s;
    RECT rect;
    s16 ofs[2];
    /* Ruling 11 (ordinary-c-judge-decidable): holds five values, each a
     * sprite-sheet list of the MOD.BIN
     * root: unk_18 (option rows; first sheet, the row loop and row 7), unk_40
     * (counter frames), unk_24 twice (icon frames at [8 + frame], then FT4
     * frames at [frame]). */
    Unk8009B0E0Record **var_3;
    /* Ruling 11: holds two values, each a mask with one bit per option row:
       D_8009BC04.unk4 (scanned for the first row drawn) and .rows_on (rows
       switched on). */
    u32 row_mask;
    /* Ruling 9: the sheet's cell array; every sheet drawn here has one 12-byte
     * header, so the cells start at +0xC. FAKE: single-use holder of the cell
     * table ahead of the descriptor's table store; stored directly: score 15.
     */
    Unk8009B400Record *var_5;
    s32 yofs;
    s32 bit;
    s32 i;

    var_3 = (*(Unk8006919CRec **)(arg0 + 4))->unk_18;
    row_mask = D_8009BC04.unk4;
    bit = 0;
    s.header = var_3[0];
    SetDrawMode((DR_MODE *)*(s32 *)(arg0 + 0x1C), 1, 0,
                gpu_GetSheetTPage(s.header, 0), 0);
    AddPrim(g_gpu_ot_ptr + 0xC, (DR_MODE *)*(s32 *)(arg0 + 0x1C));
    *(s32 *)(arg0 + 0x1C) += 0xC;
    rect.x = arg1[0];
    rect.y = arg1[1] + 0x3A;
    rect.w = 0xF6;
    rect.h = 0x92;
    SetDrawArea((DR_AREA *)*(s32 *)(arg0 + 0x20), &rect);
    AddPrim(g_gpu_ot_ptr + 0xC, (DR_AREA *)*(s32 *)(arg0 + 0x20));
    *(s32 *)(arg0 + 0x20) += 0xC;
    yofs = 0;
    for (; !(row_mask & (1 << bit)); bit++) {
        yofs -= 0x18;
    }
    ofs[0] = arg1[4];
    ofs[1] = arg1[5] + yofs;
    SetDrawOffset((DR_OFFSET *)*(s32 *)(arg0 + 0x24), ofs);
    AddPrim(g_gpu_ot_ptr + 0xC, (DR_OFFSET *)*(s32 *)(arg0 + 0x24));
    *(s32 *)(arg0 + 0x24) += 0xC;

    row_mask = D_8009BC04.rows_on;
    s.ot_idx = 0xB;
    for (i = 0; i < 7; i++) {
        s32 y = i * 0x18 + 0x3F;

        s.x = 0x76;
        s.has_color = 1;
        if ((D_800A34F8 & 0xF) == i) {
            s.semi = 0;
            s.col_r =
                ((rsin(((D_800A3514 & 0x1F) << 7) + 0x1FF) * 63) >> 12) - 0x61;
            s.col_g =
                ((rsin(((D_800A3514 & 0x1F) << 7) + 0x1FF) * 45) >> 12) - 0x7D;
            s.col_b = 0x30;
            s.y = D_800A34FC->unk_0C[1] + y;
            D_800A3514++;
        } else if ((row_mask >> i) & 1) {
            s.col_r = 0x80;
            s.col_g = 0x6C;
            s.col_b = 0x30;
            s.semi = 1;
            s.y = y;
            if (D_800A3524->unk20_0) {
                s.has_color = 0;
            }
        } else {
            s.col_r = 0x30;
            s.col_g = 0x30;
            s.col_b = 0x30;
            s.semi = 1;
            s.y = y;
        }
        s.header = var_3[i];
        var_5 = s.header->cells;
        s.table = var_5;
        s.sprt_out = *(s32 *)(arg0 + 0x14);
        *(s32 *)(arg0 + 0x14) = gpu_AddSpriteCells(&s);
        SetDrawMode((DR_MODE *)*(s32 *)(arg0 + 0x1C), 1, 0,
                    gpu_GetSheetTPage(s.header, 0), 0);
        AddPrim(g_gpu_ot_ptr + s.ot_idx, (DR_MODE *)*(s32 *)(arg0 + 0x1C));
        *(s32 *)(arg0 + 0x1C) += 0xC;
        s.has_color = 1;
        func_8006A564(arg0, &s, i);
    }

    var_3 = (*(Unk8006919CRec **)(arg0 + 4))->unk_40;
    s.ot_idx = 0xA;
    s.has_color = 0;
    s.semi = 0;
    s.x = 0;
    s.y = 0x41;
    s.header = var_3[D_800A34F8 & 0xF];
    var_5 = s.header->cells;
    s.table = var_5;
    s.sprt_out = *(s32 *)(arg0 + 0x14);
    *(s32 *)(arg0 + 0x14) = gpu_AddSpriteCells(&s);
    SetDrawMode((DR_MODE *)*(s32 *)(arg0 + 0x1C), 1, 0,
                gpu_GetSheetTPage(s.header, 0), 0);
    AddPrim(g_gpu_ot_ptr + s.ot_idx, (DR_MODE *)*(s32 *)(arg0 + 0x1C));
    *(s32 *)(arg0 + 0x1C) += 0xC;

    var_3 = (*(Unk8006919CRec **)(arg0 + 4))->unk_18;
    rect.x = arg1[0];
    rect.y = arg1[1];
    rect.w = arg1[2];
    rect.h = arg1[3];
    SetDrawArea((DR_AREA *)*(s32 *)(arg0 + 0x20), &rect);
    AddPrim(g_gpu_ot_ptr + 0xA, (DR_AREA *)*(s32 *)(arg0 + 0x20));
    *(s32 *)(arg0 + 0x20) += 0xC;
    ofs[0] = arg1[4];
    ofs[1] = arg1[5];
    SetDrawOffset((DR_OFFSET *)*(s32 *)(arg0 + 0x24), ofs);
    AddPrim(g_gpu_ot_ptr + 0xA, (DR_OFFSET *)*(s32 *)(arg0 + 0x24));
    *(s32 *)(arg0 + 0x24) += 0xC;

    s.x = 0x76;
    s.has_color = 1;
    if ((D_800A34F8 & 0xF) == 7) {
        s.semi = 0;
        s.col_r =
            ((rsin(((D_800A3514 & 0x1F) << 7) + 0x1FF) * 63) >> 12) - 0x61;
        s.col_g =
            ((rsin(((D_800A3514 & 0x1F) << 7) + 0x1FF) * 45) >> 12) - 0x7D;
        s.col_b = 0x30;
        s.y = D_800A34FC->unk_0C[1] + 0xCF;
        D_800A3514++;
    } else if ((row_mask >> 7) & 1) {
        s.col_r = 0x80;
        s.col_g = 0x6C;
        s.col_b = 0x30;
        s.semi = 1;
        s.y = 0xCF;
        if (D_800A3524->unk20_0) {
            s.has_color = 0;
        }
    } else {
        s.col_r = 0x30;
        s.col_g = 0x30;
        s.col_b = 0x30;
        s.semi = 1;
        s.y = 0xCF;
    }
    s.ot_idx = 9;
    s.header = var_3[7];
    var_5 = s.header->cells;
    s.table = var_5;
    s.sprt_out = *(s32 *)(arg0 + 0x14);
    *(s32 *)(arg0 + 0x14) = gpu_AddSpriteCells(&s);
    SetDrawMode((DR_MODE *)*(s32 *)(arg0 + 0x1C), 1, 0,
                gpu_GetSheetTPage(s.header, 0), 0);
    AddPrim(g_gpu_ot_ptr + s.ot_idx, (DR_MODE *)*(s32 *)(arg0 + 0x1C));
    *(s32 *)(arg0 + 0x1C) += 0xC;
    s.has_color = 1;
    func_8006A564(arg0, &s, 7);
    func_8006A494((s32 *)arg0, &s);

    s.has_color = 0;
    SetDrawMode((DR_MODE *)*(s32 *)(arg0 + 0x1C), 1, 0,
                gpu_GetSheetTPage(s.header, 0), 0);
    AddPrim(g_gpu_ot_ptr + 1, (DR_MODE *)*(s32 *)(arg0 + 0x1C));
    *(s32 *)(arg0 + 0x1C) += 0xC;
    SetDrawMode((DR_MODE *)*(s32 *)(arg0 + 0x1C), 1, 0,
                gpu_GetSheetTPage(s.header, 0), 0);
    AddPrim(g_gpu_ot_ptr + 1, (DR_MODE *)*(s32 *)(arg0 + 0x1C));
    *(s32 *)(arg0 + 0x1C) += 0xC;

    var_3 = (*(Unk8006919CRec **)(arg0 + 4))->unk_24;
    s.header = var_3[(D_800A34F8 & 0xF) + 8];
    s.x = 0;
    s.y = 0x19;
    s.has_color = 0;
    s.semi = 0;
    s.ot_idx = 0;
    var_5 = s.header->cells;
    s.table = var_5;
    s.sprt_out = *(s32 *)(arg0 + 0x14);
    *(s32 *)(arg0 + 0x14) = gpu_AddSpriteCells(&s);
    SetDrawMode((DR_MODE *)*(s32 *)(arg0 + 0x1C), 1, 0,
                gpu_GetSheetTPage(s.header, 0), 0);
    AddPrim(g_gpu_ot_ptr, (DR_MODE *)*(s32 *)(arg0 + 0x1C));
    *(s32 *)(arg0 + 0x1C) += 0xC;

    var_3 = (*(Unk8006919CRec **)(arg0 + 4))->unk_24;
    s.header = var_3[D_800A34F8 & 0xF];
    s.scale_x = 0x200;
    s.x = 0;
    s.y = 0;
    s.scale_y = 0x100;
    s.has_color = 0;
    s.semi = 0;
    s.ot_idx = 0;
    var_5 = s.header->cells;
    s.table = var_5;
    s.ft4_out = *(s32 *)(arg0 + 8);
    *(s32 *)(arg0 + 8) = gpu_AddScaledSpriteQuads(&s, 0);

    SetTile((TILE *)*(s32 *)(arg0 + 0x18));
    ((TILE *)*(s32 *)(arg0 + 0x18))->r0 = 0;
    ((TILE *)*(s32 *)(arg0 + 0x18))->g0 = 0;
    ((TILE *)*(s32 *)(arg0 + 0x18))->b0 = 0;
    ((TILE *)*(s32 *)(arg0 + 0x18))->x0 = 0x142;
    ((TILE *)*(s32 *)(arg0 + 0x18))->y0 = 0x51;
    ((TILE *)*(s32 *)(arg0 + 0x18))->w = 0x100;
    ((TILE *)*(s32 *)(arg0 + 0x18))->h = 0x59;
    SetSemiTrans((TILE *)*(s32 *)(arg0 + 0x18), 0);
    AddPrim(g_gpu_ot_ptr + 1, (TILE *)*(s32 *)(arg0 + 0x18));
    *(s32 *)(arg0 + 0x18) += 0x10;

    SetDrawMode((DR_MODE *)*(s32 *)(arg0 + 0x1C), 1, 0,
                gpu_GetSheetTPage(s.header, 0), 0);
    AddPrim(g_gpu_ot_ptr, (DR_MODE *)*(s32 *)(arg0 + 0x1C));
    *(s32 *)(arg0 + 0x1C) += 0xC;
}

/* One argument: the caller's s32[10] (asm reads a0 only). */
void func_8006B120(s32 *arg0);

void func_8006B120(s32 *arg0) {
    Unk8007352CRec s;
    u16 r[4];
    Unk8009B0E0Record **tbl;
    s32 i;
    /* FAKE: single-use holder of the cell table ahead of the descriptor's table
     * store; stored directly: score 11. */
    Unk8009B400Record *p1;

    s.has_color = 0;
    s.ot_idx = 10;
    tbl = ((Unk8006919CRec *)arg0[1])->unk_28;
    s.header = tbl[0];
    s.y = 0;
    s.x = 0;
    s.semi = 0;
    p1 = s.header->cells;
    s.table = p1;
    s.sprt_out = arg0[5];
    arg0[5] = gpu_AddSpriteCells(&s);
    SetDrawMode((DR_MODE *)arg0[7], 1, 0, gpu_GetSheetTPage(s.header, 0), 0);
    AddPrim(g_gpu_ot_ptr + 0xA, (DR_MODE *)arg0[7]);
    arg0[7] += 0xC;
    s.x = 0;
    i = 0;

    do {
        s.header = tbl[i + 1];
        if ((((u32)D_800A34F8 >> 10) & 7) == i) {
            s.has_color = 1;
            s.y = D_800A34FC->unk_0C[1];
            s.semi = 0;
            s.col_r = s.col_g = s.col_b =
                ((rsin(((D_800A3514 & 0x1F) << 7) + 0x1FF) * 47) >> 12) - 0x80;
        } else {
            s.has_color = 0;
            s.y = 0;
            s.semi = 1;
        }
        p1 = s.header->cells;
        s.table = p1;
        s.sprt_out = arg0[5];
        arg0[5] = gpu_AddSpriteCells(&s);
        i++;
    } while (i < 6);

    s.y = 0;
    i = 0;
    do {
        s.header = tbl[i + 7];
        s.x = 0;
        s.has_color = 0;
        if (D_800A3524->unk20_0 == i) {
            s.semi = 0;
            if (!(D_800A34F8 & 0x1C00)) {
                s32 c;

                s.has_color = 1;
                s.x = D_800A34FC->unk_0C[0];
                c = ((rsin(((D_800A3514 & 0x1F) << 7) + 0x1FF) * 32) >> 12) -
                    0x80;
                s.col_b = c;
                s.col_g = c;
                s.col_r = c;
            }
        } else {
            s.semi = 1;
        }
        p1 = s.header->cells;
        s.table = p1;
        s.sprt_out = arg0[5];
        arg0[5] = gpu_AddSpriteCells(&s);
        i++;
    } while (i < 2);

    i = 0;
    do {
        s.header = tbl[i + 9];
        s.x = 0;
        s.has_color = 0;
        if (D_800A3524->unk20_1 == i) {
            s.semi = 0;
            if ((D_800A34F8 & 0x1C00) == 0x400) {
                s32 c;

                s.has_color = 1;
                s.x = D_800A34FC->unk_0C[0];
                c = ((rsin(((D_800A3514 & 0x1F) << 7) + 0x1FF) * 32) >> 12) -
                    0x80;
                s.col_b = c;
                s.col_g = c;
                s.col_r = c;
            }
        } else {
            s.semi = 1;
        }
        p1 = s.header->cells;
        s.table = p1;
        s.sprt_out = arg0[5];
        arg0[5] = gpu_AddSpriteCells(&s);
        i++;
    } while (i < 2);

    i = 0;
    do {
        s.header = tbl[i + 11];
        s.x = 0;
        s.has_color = 0;
        if (D_800A3524->unk20_2 == i) {
            s.semi = 0;
            if ((D_800A34F8 & 0x1C00) == 0x800) {
                s32 c;

                s.has_color = 1;
                s.x = D_800A34FC->unk_0C[0];
                c = ((rsin(((D_800A3514 & 0x1F) << 7) + 0x1FF) * 32) >> 12) -
                    0x80;
                s.col_b = c;
                s.col_g = c;
                s.col_r = c;
            }
        } else {
            s.semi = 1;
        }
        p1 = s.header->cells;
        s.table = p1;
        s.sprt_out = arg0[5];
        arg0[5] = gpu_AddSpriteCells(&s);
        i++;
    } while (i < 2);

    tbl = ((Unk8006919CRec *)arg0[1])->unk_28;
    s.header = tbl[1];
    SetDrawMode((DR_MODE *)arg0[7], 1, 0, gpu_GetSheetTPage(s.header, 0), 0);
    AddPrim(g_gpu_ot_ptr + 0xA, (DR_MODE *)arg0[7]);
    arg0[7] += 0xC;
    r[2] = 0xAF;
    r[0] = 0xE8;
    r[1] = 0x25;
    r[3] = 1;
    func_80069898(arg0, r, 0x11);
}

/* func_8006B578 — menu/config input dispatch. Its second switch's 6-entry
 * jump table lands at 0x80015988 in this TU's .rodata. */
s32 func_8006B578(s32 *arg0, u32 *arg1) {
    u32 v;
    s32 sp10;
    s32 ret;
    s32 var_s2 = 0;

    v = *arg1;
    sp10 = (v & 0xFFFF) | (v >> 16);
    ret = func_800692C0((u32 *)&sp10, 0, D_800A34FC->unk_0C, D_800A350C);
    switch (ret >> 16) {
    case 1:
        if ((D_800A34F8 & 0x1C00) == 0x1400) {
            D_800A34F8 &= ~0x1C00;
        } else {
            D_800A34F8 = (D_800A34F8 & ~0x1C00) |
                         (((((u32)D_800A34F8 >> 10) & 7) + 1) & 7) << 10;
        }
        snd_QueueSound(0, 0x7F, 0x7F);
        D_800A3514 = 0;
        break;
    case 2:
        if ((D_800A34F8 & 0x1C00) == 0) {
            D_800A34F8 = (D_800A34F8 & ~0x1C00) | 0x1400;
        } else {
            D_800A34F8 = (D_800A34F8 & ~0x1C00) |
                         (((((u32)D_800A34F8 >> 10) & 7) - 1) & 7) << 10;
        }
        snd_QueueSound(0, 0x7F, 0x7F);
        D_800A3514 = 0;
        break;
    }

    switch ((u32)D_800A34F8 >> 10 & 7) {
    case 0:
        if ((ret & 0xFF) != 0) {
            D_800A3524->unk20_0 ^= 1;
            snd_QueueSound(0, 0x7F, 0x7F);
        }
        if (*arg1 & 0x400040) {
            snd_QueueSound(1, 0x7F, 0x7F);
            D_800A34F8 = (D_800A34F8 & ~0x1C00) |
                         (((((u32)D_800A34F8 >> 10) & 7) + 1) & 7) << 10;
        }
        break;
    case 1:
        if ((ret & 0xFF) != 0) {
            D_800A3524->unk20_1 ^= 1;
            snd_QueueSound(0, 0x7F, 0x7F);
        }
        if (*arg1 & 0x400040) {
            snd_QueueSound(1, 0x7F, 0x7F);
            D_800A34F8 = (D_800A34F8 & ~0x1C00) |
                         (((((u32)D_800A34F8 >> 10) & 7) + 1) & 7) << 10;
        }
        break;
    case 2:
        if ((ret & 0xFF) != 0) {
            D_800A3524->unk20_2 ^= 1;
            snd_QueueSound(0, 0x7F, 0x7F);
        }
        if (*arg1 & 0x400040) {
            snd_QueueSound(1, 0x7F, 0x7F);
            D_800A34F8 = (D_800A34F8 & ~0x1C00) |
                         (((((u32)D_800A34F8 >> 10) & 7) + 1) & 7) << 10;
        }
        break;
    case 3:
        if (*arg1 & 0x400040) {
            snd_QueueSound(1, 0x7F, 0x7F);
            D_800A34F8 = (D_800A34F8 & 0xFFFF1FFF) | 0x4000;
            var_s2 = 2;
        }
        break;
    case 4:
        if (*arg1 & 0x400040) {
            snd_QueueSound(1, 0x7F, 0x7F);
            var_s2 = 3;
        }
        break;
    case 5:
        if (*arg1 & 0x400040) {
            snd_QueueSound(1, 0x7F, 0x7F);
            var_s2 = 1;
        }
        break;
    }

    if (*arg1 & 0x100010) {
        snd_QueueSound(2, 0x7F, 0x7F);
        var_s2 = 1;
    }
    return var_s2;
}

/* Keeps the original .rodata run contiguous around the compiler-generated
 * switch tables. */
const u8 D_800159A0[16] = "warning\n";

s32 func_8006B898(s32 arg0, u32 arg1) {
    s32 sp10[10];
    GpuDb *t;
    D_800A3514 += 1;
    t = &g_gpu_db[D_800A36AC & 1];
    func_8006E390(sp10, &D_800A3518);
    func_80069AE4(sp10, 1, t);
    func_8006B120(sp10);
    return func_8006B578(&arg0, &arg1);
}

s32 func_8006B92C(s32 *arg0, u32 *arg1) {
    u32 v;
    s32 sp10;
    s32 ret;
    s32 idx;
    s32 var_s0 = 0;

    v = *arg1;
    sp10 = (v & 0xFFFF) | (v >> 16);
    ret = func_800692C0(&sp10, 0, D_800A34FC->unk_0C, D_800A350C);
    switch (ret >> 16) {
    case 1:
        if ((D_800A34F8 & 0xE000) == 0x4000) {
            D_800A34F8 &= 0xFFFF1FFF;
        } else {
            D_800A34F8 = (D_800A34F8 & 0xFFFF1FFF) |
                         (((((u32)D_800A34F8 >> 13) & 7) + 1) & 7) << 13;
        }
        snd_QueueSound(0, 0x7F, 0x7F);
        break;
    case 2:
        if ((D_800A34F8 & 0xE000) == 0) {
            D_800A34F8 = (D_800A34F8 & 0xFFFF1FFF) | 0x4000;
        } else {
            D_800A34F8 = (D_800A34F8 & 0xFFFF1FFF) |
                         (((((u32)D_800A34F8 >> 13) & 7) - 1) & 7) << 13;
        }
        snd_QueueSound(0, 0x7F, 0x7F);
        break;
    }

    idx = ((u32)D_800A34F8 >> 13) & 7;
    switch (idx) {
    case 0:
        if (*arg1 & 0x400040) {
            snd_QueueSound(1, 0x7F, 0x7F);
            var_s0 = 2;
        }
        break;
    case 1:
        if (*arg1 & 0x400040) {
            snd_QueueSound(1, 0x7F, 0x7F);
            var_s0 = 3;
        }
        break;
    case 2:
        if (*arg1 & 0x400040) {
            snd_QueueSound(1, 0x7F, 0x7F);
            var_s0 = 1;
            D_800A34F8 = (D_800A34F8 & ~0x1C00) |
                         ((((((u32)D_800A34F8 >> 10) & 7) + 1) & 7) << 10);
        }
        break;
    }

    if (*arg1 & 0x100010) {
        snd_QueueSound(2, 0x7F, 0x7F);
        var_s0 = 1;
    }
    return var_s0;
}

void func_8006BB68(s32 *arg0) {
    Unk8007352CRec s;
    u16 var_1[4];
    s32 i;
    Unk8009B0E0Record **q;
    /* FAKE: single-use holder of the cell table ahead of the descriptor's table
     * store; stored directly: score 11. */
    Unk8009B400Record *p1;

    s.y = 0;
    s.x = 0;
    s.ot_idx = 0xA;
    s.has_color = 0;
    q = ((Unk8006919CRec *)arg0[1])->unk_2C;
    {
        s.header = q[0];
        s.semi = 0;
        p1 = s.header->cells;
        s.table = p1;
    }
    s.sprt_out = arg0[5];
    arg0[5] = gpu_AddSpriteCells(&s);
    SetDrawMode((DR_MODE *)arg0[7], 1, 0, gpu_GetSheetTPage(s.header, 0), 0);
    AddPrim(g_gpu_ot_ptr + 0xA, (DR_MODE *)arg0[7]);
    arg0[7] += 0xC;

    for (i = 0; i < 3; i++) {
        s.header = q[1];
        p1 = s.header->cells;
        s.table = p1;
        if ((((u32)D_800A34F8 >> 13) & 7) == i) {
            s.y = D_800A34FC->unk_0C[1];
            s.semi = 0;
        } else {
            s.y = 0;
            s.semi = 1;
        }
        s.sprt_out = arg0[5];
        arg0[5] = gpu_AddSpriteCells(&s);
        q++;
    }

    q = ((Unk8006919CRec *)arg0[1])->unk_28;
    s.header = q[1];
    SetDrawMode((DR_MODE *)arg0[7], 1, 0, gpu_GetSheetTPage(s.header, 0), 0);
    AddPrim(g_gpu_ot_ptr + 0xA, (DR_MODE *)arg0[7]);
    arg0[7] += 0xC;

    var_1[2] = 0xAF;
    var_1[0] = 0xE8;
    var_1[1] = 0x25;
    var_1[3] = 1;
    func_80069898(arg0, var_1, 0x11);
}

extern DR_MODE *D_800A36E0;
extern u8 *D_800A36E4;

void func_8006BD28(s32 arg0, s32 arg1, Unk8007352CRec *arg2, s32 arg3) {
    /* sprite-sheet header pointers, two per arg0: the unk_20 list of the
       MOD.BIN root (D_800A34FC->unk_24) */
    Unk8009B0E0Record **var_0;
    /* the sheet's 8-byte cells, just past its 12-byte header */
    Unk8009B400Record *cells;
    s32 i, j, n;

    var_0 = D_800A34FC->unk_24->unk_20;

    arg2->col_b = 0x30;
    arg2->col_g = 0x30;
    arg2->col_r = 0x30;

    for (i = 0; i < 2; i++) {
        /* FAKE: operand grouping (or-tree-shape-shift carve-out): with
         * (var_0 + i) + arg0 * 2, arg0 * 8 is hoisted alone as in the target;
         * var_0[arg0 * 2 + i]: score 33. */
        arg2->header = *(var_0 + i + arg0 * 2);
        if (arg2->header == (Unk8009B0E0Record *)-1)
            return;

        n = (arg0 == 0x12) ? 3 : 1;

        for (j = 0; j < n; j++) {
            arg2->x = 0;
            arg2->y = arg1;
            arg2->ot_idx = 8;
            if (arg0 != 0x12 || j == arg3 || j == 2) {
                arg2->has_color = 0;
            } else {
                arg2->has_color = 1;
            }
            arg2->semi = 0;
            arg2->sprt_out = (s32)D_800A36E4;
            cells = arg2->header->cells;
            arg2->table = cells + j;
            D_800A36E4 = (u8 *)gpu_AddSpriteCells(arg2);
        }

        SetDrawMode(D_800A36E0, 1, 0, gpu_GetSheetTPage(arg2->header, 0), 0);
        AddPrim(g_gpu_ot_ptr + 8, D_800A36E0);
        D_800A36E0++;
    }
}

extern s32 D_800A3900;
extern TILE *D_800A36DC;
extern u8 D_800F11E0[];
extern DR_MODE D_800F1438[];
extern TILE D_800F1498[];

void func_8006BEC4(s32 arg0, s32 arg1) {
    Unk8007352CRec sp10;
    s32 var_1;
    Size2s16 *var_2;
    s16 i;
    s32 r;
    s32 w;
    s32 h;
    s32 x0;

    D_800A3900 = 0;
    var_1 = D_800A36AC & 1;
    D_800A36E4 = D_800F11E0 + var_1 * 0x12C;
    D_800A36E0 = D_800F1438 + var_1 * 4;
    D_800A36DC = D_800F1498 + var_1 * 4;
    func_8006BD28(arg0, 0, &sp10, 0);
    h = 0;
    var_2 = D_800A34FC->unk_24->unk_48;
    var_2 += arg0;
    if (arg1 != -1) {
        w = arg1 ? 0x1F : 0x28;
        h = 0x10;
        D_800A3900 = (var_2->y + 0x10) >> 1;
        func_8006BD28(0x12, D_800A3900, &sp10, arg1);
        x0 = (arg1 & 1) * 0x3A + 0x113;
        for (i = 0; i < 3; i++) {
            SetTile(D_800A36DC);
            if (i == 0) {
                r = 0xFF;
                SetSemiTrans(D_800A36DC, 0);
            } else {
                r = 0xFF - ((i - 1) << 7);
                SetSemiTrans(D_800A36DC, 1);
            }
            D_800A36DC->r0 = r;
            D_800A36DC->g0 = 0;
            D_800A36DC->b0 = 0;
            D_800A36DC->y0 = D_800A3900 + 0x7C - i;
            D_800A36DC->x0 = x0;
            D_800A36DC->w = w;
            D_800A36DC->h = 1;
            AddPrim(g_gpu_ot_ptr + 8, D_800A36DC);
            D_800A36DC++;
        }
    }
    SetTile(D_800A36DC);
    D_800A36DC->r0 = 0;
    D_800A36DC->g0 = 0;
    D_800A36DC->b0 = 0;
    D_800A36DC->x0 = 0x140 - (var_2->x >> 1);
    D_800A36DC->y0 = 0x78 - (var_2->y >> 1);
    D_800A36DC->w = var_2->x;
    D_800A36DC->h = var_2->y + h;
    SetSemiTrans(D_800A36DC, 1);
    AddPrim(g_gpu_ot_ptr + 8, D_800A36DC);
    D_800A36DC++;
}

extern s32 func_8006B92C();

s32 func_8006C168(s32 arg0, s32 arg1) {
    s32 sp10[22];
    GpuDb *t;
    D_800A3514 += 1;
    t = &g_gpu_db[D_800A36AC & 1];
    func_8006E390(sp10, &D_800A3518);
    func_80069AE4(sp10, 1, t);
    func_8006BB68(sp10);
    return func_8006B92C(&arg0, &arg1);
}

s32 func_8006C1FC(s32 a0, s32 a1) { return func_8006C168(a0, a1); }

#define setXYWH(p, _x0, _y0, _w, _h)                                           \
    (p)->x0 = (_x0), (p)->y0 = (_y0), (p)->x1 = (_x0) + (_w), (p)->y1 = (_y0), \
    (p)->x2 = (_x0), (p)->y2 = (_y0) + (_h), (p)->x3 = (_x0) + (_w),           \
    (p)->y3 = (_y0) + (_h)

void func_8006C21C(s32 *arg0) {
    Unk8007352CRec s;
    s32 j;
    Unk8009B0E0Record **table;
    Rec_8006C21C *recs;
    Rec_8006C21C *rec;
    Rec_8006C21C *var_5;
    TILE *tile;
    POLY_G4 *poly;
    /* FAKE: constant-holder (named-local-fake-exception) for the 0 passed as
     * gpu_GetSheetTPage's second argument; held in $s5 as in the target. A
     * literal 0 at every call: score 3. */
    s32 var_8;
    /* `var_9` counts the phase-2 option-bit, phase-4 sprite and phase-6
     * rectangle loops, then holds each player's menu row cursor
     * (unk_28.half[j], tested against 5 and used as the recs subscript),
     * the way SOTN's AluInit reuses its loop counter i for the weapon id
     * (Q51); one local per role: score 90 */
    /* SOTN: src/dra/692E8.c:299 @db41b28 */
    s32 var_9;
    /* FAKE: per-branch constant holder (named-local-fake-exception, Q27 (B)) --
     * the bar's far-corner red, 0 in two arms and 0x80 in the other two; set in
     * each arm, it is not hoisted out of the row loop. The literals: score 8.
     */
    s32 var_10;
    /* FAKE: always-zero narrow locals (named-local-fake-exception, Q27 (A)) --
     * SetDrawMode's dither and texture-window arguments and the descriptor
     * position, read up to the phase-4 head; their dead extension temps take
     * the target's four untouched frame slots (0x60-0x78). `var_14` reaches
     * SetDrawMode's RECT *tw through a
     * `(RECT *)(s32)var_14` conversion (Q113). Literals at every read:
     * score 37. */
    s16 var_11;
    s16 xpos;
    s16 ypos;
    s16 var_14;
    s32 var_15;
    s32 x;
    s32 row;
    s32 var_18;
    /* the sprite sheet's 8-byte cells, just past its 12-byte header. Ruling 9:
     * one meaning, header + 0xC at every write. FAKE: single-use holder of the
     * cell table ahead of the descriptor's table store; stored directly:
     * score 12. */
    Unk8009B400Record *var_19;

    var_8 = 0;
    var_11 = 0;
    xpos = 0;
    ypos = 0;
    var_14 = 0;
    s.ot_idx = 10;
    s.has_color = 0;
    table = ((Unk8006919CRec *)arg0[1])->unk_30;
    s.y = ypos;
    s.x = xpos;
    s.header = table[0];
    s.semi = 0;
    var_19 = s.header->cells;
    s.table = var_19;
    s.sprt_out = arg0[5];
    arg0[5] = gpu_AddSpriteCells(&s);
    SetDrawMode((DR_MODE *)arg0[7], 1, var_11,
                gpu_GetSheetTPage(s.header, var_8), (RECT *)(s32)var_14);
    AddPrim(g_gpu_ot_ptr + 0xA, (DR_MODE *)arg0[7]);
    arg0[7] += 0xC;

    s.ot_idx = 9;
    s.has_color = 0;
    table = ((Unk8006919CRec *)arg0[1])->unk_30;
    s.y = ypos;
    for (var_15 = 0; var_15 < 2; var_15++) {
        s.x = var_15 * 280;
        if (D_800A34FC->unk_28.half[var_15] < 3) {
            for (var_9 = 0; var_9 < 4; var_9++) {
                if (D_800A3524->unk17[D_800A34FC->unk_28.half[var_15]] &
                    ((1 << var_9) << (var_15 * 4))) {
                    s.header = table[var_9 + 13];
                    s.semi = 0;
                    var_19 = s.header->cells;
                    s.table = var_19;
                    s.sprt_out = arg0[5];
                    arg0[5] = gpu_AddSpriteCells(&s);
                }
            }
        }
    }
    s.header = table[13];
    SetDrawMode((DR_MODE *)arg0[7], 1, var_11,
                gpu_GetSheetTPage(s.header, var_8), (RECT *)(s32)var_14);
    AddPrim(g_gpu_ot_ptr + 9, (DR_MODE *)arg0[7]);
    arg0[7] += 0xC;

    s.x = xpos;
    s.y = ypos;
    s.ot_idx = 10;
    s.has_color = 0;
    table = ((Unk8006919CRec *)arg0[1])->unk_30;
    s.header = table[1];
    s.semi = 0;
    var_19 = s.header->cells;
    s.table = var_19;
    s.sprt_out = arg0[5];
    arg0[5] = gpu_AddSpriteCells(&s);
    for (var_9 = 0; var_9 < 6; var_9++) {
        s.header = table[var_9 + 2];
        s.ot_idx = 10;
        s.has_color = 0;
        s.semi = 0;
        s.y = 0;
        var_19 = s.header->cells;
        s.table = var_19;
        for (j = 0; j < 2; j++) {
            s.x = j ? 280 : 0;
            s.sprt_out = arg0[5];
            arg0[5] = gpu_AddSpriteCells(&s);
        }
    }
    table = ((Unk8006919CRec *)arg0[1])->unk_30;
    s.header = table[1];
    SetDrawMode(
        (DR_MODE *)arg0[7], 1, 0, gpu_GetSheetTPage(s.header, var_8), 0);
    AddPrim(g_gpu_ot_ptr + 0xA, (DR_MODE *)arg0[7]);
    arg0[7] += 0xC;

    recs = D_800A34FC->unk_24->unk_44;
    tile = (TILE *)arg0[6];
    for (var_9 = 0; var_9 < 11; var_9++) {
        var_5 = &recs[var_9];
        for (j = 0; j < 2; j++) {
            SetTile(tile);
            tile->r0 = var_5->r;
            tile->g0 = var_5->g;
            tile->b0 = var_5->b;
            tile->x0 = var_5->x + j * 280;
            tile->y0 = var_5->y;
            tile->w = var_5->w;
            tile->h = var_5->h;
            SetSemiTrans(tile, 0);
            AddPrim(g_gpu_ot_ptr + 0xC, tile);
            tile++;
        }
    }
    arg0[6] = (s32)tile;

    var_18 = ((rcos((D_800A3518 << 7) & 0xF80) * 32) >> 12) + 0xD0;
    poly = (POLY_G4 *)arg0[4];
    x = 0;
    for (j = 0; j < 2; j++) {
        var_9 = D_800A34FC->unk_28.half[j];
        rec = &recs[var_9 + 1];
        for (row = 0; row < 2; row++) {
            SetPolyG4(poly);
            if (var_9 == 5) {
                SetSemiTrans(poly, 1);
                var_10 = 0;
                poly->r0 = var_18;
                poly->g0 = var_18;
                poly->b0 = var_18;
                poly->r1 = var_18;
                poly->g1 = var_18;
                poly->b1 = var_18;
                poly->r2 = var_10;
                poly->g2 = 0;
                poly->b2 = 0;
                poly->r3 = var_10;
                poly->g3 = 0;
                poly->b3 = 0;
                setXYWH(poly, rec[row].x + x, rec[row].y + 1 - row, rec[row].w,
                        -4 * row + 2);
            } else {
                SetSemiTrans(poly, 0);
                var_10 = 0x80;
                poly->r0 = var_18;
                poly->g0 = 0;
                poly->b0 = 0;
                poly->r1 = var_18;
                poly->g1 = 0;
                poly->b1 = 0;
                poly->r2 = var_10;
                poly->g2 = 0;
                poly->b2 = 0;
                poly->r3 = var_10;
                poly->g3 = 0;
                poly->b3 = 0;
                setXYWH(poly, rec[row].x + x, rec[row].y + 1 - row, rec[row].w,
                        -2 * row + 1);
            }
            AddPrim(g_gpu_ot_ptr + 8, poly);
            poly++;
            SetPolyG4(poly);
            if (var_9 == 5) {
                SetSemiTrans(poly, 1);
                var_10 = 0;
                poly->r0 = var_18;
                poly->g0 = var_18;
                poly->b0 = var_18;
                poly->r2 = var_18;
                poly->g2 = var_18;
                poly->b2 = var_18;
                poly->r1 = var_10;
                poly->g1 = 0;
                poly->b1 = 0;
                poly->r3 = var_10;
                poly->g3 = 0;
                poly->b3 = 0;
                poly->x0 = rec->x + rec->w * row + x;
                poly->y0 = rec->y + 1;
                poly->x1 = rec->x + rec->w * row + x + (4 + row * -8);
                poly->y1 = rec->y + 1;
                poly->x2 = rec->x + rec->w * row + x;
                poly->y2 = rec[1].y - 1;
                poly->x3 = rec->x + rec->w * row + x + (4 + row * -8);
                poly->y3 = rec[1].y - 1;
            } else {
                SetSemiTrans(poly, 0);
                var_10 = 0x80;
                poly->r0 = var_18;
                poly->g0 = 0;
                poly->b0 = 0;
                poly->r2 = var_18;
                poly->g2 = 0;
                poly->b2 = 0;
                poly->r1 = var_10;
                poly->g1 = 0;
                poly->b1 = 0;
                poly->r3 = var_10;
                poly->g3 = 0;
                poly->b3 = 0;
                poly->x0 = rec->x + rec->w * row + x;
                poly->y0 = rec->y + 1;
                poly->x1 = rec->x + rec->w * row + x + (-4 * row + 2);
                poly->y1 = rec->y + 1;
                poly->x2 = rec->x + rec->w * row + x;
                poly->y2 = rec[1].y - 1;
                poly->x3 = rec->x + rec->w * row + x + (-4 * row + 2);
                poly->y3 = rec[1].y - 1;
            }
            AddPrim(g_gpu_ot_ptr + 8, poly);
            poly++;
        }
        SetDrawMode((DR_MODE *)arg0[7], 1, 0, 0x40, 0);
        AddPrim(g_gpu_ot_ptr + 8, (DR_MODE *)arg0[7]);
        arg0[7] += 0xC;
        x += 280;
    }
    arg0[4] = (s32)poly;
}

void menu_SetExclusiveRowBit(s32 arg0, s32 arg1) {
    s32 code;
    s16 i;
    s32 mask;

    if (arg1 & (0x10 << (arg0 * 16))) {
        code = 1;
    } else if (arg1 & (0x40 << (arg0 * 16))) {
        code = 2;
    } else if (arg1 & (0x80 << (arg0 * 16))) {
        code = 3;
    } else if (arg1 & (0x20 << (arg0 * 16))) {
        code = 0;
    }

    mask = (1 << code) << (arg0 * 4);

    for (i = 0; i < 3; i++) {
        if (i == D_800A34FC->unk_28.half[arg0]) {
            D_800A3524->unk17[i] |= mask;
        } else {
            D_800A3524->unk17[i] &= ~mask;
        }
    }
}

s32 menu_HandleRowBitInput(s32 *arg0, s32 *arg1, s16 arg2) {
    s32 i;
    s16 lim;
    s32 shift;
    s32 nib;
    s32 var_4;
    s32 j;
    u8 *rec;
    s32 ret;
    s32 var_8;
    s16 var_9;

    ret = 0;
    if ((D_800A34FC->unk_28.word == 0x50005) && (*arg1 & 0x400040)) {
        snd_QueueSound(1, 0x7F, 0x7F);
        ret = 1;
    }

    i = 0;
    nib = 0xF;
    var_4 = 0;
    shift = 0;
    for (; i < 2; shift += 0x10, i++) {
        lim = ((arg2 >> i) & 1) ? 4 : 5;

        if (*arg1 & (0x1000 << shift)) {
            snd_QueueSound(0, 0x7F, 0x7F);
            if (D_800A34FC->unk_28.half[i] <= 0) {
                D_800A34FC->unk_28.half[i] = lim;
            } else {
                D_800A34FC->unk_28.half[i] = D_800A34FC->unk_28.half[i] - 1;
            }
        } else if (*arg1 & (0x4000 << shift)) {
            snd_QueueSound(0, 0x7F, 0x7F);
            if (D_800A34FC->unk_28.half[i] >= lim) {
                D_800A34FC->unk_28.half[i] = 0;
            } else {
                D_800A34FC->unk_28.half[i] = D_800A34FC->unk_28.half[i] + 1;
            }
        }

        var_9 = D_800A34FC->unk_28.half[i];
        switch (var_9) {
        case 0:
        case 1:
        case 2:
            if (*arg1 & (0xF0 << shift)) {
                snd_QueueSound(0, 0x7F, 0x7F);
                menu_SetExclusiveRowBit(i, *arg1);
            }
            break;
        case 3:
            if (*arg1 & (0x40 << shift)) {
                snd_QueueSound(1, 0x7F, 0x7F);
                for (j = 0; j < 3; j++) {
                    rec = (u8 *)D_800A3524 + j;
                    var_8 = *(rec + 0x1A) & (nib << var_4);
                    *(rec + 0x17) =
                        (u8)((*(rec + 0x17) & ((i == 0) ? 0xF0 : 0xF)) + var_8);
                }
            }
            break;
        case 4:
            if (*arg1 & (0x40 << shift)) {
                snd_QueueSound(1, 0x7F, 0x7F);
                for (j = 0; j < 3; j++) {
                    rec = (u8 *)D_800A3524 + j;
                    var_8 = *(rec + 0x1D) & (nib << var_4);
                    *(rec + 0x17) =
                        (u8)((*(rec + 0x17) & ((i == 0) ? 0xF0 : 0xF)) + var_8);
                }
            }
            break;
        }
        var_4 += 4;
    }
    return ret;
}

/* The two per-row sprite counters: the target clears both with one word
 * store, so the storage is the counter array plus one word view (Q33); every
 * other access goes through count[]. */
typedef union {
    s16 count[2];
    s32 word;
} Counts_8006CFBC;

s32 func_8006CFBC(s32 *arg0) {
    Unk8007352CRec s;
    Counts_8006CFBC counts;
    Unk8009B0E0Record **table;
    s16 var_3;
    s16 column;
    s16 row;
    s16 result;
    /* Ruling 11 (ordinary-c-judge-decidable): holds three values, each
     * s.header + 0xC stored to s.table
     * (for table[column + 8], table[12] and table[17..19]). FAKE: single-use
     * holder of the cell table ahead of the descriptor's table store; stored
     * directly: score 8. */
    Unk8009B400Record *temp;

    result = 0;
    table = ((Unk8006919CRec *)arg0[1])->unk_30;
    s.ot_idx = 8;
    s.has_color = 0;
    s.semi = 0;

    var_3 = 0;
    do {
        counts.word = 0;
        s.y = var_3 * 16;
        column = 0;
        do {
            s.header = table[column + 8];
            temp = s.header->cells;
            s.table = temp;
            for (row = 0; row < 2; row++) {
                if (D_800A3524->unk17[var_3] & ((1 << (row * 4)) << column)) {
                    s.x = row * 280 + counts.count[row] * 23;
                    s.sprt_out = arg0[5];
                    arg0[5] = gpu_AddSpriteCells(&s);
                    counts.count[row]++;
                }
            }
            column++;
        } while (column < 4);

        row = 0;
        do {
            if (counts.count[row] == 0) {
                s.x = row * 280;
                result |= 1 << row;
                s.header = table[12];
                temp = s.header->cells;
                s.table = temp;
                s.sprt_out = arg0[5];
                arg0[5] = gpu_AddSpriteCells(&s);
            }
            row++;
        } while (row < 2);
        var_3++;
    } while (var_3 < 3);

    row = 0;
    do {
        if (D_800A34FC->unk_28.word == 0x50005) {
            s.header = table[17];
        } else if ((result >> row) & 1) {
            s.header = table[19];
        } else {
            s.header = table[18];
        }
        s.x = row * 280;
        s.y = 0;
        temp = s.header->cells;
        s.table = temp;
        s.sprt_out = arg0[5];
        arg0[5] = gpu_AddSpriteCells(&s);
        row++;
    } while (row < 2);

    s.header = table[14];
    SetDrawMode((DR_MODE *)arg0[7], 1, 0, gpu_GetSheetTPage(s.header, 0), 0);
    AddPrim(g_gpu_ot_ptr + 8, (DR_MODE *)arg0[7]);
    arg0[7] += 0xC;

    {
        u16 var_8[4];
        var_8[2] = 0xE1;
        var_8[0] = 0xCF;
        var_8[1] = 0x25;
        var_8[3] = 1;
        func_80069898(arg0, var_8, 0x11);
    }
    return (s16)result;
}

void menu_ResetRowBitCursors(void) {
    D_800A34FC->unk_28.half[1] = 5;
    D_800A34FC->unk_28.half[0] = 5;
}

extern void func_8006C21C(s32 *);
extern s32 func_8006CFBC(s32 *);

s32 menu_UpdateRowBitMenu(s32 arg0, s32 arg1) {
    s32 sp10[22];
    GpuDb *t;
    s32 r;
    D_800A3514 += 1;
    t = &g_gpu_db[D_800A36AC & 1];
    func_8006E390(sp10, &D_800A3518);
    func_80069AE4(sp10, 2, t);
    func_8006C21C(sp10);
    r = func_8006CFBC(sp10);
    return menu_HandleRowBitInput(&arg0, &arg1, (s32)((r << 16) >> 16));
}

void func_8006D3DC(s32 *arg0) {
    Unk8007352CRec s;
    u16 var_1[4];
    Unk8009B0E0Record **q;
    /* FAKE: constant holder for gpu_GetSheetTPage's 0; the literal: score 8. */
    s32 var_3 = 0;
    s16 i = 0;
    /* FAKE: constant holder for s.col_r / g / b; the literal: score 25. */
    u8 var_5 = 0x40;

    s.ot_idx = 0xA;
    q = ((Unk8006919CRec *)arg0[1])->unk_38;
    s.x = 0;
    s.has_color = 1;

    for (; i < 6; i++) {
        s.has_color = 1;
        if (i == 0) {
            s.y = 0;
            s.has_color = 0;
            s.semi = 0;
        } else if (i == D_800A3528 + 1) {
            s.y = D_800A34FC->unk_0C[1];
            s.col_r = s.col_g = s.col_b =
                ((rsin(((D_800A3514 & 0x1F) << 7) + 0x1FF) << 5) >> 12) - 0x80;
            s.semi = 0;
        } else if (i != 1 && i != 2 && i != 3 && i == D_800A3528 + 4) {
            s.col_r = s.col_g = s.col_b = 0x80;
            s.y = 0;
            s.semi = 0;
        } else {
            s.col_r = s.col_g = s.col_b = var_5;
            s.y = 0;
            s.semi = 1;
        }
        s.header = q[i];
        s.table = s.header->cells;
        s.sprt_out = arg0[5];
        arg0[5] = gpu_AddSpriteCells(&s);
        SetDrawMode(
            (DR_MODE *)arg0[7], 1, 0, gpu_GetSheetTPage(s.header, var_3), 0);
        AddPrim(g_gpu_ot_ptr + 0xA, (DR_MODE *)arg0[7]);
        arg0[7] += 0xC;
    }

    var_1[0] = 0xDA;
    var_1[1] = 0x25;
    var_1[2] = 0xCB;
    var_1[3] = 1;
    func_80069898(arg0, var_1, 0x11);
}

s32 func_8006D5D4(s32 arg0, u32 arg1) {
    s32 sp10;
    s32 result = 0;
    s32 ret;
    s32 sval;

    sp10 = (arg1 & 0xFFFF) | (arg1 >> 16);
    ret = func_800692C0(&sp10, 0, D_800A34FC->unk_0C, D_800A350C);
    if ((ret >> 16) == 1) {
        D_800A3528 = D_800A3528 + 1;
        snd_QueueSound(0, 0x7F, 0x7F);
    } else if ((ret >> 16) == 2) {
        D_800A3528 = D_800A3528 - 1;
        snd_QueueSound(0, 0x7F, 0x7F);
    }
    if ((s16)D_800A3528 < 0) {
        D_800A3528 = 2;
    }
    D_800A3528 = (s16)D_800A3528 % 3;
    if (arg1 & 0x100010) {
        snd_QueueSound(2, 0x7F, 0x7F);
        result = -1;
    } else if (arg1 & 0x400040) {
        snd_QueueSound(1, 0x7F, 0x7F);
        sval = (s16)D_800A3528;
        if (sval == 2) {
            result = -1;
        } else {
            D_800A3524->unk14_17 = sval;
            result = 1;
        }
    }
    return result;
}

extern s32 func_8006D5D4(s32, u32);

s32 func_8006D74C(s32 arg0, s32 arg1) {
    s32 sp_buf[22];
    s32 result;
    GpuDb *db;
    D_800A3514 += 1;
    db = &g_gpu_db[D_800A36AC & 1];
    func_8006E390((s32)&sp_buf[0], (s32)&D_800A3518);
    func_80069AE4(&sp_buf[0], 1, db);
    func_8006D3DC(&sp_buf[0]);
    result = func_8006D5D4(arg0, arg1);
    snd_FlushSoundQueue();
    return result;
}

s32 menu_ResetConfirmCursor(void) {
    D_800A352C = 0;
    return 1;
}

void func_8006D808(
    s32 *arg0, s32 *arg1, Unk8006D808Rec *arg2, s32 arg3, s32 arg4) {
    Unk8007352CRec s;
    /* FAKE: oversized digit array (dead-vars-local-array OVERSIZED-LOCALS):
     * only d[0]/d[1] are used; the unwritten tail brings the locals to the
     * target's 0x88 frame (s16 d[2] gives 0x80): score 26. */
    s16 d[5];
    s16 i;
    s16 k;
    s16 idx;
    s16 n;

    s.ot_idx = arg3;
    s.y = 0;
    s.x = 0;
    s.semi = 0;
    s.has_color = 0;
    s.col_r = s.col_g = s.col_b = 0xA0;
    s.header = arg2->unk_00[2];
    for (i = 0; i < 3; i++) {
        s.header = arg2->unk_00[i];
        s.table = s.header->cells;
        s.sprt_out = *arg0;
        *arg0 = gpu_AddSpriteCells(&s);
    }
    s.header = arg2->unk_00[0];
    SetDrawMode((DR_MODE *)*arg1, 1, 0, gpu_GetSheetTPage(s.header, 0), 0);
    AddPrim(g_gpu_ot_ptr + arg3, (DR_MODE *)*arg1);
    *arg1 += 0xC;

    n = 4;
    if (arg4 == -1) {
        n = 3;
    }
    for (i = 0; i < n; i++) {
        idx = D_800A3524->unk21[i].unk3;
        if (idx >= 12 && idx < 22) {
            idx -= 2;
        }
        s.header = arg2->unk_0C;
        s.table = arg2->unk_10 + 3 + idx * 3;
        s.x = arg2->unk_1C[idx];
        s.y = i * 26;
        if (i == 3) {
            s.x -= 50;
            s.y = 98;
        }
        s.sprt_out = *arg0;
        *arg0 = gpu_AddSpriteCells(&s);
        if (idx == 8) {
            s.table = arg2->unk_10;
            s.sprt_out = *arg0;
            *arg0 = gpu_AddSpriteCells(&s);
        }
    }

    s.header = arg2->unk_0C;
    SetDrawMode((DR_MODE *)*arg1, 1, 0, gpu_GetSheetTPage(s.header, 0), 0);
    AddPrim(g_gpu_ot_ptr + arg3, (DR_MODE *)*arg1);
    *arg1 += 0xC;

    for (i = 0; i < 3; i++) {
        for (k = 0; k < n; k++) {
            if ((D_800A36AC & 1) && ((k == arg4 && k < 3) || k == 3)) {
                s.has_color = 1;
            } else {
                s.has_color = 0;
            }
            switch (i) {
            case 0:
                d[0] = d[1] = D_800A3524->unk21[k].unk0;
                break;
            case 1:
                d[0] = d[1] = D_800A3524->unk21[k].unk1;
                break;
            case 2:
                d[0] = d[1] = D_800A3524->unk21[k].unk2;
                break;
            }
            d[0] = d[0] % 10;
            d[1] = d[1] / 10 % 10;
            s.header = arg2->unk_14;
            s.header->ubase = d[0] * 24;
            s.table = arg2->unk_18;
            s.x = i * 58 + 24;
            s.y = k * 26 + 2;
            if (k == 3) {
                s.x = i * 58 + 3;
                s.y = 100;
            }
            s.sprt_out = *arg0;
            *arg0 = gpu_AddSpriteCells(&s);
            s.header->ubase = d[1] * 24;
            s.x -= 24;
            s.sprt_out = *arg0;
            *arg0 = gpu_AddSpriteCells(&s);
        }
    }

    s.header = arg2->unk_14;
    SetDrawMode((DR_MODE *)*arg1, 1, 0, gpu_GetSheetTPage(s.header, 0), 0);
    AddPrim(g_gpu_ot_ptr + arg3, (DR_MODE *)*arg1);
    *arg1 += 0xC;
}

/* FAKE: frame layout (oversized live object), as Unk80069F80Rec; the plain
 * descriptor: score 21. */
typedef struct {
    Unk8007352CRec dd94_00;
    s32 tail[2];
} Unk8006DD94Rec;

void func_8006DD94(s32 *arg0) {
    /* FAKE: frame layout - `s` is the live descriptor (its address goes to
     * gpu_AddSpriteCells); tail[0..1] is unwritten padding that brings the
     * locals to the target's 0x78 frame with var_1 at sp+0x50 (an 8-aligned
     * slot, BIGGEST_ALIGNMENT -- mips.h:1082) (dead-vars-local-array
     * OVERSIZED-LOCALS). The plain descriptor: score 21. */
    Unk8006DD94Rec s;
    u16 var_1[4];
    s16 i;
    Unk8006DD94List *q;
    /* FAKE: constant holder for s.dd94_00.semi and gpu_GetSheetTPage's 0; the
     * literal: score 8. */
    s32 var_4 = 0;

    s.dd94_00.ot_idx = 0xA;
    q = ((Unk8006919CRec *)arg0[1])->unk_3C;
    s.dd94_00.x = 0;
    s.dd94_00.semi = var_4;

    for (i = 0; i < 3; i++) {
        s.dd94_00.has_color = 1;
        if (i == D_800A352C + 1) {
            s.dd94_00.y = D_800A34FC->unk_0C[1];
            s.dd94_00.col_r = s.dd94_00.col_g = s.dd94_00.col_b =
                ((rsin(((D_800A3514 & 0x1F) << 7) + 0x1FF) << 5) >> 12) - 0x80;
        } else {
            if (i == 0) {
                s.dd94_00.col_r = s.dd94_00.col_g = s.dd94_00.col_b = 0x80;
            } else {
                s.dd94_00.col_r = s.dd94_00.col_g = s.dd94_00.col_b = 0x40;
            }
            s.dd94_00.y = 0;
        }
        s.dd94_00.header = q->unk_20[i];
        s.dd94_00.table = s.dd94_00.header->cells;
        s.dd94_00.sprt_out = arg0[5];
        arg0[5] = gpu_AddSpriteCells(&s.dd94_00);
        SetDrawMode((DR_MODE *)arg0[7], 1, 0,
                    gpu_GetSheetTPage(s.dd94_00.header, var_4), 0);
        AddPrim(g_gpu_ot_ptr + 0xA, (DR_MODE *)arg0[7]);
        arg0[7] += 0xC;
    }

    func_8006D808(&arg0[5], &arg0[7], &q->unk_00, s.dd94_00.ot_idx, -1);

    var_1[2] = 0x96;
    var_1[0] = 0xF5;
    var_1[1] = 0x25;
    var_1[3] = 1;
    func_80069898(arg0, var_1, 0x11);
}

s32 menu_HandleConfirmInput(s32 arg0, u32 arg1) {
    s32 sp10;
    s32 ret;
    s32 result = 0;

    sp10 = (arg1 & 0xFFFF) | (arg1 >> 16);
    ret = func_800692C0(&sp10, 0, D_800A34FC->unk_0C, D_800A350C);
    if (((ret >> 16) & 0xFF) != 0) {
        D_800A352C += 1;
        snd_QueueSound(0, 0x7F, 0x7F);
    }
    D_800A352C &= 1;
    if ((arg1 & 0x100010) != 0) {
        result = -1;
        snd_QueueSound(2, 0x7F, 0x7F);
    } else if ((arg1 & 0x400040) != 0) {
        snd_QueueSound(1, 0x7F, 0x7F);
        if (D_800A352C != 0) {
            result = -1;
        } else {
            result = 1;
        }
    }
    snd_FlushSoundQueue();
    return result;
}

extern s32 menu_HandleConfirmInput();

s32 menu_UpdateConfirmMenu(s32 arg0, s32 arg1) {
    s32 sp10[22];
    GpuDb *t;
    D_800A3514 += 1;
    t = &g_gpu_db[D_800A36AC & 1];
    func_8006E390(sp10, &D_800A3518);
    func_80069AE4(sp10, 1, t);
    func_8006DD94(sp10);
    return menu_HandleConfirmInput(arg0, arg1);
}

extern RECT D_800A32D8;

s32 gpu_BeginFullscreenImageDisplay(void) {
    s32 temp_s3 = D_800A3500;
    RECT rect;
    s32 v0;

    rect = D_800A32D8;
    if (D_800A3524->unk20_0) {
        v0 = cdrom_GetFileIndex(2, 0x60);
    } else {
        v0 = cdrom_GetFileIndex(2, 7);
    }
    cdrom_StartRead(v0, D_800A3500);
    cdrom_WaitIdle();
    cdrom_GetFileSize(v0);
    SetDispMask(0);
    SetDefDrawEnv(&g_gpu_db[0].draw, 0, 0, 0x280, 0xF0);
    SetDefDrawEnv(&g_gpu_db[1].draw, 0, 0xF0, 0x280, 0xF0);
    SetDefDispEnv(&g_gpu_db[0].disp, 0, 0xF0, 0x280, 0xF0);
    SetDefDispEnv(&g_gpu_db[1].disp, 0, 0, 0x280, 0xF0);
    g_gpu_db[0].disp.isinter = 0;
    g_gpu_db[1].disp.isinter = 0;
    g_gpu_db[0].disp.isrgb24 = 0;
    g_gpu_db[1].disp.isrgb24 = 0;
    DrawSync(0);
    ClearImage(&rect, 0, 0, 0);
    DrawSync(0);
    LoadImage(&rect, (u32 *)(temp_s3 + 0x14));
    DrawSync(0);
    PutDrawEnv(&g_gpu_db[0].draw);
    PutDispEnv(&g_gpu_db[1].disp);
    SetDispMask(1);
    return 1;
}

extern RECT D_800A32E0;

s32 gpu_EndFullscreenImageDisplay(void) {
    RECT rect;
    SetDispMask(0);
    PutDrawEnv(&g_gpu_db[D_800A3518 & 1].draw);
    PutDispEnv(&g_gpu_db[D_800A3518 & 1].disp);
    DrawSync(0);
    rect = D_800A32E0;
    ClearImage(&rect, 0, 0, 0);
    DrawSync(0);
    gpu_SetDrawEnvBg(1, 0, 0, 0);
    SetDispMask(1);
    return 1;
}

Unk8006E49CRec *func_80069120(s32);

void func_8006E390(s32 *a0, s32 *a1) {
    s32 *s0 = a0;
    Unk8006E49CRec *v0;
    a1[0]++;
    v0 = func_80069120(a1[0] & 1);
    s0[1] = (s32)D_800A34FC->unk_24;
    s0[3] = (s32)v0->unk_04;
    s0[2] = v0->unk_00;
    s0[4] = (s32)v0->unk_08;
    s0[5] = v0->unk_10;
    s0[6] = (s32)v0->unk_0C;
    s0[7] = (s32)v0->unk_14;
    s0[8] = (s32)v0->unk_18;
    D_800A3520 = v0;
    s0[9] = (s32)v0->unk_1C;
}

void reloc_RelocateSelfOffsets(s32 *a0) {
    s32 *p = a0;
    if (*p == -1)
        return;
    while (*p != -1) {
        *p = *p + (s32)a0;
        p++;
    }
}

s32 gpu_GetSheetTPage(Unk8009B0E0Record *arg0, s32 a1) {
    return (arg0->tp0 & 0xFE1F) + (arg0->tp1 << 7) + a1;
}

s32 gpu_InitPrimBufferPair(s32 arg0, Unk8006E49CRec *arg1) {
    u8 *base1;
    u8 *base2;
    u8 *base3;
    u8 *base4;
    arg1[0].unk_00 = arg0;
    base1 = (u8 *)arg1[0].unk_00 + 0x9C40;
    arg1[0].unk_08 = base1 + 0x5DC0;
    arg1[0].unk_04 = base1;
    base2 = base1 + 0x6838;
    arg1[0].unk_0C = base1 + 0x61F8;
    arg1[0].unk_14 = base2 + 0x1B58;
    arg1[0].unk_18 = base2 + 0x1DB0;
    arg1[0].unk_1C = base2 + 0x1E28;
    arg1[0].unk_20 = base2 + 0x1EA0;
    arg1[0].unk_10 = (s32)base2;
    arg1[1].unk_00 = (s32)(base2 + 0x1FB0);
    base3 = (u8 *)arg1[1].unk_00 + 0x9C40;
    arg1[1].unk_08 = base3 + 0x5DC0;
    arg1[1].unk_04 = base3;
    base4 = base3 + 0x6838;
    arg1[1].unk_0C = base3 + 0x61F8;
    arg1[1].unk_14 = base4 + 0x1B58;
    arg1[1].unk_18 = base4 + 0x1DB0;
    arg1[1].unk_1C = base4 + 0x1E28;
    arg1[1].unk_20 = base4 + 0x1EA0;
    arg1[1].unk_10 = (s32)base4;
    return (s32)(base4 + 0x1FB0);
}

/* Q65: this file's initialized small data (.sdata, 0x800A32C0..0x800A32E8),
 * in address order; values from the original EXE. */
/* copied whole into a local 8-byte buffer */
u8 D_800A32C0[8] = {0x70, 0x00, 0xE1, 0x01, 0x30, 0x00, 0x01, 0x00};
u32 D_800A32C8[2] = {0x2000, 0x4000};
u32 D_800A32D0[2] = {0x8000, 0x1000};
RECT D_800A32D8 = {0, 0, 640, 240};
RECT D_800A32E0 = {0, 0, 640, 480};

/* Q65: tentative definitions (COMMON) of the small data this file reaches
 * gp-relative. */
TILE *D_800A36DC;
DR_MODE *D_800A36E0;
u8 *D_800A36E4;
s32 D_800A372C;
s32 D_800A3900;

/* .data 0x8009BC04..0x8009BC1C, this file's whole .data (51268.c's
 * D_8009BBFC ends at 0x8009BC04, d_8C41C.c's D_8009BC1C follows): the list
 * menu's state, rows 0-7 switched on, row mask 0xBF, then the eight option
 * rows {state, mode}. Defined after every function, so no body sees a
 * definition where it saw an extern. */
MenuState D_8009BC04 = {
    0xFF,
    0xBF,
    {{0, 3}, {1, 3}, {2, 3}, {4, 1}, {5, 1}, {6, 2}, {3, 3}, {7, 0}},
};
