#define INCLUDE_ASM_USE_MACRO_INC 1
#include "common.h"
#include "include_asm.h"
#include "sound.h"
#include "game.h"
#include "code6cac.h"
#include "gpu.h"

/* Declarations from the file this TU was split from (text1b.c). */
extern s32 ClearOTagR(s32, s32);
extern s32 D_800A36AC;
extern s32 rsin();
extern s32 g_gpu_ot_ptr;
typedef struct {
    u8 chr;
    u8 unk1;
} Unk8009BD24Record;
extern Unk8009BD24Record D_8009BD24[2][5];
typedef struct {
    u32 unk0 : 4;
    u32 unk4 : 6;
    u32 unk10 : 2;
    u32 unk12 : 2;
    u32 unk14 : 1;
    u32 unk15 : 2;
    u32 unk17 : 1;
    u32 unk18 : 6;
} Unk8009BD38Flags;
extern Unk8009BD38Flags D_8009BD38;
extern u8 D_8009BD58;
extern u8 D_8009BD59;
void func_8005B6FC(void);
typedef struct GameObj {
    u8 field_00; u8 field_01; s16 field_02;
    s16 field_04; s16 field_06; s16 field_08; s16 field_0A;
    s16 field_0C; s16 field_0E; s16 field_10; s16 field_12;
    s16 field_14; s16 field_16; s32 field_18; s32 field_1C;
    s32 field_20; s32 field_24; s32 field_28; s32 field_2C;
    s16 field_30; s16 field_32; s16 field_34; s16 field_36;
    s16 field_38; s16 field_3A; s16 field_3C; s16 field_3E;
    s16 field_40; s16 field_42; s32 field_44; s32 field_48;
    s32 field_4C; s32 field_50; s16 field_54; s16 field_56;
    s32 field_58; s16 field_5C; s16 field_5E; s32 field_60;
    s32 field_64; s32 field_68; s32 field_6C; s32 field_70;
    s32 field_74; s32 field_78; s32 field_7C; s32 field_80;
    s16 field_84; s16 field_86; s16 field_88; s16 field_8A;
    s32 field_8C; s32 field_90; s32 field_94; s32 field_98;
    s32 field_9C; s32 field_A0; s32 field_A4; s32 field_A8;
    s32 field_AC; s32 field_B0; s32 field_B4; s32 field_B8;
    s32 field_BC; s32 field_C0; s32 field_C4; s32 field_C8;
    s32 field_CC; s32 field_D0; s32 field_D4; s32 field_D8;
    s32 field_DC; s32 field_E0; s32 field_E4; s32 field_E8;
    s32 field_EC; s32 field_F0; s32 field_F4; s16 field_F8;
    s16 field_FA; s32 field_FC;
} GameObj;
void func_8005C650(s32 a0, s32 a1, s32 a2);
extern s32 func_80073728(s32, s32);
extern s32 func_8007352C(s32);
extern s32 func_8006E480(s32, s32);
extern s32 SetDrawMode(s32, s32, s32, s32, s32);
extern s32 AddPrim(s32, s32);
extern s32 SetSemiTrans(void *, s32);
extern s32 SetTile(void *);
extern u8 g_gpu_db;
extern s32 SetDrawArea();
extern s32 func_8006E480();
extern s32 func_8007352C();
extern s32 snd_StopAll(void);
s32 func_80068F70(s32 arg0, s32 *arg1);
void func_8006920C(s32 *, s32);
void func_8006920C(s32 *a0, s32 a1);
s32 func_80069250(s32 arg0, s32 arg1);
s32 func_800692C0(u32 *arg0, s32 arg1, s16 *arg2, s16 *arg3);
s32 func_800693CC(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
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
extern u8 g_gpu_db;  /* one type per TU: all uses here take (s32)&g_gpu_db */
s32 func_8006D74C(s32 arg0, s32 arg1);
s32 func_8006D7FC(void);
extern s32 func_800692C0();
void func_8006E068(s32 arg0, s32 arg1);
s32 func_8006E10C(void);
extern void gpu_SetDrawEnvBg(s32, s32, s32, s32);
s32 func_8006E2A8(void);
s32 func_8006E480(s32 a0_addr, s32 a1);
s32 func_8006E49C(s32 arg0, s32 *arg1);
s32 func_8006E534(s32 arg0, s32 arg1, u8 *arg2, u32 arg3);
void func_8006E950(s32 *a0, s32 *a1);
s32 func_8006EACC(s32 arg0, s32 arg1);
s32 func_8007352C(s32 env_addr);
s32 func_80073728(s32 env_addr, s32 mode);
void func_80074220(s32 *arg0, s32 arg1);
typedef struct {
    s32 sp18;
    s32 sp1C;
    s32 sp20;
    s32 sp24;
    s32 sp28;
    s32 sp2C;
    s32 sp30;
    s32 sp34;
    s32 sp38;
    s32 sp3C;
    s8 sp40;
    u8 sp41;
    u8 sp42;
    u8 sp43;
} S_80074488;
void func_80074488(s32 *arg0);

/* func_800747D8: the duplicated `sound = 4;` below is claimed under
 * .claude/rules/duplicated-statement-into-arms.md and carries its FAKE
 * annotation inline; self-vet: pre-slim-2026-10-01:memory/grind/func_800747D8/self_vet.md. */
/* 0x800A35D0: one {s16, s16} pair per player (two words,
 * asm/data/91C98.data.s:4279-4282), passed to func_800692C0 beside SelWork
 * f40[player]; func_800768DC indexes it by player * 4 (0x80076948/5C). */
extern s16 D_800A35D0[2][2];
extern s8 D_800A35DC;

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
             * shared copy after `selection_sound:`; mechanism: reg_set_last
             * (tools/gcc-2.7.2/rtlanal.c:886-888) stops scanning at the
             * `selection_sound:` CODE_LABEL, so the store-flag gate at
             * tools/gcc-2.7.2/jump.c:1178 sees temp3 = a REG rather than a
             * CONST_INT and (BRANCH_COST == 1 on R3000) refuses the branchless
             * sltiu/sll fold; lever-exhaustion: pre-slim-2026-10-01:memory/grind/func_800747D8/
             * hypotheses.md s1-s9 (20 measured kills, incl. the s9
             * break-converged single-assignment control at score 10/205). */
            sound = 4;
            goto selection_sound;
        case 2:
            if (SELWORK->f65 == 0) {
                SELWORK->f65 = SELWORK->f64;
            } else {
                SELWORK->f65 -= 1;
            }
            /* FAKE: `sound = 4;` is written into BOTH inner-switch arms instead of one
             * shared copy after `selection_sound:`; mechanism: reg_set_last
             * (tools/gcc-2.7.2/rtlanal.c:886-888) stops scanning at the
             * `selection_sound:` CODE_LABEL, so the store-flag gate at
             * tools/gcc-2.7.2/jump.c:1178 sees temp3 = a REG rather than a
             * CONST_INT and (BRANCH_COST == 1 on R3000) refuses the branchless
             * sltiu/sll fold; lever-exhaustion: pre-slim-2026-10-01:memory/grind/func_800747D8/
             * hypotheses.md s1-s9 (20 measured kills, incl. the s9
             * break-converged single-assignment control at score 10/205). */
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



extern s32 g_gpu_ot_ptr;

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
            SetTile((GameObj *)p);
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
    /* work holds two values (Ruling 11, owner 2026-09-26; the first under its
       Q20 per-branch-constant clause): the backdrop TILE's OT index (0xE for
       player 1, 4 for player 0), then ot_idx * 4, the OT byte offset the last
       three AddPrim calls add. Proof: memory/grind/func_80074E08/r11/proof.md. */
    s32 work;
    s32 rect_x;
    /* the sprite sheet's cell array (8-byte SprtEntA cells), which starts
       just past the sheet's one 12-byte SprtHdrA header (+0xC) on each of
       the four root+0x18 sheets this function draws: one meaning at one
       constant offset, written once per sheet (Ruling 9, owner 2026-09-25).
       D_SEL.BIN census and receipts: memory/grind/func_80074E08/r9/. */
    s32 cells;
    s16 i;

    prim = arg0[5];
    SetTile(prim);
    SetSemiTrans(prim, 0);
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

extern s32 g_gpu_ot_ptr;

extern s32 func_8006E480(s32, s32);
extern s32 SetDrawMode(s32, s32, s32, s32, s32);

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
       cse.c only folds the constant within the entry extended basic block;
       lever-exhaustion: memory/grind/func_800753D8/hypotheses.md H2
       (inline literal 0 measured 70 vs 61,
       rejected/literal-zero-arg-no-holder-score70.c).
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
extern void func_8005C650(s32, s32, s32);
extern s32 func_800692C0();
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
    if (SELWORK->f1C[arg2] == var_a1 && SELWORK->f20[arg2] == var_a2) {
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

INCLUDE_ASM("asm/funcs", func_800759D0);

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
            SELWORK->f1C[arg3] = (SELWORK->f1C[arg3] + 1) & 1;
        }
    }

    switch (result >> 16) {
    case 1: {
        s16 value;

        value = SELWORK->f20[arg3];
        if (value == 4) {
            SELWORK->f20[arg3] = 0;
        } else {
            SELWORK->f20[arg3] = value + 1;
        }
    } break;
    case 2: {
        s16 value;

        value = SELWORK->f20[arg3];
        if (value == 0) {
            SELWORK->f20[arg3] = 4;
        } else {
            SELWORK->f20[arg3] = value - 1;
        }
    } break;
    }

    {
        u8 entry;

        entry = D_8009BCF8[arg1][SELWORK->f1C[arg3] * 5 + SELWORK->f20[arg3]].unk0;
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
             * the address (6).  Ledger: memory/grind/func_80075F80/manual-2026-10-01/scores.txt */
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
       per player) on the highlightable ones (+0x24). D_SEL.BIN layout:
       pre-slim-2026-10-01:memory/grind/func_8007636C/evidence.md "Ruling 9 re-audit". */
    s32 cells;
    s32 color;
    s16 i;
    /* FAKE: constant-holder (named-local-fake-exception) -- the 0 passed as
     * func_8006E480's second argument at both call sites. Set once and live
     * past the first loop, so cse substitutes its pseudo into that loop's
     * `(s16)i < f65 + 3` entry guard (slt needs a register operand) and reload
     * rematerializes it as the target's `move t0,zero; slt` (0x800764A0); a
     * literal 0 lets combine fold the guard to a beqz (3/348). The case-2
     * sibling func_800759D0 holds this same argument's zero in $fp (asm lines
     * 20/56/334/356). Lever exhaustion: pre-slim-2026-10-01:memory/grind/func_8007636C/hypotheses.md. */
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
 * draws the full-screen TILE with f36 as its colour.  The record and
 * bitfield layout, the do-while(0) tail and their measurements:
 * 3e35ec719^:memory/grind/func_80076D74/hypotheses.md (ledger closed at
 * 3e35ec719). */
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
    SetTile((GameObj *)p);
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
    do { /* FAKE: do-while(0) wrap, loop-end note pins the return copy after the sw so the increment temp takes v0; mechanism: sched.c loop_notes dependence on the first insn after NOTE_INSN_LOOP_END; lever-exhaustion: 3e35ec719^:memory/grind/func_80076D74/hypotheses.md s1-s2 */
        arg0[6] += 0xC;
    } while (0);
    return ret;
}
void func_8006920C(s32 *, s32);
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
extern s32 D_800A35D8;
s32 func_80077098(s32 a0) {
    return D_800A35D8 + a0 * 44;
}

extern s32 D_800A35D8;
extern s8 D_800A35DC;
extern s32 g_gpu_ot_ptr;
extern s32 ClearOTagR(s32, s32);
extern s32 snd_StopAll(void);

extern s32 func_80076FF8(s32 *);

INCLUDE_ASM("asm/funcs", func_800770B8);

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
extern s32 D_800A36AC;

extern u8 g_gpu_db;  /* one type per TU: all uses here take (s32)&g_gpu_db */
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
    SELWORK->f24 = &g_gpu_db + (D_800A36AC & 1) * 0x4090;
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
extern s32 D_800A35E4;


void gpu_SetDrawEnvBg(s32, s32, s32, s32);
s32 func_80077820(s32 a0) {
    func_80068F70(a0, (s32 *)&D_8009BD24);
    gpu_SetDrawEnvBg(1, 0, 0, 0);
    D_800A35E4 = 0;
    return 1;
}

extern s32 D_800A35E4;

s32 func_80077860(void) {
    if (((s32 (*)())func_80069250)() == 1) {
        D_800A35E4 = 0;
        return 1;
    }
    return 0;
}
s32 func_80077894(void) {
    s32 ret;
    s32 result;

    ret = 0;
    result = ((s32 (*)())func_800693CC)();
    if (result >= 0) {
        ret = 1;
        D_800A35E4 = 0;
        D_8009BD38.unk0 = result;
    } else if (result == -2) {
        ret = -1;
    }
    return ret;
}
extern s32 D_800A35E0;
s32 func_80077904(void) {
    s32 i;

    D_800A35E4 = 0;
    i = D_8009BD38.unk0 * 2;
    D_800A35E0 = *((u8 *)&D_8009BD59 + i);
    return *((u8 *)&D_8009BD58 + i);
}
extern s32 D_800A35E8;
void func_80077940(s32 arg0) {
    D_800A35E8 = (arg0 & 0x3FF) + ((u32) (arg0 & 0x3FF000) >> 2) + ((u32) (arg0 & 0x01000000) >> 4) + ((u32) (arg0 & 0x04000000) >> 5);
}
extern s32 D_800A35E0;
extern s32 D_800A35E8;

s32 func_8006E534(s32, s32, u8*, u32);
s32 func_80077984(s32 a0) {
    func_8006E534(a0, D_800A35E0, &D_8009BD24[0][0].chr, D_800A35E8);
    gpu_SetDrawEnvBg(1, 0, 0, 0);
    return 1;
}

void func_8005B6FC(void);
s32 func_800779C8(void) {
    s32 ret = ((s32 (*)())func_8006EACC)();
    if (ret) {
        func_8005B6FC();
    }
    return ret;
}
extern s32 D_800A35E4;

void func_80077A04(s32 a0, s32 a1) {
    D_800A35E4 = 0;
    func_8006D74C(a0, a1);
}
extern s32 D_800A35E4;
void gpu_SetDrawEnvBg(s32, s32, s32, s32);
s32 func_8006D7FC(void);
void func_80077A28(void) {
    D_800A35E4 = 0;
    gpu_SetDrawEnvBg(1, 0, 0, 0);
    func_8006D7FC();
}

void func_80077A60(void) {
    ((void (*)())func_8006E068)();
}
extern s32 D_800A35E8;

s32 func_800770B8(s32, s32, s32);
void gpu_SetDrawEnvBg(s32, s32, s32, s32);
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
extern s32 D_800A35E4;
void func_80077B20(void) {
    D_800A35E4 = 1;
}
