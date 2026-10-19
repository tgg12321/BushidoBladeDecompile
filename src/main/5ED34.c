/* func_8006E534..func_80073200. .text 0x8006E534 (ROM 0x5ED34). Start boundary:
 * G8. Compiled -G8 (Q10, Q44, Q54): the original bytes reach the small globals
 * 0x800A32E8..0x800A35CA straight off $gp, which the original compiler emits
 * only at -G8, and no function outside this range gp-accesses any of them.
 * gpu_LoadVramBottomStrip, func_8006E950, func_8006EA28 and
 * gpu_AddPanelGridTile have no gp access and compile identically at -G8 and -G0
 * (Q54). */
#define INCLUDE_ASM_USE_MACRO_INC 1
#include "common.h"
#include "include_asm.h"
#include "bb2.h"

extern s32 snd_LoadVabPack(Unk8005C2A8Pack *, s16, s32);
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

/* Q65: this file's statics (.sbss, allocated per file by PSYLINK), in
 * address order. */
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
static s32 D_800A3574; /* not named by any code or data: size from the gap */
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
    func_80046AA0();

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
    D_800A356C = gpu_InitPrimBufferPair(D_800A356C, D_800A35AC);
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

Unk8006E49CRec *func_8006E8AC(s32 a0) { return &D_800A35AC[a0]; }

void gpu_LoadVramBottomStrip(Unk8006E950Head *a0) {
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

    cdrom_WaitIdle();
    cdrom_StartRead(cdrom_GetFileIndex(2, a0), (s32)a1);
    cdrom_WaitIdle();
    reloc_RelocateSelfOffsets((s32 *)a1);

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

    gpu_LoadVramBottomStrip(a1);
}

s32 func_8006EA28(Unk8006EA28Rec *a0) {
    reloc_RelocateOffsetList((s32 *)a0, (s32)a0->unk_54);
    reloc_RelocateOffsetList((s32 *)a0, (s32)a0->unk_58);
    reloc_RelocateOffsetList((s32 *)a0, (s32)a0->unk_5C);
    reloc_RelocateOffsetList((s32 *)a0, (s32)a0->unk_60);
    reloc_RelocateOffsetList((s32 *)a0, (s32)a0->unk_64);
    reloc_RelocateOffsetList((s32 *)a0, (s32)a0->unk_68);
    reloc_RelocateOffsetList((s32 *)a0, (s32)a0->unk_6C);
    reloc_RelocateOffsetList((s32 *)a0, (s32)a0->unk_70);
    reloc_RelocateOffsetList((s32 *)a0, (s32)a0->unk_74);
    snd_LoadVabPack(a0->unk_00.unk_00, 1, a0->unk_00.unk_04);
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
    switch (D_800A3578 & 0xFF) {
    case 1:
    case 3:
        D_800A3570 += 0x20;
        if (D_800A3570 >= 0x1E8) {
            D_800A3570 = 0x1E8;
            D_800A3580 = D_800A3584;
            if ((D_800A3578 >> 8) == 0) {
                D_800A3578++;
            }
        }
        break;
    case 2:
    case 4:
        if (D_800A3570 == 0x1E8) {
            snd_QueueSound(5, 0x7F, 0x7F);
        }
        D_800A3570 -= 0x20;
        if (D_800A3570 <= 0) {
            D_800A3570 = 0;
            D_800A3578 = 0;
        }
        break;
    }
}

extern u8 D_8009BC7C[];
extern SelectEntryE534 D_8009BC40[][6];
extern u8 D_800A32E8;
extern u8 D_800A32E9;
extern RECT D_800A32F4;

void func_8006ECF4(Unk8006EACCRec *arg0) {
    Unk8007352CRec s;
    Unk8006ECF4Rec *s3;
    Unk8009B0E0Record *s0;
    s32 var_3;
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
        var_3 = D_8009BC40[D_800A358C[i]][D_800A3588[i]].value;
        if (D_8009BC7C[var_3] & 1) {
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

        switch (var_3) {
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
            s.header = s0 + var_3;
            goto draw;
        }
        if (i != 0) {
            if (D_800A32E8 != var_3 || D_800A32E9 != D_800A3554) {
                rectbuf = D_800A32F4;
                LoadImage(&rectbuf, (u32 *)a2);
                DrawSync(0);
            }
        } else {
            s.header = s0 + var_3;
        }
    draw:
        if (D_800A35B0 == 0 && !(D_8009BC7C[D_800A3560.rec[i].unk1] & 2) &&
            (D_800A35BC != 2 || !D_800A3568->unk14_17)) {
            s.table = s3->unk_00[1];
        } else {
            s.table = s3->unk_00[i];
        }
        s.ft4_out = arg0->unk_04.unk_00;
        if (i != 0) {
            arg0->unk_04.unk_00 = func_80073728(&s, 1);
        } else {
            arg0->unk_04.unk_00 = func_80073728(&s, 0);
        }
    }
}

void gpu_AddScreenDarken(Unk8006EACCRec *arg0) {
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

extern void gpu_AddGradientPanel(Unk8006EACCRec *);
extern void func_80073200(Unk8006EACCRec *);

void func_8006F100(Unk8006EACCRec *arg0) {
    Unk8007352CRec s;
    s32 i;
    Unk8009B0E0Record **base;
    Unk8009B0E0Record *obj;
    s32 var_4;
    s16 t1;
    s16 t2;
    s16 dx;
    s16 dy;

    if (D_800A3550 != 0) {
        D_800A3550 += 0x10;
        if (D_800A3550 >= 0x100) {
            D_800A3550 = 0xFF;
            gpu_AddScreenDarken(arg0);
            D_800A3550 = 0;
        } else {
            gpu_AddScreenDarken(arg0);
        }
        func_8006ECF4(arg0);
        gpu_AddGradientPanel(arg0);
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
        var_4 = 1;
        if (D_800A35BC == 2 && !(D_800A3568->unk14_17) && i == 1) {
            if (D_8009BC7C[D_800A3560.rec[0].unk1] & 2) {
                var_4 = 1;
            } else {
                var_4 = 0;
            }
        } else if (D_8009BC7C[D_800A3560.rec[i].unk1] & 2) {
            var_4 = 0;
        }
        s.header = &obj[var_4];
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
            /* FAKE: constant-holder local: keeps the screen-centre offset added
             * to the shake offset before the table origin (`addiu 0x140` then
             * `addu tbl,v0`); a literal gets reassociated. Both literals
             * inline (cx and cy): score 44 (named-local-fake-exception)
             */
            s32 cx = 0x140;

            s.x = D_8009BC94[i][D_800A3590[i]].x + (t1 + cx) -
                  ((dx * s.scale_x >> 8) / 2);
        }
        {
            /* FAKE: same constant-holder mechanism as `cx` above, for the
             * vertical centre 0x9D; a literal alone scores 26. */
            s32 cy = 0x9D;

            s.y = D_8009BC94[i][D_800A3590[i]].y + (t2 + cy) -
                  ((dy * s.scale_y >> 8) / 2);
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
            snd_QueueSound(7, 0x7F, 0x7F);
        }
        if (D_800A35C8[i] < 0) {
            D_800A35C8[i] = 0;
            snd_QueueSound(8, 0x7F, 0x7F);
        }
    }
    D_800A355C++;
}

void func_8006F528(Unk8006EACCRec *arg0) {
    Unk8007352CRec s;
    RECT rect;
    Unk8009B0E0Record **var_2;
    TILE *prim;
    s16 state;
    /* FAKE: the holder between the header's cells and s.table; stored directly
       at all five sites: score 26 (one site at a time: 13 / 3 / 4 / 3 / 3). */
    Unk8009B400Record *p1;

    s.semi = 0;
    s.y = 0;
    s.scale_x = 0x100;
    s.scale_y = 0x100;
    s.has_color = 0;

    var_2 = D_800A35A8->unk_5C;
    s.ot_idx = 0x10;
    state = D_800A3578 & 0xFF;
    s.header = var_2[0];
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
    s.header = var_2[1];
    p1 = s.header->cells;
    s.table = p1;
    s.ft4_out = arg0->unk_04.unk_00;
    arg0->unk_04.unk_00 = func_80073728(&s, 0);

    s.header = var_2[0];
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
    s.header = var_2[1];
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
           `D_800A35C4->unk_10[0] = offset_x`, the D_800A35C4 load (`lw a1`)
           sinks from the state test to the store, the test's 2 moves from $a2
           to $a1 and offset_x from $v0 to $v1 (score 8). */
        Unk800A35C4Rec *var_6;
        s32 x;
        s32 offset_x;

        var_6 = D_800A35C4;
        x = D_800A35C0->draw.ofs[0];
        if (state == 2) {
            offset_x = x - D_800A3570;
        } else {
            offset_x = x + D_800A3570;
        }
        var_6->unk_10[0] = offset_x;
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

    s.header = var_2[2];
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

extern void menu_UpdateEntryGridCursors(Unk8006EACCRec *);
extern void func_80073200(Unk8006EACCRec *);

void menu_UpdateEntryGrid(Unk8006EACCRec *arg0) {
    Unk8007352CRec s;
    s16 shift[2];
    u16 var_2[4];
    Unk8009B0E0Record **var_3;
    /* the sprite sheet's cell array (8-byte Unk8009B400Record cells), which
       starts just past the sheet's 12-byte Unk8009B0E0Record headers: three
       on var_3[0] (normal, then one highlight per player, +0x24), one on every
       other sheet (+0xC). FAKE: the holder between the header's cells and
       s.table; stored directly: score 98. */
    Unk8009B400Record *var_4;
    s16 i;
    s16 row;
    s16 col;

    s.scale_x = 0x100;
    s.scale_y = 0x100;
    s.semi = 0;
    s.col_r = s.col_g = s.col_b = 0x70;
    var_3 = D_800A35A8->unk_60;
    s.header = var_3[0];
    SetDrawMode(arg0->unk_04.unk_14, 1, 0, gpu_GetSheetTPage(s.header, 0), 0);
    AddPrim(g_gpu_ot_ptr + 0xD, arg0->unk_04.unk_14);
    arg0->unk_04.unk_14++;

    s.ot_idx = 0xC;
    s.x = 0x82;
    s.y = 0x86;
    s.header = var_3[0];
    var_4 = s.header[2].cells;
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
                s.y += (D_800A35C4->unk_00[i] *
                        rsin((D_800A35C4->unk_08 * 288) & 0xFE0)) >>
                       12;
            }
            break;
        }
    }
    s.table = var_4;
    s.sprt_out = arg0->unk_04.unk_0C;
    arg0->unk_04.unk_0C = func_8007352C(&s);

    s.x = 0x17E;
    s.y = 0x86;
    s.header = var_3[0];
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
                s.y += (D_800A35C4->unk_00[i] *
                        rsin((D_800A35C4->unk_08 * 288) & 0xFE0)) >>
                       12;
            }
            break;
        }
    }
    s.table += s.header->count;
    s.sprt_out = arg0->unk_04.unk_0C;
    arg0->unk_04.unk_0C = func_8007352C(&s);

    s.x = 0;
    s.y = 0;
    s.header = var_3[1];
    SetDrawMode(
        arg0->unk_04.unk_14, 1, 0, gpu_GetSheetTPage(s.header, 0x60), 0);
    AddPrim(g_gpu_ot_ptr + 0xB, arg0->unk_04.unk_14);
    arg0->unk_04.unk_14++;

    for (row = 0; row < 4; row++) {
        for (col = 0; col < 5; col++) {
            if ((D_800A35BC == 0 || D_800A35BC == 1 || D_800A35BC == 2 ||
                 D_800A35BC == 3) &&
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
                    if (col == D_800A358C[i] && row == D_800A3588[i] &&
                        D_800A35C4->unk_00[i] != 0) {
                        s.has_color = 1;
                        s.col_r = s.col_g = s.col_b =
                            ((rsin(((D_800A35C4->unk_08 & 0x1F) << shift[i]) +
                                   i * 511) *
                              63) >>
                             12) -
                            0x40;
                        break;
                    }
                }
                s.header = var_3[D_8009BC40[col][row].value + 1];
                var_4 = s.header->cells;
                s.table = var_4;
                s.sprt_out = arg0->unk_04.unk_0C;
                s.ot_idx = 0xA;
                arg0->unk_04.unk_0C = func_8007352C(&s);
            } else {
                s.y = col << 4;
                s.x = row * 116 + (row >> 1) * 20;
                s.has_color = 0;
                /* FAKE: the draw tail is written in both arms, as the two tails
                 * the target cross-jumped; one shared tail after the if/else
                 * puts the a0 setup after the label: score 29
                 * (duplicated-statement-into-arms) */
                s.header = var_3[21];
                var_4 = s.header->cells;
                s.table = var_4;
                s.sprt_out = arg0->unk_04.unk_0C;
                s.ot_idx = 0xA;
                arg0->unk_04.unk_0C = func_8007352C(&s);
            }
        }
    }

    s.y = 0;
    s.x = 0;
    s.has_color = 0;
    s.header = var_3[22];
    var_4 = s.header->cells;
    s.table = var_4;
    s.sprt_out = arg0->unk_04.unk_0C;
    s.ot_idx = 1;
    arg0->unk_04.unk_0C = func_8007352C(&s);
    SetDrawMode(
        arg0->unk_04.unk_14, 1, 0, gpu_GetSheetTPage(s.header, 0x60), 0);
    AddPrim(g_gpu_ot_ptr + 1, arg0->unk_04.unk_14);
    arg0->unk_04.unk_14++;
    var_2[2] = 0xF3;
    var_2[0] = 0xC6;
    var_2[1] = 0x25;
    var_2[3] = 1;
    /* The original passes arg0 (the Unk8006EACCRec) here, unadjusted:
     * func_80069898 reads +0x18 as its TILE cursor, and arg0's +0x18 is the
     * DR_MODE cursor unk_04.unk_14. */
    func_80069898((s32 *)arg0, var_2, 1);
    menu_UpdateEntryGridCursors(arg0);
    func_8006ECF4(arg0);
    D_800A32E8 = D_800A3560.rec[1].unk1;
    D_800A32E9 = D_800A3554;
    gpu_AddGradientPanel(arg0);
    func_80073200(arg0);
}

extern s16 D_800A3530[];
extern s16 D_800A3534[];

void menu_UpdateEntryGridCursors(Unk8006EACCRec *arg0) {
    Unk8007352CRec s;
    Unk8009B0E0Record **var_1;
    s16 *col;
    s16 *row;
    u8 *flags;
    s16 i;
    s16 var_6;
    s16 var_7;

    s.semi = 0;
    s.scale_x = 0x100;
    s.scale_y = 0x100;
    var_1 = D_800A35A8->unk_74;
    s.header = var_1[0];
    SetDrawMode(
        arg0->unk_04.unk_14, 1, 0, gpu_GetSheetTPage(s.header, 0x60), 0);
    AddPrim(g_gpu_ot_ptr + 8, arg0->unk_04.unk_14);
    arg0->unk_04.unk_14++;
    for (i = 0; i < 1 + D_800A35B0 + (var_7 = D_800A3554); i++) {
        col = &D_800A3588[i];
        row = &D_800A358C[i];
        flags = &D_8009BC7C[D_8009BC40[*row][*col].value];
        D_800A3560.rec[i].unk1 = D_8009BC40[*row][*col].value;
        var_6 = i - var_7;
        if (D_800A3560.rec[i].unk0 == 0xFF) {
            D_800A35C4->unk_00[i] = 0x1E;
            if (D_800A354C & (0x4000 << (var_6 * 16))) {
                snd_QueueSound(0, 0x7F, 0x7F);
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
            } else if (D_800A354C & (0x1000 << (var_6 * 16))) {
                snd_QueueSound(0, 0x7F, 0x7F);
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
            if (D_800A354C & (0x2000 << (var_6 * 16))) {
                snd_QueueSound(0, 0x7F, 0x7F);
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
            } else if (D_800A354C & (0x8000 << (var_6 * 16))) {
                snd_QueueSound(0, 0x7F, 0x7F);
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
            s.header = var_1[3];
            s.header->count = D_800A3534[i];
            s.table = s.header->cells;
            s.has_color = 0;
            s.x = *col * 116 + 0x6E + (*col >> 1) * 20;
            s.y = *row * 16 + 0x80;
            s.ot_idx = 7;
            s.sprt_out = arg0->unk_04.unk_0C;
            arg0->unk_04.unk_0C = func_8007352C(&s);
        }
        if ((D_800A354C & (0x40 << (var_6 * 16))) &&
            D_800A3560.rec[i].unk0 == 0xFF) {
            if (*flags & 1) {
                D_800A3560.rec[i].unk2 = 0xFF;
                D_800A3590[i] = 2;
                D_800A3560.rec[i].unk0 = D_8009BC40[*row][*col].unk1;
                snd_QueueSound(D_8009BC40[*row][*col].value + 0xB, 0x7F, 0x7F);
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
                        D_800A3560.rec[1].unk1 =
                            D_8009BC40[0][D_800A3588[1]].value;
                        D_8009BC7C[D_8009BC40[0][D_800A3588[1]].value] |= 4;
                    }
                    break;
                }
            } else {
                snd_QueueSound(0xA, 0x7F, 0x7F);
            }
        } else if (D_800A354C & (0x10 << (var_6 * 16))) {
            if (D_800A3578 == 0) {
                snd_QueueSound(2, 0x7F, 0x7F);
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
            s.header = var_1[i + 1];
        } else {
            s.header = var_1[i];
        }
        s.table = s.header->cells;
        s.col_r = s.col_g = s.col_b =
            ((rsin(((D_800A35C4->unk_08 & 0x1F) << D_800A3530[i]) + i * 511) *
              63) >>
             12) -
            0x40;
        s.x = *col * 116 + 0x4E + (*col >> 1) * 20;
        s.y = *row * 16 + 0x80;
        s.ot_idx = 7;
        s.sprt_out = arg0->unk_04.unk_0C;
        arg0->unk_04.unk_0C = func_8007352C(&s);
    }
    if (D_800A3580 == 0 && D_800A3560.rec[0].unk0 != 0xFF &&
        D_800A35C4->unk_00[0] == 0) {
        if (D_800A35BC == 2 && (D_800A3568->unk14_17)) {
            D_800A3554 = 1;
        }
        if ((D_800A3560.rec[1].unk0 != 0xFF && D_800A35C4->unk_00[1] == 0) ||
            D_800A35B0 + D_800A3554 == 0) {
            D_800A3578 = 1;
            D_800A3558 = 0;
            if (D_800A35BC == 2) {
                D_800A3560.rec[1].unk2 = 0xFF;
                D_800A3590[1] = 2;
            }
            D_800A3584 = 1;
        }
    }
    snd_FlushSoundQueue();
}

extern void menu_UpdateRowCursors(Unk8006EACCRec *a0, Unk8007352CRec *s);
extern void func_8006ECF4(Unk8006EACCRec *);
extern void gpu_AddGradientPanel(Unk8006EACCRec *);
extern void func_80073200(Unk8006EACCRec *);

typedef struct Unk80070C70Rec {
    s16 sp48;
    s16 sp4A;
    s16 sp4C;
    s16 sp4E;
} Unk80070C70Rec;

void menu_UpdateRow(Unk8006EACCRec *arg0) {
    /* FAKE: constant-holder local kept in a callee-saved register across
     * the calls (the target's `li s4,96`, then `move a1,s4`); the inline
     * literal re-materializes `li a1,0x60` at each call site: score 7 */
    s32 c60 = 0x60;
    Unk8007352CRec var_1;
    u16 var_2[4];
    Unk8009B0E0Record **var_3;
    s32 var_s0;
    /* FAKE: the holder between the header's cells and var_1.table; stored
     * directly: score 11. */
    Unk8009B400Record *t;
    u8 code;

    var_1.semi = 0;
    var_1.x = 0;
    var_1.y = 0;
    var_1.scale_x = 0x100;
    var_1.scale_y = 0x100;
    var_1.has_color = 0;
    var_3 = D_800A35A8->unk_64;
    /* Not a coercion: the target stores zero to both fields twice, before
     * and again after the var_3 load. */
    var_1.y = 0;
    var_1.x = 0;
    var_1.header = var_3[1];
    t = var_1.header->cells;
    var_1.table = t;
    var_1.sprt_out = arg0->unk_04.unk_0C;
    var_1.ot_idx = 1;
    arg0->unk_04.unk_0C = func_8007352C(&var_1);
    SetDrawMode(
        arg0->unk_04.unk_14, 1, 0, gpu_GetSheetTPage(var_1.header, c60), 0);
    AddPrim(g_gpu_ot_ptr + 1, arg0->unk_04.unk_14);
    arg0->unk_04.unk_14++;
    var_2[2] = 0xE7;
    var_2[0] = 0xCC;
    var_2[1] = 0x25;
    var_2[3] = 1;
    /* The original passes arg0 (the Unk8006EACCRec) here, unadjusted:
     * func_80069898 reads +0x18 as its TILE cursor, and arg0's +0x18 is the
     * DR_MODE cursor unk_04.unk_14. */
    func_80069898((s32 *)arg0, var_2, 1);
    var_1.header = var_3[0];
    t = var_1.header[5].cells;
    var_1.table = t;
    for (var_s0 = 0; var_s0 < 6; var_s0++) {
        var_1.x = var_s0 << 6;
        var_1.sprt_out = arg0->unk_04.unk_0C;
        var_1.ot_idx = 0xA;
        arg0->unk_04.unk_0C = func_8007352C(&var_1);
        var_1.header++;
    }
    var_1.header = var_3[0];
    SetDrawMode(
        arg0->unk_04.unk_14, 1, 0, gpu_GetSheetTPage(var_1.header, c60), 0);
    AddPrim(g_gpu_ot_ptr + 0xA, arg0->unk_04.unk_14);
    arg0->unk_04.unk_14++;
    var_1.header = var_3[2];
    for (var_s0 = 0; var_s0 < 1 + D_800A35B0 + D_800A3558; var_s0++) {
        code = D_800A3560.rec[var_s0].unk0;
        if ((code != 5) && (code != 16)) {
            t = var_1.header->cells;
            var_1.table = t;
            var_1.table += D_800A3590[var_s0] * 2;
            if (((D_800A35B0 + D_800A3558) != 0) || (D_800A35BC == 2)) {
                var_1.x = 0x50 + var_s0 * 0x16C;
            } else {
                var_1.x = 0x105;
            }
            var_1.sprt_out = arg0->unk_04.unk_0C;
            var_1.ot_idx = 1;
            arg0->unk_04.unk_0C = func_8007352C(&var_1);
        }
    }
    SetDrawMode(
        arg0->unk_04.unk_14, 1, 0, gpu_GetSheetTPage(var_1.header, c60), 0);
    AddPrim(g_gpu_ot_ptr + 1, arg0->unk_04.unk_14);
    arg0->unk_04.unk_14++;
    menu_UpdateRowCursors(arg0, &var_1);
    func_8006ECF4(arg0);
    gpu_AddGradientPanel(arg0);
    func_80073200(arg0);
}

extern u8 D_8009BC38[];

void menu_UpdateRowCursors(Unk8006EACCRec *arg0, Unk8007352CRec *s) {
    /* several values of one kind, a sprite-sheet header table:
     * D_800A35A8->unk_74 (loaded at entry and again in the selected-slot arm)
     * and D_800A35A8->unk_60 (the table the last loop draws from) (Ruling 11,
     * ordinary-c-judge-decidable). */
    Unk8009B0E0Record **var_0;
    s32 flag;
    /* several values of one kind: the cell table following a sheet
     * header (s->header->cells in the two loop-2 draws, s->header[2].cells
     * for the last loop) (Ruling 11, proof at tag pre-slim-2026-10-01). FAKE:
     * the holder between the header's cells and s->table; stored directly:
     * score 9. */
    Unk8009B400Record *var_2;
    s16 i;
    s16 var_4;
    s16 var_5;

    flag = 0;
    var_0 = D_800A35A8->unk_74;
    s->header = var_0[0];
    SetDrawMode(
        arg0->unk_04.unk_14, 1, 0, gpu_GetSheetTPage(s->header, 0x60), 0);
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
    for (i = 0; i < 1 + D_800A35B0 + (var_5 = D_800A3558); i++) {
        /* several values of one kind: player i's VRAM rect row,
         * D_800A35A8->unk_7C + i * 8, computed at the top of the loop and
         * again in the locked-slot arm (Ruling 11, proof at tag
         * pre-slim-2026-10-01). */
        RECT *var_6;

        /* FAKE: base and step as two statements, here and in the locked-slot
         * arm; `var_6 = D_800A35A8->unk_7C + i * 8` scores 28 */
        var_6 = D_800A35A8->unk_7C;
        var_6 += i * 8;
        var_4 = i - var_5;
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
                if ((D_800A354C & (0x2000 << (var_4 * 16))) &&
                    D_800A3590[i] < max) {
                    snd_QueueSound(0, 0x7F, 0x7F);
                    D_800A3590[i]++;
                } else if ((D_800A354C & (0x8000 << (var_4 * 16))) &&
                           min < D_800A3590[i]) {
                    snd_QueueSound(0, 0x7F, 0x7F);
                    D_800A3590[i]--;
                }
                D_800A3594[i] = i;
                if (D_800A35BC == 3) {
                    s32 sel;
                    s32 id;
                    s32 *tim;

                    snd_QueueSound(1, 0x7F, 0x7F);
                    D_800A3560.rec[1].unk2 = 0;
                    D_800A3560.rec[0].unk2 = 0;
                    if (D_8009BC7C[D_800A3560.rec[i].unk1] & 2) {
                        sel = 0;
                    } else {
                        sel = 1;
                    }
                    id = D_800A3560.rec[i].unk2;
                    tim = &D_800A35A8->unk_14[id][sel];
                    LoadImage(&var_6[id], (u32 *)*tim);
                    DrawSync(0);
                } else if (D_800A354C & (0x40 << (var_4 * 16))) {
                    s32 sel;
                    s32 id;
                    s32 *tim;

                    snd_QueueSound(1, 0x7F, 0x7F);
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
                    LoadImage(&var_6[id], (u32 *)*tim);
                    DrawSync(0);
                } else if (D_800A354C & (0x10 << (var_4 * 16))) {
                    snd_QueueSound(2, 0x7F, 0x7F);
                    if (D_800A3558 == 1) {
                        D_800A3558 = 0;
                        D_800A3560.rec[0].unk2 = 0xFF;
                    } else {
                        D_800A3560.rec[1].unk2 = 0xFF;
                        D_800A3560.rec[0].unk2 = 0xFF;
                        D_800A3578 = 3;
                        D_800A3584 = 0;
                        snd_QueueSound(6, 0x7F, 0x7F);
                        if (D_800A3554 == 1) {
                            D_800A3560.rec[1].unk0 = 0xFF;
                        } else {
                            D_800A3560.rec[i].unk0 = 0xFF;
                        }
                        break;
                    }
                }
            } else {
                if ((D_800A354C & (0x10 << (var_4 * 16))) && D_800A3578 == 0 &&
                    D_800A35BC != 3) {
                    snd_QueueSound(2, 0x7F, 0x7F);
                    D_800A35C8[0] = 0xF;
                    D_800A35C8[1] = 0x14;
                    D_800A3560.rec[i].unk2 = 0xFF;
                }
                D_800A3544[i] = 9;
                if (D_800A35C4->unk_04[i] != 0) {
                    D_800A35C4->unk_04[i]--;
                }
                var_0 = D_800A35A8->unk_74;
                s->header = var_0[3];
                if (D_800A3540[i] < 12) {
                    D_800A3540[i]++;
                }
                s->header->count = D_800A3540[i];
                s->has_color = 0;
                var_2 = s->header->cells;
                s->table = var_2;
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
                    s->header = var_0[i + 1];
                } else {
                    s->header = var_0[i];
                }
                var_2 = s->header->cells;
                s->table = var_2;
                s->col_r = s->col_g = s->col_b =
                    ((rsin(((D_800A35C4->unk_08 & 0x1F) << D_800A3544[i]) +
                           i * 511) *
                      63) >>
                     12) -
                    0x40;
                s->x = (D_800A3590[i] << 6) + 0x80;
                s->y = 0xAC - (D_800A3594[i] << 5);
                s->ft4_out = arg0->unk_04.unk_00;
                s->ot_idx = 7;
                arg0->unk_04.unk_00 = func_80073728(s, 0);
            }
        } else {
            if (D_800A3578 == 0) {
                if (D_800A3560.rec[i == 0 ? 1 : 0].unk2 != 0xFF &&
                    D_800A35C4->unk_04[i == 0 ? 1 : 0] == 0) {
                    s32 sel;
                    s32 id;
                    s32 *tim;

                    snd_QueueSound(1, 0x7F, 0x7F);
                    flag = 2;
                    var_6 = D_800A35A8->unk_7C;
                    var_6 += i * 8;
                    if (D_8009BC7C[D_800A3560.rec[i].unk1] & 2) {
                        sel = 0;
                    } else {
                        sel = 1;
                    }
                    id = D_800A3560.rec[i].unk2;
                    tim = &D_800A35A8->unk_14[id][sel];
                    LoadImage(&var_6[id], (u32 *)*tim);
                    DrawSync(0);
                }
            }
            if (D_800A354C & (0x10 << (i * 16))) {
                snd_QueueSound(2, 0x7F, 0x7F);
                D_800A3578 = 3;
                snd_QueueSound(6, 0x7F, 0x7F);
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
    var_0 = D_800A35A8->unk_60;
    s->header = var_0[0];
    s->scale_x = 0x80;
    s->scale_y = 0x80;
    s->has_color = 0;
    s->ot_idx = 0xC;
    var_2 = s->header[2].cells;
    for (i = 0; i < 1 + D_800A35B0 + D_800A3558; i++) {
        if (D_800A3560.rec[i].unk0 != 5 && D_800A3560.rec[i].unk0 != 0x10 &&
            D_800A3578 != 3) {
            s->table = var_2;
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
        if (((D_800A3560.rec[1].unk2 != 0xFF && D_800A35C4->unk_04[1] == 0) ||
             D_800A35B0 + D_800A3558 == 0) &&
            flag != 1) {
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
    Unk8007352CRec s;
    s32 i;
    Unk8009B0E0Record **base;
    Unk8009B0E0Record *obj;
    s32 var_4;
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
            var_4 = 1;
            if (D_800A35BC == 2 && !(D_800A3568->unk14_17) && i == 1) {
                if (D_8009BC7C[D_800A3560.rec[0].unk1] & 2) {
                    var_4 = 1;
                } else {
                    var_4 = 0;
                }
            } else if (D_8009BC7C[D_800A3560.rec[i].unk1] & 2) {
                var_4 = 0;
            }
            s.header = &obj[var_4];
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
            s.x = D_8009BC94[i][D_800A3590[i]].x + 0x140 -
                  ((dx * s.scale_x >> 8) / 2);
            s.y = D_8009BC94[i][D_800A3590[i]].y + 0x9D -
                  ((dy * s.scale_y >> 8) / 2);
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
        snd_QueueSound(6, 0x7F, 0x7F);
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
            D_800A3568->unk00[i][0].bd0 = D_800A3560.rec[i].unk0;
        }
        for (i = 0; i < 1 + D_800A35B0 + D_800A3558; i++) {
            D_800A3568->unk00[i][0].unk1 = D_800A3560.rec[i].unk2;
        }
    }
    gpu_AddScreenDarken(arg0);
}

void menu_UpdateChoiceGrid(Unk8006EACCRec *, Unk8009B0E0Record **, s32);

void func_80072084(Unk8006EACCRec *a0) {
    menu_UpdateChoiceGrid(a0, D_800A35A8->unk_68, 0);
}

void func_800720AC(Unk8006EACCRec *a0) {
    menu_UpdateChoiceGrid(a0, D_800A35A8->unk_6C, 1);
}

void func_800720D4(Unk8006EACCRec *a0) {
    menu_UpdateChoiceGrid(a0, D_800A35A8->unk_70, 2);
}

void menu_UpdateChoiceGrid(
    Unk8006EACCRec *arg0, Unk8009B0E0Record **arg1, s32 mode) {
    Unk8007352CRec s;
    u16 var_1[4];
    RECT rect;
    u8 *var_3;
    Unk8009B0E0Record **var_4;
    /* several values of one kind: the cell table following the
     * header(s) of each sheet drawn (s.header[1].cells for the two-header
     * sheet, s.header->cells for the others) (Ruling 11,
     * ordinary-c-judge-decidable). FAKE: the holder
     * between the header's cells and s.table; stored directly: score 123. */
    Unk8009B400Record *var_5;
    s32 i;
    s32 j;
    s32 d;
    u8 code;
    u8 var_10;

    var_3 = D_800A35A8->unk_80;
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
        var_4 = D_800A35A8->unk_74;
        s.header = var_4[4];
        var_5 = s.header[1].cells;
        if (!(D_800A359C * 2 + D_800A3598 == 3 && D_800A35BC == 6 &&
              mode == 2)) {
            s.table = var_5;
            s.table += (D_800A359C + mode * 3) * 2 + D_800A3598;
            s.sprt_out = arg0->unk_04.unk_0C;
            arg0->unk_04.unk_0C = func_8007352C(&s);
        }
        s.header++;
        for (i = 0; i < 6; i++) {
            if (!(i == 3 && D_800A35BC == 6 && mode == 2) &&
                D_800A359C * 2 + D_800A3598 != i) {
                s.table = var_5 + (mode * 6 + i);
                s.sprt_out = arg0->unk_04.unk_0C;
                arg0->unk_04.unk_0C = func_8007352C(&s);
            }
        }
        s.header = var_4[4];
        SetDrawMode(
            arg0->unk_04.unk_14, 1, 0, gpu_GetSheetTPage(s.header, 0), 0);
        AddPrim(g_gpu_ot_ptr + s.ot_idx, arg0->unk_04.unk_14);
        arg0->unk_04.unk_14++;
    }

    s.x = 0x90;
    s.y = 0x28;
    s.header = arg1[0];
    var_5 = s.header->cells;
    s.table = var_5;
    s.sprt_out = arg0->unk_04.unk_0C;
    arg0->unk_04.unk_0C = func_8007352C(&s);
    D_800A35C4->unk_10[0] = D_800A35C0->draw.ofs[0] - D_8009BCC4[mode][0];
    D_800A35C4->unk_10[1] = D_800A35C0->draw.ofs[1] - D_8009BCC4[mode][1];
    if (((D_800A3578 & 0xFF) == 1 || (D_800A3578 & 0xFF) == 3) &&
        D_800A3584 >= 4 && D_800A3584 < 7 && D_800A3580 >= 4 &&
        D_800A3580 < 7) {
        for (i = 0; i < 2; i++) {
            d = D_8009BCC4[D_800A3584 - 4][i] - D_8009BCC4[D_800A3580 - 4][i];
            d *= 30;
            if ((d >= 0 ? d : -d) >
                (D_8009BCD0[i] >= 0 ? D_8009BCD0[i] : -D_8009BCD0[i])) {
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
    SetDrawMode(arg0->unk_04.unk_14, 1, 0, gpu_GetSheetTPage(s.header, 0), 0);
    AddPrim(g_gpu_ot_ptr + s.ot_idx, arg0->unk_04.unk_14);
    arg0->unk_04.unk_14++;

    s.scale_x = 0x100;
    s.scale_y = 0x100;
    s.y = 0;
    s.x = 0;
    s.ot_idx = 0xB;
    s.header = arg1[1];
    s.scale_x += 0x80;
    s.scale_y += 0x20;
    var_5 = s.header->cells;
    s.table = var_5;
    s.ft4_out = arg0->unk_04.unk_00;
    arg0->unk_04.unk_00 = func_80073728(&s, 0);
    s.table += s.header->count;
    s.ft4_out = arg0->unk_04.unk_00;
    arg0->unk_04.unk_00 = func_80073728(&s, 1);

    if (D_800A354C & 0xA000A000) {
        if ((D_800A3578 & 0xFF) == 0) {
            snd_QueueSound(0, 0x7F, 0x7F);
            if (((D_800A354C & 0x20002000) && D_800A3598 != 0) ||
                ((D_800A354C & 0x80008000) && D_800A3598 == 0)) {
                code = var_3[(D_800A3580 - 4) * 8 + 3 * 2 + D_800A3598];
                D_800A3584 = code & 0xF;
                if (D_800A3584 != 0xF) {
                    D_800A3578 = code >> 4;
                    snd_QueueSound(6, 0x7F, 0x7F);
                }
            }
            D_800A3598 = D_800A3598 == 0;
        }
    }
    if (D_800A354C & 0x40004000) {
        snd_QueueSound(0, 0x7F, 0x7F);
        D_800A359C++;
    } else if (D_800A354C & 0x10001000) {
        snd_QueueSound(0, 0x7F, 0x7F);
        D_800A359C--;
    }
    if (D_800A359C >= 3) {
        D_800A359C = 0;
    } else if (D_800A359C < 0) {
        D_800A359C = 2;
    }

    s.col_r = s.col_g = s.col_b =
        ((rsin((D_800A35C4->unk_08 & 0x1F) * 128 + 0x1FF) * 63) >> 12) - 0x40;
    s.scale_x = 0x100;
    s.scale_y = 0x100;
    s.y = 0;
    s.x = 0;
    s.ot_idx = 1;
    s.header = arg1[2];
    var_5 = s.header->cells;
    s.table = var_5;
    s.ft4_out = arg0->unk_04.unk_00;
    arg0->unk_04.unk_00 = func_80073728(&s, 0);
    var_1[2] = 0x111;
    var_1[0] = 0xB7;
    var_1[1] = 0x25;
    var_1[3] = 1;
    /* The original passes arg0 (the Unk8006EACCRec) here, unadjusted:
     * func_80069898 reads +0x18 as its TILE cursor, and arg0's +0x18 is the
     * DR_MODE cursor unk_04.unk_14. */
    func_80069898((s32 *)arg0, var_1, 1);
    s.ot_idx = 0xA;
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 2; j++) {
            if (i == D_800A359C && j == D_800A3598 &&
                (D_800A3578 & 0xFF) == 0) {
                s.has_color = 1;
            } else {
                s.has_color = 0;
            }
            if (i + i + j != 3 || D_800A35BC != 6 || mode != 2) {
                s.header = (arg1 + (i + i))[j + 3];
            } else {
                s.header = arg1[12];
            }
            var_5 = s.header->cells;
            s.table = var_5;
            s.ft4_out = arg0->unk_04.unk_00;
            arg0->unk_04.unk_00 = func_80073728(&s, 0);
        }
    }
    s.has_color = 0;
    s.header = arg1[11];
    var_5 = s.header->cells;
    s.table = var_5;
    s.ft4_out = arg0->unk_04.unk_00;
    arg0->unk_04.unk_00 = func_80073728(&s, 0);

    if (D_800A3578 == 0) {
        for (i = 0; i < D_800A35B0 + 1; i++) {
            if (D_800A354C & (0x10 << (i * 16))) {
                s32 var_11;

                snd_QueueSound(2, 0x7F, 0x7F);
                var_11 = D_800A3560.rec[i].unk0;
                D_800A3560.rec[i].unk2 = 0xFF;
                if (var_11 == 5 || var_11 == 16) {
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
            var_10 = var_3[(D_800A3580 - 4) * 8 + D_800A359C * 2 + D_800A3598];
            snd_QueueSound(1, 0x7F, 0x7F);
            if (var_10 != 0xD || D_800A35BC != 6) {
                D_800A3568->unk14_4 = var_10;
            } else {
                D_800A3568->unk14_4 = 0x25;
            }
            D_800A35A0 = 1;
        }
    }
end:
    gpu_AddGradientPanel(arg0);
    func_80073200(arg0);
    snd_FlushSoundQueue();
}

POLY_G4 *gpu_AddPanelUpperQuad(s32 arg0, POLY_G4 *arg1) {
    u8 var_v0;
    /* FAKE: constant holder -- 0xFC held from before the branch; the literal at
     * each store rematerialises it at the r3 store and swaps v0 / v1: score 5
     */
    int var_1;

    SetPolyG4(arg1);
    SetSemiTrans(arg1, 0);
    var_1 = 0xFC;
    if (arg0 < 4) {
        arg1->r0 = 0;
        arg1->g0 = 0;
        arg1->b0 = 0;
        arg1->r1 = var_1;
        arg1->g1 = 0x82;
        arg1->b1 = 0;
        arg1->r2 = var_1;
        arg1->g2 = 0x82;
        arg1->b2 = 0;
        if (D_800A35C4->unk_08 & 4) {
            arg1->g3 = 0xC3;
            var_v0 = 0x1E;
        } else {
            arg1->g3 = 0xC3;
            var_v0 = 0x50;
        }
        arg1->r3 = var_1;
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

/* gpu_AddPanelLowerQuad - colours a Gouraud quad (POLY_G4) by mode: warm
 * per-vertex RGB triples (two variants on D_800A35C4's flag 4) for modes < 4, a
 * fixed dark set otherwise; then adds it to the OT and returns the next
 * primitive slot. */
POLY_G4 *gpu_AddPanelLowerQuad(s32 arg0, POLY_G4 *arg1) {
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

void gpu_AddGradientPanel(Unk8006EACCRec *arg0) {
    POLY_G4 *p;
    gpu_AddPanelGrid(arg0);
    p = arg0->unk_04.unk_08;
    p->x0 = 0x50;
    p->y0 = 0x32;
    p->x1 = 0x50;
    p->y1 = 0x52;
    p->x2 = 0x140;
    p->y2 = 0x32;
    p->x3 = 0x140;
    p->y3 = 0x52;
    p = gpu_AddPanelUpperQuad(D_800A3580, p);
    p->x0 = 0x231;
    p->y0 = 0x32;
    p->x1 = 0x231;
    p->y1 = 0x52;
    p->x2 = 0x140;
    p->y2 = 0x32;
    p->x3 = 0x140;
    p->y3 = 0x52;
    p = gpu_AddPanelUpperQuad(D_800A3580, p);
    p->x0 = 0x140;
    p->y0 = 0x52;
    p->x1 = 0x140;
    p->y1 = 0x71;
    p->x2 = 0x50;
    p->y2 = 0x52;
    p->x3 = 0x50;
    p->y3 = 0x71;
    p = gpu_AddPanelLowerQuad(D_800A3580, p);
    p->x0 = 0x140;
    p->y0 = 0x52;
    p->x1 = 0x140;
    p->y1 = 0x71;
    p->x2 = 0x231;
    p->y2 = 0x52;
    p->x3 = 0x231;
    p->y3 = 0x71;
    p = gpu_AddPanelLowerQuad(D_800A3580, p);
    arg0->unk_04.unk_08 = p;
}

TILE *gpu_AddPanelGridTile(s32 a0, TILE *a1) {
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

TILE *func_80072FCC(s32 arg0, TILE *a1) {
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

void gpu_AddPanelGrid(Unk8006EACCRec *arg0) {
    TILE *p;
    s32 i;
    p = arg0->unk_04.unk_10;
    for (i = 0x6F; i < 0x210; i += 0x20) {
        p->x0 = i;
        p->y0 = 0x32;
        p->w = 2;
        p->h = 0x3F;
        p = gpu_AddPanelGridTile(D_800A3580, p);
    }
    p->x0 = 0x51;
    p->y0 = 0x41;
    p->w = 0x1E0;
    p->h = 1;
    p = gpu_AddPanelGridTile(D_800A3580, p);
    p->x0 = 0x51;
    p->y0 = 0x61;
    p->w = 0x1E0;
    p->h = 1;
    p = gpu_AddPanelGridTile(D_800A3580, p);
    p->x0 = 0x51;
    p->y0 = 0x50;
    p->w = 0x1E0;
    p->h = 2;
    p = gpu_AddPanelGridTile(D_800A3580, p);
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

/* func_80073200 - draws three sprite-sheet layers: in modes D_800A3580 < 4,
 * four cells of var_1[4]'s sheet through func_80073728; then var_1[5]'s sheet
 * and, for D_800A3580 < 2, the var_1[10..13] sheet picked by the frame counter
 * at D_800A35C4->unk_08 (incremented in func_8006EACC), each
 * through func_8007352C, with a draw-mode primitive per layer. */
void func_80073200(Unk8006EACCRec *arg0) {
    Unk8007352CRec s;
    Unk8009B0E0Record **var_1;
    /* FAKE: the holder between the header's cells and s.table; stored directly:
     * score 15. */
    Unk8009B400Record *s1;

    s.x = 0;
    s.y = 0;
    s.scale_x = 0x100;
    s.scale_y = 0x100;
    var_1 = D_800A35A8->unk_5C;
    s.header = var_1[3];
    SetDrawMode(arg0->unk_04.unk_14, 1, 0, gpu_GetSheetTPage(s.header, 0), 0);
    AddPrim(g_gpu_ot_ptr + 0x1C, arg0->unk_04.unk_14);
    arg0->unk_04.unk_14++;
    s.has_color = 1;
    s.header = var_1[4];
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
            /* FAKE: `s.col_b = 0x14;` written in both arms instead of once at
             * the join (cross-jump re-merges them: no extra insn). Keeping
             * the sb stores out of the join block lets the first
             * func_80073728 call's `&s` argument set land after them, as in
             * the target; stored once at the join it is emitted first:
             * score 2 */
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
    s.header = var_1[5];
    s1 = s.header->cells;
    s.table = s1;
    s.sprt_out = arg0->unk_04.unk_0C;
    arg0->unk_04.unk_0C = func_8007352C(&s);
    SetDrawMode(
        arg0->unk_04.unk_14, 1, 0, gpu_GetSheetTPage(s.header, 0x60), 0);
    AddPrim(g_gpu_ot_ptr + s.ot_idx, arg0->unk_04.unk_14);
    arg0->unk_04.unk_14++;
    if (D_800A3580 < 2) {
        s.ot_idx = 0x12;
        s.semi = 1;
        s.header = var_1[10 + D_800A35C4->unk_08 % 4];
        s1 = s.header->cells;
        s.table = s1;
        s.sprt_out = arg0->unk_04.unk_0C;
        arg0->unk_04.unk_0C = func_8007352C(&s);
        SetDrawMode(
            arg0->unk_04.unk_14, 1, 0, gpu_GetSheetTPage(s.header, 0x20), 0);
        AddPrim(g_gpu_ot_ptr + s.ot_idx, arg0->unk_04.unk_14);
        arg0->unk_04.unk_14++;
    }
}

/* Q65: this file's initialized small data (.sdata), in address order. */
u8 D_800A32E8 = 0;
u8 D_800A32E9 = 0;
