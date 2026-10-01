#define INCLUDE_ASM_USE_MACRO_INC 1
#include "common.h"
#include "include_asm.h"
#include "bios.h"
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







extern s32 column;











































































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
    extern s32 g_gpu_ot_ptr;
































































extern void gpu_SetDrawEnvBg(s32, s32, s32, s32);











































extern s32 ClearOTagR(s32, s32);
extern void func_8006E950(s32, s32 *);

extern s32 func_8006E49C(s32, s32 *);







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
typedef struct {
    u8 chr;
    u8 unk1;
} Unk8009BD24Record;
extern Unk8009BD24Record D_8009BD24[2][5];
s32* func_80077D00(void) {
    return (s32 *)D_8009BD24;
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
extern s32 func_8006E480(s32, s32);
extern s32 func_8007352C(s32);
extern s32 SetDrawMode(s32, s32, s32, s32, s32);
extern s32 AddPrim(s32, s32);
extern void LoadImage(s32, s32);
extern s32 g_gpu_ot_ptr;

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
       literal re-materializes `li a1,0x20` per call: 12/465 vs 0/468.
       Same shape as func_80078654's `zero` and func_80070C70's `c60`.
       Lever exhaustion: pre-slim-2026-10-01:memory/grind/func_80077D94/evidence.md. */
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
       a goto) measures 23/470 vs 0/468.
       Lever exhaustion: pre-slim-2026-10-01:memory/grind/func_80077D94/evidence.md. */
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
                   load displacement and the giv is not reduced: 9/467.
                   Lever exhaustion: pre-slim-2026-10-01:memory/grind/func_80077D94/evidence.md. */
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
               a separate pseudo and costs a `move` at the join: 26/467 vs
               0/468. Lever exhaustion: pre-slim-2026-10-01:memory/grind/func_80077D94/evidence.md. */
            if (D_800A35F0 < w->on + 60) {
                s.has_color = 1;
                v = ((D_800A35F0 - w->on) * 112) / 60;
                s.semi = 1;
                s.col_r = s.col_g = s.col_b = v;
            } else if (D_800A35F0 >= w->off && D_800A35F0 < w->off + 60) {
                /* FAKE: named intermediate for the frame 60 ticks back.
                   Mechanism: inline, fold reassociates off - (cnt - 60)
                   into (off + 60) - cnt; the target computes cnt - 60
                   first (`addiu v0,a1,-60`). Inline: 16/469;
                   60 - (cnt - off): 16/469.
                   Lever exhaustion: pre-slim-2026-10-01:memory/grind/func_80077D94/evidence.md. */
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
extern s32 func_8007352C(s32);
extern s32 SetDrawMode(s32, s32, s32, s32, s32);
extern s32 AddPrim(s32, s32);
extern s32 D_800A3608;
extern s32 *D_800A3610;
extern s32 g_gpu_ot_ptr;

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
    arg0[3] = func_8007352C((s32)&s.a);
    SetDrawMode(arg0[5], 1, 0, func_8006E480(s.a, zero), 0);
    AddPrim(g_gpu_ot_ptr + (s.f * 4), arg0[5]);
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

BIOS_A_FUNCTION(Exec, 0x43);
BIOS_A_FUNCTION(_bu_init, 0x70);
BIOS_A_FUNCTION(SetMem, 0x9F);
BIOS_B_FUNCTION(OpenEvent, 0x8);
BIOS_B_FUNCTION(CloseEvent, 0x9);
BIOS_B_FUNCTION(TestEvent, 0xB);
BIOS_B_FUNCTION(EnableEvent, 0xC);
INCLUDE_ASM("asm/funcs", EnterCriticalSection);
INCLUDE_ASM("asm/funcs", ExitCriticalSection);
INCLUDE_ASM("asm/funcs", SetSp);
PAD_NOPS_1; /* padding after func_800789D8 */
BIOS_B_FUNCTION(open, 0x32);
BIOS_B_FUNCTION(read, 0x34);
BIOS_B_FUNCTION(write, 0x35);
BIOS_B_FUNCTION(close, 0x36);
BIOS_B_FUNCTION(format, 0x41);
BIOS_B_FUNCTION(firstfile, 0x42);
BIOS_B_FUNCTION(nextfile, 0x43);
BIOS_B_FUNCTION(ChangeClearPAD, 0x5B);
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
extern u32 patch0_plus_0x4;
extern u32 patch0_plus_0x8;
extern u32 patch0;
extern u32 patch0_plus_0xC;
s32 SetPatchPad(void) {
    u32 *v1 = &patch0_plus_0x4;
    u32 *s0 = v1 - 1;
    EnterCriticalSection();
    *v1 = (u32)_Pad1;
    patch0_plus_0x8 = (u32)_IsVSync;
    patch0 = 0;
    patch0_plus_0xC = 0;
    SysDeqIntRP(1, s0);
    SysEnqIntRP(1, s0);
    ExitCriticalSection();
    return 1;
}
void EnterCriticalSection(void);

void ExitCriticalSection(void);

s32 RemovePatchPad(void) {
    EnterCriticalSection();
    SysDeqIntRP(1, &patch0);
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
BIOS_B_FUNCTION(InitPAD2, 0x12);
BIOS_B_FUNCTION(StartPAD2, 0x13);
BIOS_B_FUNCTION(StopPAD2, 0x14);
BIOS_B_FUNCTION(PAD_init2, 0x15);
BIOS_C_FUNCTION(SysEnqIntRP, 0x2);
BIOS_C_FUNCTION(SysDeqIntRP, 0x3);

extern void (*jtbl_800A3624)(void);
/* func_80078F60 / func_80078F74: 5-insn bare tail-jump trampolines
   (lui/lw/nop/jr/nop) through the jtbl_800A3620 / jtbl_800A3624 function
   pointers that the Pad-init wrapper func_80078F88 installs at runtime. GCC
   2.7.2 has no MIPS sibling-call optimization, so no pure-C `(*fp)()` form
   emits a frameless `jr $t1` (it always builds a stack frame + jalr + jr $ra).
   Hand-coded canonical asm; user-authorized 2026-06-12. */
INCLUDE_ASM("asm/funcs", EnablePAD);
INCLUDE_ASM("asm/funcs", DisablePAD);
INCLUDE_ASM("asm/funcs", _patch_pad);
INCLUDE_ASM("asm/funcs", FlushCache);
INCLUDE_ASM("asm/funcs", _send_pad);
INCLUDE_ASM("asm/funcs", func_800790A4);
PAD_NOPS_3; /* padding after func_800790A4 */
PAD_NOPS_3; /* padding after func_800790A4 */
INCLUDE_ASM("asm/funcs", _remove_ChgclrPAD);
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
