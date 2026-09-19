#define INCLUDE_ASM_USE_MACRO_INC 1
#include "common.h"
#include "include_asm.h"
#include "sound.h"
#include "game.h"
#include "code6cac.h"


#define NULL ((void *)0)

typedef struct Vec2s16 { s16 x; s16 y; } Vec2s16;
typedef struct Vec3s16 { s16 x; s16 y; s16 z; } Vec3s16;
typedef struct Vec3s32 { s32 x; s32 y; s32 z; } Vec3s32;
typedef struct Vec3 { s32 vx, vy, vz, pad; } Vec3;
typedef struct VECTOR  { s32 vx, vy, vz, pad; } VECTOR;
typedef struct SVECTOR { s16 vx, vy, vz, pad; } SVECTOR;
typedef struct CVECTOR { u8 r, g, b, cd; } CVECTOR;
typedef struct DVECTOR { s16 vx, vy; } DVECTOR;
typedef struct MATRIX  { s16 m[3][3]; u16 pad; s32 t[3]; } MATRIX;

/* GameObj: 0x100-byte polymorphic struct used across ~340 functions. The
 * field layout is the union of all observed accesses; m2c picks the type
 * that best fits each access site. Mirroring smart_match.py's layout. */
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
s32 _Pad1(void);



































extern s32 (*g_anim_func_table)(s16 *, s16 *);























































































































extern u8 D_8009BA60;
extern s32 chractar_use_pset_combo_id_table;

























extern s32 D_8009BD68;
extern s32 D_8009BD6C;
extern s32 D_8009BD70;
extern s32 D_8009BD84;
extern s32 D_8009BD88;




























































































extern s32 D_800F0FB8;
extern s32 D_800F0FBC;
extern s32 D_800F0FC0;



extern s16 D_800F10A0;
extern s16 D_800F10A2;
extern s16 D_800F10A4;
extern s32 D_800F10D0;

















extern s32 D_800F1138;







extern s32 D_800F1850;











































































    extern s32 rand(void);
    extern u8 D_8009B2E0;
    extern s32 D_8009B388;
    extern s32 D_8009B390;
    extern s32 D_800A326C;
    extern s32 D_800A3418;


















































    extern s32 D_800A3468;
    extern s32 D_800A3478;
    extern s32 D_800A347C;
    extern s32 D_800A32BC;
    extern u8 D_8009BA60;
    extern s32 D_800F10D0;
    extern s32 chractar_use_pset_combo_id_table;
    extern s32 D_800A346C;
    extern s32 D_800A3470;
    extern s32 D_800A3474;
    extern void func_80061FAC(s32, s32, s32);







extern s32 D_800A3460;



































extern s32 D_800A34E4;
extern s32 D_800A34E8;
extern s32 D_800A34EC;









extern s32 D_800A37D4;
















    extern s32 D_800A3460;
    extern volatile s32 D_800A347C;
    extern volatile s32 D_800A3478;
    extern s16 D_800F0C04;
    extern s32 D_800F0FB8;
    extern s32 D_800F0FBC;
    extern s32 D_800F0FC0;



    extern s32 D_800F1138;

















































































    extern s32 D_800A34EC;
    extern s32 D_800A37D4;
    extern s32 D_800A3724;
    extern s32 D_800A34E4;
    extern s32 D_800A34E8;
    extern s32 D_800A374C;
































































extern void disp_SetFramebufferMode(s32, s32, s32, s32);











































extern s32 ClearOTagR(s32, s32);
extern s32 func_8006E950(s32, s32 *);

extern u8 *func_8006E49C(s32, s32);







/* Padding NOP macro */
#define PAD_NOPS_1 __asm__(".section .text\n    nop\n")
#define PAD_NOPS_2 __asm__(".section .text\n    nop\n    nop\n")
#define PAD_NOPS_3 __asm__(".section .text\n    nop\n    nop\n    nop\n")

/* --- Functions from text1b segment (0x80047ED0 - 0x80079A30) --- */

extern s32 D_800A35E4;
extern u8 D_8009BD3B;
extern u8 D_8009BD3C;
extern u8 D_8009BD3D;
extern u8 D_8009BD41;
extern u8 D_8009BD42;
extern u8 D_8009BD43;


extern void func_8006D324(void);

s32 func_80077B30(s32 arg0, s32 arg1) {
    extern s32 func_8006B898(s32, s32);
    extern s32 func_8006C1FC(s32, s32);
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
extern s32 D_8009BD24;
s32* func_80077D00(void) {
    return &D_8009BD24;
}
void func_8006920C(s32*, s32);
s32 func_80077D10(s32 *a0) {
    func_8006920C(a0, a0[6]);
    func_8006920C(a0, a0[7]);
    func_8006920C(a0, a0[8]);
    func_8006920C(a0, a0[9]);
    func_8006920C(a0, a0[10]);
    return a0[1];
}
extern s32 D_800A35F4;
extern s32 D_800A35F0;
extern s32 D_800A35F8;
extern s32 D_800A35FC;
extern s32 D_800A3600;
s32 func_80077D74(s32 a0) {
    return D_800A35F4 + a0 * 44;
}
INCLUDE_ASM("asm/funcs", func_80077D94);
s32 func_800784E4(s32 arg0) {
    s32 s0;
    s32 r;

    ClearOTagR(D_800A374C, 0x1008);
    s0 = arg0 + 0x58;
    D_800A35F4 = arg0;
    D_800A35F8 = s0;
    func_8006E950(0x32, s0);
    r = func_80077D10(s0);
    func_8006E49C(r, D_800A35F4);
    D_800A35FC = 0;
    D_800A35F0 = 0;
    D_800A3600 = 0;
    return 1;
}

extern void func_80077D94(s32 *);
extern s32 D_800A35F0;

extern s32 D_800A35FC;
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
extern s32 D_800A360C;
s32 func_80078634(s32 a0) {
    return D_800A360C + a0 * 44;
}
extern s32 func_8006E480(s32, s32);
extern s32 func_8007352C(s32 *);
extern s32 SetDrawMode(s32, s32, s32, s32, s32);
extern s32 AddPrim(s32, s32);
extern s32 D_800A3608;
extern s32 *D_800A3610;
extern s32 D_800A374C;

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
       replacing it with the literal 0 was measured this session at 113 insns
       vs the target's 116 (tmp/grind/func_80078654/s5/s11_w25/, chassis
       tmp/grind/func_80078654/s11/w25_nozero.c), so the holder is
       load-bearing.  Lever-exhaustion: memory/grind/func_80078654/hypotheses.md
       s1-s11. */
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
        arg0[3] = func_8007352C(&s.a);
        SetDrawMode(arg0[5], 1, 0, func_8006E480(s.a, zero), 0);
        AddPrim(D_800A374C + (s.f * 4), arg0[5]);
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
       Single level measured INSUFFICIENT (prerequisite 3 of
       .claude/rules/do-while-zero-exception.md): depth 1 yields 6 of the 13
       references the priority inversion requires; depth 8 is the minimum
       that reaches 13 at the only wrap site that costs no delay slot.
       Lever-exhaustion: memory/grind/func_80078654/hypotheses.md s1-s11
       (11 sessions, 8 modalities, 129k permuter iterations, 14 banked
       rejected forms). */
    do { do { do { do { do { do { do { do {
    s.a = var_s0[0];
    s.b = s.a + 0xC;
    s.h = -D_800A3608;
    } while (0); } while (0); } while (0); } while (0); } while (0); } while (0); } while (0); } while (0);
    s.c = arg0[3];
    arg0[3] = func_8007352C(&s.a);
    SetDrawMode(arg0[5], 1, 0, func_8006E480(s.a, zero), 0);
    AddPrim(D_800A374C + (s.f * 4), arg0[5]);
    var_s0++;
    arg0[5] = arg0[5] + 0xC;
check:
    if (var_s0[1] != -1) goto loop;
}

extern s32 D_800A3614;
extern s32 D_800A3304;
extern s32 D_800A3608;

s32 func_80078824(s32 arg0) {
    s32 s0;
    s32 r;

    ClearOTagR(D_800A374C, 0x1008);
    s0 = arg0 + 0x58;
    D_800A360C = arg0;
    D_800A3610 = s0;
    func_8006E950(0x5F, s0);
    r = func_80078628(s0);
    func_8006E49C(r, D_800A360C);
    D_800A3304 = 0;
    D_800A3608 = 0;
    D_800A3614 = 0;
    disp_SetFramebufferMode(1, 0, 0, 0);
    return 1;
}
extern s32 D_800A3304;
extern s32 D_800A3608;

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

__asm__(
    ".section .text\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel Exec\n"
    "addiu $t2, $zero, 0xA0\n"
    "jr    $t2\n"
    "addiu $t1, $zero, 0x43\n"
    "endlabel Exec\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
PAD_NOPS_1; /* padding after bios_Exec */
__asm__(
    ".section .text\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel _bu_init\n"
    "addiu $t2, $zero, 0xA0\n"
    "jr    $t2\n"
    "addiu $t1, $zero, 0x70\n"
    "endlabel _bu_init\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
PAD_NOPS_1; /* padding after bios__bu_init_A0 */
__asm__(
    ".section .text\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel SetMem\n"
    "addiu $t2, $zero, 0xA0\n"
    "jr    $t2\n"
    "addiu $t1, $zero, 0x9F\n"
    "endlabel SetMem\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
PAD_NOPS_1; /* padding after bios_SetMem */
__asm__(
    ".section .text\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel OpenEvent\n"
    "addiu $t2, $zero, 0xB0\n"
    "jr    $t2\n"
    "addiu $t1, $zero, 0x8\n"
    "endlabel OpenEvent\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
PAD_NOPS_1; /* padding after bios_OpenEvent */
__asm__(
    ".section .text\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel CloseEvent\n"
    "addiu $t2, $zero, 0xB0\n"
    "jr    $t2\n"
    "addiu $t1, $zero, 0x9\n"
    "endlabel CloseEvent\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
PAD_NOPS_1; /* padding after bios_CloseEvent */
__asm__(
    ".section .text\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel TestEvent\n"
    "addiu $t2, $zero, 0xB0\n"
    "jr    $t2\n"
    "addiu $t1, $zero, 0xB\n"
    "endlabel TestEvent\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
PAD_NOPS_1; /* padding after bios_TestEvent */
__asm__(
    ".section .text\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel EnableEvent\n"
    "addiu $t2, $zero, 0xB0\n"
    "jr    $t2\n"
    "addiu $t1, $zero, 0xC\n"
    "endlabel EnableEvent\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
PAD_NOPS_1; /* padding after bios_EnableEvent */
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel EnterCriticalSection\n"
    "    addiu  $a0,$zero,1\n"
    "    .word 0x0000000C\n"
    "    jr  $ra\n"
    "    nop\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel ExitCriticalSection\n"
    "    addiu  $a0,$zero,2\n"
    "    .word 0x0000000C\n"
    "    jr  $ra\n"
    "    nop\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel SetSp\n"
    "    addu  $v0,$sp,$zero\n"
    "    jr  $ra\n"
    "    addu  $sp,$a0,$zero\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
PAD_NOPS_1; /* padding after func_800789D8 */
__asm__(
    ".section .text\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel open\n"
    "addiu $t2, $zero, 0xB0\n"
    "jr    $t2\n"
    "addiu $t1, $zero, 0x32\n"
    "endlabel open\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
PAD_NOPS_1; /* padding after bios_FileOpen_B */
__asm__(
    ".section .text\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel read\n"
    "addiu $t2, $zero, 0xB0\n"
    "jr    $t2\n"
    "addiu $t1, $zero, 0x34\n"
    "endlabel read\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
PAD_NOPS_1; /* padding after bios_FileRead_B */
__asm__(
    ".section .text\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel write\n"
    "addiu $t2, $zero, 0xB0\n"
    "jr    $t2\n"
    "addiu $t1, $zero, 0x35\n"
    "endlabel write\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
PAD_NOPS_1; /* padding after bios_FileWrite_B */
__asm__(
    ".section .text\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel close\n"
    "addiu $t2, $zero, 0xB0\n"
    "jr    $t2\n"
    "addiu $t1, $zero, 0x36\n"
    "endlabel close\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
PAD_NOPS_1; /* padding after bios_FileClose_B */
__asm__(
    ".section .text\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel format\n"
    "addiu $t2, $zero, 0xB0\n"
    "jr    $t2\n"
    "addiu $t1, $zero, 0x41\n"
    "endlabel format\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
PAD_NOPS_1; /* padding after bios_FormatDevice_B */
__asm__(
    ".section .text\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel firstfile\n"
    "addiu $t2, $zero, 0xB0\n"
    "jr    $t2\n"
    "addiu $t1, $zero, 0x42\n"
    "endlabel firstfile\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
PAD_NOPS_1; /* padding after bios_firstfile_B */
__asm__(
    ".section .text\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel nextfile\n"
    "addiu $t2, $zero, 0xB0\n"
    "jr    $t2\n"
    "addiu $t1, $zero, 0x43\n"
    "endlabel nextfile\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
PAD_NOPS_1; /* padding after bios_nextfile_B */
__asm__(
    ".section .text\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel ChangeClearPAD\n"
    "addiu $t2, $zero, 0xB0\n"
    "jr    $t2\n"
    "addiu $t1, $zero, 0x5B\n"
    "endlabel ChangeClearPAD\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
PAD_NOPS_1; /* padding after bios_ChangeClearPad */
s32 SetRCnt(s32 arg0, s32 arg1, s32 arg2) {
    s32 a3;
    s32 t0;
    s32 v0;
    s32 base;
    t0 = arg0 & 0xFFFF;
    a3 = 0x48;
    if (t0 >= 3) {
        return 0;
    }
    base = (t0 * 0x10) + D_8009BD6C;
    *(volatile u16 *) (base + 4) = 0;
    *(volatile u16 *) (base + 8) = arg1;
    if (((u32) t0) < 2U) {
        if (arg2 & 0x10) {
            a3 = 0x49;
        }
        v0 = arg2 & 0x1000;
        if (!(arg2 & 1)) {
            a3 |= 0x100;
        }
    } else {
        v0 = arg2 & 0x1000;
        if (t0 == 2) {
            ;
            if (!(arg2 & 1)) {
                a3 = 0x248;
            }
        }
    }
    if ((arg2 & 0x1000) != 0) {
        a3 |= 0x10;
    }
    *(volatile u16 *) (((t0 * 0x10) + D_8009BD6C) + 4) = a3;
    return 1;
}
s32 GetRCnt(s32 arg0) {
    s32 v = arg0 & 0xFFFF;
    if (v >= 3) {
        return 0;
    }
    return *(volatile u16 *)(D_8009BD6C + v * 0x10);
}
s32 StartRCnt(s32 arg0) {
    s32 v;
    volatile s32 *base;
    v = arg0 & 0xFFFF;
    base = (volatile s32 *)D_8009BD68;
    base[1] = base[1] | (&D_8009BD70)[v];
    return v < 3;
}
s32 StopRCnt(s32 arg0) {
    s32 v;
    volatile s32 *base;
    v = arg0 & 0xFFFF;
    base = (volatile s32 *)D_8009BD68;
    base[1] = base[1] & ~(&D_8009BD70)[v];
    return 1;
}
s32 ResetRCnt(s32 arg0) {
    s32 v = arg0 & 0xFFFF;
    if (v >= 3) {
        return 0;
    }
    *(volatile u16 *)(D_8009BD6C + v * 0x10) = 0;
    return 1;
}
extern s32 D_8009BD80;
void SetInitPadFlag(s32 a0) {
    D_8009BD80 = a0;
}
extern s32 D_8009BD80;
s32 ReadInitPadFlag(void) {
    return D_8009BD80;
}
void _remove_ChgclrPAD(void);
void EnterCriticalSection(void);
void _patch_pad(void);
void ExitCriticalSection(void);
void ChangeClearPAD(s32);
s32 SetPatchPad(void);
void PAD_init2(s32, s32, s32, s32);
void _send_pad(void);
extern s32 D_8009BD80;
void PAD_init(s32 a0, s32 a1, s32 a2, s32 a3) {
    _remove_ChgclrPAD();
    EnterCriticalSection();
    _patch_pad();
    ExitCriticalSection();
    ChangeClearPAD(0);
    SetPatchPad();
    PAD_init2(a0, a1, a2, a3);
    _send_pad();
    D_8009BD80 = 1;
}
void _remove_ChgclrPAD(void);
void EnterCriticalSection(void);
void _patch_pad(void);
void ExitCriticalSection(void);
void ChangeClearPAD(s32);
s32 SetPatchPad(void);
void InitPAD2(s32, s32, s32, s32);
void _send_pad(void);
extern s32 D_8009BD80;
void InitPAD(s32 a0, s32 a1, s32 a2, s32 a3) {
    _remove_ChgclrPAD();
    EnterCriticalSection();
    _patch_pad();
    ExitCriticalSection();
    ChangeClearPAD(0);
    SetPatchPad();
    InitPAD2(a0, a1, a2, a3);
    _send_pad();
    D_8009BD80 = 1;
}
void StartPAD2(void);
void ChangeClearPAD(s32);
void EnablePAD(void);
void StartPAD(void) {
    StartPAD2();
    ChangeClearPAD(0);
    EnablePAD();
}
extern s32 D_8009BD80;
void DisablePAD(void);
void StopPAD2(void);
s32 RemovePatchPad(void);
void StopPAD(void) {
    DisablePAD();
    StopPAD2();
    RemovePatchPad();
    D_8009BD80 = 0;
}
extern void EnterCriticalSection(void);
extern void ExitCriticalSection(void);
extern void SysDeqIntRP(s32, u32 *);
extern void SysEnqIntRP(s32, u32 *);

extern s32 _IsVSync(void);
extern u32 D_800F183C;
extern u32 D_800F1840;
extern u32 D_800F1838;
extern u32 D_800F1844;
s32 SetPatchPad(void) {
    u32 *v1 = &D_800F183C;
    u32 *s0 = v1 - 1;
    EnterCriticalSection();
    *v1 = (u32)_Pad1;
    D_800F1840 = (u32)_IsVSync;
    D_800F1838 = 0;
    D_800F1844 = 0;
    SysDeqIntRP(1, s0);
    SysEnqIntRP(1, s0);
    ExitCriticalSection();
    return 1;
}
void EnterCriticalSection(void);

void ExitCriticalSection(void);

s32 RemovePatchPad(void) {
    EnterCriticalSection();
    SysDeqIntRP(1, &D_800F1838);
    ExitCriticalSection();
    return 1;
}
/* PsyQ 4.0 LIBAPI PAD: _Pad1 (static) — verbatim-linked Sony object
   (census 2026-07-09). FAKE(partial-use volatile array, Ruling 3
   2026-07-10): volatile delay-counter array, only [0] used (frame 16 =
   i[3]) — SOTN vsync.c precedent; original author idiom. */
s32 _Pad1(void) {
    volatile s32 i[3];
    *(s16 *)((u8 *)D_8009BD84 + 0xA) = 0;
    i[0] = 10;
    i[0] = i[0] - 1;
    if (i[0] != -1) {
        do {
            i[0] = i[0] - 1;
        } while (i[0] != -1);
    }
    return 0;
}
s32 _IsVSync(void) {
    s32 *p = (s32 *)D_8009BD88;
    s32 ret;
    if ((p[1] & 1) == 0) return 0;
    if ((p[0] & 1) != 0) {
        ret = 1;
    } else {
        ret = 1; /* FAKE: two-set else arm defeats jump.c store-flag fold (dead-store-fake-exception) */
        ret = 0;
    }
    return ret;
}
__asm__(
    ".section .text\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel InitPAD2\n"
    "addiu $t2, $zero, 0xB0\n"
    "jr    $t2\n"
    "addiu $t1, $zero, 0x12\n"
    "endlabel InitPAD2\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
PAD_NOPS_1; /* padding after bios_InitPad */
__asm__(
    ".section .text\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel StartPAD2\n"
    "addiu $t2, $zero, 0xB0\n"
    "jr    $t2\n"
    "addiu $t1, $zero, 0x13\n"
    "endlabel StartPAD2\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
PAD_NOPS_1; /* padding after bios_StartPad */
__asm__(
    ".section .text\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel StopPAD2\n"
    "addiu $t2, $zero, 0xB0\n"
    "jr    $t2\n"
    "addiu $t1, $zero, 0x14\n"
    "endlabel StopPAD2\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
PAD_NOPS_1; /* padding after bios_StopPad */
__asm__(
    ".section .text\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel PAD_init2\n"
    "addiu $t2, $zero, 0xB0\n"
    "jr    $t2\n"
    "addiu $t1, $zero, 0x15\n"
    "endlabel PAD_init2\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
PAD_NOPS_1; /* padding after bios_OutdatedPadInitAndStart */
__asm__(
    ".section .text\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel SysEnqIntRP\n"
    "addiu $t2, $zero, 0xC0\n"
    "jr    $t2\n"
    "addiu $t1, $zero, 0x2\n"
    "endlabel SysEnqIntRP\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
PAD_NOPS_1; /* padding after bios_SysEnqIntRP */
__asm__(
    ".section .text\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel SysDeqIntRP\n"
    "addiu $t2, $zero, 0xC0\n"
    "jr    $t2\n"
    "addiu $t1, $zero, 0x3\n"
    "endlabel SysDeqIntRP\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
PAD_NOPS_1; /* padding after bios_SysDeqIntRP */

extern void (*jtbl_800A3624)(void);
/* func_80078F60 / func_80078F74: 5-insn bare tail-jump trampolines
   (lui/lw/nop/jr/nop) through the jtbl_800A3620 / jtbl_800A3624 function
   pointers that the Pad-init wrapper func_80078F88 installs at runtime. GCC
   2.7.2 has no MIPS sibling-call optimization, so no pure-C `(*fp)()` form
   emits a frameless `jr $t1` (it always builds a stack frame + jalr + jr $ra).
   Hand-coded canonical asm; user-authorized 2026-06-12. */
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel EnablePAD\n"
    "    lui  $t1,%hi(jtbl_800A3620)\n"
    "    lw  $t1,%lo(jtbl_800A3620)($t1)\n"
    "    nop\n"
    "    jr  $t1\n"
    "    nop\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel DisablePAD\n"
    "    lui  $t1,%hi(jtbl_800A3624)\n"
    "    lw  $t1,%lo(jtbl_800A3624)($t1)\n"
    "    nop\n"
    "    jr  $t1\n"
    "    nop\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel _patch_pad\n"
    "    lui  $at,%hi(D_800A3618)\n"
    "    sw  $ra,%lo(D_800A3618)($at)\n"
    "    jal  EnterCriticalSection\n"
    "    nop\n"
    "    addiu  $t2,$zero,176\n"
    "    jalr  $t2\n"
    "    addiu  $t1,$zero,87\n"
    "    lw  $v0,364($v0)\n"
    "    lui  $at,%hi(jtbl_800A3620)\n"
    "    addi  $v1,$v0,2180\n"
    "    sw  $v1,%lo(jtbl_800A3620)($at)\n"
    "    lui  $at,%hi(jtbl_800A3624)\n"
    "    addi  $v1,$v0,2196\n"
    "    addiu  $t1,$zero,11\n"
    "    sw  $v1,%lo(jtbl_800A3624)($at)\n"
    ".L80078FC4:\n"
    "    addiu  $t1,$t1,-1\n"
    "    sw  $zero,1428($v0)\n"
    "    bnez  $t1,.L80078FC4\n"
    "    addiu  $v0,$v0,4\n"
    "    jal  FlushCache\n"
    "    nop\n"
    "    lui  $ra,%hi(D_800A3618)\n"
    "    lw  $ra,%lo(D_800A3618)($ra)\n"
    "    nop\n"
    "    jr  $ra\n"
    "    nop\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel FlushCache\n"
    "    addiu $t2, $zero, 0xA0\n"
    "    jr    $t2\n"
    "    addiu $t1, $zero, 0x44\n"
    "    nop\n"
    "    lui   $t1, %hi(D_800A362C)\n"
    "    lw    $t1, %lo(D_800A362C)($t1)\n"
    "    addiu $sp, $sp, -24\n"
    "    sw    $ra, 20($sp)\n"
    "    jalr  $t1\n"
    "    nop\n"
    "    lw    $ra, 20($sp)\n"
    "    addiu $sp, $sp, 24\n"
    "    jr    $ra\n"
    "    nop\n"
    "endlabel FlushCache\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel _send_pad\n"
    "    lui  $at,%hi(D_800A3628)\n"
    "    sw  $ra,%lo(D_800A3628)($at)\n"
    "    jal  EnterCriticalSection\n"
    "    nop\n"
    "    addiu  $t2,$zero,176\n"
    "    jalr  $t2\n"
    "    addiu  $t1,$zero,87\n"
    "    lw  $v0,364($v0)\n"
    "    lui  $t2,%hi(func_800790A4)\n"
    "    lui  $t1,%hi(D_800790B4)\n"
    "    lui  $at,%hi(D_800A362C)\n"
    "    addi  $v1,$v0,1952\n"
    "    addiu  $t2,$t2,%lo(func_800790A4)\n"
    "    addiu  $t1,$t1,%lo(D_800790B4)\n"
    "    sw  $v1,%lo(D_800A362C)($at)\n"
    ".L80079064:\n"
    "    lw  $v1,0($t2)\n"
    "    addiu  $t2,$t2,4\n"
    "    sw  $v1,984($v0)\n"
    "    addiu  $v0,$v0,4\n"
    "    bne  $t2,$t1,.L80079064\n"
    "    sw  $v1,1244($v0)\n"
    "    jal  FlushCache\n"
    "    nop\n"
    "    jal  ExitCriticalSection\n"
    "    nop\n"
    "    lui  $ra,%hi(D_800A3628)\n"
    "    lw  $ra,%lo(D_800A3628)($ra)\n"
    "    lui  $v0,%hi(D_800A362C)\n"
    "    lw  $v0,%lo(D_800A362C)($v0)\n"
    "    jr  $ra\n"
    "    nop\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel func_800790A4\n"
    "    and  $v0,$v0,$s5\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
PAD_NOPS_3; /* padding after func_800790A4 */
PAD_NOPS_3; /* padding after func_800790A4 */
__asm__(
    ".set\tnoat\n"
    ".set\tnoreorder\n"
    ".set noat\n"
    ".set noreorder\n"
    "glabel _remove_ChgclrPAD\n"
    "    lui  $at,%hi(D_800A3638)\n"
    "    sw  $ra,%lo(D_800A3638)($at)\n"
    "    jal  EnterCriticalSection\n"
    "    nop\n"
    "    addiu  $t2,$zero,176\n"
    "    jalr  $t2\n"
    "    addiu  $t1,$zero,87\n"
    "    lw  $v0,364($v0)\n"
    "    addiu  $t2,$zero,9\n"
    "    addi  $v1,$v0,1580\n"
    ".L800790E8:\n"
    "    addiu  $t2,$t2,-1\n"
    "    sw  $zero,0($v1)\n"
    "    bnez  $t2,.L800790E8\n"
    "    addiu  $v1,$v1,4\n"
    "    jal  FlushCache\n"
    "    nop\n"
    "    jal  ExitCriticalSection\n"
    "    nop\n"
    "    lui  $ra,%hi(D_800A3638)\n"
    "    lw  $ra,%lo(D_800A3638)($ra)\n"
    "    nop\n"
    "    jr  $ra\n"
    "    nop\n"
    ".set\treorder\n"
    ".set\tat\n"
    ".set reorder\n"
    ".set at\n"
);
PAD_NOPS_1; /* padding after func_800790C0 */
u8* memcpy(u8 *dst, u8 *src, s32 len) {
    u8 *ret;
    if (!dst) {
        return 0;
    }
    ret = dst;
    while (len > 0) {
        *dst = *src;
        src++;
        len--;
        dst++;
    }
    return ret;
}
extern u32 D_800F1848;
s32 rand(void) {
    D_800F1848 = D_800F1848 * 0x41C64E6D + 0x3039;
    return (D_800F1848 >> 16) & 0x7FFF;
}

void srand(s32 a0) {
    D_800F1848 = a0;
}
u8 *strcpy(u8 *a0, u8 *a1) {
    u8 *v1;
    if (!a0) {
        return 0;
    }
    if (!a1) {
        return 0;
    }
    v1 = a0;
    while ((*a0++ = *a1++) != 0) {
    }
    return v1;
}
s32 strlen(u8 *a0) {
    s32 v1 = 0;
    if (!a0) {
        return 0;
    }
    while (*a0++ != 0) {
        v1++;
    }
    return v1;
}
void printf(s32 fmt, s32 a, s32 b, s32 c) {
    s32 *ap = &fmt;
    ap[1] = a;
    ap[2] = b;
    ap[3] = c;
    prnt(1, fmt, ap + 1);
}
INCLUDE_ASM("asm/funcs", prnt);
extern u8 D_8009BD8D;
u8 toupper(u8 a0) {
    u8 c = a0;
    if ((&D_8009BD8D)[c] & 2) {
        c = a0 - 0x20;
    }
    return c;
}
extern u8 D_8009BD8D;
u8 tolower(u8 a0) {
    u8 c = a0;
    if ((&D_8009BD8D)[c] & 1) {
        c = a0 + 0x20;
    }
    return c;
}
u8 *memchr(u8 *buf, s32 ch, s32 len) {
    if (buf == 0) return 0;
    if (len <= 0) return 0;
    len--;
    goto check;
found:
    return buf - 1;
check:
    if (len < 0) return 0;
    ch &= 0xFF;
loop:
    if (*buf++ == ch) goto found;
    --len;
    if (len >= 0) goto loop;
    return 0;
}
void write(s32, u8 *, s32);
void putchar(s8 arg0) {
    u8 sp10;
    s32 temp_a0;

    sp10 = arg0;
    temp_a0 = arg0 & 0xFF;
    if (temp_a0 == 9) goto loop;
    if (temp_a0 == 0xA) {
        putchar(0xD);
        D_800F1850 = 0;
        goto tail;
    }
    goto def;
loop:
    putchar(0x20);
    if ((D_800F1850 & 7) == 0) return;
    goto loop;
def:
    if ((&D_8009BD8D)[temp_a0] & 0x97) {
        D_800F1850 += 1;
    }
tail:
    write(1, &sp10, 1);
}
INCLUDE_ASM("asm/funcs", sprintf);
