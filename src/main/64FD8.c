/* Game functions func_800747D8 .. func_800788B0. .text 0x800747D8 (ROM 0x64FD8). Start boundary:
 * PHASE (site 5). Ends where the PsyQ 4.0 LIBAPI C67 module starts (0x80078948, LIBSCAN, Q106
 * D3); the library modules that followed are in src/main/psxsdk/libapi/ and libc2/. */
#define INCLUDE_ASM_USE_MACRO_INC 1
#include "common.h"
#include "include_asm.h"
#include "bb2.h"

/* ---- merged from text1b_tu2.c (owner ruling Q67: one original file) ---- */
/* Declarations from the file this TU was split from (text1b.c). */
extern s32 ClearOTagR(s32, s32);
extern s32 rsin();
extern s32 g_gpu_ot_ptr;
extern Unk8009BD38Flags D_8009BD38;
extern s32 SetDrawMode(s32, s32, s32, s32, s32);
extern s32 AddPrim(s32, s32);
extern s32 SetDrawArea();
extern void SetDrawOffset();
/* func_8007352C's draw descriptor: .header = the sprite sheet's SprtHdrA, .table = its
   SprtEntA cell array (the s32 form of S_80074488 / DescF97C). */
typedef struct EnvA {
    s32  header;
    s32  table;
    s32  out;
    s32  pad0C;
    s32  semi;
    u32  ot_idx;
    s32  x;
    s32  y;
    s32  pad20, pad24;
    u8   has_color;
    u8   col_r;
    u8   col_g;
    u8   col_b;
} EnvA;
s32 func_8006E49C(s32 arg0, s32 *arg1);
void func_8006E950(s32 a0, s32 *a1);

/* func_800747D8: the duplicated `sound = 4;` below is the
 * duplicated-statement-into-arms shape and carries its FAKE annotation inline. */
/* 0x800A35D0: one {s16, s16} pair per player (two words,
 * asm/data/91C98.data.s:4279-4282), passed to func_800692C0 beside SelWork
 * f40[player]; func_800768DC indexes it by player * 4 (0x80076948/5C). */

/* Q65: this file's statics (.sbss, allocated per file in link order by PSYLINK), in address order. */
static s16 D_800A35D0[2][2];
static s32 D_800A35D8;
static s8 D_800A35DC;
static s32 D_800A35E0;
static s32 D_800A35E4;
static s32 D_800A35E8;
static s32 D_800A35EC;  /* not named by any code or data: size from the gap */
static s32 D_800A35F0;
static s32 D_800A35F4;
static s32 D_800A35F8;
static s32 D_800A35FC;
static s32 D_800A3600;
static s32 D_800A3604;  /* not named by any code or data: size from the gap */
static s32 D_800A3608;
static s32 D_800A360C;
static s32 * D_800A3610;
static s32 D_800A3614;

s32 func_800747D8(u32 input) {
    SelWork *base;
    s32 sp10;
    s32 ret;
    s32 state;
    s32 result;
    s32 i;
    u8 row;
    s32 sound;

    base = SELWORK;
    result = 0;
    if (base->f10.word == 0) {
        sp10 = (input & 0xFFFF) | (input >> 16);
        ret = func_800692C0((u32 *)&sp10, 0, base->f40[0], D_800A35D0[0]);
        switch (ret >> 16) {
    case 1:
        SELWORK->f34 = 0;
        SELWORK->f3C[0] += 1;
        if (SELWORK->f3C[0] >= 5) {
            SELWORK->f3C[0] = 0;
        }
        func_8005C650(0, 0x7F, 0x7F);
        break;
    case 2:
        SELWORK->f34 = 0;
        SELWORK->f3C[0] -= 1;
        if (SELWORK->f3C[0] < 0) {
            SELWORK->f3C[0] = 4;
        }
        func_8005C650(0, 0x7F, 0x7F);
        break;
        }

        state = SELWORK->f3C[0];
        switch (state) {
    case 0:
        switch (ret & 0xFF) {
        case 1:
            if (SELWORK->f65 == SELWORK->f64) {
                SELWORK->f65 = 0;
            } else {
                SELWORK->f65 += 1;
            }
            /* FAKE: `sound = 4;` is written into BOTH inner-switch arms instead of one
             * shared copy after `selection_sound:` -- reg_set_last
             * (tools/gcc-2.7.2/rtlanal.c:886-888) stops scanning at the
             * `selection_sound:` CODE_LABEL, so the store-flag gate at
             * tools/gcc-2.7.2/jump.c:1178 sees temp3 = a REG rather than a
             * CONST_INT and (BRANCH_COST == 1 on R3000) refuses the branchless
             * sltiu/sll fold. */
            sound = 4;
            goto selection_sound;
        case 2:
            if (SELWORK->f65 == 0) {
                SELWORK->f65 = SELWORK->f64;
            } else {
                SELWORK->f65 -= 1;
            }
            /* FAKE: `sound = 4;` is written into BOTH inner-switch arms instead of one
             * shared copy after `selection_sound:` -- reg_set_last
             * (tools/gcc-2.7.2/rtlanal.c:886-888) stops scanning at the
             * `selection_sound:` CODE_LABEL, so the store-flag gate at
             * tools/gcc-2.7.2/jump.c:1178 sees temp3 = a REG rather than a
             * CONST_INT and (BRANCH_COST == 1 on R3000) refuses the branchless
             * sltiu/sll fold. */
            sound = 4;
            goto selection_sound;
        }
        goto confirm;
selection_sound:
        if (SELWORK->f64 != 0) {
            sound = 0;
        }
        func_8005C650(sound, 0x7F, 0x7F);
        goto confirm;
    case 1:
        if ((ret & 0xFF) != 0) {
            SELWORK->f67 += 1;
            SELWORK->f67 &= 1;
            SELWORK->f66 = D_8009BD20[SELWORK->f67][D_800A35DC & 1];
            func_8005C650(0, 0x7F, 0x7F);
        }
        goto confirm;
    case 2:
        if ((ret & 0xFF) != 0) {
            row = SELWORK->f67;
            D_800A35DC += 1;
            SELWORK->f66 = D_8009BD20[row][D_800A35DC & 1];
            func_8005C650(0, 0x7F, 0x7F);
        }
confirm:
        if (input & 0x400040) {
            func_8005C650(1, 0x7F, 0x7F);
            SELWORK->f3C[0] = 3;
        }
        break;
    case 3:
        if (input & 0x400040) {
            func_8005C650(1, 0x7F, 0x7F);
            for (i = 0; i < 2; i++) {
                SELWORK->f10.half[i] = 3;
                SELWORK->f18[i] = 1;
                SELWORK->f38[i] = 0;
            }
        }
        break;
    case 4:
        if (input & 0x400040) {
            func_8005C650(1, 0x7F, 0x7F);
            result = -1;
        }
        break;
        }
        if (input & 0x100010) {
            func_8005C650(2, 0x7F, 0x7F);
            result = -1;
        }
    }
    return result;
}




void func_80074B18(s32 *arg0, s32 arg1, s32 arg2) {
    u8 *p;
    u8 *t;
    s16 i;
    s16 j;
    s16 n;
    s32 ot;

    n = 5;
    if (arg2 != 0) {
        n = 8;
    }
    p = (u8 *)arg0[5];
    for (i = 0; i < SELWORK->f65 + 3; i++) {
        t = (u8 *)SELWORK->f04[0x3C / 4];
        for (j = 0; j < n; j++) {
            SetTile((TILE *)p);
            *(u8 *)(p + 4) = *(u8 *)(t + 8);
            *(u8 *)(p + 5) = *(u8 *)(t + 9);
            *(u8 *)(p + 6) = *(u8 *)(t + 0xA);
            *(u16 *)(p + 0xC) = *(u16 *)(t + 4);
            *(u16 *)(p + 0xE) = *(u16 *)(t + 6);
            SetSemiTrans((GameObj *)p, 0);
            if (arg2 != 0) {
                *(s16 *)(p + 8) = *(u16 *)(t + 0) + arg1 * 240;
                *(s16 *)(p + 0xA) = *(u16 *)(t + 2) + i * 34 + 0x2B;
            } else {
                *(s16 *)(p + 8) = *(u16 *)(t + 0) + arg1 * 240;
                *(s16 *)(p + 0xA) = *(u16 *)(t + 2) + i * 17 + 0x7C;
            }
            ot = 0xB;
            if (arg1 != 0) {
                ot = 0x15;
            }
            AddPrim(g_gpu_ot_ptr + ot * 4, (GameObj *)p);
            p += 0x10;
            t += 0xC;
        }
    }
    arg0[5] = (s32)p;
}
typedef struct {
    s32 sp18, sp1C, sp20, sp24, sp28, sp2C, sp30, sp34, sp38, sp3C;
    s8 sp40;
} S_80074D2C;

void func_80074D2C(s32 arg0, s32 arg1, s32 arg2) {
    S_80074D2C s;
    s32 var_s1;
    s32 sp18_val;
    s32 inner_ptr;

    var_s1 = 0xC;
    s.sp28 = 0;
    s.sp40 = 0;
    inner_ptr = *(s32 *)((s32)*(s32 *)arg0 + 0x1C);
    sp18_val = *(s32 *)(((arg2 << 16) >> 14) + inner_ptr);
    s.sp30 = arg1 * 0xF0;
    s.sp34 = 0;
    s.sp18 = sp18_val;
    s.sp1C = sp18_val + 0xC;
    if (arg1 != 0) {
        var_s1 = 0x16;
    }
    s.sp2C = var_s1;
    s.sp20 = *(s32 *)(arg0 + 0x10);
    *(s32 *)(arg0 + 0x10) = func_8007352C((s32)&s.sp18);
    SetDrawMode(*(s32 *)(arg0 + 0x18), 1, 0, func_8006E480(s.sp18, 0), 0);
    AddPrim(g_gpu_ot_ptr + var_s1 * 4, *(s32 *)(arg0 + 0x18));
    *(s32 *)(arg0 + 0x18) += 0xC;
}
void func_80074E08(s32 *arg0, s32 arg1) {
    EnvA s;
    u16 rect[4];
    u16 offset[2];
    s32 *records;
    s32 prim;
    s32 ot_idx;
    /* work holds two values (owner Ruling 11; the first under its Q20
       per-branch-constant clause): the backdrop TILE's OT index (0xE for
       player 1, 4 for player 0), then ot_idx * 4, the OT byte offset the last
       three AddPrim calls add. */
    s32 work;
    s32 rect_x;
    /* the sprite sheet's cell array (8-byte SprtEntA cells), which starts
       just past the sheet's one 12-byte SprtHdrA header (+0xC) on each of
       the four root+0x18 sheets this function draws: one meaning at one
       constant offset, written once per sheet (owner Ruling 9). */
    s32 cells;
    s16 i;

    prim = arg0[5];
    SetTile((TILE *)prim);
    SetSemiTrans((TILE *)prim, 0);
    *(u8 *)(prim + 4) = 0xD0;
    *(u8 *)(prim + 5) = 0xC8;
    *(u8 *)(prim + 6) = 0xB8;
    *(s16 *)(prim + 8) = arg1 * 0xF0 + 0x62;
    *(s16 *)(prim + 0xA) = 0x14;
    *(s16 *)(prim + 0xC) = 0xCC;
    *(s16 *)(prim + 0xE) = 0xC8;
    if (arg1 != 0) {
        work = 0xE;
    } else {
        work = 4;
    }
    AddPrim(g_gpu_ot_ptr + work * 4 + 0x24, prim);
    prim += 0x10;
    arg0[5] = prim;

    records = *(s32 **)(arg0[0] + 0x18);
    s.semi = 0;
    s.has_color = 0;
    s.x = arg1 * 0xF0;
    s.ot_idx = 2;
    i = 0;
    do {
        s.y = (0xD2 - SELWORK->f0C[arg1]) * i;
        s.header = records[3];
        cells = s.header + 0xC;
        s.table = cells;
        s.out = arg0[4];
        arg0[4] = func_8007352C((s32)&s);
        s.header = records[2];
        cells = s.header + 0xC;
        s.table = cells;
        s.out = arg0[4];
        arg0[4] = func_8007352C((s32)&s);
        s.table += *(u8 *)(s.header + 2) * 8;
        s.pad20 = 0xCE00;
        s.pad24 = 0x100;
        s.pad0C = arg0[1];
        arg0[1] = func_80073728((s32)&s, 0);
        i++;
    } while (i < 2);

    s.x = arg1 * 0xF0;
    s.y = 0;
    s.header = records[0];
    cells = s.header + 0xC;
    s.table = cells;
    if (arg1 != 0) {
        s.ot_idx = 0x16;
    } else {
        s.ot_idx = 0xC;
    }
    s.out = arg0[4];
    arg0[4] = func_8007352C((s32)&s);
    s.header = records[1];
    cells = s.header + 0xC;
    s.table = cells;
    s.out = arg0[4];
    arg0[4] = func_8007352C((s32)&s);
    SetDrawMode(arg0[6], 1, 0, func_8006E480(s.header, 0), 0);
    AddPrim(g_gpu_ot_ptr + s.ot_idx * 4, arg0[6]);
    arg0[6] += 0xC;

    if (arg1 != 0) {
        ot_idx = 0xE;
        rect_x = 0x14E;
    } else {
        ot_idx = 4;
        rect_x = 0x5E;
    }
    rect[0] = ((DRAWENV *)SELWORK->f24)->clip.x + rect_x;
    rect[1] = ((DRAWENV *)SELWORK->f24)->clip.y + 0x14;
    rect[2] = 0xD4;
    rect[3] = 0xC8 - SELWORK->f0C[arg1];
    SetDrawArea(arg0[7], rect);
    AddPrim(g_gpu_ot_ptr + ot_idx * 4 + 0x24, arg0[7]);
    arg0[7] += 0xC;

    rect[0] = ((DRAWENV *)SELWORK->f24)->clip.x;
    rect[1] = ((DRAWENV *)SELWORK->f24)->clip.y;
    rect[2] = ((DRAWENV *)SELWORK->f24)->clip.w;
    rect[3] = ((DRAWENV *)SELWORK->f24)->clip.h;
    SetDrawArea(arg0[7], rect);
    work = ot_idx * 4;
    AddPrim(g_gpu_ot_ptr + work, arg0[7]);
    arg0[7] += 0xC;

    offset[0] = ((DRAWENV *)SELWORK->f24)->ofs[0];
    offset[1] = ((DRAWENV *)SELWORK->f24)->ofs[1]
              - SELWORK->f08[arg1];
    SetDrawOffset(arg0[8], offset);
    AddPrim(g_gpu_ot_ptr + work + 0x24, arg0[8]);
    arg0[8] += 0xC;

    offset[0] = ((DRAWENV *)SELWORK->f24)->ofs[0];
    offset[1] = ((DRAWENV *)SELWORK->f24)->ofs[1];
    SetDrawOffset(arg0[8], offset);
    AddPrim(g_gpu_ot_ptr + work, arg0[8]);
    arg0[8] += 0xC;
}
void func_8007526C(void) {
    SelWork *base;
    s32 i;

    base = SELWORK;
    i = 0;
    do {
        switch ((u8)base->f10.half[i]) {
        case 1:
            base->f08[i] += 0xA;
            base->f0C[i] += 0xA;
            if (base->f0C[i] >= 0xC8) {
                if (((u16)base->f10.half[i] >> 8) == 0) {
                    base->f10.half[i] += 1;
                }
                base->f08[i] = 0;
                base->f0C[i] = 0xC8;
                base->f14.half[i] = base->f18[i];
                base->f3C[i] = base->f38[i];
            }
            break;
        case 3:
            base->f0C[i] += 0xA;
            if (base->f0C[i] >= 0xC8) {
                base->f08[i] = 0xC8;
                base->f0C[i] = 0xC8;
                base->f14.half[i] = base->f18[i];
                base->f3C[i] = base->f38[i];
                if (((u16)base->f10.half[i] >> 8) == 0) {
                    base->f10.half[i] += 1;
                }
            }
            break;
        case 2:
            base->f0C[i] -= 0xA;
            if (base->f0C[i] <= 0) {
                base->f08[i] = 0;
                base->f0C[i] = 0;
                base->f10.half[i] = 0;
            }
            break;
        case 4:
            base->f08[i] -= 0xA;
            base->f0C[i] -= 0xA;
            if (base->f0C[i] <= 0) {
                base->f08[i] = 0;
                base->f0C[i] = 0;
                base->f10.half[i] = 0;
            }
            break;
        }
        i++;
    } while (i < 2);
}
typedef struct {
    s32 sp18, sp1C, sp20, sp24, sp28, sp2C, sp30, sp34, sp38, sp3C;
    u8 sp40, sp41, sp42, sp43;
} S_753D8;



extern s32 rsin(s32);

/* Per-player gauge draw: fills the 0x2C-byte sprite descriptor `s` (the same
   descriptor shape func_8006DD94 / func_80069F80 hand to func_8007352C) for the
   player's gauge frame, two fixed sub-elements from the 0x14 table, and two
   layered elements from the 0x2C table, emitting a SetDrawMode prim after each
   group. x position is arg1 * 240 (player 1 sits one screen-half right); the
   frame's y is the per-player 0x68 counter * 90 plus the per-player s16 at
   0x42; the frame colour breathes with rsin of the 5-bit timer at 0x34. */
void func_800753D8(s32 *arg0, s32 arg1) {
    S_753D8 s;
    s32 *tbl;
    s16 i;
    s32 body;
    s32 c;
    /* FAKE: constant-holder — the 0 passed to both func_8006E480 calls is
       kept live in callee-save $s5 (target: `addu $s5,$zero,$zero` in the
       prologue, `addu $a1,$s5,$zero` at both call sites) instead of being
       re-materialized as `li $a1,0`; mechanism: global.c allocates the
       once-set constant pseudo a callee-save because it crosses calls and
       cse.c only folds the constant within the entry extended basic block.
       SOTN ships this exact shape: src/dra/7879C.c:2067 `s32 zero = 0;`. */
    s32 zero;
    SelWork *base;

    zero = 0;
    s.sp28 = 0;
    if (arg1 != 0) {
        s.sp2C = 0x16;
    } else {
        s.sp2C = 0xC;
    }
    tbl = *(s32 **)(arg0[0] + 0x2C);
    s.sp18 = tbl[arg1 + 2];
    s.sp30 = arg1 * 240;
    body = s.sp18 + 0xC;
    s.sp1C = body;
    base = SELWORK;
    s.sp34 = base->f68[arg1] * 90 + base->f40[arg1][1];
    c = ((rsin(((base->f34 & 0x1F) << 7) + 0x1FF) << 5) >> 12) - 0x80;
    s.sp43 = c;
    s.sp42 = c;
    s.sp41 = c;
    s.sp40 = 1;
    s.sp20 = arg0[4];
    arg0[4] = func_8007352C((s32)&s);
    s.sp40 = 0;
    tbl = *(s32 **)(arg0[0] + 0x14);
    i = 0;
    s.sp18 = tbl[0];
    s.sp30 = arg1 * 240 + 0x9D;
    s.sp34 = 0x36;
    body = s.sp18 + 0xC;
    s.sp1C = body;
    s.sp20 = arg0[4];
    arg0[4] = func_8007352C((s32)&s);
    s.sp34 = 0x90;
    s.sp1C += *(u8 *)(s.sp18 + 2) << 3;
    s.sp20 = arg0[4];
    arg0[4] = func_8007352C((s32)&s);
    SetDrawMode(arg0[6], 1, 0, func_8006E480(s.sp18, zero), 0);
    AddPrim(g_gpu_ot_ptr + s.sp2C * 4, arg0[6]);
    arg0[6] += 0xC;
    tbl = *(s32 **)(arg0[0] + 0x2C);
    do {
        s.sp18 = tbl[i];
        s.sp30 = arg1 * 240;
        s.sp34 = 0;
        body = s.sp18 + 0xC;
        if (arg1 != 0) {
            s.sp2C = 0x16;
        } else {
            s.sp2C = 0xC;
        }
        s.sp1C = body;
        s.sp20 = arg0[4];
        arg0[4] = func_8007352C((s32)&s);
        SetDrawMode(arg0[6], 1, 0, func_8006E480(s.sp18, zero), 0);
        AddPrim(g_gpu_ot_ptr + s.sp2C * 4, arg0[6]);
        i++;
        arg0[6] += 0xC;
    } while (i < 2);
}
void func_80075670(s32 arg0, s32 arg1) {
    s16 i;
    SelWork *base;
    SelWork *work;

    base = SELWORK;
    if (base->f10.word != 0) {
        return;
    }
    if ((func_800692C0(&arg0, arg1, base->f40[arg1], D_800A35D0[arg1]) >> 16) != 0) {
        SELWORK->f34 = 0;
        SELWORK->f68[arg1] = SELWORK->f68[arg1] + 1;
        SELWORK->f68[arg1] &= 1;
        func_8005C650(0, 0x7F, 0x7F);
    }
    work = SELWORK;
    work->f3C[arg1] = work->f68[arg1];
    if (arg0 & (0x40 << (arg1 * 16))) {
        for (i = 0; i < 2; i++) {
            work->f38[i] = 0;
            work->f10.half[i] = 1;
            work->f18[i] = 2;
        }
        SELWORK->f68[(arg1 + 1) & 1] = (SELWORK->f68[arg1] + 1) & 1;
        func_8005C650(1, 0x7F, 0x7F);
        return;
    }
    if (arg0 & (0x10 << (arg1 * 16))) {
        if (work->f14.half[(arg1 != 0) ? 0 : 1] == 1) {
            for (i = 0; i < 2; i++) {
                work->f38[i] = 0;
                work->f10.half[i] = 3;
                work->f18[i] = 0;
            }
            func_8005C650(2, 0x7F, 0x7F);
        } else {
            func_8005C650(4, 0x7F, 0x7F);
        }
    }
}
void func_80075830(s32 *arg0, s32 arg1, s32 arg2, s32 arg3) {
    S_80074488 s;
    s16 var_a1;
    s16 var_a2;
    s32 temp_v0;
    s32 temp_v1;
    s.sp28 = arg3;
    temp_v1 = ((s32) (rsin(((SELWORK->f34 & 0x1F) << 7) + 0x1FF) * 0x30) >> 12) - 0x40;
    s.sp43 = temp_v1;
    s.sp42 = temp_v1;
    s.sp41 = temp_v1;
    temp_v0 = *(s32 *)(*(s32 *)(*arg0 + 0x14) + 0x54);
    s.sp18 = temp_v0;
    s.sp1C = temp_v0 + 0xC;
    if (arg1 < 0xA) {
        var_a1 = arg1 / 5;
        var_a2 = arg1 % 5;
    } else {
        var_a1 = (arg1 - 0xA) / 5;
        var_a2 = (arg1 - 0xA) % 5;
    }
    if (SELWORK->f1C.half[arg2] == var_a1 && SELWORK->f20.half[arg2] == var_a2) {
        s.sp40 = 1;
    } else {
        s.sp40 = 0;
    }
    s.sp30 = arg2 * 0xF0 + var_a1 * 0x64;
    s.sp34 = var_a2 * 16;
    if (arg2 != 0) {
        s.sp2C = 0x13;
    } else {
        s.sp2C = 9;
    }
    s.sp20 = arg0[0x10 / 4];
    arg0[0x10 / 4] = func_8007352C((s32)&s);
}
/* 0x8009BCE4: 20 flag bytes indexed by entry id (asm/data/7D920.data.s:23732-23753;
 * func_800768DC lbu/sb at 0x80076B90/0x80076BA4). */
extern u8 D_8009BCE4[20];

/* Character-select grid renderer (select-screen case 2 of func_80077374; the
 * draw half of func_80075F80), called once per player per frame.
 *   arg0 = draw context (arg0[0] root table, arg0[4] sprite chain,
 *          arg0[6] DR_MODE cursor)
 *   arg1 = select page into D_8009BCF8 (10 cells per page)
 *   arg2 = this player's pick list (-1 = cleared slot)
 *   arg3 = player index (0/1)
 * Draws the page frame, the page's 10 character cells (selectable cells as
 * sprites with the cursor cell highlighted, taken cells via func_80075830,
 * unselectable cells via func_80075830), the picks made so far, then the
 * f65+3 slot sprites, and closes with two DR_MODE/AddPrim pairs. */
void func_800759D0(s32 *arg0, s32 arg1, s16 *arg2, s32 arg3) {
    S_80074488 s;
    /* table holds three values, each a sprite-sheet pointer table read from the
     * root at arg0[0]: the page table at +0x14 (the head; read by the head and
     * loops 1-2), the slot table at +0x20 + f65 * 4 (reloaded every loop-3
     * iteration), and the page table at +0x14 again for the closing DR_MODE
     * pair.  One local, not three (owner Ruling 11). */
    s32 *table;
    s32 color;
    /* FAKE: constant-holder (named-local-fake-exception) -- the 0 passed as
       func_8006E480's second argument at all three call sites is kept in
       callee-save $fp (target: `addu $fp,$zero,$zero` in the entry block,
       `addu $a1,$fp,$zero` at each call) instead of being re-materialized;
       global.c gives the once-set constant pseudo a callee-save because it
       crosses every call, and cse only folds a constant within the entry
       extended basic block. Same shape as the siblings
       func_800753D8 (`zero`) and func_8007636C (`mode`). */
    s32 zero;
    s16 i;
    /* the sheet's cell array. The code treats each sheet it draws here as
       12-byte SprtHdrA headers followed by 8-byte SprtEntA cells: the head
       treats the table[0] page sheet as one header (cells at +0xC), and
       loops 1-3 treat their sheets as three headers, normal then one cursor
       highlight per player at +12/+24 (cells at +0x24). Loop 2 applies that
       three-header view to every pick slot, including the 0x14 placeholder
       func_80075F80 stores for an unavailable cell, whose sheet the code
       elsewhere draws as one header (func_80075830); there the +0x24 runs
       past the sheet into bytes nothing references (owner Ruling 9). */
    s32 cells;

    zero = 0;
    s.sp28 = 0;
    s.sp40 = 0;
    table = *(s32 **)(arg0[0] + 0x14);
    s.sp18 = table[0];
    s.sp30 = arg3 * 240 + 0x88;
    s.sp34 = 0x33;
    cells = s.sp18 + 0xC;
    s.sp1C = cells;
    if (arg3 != 0) {
        s.sp2C = 0x16;
    } else {
        s.sp2C = 0xC;
    }
    if (arg1 != 0) {
        s.sp1C += *(u8 *)(s.sp18 + 2) * 8;
    }
    s.sp20 = arg0[4];
    arg0[4] = func_8007352C((s32)&s);
    SetDrawMode(arg0[6], 1, 0, func_8006E480(s.sp18, zero), 0);
    AddPrim(g_gpu_ot_ptr + s.sp2C * 4, arg0[6]);
    arg0[6] += 0xC;

    color = ((rsin(((SELWORK->f34 & 0x1F) << 7) + 0x1FF) << 5) >> 12) - 0x50;
    s.sp41 = s.sp42 = s.sp43 = color;
    if (arg3 != 0) {
        s.sp2C = 0x14;
    } else {
        s.sp2C = 0xA;
    }
    s.sp30 = arg3 * 240;
    for (i = arg1 * 10; i < arg1 * 10 + 10; i++) {
        /* i indexes the whole table flat (arg1 * 10 + cell), from the first record. */
        /* SOTN: src/dra/62DEC.c:1091 @aa53500 */
        /* FAKE: pointer form of the flat read (Q53).  `&D_8009BCF8[0][0] + i`
         * forms the address as its own value, so cse reuses it for the second
         * read and loop.c hoists the table base into $s6 (the target's
         * `lui/addiu $s6` and 2-byte step); D_8009BCF8[0][i].unk0 at the three
         * sites folds the symbol into each load (`lui $at; addu; lbu %lo`). */
        u8 entry = (&D_8009BCF8[0][0] + i)->unk0;

        if (D_8009BCE4[entry] & 1) {
            s.sp18 = table[entry + 1];
            cells = s.sp18 + 0x24;
            s.sp1C = cells;
            if (D_8009BCF8[arg1][SELWORK->f1C.half[arg3] * 5 + SELWORK->f20.half[arg3]].unk0 == (&D_8009BCF8[0][0] + i)->unk0) {
                s.sp40 = 1;
                s.sp18 = s.sp18 + 12 + arg3 * 12;
            } else {
                s.sp40 = 0;
            }
            s.sp34 = 0;
            s.sp20 = arg0[4];
            arg0[4] = func_8007352C((s32)&s);
            if (D_8009BCE4[(&D_8009BCF8[0][0] + i)->unk0] & (4 << arg3)) {
                func_80075830(arg0, i, arg3, 1);
            }
        } else {
            func_80075830(arg0, i, arg3, 0);
        }
    }

    for (i = 0; i < SELWORK->f3C[arg3] + 1; i++) {
        if (arg2[i] >= 0) {
            s.sp18 = table[arg2[i] + 1];
            cells = s.sp18 + 0x24;
            if (i != SELWORK->f3C[arg3]) {
                s.sp40 = 0;
            } else {
                s.sp40 = 1;
                s.sp18 = s.sp18 + 12 + arg3 * 12;
            }
            s.sp34 = i * 17;
            s.sp1C = cells;
            s.sp1C += *(u8 *)(s.sp18 + 2) * 8;
            s.sp20 = arg0[4];
            arg0[4] = func_8007352C((s32)&s);
        }
    }

    s.sp40 = 0;
    for (i = 0; i < SELWORK->f65 + 3; i++) {
        table = *(s32 **)(arg0[0] + SELWORK->f65 * 4 + 0x20);
        s.sp18 = table[i];
        cells = s.sp18 + 0x24;
        if (i == SELWORK->f3C[arg3]) {
            s.sp18 = s.sp18 + 12 + arg3 * 12;
        }
        s.sp1C = cells;
        s.sp30 = arg3 * 240;
        s.sp34 = i * 17;
        if (arg3 != 0) {
            s.sp2C = 0x14;
        } else {
            s.sp2C = 0xA;
        }
        s.sp20 = arg0[4];
        arg0[4] = func_8007352C((s32)&s);
    }

    table = *(s32 **)(arg0[0] + 0x14);
    s.sp18 = table[1];
    SetDrawMode(arg0[6], 1, 0, func_8006E480(s.sp18, zero), 0);
    AddPrim(g_gpu_ot_ptr + s.sp2C * 4, arg0[6]);
    arg0[6] += 0xC;
    SetDrawMode(arg0[6], 1, 0, func_8006E480(s.sp18, zero), 0);
    AddPrim(g_gpu_ot_ptr + s.sp2C * 4 - 4, arg0[6]);
    arg0[6] += 0xC;
}

/* Character-select cursor / pick handler; called once per player per frame.
 *   arg0 = this frame's pad bits, both players packed (player N in bits N*16)
 *   arg1 = select page into D_8009BCF8 (2 rows x 5 columns per page)
 *   arg2 = this player's pick list (character ids, -1 = cleared slot)
 *   arg3 = player index (0/1)
 * SELWORK is the shared select work area (per-player pairs); D_8009BCE4[] is
 * the per-character flag byte (bit 0 = selectable, bit 4<<player = already
 * taken by that player). */
void func_80075F80(s32 arg0, s32 arg1, s16 *arg2, s32 arg3) {
    u8 *flag;
    s32 result;
    s32 bit;
    s32 i;

    if (SELWORK->f10.half[arg3] != 0) {
        return;
    }

    if (arg0 & (0x10 << (arg3 * 16))) {
        func_8005C650(2, 0x7F, 0x7F);
        if (SELWORK->f3C[arg3] != 0) {
            arg2[SELWORK->f3C[arg3]] = -1;
            SELWORK->f3C[arg3]--;
            D_8009BCE4[arg2[SELWORK->f3C[arg3]]] &= ~(4 << arg3);
            return;
        }
        /* Neither player has a pick yet.  fold merges the two adjacent halfword
         * tests into the one `lw 0x3C` word test the target has at 0x80076060
         * (tools/gcc-2.7.2/fold-const.c:2687 fold_truthop). */
        if (SELWORK->f3C[0] != 0 || SELWORK->f3C[1] != 0) {
            return;
        }
        if (SELWORK->f14.half[(arg3 != 0) ? 0 : 1] == 2) {
            SELWORK->f10.half[1] = 3;
            SELWORK->f10.half[0] = 3;
            SELWORK->f18[1] = 1;
            SELWORK->f18[0] = 1;
        }
        return;
    }

    result = func_800692C0(&arg0, arg3, SELWORK->f40[arg3], D_800A35D0[arg3]);
    if (arg0 & (0xF000 << (arg3 * 16))) {
        func_8005C650(0, 0x7F, 0x7F);
    }
    {
        s32 low;

        low = result & 0xFF;
        if (low < 3 && low != 0) {
            SELWORK->f1C.half[arg3] = (SELWORK->f1C.half[arg3] + 1) & 1;
        }
    }

    switch (result >> 16) {
    case 1: {
        s16 value;

        value = SELWORK->f20.half[arg3];
        if (value == 4) {
            SELWORK->f20.half[arg3] = 0;
        } else {
            SELWORK->f20.half[arg3] = value + 1;
        }
    } break;
    case 2: {
        s16 value;

        value = SELWORK->f20.half[arg3];
        if (value == 0) {
            SELWORK->f20.half[arg3] = 4;
        } else {
            SELWORK->f20.half[arg3] = value - 1;
        }
    } break;
    }

    {
        u8 entry;

        entry = D_8009BCF8[arg1][SELWORK->f1C.half[arg3] * 5 + SELWORK->f20.half[arg3]].unk0;
        flag = &D_8009BCE4[entry];
        if ((*flag & 1) != 0) {
            bit = 4 << arg3;
            if ((*flag & bit) == 0) {
                u16 count;

                arg2[SELWORK->f3C[arg3]] = entry;
                if (arg0 & (0x40 << (arg3 * 16))) {
                    func_8005C650(1, 0x7F, 0x7F);
                    *flag |= bit;
                    count = SELWORK->f3C[arg3];
                    SELWORK->f3C[arg3] = count + 1;
                    if (SELWORK->f3C[arg3] == SELWORK->f65 + 3) {
                        SELWORK->f10.half[arg3] = 1;
                        SELWORK->f3C[arg3] = count;
                        SELWORK->f38[arg3] = 0;
                        SELWORK->f18[arg3] = 3;
                        for (i = 0; i < SELWORK->f60[arg3]; i++) {
                            SELWORK->f48[arg3][i] = i;
                        }
                        if (arg1 != 0) {
                            SELWORK->f48[arg3][4] = 5;
                        }
                    }
                }
                return;
            }
            arg2[SELWORK->f3C[arg3]] = 0x14;
            if (arg0 & (0x40 << (arg3 * 16))) {
                func_8005C650(4, 0x7F, 0x7F);
            }
        } else {
            /* FAKE: the placeholder tail is written in both arms (duplicated-
             * statement-into-arms, calls per Q47: the store and the call are real on
             * both paths).  Each arm is then reached by one path, so cse carries the
             * work-area address it formed for the cursor read into the 0x3C load;
             * jump2's cross-jump (tools/gcc-2.7.2/jump.c find_cross_jump) merges the two
             * tails back into the target's one copy (`lh v0,0x3C($a1)` at 0x8007630C,
             * 0x14 set in each predecessor).  One shared tail after the if recomputes
             * the address. */
            arg2[SELWORK->f3C[arg3]] = 0x14;
            if (arg0 & (0x40 << (arg3 * 16))) {
                func_8005C650(4, 0x7F, 0x7F);
            }
        }
    }
}
void func_8007636C(s32 *arg0, s32 arg1, s16 *arg2, s32 arg3) {
    S_80074488 s;
    s32 ot;
    s32 *table;
    /* the sprite sheet's cell array (8-byte SprtEntA cells), which starts
       just past the sheet's 12-byte SprtHdrA headers: one header on the
       single-state sheets (+0xC), three (normal, then one cursor highlight
       per player) on the highlightable ones (+0x24). */
    s32 cells;
    s32 color;
    s16 i;
    /* FAKE: constant-holder (named-local-fake-exception) -- the 0 passed as
     * func_8006E480's second argument at both call sites. Set once and live
     * past the first loop, so cse substitutes its pseudo into that loop's
     * `(s16)i < f65 + 3` entry guard (slt needs a register operand) and reload
     * rematerializes it as the target's `move t0,zero; slt` (0x800764A0); a
     * literal 0 lets combine fold the guard to a beqz. The case-2
     * sibling func_800759D0 holds this same argument's zero in $fp (asm lines
     * 20/56/334/356). */
    s32 mode;
    u16 idx;

    mode = 0;
    ot = 10;
    s.sp28 = 0;
    if (arg3 != 0) {
        ot = 20;
    }
    table = *(s32 **)(arg0[0] + 0x30);
    if (SELWORK->f14.half[arg3] < 4) {
        s.sp18 = table[12];
        s.sp40 = 0;
        s.sp30 = arg3 * 240;
        cells = s.sp18 + 0xC;
        s.sp1C = cells;
        s.sp34 = SELWORK->f3C[arg3] * 34;
        s.sp2C = ot;
        s.sp20 = arg0[4];
        arg0[4] = func_8007352C((s32)&s);
        SetDrawMode(arg0[6], 1, 0, func_8006E480(s.sp18, mode), 0);
        AddPrim(g_gpu_ot_ptr + ot * 4, arg0[6]);
        arg0[6] += 0xC;
    }

    s.sp40 = 0;
    color = ((rsin(((SELWORK->f34 & 0x1F) << 7) + 0x1FF) << 5) >> 12) - 0x50;
    s.sp41 = s.sp42 = s.sp43 = color;
    table = *(s32 **)(arg0[0] + 0x14);
    for (i = 0; i < SELWORK->f65 + 3; i++) {
        s.sp40 = 0;
        s.sp18 = table[arg2[i] + 1];
        cells = s.sp18 + 0x24;
        if (SELWORK->f3C[arg3] == i || SELWORK->f14.half[arg3] >= 4) {
            s.sp18 = s.sp18 + 12 + arg3 * 12;
        }
        s.sp1C = cells;
        s.sp1C += *(u8 *)(s.sp18 + 2) * 16;
        s.sp30 = arg3 * 240;
        s.sp34 = i * 34;
        s.sp2C = ot;
        s.sp20 = arg0[4];
        arg0[4] = func_8007352C((s32)&s);
    }

    table = *(s32 **)(arg0[0] + 0x30);
    for (i = 0; i < SELWORK->f3C[arg3] + 1; i++) {
        if (SELWORK->f3C[arg3] != i || SELWORK->f14.half[arg3] >= 4) {
            idx = SELWORK->f7E[arg3][i];
            s.sp40 = 0;
        } else {
            idx = SELWORK->f48[arg3][SELWORK->f5C[arg3]];
            s.sp40 = 1;
        }
        s.sp18 = table[(s16)idx * 2];
        cells = s.sp18 + 0x24;
        if (SELWORK->f3C[arg3] == i || SELWORK->f14.half[arg3] >= 4) {
            s.sp18 = s.sp18 + 12 + arg3 * 12;
        }
        s.sp30 = arg3 * 240;
        s.sp1C = cells;
        s.sp34 = i * 34;
        s.sp2C = ot;
        s.sp20 = arg0[4];
        arg0[4] = func_8007352C((s32)&s);
        s.sp18 = table[(s16)idx * 2 + 1];
        s.sp40 = 0;
        cells = s.sp18 + 0xC;
        s.sp1C = cells;
        s.sp20 = arg0[4];
        arg0[4] = func_8007352C((s32)&s);
    }

    table = *(s32 **)(arg0[0] + SELWORK->f65 * 4 + 0x20);
    for (i = 0; i < SELWORK->f65 + 3; i++) {
        s.sp40 = 0;
        s.sp18 = table[i];
        cells = s.sp18 + 0x24;
        if (SELWORK->f3C[arg3] == i || SELWORK->f14.half[arg3] >= 4) {
            s.sp18 = s.sp18 + 12 + arg3 * 12;
        }
        s.sp1C = cells;
        s.sp1C += *(u8 *)(s.sp18 + 2) * 8;
        s.sp30 = arg3 * 240;
        s.sp34 = i * 34;
        s.sp20 = arg0[4];
        arg0[4] = func_8007352C((s32)&s);
    }

    table = *(s32 **)(arg0[0] + 0x14);
    s.sp18 = table[1];
    SetDrawMode(arg0[6], 1, 0, func_8006E480(s.sp18, mode), 0);
    AddPrim(g_gpu_ot_ptr + ot * 4, arg0[6]);
    arg0[6] += 0xC;
}

void func_800768DC(s32 arg0, s32 arg1, s16 *arg2, s32 arg3) {
    s16 i;
    s16 j;

    if (SELWORK->f10.half[arg3] != 0) {
        return;
    }
    if (arg0 & (0xA000 << (arg3 * 16))) {
        func_8005C650(0, 0x7F, 0x7F);
    }
    switch (func_800692C0((u32 *)&arg0, arg3, SELWORK->f40[arg3], D_800A35D0[arg3]) & 0xFF) {
    case 1:
        SELWORK->f5C[arg3]++;
        if (SELWORK->f5C[arg3] >= SELWORK->f60[arg3]) {
            SELWORK->f5C[arg3] = 0;
        }
        break;
    case 2:
        SELWORK->f5C[arg3]--;
        if (SELWORK->f5C[arg3] < 0) {
            SELWORK->f5C[arg3] = SELWORK->f60[arg3] - 1;
        }
        break;
    }

    if (arg0 & (0x40 << (arg3 * 16))) {
        if (SELWORK->f3C[arg3] < SELWORK->f65 + 3) {
            func_8005C650(1, 0x7F, 0x7F);
            SELWORK->f7E[arg3][SELWORK->f3C[arg3]] = SELWORK->f48[arg3][SELWORK->f5C[arg3]];
            if (SELWORK->f3C[arg3] == SELWORK->f65 + 2) {
                SELWORK->f14.half[arg3] = 4;
                return;
            }
            SELWORK->f60[arg3]--;
            for (i = SELWORK->f5C[arg3]; i < SELWORK->f60[arg3]; i++) {
                SELWORK->f48[arg3][i] = SELWORK->f48[arg3][i + 1];
            }
            SELWORK->f5C[arg3] = 0;
            SELWORK->f3C[arg3]++;
        }
    } else if (arg0 & (0x10 << (arg3 * 16))) {
        func_8005C650(2, 0x7F, 0x7F);
        if (SELWORK->f3C[arg3] == 0) {
            SELWORK->f10.half[arg3] = 3;
            SELWORK->f18[arg3] = 2;
            SELWORK->f38[arg3] = SELWORK->f65 + 2;
            D_8009BCE4[arg2[SELWORK->f38[arg3]]] &= ~(4 << arg3);
            SELWORK->f60[arg3] = 5;
            for (i = 0; i < SELWORK->f60[arg3]; i++) {
                SELWORK->f48[arg3][i] = i;
            }
            if (arg1 != 0) {
                SELWORK->f48[arg3][4] = 5;
            }
            return;
        }
        /* Put the last taken entry back into the list in sorted position. */
        for (i = 0; i < SELWORK->f60[arg3]; i++) {
            if (SELWORK->f48[arg3][i] > SELWORK->f7E[arg3][SELWORK->f3C[arg3] - 1]) {
                for (j = SELWORK->f60[arg3] - 1; j >= i; j--) {
                    SELWORK->f48[arg3][j + 1] = SELWORK->f48[arg3][j];
                }
                SELWORK->f48[arg3][i] = SELWORK->f7E[arg3][SELWORK->f3C[arg3] - 1];
                break;
            }
            if (i == SELWORK->f60[arg3] - 1) {
                SELWORK->f48[arg3][i + 1] = SELWORK->f7E[arg3][SELWORK->f3C[arg3] - 1];
            }
        }
        SELWORK->f60[arg3]++;
        SELWORK->f3C[arg3]--;
    }
}
/* func_80076D74: select-screen fade-out.  Raises SELWORK->f36 by 8 per frame
 * (capped at 0xFF) and, once it reaches 0xFF, fills the result record at
 * SELWORK->f00 (f65, f66, f67, f68 packed into bitfields; per player and pick,
 * the picked entry's D_8009BCF8 column 1 and its f7E value).  Every frame it
 * draws the full-screen TILE with f36 as its colour. */
typedef struct {
    u8 cells[2][5][2];  /* 0x00: [row][col][{glyph, attr}] */
    u32 pad10 : 10;     /* 0x14 */
    u32 f10 : 2;
    u32 f12 : 2;
    u32 f14 : 1;
    u32 f15 : 2;
} S_80076D74;

s32 func_80076D74(s32 *arg0) {
    u8 *p;
    S_80076D74 *hdr;
    u16 *cnt;
    s16 v;
    s16 i;
    s16 j;
    s32 sel;
    s32 ret;

    ret = 0;
    cnt = &SELWORK->f36;
    v = *cnt + 8;
    *cnt = v;
    if (v >= 0xFF) {
        *cnt = 0xFF;
        hdr = SELWORK->f00;
        hdr->f10 = SELWORK->f65;
        ret = 1;
        if (SELWORK->f66 < 3) {
            sel = SELWORK->f66 - 1;
        } else {
            sel = SELWORK->f66 - 2;
        }
        hdr->f12 = sel;
        hdr->f14 = SELWORK->f67;
        hdr->f15 = SELWORK->f68[0] + SELWORK->f68[1] * 2;
        for (i = 0; i < 2; i++) {
            for (j = 0; j < SELWORK->f65 + 3; j++) {
                /* flat character index into both pages, through row 0. */
                /* SOTN: src/st/e_grave_keeper.h:534 @aa53500 */
                hdr->cells[i][j][0] = D_8009BCF8[0][SELWORK->f6A[i][j]].unk1;
                hdr->cells[i][j][1] = SELWORK->f7E[i][j];
            }
        }
    }
    p = (u8 *)arg0[5];
    SetTile((TILE *)p);
    *(u8 *)(p + 4) = *cnt;
    *(u8 *)(p + 5) = *cnt;
    *(u8 *)(p + 6) = *cnt;
    *(s16 *)(p + 8) = 0;
    *(s16 *)(p + 0xA) = 0;
    *(s16 *)(p + 0xC) = 0x280;
    *(s16 *)(p + 0xE) = 0xF0;
    SetSemiTrans((GameObj *)p, 1);
    AddPrim(g_gpu_ot_ptr, (GameObj *)p);
    p += 0x10;
    arg0[5] = (s32)p;
    SetDrawMode(arg0[6], 1, 0, 0x40, 0);
    AddPrim(g_gpu_ot_ptr, (GameObj *)arg0[6]);
    do { /* FAKE: do-while(0) wrap, loop-end note pins the return copy after the sw so the increment temp takes v0; mechanism: sched.c loop_notes dependence on the first insn after NOTE_INSN_LOOP_END */
        arg0[6] += 0xC;
    } while (0);
    return ret;
}
s32 func_80076FF8(s32 *a0) {
    func_8006920C(a0, a0[5]);
    func_8006920C(a0, a0[6]);
    func_8006920C(a0, a0[7]);
    func_8006920C(a0, a0[8]);
    func_8006920C(a0, a0[9]);
    func_8006920C(a0, a0[10]);
    func_8006920C(a0, a0[11]);
    func_8006920C(a0, a0[12]);
    func_8006920C(a0, a0[13]);
    func_8006920C(a0, a0[14]);
    return a0[1];
}
s32 func_80077098(s32 a0) {
    return D_800A35D8 + a0 * 44;
}


extern s32 func_80076FF8(s32 *);

s32 func_800770B8(s32 arg0, s32 arg1, s32 arg2) {
    s16 sp[2];
    /* work holds two values: the entry list pointer (arg0 + 0x58, passed to func_8006E950 /
       func_80076FF8 and stored as the work area's f04), then the work area func_8006E49C returns
       (stored to D_800A36A0). Owner Ruling 11 with owner ruling Q78. */
    void *work;
    s32 r;
    s16 t0;
    s16 a2;

    /* FAKE: empty do-while(0) wrap (do-while-zero-exception). Effect: its NOTE_INSN_LOOP_BEG/END
       pair bounds sched2's region at this point, so the five frame-save stores are not interleaved
       with the first body insns as in the target's prologue order. */
    do { } while (0);
    sp[0] = 0;
    sp[1] = 0;
    ClearOTagR(g_gpu_ot_ptr, 0x1008);
    work = (void *)(arg0 + 0x58);
    D_800A35D8 = arg0;
    snd_StopAll();
    func_8006E950(6, work);
    r = func_80076FF8(work);
    {
        s32 *list = work;
        work = (void *)func_8006E49C(r, (s32 *)D_800A35D8);
        D_800A36A0 = work;
        SELWORK->f04 = list;
        /* FAKE: dead store (dead-store-fake-exception; owner ruling Q78): work's
           restored value is never read. Effect: cse.c make_regs_eqv puts work and the call's $v0 in
           one quantity with work canonical, so the D_800A36A0 reloads for the 0x30/0x34 clears below
           would become work ($s1); this store takes work out of that class first (cse.c
           delete_reg_equiv), the reloads resolve to the f04 store's reload copy of the call result
           instead, and the clears use $v0 as in the target (0x80077144/48). */
        work = list;
        SELWORK->f30 = 0;
        SELWORK->f34 = 0;
    }
    t0 = 0;
    do {
        SelWork *base = SELWORK;
        /* FAKE: typed row pointer to D_800A35D0 (pointer-alias-fake-exception, the
           `s16 (*p)[] = &D_xxx` re-view), kept so the same-value re-set below has a pseudo to re-set
           (loop.c keeps its lui/addiu inside the outer loop). */
        s16 (*row)[2];
        a2 = 0;
        base->f10.half[t0] = 0;
        base->f08[t0] = 0;
        base->f0C[t0] = 0;
        base->f14.half[t0] = 0;
        base->f3C[t0] = 0;
        row = D_800A35D0;
        row[t0][1] = 0;
        row[t0][0] = 0;
        base->f40[t0][1] = 0;
        base->f40[t0][0] = 0;
        base->f68[t0] = t0;
        do {
            SELWORK->f6A[t0][a2] = -1;
            SELWORK->f7E[t0][a2] = 0;
            a2 = (s16)(a2 + 1);
        } while (a2 < 5);
        SELWORK->f5C[t0] = 0;
        SELWORK->f60[t0] = 5;
        for (a2 = 0; a2 < 0xA; a2 = (s16)(a2 + 1)) {
            s16 idx = (s16)(a2 + (t0 * 10));
            s32 mask = 1 << idx;
            D_8009BCE4[idx] = (u8)(D_8009BCE4[idx] & 0xF2);
            if ((arg2 & mask) != 0) {
                D_8009BCE4[idx] = (u8)(D_8009BCE4[idx] | 1);
                sp[t0] += 1;
                /* FAKE: same-value dead store (dead-store-fake-exception): row is not read after
                   this. Effect: a second set of row's pseudo in another basic block makes loop.c
                   count_loop_regs_set mark it may_not_move (loop.c:3040-3041), so the lui/addiu of
                   D_800A35D0 stays inside the outer loop where the target builds it instead of
                   being hoisted to the pre-header. */
                row = D_800A35D0;
            }
        }
        t0 = (s16)(t0 + 1);
    } while (t0 < 2);
    {
        SelWork *p = SELWORK;
        p->f20.word = 0;
        p->f1C.word = 0;
        if (sp[0] < sp[1]) {
            p->f64 = sp[0] - 3;
        } else {
            p->f64 = sp[1] - 3;
        }
    }
    if (SELWORK->f64 >= 3) {
        SELWORK->f64 = 2;
    }
    {
        SelWork *q = SELWORK;
        q->f00 = arg1;
        q->f65 = 0;
    }
    SELWORK->f67 = 1;
    SELWORK->f66 = D_8009BD20[SELWORK->f67][1];
    D_800A35DC = 1;
    return 1;
}

/* Tail word after func_800747D8's five-entry compiler-generated switch table. */
const u32 D_80015A20[1] = { 0x00000000 };

s32 func_80077374(s32 arg0, s32 *arg1) {
    s32 ret;
    s16 i;

    ret = 0;
    if (SELWORK->f14.word == 0x40004) {
        SELWORK->f14.half[1] = 5;
        SELWORK->f14.half[0] = 5;
        SELWORK->f36 = 0;
    }
    func_80074220(arg1, SELWORK->f14.half[0]);
    if (SELWORK->f14.half[0] != 5) {
        func_8007526C();
    }

    for (i = 0; i < 2; i++) {
        switch (SELWORK->f14.half[i]) {
        case 0:
            if (i == 0) {
                ret = func_800747D8(arg0);
                func_80074488(arg1);
            }
            break;
        case 1:
            func_80075670(arg0, i);
            func_80074D2C((s32)arg1, i,
                          (s16)(SELWORK->f14.half[i] - 1));
            func_800753D8(arg1, i);
            func_80074E08(arg1, i);
            break;
        case 2:
            func_80075F80(arg0, SELWORK->f68[i],
                          SELWORK->f6A[i], i);
            func_80074D2C((s32)arg1, i,
                          (s16)(SELWORK->f14.half[i] - 1));
            func_80074B18(arg1, i, 0);
            func_800759D0(arg1, SELWORK->f68[i],
                          SELWORK->f6A[i], i);
            func_80074E08(arg1, i);
            break;
        case 3:
            func_800768DC(arg0, SELWORK->f68[i],
                          SELWORK->f6A[i], i);
            func_80074D2C((s32)arg1, i,
                          (s16)(SELWORK->f14.half[i] - 1));
            func_80074B18(arg1, i, 1);
            func_8007636C(arg1, SELWORK->f68[i],
                          SELWORK->f6A[i], i);
            func_80074E08(arg1, i);
            break;
        case 4:
            func_80074D2C((s32)arg1, i, 2);
            func_80074B18(arg1, i, 1);
            func_8007636C(arg1, SELWORK->f68[i],
                          SELWORK->f6A[i], i);
            func_80074E08(arg1, i);
            if (arg0 & (0x10 << (i * 16))) {
                func_8005C650(2, 0x7F, 0x7F);
                SELWORK->f14.half[i] = 3;
            }
            break;
        case 5:
            func_80074D2C((s32)arg1, i, 2);
            func_80074B18(arg1, i, 1);
            func_8007636C(arg1, SELWORK->f68[i],
                          SELWORK->f6A[i], i);
            ret = func_80076D74(arg1);
            func_80074E08(arg1, i);
            break;
        }
    }
    return ret;
}

typedef struct {
    s32 sp10;
    s32 sp14;
    s32 sp18;
    s32 sp1C;
    s32 sp20;
    s32 sp24;
    s32 sp28;
    s32 sp2C;
    s32 sp30;
    s32 sp34;
} S7724;
void func_80077724(s32 arg0, s32 arg1) {
    S7724 s;
    s32 *p;
    s32 temp_v1;
    SELWORK->f24 = &g_gpu_db[D_800A36AC & 1];
    temp_v1 = SELWORK->f30 + 1;
    SELWORK->f34 = SELWORK->f34 + 1;
    SELWORK->f30 = temp_v1;
    p = func_80077098(temp_v1 & 1);
    SELWORK->f2C = p;
    s.sp10 = (s32)SELWORK->f04;
    s.sp14 = p[0];
    s.sp18 = p[1];
    s.sp1C = p[2];
    s.sp20 = p[4];
    s.sp24 = p[3];
    s.sp28 = p[5];
    s.sp2C = p[6];
    s.sp30 = p[7];
    s.sp34 = p[8];
    func_80077374(arg1, &s.sp10);
}


s32 func_80077820(s32 a0) {
    func_80068F70(a0, (s32 *)&D_8009BD24);
    gpu_SetDrawEnvBg(1, 0, 0, 0);
    D_800A35E4 = 0;
    return 1;
}


s32 func_80077860(void) {
    if (((s32 (*)())func_80069250)() == 1) {
        D_800A35E4 = 0;
        return 1;
    }
    return 0;
}
s32 func_80077894(s32 held, s32 pressed) {
    s32 ret;
    s32 result;

    ret = 0;
    result = func_800693CC(held, pressed);
    if (result >= 0) {
        ret = 1;
        D_800A35E4 = 0;
        D_8009BD38.unk0 = result;
    } else if (result == -2) {
        ret = -1;
    }
    return ret;
}
s32 func_80077904(void) {
    D_800A35E4 = 0;
    D_800A35E0 = D_8009BD58[D_8009BD38.unk0][1];
    return D_8009BD58[D_8009BD38.unk0][0];
}
void func_80077940(s32 arg0) {
    D_800A35E8 = (arg0 & 0x3FF) + ((u32) (arg0 & 0x3FF000) >> 2) + ((u32) (arg0 & 0x01000000) >> 4) + ((u32) (arg0 & 0x04000000) >> 5);
}

s32 func_80077984(s32 a0) {
    func_8006E534(a0, D_800A35E0, &D_8009BD24[0][0].chr, D_800A35E8);
    gpu_SetDrawEnvBg(1, 0, 0, 0);
    return 1;
}

s32 func_800779C8(void) {
    s32 ret = ((s32 (*)())func_8006EACC)();
    if (ret) {
        snd_CloseVab1();
    }
    return ret;
}

void func_80077A04(s32 a0, s32 a1) {
    D_800A35E4 = 0;
    func_8006D74C(a0, a1);
}
void func_80077A28(void) {
    D_800A35E4 = 0;
    gpu_SetDrawEnvBg(1, 0, 0, 0);
    func_8006D7FC();
}

void func_80077A60(void) {
    ((void (*)())func_8006E068)();
}

s32 func_800770B8(s32, s32, s32);
s32 func_80077A80(s32 a0) {
    func_800770B8(a0, (s32)&D_8009BD24, D_800A35E8);
    gpu_SetDrawEnvBg(1, 0, 0, 0);
    return 1;
}


void func_80077AC0(void) {
    ((void (*)())func_80077724)();
}

void func_80077AE0(void) {
    func_8006E10C();
}

void func_80077B00(void) {
    func_8006E2A8();
}
void func_80077B20(void) {
    D_800A35E4 = 1;
}

/* ---- merged from text1b_b.c (owner ruling Q67: one original file) ---- */
#define NULL ((void *)0)

typedef struct Vec3s16 { s16 x; s16 y; s16 z; } Vec3s16;
typedef struct Vec3s32 { s32 x; s32 y; s32 z; } Vec3s32;
typedef struct Vec3 { s32 vx, vy, vz, pad; } Vec3;

extern u8 D_8009BA60;
extern s32 chractar_use_pset_combo_id_table;
























































































































extern s32 D_800F10D0;

















extern s32 column;











































































    extern s32 rand(void);

    extern void func_80061FAC(s32, s32, s32);



















































    extern s32 D_800A3724;











































































































extern void func_8006E950(s32, s32 *);

extern s32 func_8006E49C(s32, s32 *);







/* --- Functions from text1b segment (0x80047ED0 - 0x80079A30) --- */

extern u8 D_8009BD3B;
extern u8 D_8009BD3C;
extern u8 D_8009BD3D;
extern u8 D_8009BD41;
extern u8 D_8009BD42;
extern u8 D_8009BD43;



s32 func_80077B30(s32 arg0, s32 arg1) {
    extern s32 func_8006B898(s32, s32);
    extern s32 func_8006D338(s32, s32);
    s32 s2;
    s32 result;

    if ((u32)D_800A35E4 >= 6) goto end;
    switch (D_800A35E4) {
    case 0:
        s2 = 0;
        result = func_8006B898(arg0, arg1);
        switch (result) {
        case 1: s2 = -1; goto end;
        case 2: D_800A35E4 = 1; goto end;
        case 3:
            D_8009BD3B = D_8009BD41;
            D_8009BD3C = D_8009BD42;
            D_8009BD3D = D_8009BD43;
            D_800A35E4 = 4;
            goto end;
        }
        goto end;
    case 1:
        s2 = 0;
        result = func_8006C1FC(arg0, arg1);
        if (result == 1) { D_800A35E4 = 0; goto end; }
        if (result == 2) { D_800A35E4 = result; goto end; }
        if (result == 3) { D_800A35E4 = result; }
        goto end;
    case 2:
        s2 = 2;
        goto end;
    case 3:
        s2 = 3;
        goto end;
    case 4:
        func_8006D324();
        D_800A35E4 = D_800A35E4 + 1;
        /* fall through */
    case 5:
        s2 = 0;
        result = func_8006D338(arg0, arg1);
        if (result == 1) {
            D_800A35E4 = 0;
            D_8009BD41 = D_8009BD3B;
            D_8009BD42 = D_8009BD3C;
            D_8009BD43 = D_8009BD3D;
            goto end;
        }
        if (result == -1) { D_800A35E4 = 0; }
        goto end;
    }
end:
    return s2;
}
s32* func_80077D00(void) {
    return (s32 *)D_8009BD24;
}
s32 func_80077D10(s32 *a0) {
    func_8006920C(a0, a0[6]);
    func_8006920C(a0, a0[7]);
    func_8006920C(a0, a0[8]);
    func_8006920C(a0, a0[9]);
    func_8006920C(a0, a0[10]);
    return a0[1];
}
s32 func_80077D74(s32 a0) {
    return D_800A35F4 + a0 * 44;
}
extern void LoadImage(s32, s32);

typedef struct {
    s16 x, y, w, h;
} Rect77D94;
extern Rect77D94 D_800A32FC;

/* 0x2C-byte draw descriptor func_8007352C consumes (EnvA layout). */
typedef struct {
    s32 header;
    s32 table;
    s32 out;
    s32 pad0C;
    s32 semi;
    s32 ot_idx;
    s32 x;
    s32 y;
    s32 pad20, pad24;
    u8 has_color;
    u8 col_r;
    u8 col_g;
    u8 col_b;
} Env77D94;

typedef struct {
    s16 on, off;
} Win77D94;

typedef struct {
    u8 pad00[0x14];
    s32 table;
    s32 *hdr18;
    s32 *hdr1C;
    s32 *hdr20;
    s32 *hdr24;
    s32 *hdr28;
    Win77D94 *win2C;
    s16 *in30;
    s16 *out34;
    s32 img38[5];
} Ctx77D94;

void func_80077D94(s32 *arg0) {
    Env77D94 s;
    Rect77D94 rect;
    s16 *in;
    s16 *out;
    s32 table;
    /* FAKE: constant-holder local (named-local-fake-exception). abr is the
       tpage blend-mode bits func_8006E480 adds to every sprite's tpage.
       Mechanism: a pseudo live across the draw calls is seated once in
       callee-saved $s7 (`addiu s7,zero,0x20` in the prologue branch's
       delay slot, then `addu a1,s7,zero` at all 5 call sites). The inline
       literal re-materializes `li a1,0x20` per call.
       Same shape as func_80078654's `zero` and func_80070C70's `c60`. */
    s32 abr;
    s32 *hp;
    s32 i;
    s32 j;
    s32 x;
    s32 v;
    Win77D94 *w;

    s.ot_idx = 2;
    s.y = 0x1E;
    s.semi = 0;
    in = ((Ctx77D94 *)D_800A35F8)->in30;
    table = ((Ctx77D94 *)D_800A35F8)->table;
    out = ((Ctx77D94 *)D_800A35F8)->out34;
    abr = 0x20;
    /* FAKE (duplicated-statement-into-arms): both fade arms store their own
       has_color/r/g/b; the compiler cross-jumps the identical tails. The
       shared-tail spelling (arms set v only, the else skips the stores with
       a goto) does not match. */
    if (D_800A35F0 < in[D_800A3600] + 60 && D_800A35F0 >= in[D_800A3600]) {
        v = ((D_800A35F0 - in[D_800A3600]) << 7) / 60;
        s.has_color = 1;
        s.col_r = s.col_g = s.col_b = v;
    } else if (D_800A35F0 >= out[D_800A3600] && D_800A35F0 < out[D_800A3600] + 60) {
        v = ((60 - (D_800A35F0 - out[D_800A3600])) << 7) / 60;
        s.has_color = 1;
        s.col_r = s.col_g = s.col_b = v;
    } else {
        s.has_color = 0;
    }

    if (D_800A35F0 < out[5] + 60) {
        switch (D_800A3600) {
        case 0:
            if (D_800A35F0 + 1 >= out[D_800A3600] + 60) {
                if (D_800A35F0 + 1 >= in[D_800A3600 + 1]) {
                    D_800A3600 = D_800A3600 + 1;
                }
            } else if (D_800A35F0 >= in[D_800A3600]) {
                hp = ((Ctx77D94 *)D_800A35F8)->hdr18;
                for (i = 0; i < 5; hp++, i++) {
                    s.header = *hp;
                    s.table = table;
                    s.x = i << 7;
                    s.out = arg0[4];
                    arg0[4] = func_8007352C((s32)&s.header);
                    SetDrawMode(arg0[6], 1, 0, func_8006E480(s.header, abr), 0);
                    AddPrim(g_gpu_ot_ptr + s.ot_idx * 4, arg0[6]);
                    arg0[6] += 0xC;
                }
            }
            break;
        case 2:
            if (D_800A35F0 + 1 >= out[D_800A3600] + 60) {
                if (D_800A35F0 + 1 >= in[D_800A3600 + 1]) {
                    D_800A3600 = D_800A3600 + 1;
                }
                break;
            }
            hp = ((Ctx77D94 *)D_800A35F8)->hdr24;
            for (i = 0, x = 0x156; i < 2; x += 0x80, hp++, i++) {
                s.header = *hp;
                s.x = x;
                s.table = table;
                s.out = arg0[4];
                arg0[4] = func_8007352C((s32)&s.header);
                SetDrawMode(arg0[6], 1, 0, func_8006E480(s.header, abr), 0);
                AddPrim(g_gpu_ot_ptr + s.ot_idx * 4, arg0[6]);
                arg0[6] += 0xC;
            }
            /* fall through */
        case 1:
            if (D_800A35F0 + 1 >= in[D_800A3600 + 1] && D_800A3600 == 1) {
                D_800A3600 = 2;
            }
            if (D_800A35F0 < out[2] && D_800A35F0 >= in[2]) {
                s.has_color = 0;
            }
            hp = ((Ctx77D94 *)D_800A35F8)->hdr20;
            for (i = 0, x = 0x2B; i < 2; x += 0x80, hp++, i++) {
                s.header = *hp;
                s.x = x;
                s.table = table;
                s.out = arg0[4];
                arg0[4] = func_8007352C((s32)&s.header);
                SetDrawMode(arg0[6], 1, 0, func_8006E480(s.header, abr), 0);
                AddPrim(g_gpu_ot_ptr + s.ot_idx * 4, arg0[6]);
                arg0[6] += 0xC;
            }
            break;
        case 3:
            rect = D_800A32FC;
            for (i = 0; i < 5; i++) {
                /* FAKE: named intermediate for the TIM-pointer slot offset
                   (0x38 + 4i). Mechanism: loop.c strength-reduces it to
                   the target's giv ($s1 = 0x38, += 4). Written inline
                   (`((Ctx77D94 *)D_800A35F8)->img38[i]` or
                   `D_800A35F8 + i * 4 + 0x38`), fold moves 0x38 into the
                   load displacement and the giv is not reduced. */
                j = i * 4 + 0x38;
                LoadImage((s32)&rect, *(s32 *)(D_800A35F8 + j) + 0x220);
                DrawSync(0);
                rect.x += 0x40;
            }
            D_800A3600 = D_800A3600 + 1;
            break;
        case 4:
            if (D_800A35F0 + 1 < out[D_800A3600] + 60) {
                hp = ((Ctx77D94 *)D_800A35F8)->hdr1C;
                for (i = 0; i < 5; hp++, i++) {
                    s.header = *hp;
                    s.table = table;
                    s.x = i << 7;
                    s.out = arg0[4];
                    arg0[4] = func_8007352C((s32)&s.header);
                    SetDrawMode(arg0[6], 1, 0, func_8006E480(s.header, abr), 0);
                    AddPrim(g_gpu_ot_ptr + s.ot_idx * 4, arg0[6]);
                    arg0[6] += 0xC;
                }
            }
            break;
        }
    }

    s.ot_idx = 1;
    s.y = 0;
    s.x = 0;
    hp = ((Ctx77D94 *)D_800A35F8)->hdr28;
    for (i = 0; i < 21; i++) {
        w = (Win77D94 *)(i * 4 + (s32)((Ctx77D94 *)D_800A35F8)->win2C);
        if (D_800A35F0 < w->off + 60 && D_800A35F0 >= w->on) {
            /* FAKE (duplicated-statement-into-arms): each arm stores its own
               r/g/b chain; the compiler cross-jumps the identical tails back
               into one. One shared chain after the if/else puts the value in
               a separate pseudo and costs a `move` at the join. */
            if (D_800A35F0 < w->on + 60) {
                s.has_color = 1;
                v = ((D_800A35F0 - w->on) * 112) / 60;
                s.semi = 1;
                s.col_r = s.col_g = s.col_b = v;
            } else if (D_800A35F0 >= w->off && D_800A35F0 < w->off + 60) {
                /* FAKE: named intermediate for the frame 60 ticks back.
                   Mechanism: inline, fold reassociates off - (cnt - 60)
                   into (off + 60) - cnt; the target computes cnt - 60
                   first (`addiu v0,a1,-60`); neither the inline form nor
                   60 - (cnt - off) matches. */
                s32 t = D_800A35F0 - 60;
                s.has_color = 1;
                v = ((w->off - t) * 112) / 60;
                s.semi = 1;
                s.col_r = s.col_g = s.col_b = v;
            } else {
                s.has_color = 1;
                v = 0x72;
                s.semi = 1;
                s.col_r = s.col_g = s.col_b = v;
            }
            s.header = hp[i];
            s.table = s.header + 0xC;
            s.out = arg0[4];
            arg0[4] = func_8007352C((s32)&s.header);
            SetDrawMode(arg0[6], 1, 0, func_8006E480(s.header, abr), 0);
            AddPrim(g_gpu_ot_ptr + s.ot_idx * 4, arg0[6]);
            arg0[6] += 0xC;
        }
    }
}
s32 func_800784E4(s32 arg0) {
    s32 s0;
    s32 r;

    ClearOTagR(g_gpu_ot_ptr, 0x1008);
    s0 = arg0 + 0x58;
    D_800A35F4 = arg0;
    D_800A35F8 = s0;
    func_8006E950(0x32, s0);
    r = func_80077D10(s0);
    func_8006E49C(r, (s32 *)D_800A35F4);
    D_800A35FC = 0;
    D_800A35F0 = 0;
    D_800A3600 = 0;
    return 1;
}

extern void func_80077D94(s32 *);

typedef struct {
    s32 sp10;
    s32 sp14;
    s32 sp18;
    s32 sp1C;
    s32 sp20;
    s32 sp24;
    s32 sp28;
    s32 sp2C;
    s32 sp30;
} S855C;
s32 func_8007855C(s32 arg0) {
    S855C s;
    s32 *p;
    s32 c;
    c = D_800A35FC + 1;
    D_800A35FC = c;
    p = func_80077D74(c & 1);
    s.sp14 = p[0];
    s.sp1C = p[2];
    s.sp20 = p[4];
    s.sp24 = p[3];
    s.sp28 = p[5];
    s.sp2C = p[6];
    s.sp30 = p[7];
    if (D_800A35F0 > 0 && (arg0 & 0x40)) {
        return 1;
    }
    func_80077D94(&s.sp10);
    {
        s32 *p_struct = *(s32 **)((s32)D_800A35F8 + 0x34);
        s32 new_val = D_800A35F0 + 1;
        int cond = new_val < *(s16 *)((s32)p_struct + 0xA);
        D_800A35F0 = new_val;
        return cond ? 0 : 1;
    }
}
s32 func_80078628(s32 *a0) {
    return a0[1];
}
s32 func_80078634(s32 a0) {
    return D_800A360C + a0 * 44;
}

typedef struct {
    s32 a;       /* sp18 - 0x00 */
    s32 b;       /* sp1C - 0x04 */
    s32 c;       /* sp20 - 0x08 */
    s32 d;       /* sp24 - 0x0C - unused */
    s32 e;       /* sp28 - 0x10 */
    s32 f;       /* sp2C - 0x14 */
    s32 g;       /* sp30 - 0x18 */
    s32 h;       /* sp34 - 0x1C */
    s32 i;       /* sp38 - 0x20 - unused */
    s32 j;       /* sp3C - 0x24 - unused */
    u8 cd_flag;  /* sp40 - 0x28 */
    u8 r;        /* sp41 - 0x29 */
    u8 g_;       /* sp42 - 0x2A */
    u8 b_;       /* sp43 - 0x2B */
} S78654;

void func_80078654(s32 *arg0) {
    S78654 s;
    s32 *var_s0;
    /* FAKE: constant-holder local, kept live across the SetDrawMode /
       func_8006E480 / AddPrim call sequence so the 0 argument comes out of a
       register instead of being re-materialized at each use.  Mechanism:
       local-alloc/global-alloc seat the constant in a call-saved quantity;
       with the literal 0 the function comes out 3 instructions short of
       the target. */
    s32 zero;

    zero = 0;
    s.f = 2;
    s.cd_flag = 0;
    s.e = 0;
    s.g = 0;
    s.a = D_800A3610[0xF];
    s.h = 0;
    s.b = s.a + 0xC;
    var_s0 = D_800A3610 + 5;
    if (D_800A3608 >= 0xAAA) {
        if (D_800A3608 >= 0xB04) {
            s16 sv;
            s.cd_flag = 1;
            sv = 0x80 - (((D_800A3608 - 0xB04) << 7) / 15);
            if (sv < 0) {
                sv = 0;
            }
            s.r = (s.g_ = (s.b_ = (u8) sv));
        }
        s.c = arg0[3];
        arg0[3] = func_8007352C((s32)&s.a);
        SetDrawMode(arg0[5], 1, 0, func_8006E480(s.a, zero), 0);
        AddPrim(g_gpu_ot_ptr + (s.f * 4), arg0[5]);
        arg0[5] = arg0[5] + 0xC;
    }
    s.cd_flag = 0;
    goto check;
loop:
    /* FAKE: 8-deep do-while(0) wrap around the walk-pointer read block.
       Effect: seats the table-walk pointer var_s0 in $s0 and the parameter
       arg0 in $s1 (loop-note reference weighting -> allocno priority).
       Mechanism: flow.c:2081 counts each register mention as loop_depth
       references, so the wrap multiplies var_s0's reg_n_refs without
       emitting an instruction; global.c's allocno priority then ranks
       var_s0 (13 refs / 91 live, pri 4285) above arg0 (13 / 98, pri 3979).
       A single level is insufficient: depth 1 yields 6 of the 13
       references the priority inversion requires; depth 8 is the minimum
       that reaches 13 at the only wrap site that costs no delay slot. */
    do { do { do { do { do { do { do { do {
    s.a = var_s0[0];
    s.b = s.a + 0xC;
    s.h = -D_800A3608;
    } while (0); } while (0); } while (0); } while (0); } while (0); } while (0); } while (0); } while (0);
    s.c = arg0[3];
    arg0[3] = func_8007352C((s32)&s.a);
    SetDrawMode(arg0[5], 1, 0, func_8006E480(s.a, zero), 0);
    AddPrim(g_gpu_ot_ptr + (s.f * 4), arg0[5]);
    var_s0++;
    arg0[5] = arg0[5] + 0xC;
check:
    if (var_s0[1] != -1) goto loop;
}

extern s32 D_800A3304;

s32 func_80078824(s32 arg0) {
    s32 s0;
    s32 r;

    ClearOTagR(g_gpu_ot_ptr, 0x1008);
    s0 = arg0 + 0x58;
    D_800A360C = arg0;
    D_800A3610 = s0;
    func_8006E950(0x5F, s0);
    r = func_80078628(s0);
    func_8006E49C(r, (s32 *)D_800A360C);
    D_800A3304 = 0;
    D_800A3608 = 0;
    D_800A3614 = 0;
    gpu_SetDrawEnvBg(1, 0, 0, 0);
    return 1;
}

void func_80078654(s32 *);
s32 func_800788B0(void) {
    s32 buf[8];
    s32 *v0;
    D_800A3304++;
    v0 = func_80078634(D_800A3304 & 1);
    buf[0] = v0[0];
    buf[2] = v0[2];
    buf[3] = v0[4];
    buf[4] = v0[3];
    buf[5] = v0[5];
    buf[6] = v0[6];
    buf[7] = v0[7];
    func_80078654(buf);
    D_800A3608++;
    return D_800A3608 >= 0xB40;
}

/* Q65: this file's initialized small data (.sdata), in address order; values from the original EXE. */
s32 D_800A3304 = 0;
/* Q65: tentative definitions (COMMON) of the small data this file reaches gp-relative. */
u8 * D_800A36A0;
