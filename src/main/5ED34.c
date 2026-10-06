/* func_8006E534..func_80073200. .text 0x8006E534 (ROM 0x5ED34). Start boundary: G8.
 * Compiled -G8 (Makefile GP_FILES; per-file -G8 by proof, owner rulings Q10,
 * Q44 and Q54). Their original bytes reach the small globals 0x800A32E8..0x800A35CA
 * (40 addresses, 403 accesses) straight off $gp, which the original compiler
 * emits only at -G8, and no function outside this range gp-accesses any of them.
 * func_8006E8CC, func_8006E950, func_8006EA28 and func_80072F30 have no gp access
 * and compile to identical bytes at -G8 and -G0 (Q54). Proof and both-ways
 * listings: pre-slim-2026-10-01:memory/grind/func_80070F78/g8-evidence.md. */
#define INCLUDE_ASM_USE_MACRO_INC 1
#include "common.h"
#include "include_asm.h"
#include "bb2.h"

/* Declarations from the file this TU was split from (text1b_tu1c.c). */
extern s32 func_8005C2A8(Unk8005C2A8Pack *, s16, s32);
extern void AddPrim(void *, void *);

typedef struct SelectEntryE534 {
    u8 value;
    u8 unk1;
} SelectEntryE534;

extern SelectEntryE534 D_8009BC40[][6];
extern u8 D_8009BC7C[];
extern u8 D_800A32E8;
extern u8 D_800A32E9;
extern RECT D_800A32EC;

/* Q65: this file's statics (.sbss, allocated per file in link order by PSYLINK), in address order. */
static s16 D_800A3540[2];
static s16 D_800A3544[2];
static s32 D_800A3548;
static s32 D_800A354C;
static s16 D_800A3550;
static s16 D_800A3554;
static s16 D_800A3558;
static s16 D_800A355C;
static Unk800A3560Slots D_800A3560;
static Unk8009BD24Block *D_800A3568;
static s32 D_800A356C;
static s16 D_800A3570;
static s32 D_800A3574;  /* not named by any code or data: size from the gap */
static s16 D_800A3578;
static s16 D_800A357C;
static s16 D_800A3580;
static s16 D_800A3584;
static s16 D_800A3588[2];
static s16 D_800A358C[2];
static s16 D_800A3590[2];
static s16 D_800A3594[2];
static s16 D_800A3598;
static s16 D_800A359C;
static s32 D_800A35A0;
static Unk8006E49CRec *D_800A35A4;
static Unk8006EA28Rec *D_800A35A8;
static Unk8006E49CRec *D_800A35AC;
static s32 D_800A35B0;
static s16 D_800A35B4;
static s16 D_800A35B8;
static s32 D_800A35BC;
static GpuDb *D_800A35C0;
static Unk800A35C4Rec *D_800A35C4;
static s16 D_800A35C8[2];

s32 func_8006E534(s32 arg0, s32 arg1, Unk8009BD24Block *arg2, u32 arg3) {
    RECT rect;
    s16 i;

    ClearOTagR(g_gpu_ot_ptr, 0x1008);
    D_800A32E8 = 0x7F;
    D_800A35BC = arg2->unk14_0;
    D_800A35AC = (Unk8006E49CRec *)arg0;
    D_800A3568 = arg2;
    D_800A3558 = 0;
    D_800A3554 = 0;
    D_800A32E9 = 0;
    D_800A35B0 = arg1;
    D_800A356C = arg0 + 0x58;
    D_800A35A8 = (Unk8006EA28Rec *)(arg0 + 0x58);
    snd_StopAll();

    switch (D_800A35BC) {
    case 0:
        func_8006E950(5, (Unk8006E950Head *)D_800A356C);
        break;
    case 4:
    case 6:
        func_8006E950(3, (Unk8006E950Head *)D_800A356C);
        break;
    case 2:
        if (D_800A3568->unk14_17) {
            func_8006E950(3, (Unk8006E950Head *)D_800A356C);
        } else {
            func_8006E950(4, (Unk8006E950Head *)D_800A356C);
        }
        break;
    case 1:
    case 3:
        func_8006E950(4, (Unk8006E950Head *)D_800A356C);
        break;
    }

    D_800A356C = func_8006EA28((Unk8006EA28Rec *)D_800A356C);
    D_800A356C = func_8006E49C(D_800A356C, D_800A35AC);
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
        D_8009BC7C[i] &= 0xFA;
        if (arg3 & (1 << i)) {
            D_8009BC7C[i] |= 1;
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
    D_8009BC7C[D_8009BC40[D_800A358C[0]][D_800A3588[0]].value] |= 4;
    if (D_800A35B0 != 0) {
        D_8009BC7C[D_8009BC40[D_800A358C[1]][D_800A3588[1]].value] |= 4;
    }

    rect = D_800A32EC;
    D_800A359C = 0;
    D_800A3598 = 0;
    DrawSync(0);
    MoveImage(&rect, 0x3C0, 0x1FE);
    DrawSync(0);
    D_800A35C4 = (Unk800A35C4Rec *)D_800A356C;
    D_800A356C += 0x14;
    D_800A35C4->unk_08 = 0;
    D_800A35C4->unk_0C = 0;
    return 1;
}

Unk8006E49CRec *func_8006E8AC(s32 a0) {
    return &D_800A35AC[a0];
}

void func_8006E8CC(Unk8006E950Head *a0) {
    Unk8009BD24Block *p;
    s32 data;
    RECT rect;
    p = func_80077D00();
    if (p->unk20_0) {
        data = a0->unk_10;
    } else {
        data = a0->unk_0C;
    }
    rect.x = 0;
    rect.y = 0x1E0;
    rect.w = 0x280;
    rect.h = 0x20;
    DrawSync(0);
    LoadImage(&rect, (u32 *)data);
    DrawSync(0);
}
void func_8006E950(s32 a0, Unk8006E950Head *a1) {
    s32 s3;
    RECT rect;

    game_FrameLoop();
    cdrom_StartRead(func_80036EA8(2, a0), (s32)a1);
    game_FrameLoop();
    func_8006E440((s32 *)a1);

    s3 = a1->unk_08;

    rect.x = 0x280;
    rect.y = 0;
    rect.w = 0x180;
    rect.h = 0x1DC;
    DrawSync(0);
    LoadImage(&rect, (u32 *)s3);

    rect.w = 0x170;
    rect.x = 0x280;
    rect.y = 0x1DC;
    rect.h = 0x24;
    DrawSync(0);
    LoadImage(&rect, (u32 *)(s3 + 0x59400));

    func_8006E8CC(a1);
}
s32 func_8006EA28(Unk8006EA28Rec *a0) {
    func_8006920C((s32 *)a0, (s32)a0->unk_54);
    func_8006920C((s32 *)a0, (s32)a0->unk_58);
    func_8006920C((s32 *)a0, (s32)a0->unk_5C);
    func_8006920C((s32 *)a0, (s32)a0->unk_60);
    func_8006920C((s32 *)a0, (s32)a0->unk_64);
    func_8006920C((s32 *)a0, (s32)a0->unk_68);
    func_8006920C((s32 *)a0, (s32)a0->unk_6C);
    func_8006920C((s32 *)a0, (s32)a0->unk_70);
    func_8006920C((s32 *)a0, (s32)a0->unk_74);
    func_8005C2A8(a0->unk_00.unk_00, 1, a0->unk_00.unk_04);
    return a0->unk_00.unk_04;
}
extern void (*D_8009BC1C[7])(Unk8006EACCRec *);
void func_8006EC0C(void);
void func_8006F528(Unk8006EACCRec *);
s32 func_8006EACC(s32 arg0, s32 arg1) {
    Unk8006EACCRec sp10;
    Unk8006E49CRec *temp_v0;
    s32 temp_v1;

    D_800A35C0 = &g_gpu_db[D_800A36AC & 1];
    D_800A3548 = arg0;
    D_800A354C = arg1;
    if (D_800A35BC == 2) {
        D_800A354C = arg1 & 0xFFFF;
    }
    func_8006EC0C();
    temp_v1 = D_800A35C4->unk_0C + 1;
    D_800A35C4->unk_08 = D_800A35C4->unk_08 + 1;
    D_800A35C4->unk_0C = temp_v1;
    temp_v0 = func_8006E8AC(temp_v1 & 1);
    sp10.unk_00.v8006EA28 = D_800A35A8;
    sp10.unk_04.unk_00 = temp_v0->unk_00;
    sp10.unk_04.unk_08 = temp_v0->unk_08;
    sp10.unk_04.unk_0C = temp_v0->unk_10;
    sp10.unk_04.unk_10 = temp_v0->unk_0C;
    sp10.unk_04.unk_14 = temp_v0->unk_14;
    sp10.unk_04.unk_18 = temp_v0->unk_18;
    D_800A35A4 = temp_v0;
    sp10.unk_04.unk_1C = temp_v0->unk_1C;
    sp10.unk_24 = temp_v0->unk_20;
    if ((u16)(D_800A3580 - 2) >= 2U) {
        func_8006F528(&sp10);
    }
    D_8009BC1C[D_800A3580](&sp10);
    return D_800A35A0;
}
void func_8006EC0C(void) {
    s32 state = D_800A3578 & 0xFF;

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
        s16 v3584 = D_800A3584; /* FAKE: read ahead of the D_800A3570 store; at its use: score 2 */
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
extern u8 D_8009BC7C[];
extern SelectEntryE534 D_8009BC40[][6];
extern u8 D_800A32E8;
extern u8 D_800A32E9;
extern RECT D_800A32F4;

void func_8006ECF4(Unk8006EACCRec *arg0) {
    Unk8007352CEnv s;
    Unk8006ECF4Rec *s3;
    Unk8009B0E0Record *s0;
    s32 sel;
    s32 a2;
    s16 i;
    RECT rectbuf;

    s.ot_idx = 0x14;
    s.scale_x = 0x200;
    s.x = 0;
    s.y = 0;
    s.scale_y = 0x100;

    s3 = arg0->unk_00.v8006EA28->unk_54;
    s0 = s3->unk_0C;

    for (i = 0; i < D_800A35B0 + 1 + D_800A3554; i++) {
        sel = D_8009BC40[D_800A358C[i]][D_800A3588[i]].value;
        if (D_8009BC7C[sel] & 1) {
            s.semi = 0;
            if (D_800A35C4->unk_08 & 4) {
                s.has_color = 1;
            } else {
                s.has_color = 0;
            }
            s.col_r = 0x94;
            s.col_g = 0x80;
            s.col_b = 0x6E;
        } else {
            s.semi = 1;
            s.has_color = 1;
            s.col_b = 0;
            s.col_g = 0;
            s.col_r = 0;
        }

        switch (sel) {
        case 12:
            a2 = D_800A35A8->unk_84[0];
            s.header = s0 + 22;
            break;
        case 13:
            a2 = D_800A35A8->unk_84[1];
            s.header = s0 + 23;
            break;
        case 14:
            a2 = D_800A35A8->unk_84[2];
            s.header = s0 + 24;
            break;
        case 0:
            a2 = D_800A35A8->unk_84[3];
            s.header = s0 + 25;
            break;
        case 3:
            a2 = D_800A35A8->unk_84[4];
            s.header = s0 + 26;
            break;
        default:
            s.header = s0 + sel;
            goto p1_dispatch;
        }
        /* FAKE: the default `s.header = s0 + sel;` is written TWICE --
         * once in the switch default above and once at `default_p0:` -- instead of
         * sharing one copy behind the label: the shared copy flips jump2's
         * cross-jump merge direction (same instruction count, different block
         * layout).  The statement is real on both paths (the target recomputes p0
         * for i == 0 at .L8006EF14).  Family: duplicated-statement-into-arms.
         * One shared copy: score 20. */
        if (i == 0) goto default_p0;
        if (D_800A32E8 != sel || D_800A32E9 != D_800A3554) {
            rectbuf = D_800A32F4;
            LoadImage(&rectbuf, (u32 *)a2);
            DrawSync(0);
        }
        goto p1_dispatch;
    default_p0:
        s.header = s0 + sel;
    p1_dispatch:;

        if (D_800A35B0 != 0) goto p1_idx;
        if (D_8009BC7C[D_800A3560.rec[i].unk1] & 2) goto p1_idx;
        if (D_800A35BC != 2) goto p1_fallback;
        if (D_800A3568->unk14_17) goto p1_idx;
    p1_fallback:
        s.table = s3->unk_00[1];
        goto p1_done;
    p1_idx:
        s.table = s3->unk_00[i];
    p1_done:;

        s.ft4_out = arg0->unk_04.unk_00;
        if (i != 0) {
            arg0->unk_04.unk_00 = func_80073728(&s, 1);
        } else {
            arg0->unk_04.unk_00 = func_80073728(&s, 0);
        }
    }
}


void func_8006F038(Unk8006EACCRec *arg0) {
    TILE *temp_s0;
    s16 v1;
    s16 v2;
    s16 v3;

    temp_s0 = arg0->unk_04.unk_10;
    SetTile(temp_s0);
    v1 = D_800A3550;
    temp_s0->x0 = 0;
    temp_s0->y0 = 0;
    temp_s0->r0 = v1;
    v2 = D_800A3550;
    temp_s0->w = 0x280;
    temp_s0->g0 = v2;
    v3 = D_800A3550;
    temp_s0->h = 0xF0;
    temp_s0->b0 = v3;
    SetSemiTrans(temp_s0, 1);
    AddPrim(g_gpu_ot_ptr, temp_s0);
    temp_s0++;
    arg0->unk_04.unk_10 = temp_s0;
    SetDrawMode(arg0->unk_04.unk_14, 1, 0, 0x40, 0);
    AddPrim(g_gpu_ot_ptr, arg0->unk_04.unk_14);
    arg0->unk_04.unk_14++;
}
extern void func_80072E10(Unk8006EACCRec *);
extern void func_80073200(Unk8006EACCRec *);
extern s32 func_80073C78();


void func_8006F100(Unk8006EACCRec *arg0) {
    Unk8007352CEnv s;
    s32 i;
    Unk8009B0E0Record **base;
    Unk8009B0E0Record *obj;
    s32 sel;
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
    s.ot_idx = 0x13;
    s.semi = 0;
    s.has_color = 0;
    base = D_800A35A8->unk_58;
    for (i = 0; i < 1 + D_800A35B0 + D_800A3558; i++) {
        obj = base[D_800A3560.rec[i].unk2];
        sel = 1;
        if (D_800A35BC == 2 && !(D_800A3568->unk14_17) && i == 1) {
            if (D_8009BC7C[D_800A3560.rec[0].unk1] & 2) {
                sel = 1;
            } else {
                sel = 0;
            }
        } else if (D_8009BC7C[D_800A3560.rec[i].unk1] & 2) {
            sel = 0;
        }
        s.header = &obj[sel];
        if (i != 0) {
            s.header->ubase = 0x40;
        } else {
            s.header->ubase = 0;
        }
        s.table = obj[1].cells;
        t1 = -(D_800A35C8[i] * 800) / 20;
        {
            s32 idx = s.header->count - 1;
            dx = s.table[idx].x + s.table[idx].w - obj[1].cells[0].x;
            dy = s.table[idx].y + s.table[idx].h - obj[1].cells[0].y;
        }
        t2 = -(D_800A35C8[i] * 664) / 20;
        if (i != 0) {
            t1 = -t1;
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
             * from hoisting it into a callee-save. Both literals inline (cx and cy):
             * score 44. */
            s32 cx = 0x140;

            s.x = D_8009BC94[i][D_800A3590[i]].x + (t1 + cx) - ((dx * s.scale_x >> 8) / 2);
        }
        {
            /* FAKE: same constant-holder mechanism as `cx` above, for the
             * vertical centre 0x9D (scored with `cx`). */
            s32 cy = 0x9D;

            s.y = D_8009BC94[i][D_800A3590[i]].y + (t2 + cy) - ((dy * s.scale_y >> 8) / 2);
        }
        s.ft4_out = arg0->unk_04.unk_00;
        if (i != 0) {
            arg0->unk_04.unk_00 = func_80073C78(&s, 0x1C0, 1);
        } else {
            arg0->unk_04.unk_00 = func_80073C78(&s, 0xE40, 0);
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
void func_8006F528(Unk8006EACCRec *arg0) {
    Unk8007352CEnv s;
    RECT rect;
    Unk8009B0E0Record **ctx;
    TILE *prim;
    s16 state;
    /* FAKE: the holder between the header's cells and s.table; stored directly at all five sites:
       score 26 (one site at a time: 13 / 3 / 4 / 3 / 3). */
    Unk8009B400Record *p1;

    s.semi = 0;
    s.y = 0;
    s.scale_x = 0x100;
    s.scale_y = 0x100;
    s.has_color = 0;

    ctx = D_800A35A8->unk_5C;
    s.ot_idx = 0x10;
    state = D_800A3578 & 0xFF;
    s.header = ctx[0];
    p1 = s.header->cells;
    if (state >= 3) {
        s.x = D_800A3570;
    } else {
        s.x = 0;
    }
    s.table = p1;
    s.ft4_out = arg0->unk_04.unk_00;
    arg0->unk_04.unk_00 = func_80073728(&s, 0);

    s.table++;
    s.scale_y = 0x4C00;
    s.ft4_out = arg0->unk_04.unk_00;
    arg0->unk_04.unk_00 = func_80073728(&s, 0);

    s.scale_y = 0x100;
    s.header = ctx[1];
    p1 = s.header->cells;
    s.table = p1;
    s.ft4_out = arg0->unk_04.unk_00;
    arg0->unk_04.unk_00 = func_80073728(&s, 0);

    s.header = ctx[0];
    p1 = s.header->cells;
    if (state < 3) {
        s.x = -D_800A3570 + 0x200;
    } else {
        s.x = 0x200;
    }
    s.table = p1;
    s.ft4_out = arg0->unk_04.unk_00;
    arg0->unk_04.unk_00 = func_80073728(&s, 1);

    s.table++;
    s.scale_y = 0x4C00;
    s.ft4_out = arg0->unk_04.unk_00;
    arg0->unk_04.unk_00 = func_80073728(&s, 0);

    s.scale_y = 0x100;
    s.header = ctx[1];
    p1 = s.header->cells;
    s.table = p1;
    s.ft4_out = arg0->unk_04.unk_00;
    arg0->unk_04.unk_00 = func_80073728(&s, 1);

    if (state >= 3) {
        rect.x = D_800A3570 + 0x4C;
    } else {
        rect.x = 0x4C;
    }
    rect.y = D_800A35C0->draw.clip.y + 0x7C;
    rect.w = 0x1E8 - D_800A3570;
    rect.h = 0x54;
    SetDrawArea(arg0->unk_04.unk_18, &rect);
    AddPrim(g_gpu_ot_ptr + 0xF, arg0->unk_04.unk_18);
    arg0->unk_04.unk_18++;

    rect.x = D_800A35C0->draw.clip.x;
    rect.y = D_800A35C0->draw.clip.y;
    rect.w = D_800A35C0->draw.clip.w;
    rect.h = D_800A35C0->draw.clip.h;
    SetDrawArea(arg0->unk_04.unk_18, &rect);
    AddPrim(g_gpu_ot_ptr + 6, arg0->unk_04.unk_18);
    arg0->unk_04.unk_18++;

    switch (state) {
    case 2:
    case 4: {
        /* FAKE: one-use alias of D_800A35C4 for the offset store: written
           `D_800A35C4->unk_10[0] = offset_x`, the D_800A35C4 load (`lw a1`) sinks from the
           state test to the store, the test's 2 moves from $a2 to $a1 and offset_x from
           $v0 to $v1 (score 8). */
        Unk800A35C4Rec *blk;
        s32 x;
        s32 offset_x;

        blk = D_800A35C4;
        x = D_800A35C0->draw.ofs[0];
        if (state == 2) {
            offset_x = x - D_800A3570;
        } else {
            offset_x = x + D_800A3570;
        }
        blk->unk_10[0] = offset_x;
        D_800A35C4->unk_10[1] = D_800A35C0->draw.ofs[1];
        SetDrawOffset(arg0->unk_04.unk_1C, D_800A35C4->unk_10);
        AddPrim(g_gpu_ot_ptr + 0xF, arg0->unk_04.unk_1C);
        arg0->unk_04.unk_1C++;

        D_800A35C4->unk_10[0] = D_800A35C0->draw.ofs[0];
        D_800A35C4->unk_10[1] = D_800A35C0->draw.ofs[1];
        SetDrawOffset(arg0->unk_04.unk_1C, D_800A35C4->unk_10);
        AddPrim(g_gpu_ot_ptr + 6, arg0->unk_04.unk_1C);
        arg0->unk_04.unk_1C++;
        break;
    }
    }

    prim = arg0->unk_04.unk_10;
    SetTile(prim);
    SetSemiTrans(prim, 0);
    if (D_800A3568->unk20_0) {
        prim->r0 = 0xC8;
        prim->g0 = 0xC8;
        prim->b0 = 0xC8;
    } else {
        prim->r0 = 0xD0;
        prim->g0 = 0xC8;
        prim->b0 = 0xB8;
    }
    prim->x0 = 0x4C;
    prim->y0 = 0x80;
    prim->w = 0x215;
    prim->h = 0x4C;
    AddPrim(g_gpu_ot_ptr + 0xE, prim);
    prim++;
    arg0->unk_04.unk_10 = prim;

    s.header = ctx[2];
    p1 = s.header->cells;
    s.table = p1;
    s.scale_y = 0x100;
    s.x = 0;
    s.ot_idx = 0xE;
    s.ft4_out = arg0->unk_04.unk_00;
    arg0->unk_04.unk_00 = func_80073728(&s, 0);

    s.y = 0x50;
    s.ft4_out = arg0->unk_04.unk_00;
    arg0->unk_04.unk_00 = func_80073728(&s, 2);
}
extern void func_80070188(Unk8006EACCRec *);
extern void func_80073200(Unk8006EACCRec *);
void func_8006F97C(Unk8006EACCRec *arg0) {
    Unk8007352CEnv s;
    s16 shift[2];
    u16 rect[4];
    Unk8009B0E0Record **ctx;
    /* the sprite sheet's cell array (8-byte Unk8009B400Record cells), which starts just
       past the sheet's 12-byte Unk8009B0E0Record headers: three on ctx[0] (normal, then
       one highlight per player, +0x24), one on every other sheet (+0xC).
       SEL.BIN/SEL1.BIN/SEL2.BIN census: pre-slim-2026-10-01:memory/grind/func_8006F97C/evidence.md.
       FAKE: the holder between the header's cells and s.table; stored directly: score 98. */
    Unk8009B400Record *cells;
    s16 i;
    s16 row;
    s16 col;

    s.scale_x = 0x100;
    s.scale_y = 0x100;
    s.semi = 0;
    s.col_r = s.col_g = s.col_b = 0x70;
    ctx = D_800A35A8->unk_60;
    s.header = ctx[0];
    SetDrawMode(arg0->unk_04.unk_14, 1, 0, func_8006E480(s.header, 0), 0);
    AddPrim(g_gpu_ot_ptr + 0xD, arg0->unk_04.unk_14);
    arg0->unk_04.unk_14++;

    s.ot_idx = 0xC;
    s.x = 0x82;
    s.y = 0x86;
    s.header = ctx[0];
    cells = s.header[2].cells;
    s.has_color = 0;
    for (i = 0; i < 1 + D_800A35B0 + D_800A3554; i++) {
        if (D_800A3588[i] == 5) {
            s.header += 1 + i;
            if (D_800A35C4->unk_00[i] != 0) {
                s.x += (rsin((D_800A35C4->unk_08 * 192) & 0xFC0) * 5) >> 12;
                s.y += (rcos((D_800A35C4->unk_08 << 7) & 0xF80) * 3) >> 12;
            } else {
                s.has_color = 1;
            }
            if (D_800A35C4->unk_00[i] != 0x1E) {
                s.y += (D_800A35C4->unk_00[i] * rsin((D_800A35C4->unk_08 * 288) & 0xFE0)) >> 12;
            }
            break;
        }
    }
    s.table = cells;
    s.sprt_out = arg0->unk_04.unk_0C;
    arg0->unk_04.unk_0C = func_8007352C(&s);

    s.x = 0x17E;
    s.y = 0x86;
    s.header = ctx[0];
    s.has_color = 0;
    for (i = 0; i < 1 + D_800A35B0 + D_800A3554; i++) {
        if (D_800A3588[i] == 4) {
            s.header += 1 + i;
            if (D_800A35C4->unk_00[i] != 0) {
                s.x += (rsin((D_800A35C4->unk_08 * 192) & 0xFC0) * 5) >> 12;
                s.y += (rcos((D_800A35C4->unk_08 << 7) & 0xF80) * 3) >> 12;
            } else {
                s.has_color = 1;
            }
            if (D_800A35C4->unk_00[i] != 0x1E) {
                s.y += (D_800A35C4->unk_00[i] * rsin((D_800A35C4->unk_08 * 288) & 0xFE0)) >> 12;
            }
            break;
        }
    }
    s.table += s.header->count;
    s.sprt_out = arg0->unk_04.unk_0C;
    arg0->unk_04.unk_0C = func_8007352C(&s);

    s.x = 0;
    s.y = 0;
    s.header = ctx[1];
    SetDrawMode(arg0->unk_04.unk_14, 1, 0, func_8006E480(s.header, 0x60), 0);
    AddPrim(g_gpu_ot_ptr + 0xB, arg0->unk_04.unk_14);
    arg0->unk_04.unk_14++;

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
                    if (col == D_800A358C[i] && row == D_800A3588[i] && D_800A35C4->unk_00[i] != 0) {
                        s.has_color = 1;
                        s.col_r = s.col_g = s.col_b =
                            ((rsin(((D_800A35C4->unk_08 & 0x1F) << shift[i]) + i * 511) * 63) >> 12) - 0x40;
                        break;
                    }
                }
                s.header = ctx[D_8009BC40[col][row].value + 1];
                cells = s.header->cells;
                s.table = cells;
                s.sprt_out = arg0->unk_04.unk_0C;
                s.ot_idx = 0xA;
                arg0->unk_04.unk_0C = func_8007352C(&s);
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
                 * join): score 29. */
                s.header = ctx[21];
                cells = s.header->cells;
                s.table = cells;
                s.sprt_out = arg0->unk_04.unk_0C;
                s.ot_idx = 0xA;
                arg0->unk_04.unk_0C = func_8007352C(&s);
            }
        }
    }

    s.y = 0;
    s.x = 0;
    s.has_color = 0;
    s.header = ctx[22];
    cells = s.header->cells;
    s.table = cells;
    s.sprt_out = arg0->unk_04.unk_0C;
    s.ot_idx = 1;
    arg0->unk_04.unk_0C = func_8007352C(&s);
    SetDrawMode(arg0->unk_04.unk_14, 1, 0, func_8006E480(s.header, 0x60), 0);
    AddPrim(g_gpu_ot_ptr + 1, arg0->unk_04.unk_14);
    arg0->unk_04.unk_14++;
    rect[2] = 0xF3;
    rect[0] = 0xC6;
    rect[1] = 0x25;
    rect[3] = 1;
    /* The original passes its own context base here, unadjusted (func_8006F97C.s:498-514):
     * func_80069898 reads +0x18 as its TILE cursor (func_80069898.s:11, 37 / 65 / 91, 94),
     * and in this context +0x18 is the DR_MODE cursor. */
    func_80069898((s32 *)arg0, rect, 1);
    func_80070188(arg0);
    func_8006ECF4(arg0);
    D_800A32E8 = D_800A3560.rec[1].unk1;
    D_800A32E9 = D_800A3554;
    func_80072E10(arg0);
    func_80073200(arg0);
}
extern s16 D_800A3530[];
extern s16 D_800A3534[];
void func_80070188(Unk8006EACCRec *arg0) {
    Unk8007352CEnv s;
    Unk8009B0E0Record **sheets;
    s16 *col;
    s16 *row;
    u8 *flags;
    s16 i;
    s16 port;
    s16 port_ofs;

    s.semi = 0;
    s.scale_x = 0x100;
    s.scale_y = 0x100;
    sheets = D_800A35A8->unk_74;
    s.header = sheets[0];
    SetDrawMode(arg0->unk_04.unk_14, 1, 0, func_8006E480(s.header, 0x60), 0);
    AddPrim(g_gpu_ot_ptr + 8, arg0->unk_04.unk_14);
    arg0->unk_04.unk_14++;
    for (i = 0; i < 1 + D_800A35B0 + (port_ofs = D_800A3554); i++) {
        col = &D_800A3588[i];
        row = &D_800A358C[i];
        flags = &D_8009BC7C[D_8009BC40[*row][*col].value];
        D_800A3560.rec[i].unk1 = D_8009BC40[*row][*col].value;
        port = i - port_ofs;
        if (D_800A3560.rec[i].unk0 == 0xFF) {
            D_800A35C4->unk_00[i] = 0x1E;
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
            if (D_800A35C4->unk_00[i] != 0) {
                D_800A35C4->unk_00[i]--;
            }
            if (D_800A3534[i] < 12) {
                D_800A3534[i]++;
            }
            s.header = sheets[3];
            s.header->count = D_800A3534[i];
            s.table = s.header->cells;
            s.has_color = 0;
            s.x = *col * 116 + 0x6E + (*col >> 1) * 20;
            s.y = *row * 16 + 0x80;
            s.ot_idx = 7;
            s.sprt_out = arg0->unk_04.unk_0C;
            arg0->unk_04.unk_0C = func_8007352C(&s);
        }
        if ((D_800A354C & (0x40 << (port * 16))) && D_800A3560.rec[i].unk0 == 0xFF) {
            if (*flags & 1) {
                D_800A3560.rec[i].unk2 = 0xFF;
                D_800A3590[i] = 2;
                D_800A3560.rec[i].unk0 = D_8009BC40[*row][*col].unk1;
                func_8005C650(D_8009BC40[*row][*col].value + 0xB, 0x7F, 0x7F);
                D_800A35C8[0] = 0xF;
                D_800A35C8[1] = 0x14;
                if (D_800A35BC == 2 && (D_800A3568->unk14_17) && i == 0) {
                    if (D_8009BC7C[D_800A3560.rec[0].unk1] & 2) {
                        D_800A3588[1] = 2;
                    } else {
                        D_800A3588[1] = 0;
                    }
                    D_800A358C[1] = 0;
                    if (D_800A35C4->unk_00[0] == 0) {
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
        if (D_800A35C4->unk_00[i] != 0) {
            s.has_color = 1;
        } else {
            s.has_color = 0;
        }
        if (D_800A3554 != 0 && i != 0) {
            s.header = sheets[i + 1];
        } else {
            s.header = sheets[i];
        }
        s.table = s.header->cells;
        s.col_r = s.col_g = s.col_b = ((rsin(((D_800A35C4->unk_08 & 0x1F) << D_800A3530[i]) + i * 511) * 63) >> 12) - 0x40;
        s.x = *col * 116 + 0x4E + (*col >> 1) * 20;
        s.y = *row * 16 + 0x80;
        s.ot_idx = 7;
        s.sprt_out = arg0->unk_04.unk_0C;
        arg0->unk_04.unk_0C = func_8007352C(&s);
    }
    if (D_800A3580 == 0 && D_800A3560.rec[0].unk0 != 0xFF && D_800A35C4->unk_00[0] == 0) {
        if (D_800A35BC == 2 && (D_800A3568->unk14_17)) {
            D_800A3554 = 1;
        }
        if ((D_800A3560.rec[1].unk0 != 0xFF && D_800A35C4->unk_00[1] == 0) || D_800A35B0 + D_800A3554 == 0) {
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



extern void func_80070F78(Unk8006EACCRec *a0, Unk8007352CEnv *s);
extern void func_8006ECF4(Unk8006EACCRec *);
extern void func_80072E10(Unk8006EACCRec *);
extern void func_80073200(Unk8006EACCRec *);

typedef struct IconC70 {
    s16 sp48;
    s16 sp4A;
    s16 sp4C;
    s16 sp4E;
} IconC70;

void func_80070C70(Unk8006EACCRec *arg0) {
    s32 c60 = 0x60; /* FAKE: constant-holder local, mechanism: local-alloc/global.c keeps a
                     * live-across-call pseudo in a callee-saved register (the target's
                     * `li s4,96` + one `li a1,0x60` at the first call site (asm:36) plus two
                     * `move a1,s4` at the other two (asm:85,170)); the inline literal re-materializes
                     * `li a1,0x60` at each call site (score 7). */
    Unk8007352CEnv prim;
    u16 rect[4];
    Unk8009B0E0Record **ctx;
    s32 var_s0;
    /* FAKE: the holder between the header's cells and prim.table; stored directly: score 11. */
    Unk8009B400Record *t;
    u8 code;

    prim.semi = 0;
    prim.x = 0;
    prim.y = 0;
    prim.scale_x = 0x100;
    prim.scale_y = 0x100;
    prim.has_color = 0;
    ctx = D_800A35A8->unk_64;
    /* NOT a coercion: the target itself stores zero to both fields twice - asm/funcs/
     * func_80070C70.s emits `sw zero,48(sp)` / `sw zero,52(sp)` before the `lw s2,100(v1)`
     * context fetch AND again after it.  The original source clears x/y a second
     * time after fetching the context; both stores are in the matched 194 insns. */
    prim.y = 0;
    prim.x = 0;
    prim.header = ctx[1];
    t = prim.header->cells;
    prim.table = t;
    prim.sprt_out = arg0->unk_04.unk_0C;
    prim.ot_idx = 1;
    arg0->unk_04.unk_0C = func_8007352C(&prim);
    SetDrawMode(arg0->unk_04.unk_14, 1, 0, func_8006E480(prim.header, c60), 0);
    AddPrim(g_gpu_ot_ptr + 1, arg0->unk_04.unk_14);
    arg0->unk_04.unk_14++;
    rect[2] = 0xE7;
    rect[0] = 0xCC;
    rect[1] = 0x25;
    rect[3] = 1;
    /* The original passes its own context base here, unadjusted (func_80070C70.s:48-61):
     * func_80069898 reads +0x18 as its TILE cursor (func_80069898.s:11, 37 / 65 / 91, 94),
     * and in this context +0x18 is the DR_MODE cursor. */
    func_80069898((s32 *)arg0, rect, 1);
    prim.header = ctx[0];
    t = prim.header[5].cells;
    prim.table = t;
    for (var_s0 = 0; var_s0 < 6; var_s0++) {
        prim.x = var_s0 << 6;
        prim.sprt_out = arg0->unk_04.unk_0C;
        prim.ot_idx = 0xA;
        arg0->unk_04.unk_0C = func_8007352C(&prim);
        prim.header++;
    }
    prim.header = ctx[0];
    SetDrawMode(arg0->unk_04.unk_14, 1, 0, func_8006E480(prim.header, c60), 0);
    AddPrim(g_gpu_ot_ptr + 0xA, arg0->unk_04.unk_14);
    arg0->unk_04.unk_14++;
    prim.header = ctx[2];
    for (var_s0 = 0; var_s0 < 1 + D_800A35B0 + D_800A3558; var_s0++) {
            code = D_800A3560.rec[var_s0].unk0;
            if ((code != 5) && (code != 16)) {
                t = prim.header->cells;
                prim.table = t;
                prim.table += D_800A3590[var_s0] * 2;
                if (((D_800A35B0 + D_800A3558) != 0) || (D_800A35BC == 2)) {
                    prim.x = 0x50 + var_s0 * 0x16C;
                } else {
                    prim.x = 0x105;
                }
                prim.sprt_out = arg0->unk_04.unk_0C;
                prim.ot_idx = 1;
                arg0->unk_04.unk_0C = func_8007352C(&prim);
            }
    }
    SetDrawMode(arg0->unk_04.unk_14, 1, 0, func_8006E480(prim.header, c60), 0);
    AddPrim(g_gpu_ot_ptr + 1, arg0->unk_04.unk_14);
    arg0->unk_04.unk_14++;
    func_80070F78(arg0, &prim);
    func_8006ECF4(arg0);
    func_80072E10(arg0);
    func_80073200(arg0);
}
extern u8 D_8009BC38[];
void func_80070F78(Unk8006EACCRec *arg0, Unk8007352CEnv *s) {
    Unk8009B0E0Record **sheets; /* several values of one kind, a sprite-sheet header table:
                  * D_800A35A8->unk_74 (loaded at entry and again in the
                  * selected-slot arm) and D_800A35A8->unk_60 (the table the
                  * last loop draws from). Ruling 11
                  * (ordinary-c-judge-decidable.md), proof in
                  * pre-slim-2026-10-01:memory/grind/func_80070F78/r11/README.md. */
    s32 flag;
    Unk8009B400Record *cells; /* several values of one kind: the cell table following a sheet
                * header (s->header->cells in the two loop-2 draws, s->header[2].cells,
                * after three headers, for the last loop). Ruling 11, proof in
                * pre-slim-2026-10-01:memory/grind/func_80070F78/r11/README.md.
                * FAKE: the holder between the header's cells and s->table; stored directly: score 9. */
    s16 i;
    s16 port;
    s16 port_ofs;

    flag = 0;
    sheets = D_800A35A8->unk_74;
    s->header = sheets[0];
    SetDrawMode(arg0->unk_04.unk_14, 1, 0, func_8006E480(s->header, 0x60), 0);
    AddPrim(g_gpu_ot_ptr + 8, arg0->unk_04.unk_14);
    arg0->unk_04.unk_14++;
    s->has_color = 1;
    s->scale_x = 0x100;
    s->scale_y = 0x100;
    for (i = 0; i < 1 + D_800A35B0 + D_800A3558; i++) {
        if (D_800A3578 == 0) {
            if (D_800A3560.rec[i].unk0 == 5) {
                D_800A3560.rec[i].unk2 = 6;
                flag = 1;
                D_800A35C4->unk_04[i] = 0;
            } else if (D_800A3560.rec[i].unk0 == 0x10) {
                D_800A3560.rec[i].unk2 = 7;
                flag = 1;
                D_800A35C4->unk_04[i] = 0;
            }
        }
    }
    for (i = 0; i < 1 + D_800A35B0 + (port_ofs = D_800A3558); i++) {
        RECT *vram; /* several values of one kind: player i's VRAM rect row,
                   * D_800A35A8->unk_7C + i * 8, computed at the top of the
                   * loop and again in the locked-slot arm. Ruling 11, proof in
                   * pre-slim-2026-10-01:memory/grind/func_80070F78/r11/README.md. */

        /* FAKE: base and step as two statements, here and in the locked-slot arm;
         * `vram = D_800A35A8->unk_7C + i * 8` scores 28 */
        vram = D_800A35A8->unk_7C;
        vram += i * 8;
        port = i - port_ofs;
        if (D_800A3560.rec[i].unk0 != 5 && D_800A3560.rec[i].unk0 != 0x10) {
            s32 max;
            s32 min;

            if (D_800A35BC == 2 && !(D_800A3568->unk14_17) && i == 1) {
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
                D_800A35C4->unk_04[i] = 0x1E;
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
                               * Same construct at the confirm and locked sites.
                               * Written inline: score 27. */

                    func_8005C650(1, 0x7F, 0x7F);
                    D_800A3560.rec[1].unk2 = 0;
                    D_800A3560.rec[0].unk2 = 0;
                    if (D_8009BC7C[D_800A3560.rec[i].unk1] & 2) {
                        sel = 0;
                    } else {
                        sel = 1;
                    }
                    id = D_800A3560.rec[i].unk2;
                    tim = &D_800A35A8->unk_14[id][sel];
                    LoadImage(&vram[id], (u32 *)*tim);
                    DrawSync(0);
                } else if (D_800A354C & (0x40 << (port * 16))) {
                    s32 sel;
                    s32 id;
                    s32 *tim; /* FAKE: image pointer address, mechanism and score at the ==3 `tim` */

                    func_8005C650(1, 0x7F, 0x7F);
                    D_800A3560.rec[i].unk2 = D_8009BC38[D_800A3590[i]];
                    sel = 1;
                    if (D_800A35BC == 2 && !(D_800A3568->unk14_17) && i == 1) {
                        if (!(D_8009BC7C[D_800A3560.rec[0].unk1] & 2)) {
                            sel = 0;
                        }
                    } else if (D_8009BC7C[D_800A3560.rec[i].unk1] & 2) {
                        sel = 0;
                    }
                    id = D_800A3560.rec[i].unk2;
                    tim = &D_800A35A8->unk_14[id][sel];
                    LoadImage(&vram[id], (u32 *)*tim);
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
                if (D_800A35C4->unk_04[i] != 0) {
                    D_800A35C4->unk_04[i]--;
                }
                sheets = D_800A35A8->unk_74;
                s->header = sheets[3];
                if (D_800A3540[i] < 12) {
                    D_800A3540[i]++;
                }
                s->header->count = D_800A3540[i];
                s->has_color = 0;
                cells = s->header->cells;
                s->table = cells;
                s->x = (D_800A3590[i] << 6) + 0x85;
                s->y = 0xB6 - (D_800A3594[i] << 5);
                s->ot_idx = 7;
                s->sprt_out = arg0->unk_04.unk_0C;
                arg0->unk_04.unk_0C = func_8007352C(s);
            }
            if (D_800A3578 != 3) {
                s->scale_x = 0x100;
                s->scale_y = 0x100;
                if (D_800A35C4->unk_04[i] != 0) {
                    s->has_color = 1;
                } else {
                    s->has_color = 0;
                }
                if (D_800A3558 != 0 && i != 0) {
                    s->header = sheets[i + 1];
                } else {
                    s->header = sheets[i];
                }
                cells = s->header->cells;
                s->table = cells;
                s->col_r = s->col_g = s->col_b = ((rsin(((D_800A35C4->unk_08 & 0x1F) << D_800A3544[i]) + i * 511) * 63) >> 12) - 0x40;
                s->x = (D_800A3590[i] << 6) + 0x80;
                s->y = 0xAC - (D_800A3594[i] << 5);
                s->ft4_out = arg0->unk_04.unk_00;
                s->ot_idx = 7;
                arg0->unk_04.unk_00 = func_80073728(s, 0);
            }
        } else {
            if (D_800A3578 == 0) {
                if (D_800A3560.rec[i == 0 ? 1 : 0].unk2 != 0xFF && D_800A35C4->unk_04[i == 0 ? 1 : 0] == 0) {
                    s32 sel;
                    s32 id;
                    s32 *tim; /* FAKE: image pointer address, mechanism and score at the ==3 `tim` */

                    func_8005C650(1, 0x7F, 0x7F);
                    flag = 2;
                    vram = D_800A35A8->unk_7C;
                    vram += i * 8;
                    if (D_8009BC7C[D_800A3560.rec[i].unk1] & 2) {
                        sel = 0;
                    } else {
                        sel = 1;
                    }
                    id = D_800A3560.rec[i].unk2;
                    tim = &D_800A35A8->unk_14[id][sel];
                    LoadImage(&vram[id], (u32 *)*tim);
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
                D_800A35C4->unk_04[i] = 0x1E;
            }
        }
    }
    sheets = D_800A35A8->unk_60;
    s->header = sheets[0];
    s->scale_x = 0x80;
    s->scale_y = 0x80;
    s->has_color = 0;
    s->ot_idx = 0xC;
    cells = s->header[2].cells;
    for (i = 0; i < 1 + D_800A35B0 + D_800A3558; i++) {
        if (D_800A3560.rec[i].unk0 != 5 && D_800A3560.rec[i].unk0 != 0x10 && D_800A3578 != 3) {
            s->table = cells;
            s->x = (D_800A3590[i] << 6) + 0x80;
            s->y = 0xAC - (D_800A3594[i] << 5);
            if (D_800A35BC == 2 && !(D_800A3568->unk14_17) && i == 1) {
                if (D_8009BC7C[D_800A3560.rec[0].unk1] & 2) {
                    s->table += s->header->count;
                }
            } else if (!(D_8009BC7C[D_800A3560.rec[i].unk1] & 2)) {
                s->table += s->header->count;
            }
            s->ft4_out = arg0->unk_04.unk_00;
            arg0->unk_04.unk_00 = func_80073728(s, 0);
        }
    }
    if (D_800A3560.rec[0].unk2 != 0xFF && D_800A35C4->unk_04[0] == 0) {
        if (D_800A35BC == 2) {
            D_800A3558 = 1;
        }
        if (((D_800A3560.rec[1].unk2 != 0xFF && D_800A35C4->unk_04[1] == 0) || D_800A35B0 + D_800A3558 == 0) && flag != 1) {
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
void func_80071C4C(Unk8006EACCRec *arg0) {
    Unk8007352CEnv s;
    s32 i;
    Unk8009B0E0Record **base;
    Unk8009B0E0Record *obj;
    s32 sel;
    s16 dx;
    s16 dy;

    base = D_800A35A8->unk_58;
    for (i = 0; i < 1 + D_800A35B0 + D_800A3558; i++) {
        if (D_800A3560.rec[i].unk0 != 5 && D_800A3560.rec[i].unk0 != 16) {
            s.ot_idx = 1;
            s.semi = 0;
            s.scale_x = 0x100;
            s.scale_y = 0x100;
            s.has_color = 0;
            obj = base[D_800A3560.rec[i].unk2];
            sel = 1;
            if (D_800A35BC == 2 && !(D_800A3568->unk14_17) && i == 1) {
                if (D_8009BC7C[D_800A3560.rec[0].unk1] & 2) {
                    sel = 1;
                } else {
                    sel = 0;
                }
            } else if (D_8009BC7C[D_800A3560.rec[i].unk1] & 2) {
                sel = 0;
            }
            s.header = &obj[sel];
            if (i != 0) {
                s.header->ubase = 0x40;
            } else {
                s.header->ubase = 0;
            }
            s.table = obj[1].cells;
            {
                s32 idx = s.header->count - 1;
                dx = s.table[idx].x + s.table[idx].w - obj[1].cells[0].x;
                dy = s.table[idx].y + s.table[idx].h - obj[1].cells[0].y;
            }
            s.x = D_8009BC94[i][D_800A3590[i]].x + 0x140 - ((dx * s.scale_x >> 8) / 2);
            s.y = D_8009BC94[i][D_800A3590[i]].y + 0x9D - ((dy * s.scale_y >> 8) / 2);
            s.ft4_out = arg0->unk_04.unk_00;
            if (i != 0) {
                arg0->unk_04.unk_00 = func_80073C78(&s, 0x1C0, 1);
            } else {
                arg0->unk_04.unk_00 = func_80073C78(&s, 0xE40, 0);
            }
        }
    }

    D_800A3550 += 8;
    if (D_800A3550 >= 0xFF) {
        D_800A3550 = 0xFF;
        D_800A3578 = 1;
        func_8005C650(6, 0x7F, 0x7F);
        if ((u32)D_800A35BC < 2) {
            D_800A3568->unk14_4 = func_80071C20();
            D_800A35A0 = 1;
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
            D_800A3568->unk00[i][0].chr = D_800A3560.rec[i].unk0;
        }
        for (i = 0; i < 1 + D_800A35B0 + D_800A3558; i++) {
            D_800A3568->unk00[i][0].unk1 = D_800A3560.rec[i].unk2;
        }
    }
    func_8006F038(arg0);
}
void func_800720FC(Unk8006EACCRec *, Unk8009B0E0Record **, s32);
void func_80072084(Unk8006EACCRec *a0) {
    func_800720FC(a0, D_800A35A8->unk_68, 0);
}
void func_800720AC(Unk8006EACCRec *a0) {
    func_800720FC(a0, D_800A35A8->unk_6C, 1);
}
void func_800720D4(Unk8006EACCRec *a0) {
    func_800720FC(a0, D_800A35A8->unk_70, 2);
}
void func_800720FC(Unk8006EACCRec *arg0, Unk8009B0E0Record **arg1, s32 mode) {
    Unk8007352CEnv s;
    u16 rect2[4];
    RECT rect;
    u8 *menu;
    Unk8009B0E0Record **sheets;
    Unk8009B400Record *cells; /* several values of one kind: the cell table following the
                * header(s) of each sheet drawn (s.header[1].cells for the
                * two-header sheet, s.header->cells for the others). Ruling 11
                * (ordinary-c-judge-decidable.md), proof in
                * pre-slim-2026-10-01:memory/grind/func_800720FC/r11/.
                * FAKE: the holder between the header's cells and s.table; stored directly: score 123. */
    s32 i;
    s32 j;
    s32 d;
    u8 code;
    u8 action;

    menu = D_800A35A8->unk_80;
    s.semi = 0;
    s.has_color = 0;
    s.ot_idx = 0x11;
    rect.x = D_800A35C0->draw.clip.x;
    rect.y = D_800A35C0->draw.clip.y;
    rect.w = D_800A35C0->draw.clip.w;
    rect.h = D_800A35C0->draw.clip.h;
    SetDrawArea(arg0->unk_04.unk_18, &rect);
    AddPrim(g_gpu_ot_ptr + s.ot_idx, arg0->unk_04.unk_18);
    arg0->unk_04.unk_18++;
    D_800A35C4->unk_10[0] = D_800A35C0->draw.ofs[0];
    D_800A35C4->unk_10[1] = D_800A35C0->draw.ofs[1];
    SetDrawOffset(arg0->unk_04.unk_1C, D_800A35C4->unk_10);
    AddPrim(g_gpu_ot_ptr + s.ot_idx, arg0->unk_04.unk_1C);
    arg0->unk_04.unk_1C++;

    if ((D_800A3578 & 0xFF) == 0) {
        s.x = -10;
        s.y = -5;
        sheets = D_800A35A8->unk_74;
        s.header = sheets[4];
        cells = s.header[1].cells;
        if (!(D_800A359C * 2 + D_800A3598 == 3 && D_800A35BC == 6 && mode == 2)) {
            s.table = cells;
            s.table += (D_800A359C + mode * 3) * 2 + D_800A3598;
            s.sprt_out = arg0->unk_04.unk_0C;
            arg0->unk_04.unk_0C = func_8007352C(&s);
        }
        s.header++;
        for (i = 0; i < 6; i++) {
            if (!(i == 3 && D_800A35BC == 6 && mode == 2) && D_800A359C * 2 + D_800A3598 != i) {
                s.table = cells + (mode * 6 + i);
                s.sprt_out = arg0->unk_04.unk_0C;
                arg0->unk_04.unk_0C = func_8007352C(&s);
            }
        }
        s.header = sheets[4];
        SetDrawMode(arg0->unk_04.unk_14, 1, 0, func_8006E480(s.header, 0), 0);
        AddPrim(g_gpu_ot_ptr + s.ot_idx, arg0->unk_04.unk_14);
        arg0->unk_04.unk_14++;
    }

    s.x = 0x90;
    s.y = 0x28;
    s.header = arg1[0];
    cells = s.header->cells;
    s.table = cells;
    s.sprt_out = arg0->unk_04.unk_0C;
    arg0->unk_04.unk_0C = func_8007352C(&s);
    D_800A35C4->unk_10[0] = D_800A35C0->draw.ofs[0] - D_8009BCC4[mode][0];
    D_800A35C4->unk_10[1] = D_800A35C0->draw.ofs[1] - D_8009BCC4[mode][1];
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
            D_800A35C4->unk_10[i] -= D_8009BCD0[i] / 30;
        }
    } else {
        D_8009BCD0[1] = 0;
        D_8009BCD0[0] = 0;
    }
    SetDrawOffset(arg0->unk_04.unk_1C, D_800A35C4->unk_10);
    AddPrim(g_gpu_ot_ptr + s.ot_idx, arg0->unk_04.unk_1C);
    arg0->unk_04.unk_1C++;
    rect.x = s.x;
    rect.y = D_800A35C0->draw.clip.y + s.y;
    rect.w = 0x160;
    rect.h = 0x4A;
    SetDrawArea(arg0->unk_04.unk_18, &rect);
    AddPrim(g_gpu_ot_ptr + s.ot_idx, arg0->unk_04.unk_18);
    arg0->unk_04.unk_18++;
    SetDrawMode(arg0->unk_04.unk_14, 1, 0, func_8006E480(s.header, 0), 0);
    AddPrim(g_gpu_ot_ptr + s.ot_idx, arg0->unk_04.unk_14);
    arg0->unk_04.unk_14++;

    s.scale_x = 0x100; /* FAKE: dead store (overwritten by 0x180 below before any
                        * read) - the shipped code performs it: asm lines 356-358
                        * store 0x100 to both scales and lines 364-367 then store
                        * 0x180/0x120; GCC 2.7.2 has no dead-store elimination for
                        * the stack descriptor, so only a source that stores both
                        * emits both. dead-store-fake-exception.md. Both removed:
                        * score 3. */
    s.scale_y = 0x100; /* FAKE: same dead store as above (asm line 358; scored there). */
    s.y = 0;
    s.x = 0;
    s.ot_idx = 0xB;
    s.header = arg1[1];
    s.scale_x = 0x180;
    s.scale_y = 0x120;
    cells = s.header->cells;
    s.table = cells;
    s.ft4_out = arg0->unk_04.unk_00;
    arg0->unk_04.unk_00 = func_80073728(&s, 0);
    s.table += s.header->count;
    s.ft4_out = arg0->unk_04.unk_00;
    arg0->unk_04.unk_00 = func_80073728(&s, 1);

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

    s.col_r = s.col_g = s.col_b = ((rsin((D_800A35C4->unk_08 & 0x1F) * 128 + 0x1FF) * 63) >> 12) - 0x40;
    s.scale_x = 0x100;
    s.scale_y = 0x100;
    s.y = 0;
    s.x = 0;
    s.ot_idx = 1;
    s.header = arg1[2];
    cells = s.header->cells;
    s.table = cells;
    s.ft4_out = arg0->unk_04.unk_00;
    arg0->unk_04.unk_00 = func_80073728(&s, 0);
    rect2[2] = 0x111;
    rect2[0] = 0xB7;
    rect2[1] = 0x25;
    rect2[3] = 1;
    /* The original passes its own context base here, unadjusted (func_800720FC.s:517-528):
     * func_80069898 reads +0x18 as its TILE cursor (func_80069898.s:11, 37 / 65 / 91, 94),
     * and in this context +0x18 is the DR_MODE cursor. */
    func_80069898((s32 *)arg0, rect2, 1);
    s.ot_idx = 0xA;
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 2; j++) {
            if (i == D_800A359C && j == D_800A3598 && (D_800A3578 & 0xFF) == 0) {
                s.has_color = 1;
            } else {
                s.has_color = 0;
            }
            if (i + i + j != 3 || D_800A35BC != 6 || mode != 2) {
                s.header = (arg1 + (i + i))[j + 3];
            } else {
                s.header = arg1[12];
            }
            cells = s.header->cells;
            s.table = cells;
            s.ft4_out = arg0->unk_04.unk_00;
            arg0->unk_04.unk_00 = func_80073728(&s, 0);
        }
    }
    s.has_color = 0;
    s.header = arg1[11];
    cells = s.header->cells;
    s.table = cells;
    s.ft4_out = arg0->unk_04.unk_00;
    arg0->unk_04.unk_00 = func_80073728(&s, 0);

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
                D_800A35C8[0] = 0xF;
                D_800A35C8[1] = 0x14;
                D_800A35C4->unk_04[1] = 0;
                D_800A35C4->unk_04[0] = 0;
                goto end;
            }
        }
        if (D_800A354C & 0x400040) {
            action = menu[(D_800A3580 - 4) * 8 + D_800A359C * 2 + D_800A3598];
            func_8005C650(1, 0x7F, 0x7F);
            if (action != 0xD || D_800A35BC != 6) {
                D_800A3568->unk14_4 = action;
            } else {
                D_800A3568->unk14_4 = 0x25;
            }
            D_800A35A0 = 1;
        }
    }
end:
    func_80072E10(arg0);
    func_80073200(arg0);
    func_8005C6D0();
}


POLY_G4 *func_80072BC4(s32 arg0, POLY_G4 *arg1) {
    u8 var_v0;
    int fc_const; /* FAKE: constant holder -- 0xFC held from before the branch; the literal at each store rematerialises it at the r3 store and swaps v0 / v1: score 5 */

    SetPolyG4(arg1);
    SetSemiTrans(arg1, 0);
    fc_const = 0xFC;
    if (arg0 < 4) {
        arg1->r0 = 0;
        arg1->g0 = 0;
        arg1->b0 = 0;
        arg1->r1 = fc_const;
        arg1->g1 = 0x82;
        arg1->b1 = 0;
        arg1->r2 = fc_const;
        arg1->g2 = 0x82;
        arg1->b2 = 0;
        if (D_800A35C4->unk_08 & 4) {
            arg1->g3 = 0xC3;
            var_v0 = 0x1E;
        } else {
            arg1->g3 = 0xC3;
            var_v0 = 0x50;
        }
        arg1->r3 = fc_const;
        arg1->b3 = var_v0;
    } else {
        arg1->r0 = 0;
        arg1->g0 = 0;
        arg1->b0 = 0;
        arg1->r1 = 0x40;
        arg1->g1 = 0;
        arg1->b1 = 0x80;
        arg1->r2 = 0x50;
        arg1->g2 = 0xA0;
        arg1->b2 = 0x40;
        arg1->r3 = 0x10;
        arg1->g3 = 0x40;
        arg1->b3 = 0x80;
    }
    AddPrim(g_gpu_ot_ptr + 0x18, arg1);
    return arg1 + 1;
}
/* func_80072CD4 - colours a Gouraud quad (POLY_G4) by mode: warm per-vertex
 * RGB triples (two variants on D_800A35C4's flag 4) for modes < 4, a fixed
 * dark set otherwise; then adds it to the OT and returns the next primitive slot. */
POLY_G4 *func_80072CD4(s32 arg0, POLY_G4 *arg1) {
    SetPolyG4(arg1);
    SetSemiTrans(arg1, 0);
    if (arg0 < 4) {
        if (D_800A35C4->unk_08 & 4) {
            arg1->r0 = 0xFC;
            arg1->g0 = 0xC3;
            arg1->b0 = 0x1E;
            arg1->r1 = 0xFC;
            arg1->g1 = 0xC8;
            arg1->b1 = 0x32;
        } else {
            arg1->r0 = 0xFC;
            arg1->g0 = 0xC3;
            arg1->b0 = 0x50;
            arg1->r1 = 0xFC;
            arg1->g1 = 0xDC;
            arg1->b1 = 0x46;
        }
        arg1->r2 = 0xFC;
        arg1->g2 = 0x82;
        arg1->r3 = 0x32;
        arg1->g3 = 0x28;
        arg1->b2 = 0;
        arg1->b3 = 0xA;
    } else {
        arg1->r0 = 0x10;
        arg1->g0 = 0x30;
        arg1->b0 = 0x60;
        arg1->r1 = 0x18;
        arg1->g1 = 0;
        arg1->b1 = 0x40;
        arg1->r2 = 0x30;
        arg1->g2 = 0;
        arg1->b2 = 0x60;
        arg1->r3 = 0;
        arg1->g3 = 0;
        arg1->b3 = 0;
    }
    AddPrim(g_gpu_ot_ptr + 0x18, arg1);
    return arg1 + 1;
}

/* BEGIN func_80072E10 */
void func_80072E10(Unk8006EACCRec *arg0) {
    POLY_G4 *p;
    func_80073060(arg0);
    p = arg0->unk_04.unk_08;
    p->x0 = 0x50;
    p->y0 = 0x32;
    p->x1 = 0x50;
    p->y1 = 0x52;
    p->x2 = 0x140;
    p->y2 = 0x32;
    p->x3 = 0x140;
    p->y3 = 0x52;
    p = func_80072BC4(D_800A3580, p);
    p->x0 = 0x231;
    p->y0 = 0x32;
    p->x1 = 0x231;
    p->y1 = 0x52;
    p->x2 = 0x140;
    p->y2 = 0x32;
    p->x3 = 0x140;
    p->y3 = 0x52;
    p = func_80072BC4(D_800A3580, p);
    p->x0 = 0x140;
    p->y0 = 0x52;
    p->x1 = 0x140;
    p->y1 = 0x71;
    p->x2 = 0x50;
    p->y2 = 0x52;
    p->x3 = 0x50;
    p->y3 = 0x71;
    p = func_80072CD4(D_800A3580, p);
    p->x0 = 0x140;
    p->y0 = 0x52;
    p->x1 = 0x140;
    p->y1 = 0x71;
    p->x2 = 0x231;
    p->y2 = 0x52;
    p->x3 = 0x231;
    p->y3 = 0x71;
    p = func_80072CD4(D_800A3580, p);
    arg0->unk_04.unk_08 = p;
}
/* END func_80072E10 */



TILE *func_80072F30(s32 a0, TILE *a1) {
    SetTile(a1);
    if (a0 < 4) {
        a1->r0 = 0x9E;
        a1->g0 = 0x64;
        a1->b0 = 0;
        SetSemiTrans(a1, 1);
    } else {
        a1->r0 = 0x28;
        a1->g0 = 0x28;
        a1->b0 = 0x18;
        SetSemiTrans(a1, 0);
    }
    AddPrim(g_gpu_ot_ptr + 0x17, a1);
    return a1 + 1;
}
TILE *func_80072FCC(s32 ignored, TILE *a1) {
    SetTile(a1);
    if (D_800A3580 < 4) {
        a1->r0 = 0x46;
        a1->g0 = 0x24;
        a1->b0 = 0x0A;
        SetSemiTrans(a1, 1);
    } else {
        a1->r0 = 0;
        a1->g0 = 0;
        a1->b0 = 0;
        SetSemiTrans(a1, 0);
    }
    AddPrim(g_gpu_ot_ptr + 0x17, a1);
    return a1 + 1;
}
/* BEGIN func_80073060 */
void func_80073060(Unk8006EACCRec *arg0) {
    TILE *p;
    s32 i;
    p = arg0->unk_04.unk_10;
    for (i = 0x6F; i < 0x210; i += 0x20) {
        p->x0 = i;
        p->y0 = 0x32;
        p->w = 2;
        p->h = 0x3F;
        p = func_80072F30(D_800A3580, p);
    }
    p->x0 = 0x51;
    p->y0 = 0x41;
    p->w = 0x1E0;
    p->h = 1;
    p = func_80072F30(D_800A3580, p);
    p->x0 = 0x51;
    p->y0 = 0x61;
    p->w = 0x1E0;
    p->h = 1;
    p = func_80072F30(D_800A3580, p);
    p->x0 = 0x51;
    p->y0 = 0x50;
    p->w = 0x1E0;
    p->h = 2;
    p = func_80072F30(D_800A3580, p);
    for (i = 0; i < 5; i++) {
        p->x0 = 0x6A + i * 0x21;
        p->y0 = 0x32;
        p->w = 5 - i;
        p->h = 0x3F;
        p = func_80072FCC(D_800A3580, p);
    }
    for (i = 0; i < 5; i++) {
        p->x0 = 0x211 - i * 0x20;
        p->y0 = 0x32;
        p->w = 5 - i;
        p->h = 0x3F;
        p = func_80072FCC(D_800A3580, p);
    }
    arg0->unk_04.unk_10 = p;
}
/* END func_80073060 */
/* func_80073200 - draws three sprite-sheet layers: in modes D_800A3580 < 4,
 * four cells of ctx[4]'s sheet through func_80073728; then ctx[5]'s sheet and,
 * for D_800A3580 < 2, the ctx[10..13] sheet picked by the frame counter at
 * D_800A35C4->unk_08 (incremented in func_8006EACC), each
 * through func_8007352C, with a draw-mode primitive per layer. */
void func_80073200(Unk8006EACCRec *arg0) {
    Unk8007352CEnv s;
    Unk8009B0E0Record **ctx;
    /* FAKE: the holder between the header's cells and s.table; stored directly: score 15. */
    Unk8009B400Record *s1;

    s.x = 0;
    s.y = 0;
    s.scale_x = 0x100;
    s.scale_y = 0x100;
    ctx = D_800A35A8->unk_5C;
    s.header = ctx[3];
    SetDrawMode(arg0->unk_04.unk_14, 1, 0, func_8006E480(s.header, 0), 0);
    AddPrim(g_gpu_ot_ptr + 0x1C, arg0->unk_04.unk_14);
    arg0->unk_04.unk_14++;
    s.has_color = 1;
    s.header = ctx[4];
    s1 = s.header->cells;
    if (D_800A3580 < 4) {
        s.semi = 1;
        if (D_800A35C4->unk_08 & 4) {
            s.col_r = 0xBC;
            s.col_g = 0x78;
            s.col_b = 0x14;
        } else {
            s.col_r = 0xA8;
            s.col_g = 0x6E;
            /* FAKE: `s.col_b = 0x14;` is written in BOTH arms instead of once
             * at the join.  No extra instruction - jump2's find_cross_jump re-merges
             * the two identical arm tails, so nothing extra materializes.
             * mechanism: the FIRST scheduling pass, schedule_select's
             * `potential_hazard` ready-list swap (tools/gcc-2.7.2/sched.c:2717):
             * at equal priority a ready store (MIPS "memory" unit) always
             * displaces a ready address-arith insn.  Keeping the `sb` stores
             * out of the join block lets the first func_80073728 call's
             * `&s` argument set land 4th, after `sb v0,0x42(sp); li v0,0x14;
             * sb v0,0x43(sp)`, as in the target (asm/funcs/func_80073200.s:59-62);
             * with col_b stored once at the join it is emitted first: score 2. */
            s.col_b = 0x14;
        }
        s.ot_idx = 0x14;
        s.table = s1;
        s.ft4_out = arg0->unk_04.unk_00;
        arg0->unk_04.unk_00 = func_80073728(&s, 0);
        s.table = s1 + 1;
        s.ft4_out = arg0->unk_04.unk_00;
        arg0->unk_04.unk_00 = func_80073728(&s, 1);
        s.table = s1 + 2;
        s.ft4_out = arg0->unk_04.unk_00;
        arg0->unk_04.unk_00 = func_80073728(&s, 2);
        s.table = s1 + 3;
        s.ft4_out = arg0->unk_04.unk_00;
        arg0->unk_04.unk_00 = func_80073728(&s, 3);
        SetDrawMode(arg0->unk_04.unk_14, 1, 0, 0x60, 0);
        AddPrim(g_gpu_ot_ptr + s.ot_idx, arg0->unk_04.unk_14);
        arg0->unk_04.unk_14++;
    } else {
        s.col_r = 0x32;
        s.col_g = 0x32;
        s.col_b = 0x5A;
    }
    s.y = 0;
    s.x = 0;
    s.ot_idx = 0x12;
    s.semi = 0;
    s.header = ctx[5];
    s1 = s.header->cells;
    s.table = s1;
    s.sprt_out = arg0->unk_04.unk_0C;
    arg0->unk_04.unk_0C = func_8007352C(&s);
    SetDrawMode(arg0->unk_04.unk_14, 1, 0, func_8006E480(s.header, 0x60), 0);
    AddPrim(g_gpu_ot_ptr + s.ot_idx, arg0->unk_04.unk_14);
    arg0->unk_04.unk_14++;
    if (D_800A3580 < 2) {
        s.ot_idx = 0x12;
        s.semi = 1;
        s.header = ctx[10 + D_800A35C4->unk_08 % 4];
        s1 = s.header->cells;
        s.table = s1;
        s.sprt_out = arg0->unk_04.unk_0C;
        arg0->unk_04.unk_0C = func_8007352C(&s);
        SetDrawMode(arg0->unk_04.unk_14, 1, 0, func_8006E480(s.header, 0x20), 0);
        AddPrim(g_gpu_ot_ptr + s.ot_idx, arg0->unk_04.unk_14);
        arg0->unk_04.unk_14++;
    }
}

/* Q65: this file's initialized small data (.sdata), in address order; values from the original EXE. */
u8 D_800A32E8 = 0;
u8 D_800A32E9 = 0;
