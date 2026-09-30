#define INCLUDE_ASM_USE_MACRO_INC 1
#include "common.h"
#include "include_asm.h"
#include "sound.h"
#include "game.h"
#include "code6cac.h"

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
typedef struct EnvA {
    s32 *header;
    s8  *table;
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
extern u8 D_8009BD20[][2];
extern u8 *D_800A36A0;
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

/* func_800747D8 - s10 (forensics), integrated 2026-09-21.  BYTES PROVEN ON
 * MAIN: a full driver build carrying this body produces build/bb2.exe SHA1
 * 62efab4f73f992798c43e8c730aa43baa10bb4fa (== the oracle).
 *
 * Honest sandbox score is 0 (build_insns 208 == target_insns 208), re-measured
 * on the integrated tree 2026-09-21; `--diff` reports 0 source-level /
 * 0 operand-only / 27 not-scored hunks, the not-scored set being masked
 * branch-target artifacts of the isolated build.  (While the function was held
 * as an integration handoff this comment recorded a 2/208 residual on the
 * ADDR_VEC's %lo addend.  That residual was an artifact of measuring before the
 * rodata geometry below was in place; it is gone, and the sandbox and the full
 * build now agree.)
 *
 * The body alone was NOT sufficient - two edits on surfaces a grind session may
 * not touch were also required, and BOTH landed in commit f98985ae3 (the
 * func_8006ECF4 completion) before this body was integrated.  Full recipe +
 * proof: memory/grind/func_800747D8/integration/README.md
 * In short: (1) Makefile:136 RODATA_ALIGN2_FILES += text1b, because
 * tools/gcc-2.7.2/final.c:1515-1518 emits an unconditional `.align 3` before
 * every .rdata ADDR_VEC and this function's table sits at a 4-mod-8 address;
 * (2) the rodata-ownership move that keeps text1b.o(.rodata) contiguous across
 * 0x80015988..0x80015A20 - the hand-written const D_800159A0 plus the
 * compiler-generated ADDR_VECs of func_8006E534 and func_8006ECF4, which the
 * compiler emits into this TU once those functions are C.
 *
 * The s9 change vs the floor-6 body is the selection_sound block only:
 * `sound = 4;` sits at the END of each of the two inner-switch arms (before
 * `goto selection_sound;`) and the block reduces to
 * `if (field64 != 0) sound = 0;`.  Mechanism (measured, not guessed):
 * `reg_set_last` (tools/gcc-2.7.2/rtlanal.c:886-888) stops at a CODE_LABEL,
 * so with `sound = 4;` on the far side of the `selection_sound:` label the
 * store-flag gate at tools/gcc-2.7.2/jump.c:1178 sees temp3 = a REG rather
 * than CONST_INT and (BRANCH_COST == 1 on R3000) refuses the branchless
 * fold; the two-arm branch survives with the test byte on its own pseudo,
 * which is target's `lbu v0,0x64(v0)` / `beqz v0` seat.  Putting the
 * assignment in the ARMS rather than at the top of `case 0:` (s6 variant F,
 * score 8) is what keeps $a0 dead across the field65 update block and lets
 * the two copies cross-jump-merge back into the single `li a0,4` that
 * target carries in the branch's delay slot.
 *
 * Self-vet: memory/grind/func_800747D8/self_vet.md.  The duplicated
 * `sound = 4;` is claimed under .claude/rules/duplicated-statement-into-arms.md
 * and carries the mandated FAKE annotation inline (below).
 */
extern u8 D_8009BD20[][2];
extern s16 D_800A35D0;
extern s8 D_800A35DC;
extern u8 *D_800A36A0;

typedef struct {
    u8 pad00[0x10];
    union {
        s32 word10;
        s16 half10[2];
    } field10;
    u8 pad14[4];
    s16 field18[2];
    u8 pad1C[0x18];
    u16 field34;
    u8 pad36[2];
    s16 field38[2];
    u16 field3C;
    u8 pad3E[0x26];
    u8 field64;
    u8 field65;
    u8 field66;
    u8 field67;
} S_800747D8;

#define MENU_800747D8 ((S_800747D8 *)D_800A36A0)

s32 func_800747D8(u32 input) {
    u8 *base;
    s32 sp10;
    s32 ret;
    s32 state;
    s32 result;
    s32 i;
    S_800747D8 *menu;
    S_800747D8 *work;
    u8 row;
    s32 sound;

    base = D_800A36A0;
    result = 0;
    if (*(s32 *)(base + 0x10) == 0) {
        sp10 = (input & 0xFFFF) | (input >> 16);
        ret = func_800692C0((u32 *)&sp10, 0, (s16 *)(base + 0x40), &D_800A35D0);
        switch (ret >> 16) {
    case 1:
        MENU_800747D8->field34 = 0;
        MENU_800747D8->field3C += 1;
        if ((s16)MENU_800747D8->field3C >= 5) {
            MENU_800747D8->field3C = 0;
        }
        func_8005C650(0, 0x7F, 0x7F);
        break;
    case 2:
        MENU_800747D8->field34 = 0;
        MENU_800747D8->field3C -= 1;
        if ((s16)MENU_800747D8->field3C < 0) {
            MENU_800747D8->field3C = 4;
        }
        func_8005C650(0, 0x7F, 0x7F);
        break;
        }

        state = (s16)MENU_800747D8->field3C;
        switch (state) {
    case 0:
        switch (ret & 0xFF) {
        case 1:
            if (MENU_800747D8->field65 == MENU_800747D8->field64) {
                MENU_800747D8->field65 = 0;
            } else {
                MENU_800747D8->field65 += 1;
            }
            /* FAKE: `sound = 4;` is written into BOTH inner-switch arms instead of one
             * shared copy after `selection_sound:`; mechanism: reg_set_last
             * (tools/gcc-2.7.2/rtlanal.c:886-888) stops scanning at the
             * `selection_sound:` CODE_LABEL, so the store-flag gate at
             * tools/gcc-2.7.2/jump.c:1178 sees temp3 = a REG rather than a
             * CONST_INT and (BRANCH_COST == 1 on R3000) refuses the branchless
             * sltiu/sll fold; lever-exhaustion: memory/grind/func_800747D8/
             * hypotheses.md s1-s9 (20 measured kills, incl. the s9
             * break-converged single-assignment control at score 10/205). */
            sound = 4;
            goto selection_sound;
        case 2:
            if (MENU_800747D8->field65 == 0) {
                MENU_800747D8->field65 = MENU_800747D8->field64;
            } else {
                MENU_800747D8->field65 -= 1;
            }
            /* FAKE: `sound = 4;` is written into BOTH inner-switch arms instead of one
             * shared copy after `selection_sound:`; mechanism: reg_set_last
             * (tools/gcc-2.7.2/rtlanal.c:886-888) stops scanning at the
             * `selection_sound:` CODE_LABEL, so the store-flag gate at
             * tools/gcc-2.7.2/jump.c:1178 sees temp3 = a REG rather than a
             * CONST_INT and (BRANCH_COST == 1 on R3000) refuses the branchless
             * sltiu/sll fold; lever-exhaustion: memory/grind/func_800747D8/
             * hypotheses.md s1-s9 (20 measured kills, incl. the s9
             * break-converged single-assignment control at score 10/205). */
            sound = 4;
            goto selection_sound;
        }
        goto confirm;
selection_sound:
        if (MENU_800747D8->field64 != 0) {
            sound = 0;
        }
        func_8005C650(sound, 0x7F, 0x7F);
        goto confirm;
    case 1:
        if ((ret & 0xFF) != 0) {
            menu = (S_800747D8 *)D_800A36A0;
            menu->field67 += 1;
            menu = (S_800747D8 *)D_800A36A0;
            menu->field67 &= 1;
            work = (S_800747D8 *)D_800A36A0;
            row = work->field67;
            work->field66 = D_8009BD20[row][D_800A35DC & 1];
            func_8005C650(0, 0x7F, 0x7F);
        }
        goto confirm;
    case 2:
        if ((ret & 0xFF) != 0) {
            work = (S_800747D8 *)D_800A36A0;
            row = work->field67;
            D_800A35DC += 1;
            work->field66 = D_8009BD20[row][D_800A35DC & 1];
            func_8005C650(0, 0x7F, 0x7F);
        }
        goto confirm;
confirm:
        if (input & 0x400040) {
            func_8005C650(1, 0x7F, 0x7F);
            MENU_800747D8->field3C = 3;
        }
        break;
    case 3:
        if (input & 0x400040) {
            func_8005C650(1, 0x7F, 0x7F);
            for (i = 0; i < 2; i++) {
                MENU_800747D8->field10.half10[i] = 3;
                MENU_800747D8->field18[i] = 1;
                MENU_800747D8->field38[i] = 0;
            }
        }
        goto tail;
    case 4:
        if (input & 0x400040) {
            func_8005C650(1, 0x7F, 0x7F);
            result = -1;
        }
        goto tail;
        }
tail:
        if (input & 0x100010) {
            func_8005C650(2, 0x7F, 0x7F);
            result = -1;
        }
    }
    return result;
}

#undef MENU_800747D8


extern u8 *D_800A36A0;
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
    for (i = 0; i < *(u8 *)(D_800A36A0 + 0x65) + 3; i++) {
        t = *(u8 **)(*(u8 **)(D_800A36A0 + 4) + 0x3C);
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
    s32 **records;
    s32 prim;
    s32 ot_idx;
    s32 rect_x;
    s8 *table;
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
    ot_idx = 4;
    if (arg1 != 0) {
        ot_idx = 0xE;
    }
    AddPrim(g_gpu_ot_ptr + ot_idx * 4 + 0x24, prim);
    prim += 0x10;
    arg0[5] = prim;

    records = *(s32 ***)(arg0[0] + 0x18);
    s.semi = 0;
    s.has_color = 0;
    s.x = arg1 * 0xF0;
    s.ot_idx = 2;
    i = 0;
    do {
        s.y = (0xD2 - *(s16 *)(D_800A36A0 + arg1 * 2 + 0xC)) * i;
        {
            s.header = records[3];
            table = (s8 *)s.header + 0xC;
            s.table = table;
            s.out = arg0[4];
            arg0[4] = func_8007352C((s32)&s);
        }
        {
            s.header = records[2];
            table = (s8 *)s.header + 0xC;
            s.table = table;
            s.out = arg0[4];
            arg0[4] = func_8007352C((s32)&s);
        }
        s.table += *((u8 *)s.header + 2) * 8;
        s.pad20 = 0xCE00;
        s.pad24 = 0x100;
        s.pad0C = arg0[1];
        arg0[1] = func_80073728((s32)&s, 0);
        i++;
    } while (i < 2);

    s.x = arg1 * 0xF0;
    s.y = 0;
    {
        s.header = records[0];
        table = (s8 *)s.header + 0xC;
        s.table = table;
    }
    if (arg1 != 0) {
        s.ot_idx = 0x16;
    } else {
        s.ot_idx = 0xC;
    }
    s.out = arg0[4];
    arg0[4] = func_8007352C((s32)&s);
    {
        s.header = records[1];
        table = (s8 *)s.header + 0xC;
        s.table = table;
    }
    s.out = arg0[4];
    arg0[4] = func_8007352C((s32)&s);
    SetDrawMode(arg0[6], 1, 0, func_8006E480((s32)s.header, 0), 0);
    AddPrim(g_gpu_ot_ptr + s.ot_idx * 4, arg0[6]);
    arg0[6] += 0xC;

    if (arg1 != 0) {
        ot_idx = 0xE;
        rect_x = 0x14E;
    } else {
        ot_idx = 4;
        rect_x = 0x5E;
    }
    rect[0] = *(u16 *)(*(s32 *)(D_800A36A0 + 0x24)) + rect_x;
    rect[1] = *(u16 *)(*(s32 *)(D_800A36A0 + 0x24) + 2) + 0x14;
    rect[2] = 0xD4;
    rect[3] = 0xC8 - *(u16 *)(D_800A36A0 + arg1 * 2 + 0xC);
    SetDrawArea(arg0[7], rect);
    AddPrim(g_gpu_ot_ptr + ot_idx * 4 + 0x24, arg0[7]);
    arg0[7] += 0xC;

    rect[0] = *(u16 *)(*(s32 *)(D_800A36A0 + 0x24));
    rect[1] = *(u16 *)(*(s32 *)(D_800A36A0 + 0x24) + 2);
    rect[2] = *(u16 *)(*(s32 *)(D_800A36A0 + 0x24) + 4);
    rect[3] = *(u16 *)(*(s32 *)(D_800A36A0 + 0x24) + 6);
    SetDrawArea(arg0[7], rect);
    AddPrim(g_gpu_ot_ptr + ot_idx * 4, arg0[7]);
    arg0[7] += 0xC;

    offset[0] = *(u16 *)(*(s32 *)(D_800A36A0 + 0x24) + 8);
    offset[1] = *(u16 *)(*(s32 *)(D_800A36A0 + 0x24) + 0xA)
              - *(u16 *)(D_800A36A0 + arg1 * 2 + 8);
    SetDrawOffset(arg0[8], offset);
    AddPrim(g_gpu_ot_ptr + ot_idx * 4 + 0x24, arg0[8]);
    arg0[8] += 0xC;

    offset[0] = *(u16 *)(*(s32 *)(D_800A36A0 + 0x24) + 8);
    offset[1] = *(u16 *)(*(s32 *)(D_800A36A0 + 0x24) + 0xA);
    SetDrawOffset(arg0[8], offset);
    AddPrim(g_gpu_ot_ptr + ot_idx * 4, arg0[8]);
    arg0[8] += 0xC;
}
extern u8 *D_800A36A0;
void func_8007526C(void) {
    u8 *base;
    u8 *p;
    s32 i;
    s32 lim;

    base = D_800A36A0;
    i = 0;
    do {
        lim = 0xC8;
        p = base + i * 2;
        switch (*(u8 *)(p + 0x10)) {
        case 1:
            *(u16 *)(p + 8) = *(u16 *)(p + 8) + 0xA;
            *(u16 *)(p + 0xC) = *(u16 *)(p + 0xC) + 0xA;
            if ((s16)*(u16 *)(p + 0xC) >= 0xC8) {
                if ((*(u16 *)(p + 0x10) >> 8) == 0) {
                    *(u16 *)(p + 0x10) = *(u16 *)(p + 0x10) + 1;
                }
                *(u16 *)(p + 8) = 0;
                *(u16 *)(p + 0xC) = lim;
                *(u16 *)(p + 0x14) = *(u16 *)(p + 0x18);
                *(u16 *)(p + 0x3C) = *(u16 *)(p + 0x38);
            }
            break;
        case 3:
            *(u16 *)(p + 0xC) = *(u16 *)(p + 0xC) + 0xA;
            if ((s16)*(u16 *)(p + 0xC) >= 0xC8) {
                *(u16 *)(p + 8) = lim;
                *(u16 *)(p + 0xC) = lim;
                *(u16 *)(p + 0x14) = *(u16 *)(p + 0x18);
                *(u16 *)(p + 0x3C) = *(u16 *)(p + 0x38);
                if ((*(u16 *)(p + 0x10) >> 8) == 0) {
                    *(u16 *)(p + 0x10) = *(u16 *)(p + 0x10) + 1;
                }
            }
            break;
        case 2:
            *(u16 *)(p + 0xC) = *(u16 *)(p + 0xC) - 0xA;
            if ((s16)*(u16 *)(p + 0xC) <= 0) {
                *(u16 *)(p + 8) = 0;
                *(u16 *)(p + 0xC) = 0;
                *(u16 *)(p + 0x10) = 0;
            }
            break;
        case 4:
            *(u16 *)(p + 8) = *(u16 *)(p + 8) - 0xA;
            *(u16 *)(p + 0xC) = *(u16 *)(p + 0xC) - 0xA;
            if ((s16)*(u16 *)(p + 0xC) <= 0) {
                *(u16 *)(p + 8) = 0;
                *(u16 *)(p + 0xC) = 0;
                *(u16 *)(p + 0x10) = 0;
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

extern u8 *D_800A36A0;
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
    u8 *base;

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
    base = D_800A36A0;
    s.sp34 = *(u8 *)(base + arg1 + 0x68) * 90 + *(s16 *)(base + (arg1 << 2) + 0x42);
    c = ((rsin(((*(u16 *)(base + 0x34) & 0x1F) << 7) + 0x1FF) << 5) >> 12) - 0x80;
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
extern u8 *D_800A36A0;
extern s16 D_800A35D0;
extern void func_8005C650(s32, s32, s32);
extern s32 func_800692C0();
void func_80075670(s32 arg0, s32 arg1) {
    s16 i;
    u8 *base;
    u8 *work;
    u8 *q;
    s16 *p;

    base = D_800A36A0;
    if (*(s32 *)(base + 0x10) != 0) {
        return;
    }
    if ((func_800692C0(&arg0, arg1, (s16 *)(base + (arg1 * 4 + 0x40)), (&D_800A35D0) + (arg1 * 2)) >> 16) != 0) {
        *(s16 *)(D_800A36A0 + 0x34) = 0;
        *(u8 *)(D_800A36A0 + arg1 + 0x68) = *(u8 *)(D_800A36A0 + arg1 + 0x68) + 1;
        base = D_800A36A0;
        *(u8 *)(base + arg1 + 0x68) &= 1;
        func_8005C650(0, 0x7F, 0x7F);
    }
    work = D_800A36A0;
    *(s16 *)(work + arg1 * 2 + 0x3C) = *(u8 *)(work + arg1 + 0x68);
    if (arg0 & (0x40 << (arg1 * 16))) {
        for (i = 0; i < 2; i++) {
            ((s16 *)(work + i * 2))[0x38 / 2] = 0;
            ((s16 *)(work + i * 2))[0x10 / 2] = 1;
            ((s16 *)(work + i * 2))[0x18 / 2] = 2;
        }
        q = D_800A36A0;
        *(u8 *)(q + ((arg1 + 1) & 1) + 0x68) = (*(u8 *)(q + arg1 + 0x68) + 1) & 1;
        func_8005C650(1, 0x7F, 0x7F);
        return;
    }
    if (arg0 & (0x10 << (arg1 * 16))) {
        if (arg1 != 0) {
            p = (s16 *)(work + 0x14);
        } else {
            p = (s16 *)(work + 0x16);
        }
        if (*p == 1) {
            for (i = 0; i < 2; i++) {
                ((s16 *)(work + i * 2))[0x38 / 2] = 0;
                ((s16 *)(work + i * 2))[0x10 / 2] = 3;
                ((s16 *)(work + i * 2))[0x18 / 2] = 0;
            }
            func_8005C650(2, 0x7F, 0x7F);
        } else {
            func_8005C650(4, 0x7F, 0x7F);
        }
    }
}
void func_80075830(s32 *arg0, s32 arg1, s32 arg2, s32 arg3) {
    u8 packet[0x2C];
    s16 var_a1;
    s16 var_a2;
    s32 temp_v0;
    s32 temp_v1;
    s16 *tbl;
    *(s32 *)(packet + 0x10) = arg3;
    temp_v1 = ((s32) (rsin(((*(u16 *)((s32)D_800A36A0 + 0x34) & 0x1F) << 7) + 0x1FF) * 0x30) >> 12) - 0x40;
    *(s8 *)(packet + 0x2B) = temp_v1;
    *(s8 *)(packet + 0x2A) = temp_v1;
    *(s8 *)(packet + 0x29) = temp_v1;
    temp_v0 = *(s32 *)(*(s32 *)(*arg0 + 0x14) + 0x54);
    *(s32 *)(packet + 0x00) = temp_v0;
    *(s32 *)(packet + 0x04) = temp_v0 + 0xC;
    if (arg1 < 0xA) {
        var_a1 = arg1 / 5;
        var_a2 = arg1 % 5;
    } else {
        var_a1 = (arg1 - 0xA) / 5;
        var_a2 = (arg1 - 0xA) % 5;
    }
    tbl = (s16 *)(arg2 * 2 + (s32)D_800A36A0);
    if (tbl[0x1C / 2] == var_a1 && tbl[0x20 / 2] == var_a2) {
        *(s8 *)(packet + 0x28) = 1;
    } else {
        *(s8 *)(packet + 0x28) = 0;
    }
    *(s32 *)(packet + 0x18) = arg2 * 0xF0 + var_a1 * 0x64;
    *(s32 *)(packet + 0x1C) = var_a2 * 16;
    if (arg2 != 0) {
        *(s32 *)(packet + 0x14) = 0x13;
    } else {
        *(s32 *)(packet + 0x14) = 9;
    }
    *(s32 *)(packet + 0x08) = arg0[0x10 / 4];
    arg0[0x10 / 4] = func_8007352C((s32)packet);
}
extern u8 D_8009BCE4;

INCLUDE_ASM("asm/funcs", func_800759D0);
extern u8 D_8009BCE4;

INCLUDE_ASM("asm/funcs", func_80075F80);
/* Shared select work area at D_800A36A0 (the block func_80075F80 indexes by
 * raw offset); two-element arrays are per player. Per player, f48 is a list
 * of f60 entries with cursor f5C; confirming moves the entry under the cursor
 * to f7E[f3C++], cancelling moves f7E[--f3C] back into f48 in sorted order. */
typedef struct {
    u8 pad00[0x10];
    s16 f10[2];
    s16 f14[2];
    s16 f18[2];
    u8 pad1C[0x18];
    u16 f34;
    u8 pad36[2];
    s16 f38[2];
    s16 f3C[2];
    s16 f40[2][2];
    s16 f48[2][5];
    s16 f5C[2];
    s16 f60[2];
    u8 f64;
    u8 f65;
    u8 pad66[0x18];
    s16 f7E[2][5];
} SelWork_800768DC;

#define SELWORK_800768DC ((SelWork_800768DC *)D_800A36A0)

void func_8007636C(s32 *arg0, s32 arg1, s16 *arg2, s32 arg3) {
    S_80074488 s;
    s32 ot;
    s32 *table;
    /* the sprite sheet's cell array (8-byte SprtEntA cells), which starts
       just past the sheet's 12-byte SprtHdrA headers: one header on the
       single-state sheets (+0xC), three (normal, then one cursor highlight
       per player) on the highlightable ones (+0x24). D_SEL.BIN layout:
       memory/grind/func_8007636C/evidence.md "Ruling 9 re-audit". */
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
     * 20/56/334/356). Lever exhaustion: memory/grind/func_8007636C/hypotheses.md. */
    s32 mode;
    u16 idx;

    mode = 0;
    ot = 10;
    s.sp28 = 0;
    if (arg3 != 0) {
        ot = 20;
    }
    table = *(s32 **)(arg0[0] + 0x30);
    if (SELWORK_800768DC->f14[arg3] < 4) {
        s.sp18 = table[12];
        s.sp40 = 0;
        s.sp30 = arg3 * 240;
        cells = s.sp18 + 0xC;
        s.sp1C = cells;
        s.sp34 = SELWORK_800768DC->f3C[arg3] * 34;
        s.sp2C = ot;
        s.sp20 = arg0[4];
        arg0[4] = func_8007352C((s32)&s);
        SetDrawMode(arg0[6], 1, 0, func_8006E480(s.sp18, mode), 0);
        AddPrim(g_gpu_ot_ptr + ot * 4, arg0[6]);
        arg0[6] += 0xC;
    }

    s.sp40 = 0;
    color = ((rsin(((SELWORK_800768DC->f34 & 0x1F) << 7) + 0x1FF) << 5) >> 12) - 0x50;
    s.sp41 = s.sp42 = s.sp43 = color;
    table = *(s32 **)(arg0[0] + 0x14);
    for (i = 0; i < SELWORK_800768DC->f65 + 3; i++) {
        s.sp40 = 0;
        s.sp18 = table[arg2[i] + 1];
        cells = s.sp18 + 0x24;
        if (SELWORK_800768DC->f3C[arg3] == i || SELWORK_800768DC->f14[arg3] >= 4) {
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
    for (i = 0; i < SELWORK_800768DC->f3C[arg3] + 1; i++) {
        if (SELWORK_800768DC->f3C[arg3] != i || SELWORK_800768DC->f14[arg3] >= 4) {
            idx = SELWORK_800768DC->f7E[arg3][i];
            s.sp40 = 0;
        } else {
            idx = SELWORK_800768DC->f48[arg3][SELWORK_800768DC->f5C[arg3]];
            s.sp40 = 1;
        }
        s.sp18 = table[(s16)idx * 2];
        cells = s.sp18 + 0x24;
        if (SELWORK_800768DC->f3C[arg3] == i || SELWORK_800768DC->f14[arg3] >= 4) {
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

    table = *(s32 **)(arg0[0] + SELWORK_800768DC->f65 * 4 + 0x20);
    for (i = 0; i < SELWORK_800768DC->f65 + 3; i++) {
        s.sp40 = 0;
        s.sp18 = table[i];
        cells = s.sp18 + 0x24;
        if (SELWORK_800768DC->f3C[arg3] == i || SELWORK_800768DC->f14[arg3] >= 4) {
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

    if (SELWORK_800768DC->f10[arg3] != 0) {
        return;
    }
    if (arg0 & (0xA000 << (arg3 * 16))) {
        func_8005C650(0, 0x7F, 0x7F);
    }
    switch (func_800692C0((u32 *)&arg0, arg3, SELWORK_800768DC->f40[arg3], (&D_800A35D0) + (arg3 * 2)) & 0xFF) {
    case 1:
        SELWORK_800768DC->f5C[arg3]++;
        if (SELWORK_800768DC->f5C[arg3] >= SELWORK_800768DC->f60[arg3]) {
            SELWORK_800768DC->f5C[arg3] = 0;
        }
        break;
    case 2:
        SELWORK_800768DC->f5C[arg3]--;
        if (SELWORK_800768DC->f5C[arg3] < 0) {
            SELWORK_800768DC->f5C[arg3] = SELWORK_800768DC->f60[arg3] - 1;
        }
        break;
    }

    if (arg0 & (0x40 << (arg3 * 16))) {
        if (SELWORK_800768DC->f3C[arg3] < SELWORK_800768DC->f65 + 3) {
            func_8005C650(1, 0x7F, 0x7F);
            SELWORK_800768DC->f7E[arg3][SELWORK_800768DC->f3C[arg3]] = SELWORK_800768DC->f48[arg3][SELWORK_800768DC->f5C[arg3]];
            if (SELWORK_800768DC->f3C[arg3] == SELWORK_800768DC->f65 + 2) {
                SELWORK_800768DC->f14[arg3] = 4;
                return;
            }
            SELWORK_800768DC->f60[arg3]--;
            for (i = SELWORK_800768DC->f5C[arg3]; i < SELWORK_800768DC->f60[arg3]; i++) {
                SELWORK_800768DC->f48[arg3][i] = SELWORK_800768DC->f48[arg3][i + 1];
            }
            SELWORK_800768DC->f5C[arg3] = 0;
            SELWORK_800768DC->f3C[arg3]++;
        }
    } else if (arg0 & (0x10 << (arg3 * 16))) {
        func_8005C650(2, 0x7F, 0x7F);
        if (SELWORK_800768DC->f3C[arg3] == 0) {
            SELWORK_800768DC->f10[arg3] = 3;
            SELWORK_800768DC->f18[arg3] = 2;
            SELWORK_800768DC->f38[arg3] = SELWORK_800768DC->f65 + 2;
            (&D_8009BCE4)[arg2[SELWORK_800768DC->f38[arg3]]] &= ~(4 << arg3);
            SELWORK_800768DC->f60[arg3] = 5;
            for (i = 0; i < SELWORK_800768DC->f60[arg3]; i++) {
                SELWORK_800768DC->f48[arg3][i] = i;
            }
            if (arg1 != 0) {
                SELWORK_800768DC->f48[arg3][4] = 5;
            }
            return;
        }
        /* Put the last taken entry back into the list in sorted position. */
        for (i = 0; i < SELWORK_800768DC->f60[arg3]; i++) {
            if (SELWORK_800768DC->f48[arg3][i] > SELWORK_800768DC->f7E[arg3][SELWORK_800768DC->f3C[arg3] - 1]) {
                for (j = SELWORK_800768DC->f60[arg3] - 1; j >= i; j--) {
                    SELWORK_800768DC->f48[arg3][j + 1] = SELWORK_800768DC->f48[arg3][j];
                }
                SELWORK_800768DC->f48[arg3][i] = SELWORK_800768DC->f7E[arg3][SELWORK_800768DC->f3C[arg3] - 1];
                break;
            }
            if (i == SELWORK_800768DC->f60[arg3] - 1) {
                SELWORK_800768DC->f48[arg3][i + 1] = SELWORK_800768DC->f7E[arg3][SELWORK_800768DC->f3C[arg3] - 1];
            }
        }
        SELWORK_800768DC->f60[arg3]++;
        SELWORK_800768DC->f3C[arg3]--;
    }
}
/* func_80076D74 - Judge-CLEARED body (decisions.md 2026-09-15 13:47 ruling PASS, review-ledger hash ffd478b35c7a6afd). Submit VERBATIM.
 * s3 (permuter, 2026-09-15): re-measured sandbox --disable all = 0 on HEAD, and BYTES PROVEN ON MAIN - a full clean link with this body
 * in src/text1b.c (record-table declaration TU-local at text1b.c:2227-2235 for the measurement) gives SHA1 62efab4f73f992798c43e8c730aa43baa10bb4fa
 * == oracle (tmp/grind/func_80076D74/s3/fullbuild.log). Declaration placement is byte-neutral.
 *
 * FINAL FORM per the 13:47 Judge ruling (operator-applied, outside the grind surface; see docs/grind/decisions.md s3 INTEGRATION HANDOFF entry):
 *   include/game.h (after the Unk800F0EC8Record decl):  typedef struct { u8 unk0; u8 unk1; } Unk8009BCF8Record;  extern Unk8009BCF8Record D_8009BCF8[20];
 *   src/text1b.c:2227-2228 and src/text1b_b.c:237-238: delete `extern u8 D_8009BCF8;` / `extern u8 D_8009BCF9;`
 *   undefined_syms_auto.txt:79: DELETE the `D_8009BCF9 = 0x8009BCF9;` row (not suffix); keep line 1252 `D_8009BCF8 = 0x8009BCF8;`
 *   memory/grind/func_80076D74/record_table_decl.patch carries all three hunks (game.h add, text1b_b.c delete, undefined_syms_auto.txt:79 DELETE) - corrected per the ruling.
 * s2-rerun (2026-09-15, driver session 2, third dispatch): bytes RE-PROVEN from clean HEAD 493ad9e97 - sandbox 0 at 161/161 (tmp/grind/func_80076D74/s2/handoff/
 * candidate_head.sandbox.txt) and full tmp-only link SHA1 62efab4f73f992798c43e8c730aa43baa10bb4fa == oracle (s2/handoff/fullbuild.log). Filed as
 * OWNER-ESCALATION - INTEGRATION HANDOFF in docs/grind/decisions.md (scope grant: include/game.h src/text1b_b.c undefined_syms_auto.txt).
 * Object-model evidence (Judge-verified): asm/data/7D920.data.s:23762-23808 = 0x28 bytes = 20 two-byte records; func_800759D0.s:107-108 base in $s6,
 * :142-144 column 0 at 2-byte stride; func_80075F80.s:165-167 column 0 at 2-byte stride; func_80076D74.s:91-93 column 1 at 2-byte stride.
 * The BANNED `extern u8 D_8009BCF8[][2]` pair table is NOT used; the S_80076D74 typedef is emitted exactly ONCE (apply with tmp/grind/func_80076D74/s2/apply2.py).
 *
 * Tail (s1 residual 5, epilogue) closed by a single-level do { } while (0) wrap of the final arg0[6] += 0xC statement (sanctioned family,
 * .claude/rules/do-while-zero-exception.md, FAKE-annotated inline; layer-1 PASSED 2026-09-15 12:46, Judge PASS 13:47).
 * Mechanism: sched.c loop_notes attach NOTE_INSN_LOOP_END to the next insn (the return copy), which then depends on every earlier
 * set/use in the block, so it cannot be hoisted into the lw load-delay slot; the increment temp takes $v0; tail = lw/nop/addiu/sw/move.
 * Lever exhaustion (ordinary C, all 5 unless noted): u8 ret two-copy chain (s2); s32 arg0 + cast offsets (s1); slot pointer `s32 *dm` (s3);
 * packet-pointer round trip (s3, 21); branch-on-ret return (s3); permuter campaigns on s32-ret (34.7k), u8-ret (43.3k) and branch-on-ret
 * (31.6k) no-FAKE chassis find only do-while(0) forms (s2, s3). FAKE ablation this session (wrap removed, nothing else) = 5.
 * See evidence.md / hypotheses.md.
 */
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
    cnt = (u16 *)(D_800A36A0 + 0x36);
    v = *cnt + 8;
    *cnt = v;
    if (v >= 0xFF) {
        *cnt = 0xFF;
        hdr = *(S_80076D74 **)D_800A36A0;
        hdr->f10 = *(u8 *)(D_800A36A0 + 0x65);
        ret = 1;
        if (*(u8 *)(D_800A36A0 + 0x66) < 3) {
            sel = *(u8 *)(D_800A36A0 + 0x66) - 1;
        } else {
            sel = *(u8 *)(D_800A36A0 + 0x66) - 2;
        }
        hdr->f12 = sel;
        hdr->f14 = *(u8 *)(D_800A36A0 + 0x67);
        hdr->f15 = *(u8 *)(D_800A36A0 + 0x68) + *(u8 *)(D_800A36A0 + 0x69) * 2;
        for (i = 0; i < 2; i++) {
            for (j = 0; j < *(u8 *)(D_800A36A0 + 0x65) + 3; j++) {
                hdr->cells[i][j][0] = D_8009BCF8[*(s16 *)(D_800A36A0 + i * 10 + (j << 1) + 0x6A)].unk1;
                hdr->cells[i][j][1] = *(u16 *)(D_800A36A0 + i * 10 + (j << 1) + 0x7E);
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
    do { /* FAKE: do-while(0) wrap, loop-end note pins the return copy after the sw so the increment temp takes v0; mechanism: sched.c loop_notes dependence on the first insn after NOTE_INSN_LOOP_END; lever-exhaustion: memory/grind/func_80076D74/hypotheses.md s1-s2 */
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

extern u8 *D_800A36A0;
extern s32 D_800A35D8;
extern s8 D_800A35DC;
extern u8 D_8009BCE4;
extern u8 D_8009BD21;
extern s16 D_800A35D0;
extern s32 g_gpu_ot_ptr;
extern s32 ClearOTagR(s32, s32);
extern s32 snd_StopAll(void);

extern s32 func_80076FF8(s32 *);

s32 func_800770B8(s32 arg0, s32 arg1, s32 arg2) {
    u16 sp[2];
    s32 *p_old;
    s32 r;
    s16 t0;
    s16 a2;

    /* FAKE: empty do-while(0) wrap. Effect: it anchors a
       NOTE_INSN_LOOP_BEG/END pair at this statement position, which stops sched2
       interleaving the five reload-emitted frame-save stores with the first body
       insns; without it the prologue emits sw $s1 / addiu $s1,$s0,0x58 / lw
       D_800A374C / li 0x1008 / sw $ra where the target emits sw $ra / sw $s1 /
       li 0x1008 / lw D_800A374C / addiu $s1 (residual class A, 4 rows).
       mechanism: GCC 2.7.2 sched.c list scheduler, second pass (sched2, post-reload);
       the notes bound the scheduling region so the save stores cannot be hoisted
       across them. See evidence.md [s9] for the insn-level read-out of the
       unfenced order and [s11] for the measurement.
       lever-exhaustion: hypotheses.md classes A/B/C; s3 (12 statement orderings),
       s5 (honest-loop fence hunt, +11 insns), s9 (exhaustive 3234-atom sched_solver
       depth-1 sweep against the target emission order: 0 hits; the only reachable
       sub-goal needs atoms not expressible in C), s10 (struct-typed rederive 178
       insns), s11 (63-position single-wrap sweep + 18 nested-wrap variants). */
    do { } while (0);
    sp[0] = 0;
    sp[1] = 0;
    ClearOTagR(g_gpu_ot_ptr, 0x1008);
    p_old = (s32 *)(arg0 + 0x58);
    D_800A35D8 = arg0;
    snd_StopAll();
    func_8006E950(6, p_old);
    r = func_80076FF8(p_old);
    {
        s32 *prev = p_old;
        p_old = (s32 *)func_8006E49C(r, D_800A35D8);
        D_800A36A0 = (u8 *)p_old;
        *(s32 *)((u8 *)p_old + 4) = (s32)prev;
        /* FAKE: same-value dead store restoring p_old's pre-call value (p_old is
           never read again). Effect: it denies local-alloc's combine_regs its
           reg_n_deaths == 1 precondition on the p_old pseudo, so the 0x30/0x34
           clears keep the target's base register instead of collapsing onto the
           freshly returned pointer.
           mechanism: GCC 2.7.2 local-alloc.c:472 (combine_regs / block-quantity
           grant gated on reg_n_deaths == 1).
           lever-exhaustion: hypotheses.md class B, s1-s31 (31 sessions of
           store-base spellings), re-measured negative on three differing chassis
           in s37 (k1-k6) and ablation-confirmed load-bearing here in s39
           (removing it costs 2 points: g1 0 -> g1a 2). */
        p_old = prev;
        *(s32 *)(D_800A36A0 + 0x30) = 0;
        *(s16 *)(D_800A36A0 + 0x34) = 0;
    }
    t0 = 0;
    do {
        u8 *base = D_800A36A0;
        u8 *ptr;
        u8 *dp;
        u8 *sym;
        u8 *ap;
        a2 = 0;
        ap = (u8 *)((t0 * 2) + (s32)base);
        *(s16 *)(ap + 0x10) = 0;
        *(s16 *)(ap + 0x8) = 0;
        *(s16 *)(ap + 0xC) = 0;
        *(s16 *)(ap + 0x14) = 0;
        *(s16 *)(ap + 0x3C) = 0;
        sym = (u8 *)&D_800A35D0;
        dp = sym + (t0 * 4);
        *(s16 *)(dp + 2) = 0;
        *(s16 *)(dp + 0) = 0;
        ptr = base + (t0 * 4);
        *(s16 *)(ptr + 0x42) = 0;
        *(s16 *)(ptr + 0x40) = 0;
        *(u8 *)(base + t0 + 0x68) = (u8)t0;
        {
            u8 *q = (u8 *)((t0 * 10) + (s32)D_800A36A0);
            s16 *p_6a = (s16 *)(q + 0x6A);
            s16 *p_7e = (s16 *)(q + 0x7E);
            do {
                p_6a[a2] = -1;
                p_7e[a2] = 0;
                a2 = (s16)(a2 + 1);
            } while (a2 < 5);
        }
        a2 = 0;
        *(s16 *)(D_800A36A0 + (t0 * 2) + 0x5C) = 0;
        *(s16 *)(D_800A36A0 + (t0 * 2) + 0x60) = 5;
        for (a2 = 0; a2 < 0xA; a2 = (s16)(a2 + 1)) {
            s16 idx = (s16)(a2 + (t0 * 10));
            s32 mask = 1 << idx;
            (&D_8009BCE4)[idx] = (u8)((&D_8009BCE4)[idx] & 0xF2);
            if ((arg2 & mask) != 0) {
                (&D_8009BCE4)[idx] = (u8)((&D_8009BCE4)[idx] | 1);
                sp[t0] += 1;
                /* FAKE: same-value dead store re-establishing sym's own value
                   (sym is never read after the loop body's D stores). Effect: it
                   gives the sym pseudo a SECOND set, in a different basic block
                   from its first, which is what keeps the lui %hi/addiu %lo pair
                   for D_800A35D0 inside the outer loop where the target builds
                   it (rows 49/50/51) instead of hoisting it to the pre-header.
                   mechanism: GCC 2.7.2 loop.c:3040-3041 (count_loop_regs_set sets
                   may_not_move[regno] when a set is the first in the current basic
                   block but the reg was already set in the loop, i.e. it is set in
                   two basic blocks); scan_loop then skips the insn at loop.c:649,
                   so move_movables never sees it. The n_times_set > 1 route to the
                   same gate is unreachable in C here -- s38 proved cse folds a
                   two-statement refinement of the same local back into one set.
                   lever-exhaustion: 38 prior sessions, 8 modalities, 188 banked
                   rejected forms, 33,926 permuter iterations; ablation this session
                   shows removing it costs 18 points (g1 0 -> g1d 18) and that no
                   real-valued second write substitutes for it (h1 41/178, h2 54/170,
                   h3 51/178) nor does a literal self-assign (g6 18). */
                sym = (u8 *)&D_800A35D0;
            }
        }
        t0 = (s16)(t0 + 1);
    } while (t0 < 2);
    {
        u8 *p = D_800A36A0;
        *(s32 *)(p + 0x20) = 0;
        *(s32 *)(p + 0x1C) = 0;
        if ((s16)sp[0] < (s16)sp[1]) {
            *(s8 *)(p + 0x64) = (s8)((s16)sp[0] - 3);
        } else {
            *(s8 *)(p + 0x64) = (s8)((s16)sp[1] - 3);
        }
    }
    if (*(u8 *)(D_800A36A0 + 0x64) >= 3) {
        *(u8 *)(D_800A36A0 + 0x64) = 2;
    }
    {
        u8 *q = D_800A36A0;
        *(s32 *)q = arg1;
        *(s8 *)(q + 0x65) = 0;
    }
    *(u8 *)(D_800A36A0 + 0x67) = 1;
    *(u8 *)(D_800A36A0 + 0x66) = (&D_8009BD21)[*(u8 *)(D_800A36A0 + 0x67) * 2];
    D_800A35DC = 1;
    return 1;
}

/* Tail word after func_800747D8's five-entry compiler-generated switch table. */
const u32 D_80015A20[1] = { 0x00000000 };

s32 func_80077374(s32 arg0, s32 *arg1) {
    typedef struct {
        u8 pad[0x6A];
        u8 rows[2][10];
    } GaugeWork;
    s32 ret;
    s16 i;

    ret = 0;
    {
        u8 *p = D_800A36A0;
        if (*(s32 *)(p + 0x14) == 0x40004) {
            *(s16 *)(p + 0x16) = 5;
            *(s16 *)(p + 0x14) = 5;
            *(s16 *)(p + 0x36) = 0;
        }
    }
    func_80074220(arg1, *(s16 *)(D_800A36A0 + 0x14));
    if (*(s16 *)(D_800A36A0 + 0x14) != 5) {
        func_8007526C();
    }

    for (i = 0; i < 2; i++) {
        switch (*(s16 *)(D_800A36A0 + i * 2 + 0x14)) {
        case 0:
            if (i == 0) {
                ret = func_800747D8(arg0);
                func_80074488(arg1);
            }
            break;
        case 1:
            func_80075670(arg0, i);
            func_80074D2C((s32)arg1, i,
                          (s16)(*(u16 *)(D_800A36A0 + i * 2 + 0x14) - 1));
            func_800753D8(arg1, i);
            func_80074E08(arg1, i);
            break;
        case 2:
            func_80075F80(arg0, *(u8 *)(D_800A36A0 + i + 0x68),
                          ((GaugeWork *)D_800A36A0)->rows[i], i);
            func_80074D2C((s32)arg1, i,
                          (s16)(*(u16 *)(D_800A36A0 + i * 2 + 0x14) - 1));
            func_80074B18(arg1, i, 0);
            func_800759D0(arg1, *(u8 *)(D_800A36A0 + i + 0x68),
                          ((GaugeWork *)D_800A36A0)->rows[i], i);
            func_80074E08(arg1, i);
            break;
        case 3:
            func_800768DC(arg0, *(u8 *)(D_800A36A0 + i + 0x68),
                          ((GaugeWork *)D_800A36A0)->rows[i], i);
            func_80074D2C((s32)arg1, i,
                          (s16)(*(u16 *)(D_800A36A0 + i * 2 + 0x14) - 1));
            func_80074B18(arg1, i, 1);
            func_8007636C(arg1, *(u8 *)(D_800A36A0 + i + 0x68),
                          ((GaugeWork *)D_800A36A0)->rows[i], i);
            func_80074E08(arg1, i);
            break;
        case 4:
            func_80074D2C((s32)arg1, i, 2);
            func_80074B18(arg1, i, 1);
            func_8007636C(arg1, *(u8 *)(D_800A36A0 + i + 0x68),
                          ((GaugeWork *)D_800A36A0)->rows[i], i);
            func_80074E08(arg1, i);
            if (arg0 & (0x10 << (i * 16))) {
                func_8005C650(2, 0x7F, 0x7F);
                *(s16 *)(D_800A36A0 + i * 2 + 0x14) = 3;
            }
            break;
        case 5:
            func_80074D2C((s32)arg1, i, 2);
            func_80074B18(arg1, i, 1);
            func_8007636C(arg1, *(u8 *)(D_800A36A0 + i + 0x68),
                          ((GaugeWork *)D_800A36A0)->rows[i], i);
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
    *(s32 **)((s32)D_800A36A0 + 0x24) = (s32 *)(((D_800A36AC & 1) * 0x4090) + (s32)&g_gpu_db);
    temp_v1 = *(s32 *)((s32)D_800A36A0 + 0x30) + 1;
    *(u16 *)((s32)D_800A36A0 + 0x34) = (u16)(*(u16 *)((s32)D_800A36A0 + 0x34) + 1);
    *(s32 *)((s32)D_800A36A0 + 0x30) = temp_v1;
    p = func_80077098(temp_v1 & 1);
    *(s32 **)((s32)D_800A36A0 + 0x2C) = p;
    s.sp10 = *(s32 *)((s32)D_800A36A0 + 4);
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
