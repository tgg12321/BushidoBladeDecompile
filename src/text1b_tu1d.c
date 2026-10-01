/* func_8006E534..func_80073200, split out of text1b_tu1c.c (tail in text1b_tu1e.c)
 * and compiled -G8 (Makefile GP_FILES; per-file -G8 by proof, owner rulings Q10,
 * Q44 and Q54). Their original bytes reach the small globals 0x800A32E8..0x800A35CA
 * (40 addresses, 403 accesses) straight off $gp, which the original compiler
 * emits only at -G8, and no function outside this range gp-accesses any of them.
 * func_8006E8CC, func_8006E950, func_8006EA28 and func_80072F30 have no gp access
 * and compile to identical bytes at -G8 and -G0 (Q54). Proof and both-ways
 * listings: pre-slim-2026-10-01:memory/grind/func_80070F78/g8-evidence.md. */
#define INCLUDE_ASM_USE_MACRO_INC 1
#include "common.h"
#include "include_asm.h"
#include "sound.h"
#include "game.h"
#include "code6cac.h"

/* Declarations from the file this TU was split from (text1b_tu1c.c). */
extern s32 func_8005C2A8(s32 *, s16, s32);
extern s32 ClearOTagR(s32, s32);
extern s32 D_800A36AC;
extern s32 rcos();
extern s32 rsin();
void DrawSync(s32);
extern s32 g_gpu_ot_ptr;
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
extern s32 func_80036EA8();
s32 game_FrameLoop();                           /* extern */
s32 cdrom_StartRead(s32, s32);               /* extern */
extern s32 func_80036EA8(s32, s32);
s32 func_80036EA8(s32, s32);
s32 game_FrameLoop(void);
s32 cdrom_StartRead(s32, s32);
s32 func_8005C2A8(s32 *, s16, s32);
extern s32 game_FrameLoop(void);
extern s32 cdrom_StartRead(s32, s32);
extern s32 *func_80077D00(void);
s32 func_8005C2A8(s32 *hdr, s16 vabid, s32 arg2);
void func_8005C650(s32 a0, s32 a1, s32 a2);
void func_8005C6D0(void);
extern s32 func_80073728(s32, s32);
typedef struct {
    void *p0;
    s32 *p1;
    s32 pad08;
    s32 ret;
    s32 zero10;
    s32 one14;
    s32 zero18;
    s32 zero1C;
    s32 c20;
    s32 c24;
    s8 byte28;
    u8 byte29;
    u8 byte2A;
    u8 byte2B;
} S46C;
extern s32 func_8007352C(s32);
extern s32 func_8006E480(s32, s32);
extern s32 SetDrawMode(s32, s32, s32, s32, s32);
extern s32 AddPrim(s32, s32);
extern s32 SetSemiTrans(void *, s32);
extern s32 SetTile(void *);
extern u8 g_gpu_db;
extern s32 SetDrawArea();
extern s32 SetPolyG4();
extern s32 func_8006E480();
extern s32 func_8007352C();
extern s32 snd_StopAll(void);
extern s32 MoveImage(u8 *, s32, s32);
void func_8006920C(s32 *, s32);
void func_8006920C(s32 *a0, s32 a1);
void func_80069898(GameObj *arg0, u16 *arg1, s32 arg2);
s32 *func_80077D00(void);
extern void SetDrawOffset();
extern u8 g_gpu_db;  /* one type per TU: all uses here take (s32)&g_gpu_db */
extern void LoadImage(u8 *, s32);
extern void DrawSync(s32);
void func_8006E440(s32 *a0);
s32 func_8006E480(s32 a0_addr, s32 a1);
s32 func_8006E49C(s32 arg0, s32 *arg1);

typedef struct SelectEntryE534 {
    u8 value;
    u8 unk1;
} SelectEntryE534;

extern SelectEntryE534 D_8009BC40[][6];
extern u8 D_8009BC7C[];
extern u8 D_800A32E8;
extern u8 D_800A32E9;
extern u8 D_800A32EC[8];
extern s16 D_800A3554;
extern s16 D_800A3558;
extern s32 D_800A3568;
extern s32 D_800A356C;
extern s16 D_800A3570;
extern s16 D_800A3578; /* s16: func_80070188, func_80070F78 and func_800720FC read it with lh; func_8006EC0C reads its halves through u8/u16 values. */
extern s16 D_800A357C;
extern s16 D_800A3580;
extern s16 D_800A3588[];
extern s16 D_800A358C[];
extern s16 D_800A3598;
extern s16 D_800A359C;
extern s32 D_800A35A0;
extern s32 D_800A35A8;
extern s32 D_800A35AC;
extern s32 D_800A35B0;
extern s16 D_800A35B4;
extern s16 D_800A35B8;
extern s32 D_800A35BC;
extern void *D_800A35C4;
extern s32 g_gpu_ot_ptr;

typedef struct RectE534 {
    s16 x;
    s16 y;
    s16 w;
    s16 h;
} RectE534;

s32 func_8006E534(s32 arg0, s32 arg1, u8 *arg2, u32 arg3) {
    RectE534 rect;
    s16 i;
    u8 value;

    ClearOTagR(g_gpu_ot_ptr, 0x1008);
    D_800A32E8 = 0x7F;
    D_800A35BC = *(s32 *)(arg2 + 0x14) & 0xF;
    D_800A35AC = arg0;
    D_800A3568 = (s32)arg2;
    D_800A3558 = 0;
    D_800A3554 = 0;
    D_800A32E9 = 0;
    D_800A35B0 = arg1;
    D_800A356C = arg0 + 0x58;
    D_800A35A8 = arg0 + 0x58;
    snd_StopAll();

    switch (D_800A35BC) {
    case 0:
        func_8006E950(5, D_800A356C);
        break;
    case 4:
    case 6:
        func_8006E950(3, D_800A356C);
        break;
    case 2:
        if (*(s32 *)(D_800A3568 + 0x14) & 0x20000) {
            func_8006E950(3, D_800A356C);
        } else {
            func_8006E950(4, D_800A356C);
        }
        break;
    case 1:
    case 3:
        func_8006E950(4, D_800A356C);
        break;
    }

    D_800A356C = func_8006EA28((s32 *)D_800A356C);
    D_800A356C = func_8006E49C(D_800A356C, (s32 *)D_800A35AC);
    D_800A3560.word = -1;
    D_800A3570 = 0;
    D_800A3578 = 0;
    D_800A357C = 0;
    D_800A3580 = 0;
    D_800A35A0 = 0;
    D_800A3588[0] = 0;
    D_800A358C[0] = 0;
    D_800A3588[1] = 2;
    D_800A358C[1] = 0;
    D_800A3560.rec[0].unk1 = D_8009BC40[0][0].value;
    D_800A3560.rec[1].unk1 = D_8009BC40[0][2].value;

    for (i = 0; i < 0x16; i++) {
        value = D_8009BC7C[i] & 0xFA;
        D_8009BC7C[i] = value;
        if (arg3 & (1 << i)) {
            D_8009BC7C[i] = value | 1;
        }
    }

    if (D_800A35BC < 4) {
        if (D_800A35BC >= 0) {
            D_8009BC7C[D_8009BC40[4][1].value] |= 4;
            D_8009BC7C[D_8009BC40[4][3].value] |= 4;
        }
    }
    D_800A35B8 = (arg3 >> 20) & 3;
    D_800A35B4 = 5;
    {
        s32 b = D_800A3588[0];
        s32 c = D_800A358C[0];
        D_8009BC7C[D_8009BC40[c][b].value] |= 4;
    }
    if (D_800A35B0 != 0) {
        s32 b = D_800A3588[1];
        s32 c = D_800A358C[1];
        D_8009BC7C[D_8009BC40[c][b].value] |= 4;
    }

    __builtin_memcpy(&rect, D_800A32EC, 8);
    D_800A359C = 0;
    D_800A3598 = 0;
    DrawSync(0);
    MoveImage(&rect, 0x3C0, 0x1FE);
    DrawSync(0);
    D_800A35C4 = (void *)D_800A356C;
    D_800A356C += 0x14;
    *(s32 *)((s32)D_800A35C4 + 8) = 0;
    *(s32 *)((s32)D_800A35C4 + 0xC) = 0;
    return 1;
}

extern s32 D_800A35AC;
s32 func_8006E8AC(s32 a0) {
    return D_800A35AC + a0 * 44;
}
s32* func_80077D00(void);
void DrawSync(s32);

void func_8006E8CC(s32 *a0) {
    s32 *p;
    s32 data;
    s16 rect[4];
    p = func_80077D00();
    if (p[8] & 1) {
        data = a0[4];
    } else {
        data = a0[3];
    }
    rect[0] = 0;
    rect[1] = 0x1E0;
    rect[2] = 0x280;
    rect[3] = 0x20;
    DrawSync(0);
    LoadImage(rect, data);
    DrawSync(0);
}
void func_8006E950(s32 *a0, s32 *a1) {
    s32 *s1 = a1;
    s32 s2;
    s32 s3;
    s32 s0;
    s32 s0_addr;
    s32 v0;
    s16 rect[4];

    s0_addr = (s32)a0;
    game_FrameLoop();
    v0 = func_80036EA8(2, s0_addr);
    cdrom_StartRead(v0, (s32)s1);
    game_FrameLoop();
    s2 = 0x280;
    func_8006E440(s1);

    s3 = ((s32 *)((unsigned char *)s1 + 8))[0];
    s0 = 0x1DC;

    rect[0] = (s16)s2;
    rect[1] = 0;
    rect[2] = 0x180;
    rect[3] = (s16)s0;
    DrawSync(0);
    LoadImage(rect, s3);

    rect[2] = 0x170;
    rect[0] = (s16)s2;
    rect[1] = (s16)s0;
    rect[3] = 0x24;
    DrawSync(0);
    LoadImage(rect, s3 + 0x59400);

    func_8006E8CC(s1);
}
void func_8006920C(s32 *, s32);
s32 func_8005C2A8(s32 *, s16, s32);
s32 func_8006EA28(s32 *a0) {
    func_8006920C(a0, a0[21]);
    func_8006920C(a0, a0[22]);
    func_8006920C(a0, a0[23]);
    func_8006920C(a0, a0[24]);
    func_8006920C(a0, a0[25]);
    func_8006920C(a0, a0[26]);
    func_8006920C(a0, a0[27]);
    func_8006920C(a0, a0[28]);
    func_8006920C(a0, a0[29]);
    func_8005C2A8(a0[0], 1, a0[1]);
    return a0[1];
}
extern s32 D_8009BC1C;
extern s32 D_800A3548;
extern s32 D_800A354C;
extern s16 D_800A3580;
extern s32 D_800A35A0;
extern void *D_800A35A4;
extern s32 D_800A35A8;
extern s32 D_800A35BC;
extern s32 D_800A35C0;
extern void *D_800A35C4;
void func_8006EC0C(void);
void func_8006F528(s32 *);
s32 func_8006EACC(s32 arg0, s32 arg1) {
    s32 sp10[10];
    s32 *temp_v0;
    s32 temp_v1;

    D_800A35C0 = ((D_800A36AC & 1) * 0x4090) + (s32)&g_gpu_db;
    D_800A3548 = arg0;
    D_800A354C = arg1;
    if (D_800A35BC == 2) {
        D_800A354C = arg1 & 0xFFFF;
    }
    func_8006EC0C();
    temp_v1 = ((s32 *)D_800A35C4)[3] + 1;
    ((s32 *)D_800A35C4)[2] = ((s32 *)D_800A35C4)[2] + 1;
    ((s32 *)D_800A35C4)[3] = temp_v1;
    temp_v0 = (s32 *)func_8006E8AC(temp_v1 & 1);
    sp10[0] = D_800A35A8;
    sp10[1] = temp_v0[0];
    sp10[3] = temp_v0[2];
    sp10[4] = temp_v0[4];
    sp10[5] = temp_v0[3];
    sp10[6] = temp_v0[5];
    sp10[7] = temp_v0[6];
    D_800A35A4 = temp_v0;
    sp10[8] = temp_v0[7];
    sp10[9] = temp_v0[8];
    if ((u16)(D_800A3580 - 2) >= 2U) {
        func_8006F528(sp10);
    }
    ((void (*)(s32 *))(&D_8009BC1C)[D_800A3580])(sp10);
    return D_800A35A0;
}
extern s16 D_800A3570;
extern s16 D_800A3578;
extern s16 D_800A3584;
void func_8006EC0C(void) {
    s32 state = *(u8 *)&D_800A3578;  /* entry dispatch reads low byte only -> lbu */

    if (state == 2) goto fade_out;
    if (state < 3) {
        if (state == 1) goto ramp_up;
        goto done;
    }
    if (state == 3) goto ramp_up;
    if (state == 4) goto fade_out;
    goto done;

ramp_up:
    D_800A3570 = (s16)(D_800A3570 + 0x20);
    if ((s32)(s16)D_800A3570 < 0x1E8) goto done;
    {
        s16 v3584 = D_800A3584;
        u16 word = D_800A3578;
        D_800A3570 = 0x1E8;
        D_800A3580 = v3584;
        if ((word >> 8) != 0) goto done;
        D_800A3578 = word + 1;
    }
    goto done;

fade_out:
    if (D_800A3570 == 0x1E8) {
        func_8005C650(5, 0x7F, 0x7F);
    }
    D_800A3570 = (s16)(D_800A3570 - 0x20);
    if ((s32)(s16)D_800A3570 > 0) goto done;
    D_800A3570 = 0;
    D_800A3578 = 0;

done: ;
}
extern s16 D_800A3554;
extern s32 D_800A35B0;
extern u8 D_8009BC7C[];
extern SelectEntryE534 D_8009BC40[][6];
extern s16 D_800A3588[];
extern s16 D_800A358C[];
extern u8 D_800A32E8;
extern u8 D_800A32E9;
extern s32 D_800A3568;
extern s32 D_800A35A8;
extern s32 D_800A35BC;
extern void *D_800A35C4;
typedef struct {
    s16 x;
    s16 y;
    s16 w;
    s16 h;
} Rect_8006ECF4;
extern Rect_8006ECF4 D_800A32F4;

void func_8006ECF4(s32 arg0) {
    S46C s;
    s32 v0;
    s32 s3;
    s32 s0;
    s32 sel;
    s32 a2;
    s16 i;
    Rect_8006ECF4 rectbuf;

    s.one14 = 0x14;
    s.c20 = 0x200;
    s.zero18 = 0;
    s.zero1C = 0;
    s.c24 = 0x100;

    v0 = *(s32 *)arg0;
    s3 = *(s32 *)(v0 + 0x54);
    s0 = s3 + 0xC;

    for (i = 0; i < D_800A35B0 + 1 + D_800A3554; i++) {
        s32 b = D_800A3588[i];
        s32 c = D_800A358C[i];
        sel = D_8009BC40[c][b].value;
        if (D_8009BC7C[sel] & 1) {
            s.zero10 = 0;
            if (*(s32 *)((s32)D_800A35C4 + 8) & 4) {
                s.byte28 = 1;
            } else {
                s.byte28 = 0;
            }
            s.byte29 = 0x94;
            s.byte2A = 0x80;
            s.byte2B = 0x6E;
        } else {
            s.zero10 = 1;
            s.byte28 = 1;
            s.byte2B = 0;
            s.byte2A = 0;
            s.byte29 = 0;
        }

        switch (sel) {
        case 12:
            a2 = *(s32 *)((s32)D_800A35A8 + 0x84);
            s.p0 = (void *)(s0 + 0x108);
            break;
        case 13:
            a2 = *(s32 *)((s32)D_800A35A8 + 0x88);
            s.p0 = (void *)(s0 + 0x114);
            break;
        case 14:
            a2 = *(s32 *)((s32)D_800A35A8 + 0x8C);
            s.p0 = (void *)(s0 + 0x120);
            break;
        case 0:
            a2 = *(s32 *)((s32)D_800A35A8 + 0x90);
            s.p0 = (void *)(s0 + 0x12C);
            break;
        case 3:
            a2 = *(s32 *)((s32)D_800A35A8 + 0x94);
            s.p0 = (void *)(s0 + 0x138);
            break;
        default:
            s.p0 = (void *)(s0 + sel * 12);
            goto p1_dispatch;
        }
        /* FAKE: the default `s.p0 = (void *)(s0 + sel * 12);` is written TWICE --
         * once in the switch default above and once at `default_p0:` -- instead of
         * sharing one copy behind the label.  Measured: the label-shared single-copy
         * form scores 20 (same 209 instruction count, different block layout) because
         * jump2's cross-jump merge direction flips.  The statement is real on both
         * paths (the target recomputes p0 for i == 0 at .L8006EF14).
         * Family: duplicated-statement-into-arms (owner ruling 2026-07-01). */
        if (i == 0) goto default_p0;
        if (D_800A32E8 != sel || D_800A32E9 != D_800A3554) {
            rectbuf = D_800A32F4;
            LoadImage((s32)&rectbuf, a2);
            DrawSync(0);
        }
        goto p1_dispatch;
    default_p0:
        s.p0 = (void *)(s0 + sel * 12);
    p1_dispatch:;

        if (D_800A35B0 != 0) goto p1_idx;
        if (D_8009BC7C[D_800A3560.rec[i].unk1] & 2) goto p1_idx;
        if (D_800A35BC != 2) goto p1_fallback;
        if (*(s32 *)((s32)D_800A3568 + 0x14) & 0x20000) goto p1_idx;
    p1_fallback:
        s.p1 = (s32 *)*(s32 *)(s3 + 4);
        goto p1_done;
    p1_idx:
        s.p1 = (s32 *)*(s32 *)(s3 + (s32)i * 4);
    p1_done:;

        s.ret = *(s32 *)(arg0 + 4);
        if (i != 0) {
            *(s32 *)(arg0 + 4) = func_80073728((s32)&s, 1);
        } else {
            *(s32 *)(arg0 + 4) = func_80073728((s32)&s, 0);
        }
    }
}
extern s16 D_800A3550;


void func_8006F038(s32 arg0) {
    s32 temp_s0;
    s16 v1;
    s16 v2;
    s16 v3;

    temp_s0 = *((s32 *)(((s32)arg0) + 0x14));
    SetTile(temp_s0);
    v1 = D_800A3550;
    *((s16 *)(((s32)temp_s0) + 8)) = 0;
    *((s16 *)(((s32)temp_s0) + 0xA)) = 0;
    *((s8 *)(((s32)temp_s0) + 4)) = v1;
    v2 = D_800A3550;
    *((s16 *)(((s32)temp_s0) + 0xC)) = 0x280;
    *((s8 *)(((s32)temp_s0) + 5)) = v2;
    v3 = D_800A3550;
    *((s16 *)(((s32)temp_s0) + 0xE)) = 0xF0;
    *((s8 *)(((s32)temp_s0) + 6)) = v3;
    SetSemiTrans(temp_s0, 1);
    AddPrim(g_gpu_ot_ptr, temp_s0);
    temp_s0 += 0x10;
    *((s32 *)(((s32)arg0) + 0x14)) = temp_s0;
    SetDrawMode(*((s32 *)(((s32)arg0) + 0x18)), 1, 0, 0x40, 0);
    AddPrim(g_gpu_ot_ptr, *((s32 *)(((s32)arg0) + 0x18)));
    *((s32 *)(((s32)arg0) + 0x18)) = (s32)(*((s32 *)(((s32)arg0) + 0x18)) + 0xC);
}
extern s16 D_800A355C;
extern s16 D_800A35C8[];
extern s16 D_800A3590[];
extern void func_80072E10(s32);
extern void func_80073200(s32);
extern s32 func_80073C78();

typedef struct {
    u8 unk0[2];
    u8 count;
    u8 unk3[5];
    s16 unk8;
    u8 unkA[2];
} Hdr_8006F100;

typedef struct {
    u16 x;
    u16 y;
    u8 unk4[2];
    u8 w;
    u8 h;
} Ent_8006F100;

typedef struct {
    Hdr_8006F100 hdr[2];
    Ent_8006F100 ent[1];
} Obj_8006F100;

typedef struct {
    Hdr_8006F100 *hdr;
    Ent_8006F100 *ent;
    s32 unk08;
    s32 ret;
    s32 unk10;
    s32 unk14;
    s32 x;
    s32 y;
    s32 scale_x;
    s32 scale_y;
    u8 unk28;
} Spr_8006F100;

void func_8006F100(s32 arg0) {
    Spr_8006F100 s;
    s32 i;
    s32 base;
    Obj_8006F100 *obj;
    s32 sel;
    s32 t0;
    s16 t1;
    s16 t2;
    s16 dx;
    s16 dy;

    if (D_800A3550 != 0) {
        D_800A3550 += 0x10;
        if (D_800A3550 >= 0x100) {
            D_800A3550 = 0xFF;
            func_8006F038(arg0);
            D_800A3550 = 0;
        } else {
            func_8006F038(arg0);
        }
        func_8006ECF4(arg0);
        func_80072E10(arg0);
        func_80073200(arg0);
        return;
    }

    if (D_800A355C >= 0x79) {
        D_800A3584 = 3;
    }
    s.scale_x = 0x100;
    s.scale_y = 0x100;
    s.unk14 = 0x13;
    s.unk10 = 0;
    s.unk28 = 0;
    base = *(s32 *)(D_800A35A8 + 0x58);
    for (i = 0; i < 1 + D_800A35B0 + D_800A3558; i++) {
        obj = *(Obj_8006F100 **)(base + D_800A3560.rec[i].unk2 * 4);
        sel = 1;
        if (D_800A35BC == 2 && !(*(s32 *)(D_800A3568 + 0x14) & 0x20000) && i == 1) {
            if (D_8009BC7C[D_800A3560.rec[0].unk1] & 2) {
                sel = 1;
            } else {
                sel = 0;
            }
        } else if (D_8009BC7C[D_800A3560.rec[i].unk1] & 2) {
            sel = 0;
        }
        s.hdr = &obj->hdr[sel];
        if (i != 0) {
            s.hdr->unk8 = 0x40;
        } else {
            s.hdr->unk8 = 0;
        }
        s.ent = obj->ent;
        t0 = -(D_800A35C8[i] * 800) / 20;
        t1 = t0;
        {
            s32 idx = s.hdr->count - 1;
            dx = s.ent[idx].x + s.ent[idx].w - obj->ent[0].x;
            dy = s.ent[idx].y + s.ent[idx].h - obj->ent[0].y;
        }
        t2 = -(D_800A35C8[i] * 664) / 20;
        if (i != 0) {
            t1 = -t0;
        }
        {
            /* FAKE: constant-holder local (named-local-fake-exception). The
             * target adds the screen-centre offset to the (sign-extended)
             * shake offset BEFORE adding the table origin -- `addiu 0x140`
             * then `addu tbl,v0`. Spelled with the literal, fold-const.c's
             * `associate:` (split_tree) reassociates `tbl + (t1 + 0x140)` to
             * `(tbl + t1) + 0x140` / `tbl + 0x140 + t1` and the order is lost;
             * a local operand is not TREE_CONSTANT, so the tree keeps the
             * grouping and cse propagates 0x140 back into the addiu
             * (byte-neutral: 266 insns either way). Block scope keeps loop.c
             * from hoisting it into a callee-save. */
            s32 cx = 0x140;

            s.x = D_8009BC94[i][D_800A3590[i]].x + (t1 + cx) - ((dx * s.scale_x >> 8) / 2);
        }
        {
            /* FAKE: same constant-holder mechanism as `cx` above, for the
             * vertical centre 0x9D. */
            s32 cy = 0x9D;

            s.y = D_8009BC94[i][D_800A3590[i]].y + (t2 + cy) - ((dy * s.scale_y >> 8) / 2);
        }
        s.ret = *(s32 *)(arg0 + 4);
        if (i != 0) {
            *(s32 *)(arg0 + 4) = func_80073C78(&s, 0x1C0, 1);
        } else {
            *(s32 *)(arg0 + 4) = func_80073C78(&s, 0xE40, 0);
        }
        if (D_800A35C8[i] > 0) {
            D_800A35C8[i]--;
        }
        if (D_800A35C8[i] == 10) {
            func_8005C650(7, 0x7F, 0x7F);
        }
        if (D_800A35C8[i] < 0) {
            D_800A35C8[i] = 0;
            func_8005C650(8, 0x7F, 0x7F);
        }
    }
    D_800A355C++;
}
typedef struct {
    u16 x;
    u16 y;
    u16 w;
    u16 h;
} Rect_8006F528;

void func_8006F528(s32 *arg0) {
    S46C s;
    Rect_8006F528 rect;
    s32 *ctx;
    u8 *prim;
    s16 state;
    s32 *p1;

    s.zero10 = 0;
    s.zero1C = 0;
    s.c20 = 0x100;
    s.c24 = 0x100;
    s.byte28 = 0;

    ctx = *(s32 **)(D_800A35A8 + 0x5C);
    s.one14 = 0x10;
    {
        s32 base = ctx[0];

        state = D_800A3578 & 0xFF;
        s.p0 = (void *)base;
        p1 = (s32 *)(base + 0xC);
        if (state >= 3) {
            s.zero18 = D_800A3570;
        } else {
            s.zero18 = 0;
        }
        s.p1 = p1;
    }
    s.ret = arg0[1];
    arg0[1] = func_80073728((s32)&s, 0);

    s.p1 = (s32 *)((s32)s.p1 + 8);
    s.c24 = 0x4C00;
    s.ret = arg0[1];
    arg0[1] = func_80073728((s32)&s, 0);

    s.c24 = 0x100;
    {
        s32 base = ctx[1];

        s.p0 = (void *)base;
        p1 = (s32 *)(base + 0xC);
        s.p1 = p1;
    }
    s.ret = arg0[1];
    arg0[1] = func_80073728((s32)&s, 0);

    {
        s32 base = ctx[0];

        s.p0 = (void *)base;
        p1 = (s32 *)(base + 0xC);
        if (state < 3) {
            s.zero18 = -D_800A3570 + 0x200;
        } else {
            s.zero18 = 0x200;
        }
        s.p1 = p1;
    }
    s.ret = arg0[1];
    arg0[1] = func_80073728((s32)&s, 1);

    s.p1 = (s32 *)((s32)s.p1 + 8);
    s.c24 = 0x4C00;
    s.ret = arg0[1];
    arg0[1] = func_80073728((s32)&s, 0);

    s.c24 = 0x100;
    {
        s32 base = ctx[1];

        s.p0 = (void *)base;
        p1 = (s32 *)(base + 0xC);
        s.p1 = p1;
    }
    s.ret = arg0[1];
    arg0[1] = func_80073728((s32)&s, 1);

    if (state >= 3) {
        rect.x = D_800A3570 + 0x4C;
    } else {
        rect.x = 0x4C;
    }
    rect.y = ((u16 *)D_800A35C0)[1] + 0x7C;
    rect.w = 0x1E8 - D_800A3570;
    rect.h = 0x54;
    SetDrawArea(arg0[7], &rect);
    AddPrim(g_gpu_ot_ptr + 0x3C, arg0[7]);
    arg0[7] += 0xC;

    rect.x = ((u16 *)D_800A35C0)[0];
    rect.y = ((u16 *)D_800A35C0)[1];
    rect.w = ((u16 *)D_800A35C0)[2];
    rect.h = ((u16 *)D_800A35C0)[3];
    SetDrawArea(arg0[7], &rect);
    AddPrim(g_gpu_ot_ptr + 0x18, arg0[7]);
    arg0[7] += 0xC;

    switch (state) {
    case 2:
    case 4: {
        u8 *offset;
        s32 x;
        s32 offset_x;

        offset = D_800A35C4;
        x = *(s16 *)(D_800A35C0 + 8);
        if (state == 2) {
            offset_x = x - D_800A3570;
        } else {
            offset_x = x + D_800A3570;
        }
        *(s16 *)(offset + 0x10) = offset_x;
        *(u16 *)(D_800A35C4 + 0x12) = *(u16 *)(D_800A35C0 + 0xA);
        SetDrawOffset(arg0[8], D_800A35C4 + 0x10);
        AddPrim(g_gpu_ot_ptr + 0x3C, arg0[8]);
        arg0[8] += 0xC;

        *(u16 *)(D_800A35C4 + 0x10) = *(u16 *)(D_800A35C0 + 8);
        *(u16 *)(D_800A35C4 + 0x12) = *(u16 *)(D_800A35C0 + 0xA);
        SetDrawOffset(arg0[8], D_800A35C4 + 0x10);
        AddPrim(g_gpu_ot_ptr + 0x18, arg0[8]);
        arg0[8] += 0xC;
        break;
    }
    }

    prim = (u8 *)arg0[5];
    SetTile(prim);
    SetSemiTrans(prim, 0);
    if (*(s32 *)(D_800A3568 + 0x20) & 1) {
        prim[4] = 0xC8;
        prim[5] = 0xC8;
        prim[6] = 0xC8;
    } else {
        prim[4] = 0xD0;
        prim[5] = 0xC8;
        prim[6] = 0xB8;
    }
    {
        s32 ot;

        ot = g_gpu_ot_ptr + 0x38;
        *(s16 *)(prim + 8) = 0x4C;
        *(s16 *)(prim + 0xA) = 0x80;
        *(s16 *)(prim + 0xC) = 0x215;
        *(s16 *)(prim + 0xE) = 0x4C;
        AddPrim(ot, prim);
    }
    prim += 0x10;
    arg0[5] = (s32)prim;

    {
        s32 base = ctx[2];

        s.p0 = (void *)base;
        p1 = (s32 *)(base + 0xC);
        s.p1 = p1;
    }
    s.c24 = 0x100;
    s.zero18 = 0;
    s.one14 = 0xE;
    s.ret = arg0[1];
    arg0[1] = func_80073728((s32)&s, 0);

    s.zero1C = 0x50;
    s.ret = arg0[1];
    arg0[1] = func_80073728((s32)&s, 2);
}
/* func_8007352C's draw descriptor (same 0x2C-byte layout as EnvA/EnvB):
   .header = the sprite sheet's SprtHdrA, .table = its SprtEntA cell array,
   .out = the SPRT cursor, +0x20/+0x24 = 8.8 fixed-point scales (0x100). */
typedef struct DescF97C {
    s32 header;
    s32 table;
    s32 out;
    s32 unk0C;
    s32 semi;
    s32 ot_idx;
    s32 x;
    s32 y;
    s32 scale_x;
    s32 scale_y;
    u8  has_color;
    u8  col_r;
    u8  col_g;
    u8  col_b;
} DescF97C;
extern void func_80070188(s32);
extern void func_80073200(s32);
void func_8006F97C(s32 *arg0) {
    DescF97C s;
    s16 shift[2];
    u16 rect[4];
    s32 *ctx;
    /* the sprite sheet's cell array (8-byte SprtEntA cells), which starts just
       past the sheet's 12-byte SprtHdrA headers: three on ctx[0] (normal, then
       one highlight per player, +0x24), one on every other sheet (+0xC).
       SEL.BIN/SEL1.BIN/SEL2.BIN census: pre-slim-2026-10-01:memory/grind/func_8006F97C/evidence.md. */
    s32 cells;
    s16 i;
    s16 row;
    s16 col;

    s.scale_x = 0x100;
    s.scale_y = 0x100;
    s.semi = 0;
    s.col_r = s.col_g = s.col_b = 0x70;
    ctx = *(s32 **)(D_800A35A8 + 0x60);
    s.header = ctx[0];
    SetDrawMode(arg0[6], 1, 0, func_8006E480(s.header, 0), 0);
    AddPrim(g_gpu_ot_ptr + 0x34, arg0[6]);
    arg0[6] += 0xC;

    s.ot_idx = 0xC;
    s.x = 0x82;
    s.y = 0x86;
    s.header = ctx[0];
    cells = s.header + 0x24;
    s.has_color = 0;
    for (i = 0; i < 1 + D_800A35B0 + D_800A3554; i++) {
        if (D_800A3588[i] == 5) {
            s.header = s.header + 12 + i * 12;
            if (((s16 *)D_800A35C4)[i] != 0) {
                s.x += (rsin((((s32 *)D_800A35C4)[2] * 192) & 0xFC0) * 5) >> 12;
                s.y += (rcos((((s32 *)D_800A35C4)[2] << 7) & 0xF80) * 3) >> 12;
            } else {
                s.has_color = 1;
            }
            if (((s16 *)D_800A35C4)[i] != 0x1E) {
                s.y += (((s16 *)D_800A35C4)[i] * rsin((((s32 *)D_800A35C4)[2] * 288) & 0xFE0)) >> 12;
            }
            break;
        }
    }
    s.table = cells;
    s.out = arg0[4];
    arg0[4] = func_8007352C((s32)&s);

    s.x = 0x17E;
    s.y = 0x86;
    s.header = ctx[0];
    s.has_color = 0;
    for (i = 0; i < 1 + D_800A35B0 + D_800A3554; i++) {
        if (D_800A3588[i] == 4) {
            s.header = s.header + 12 + i * 12;
            if (((s16 *)D_800A35C4)[i] != 0) {
                s.x += (rsin((((s32 *)D_800A35C4)[2] * 192) & 0xFC0) * 5) >> 12;
                s.y += (rcos((((s32 *)D_800A35C4)[2] << 7) & 0xF80) * 3) >> 12;
            } else {
                s.has_color = 1;
            }
            if (((s16 *)D_800A35C4)[i] != 0x1E) {
                s.y += (((s16 *)D_800A35C4)[i] * rsin((((s32 *)D_800A35C4)[2] * 288) & 0xFE0)) >> 12;
            }
            break;
        }
    }
    s.table += *(u8 *)(s.header + 2) * 8;
    s.out = arg0[4];
    arg0[4] = func_8007352C((s32)&s);

    s.x = 0;
    s.y = 0;
    s.header = ctx[1];
    SetDrawMode(arg0[6], 1, 0, func_8006E480(s.header, 0x60), 0);
    AddPrim(g_gpu_ot_ptr + 0x2C, arg0[6]);
    arg0[6] += 0xC;

    for (row = 0; row < 4; row++) {
        for (col = 0; col < 5; col++) {
            if ((D_800A35BC == 0 || D_800A35BC == 1 || D_800A35BC == 2 || D_800A35BC == 3) &&
                col == 4 && (row == 1 || row == 3)) {
                continue;
            }
            if (D_8009BC7C[D_8009BC40[col][row].value] & 1) {
                s.y = 0;
                s.x = 0;
                s.has_color = 0;
                for (i = 0; i < 1 + D_800A35B0 + D_800A3554; i++) {
                    if (D_800A3560.rec[i].unk0 == 0xFF) {
                        shift[i] = 7;
                    } else {
                        shift[i] = 9;
                    }
                    if (col == D_800A358C[i] && row == D_800A3588[i] && ((s16 *)D_800A35C4)[i] != 0) {
                        s.has_color = 1;
                        s.col_r = s.col_g = s.col_b =
                            ((rsin(((((s32 *)D_800A35C4)[2] & 0x1F) << shift[i]) + i * 511) * 63) >> 12) - 0x40;
                        break;
                    }
                }
                s.header = ctx[D_8009BC40[col][row].value + 1];
                cells = s.header + 0xC;
                s.table = cells;
                s.out = arg0[4];
                s.ot_idx = 0xA;
                arg0[4] = func_8007352C((s32)&s);
            } else {
                s.y = col << 4;
                s.x = row * 116 + (row >> 1) * 20;
                s.has_color = 0;
                /* FAKE: the draw tail is written in both arms (duplicated-statement-into-
                 * arms). The target shows two tails that jump2 cross-jumped: this arm
                 * ends `lw v0,84(fp); addiu a0,sp,24` BEFORE .L80070014, and the other
                 * arm reaches .L80070014 by `j` with `addiu a0,sp,24` in the delay slot
                 * (asm/funcs/func_8006F97C.s:409-410, 444-447). One shared tail after
                 * the if/else puts the a0 setup after the label (sched cannot cross the
                 * join): 7/515. Byte-neutral: the copies re-merge into the one call. */
                s.header = ctx[21];
                cells = s.header + 0xC;
                s.table = cells;
                s.out = arg0[4];
                s.ot_idx = 0xA;
                arg0[4] = func_8007352C((s32)&s);
            }
        }
    }

    s.y = 0;
    s.x = 0;
    s.has_color = 0;
    s.header = ctx[22];
    cells = s.header + 0xC;
    s.table = cells;
    s.out = arg0[4];
    s.ot_idx = 1;
    arg0[4] = func_8007352C((s32)&s);
    SetDrawMode(arg0[6], 1, 0, func_8006E480(s.header, 0x60), 0);
    AddPrim(g_gpu_ot_ptr + 4, arg0[6]);
    arg0[6] += 0xC;
    rect[2] = 0xF3;
    rect[0] = 0xC6;
    rect[1] = 0x25;
    rect[3] = 1;
    func_80069898((GameObj *)arg0, rect, 1);
    func_80070188((s32)arg0);
    func_8006ECF4((s32)arg0);
    D_800A32E8 = D_800A3560.rec[1].unk1;
    D_800A32E9 = D_800A3554;
    func_80072E10((s32)arg0);
    func_80073200((s32)arg0);
}
extern s16 D_800A3530[];
extern s16 D_800A3534[];
void func_80070188(s32 arg0) {
    DescF97C s;
    s32 *sheets;
    s16 *col;
    s16 *row;
    u8 *flags;
    s16 i;
    s16 port;
    s16 port_ofs;
    s32 c;

    s.semi = 0;
    s.scale_x = 0x100;
    s.scale_y = 0x100;
    sheets = *(s32 **)(D_800A35A8 + 0x74);
    s.header = sheets[0];
    SetDrawMode(((s32 *)arg0)[6], 1, 0, func_8006E480(s.header, 0x60), 0);
    AddPrim(g_gpu_ot_ptr + 0x20, ((s32 *)arg0)[6]);
    ((s32 *)arg0)[6] += 0xC;
    for (i = 0; i < 1 + D_800A35B0 + (port_ofs = D_800A3554); i++) {
        col = &D_800A3588[i];
        row = &D_800A358C[i];
        flags = &D_8009BC7C[D_8009BC40[*row][*col].value];
        D_800A3560.rec[i].unk1 = D_8009BC40[*row][*col].value;
        port = i - port_ofs;
        if (D_800A3560.rec[i].unk0 == 0xFF) {
            ((s16 *)D_800A35C4)[i] = 0x1E;
            if (D_800A354C & (0x4000 << (port * 16))) {
                func_8005C650(0, 0x7F, 0x7F);
                *flags &= ~4;
                do {
                    (*row)++;
                    if (*row >= 5) {
                        *row = 0;
                    }
                    flags = &D_8009BC7C[D_8009BC40[*row][*col].value];
                    D_800A3560.rec[i].unk1 = D_8009BC40[*row][*col].value;
                } while (*flags & 4);
                *flags |= 4;
            } else if (D_800A354C & (0x1000 << (port * 16))) {
                func_8005C650(0, 0x7F, 0x7F);
                *flags &= ~4;
                do {
                    (*row)--;
                    if (*row < 0) {
                        *row = 4;
                    }
                    flags = &D_8009BC7C[D_8009BC40[*row][*col].value];
                    D_800A3560.rec[i].unk1 = D_8009BC40[*row][*col].value;
                } while (*flags & 4);
                *flags |= 4;
            }
            if (D_800A354C & (0x2000 << (port * 16))) {
                func_8005C650(0, 0x7F, 0x7F);
                *flags &= ~4;
                do {
                    (*col)++;
                    if (*col > D_800A35B4) {
                        *col = 0;
                    }
                    flags = &D_8009BC7C[D_8009BC40[*row][*col].value];
                    D_800A3560.rec[i].unk1 = D_8009BC40[*row][*col].value;
                } while ((!(*flags & 1) && *col >= 4) || (*flags & 4));
                *flags |= 4;
            } else if (D_800A354C & (0x8000 << (port * 16))) {
                func_8005C650(0, 0x7F, 0x7F);
                *flags &= ~4;
                do {
                    (*col)--;
                    if (*col < 0) {
                        *col = D_800A35B4;
                    }
                    flags = &D_8009BC7C[D_8009BC40[*row][*col].value];
                    D_800A3560.rec[i].unk1 = D_8009BC40[*row][*col].value;
                } while ((!(*flags & 1) && *col >= 4) || (*flags & 4));
                *flags |= 4;
            }
            D_800A3530[i] = 7;
            D_800A3534[i] = 0;
        } else {
            D_800A3530[i] = 9;
            if (((s16 *)D_800A35C4)[i] != 0) {
                ((s16 *)D_800A35C4)[i]--;
            }
            if (D_800A3534[i] < 12) {
                D_800A3534[i]++;
            }
            s.header = sheets[3];
            *(u8 *)(s.header + 2) = D_800A3534[i];
            s.table = s.header + 0xC;
            s.has_color = 0;
            s.x = *col * 116 + 0x6E + (*col >> 1) * 20;
            s.y = *row * 16 + 0x80;
            s.ot_idx = 7;
            s.out = ((s32 *)arg0)[4];
            ((s32 *)arg0)[4] = func_8007352C((s32)&s);
        }
        if ((D_800A354C & (0x40 << (port * 16))) && D_800A3560.rec[i].unk0 == 0xFF) {
            if (*flags & 1) {
                D_800A3560.rec[i].unk2 = 0xFF;
                D_800A3590[i] = 2;
                D_800A3560.rec[i].unk0 = D_8009BC40[*row][*col].unk1;
                func_8005C650(D_8009BC40[*row][*col].value + 0xB, 0x7F, 0x7F);
                D_800A35C8[0] = 0xF;
                D_800A35C8[1] = 0x14;
                if (D_800A35BC == 2 && (*(s32 *)(D_800A3568 + 0x14) & 0x20000) && i == 0) {
                    if (D_8009BC7C[D_800A3560.rec[0].unk1] & 2) {
                        D_800A3588[1] = 2;
                    } else {
                        D_800A3588[1] = 0;
                    }
                    D_800A358C[1] = 0;
                    if (((s16 *)D_800A35C4)[0] == 0) {
                        D_800A3560.rec[1].unk1 = D_8009BC40[0][D_800A3588[1]].value;
                        D_8009BC7C[D_8009BC40[0][D_800A3588[1]].value] |= 4;
                    }
                    break;
                }
            } else {
                func_8005C650(0xA, 0x7F, 0x7F);
            }
        } else if (D_800A354C & (0x10 << (port * 16))) {
            if (D_800A3578 == 0) {
                func_8005C650(2, 0x7F, 0x7F);
                if (D_800A3560.rec[i].unk0 == 0xFF && D_800A3554 == 0) {
                    D_800A35A0 = -1;
                } else if (D_800A3554 == 1) {
                    if (D_800A3560.rec[1].unk0 == 0xFF) {
                        D_800A3554 = 0;
                        D_800A3560.rec[0].unk0 = 0xFF;
                    } else {
                        D_800A3560.rec[1].unk0 = 0xFF;
                    }
                    *flags &= ~4;
                } else if (D_800A3560.rec[i].unk0 != 0xFF && (*flags & 1)) {
                    D_800A3560.rec[i].unk0 = 0xFF;
                }
            }
        }
        if (((s16 *)D_800A35C4)[i] != 0) {
            s.has_color = 1;
        } else {
            s.has_color = 0;
        }
        if (D_800A3554 != 0 && i != 0) {
            s.header = sheets[i + 1];
        } else {
            s.header = sheets[i];
        }
        s.table = s.header + 0xC;
        c = ((rsin(((((s32 *)D_800A35C4)[2] & 0x1F) << D_800A3530[i]) + i * 511) * 63) >> 12) - 0x40;
        s.col_b = c;
        s.col_g = c;
        s.col_r = c;
        s.x = *col * 116 + 0x4E + (*col >> 1) * 20;
        s.y = *row * 16 + 0x80;
        s.ot_idx = 7;
        s.out = ((s32 *)arg0)[4];
        ((s32 *)arg0)[4] = func_8007352C((s32)&s);
    }
    if (D_800A3580 == 0 && D_800A3560.rec[0].unk0 != 0xFF && ((s16 *)D_800A35C4)[0] == 0) {
        if (D_800A35BC == 2 && (*(s32 *)(D_800A3568 + 0x14) & 0x20000)) {
            D_800A3554 = 1;
        }
        if ((D_800A3560.rec[1].unk0 != 0xFF && ((s16 *)D_800A35C4)[1] == 0) || D_800A35B0 + D_800A3554 == 0) {
            D_800A3578 = 1;
            D_800A3558 = 0;
            if (D_800A35BC == 2) {
                D_800A3560.rec[1].unk2 = 0xFF;
                D_800A3590[1] = 2;
            }
            D_800A3584 = 1;
        }
    }
    func_8005C6D0();
}
extern s16 D_800A3558;
extern s16 D_800A3590[];
extern s32 D_800A35A8;
extern s32 D_800A35B0;
extern s32 D_800A35BC;
extern s32 g_gpu_ot_ptr;


extern s32 SetDrawMode(s32, s32, s32, s32, s32);

extern void func_80070F78(s32 a0, DescF97C *s);
extern void func_8006ECF4(s32);
extern void func_80072E10(s32);
extern void func_80073200(s32);

typedef struct IconC70 {
    s16 sp48;
    s16 sp4A;
    s16 sp4C;
    s16 sp4E;
} IconC70;

void func_80070C70(s32 arg0) {
    s32 c60 = 0x60; /* FAKE: constant-holder local, mechanism: local-alloc/global.c keeps a
                     * live-across-call pseudo in a callee-saved register (the target's
                     * `li s4,96` + one `li a1,0x60` at the first call site (asm:36) plus two
                     * `move a1,s4` at the other two (asm:85,170)); the inline literal re-materializes
                     * `li a1,0x60` at each call site and measures 7/191 vs 0/194.
                     * lever-exhaustion: memory/grind/func_80070C70/hypotheses.md (s11-s17;
                     * literal re-measured on every chassis, 9 declaration slots inert) */
    DescF97C prim;
    u16 rect[4];
    s32 ctx_or_var_s2;
    s32 var_s0;
    s32 t;
    u8 code;
    s32 g;

    prim.semi = 0;
    prim.x = 0;
    prim.y = 0;
    prim.scale_x = 0x100;
    prim.scale_y = 0x100;
    prim.has_color = 0;
    ctx_or_var_s2 = (s32)*(s32 **)(D_800A35A8 + 0x64);
    /* NOT a coercion: the target itself stores zero to both fields twice - asm/funcs/
     * func_80070C70.s emits `sw zero,48(sp)` / `sw zero,52(sp)` before the `lw s2,100(v1)`
     * context fetch AND again after it.  The original source clears x/y a second
     * time after fetching the context; both stores are in the matched 194 insns. */
    prim.y = 0;
    prim.x = 0;
    g = *(s32 *)(ctx_or_var_s2 + 4);
    t = g + 0xC;
    prim.header = g;
    prim.table = t;
    prim.out = *(s32 *)(arg0 + 0x10);
    prim.ot_idx = 1;
    *(s32 *)(arg0 + 0x10) = func_8007352C((s32 *)&prim);
    SetDrawMode(((GameObj *)arg0)->field_18, 1, 0, func_8006E480(prim.header, c60), 0);
    AddPrim(g_gpu_ot_ptr + 4, ((GameObj *)arg0)->field_18);
    ((GameObj *)arg0)->field_18 += 0xC;
    rect[2] = 0xE7;
    rect[0] = 0xCC;
    rect[1] = 0x25;
    rect[3] = 1;
    func_80069898((GameObj *)arg0, (u16 *)rect, 1);
    g = *(s32 *)(ctx_or_var_s2);
    t = g + 0x48;
    prim.header = g;
    prim.table = t;
    for (var_s0 = 0; var_s0 < 6; var_s0++) {
        prim.x = var_s0 << 6;
        prim.out = *(s32 *)(arg0 + 0x10);
        prim.ot_idx = 0xA;
        *(s32 *)(arg0 + 0x10) = func_8007352C((s32 *)&prim);
        prim.header += 0xC;
    }
    prim.header = *(s32 *)(ctx_or_var_s2);
    SetDrawMode(((GameObj *)arg0)->field_18, 1, 0, func_8006E480(prim.header, c60), 0);
    AddPrim(g_gpu_ot_ptr + 0x28, ((GameObj *)arg0)->field_18);
    ((GameObj *)arg0)->field_18 += 0xC;
    prim.header = *(s32 *)(ctx_or_var_s2 + 8);
    for (var_s0 = 0; var_s0 < 1 + D_800A35B0 + D_800A3558; var_s0++) {
            code = D_800A3560.rec[var_s0].unk0;
            if ((code != 5) && (code != 16)) {
                g = prim.header;
                t = g + 0xC;
                prim.table = t;
                prim.table += D_800A3590[var_s0] << 4;
                if (((D_800A35B0 + D_800A3558) != 0) || (D_800A35BC == 2)) {
                    prim.x = 0x50 + var_s0 * 0x16C;
                } else {
                    prim.x = 0x105;
                }
                prim.out = *(s32 *)(arg0 + 0x10);
                prim.ot_idx = 1;
                *(s32 *)(arg0 + 0x10) = func_8007352C((s32 *)&prim);
            }
    }
    SetDrawMode(((GameObj *)arg0)->field_18, 1, 0, func_8006E480(prim.header, c60), 0);
    AddPrim(g_gpu_ot_ptr + 4, ((GameObj *)arg0)->field_18);
    ((GameObj *)arg0)->field_18 += 0xC;
    func_80070F78(arg0, &prim);
    func_8006ECF4(arg0);
    func_80072E10(arg0);
    func_80073200(arg0);
}
extern s16 D_800A3540[];
extern s16 D_800A3544[];
extern u8 D_8009BC38[];
extern s16 D_800A3594[];
void func_80070F78(s32 arg0, DescF97C *s) {
    s32 *sheets; /* several values of one kind, a sprite-sheet header table:
                  * *(D_800A35A8 + 0x74) (loaded at entry and again in the
                  * selected-slot arm) and *(D_800A35A8 + 0x60) (the table the
                  * last loop draws from). Ruling 11
                  * (ordinary-c-judge-decidable.md), proof in
                  * pre-slim-2026-10-01:memory/grind/func_80070F78/r11/README.md. */
    s32 flag;
    s32 cells; /* several values of one kind: the cell table following a sheet
                * header (s->header + 0xC in the two loop-2 draws, s->header +
                * 0x24 for the last loop). Ruling 11, proof in
                * pre-slim-2026-10-01:memory/grind/func_80070F78/r11/README.md. */
    s16 i;
    s16 port;
    s16 port_ofs;
    s32 c;

    flag = 0;
    sheets = *(s32 **)(D_800A35A8 + 0x74);
    s->header = sheets[0];
    SetDrawMode(((s32 *)arg0)[6], 1, 0, func_8006E480(s->header, 0x60), 0);
    AddPrim(g_gpu_ot_ptr + 0x20, ((s32 *)arg0)[6]);
    ((s32 *)arg0)[6] += 0xC;
    s->has_color = 1;
    s->scale_x = 0x100;
    s->scale_y = 0x100;
    for (i = 0; i < 1 + D_800A35B0 + D_800A3558; i++) {
        if (D_800A3578 == 0) {
            if (D_800A3560.rec[i].unk0 == 5) {
                D_800A3560.rec[i].unk2 = 6;
                flag = 1;
                ((s16 *)D_800A35C4)[i + 2] = 0;
            } else if (D_800A3560.rec[i].unk0 == 0x10) {
                D_800A3560.rec[i].unk2 = 7;
                flag = 1;
                ((s16 *)D_800A35C4)[i + 2] = 0;
            }
        }
    }
    for (i = 0; i < 1 + D_800A35B0 + (port_ofs = D_800A3558); i++) {
        u8 *vram; /* several values of one kind: player i's VRAM rect row,
                   * *(D_800A35A8 + 0x7C) + (i << 6), computed at the top of the
                   * loop and again in the locked-slot arm. Ruling 11, proof in
                   * pre-slim-2026-10-01:memory/grind/func_80070F78/r11/README.md. */

        vram = *(u8 **)(D_800A35A8 + 0x7C);
        vram += i << 6;
        port = i - port_ofs;
        if (D_800A3560.rec[i].unk0 != 5 && D_800A3560.rec[i].unk0 != 0x10) {
            s32 max;
            s32 min;

            if (D_800A35BC == 2 && !(*(s32 *)(D_800A3568 + 0x14) & 0x20000) && i == 1) {
                if (D_8009BC7C[D_800A3560.rec[0].unk1] & 2) {
                    max = 5;
                    min = 1;
                } else {
                    max = 4;
                    min = 0;
                }
            } else if (D_8009BC7C[D_800A3560.rec[i].unk1] & 2) {
                max = 4;
                min = 0;
            } else {
                max = 5;
                min = 1;
            }
            if (D_800A3560.rec[i].unk2 == 0xFF) {
                D_800A3544[i] = 7;
                D_800A3540[i] = 0;
                ((s16 *)D_800A35C4)[i + 2] = 0x1E;
                if ((D_800A354C & (0x2000 << (port * 16))) && D_800A3590[i] < max) {
                    func_8005C650(0, 0x7F, 0x7F);
                    D_800A3590[i]++;
                } else if ((D_800A354C & (0x8000 << (port * 16))) && min < D_800A3590[i]) {
                    func_8005C650(0, 0x7F, 0x7F);
                    D_800A3590[i]--;
                }
                D_800A3594[i] = i;
                if (D_800A35BC == 3) {
                    s32 sel;
                    s32 id;
                    s32 *tim; /* FAKE: named intermediate for the address of the image
                               * pointer LoadImage reads (named-intermediate entry,
                               * no-new-park-categories.md). Under `*(...)` the sum is an
                               * address and expands with EXPAND_SUM (INDIRECT_REF,
                               * expr.c:4563), where PLUS_EXPR's both_summands (expr.c:5248,
                               * "associate it to put the constant outside" :5257-5286) moves
                               * the constant 0x14 outside (the load becomes `lw 0x14(reg)`);
                               * assigned to a local it expands as a value (not EXPAND_SUM:
                               * expr.c:5237-5239 goto binop), computing
                               * id * 8 + 0x14 first as the target does (`addiu
                               * $v0,$a0,0x14; addu $v1,$v1,$v0; ...; lw $a1,0($v1)`).
                               * Inlined (sandbox, landed -G8 union form, 2026-09-30):
                               * this site 8, the confirm site 9, the locked site 14;
                               * earlier chassis: pre-slim-2026-10-01:memory/grind/func_80070F78/evidence.md [s2]. */

                    func_8005C650(1, 0x7F, 0x7F);
                    D_800A3560.rec[1].unk2 = 0;
                    D_800A3560.rec[0].unk2 = 0;
                    if (D_8009BC7C[D_800A3560.rec[i].unk1] & 2) {
                        sel = 0;
                    } else {
                        sel = 1;
                    }
                    id = D_800A3560.rec[i].unk2;
                    tim = (s32 *)(D_800A35A8 + 0x14 + id * 8 + sel * 4);
                    LoadImage(vram + id * 8, *tim);
                    DrawSync(0);
                } else if (D_800A354C & (0x40 << (port * 16))) {
                    s32 sel;
                    s32 id;
                    s32 *tim; /* FAKE: image pointer address, mechanism at the ==3 `tim` */

                    func_8005C650(1, 0x7F, 0x7F);
                    D_800A3560.rec[i].unk2 = D_8009BC38[D_800A3590[i]];
                    sel = 1;
                    if (D_800A35BC == 2 && !(*(s32 *)(D_800A3568 + 0x14) & 0x20000) && i == 1) {
                        if (!(D_8009BC7C[D_800A3560.rec[0].unk1] & 2)) {
                            sel = 0;
                        }
                    } else if (D_8009BC7C[D_800A3560.rec[i].unk1] & 2) {
                        sel = 0;
                    }
                    id = D_800A3560.rec[i].unk2;
                    tim = (s32 *)(D_800A35A8 + 0x14 + id * 8 + sel * 4);
                    LoadImage(vram + id * 8, *tim);
                    DrawSync(0);
                } else if (D_800A354C & (0x10 << (port * 16))) {
                    func_8005C650(2, 0x7F, 0x7F);
                    if (D_800A3558 == 1) {
                        D_800A3558 = 0;
                        D_800A3560.rec[0].unk2 = 0xFF;
                    } else {
                        D_800A3560.rec[1].unk2 = 0xFF;
                        D_800A3560.rec[0].unk2 = 0xFF;
                        D_800A3578 = 3;
                        D_800A3584 = 0;
                        func_8005C650(6, 0x7F, 0x7F);
                        if (D_800A3554 == 1) {
                            D_800A3560.rec[1].unk0 = 0xFF;
                        } else {
                            D_800A3560.rec[i].unk0 = 0xFF;
                        }
                        break;
                    }
                }
            } else {
                if ((D_800A354C & (0x10 << (port * 16))) && D_800A3578 == 0 && D_800A35BC != 3) {
                    func_8005C650(2, 0x7F, 0x7F);
                    D_800A35C8[0] = 0xF;
                    D_800A35C8[1] = 0x14;
                    D_800A3560.rec[i].unk2 = 0xFF;
                }
                D_800A3544[i] = 9;
                if (((s16 *)D_800A35C4)[i + 2] != 0) {
                    ((s16 *)D_800A35C4)[i + 2]--;
                }
                sheets = *(s32 **)(D_800A35A8 + 0x74);
                s->header = sheets[3];
                if (D_800A3540[i] < 12) {
                    D_800A3540[i]++;
                }
                *(u8 *)(s->header + 2) = D_800A3540[i];
                s->has_color = 0;
                cells = s->header + 0xC;
                s->table = cells;
                s->x = (D_800A3590[i] << 6) + 0x85;
                s->y = 0xB6 - (D_800A3594[i] << 5);
                s->ot_idx = 7;
                s->out = ((s32 *)arg0)[4];
                ((s32 *)arg0)[4] = func_8007352C((s32)s);
            }
            if (D_800A3578 != 3) {
                s->scale_x = 0x100;
                s->scale_y = 0x100;
                if (((s16 *)D_800A35C4)[i + 2] != 0) {
                    s->has_color = 1;
                } else {
                    s->has_color = 0;
                }
                if (D_800A3558 != 0 && i != 0) {
                    s->header = sheets[i + 1];
                } else {
                    s->header = sheets[i];
                }
                cells = s->header + 0xC;
                s->table = cells;
                c = ((rsin(((((s32 *)D_800A35C4)[2] & 0x1F) << D_800A3544[i]) + i * 511) * 63) >> 12) - 0x40;
                s->col_b = c;
                s->col_g = c;
                s->col_r = c;
                s->x = (D_800A3590[i] << 6) + 0x80;
                s->y = 0xAC - (D_800A3594[i] << 5);
                s->unk0C = ((s32 *)arg0)[1];
                s->ot_idx = 7;
                ((s32 *)arg0)[1] = func_80073728((s32)s, 0);
            }
        } else {
            if (D_800A3578 == 0) {
                if (D_800A3560.rec[i == 0 ? 1 : 0].unk2 != 0xFF && ((s16 *)D_800A35C4 + 2)[i == 0 ? 1 : 0] == 0) {
                    s32 sel;
                    s32 id;
                    s32 *tim; /* FAKE: image pointer address, mechanism at the ==3 `tim` */

                    func_8005C650(1, 0x7F, 0x7F);
                    flag = 2;
                    vram = *(u8 **)(D_800A35A8 + 0x7C);
                    vram += i << 6;
                    if (D_8009BC7C[D_800A3560.rec[i].unk1] & 2) {
                        sel = 0;
                    } else {
                        sel = 1;
                    }
                    id = D_800A3560.rec[i].unk2;
                    tim = (s32 *)(D_800A35A8 + 0x14 + id * 8 + sel * 4);
                    LoadImage(vram + id * 8, *tim);
                    DrawSync(0);
                }
            }
            if (D_800A354C & (0x10 << (i * 16))) {
                func_8005C650(2, 0x7F, 0x7F);
                D_800A3578 = 3;
                func_8005C650(6, 0x7F, 0x7F);
                D_800A3584 = 0;
                D_800A3560.rec[1].unk2 = 0xFF;
                D_800A3560.rec[0].unk2 = 0xFF;
                if (D_800A3558 == 1) {
                    D_800A3558 = 0;
                    D_800A3560.rec[1].unk0 = 0xFF;
                } else {
                    D_800A3560.rec[i].unk0 = 0xFF;
                }
                ((s16 *)D_800A35C4)[i + 2] = 0x1E;
            }
        }
    }
    sheets = *(s32 **)(D_800A35A8 + 0x60);
    s->header = sheets[0];
    s->scale_x = 0x80;
    s->scale_y = 0x80;
    s->has_color = 0;
    s->ot_idx = 0xC;
    cells = s->header + 0x24;
    for (i = 0; i < 1 + D_800A35B0 + D_800A3558; i++) {
        if (D_800A3560.rec[i].unk0 != 5 && D_800A3560.rec[i].unk0 != 0x10 && D_800A3578 != 3) {
            s->table = cells;
            s->x = (D_800A3590[i] << 6) + 0x80;
            s->y = 0xAC - (D_800A3594[i] << 5);
            if (D_800A35BC == 2 && !(*(s32 *)(D_800A3568 + 0x14) & 0x20000) && i == 1) {
                if (D_8009BC7C[D_800A3560.rec[0].unk1] & 2) {
                    s->table += *(u8 *)(s->header + 2) * 8;
                }
            } else if (!(D_8009BC7C[D_800A3560.rec[i].unk1] & 2)) {
                s->table += *(u8 *)(s->header + 2) * 8;
            }
            s->unk0C = ((s32 *)arg0)[1];
            ((s32 *)arg0)[1] = func_80073728((s32)s, 0);
        }
    }
    if (D_800A3560.rec[0].unk2 != 0xFF && ((s16 *)D_800A35C4)[2] == 0) {
        if (D_800A35BC == 2) {
            D_800A3558 = 1;
        }
        if (((D_800A3560.rec[1].unk2 != 0xFF && ((s16 *)D_800A35C4)[3] == 0) || D_800A35B0 + D_800A3558 == 0) && flag != 1) {
            D_800A3578 = 0x101;
            D_800A3584 = 2;
            D_800A3550 = 1;
            D_800A355C = 0;
        }
    }
}
extern u8 D_8009BC7C[];
s32 func_80071C20(void) {
    s32 v1;
    v1 = 3;
    if (D_8009BC7C[D_800A3560.rec[0].unk1] & 2) {
        v1 = 9;
    }
    return v1;
}
void func_80071C4C(s32 arg0) {
    Spr_8006F100 s;
    s32 i;
    s32 base;
    Obj_8006F100 *obj;
    s32 sel;
    s16 dx;
    s16 dy;

    base = *(s32 *)(D_800A35A8 + 0x58);
    for (i = 0; i < 1 + D_800A35B0 + D_800A3558; i++) {
        if (D_800A3560.rec[i].unk0 != 5 && D_800A3560.rec[i].unk0 != 16) {
            s.unk14 = 1;
            s.unk10 = 0;
            s.scale_x = 0x100;
            s.scale_y = 0x100;
            s.unk28 = 0;
            obj = *(Obj_8006F100 **)(base + D_800A3560.rec[i].unk2 * 4);
            sel = 1;
            if (D_800A35BC == 2 && !(*(s32 *)(D_800A3568 + 0x14) & 0x20000) && i == 1) {
                if (D_8009BC7C[D_800A3560.rec[0].unk1] & 2) {
                    sel = 1;
                } else {
                    sel = 0;
                }
            } else if (D_8009BC7C[D_800A3560.rec[i].unk1] & 2) {
                sel = 0;
            }
            s.hdr = &obj->hdr[sel];
            if (i != 0) {
                s.hdr->unk8 = 0x40;
            } else {
                s.hdr->unk8 = 0;
            }
            s.ent = obj->ent;
            {
                s32 idx = s.hdr->count - 1;
                dx = s.ent[idx].x + s.ent[idx].w - obj->ent[0].x;
                dy = s.ent[idx].y + s.ent[idx].h - obj->ent[0].y;
            }
            s.x = D_8009BC94[i][D_800A3590[i]].x + 0x140 - ((dx * s.scale_x >> 8) / 2);
            s.y = D_8009BC94[i][D_800A3590[i]].y + 0x9D - ((dy * s.scale_y >> 8) / 2);
            s.ret = *(s32 *)(arg0 + 4);
            if (i != 0) {
                *(s32 *)(arg0 + 4) = func_80073C78(&s, 0x1C0, 1);
            } else {
                *(s32 *)(arg0 + 4) = func_80073C78(&s, 0xE40, 0);
            }
        }
    }

    D_800A3550 += 8;
    if (D_800A3550 >= 0xFF) {
        D_800A3550 = 0xFF;
        D_800A3578 = 1;
        func_8005C650(6, 0x7F, 0x7F);
        if ((u32)D_800A35BC < 2) {
            s32 mode = func_80071C20();

            D_800A35A0 = 1;
            *(s32 *)(D_800A3568 + 0x14) =
                (*(s32 *)(D_800A3568 + 0x14) & ~0x3F0) | ((mode & 0x3F) << 4);
        } else if (D_800A35BC == 4) {
            D_800A3584 = 5;
        } else if (D_800A35BC == 6) {
            D_800A3584 = 6;
            D_800A359C = 1;
            D_800A3598 = 1;
        } else {
            D_800A35A0 = 1;
        }
        for (i = 0; i < 1 + D_800A35B0 + D_800A3554; i++) {
            s32 dst = i * 10; /* FAKE: named intermediate for player i's 10-byte replay-record
                               * offset (named-intermediate entry, no-new-park-categories.md).
                               * Mechanism, fixed at RTL expansion: the store's address expands
                               * with EXPAND_SUM, where an inlined `i * 10` comes back as
                               * (mult i 10) (expr.c:5359-5383) and PLUS_EXPR's "put a
                               * multiplication first" (expr.c:5288-5290) swaps it ahead of
                               * D_800A3568; force_operand (expr.c:3744) then emits
                               * (plus i*10 D_800A3568), `addu v0,a0,v0`. dst is a REG, so
                               * nothing is swapped and the add keeps the target's
                               * `addu v0,v0,a0`. Every inlined lever 1 per loop (sandbox
                               * 2026-09-30); dumps and scores:
                               * pre-slim-2026-10-01:memory/grind/_completed/func_80071C4C/hypotheses.md [g8 2026-09-30] */

            *(u8 *)(D_800A3568 + dst) = D_800A3560.rec[i].unk0;
        }
        for (i = 0; i < 1 + D_800A35B0 + D_800A3558; i++) {
            s32 dst = i * 10; /* FAKE: named intermediate for player i's 10-byte replay-record
                               * offset; same entry, mechanism and ledger as the first loop's
                               * dst (here the `+ 1` store); inlined 1 (sandbox 2026-09-30) */

            *(u8 *)(D_800A3568 + dst + 1) = D_800A3560.rec[i].unk2;
        }
    }
    func_8006F038(arg0);
}
extern s32 D_800A35A8;
void func_800720FC(s32, s32, s32);
void func_80072084(s32 a0) {
    s32 *v0 = (s32 *)D_800A35A8;
    func_800720FC(a0, v0[0x1A], 0);
}
void func_800720AC(s32 a0) {
    s32 *v0 = (s32 *)D_800A35A8;
    func_800720FC(a0, v0[0x1B], 1);
}
void func_800720D4(s32 a0) {
    s32 *v0 = (s32 *)D_800A35A8;
    func_800720FC(a0, v0[0x1C], 2);
}
typedef struct {
    s32 header;     /* sprite sheet header */
    s32 cells;      /* its cell table */
    s32 sprt_out;   /* func_8007352C output cursor */
    s32 ft4_out;    /* func_80073728 output cursor */
    s32 semi;
    s32 ot_idx;
    s32 x;
    s32 y;
    s32 scale_x;
    s32 scale_y;
    u8 has_color;
    u8 col_r;
    u8 col_g;
    u8 col_b;
} Desc720FC;
typedef struct {
    s32 hdr0;
    s32 hdr4;
    s32 hdr8;
    s32 hdrC[4][2];
    s32 hdr2C;
    s32 hdr30;
} Sheets720FC;
typedef struct {
    u8 unk0[0x14];
    u32 unk14_0 : 4;
    u32 unk14_4 : 6;
    u32 unk14_10 : 22;
} Cfg720FC;
void func_800720FC(s32 arg0, s32 arg1, s32 mode) {
    Desc720FC s;
    u16 rect2[4];
    u16 rect[4];
    u8 *menu;
    s32 *sheets;
    s32 cells; /* several values of one kind: the cell table following the
                * header(s) of each sheet drawn (s.header + 0x18 for the
                * two-header sheet, s.header + 0xC for the others). Ruling 11
                * (ordinary-c-judge-decidable.md), proof in
                * pre-slim-2026-10-01:memory/grind/func_800720FC/r11/. */
    s32 i;
    s32 j;
    s32 d;
    u8 code;
    u8 action;
    s32 c;
    s16 *timer;

    menu = *(u8 **)(D_800A35A8 + 0x80);
    s.semi = 0;
    s.has_color = 0;
    s.ot_idx = 0x11;
    rect[0] = ((u16 *)D_800A35C0)[0];
    rect[1] = ((u16 *)D_800A35C0)[1];
    rect[2] = ((u16 *)D_800A35C0)[2];
    rect[3] = ((u16 *)D_800A35C0)[3];
    SetDrawArea(((s32 *)arg0)[7], rect);
    AddPrim(g_gpu_ot_ptr + s.ot_idx * 4, ((s32 *)arg0)[7]);
    ((s32 *)arg0)[7] += 0xC;
    ((u16 *)D_800A35C4)[8] = ((u16 *)D_800A35C0)[4];
    ((u16 *)D_800A35C4)[9] = ((u16 *)D_800A35C0)[5];
    SetDrawOffset(((s32 *)arg0)[8], (u16 *)D_800A35C4 + 8);
    AddPrim(g_gpu_ot_ptr + s.ot_idx * 4, ((s32 *)arg0)[8]);
    ((s32 *)arg0)[8] += 0xC;

    if ((D_800A3578 & 0xFF) == 0) {
        s.x = -10;
        s.y = -5;
        sheets = *(s32 **)(D_800A35A8 + 0x74);
        s.header = sheets[4];
        cells = s.header + 0x18;
        if (!(D_800A359C * 2 + D_800A3598 == 3 && D_800A35BC == 6 && mode == 2)) {
            s.cells = cells;
            s.cells += ((D_800A359C + mode * 3) * 2 + D_800A3598) * 8;
            s.sprt_out = ((s32 *)arg0)[4];
            ((s32 *)arg0)[4] = func_8007352C(&s);
        }
        s.header += 0xC;
        for (i = 0; i < 6; i++) {
            if (!(i == 3 && D_800A35BC == 6 && mode == 2) && D_800A359C * 2 + D_800A3598 != i) {
                s.cells = cells + (mode * 6 + i) * 8;
                s.sprt_out = ((s32 *)arg0)[4];
                ((s32 *)arg0)[4] = func_8007352C(&s);
            }
        }
        s.header = sheets[4];
        SetDrawMode(((s32 *)arg0)[6], 1, 0, func_8006E480(s.header, 0), 0);
        AddPrim(g_gpu_ot_ptr + s.ot_idx * 4, ((s32 *)arg0)[6]);
        ((s32 *)arg0)[6] += 0xC;
    }

    s.x = 0x90;
    s.y = 0x28;
    s.header = ((Sheets720FC *)arg1)->hdr0;
    cells = s.header + 0xC;
    s.cells = cells;
    s.sprt_out = ((s32 *)arg0)[4];
    ((s32 *)arg0)[4] = func_8007352C(&s);
    ((u16 *)D_800A35C4)[8] = ((u16 *)D_800A35C0)[4] - D_8009BCC4[mode][0];
    ((u16 *)D_800A35C4)[9] = ((u16 *)D_800A35C0)[5] - D_8009BCC4[mode][1];
    if (((D_800A3578 & 0xFF) == 1 || (D_800A3578 & 0xFF) == 3) &&
        D_800A3584 >= 4 && D_800A3584 < 7 && D_800A3580 >= 4 && D_800A3580 < 7) {
        for (i = 0; i < 2; i++) {
            d = D_8009BCC4[D_800A3584 - 4][i] - D_8009BCC4[D_800A3580 - 4][i];
            d *= 30;
            if ((d >= 0 ? d : -d) > (D_8009BCD0[i] >= 0 ? D_8009BCD0[i] : -D_8009BCD0[i])) {
                D_8009BCD0[i] += d * 32 / 488;
            } else {
                D_8009BCD0[i] = d;
            }
            ((s16 *)D_800A35C4)[8 + i] -= D_8009BCD0[i] / 30;
        }
    } else {
        D_8009BCD0[1] = 0;
        D_8009BCD0[0] = 0;
    }
    SetDrawOffset(((s32 *)arg0)[8], (u16 *)D_800A35C4 + 8);
    AddPrim(g_gpu_ot_ptr + s.ot_idx * 4, ((s32 *)arg0)[8]);
    ((s32 *)arg0)[8] += 0xC;
    rect[0] = s.x;
    rect[1] = ((u16 *)D_800A35C0)[1] + s.y;
    rect[2] = 0x160;
    rect[3] = 0x4A;
    SetDrawArea(((s32 *)arg0)[7], rect);
    AddPrim(g_gpu_ot_ptr + s.ot_idx * 4, ((s32 *)arg0)[7]);
    ((s32 *)arg0)[7] += 0xC;
    SetDrawMode(((s32 *)arg0)[6], 1, 0, func_8006E480(s.header, 0), 0);
    AddPrim(g_gpu_ot_ptr + s.ot_idx * 4, ((s32 *)arg0)[6]);
    ((s32 *)arg0)[6] += 0xC;

    s.scale_x = 0x100; /* FAKE: dead store (overwritten by 0x180 below before any
                        * read) - the shipped code performs it: asm lines 356-358
                        * store 0x100 to both scales and lines 364-367 then store
                        * 0x180/0x120; GCC 2.7.2 has no dead-store elimination for
                        * the stack descriptor, so only a source that stores both
                        * emits both. dead-store-fake-exception.md; receipts in
                        * pre-slim-2026-10-01:memory/grind/func_800720FC/evidence.md. */
    s.scale_y = 0x100; /* FAKE: same dead store as above (asm line 358). */
    s.y = 0;
    s.x = 0;
    s.ot_idx = 0xB;
    s.header = ((Sheets720FC *)arg1)->hdr4;
    s.scale_x = 0x180;
    s.scale_y = 0x120;
    cells = s.header + 0xC;
    s.cells = cells;
    s.ft4_out = ((s32 *)arg0)[1];
    ((s32 *)arg0)[1] = func_80073728(&s, 0);
    s.cells += *(u8 *)(s.header + 2) * 8;
    s.ft4_out = ((s32 *)arg0)[1];
    ((s32 *)arg0)[1] = func_80073728(&s, 1);

    if (D_800A354C & 0xA000A000) {
        if ((D_800A3578 & 0xFF) == 0) {
            func_8005C650(0, 0x7F, 0x7F);
            if (((D_800A354C & 0x20002000) && D_800A3598 != 0) ||
                ((D_800A354C & 0x80008000) && D_800A3598 == 0)) {
                code = menu[(D_800A3580 - 4) * 8 + 3 * 2 + D_800A3598];
                D_800A3584 = code & 0xF;
                if (D_800A3584 != 0xF) {
                    D_800A3578 = code >> 4;
                    func_8005C650(6, 0x7F, 0x7F);
                }
            }
            D_800A3598 = D_800A3598 == 0;
        }
    }
    if (D_800A354C & 0x40004000) {
        func_8005C650(0, 0x7F, 0x7F);
        D_800A359C++;
    } else if (D_800A354C & 0x10001000) {
        func_8005C650(0, 0x7F, 0x7F);
        D_800A359C--;
    }
    if (D_800A359C >= 3) {
        D_800A359C = 0;
    } else if (D_800A359C < 0) {
        D_800A359C = 2;
    }

    c = ((rsin((((s32 *)D_800A35C4)[2] & 0x1F) * 128 + 0x1FF) * 63) >> 12) - 0x40;
    s.col_b = c;
    s.col_g = c;
    s.col_r = c;
    s.scale_x = 0x100;
    s.scale_y = 0x100;
    s.y = 0;
    s.x = 0;
    s.ot_idx = 1;
    s.header = ((Sheets720FC *)arg1)->hdr8;
    cells = s.header + 0xC;
    s.cells = cells;
    s.ft4_out = ((s32 *)arg0)[1];
    ((s32 *)arg0)[1] = func_80073728(&s, 0);
    rect2[2] = 0x111;
    rect2[0] = 0xB7;
    rect2[1] = 0x25;
    rect2[3] = 1;
    func_80069898(arg0, rect2, 1);
    s.ot_idx = 0xA;
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 2; j++) {
            if (i == D_800A359C && j == D_800A3598 && (D_800A3578 & 0xFF) == 0) {
                s.has_color = 1;
            } else {
                s.has_color = 0;
            }
            if (i + i + j != 3 || D_800A35BC != 6 || mode != 2) {
                s.header = ((s32 *)arg1 + (i + i))[j + 3];
            } else {
                s.header = ((Sheets720FC *)arg1)->hdr30;
            }
            cells = s.header + 0xC;
            s.cells = cells;
            s.ft4_out = ((s32 *)arg0)[1];
            ((s32 *)arg0)[1] = func_80073728(&s, 0);
        }
    }
    s.has_color = 0;
    s.header = ((Sheets720FC *)arg1)->hdr2C;
    cells = s.header + 0xC;
    s.cells = cells;
    s.ft4_out = ((s32 *)arg0)[1];
    ((s32 *)arg0)[1] = func_80073728(&s, 0);

    if (D_800A3578 == 0) {
        for (i = 0; i < D_800A35B0 + 1; i++) {
            if (D_800A354C & (0x10 << (i * 16))) {
                s32 slot;

                func_8005C650(2, 0x7F, 0x7F);
                slot = D_800A3560.rec[i].unk0;
                D_800A3560.rec[i].unk2 = 0xFF;
                if (slot == 5 || slot == 16) {
                    D_800A3580 = 0;
                    D_800A3560.rec[i == 0 ? 1 : 0].unk2 = 0xFF;
                    D_800A3560.rec[i].unk0 = 0xFF;
                } else {
                    D_800A3580 = 1;
                }
                timer = D_800A35C8; /* FAKE: pointer alias (pointer-alias-fake-exception.md):
                                     * `D_800A35C8[0] = 0xF; D_800A35C8[1] = 0x14;` gives
                                     * each element address its own pseudo; cse.c
                                     * use_related_value rewrites the second as
                                     * (plus base 2), so the base is used twice and loop.c
                                     * hoists it out of the player loop (asm then stores
                                     * via a callee-saved base). Through the local, the
                                     * [1] address folds to a constant in the MEM and the
                                     * single remaining use of the local is substituted
                                     * by loop.c's large-loop single-usage rule, giving
                                     * the target's two direct gp_rel stores. Receipts:
                                     * pre-slim-2026-10-01:memory/grind/func_800720FC/timers/README.md + evidence.md. */
                timer[0] = 0xF;
                timer[1] = 0x14;
                ((s16 *)D_800A35C4)[3] = 0;
                ((s16 *)D_800A35C4)[2] = 0;
                goto end;
            }
        }
        if (D_800A354C & 0x400040) {
            action = menu[(D_800A3580 - 4) * 8 + D_800A359C * 2 + D_800A3598];
            func_8005C650(1, 0x7F, 0x7F);
            if (action != 0xD || D_800A35BC != 6) {
                ((Cfg720FC *)D_800A3568)->unk14_4 = action;
            } else {
                ((Cfg720FC *)D_800A3568)->unk14_4 = 0x25;
            }
            D_800A35A0 = 1;
        }
    }
end:
    func_80072E10(arg0);
    func_80073200(arg0);
    func_8005C6D0();
}
extern s32 g_gpu_ot_ptr;

extern s32 SetPolyG4(GameObj *);

extern void *D_800A35C4;
s32 func_80072BC4(s32 arg0, GameObj *arg1) {
    u8 var_v0;
    int fc_const;

    SetPolyG4(arg1);
    SetSemiTrans(arg1, 0);
    fc_const = 0xFC;
    if (arg0 < 4) {
        *(u8 *)((s32)(arg1) + 4) = 0;
        *(u8 *)((s32)(arg1) + 5) = 0;
        *(u8 *)((s32)(arg1) + 6) = 0;
        *(u8 *)((s32)(arg1) + 0xC) = fc_const;
        *(u8 *)((s32)(arg1) + 0xD) = 0x82;
        *(u8 *)((s32)(arg1) + 0xE) = 0;
        *(u8 *)((s32)(arg1) + 0x14) = fc_const;
        *(u8 *)((s32)(arg1) + 0x15) = 0x82;
        *(u8 *)((s32)(arg1) + 0x16) = 0;
        if (*(s32 *)((s32)(D_800A35C4) + 8) & 4) {
            *(u8 *)((s32)(arg1) + 0x1D) = 0xC3;
            var_v0 = 0x1E;
        } else {
            *(u8 *)((s32)(arg1) + 0x1D) = 0xC3;
            var_v0 = 0x50;
        }
        *(u8 *)((s32)(arg1) + 0x1C) = fc_const;
        *(u8 *)((s32)(arg1) + 0x1E) = var_v0;
    } else {
        *(u8 *)((s32)(arg1) + 4) = 0;
        *(u8 *)((s32)(arg1) + 5) = 0;
        *(u8 *)((s32)(arg1) + 6) = 0;
        *(u8 *)((s32)(arg1) + 0xC) = 0x40;
        *(u8 *)((s32)(arg1) + 0xD) = 0;
        *(u8 *)((s32)(arg1) + 0xE) = 0x80;
        *(u8 *)((s32)(arg1) + 0x14) = 0x50;
        *(u8 *)((s32)(arg1) + 0x15) = 0xA0;
        *(u8 *)((s32)(arg1) + 0x16) = 0x40;
        *(u8 *)((s32)(arg1) + 0x1C) = 0x10;
        *(u8 *)((s32)(arg1) + 0x1D) = 0x40;
        *(u8 *)((s32)(arg1) + 0x1E) = 0x80;
    }
    AddPrim(g_gpu_ot_ptr + 0x60, arg1);
    return (s32)((u8 *)arg1 + 0x24);
}
/* func_80072CD4 - matched pure C. Judge PASS ruling 2026-09-04 01:32
 * (docs/grind/decisions.md): device-free ascending-order per-arm RGB triples;
 * no annotation is owed (ordinary C). */
s32 func_80072CD4(s32 arg0, GameObj *arg1) {
    int red;

    SetPolyG4(arg1);
    SetSemiTrans(arg1, 0);
    if (arg0 < 4) {
        red = 0xFC;
        if (*(s32 *)((s32)(D_800A35C4) + 8) & 4) {
            *(u8 *)((s32)(arg1) + 4) = red;
            *(u8 *)((s32)(arg1) + 5) = 0xC3;
            *(u8 *)((s32)(arg1) + 6) = 0x1E;
            *(u8 *)((s32)(arg1) + 0xC) = red;
            *(u8 *)((s32)(arg1) + 0xD) = 0xC8;
            *(u8 *)((s32)(arg1) + 0xE) = 0x32;
        } else {
            *(u8 *)((s32)(arg1) + 4) = red;
            *(u8 *)((s32)(arg1) + 5) = 0xC3;
            *(u8 *)((s32)(arg1) + 6) = 0x50;
            *(u8 *)((s32)(arg1) + 0xC) = red;
            *(u8 *)((s32)(arg1) + 0xD) = 0xDC;
            *(u8 *)((s32)(arg1) + 0xE) = 0x46;
        }
        *(u8 *)((s32)(arg1) + 0x14) = 0xFC;
        *(u8 *)((s32)(arg1) + 0x15) = 0x82;
        *(u8 *)((s32)(arg1) + 0x1C) = 0x32;
        *(u8 *)((s32)(arg1) + 0x1D) = 0x28;
        *(u8 *)((s32)(arg1) + 0x16) = 0;
        *(u8 *)((s32)(arg1) + 0x1E) = 0xA;
    } else {
        *(u8 *)((s32)(arg1) + 4) = 0x10;
        *(u8 *)((s32)(arg1) + 5) = 0x30;
        *(u8 *)((s32)(arg1) + 6) = 0x60;
        *(u8 *)((s32)(arg1) + 0xC) = 0x18;
        *(u8 *)((s32)(arg1) + 0xD) = 0;
        *(u8 *)((s32)(arg1) + 0xE) = 0x40;
        *(u8 *)((s32)(arg1) + 0x14) = 0x30;
        *(u8 *)((s32)(arg1) + 0x15) = 0;
        *(u8 *)((s32)(arg1) + 0x16) = 0x60;
        *(u8 *)((s32)(arg1) + 0x1C) = 0;
        *(u8 *)((s32)(arg1) + 0x1D) = 0;
        *(u8 *)((s32)(arg1) + 0x1E) = 0;
    }
    AddPrim(g_gpu_ot_ptr + 0x60, arg1);
    return (s32)((u8 *)arg1 + 0x24);
}

extern s32 func_80072CD4(s32, GameObj *);
extern s16 D_800A3580;
/* BEGIN func_80072E10 */
typedef struct PolyG4Xy {
    s32 tag;
    u8 r0, g0, b0, code;
    s16 x0, y0;
    u8 r1, g1, b1, pad1;
    s16 x1, y1;
    u8 r2, g2, b2, pad2;
    s16 x2, y2;
    u8 r3, g3, b3, pad3;
    s16 x3, y3;
} PolyG4Xy;
void func_80072E10(s32 arg0) {
    PolyG4Xy *p;
    func_80073060(arg0);
    p = *(PolyG4Xy **)((s32)arg0 + 0xC);
    p->x0 = 0x50;
    p->y0 = 0x32;
    p->x1 = 0x50;
    p->y1 = 0x52;
    p->x2 = 0x140;
    p->y2 = 0x32;
    p->x3 = 0x140;
    p->y3 = 0x52;
    p = (PolyG4Xy *)func_80072BC4(D_800A3580, (GameObj *)p);
    p->x0 = 0x231;
    p->y0 = 0x32;
    p->x1 = 0x231;
    p->y1 = 0x52;
    p->x2 = 0x140;
    p->y2 = 0x32;
    p->x3 = 0x140;
    p->y3 = 0x52;
    p = (PolyG4Xy *)func_80072BC4(D_800A3580, (GameObj *)p);
    p->x0 = 0x140;
    p->y0 = 0x52;
    p->x1 = 0x140;
    p->y1 = 0x71;
    p->x2 = 0x50;
    p->y2 = 0x52;
    p->x3 = 0x50;
    p->y3 = 0x71;
    p = (PolyG4Xy *)func_80072CD4(D_800A3580, (GameObj *)p);
    p->x0 = 0x140;
    p->y0 = 0x52;
    p->x1 = 0x140;
    p->y1 = 0x71;
    p->x2 = 0x231;
    p->y2 = 0x52;
    p->x3 = 0x231;
    p->y3 = 0x71;
    p = (PolyG4Xy *)func_80072CD4(D_800A3580, (GameObj *)p);
    *(PolyG4Xy **)((s32)arg0 + 0xC) = p;
}
/* END func_80072E10 */


extern s32 g_gpu_ot_ptr;

s32 *func_80072F30(s32 a0, u8 *a1) {
    SetTile((s32)a1);
    if (a0 < 4) {
        a1[4] = 0x9E;
        a1[5] = 0x64;
        a1[6] = 0;
        SetSemiTrans((s32)a1, 1);
    } else {
        a1[4] = 0x28;
        a1[5] = 0x28;
        a1[6] = 0x18;
        SetSemiTrans((s32)a1, 0);
    }
    AddPrim(g_gpu_ot_ptr + 0x5C, (s32)a1);
    return (s32 *)(a1 + 0x10);
}
extern s16 D_800A3580;
s32 *func_80072FCC(s32 ignored, u8 *a1) {
    SetTile((s32)a1);
    if (D_800A3580 < 4) {
        a1[4] = 0x46;
        a1[5] = 0x24;
        a1[6] = 0x0A;
        SetSemiTrans((s32)a1, 1);
    } else {
        a1[4] = 0;
        a1[5] = 0;
        a1[6] = 0;
        SetSemiTrans((s32)a1, 0);
    }
    AddPrim(g_gpu_ot_ptr + 0x5C, (s32)a1);
    return (s32 *)(a1 + 0x10);
}
/* BEGIN func_80073060 */
typedef struct TileXy {
    s32 tag;
    u8 r0, g0, b0, code;
    s16 x0, y0;
    s16 w, h;
} TileXy;
void func_80073060(s32 arg0) {
    TileXy *p;
    s32 i;
    p = *(TileXy **)((s32)arg0 + 0x14);
    for (i = 0x6F; i < 0x210; i += 0x20) {
        p->x0 = i;
        p->y0 = 0x32;
        p->w = 2;
        p->h = 0x3F;
        p = (TileXy *)func_80072F30(D_800A3580, (u8 *)p);
    }
    p->x0 = 0x51;
    p->y0 = 0x41;
    p->w = 0x1E0;
    p->h = 1;
    p = (TileXy *)func_80072F30(D_800A3580, (u8 *)p);
    p->x0 = 0x51;
    p->y0 = 0x61;
    p->w = 0x1E0;
    p->h = 1;
    p = (TileXy *)func_80072F30(D_800A3580, (u8 *)p);
    p->x0 = 0x51;
    p->y0 = 0x50;
    p->w = 0x1E0;
    p->h = 2;
    p = (TileXy *)func_80072F30(D_800A3580, (u8 *)p);
    for (i = 0; i < 5; i++) {
        p->x0 = 0x6A + i * 0x21;
        p->y0 = 0x32;
        p->w = 5 - i;
        p->h = 0x3F;
        p = (TileXy *)func_80072FCC(D_800A3580, (u8 *)p);
    }
    for (i = 0; i < 5; i++) {
        p->x0 = 0x211 - i * 0x20;
        p->y0 = 0x32;
        p->w = 5 - i;
        p->h = 0x3F;
        p = (TileXy *)func_80072FCC(D_800A3580, (u8 *)p);
    }
    *(TileXy **)((s32)arg0 + 0x14) = p;
}
/* END func_80073060 */
/* func_80073200 - session 9 (forensics).  FLOOR 2 -> 0.  BYTE-MATCHES.
 *
 * The s8 body sat at score 2 on ONE source-level hunk pair: our
 * `addiu a0,sp,0x18` (the `(s32)&s` argument of the FIRST of the four
 * func_80073728 calls) was emitted at the head of the if/else join block,
 * where the target emits it 4th - after `sb v0,0x42(sp); li v0,0x14;
 * sb v0,0x43(sp)` (asm/funcs/func_80073200.s:59-62).
 *
 * PASS ATTRIBUTION (this session, from the instrumented cc1 -da dumps):
 *   - The decision belongs to the FIRST scheduling pass, not the second.
 *     tmp/grind/func_80073200/s9/bb4_sched.txt (basic block 4, insns
 *     134..267 - all four call groups sit in ONE block) shows the a0 set
 *     (insn 158, INSN_PRIORITY 1) sorted to ready[0] at every step from
 *     T-47 to T-53 and displaced FOUR times by "insn N has a greater
 *     potential hazard" - at T-47 (insn 150), T-48 (147), T-50 (142) and
 *     T-52 (137).  It is therefore the LAST pick of the backward pass,
 *     i.e. the FIRST insn emitted in the block.
 *   - The predicate is tools/gcc-2.7.2/sched.c:2717, inside
 *     schedule_select: within one equal-INSN_PRIORITY group it keeps the
 *     insn with the largest `potential_hazard`.  potential_hazard
 *     (sched.c:1327) returns 0 immediately for an insn that is on no
 *     function unit, and a positive value for one whose unit has
 *     max_blockage > 1.  tools/gcc-2.7.2/insn-attrtab.c:6298 gives the MIPS
 *     "memory" unit max_blockage 3; an `addiu` (attr type "arith") is on no
 *     unit at all.  So at equal priority a ready STORE always displaces a
 *     ready address-arith insn REGARDLESS of their ready-list order (the
 *     `best_insn != 0` guard means position 0 can only be kept, never
 *     promoted).  That is precisely why the s7 spelling_enum sweep (1957
 *     no-swap orderings) and the s8 sweep (all 23 orderings of this call
 *     group) all measured >= 2: reordering the C only moves the LUID
 *     tie-break in rank_for_schedule, and the tie-break is never reached.
 *   - The SECOND pass (tmp/grind/func_80073200/s9/bb4_sched2.txt) is only a
 *     stabilizer here: after reload every one of those insns writes $2, so
 *     register anti/output dependences serialize them, and
 *     rank_for_schedule's INSN_LUID tie-break ("sort by INSN_LUID ... so
 *     that we make the sort stable") simply preserves pass 1's order.
 *
 * THE CLOSER is a pass-INPUT change, not another spelling of the same
 * input: stop putting those two `sb` stores into block 4 at all.  Writing
 * `s.sp42` and `s.sp43` inside BOTH if-arms leaves sched pass 1 a block 4
 * whose priority-1 group holds no store able to displace insn 158, so the
 * a0 set is emitted exactly where the target emits it.  jump2's
 * find_cross_jump then re-merges the two identical arm tails, so the
 * duplication is invisible in the bytes: build_insns 203 == target_insns
 * 203, score 0, `sandbox --diff` reports 0 source-level and 0 operand-only
 * hunks (the 6 remaining hunks are masked branch-target relocation
 * artifacts).  Staging the colour byte through the `var_v0` local is no
 * longer needed, so that local is gone.
 *
 * MINIMAL-FORM ABLATIONS (owner prong 4, "simplest-known-form"), all
 * measured THIS session on this chassis:
 *   - drop `v12` (the 0x12 constant holder introduced at s3), storing the
 *     literal at both sites instead: score 0, build_insns 203.  It is no
 *     longer load-bearing on the s9 chassis, so it is REMOVED.
 *     (tmp/grind/func_80073200/s9/b_no_v12.c)
 *   - drop `cond`, testing D_800A3580 inline: score 7 / 205 insns - KEPT.
 *   - split the reused `s1` into three separate locals: score 15 / 201 -
 *     the single reused table pointer is KEPT.
 *   - duplicate only `s.sp42` and leave `s.sp43 = 0x14;` at the join:
 *     score 2 - one remaining store is enough to keep the potential_hazard
 *     swap firing, so BOTH stores must move into the arms.
 *     (tmp/grind/func_80073200/s9/e_sp43_at_join.c, banked as
 *     rejected/s9-dup-sp42-only-join-sp43.c)
 */
typedef struct {
    s32 sp18, sp1C, sp20, sp24, sp28, sp2C, sp30, sp34, sp38, sp3C;
    u8 sp40, sp41, sp42, sp43;
} S73200;
void func_80073200(s32 arg0) {
    S73200 s;
    s32 *ctx;
    s32 base1;
    s32 base2;
    s32 s1;
    s32 tmp;
    s32 v1;
    s32 idx;
    s32 cond;

    s.sp30 = 0;
    s.sp34 = 0;
    s.sp38 = 0x100;
    s.sp3C = 0x100;
    ctx = *(s32 **)(D_800A35A8 + 0x5C);
    base1 = *(s32 *)((s32)ctx + 0xC);
    s.sp18 = base1;
    SetDrawMode(*(s32 *)(arg0 + 0x18), 1, 0, func_8006E480(base1, 0), 0);
    AddPrim(g_gpu_ot_ptr + 0x70, *(s32 *)(arg0 + 0x18));
    *(s32 *)(arg0 + 0x18) = *(s32 *)(arg0 + 0x18) + 0xC;
    s.sp40 = 1;
    base2 = *(s32 *)((s32)ctx + 0x10);
    s.sp18 = base2;
    s1 = base2 + 0xC;
    if (D_800A3580 < 4) {
        s.sp28 = 1;
        if (*(s32 *)((s32)D_800A35C4 + 8) & 4) {
            s.sp41 = 0xBC;
            s.sp42 = 0x78;
            s.sp43 = 0x14;
        } else {
            s.sp41 = 0xA8;
            s.sp42 = 0x6E;
            /* FAKE: `s.sp43 = 0x14;` is written in BOTH arms instead of once
             * at the join.  Byte-neutral - jump2's find_cross_jump re-merges
             * the two identical arm tails, so nothing extra materializes
             * (build_insns 203 == target_insns 203).  mechanism: the FIRST
             * scheduling pass, schedule_select's `potential_hazard` ready-list
             * swap (tools/gcc-2.7.2/sched.c:2717).  Keeping the two `sb` stores
             * out of that pass's basic block 4 removes the only ready insns
             * that could displace the `(s32)&s` argument set at its T-50 step.
             * lever-exhaustion: memory/grind/func_80073200/hypotheses.md s4-s8
             * (two permuter campaigns, the 1957-ordering spelling_enum sweep,
             * all 23 orderings of this call group, the addr-local naming). */
            s.sp43 = 0x14;
        }
        s.sp2C = 0x14;
        s.sp1C = s1;
        s.sp24 = *(s32 *)(arg0 + 4);
        *(s32 *)(arg0 + 4) = func_80073728((s32)&s, 0);
        s.sp1C = s1 + 8;
        s.sp24 = *(s32 *)(arg0 + 4);
        *(s32 *)(arg0 + 4) = func_80073728((s32)&s, 1);
        s.sp1C = s1 + 0x10;
        s.sp24 = *(s32 *)(arg0 + 4);
        *(s32 *)(arg0 + 4) = func_80073728((s32)&s, 2);
        s.sp1C = s1 + 0x18;
        s.sp24 = *(s32 *)(arg0 + 4);
        *(s32 *)(arg0 + 4) = func_80073728((s32)&s, 3);
        SetDrawMode(*(s32 *)(arg0 + 0x18), 1, 0, 0x60, 0);
        AddPrim(g_gpu_ot_ptr + (s.sp2C * 4), *(s32 *)(arg0 + 0x18));
        *(s32 *)(arg0 + 0x18) = *(s32 *)(arg0 + 0x18) + 0xC;
    } else {
        s.sp41 = 0x32;
        s.sp42 = 0x32;
        s.sp43 = 0x5A;
    }
    s.sp34 = 0;
    s.sp30 = 0;
    s.sp2C = 0x12;
    s.sp28 = 0;
    tmp = *(s32 *)((s32)ctx + 0x14);
    s.sp18 = tmp;
    s1 = tmp + 0xC;
    s.sp1C = s1;
    s.sp20 = *(s32 *)(arg0 + 0x10);
    *(s32 *)(arg0 + 0x10) = func_8007352C((s32)&s.sp18);
    SetDrawMode(*(s32 *)(arg0 + 0x18), 1, 0, func_8006E480(s.sp18, 0x60), 0);
    AddPrim(g_gpu_ot_ptr + (s.sp2C * 4), *(s32 *)(arg0 + 0x18));
    /* FAKE: `cond` names the D_800A3580 read as a fresh once-written,
     * once-read intermediate declared ahead of the pointer bump, so the read
     * is emitted before the branch rather than after it.  mechanism: LUID
     * order into the first scheduling pass - the named read becomes the
     * delay-slot-fillable insn the target puts between `lw v0,0x18(s0)` and
     * `beqz` (asm/funcs/func_80073200.s:151-157).  lever-exhaustion:
     * memory/grind/func_80073200/hypotheses.md s5/s6 (the complementary
     * in-block form and both declaration-order sweeps measured dead); ablated
     * again THIS session - removing it regresses 0 -> 7
     * (tmp/grind/func_80073200/s9/a_no_cond.c). */
    cond = D_800A3580;
    *(s32 *)(arg0 + 0x18) = *(s32 *)(arg0 + 0x18) + 0xC;
    if (cond < 2) {
        s.sp2C = 0x12;
        s.sp28 = 1;
        v1 = *(s32 *)((s32)D_800A35C4 + 8);
        idx = *(s32 *)((s32)ctx + 0x28 + (v1 % 4) * 4);
        s.sp18 = idx;
        s1 = idx + 0xC;
        s.sp1C = s1;
        s.sp20 = *(s32 *)(arg0 + 0x10);
        *(s32 *)(arg0 + 0x10) = func_8007352C((s32)&s.sp18);
        SetDrawMode(*(s32 *)(arg0 + 0x18), 1, 0, func_8006E480(s.sp18, 0x20), 0);
        AddPrim(g_gpu_ot_ptr + (s.sp2C * 4), *(s32 *)(arg0 + 0x18));
        *(s32 *)(arg0 + 0x18) = *(s32 *)(arg0 + 0x18) + 0xC;
    }
}
