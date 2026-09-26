typedef struct {
    s32 p0;
    s32 p1;
    s32 chain;
    s32 chain2;
    s32 flag10;
    s32 ot_idx;
    s32 x;
    s32 y;
    s32 w;
    s32 h;
    s8 flag28;
    u8 r, g, b;
} S_6A880;

typedef struct {
    s16 x, y, w, h;
} Rect6A880;

extern s32 D_8009BC04;
extern s32 D_8009BC08;
extern s32 SetDrawOffset();

void func_8006A880(u8 *arg0, u16 *arg1, s32 arg2) {
    S_6A880 s;
    Rect6A880 rect;
    s16 ofs[2];
    s32 *tbl;
    s32 *cursor;
    u32 mask;

    s32 cells;
    s32 yofs;
    s32 y;
    s32 i;

    tbl = *(s32 **)(*(s32 *)(arg0 + 4) + 0x18);
    mask = D_8009BC08;
    s.p0 = tbl[0];
    SetDrawMode(*(s32 *)(arg0 + 0x1C), 1, 0, func_8006E480(s.p0, 0), 0);
    AddPrim(g_gpu_ot_ptr + 0x30, *(s32 *)(arg0 + 0x1C));
    *(s32 *)(arg0 + 0x1C) += 0xC;
    rect.x = arg1[0];
    rect.y = arg1[1] + 0x3A;
    rect.w = 0xF6;
    rect.h = 0x92;
    SetDrawArea(*(s32 *)(arg0 + 0x20), &rect);
    AddPrim(g_gpu_ot_ptr + 0x30, *(s32 *)(arg0 + 0x20));
    *(s32 *)(arg0 + 0x20) += 0xC;
    yofs = 0;
    for (i = 0; !(mask & (1 << i)); i++) {
        yofs -= 0x18;
    }
    ofs[0] = arg1[4];
    ofs[1] = arg1[5] + yofs;
    SetDrawOffset(*(s32 *)(arg0 + 0x24), ofs);
    AddPrim(g_gpu_ot_ptr + 0x30, *(s32 *)(arg0 + 0x24));
    *(s32 *)(arg0 + 0x24) += 0xC;

    mask = D_8009BC04;

    s.ot_idx = 0xB;
    for (i = 0; i < 7; i++) {
        y = i * 0x18 + 0x3F;
        s.x = 0x76;
        s.flag28 = 1;
        if ((D_800A34F8 & 0xF) == i) {
            s.flag10 = 0;
            s.r = ((rsin(((D_800A3514 & 0x1F) << 7) + 0x1FF) * 63) >> 12) - 0x61;
            s.g = ((rsin(((D_800A3514 & 0x1F) << 7) + 0x1FF) * 45) >> 12) - 0x7D;
            s.b = 0x30;
            s.y = *(s16 *)(D_800A34FC + 0xE) + y;
            D_800A3514++;
        } else if ((mask >> i) & 1) {
            s.r = 0x80;
            s.g = 0x6C;
            s.b = 0x30;
            s.flag10 = 1;
            s.y = y;
            if (((s32 *)D_800A3524)[8] & 1) {
                s.flag28 = 0;
            }
        } else {
            s.r = 0x30;
            s.g = 0x30;
            s.b = 0x30;
            s.flag10 = 1;
            s.y = y;
        }
        s.p0 = tbl[i];
        cells = s.p0 + 0xC;
        s.p1 = cells;
        s.chain = *(s32 *)(arg0 + 0x14);
        *(s32 *)(arg0 + 0x14) = func_8007352C((s32)&s);
        SetDrawMode(*(s32 *)(arg0 + 0x1C), 1, 0, func_8006E480(s.p0, 0), 0);
        AddPrim(g_gpu_ot_ptr + s.ot_idx * 4, *(s32 *)(arg0 + 0x1C));
        *(s32 *)(arg0 + 0x1C) += 0xC;
        s.flag28 = 1;
        func_8006A564(arg0, (u8 *)&s, i);

    }

    tbl = *(s32 **)(*(s32 *)(arg0 + 4) + 0x40);
    s.ot_idx = 0xA;
    s.flag28 = 0;
    s.flag10 = 0;
    s.x = 0;
    s.y = 0x41;
    s.p0 = tbl[D_800A34F8 & 0xF];
    cells = s.p0 + 0xC;
    s.p1 = cells;
    s.chain = *(s32 *)(arg0 + 0x14);
    *(s32 *)(arg0 + 0x14) = func_8007352C((s32)&s);
    SetDrawMode(*(s32 *)(arg0 + 0x1C), 1, 0, func_8006E480(s.p0, 0), 0);
    AddPrim(g_gpu_ot_ptr + s.ot_idx * 4, *(s32 *)(arg0 + 0x1C));
    *(s32 *)(arg0 + 0x1C) += 0xC;

    tbl = *(s32 **)(*(s32 *)(arg0 + 4) + 0x18);
    rect.x = arg1[0];
    rect.y = arg1[1];
    rect.w = arg1[2];
    rect.h = arg1[3];
    SetDrawArea(*(s32 *)(arg0 + 0x20), &rect);
    AddPrim(g_gpu_ot_ptr + 0x28, *(s32 *)(arg0 + 0x20));
    *(s32 *)(arg0 + 0x20) += 0xC;
    ofs[0] = arg1[4];
    ofs[1] = arg1[5];
    SetDrawOffset(*(s32 *)(arg0 + 0x24), ofs);
    AddPrim(g_gpu_ot_ptr + 0x28, *(s32 *)(arg0 + 0x24));
    *(s32 *)(arg0 + 0x24) += 0xC;

    s.x = 0x76;
    s.flag28 = 1;
    if ((D_800A34F8 & 0xF) == 7) {
        s.flag10 = 0;
        s.r = ((rsin(((D_800A3514 & 0x1F) << 7) + 0x1FF) * 63) >> 12) - 0x61;
        s.g = ((rsin(((D_800A3514 & 0x1F) << 7) + 0x1FF) * 45) >> 12) - 0x7D;
        s.b = 0x30;
        s.y = *(s16 *)(D_800A34FC + 0xE) + 0xCF;
        D_800A3514++;
    } else if ((mask >> 7) & 1) {
        s.r = 0x80;
        s.g = 0x6C;
        s.b = 0x30;
        s.flag10 = 1;
        s.y = 0xCF;
        if (((s32 *)D_800A3524)[8] & 1) {
            s.flag28 = 0;
        }
    } else {
        s.r = 0x30;
        s.g = 0x30;
        s.b = 0x30;
        s.flag10 = 1;
        s.y = 0xCF;
    }
    s.ot_idx = 9;
    s.p0 = tbl[7];
    cells = s.p0 + 0xC;
    s.p1 = cells;
    s.chain = *(s32 *)(arg0 + 0x14);
    *(s32 *)(arg0 + 0x14) = func_8007352C((s32)&s);
    SetDrawMode(*(s32 *)(arg0 + 0x1C), 1, 0, func_8006E480(s.p0, 0), 0);
    AddPrim(g_gpu_ot_ptr + s.ot_idx * 4, *(s32 *)(arg0 + 0x1C));
    *(s32 *)(arg0 + 0x1C) += 0xC;
    s.flag28 = 1;
    func_8006A564(arg0, (u8 *)&s, 7);
    func_8006A494((s32 *)arg0, (u8 *)&s);

    s.flag28 = 0;
    SetDrawMode(*(s32 *)(arg0 + 0x1C), 1, 0, func_8006E480(s.p0, 0), 0);
    AddPrim(g_gpu_ot_ptr + 4, *(s32 *)(arg0 + 0x1C));
    *(s32 *)(arg0 + 0x1C) += 0xC;
    SetDrawMode(*(s32 *)(arg0 + 0x1C), 1, 0, func_8006E480(s.p0, 0), 0);
    AddPrim(g_gpu_ot_ptr + 4, *(s32 *)(arg0 + 0x1C));
    *(s32 *)(arg0 + 0x1C) += 0xC;

    tbl = *(s32 **)(*(s32 *)(arg0 + 4) + 0x24);
    s.p0 = tbl[(D_800A34F8 & 0xF) + 8];
    s.x = 0;
    s.y = 0x19;
    s.flag28 = 0;
    s.flag10 = 0;
    s.ot_idx = 0;
    cells = s.p0 + 0xC;
    s.p1 = cells;
    s.chain = *(s32 *)(arg0 + 0x14);
    *(s32 *)(arg0 + 0x14) = func_8007352C((s32)&s);
    SetDrawMode(*(s32 *)(arg0 + 0x1C), 1, 0, func_8006E480(s.p0, 0), 0);
    AddPrim(g_gpu_ot_ptr, *(s32 *)(arg0 + 0x1C));
    *(s32 *)(arg0 + 0x1C) += 0xC;

    tbl = *(s32 **)(*(s32 *)(arg0 + 4) + 0x24);
    s.p0 = tbl[D_800A34F8 & 0xF];
    s.w = 0x200;
    s.x = 0;
    s.y = 0;
    s.h = 0x100;
    s.flag28 = 0;
    s.flag10 = 0;
    s.ot_idx = 0;
    cells = s.p0 + 0xC;
    s.p1 = cells;
    s.chain2 = *(s32 *)(arg0 + 8);
    *(s32 *)(arg0 + 8) = func_80073728((s32)&s, 0);

    SetTile(*(u8 **)(arg0 + 0x18));
    (*(u8 **)(arg0 + 0x18))[4] = 0;
    (*(u8 **)(arg0 + 0x18))[5] = 0;
    (*(u8 **)(arg0 + 0x18))[6] = 0;
    *(s16 *)(*(u8 **)(arg0 + 0x18) + 8) = 0x142;
    *(s16 *)(*(u8 **)(arg0 + 0x18) + 0xA) = 0x51;
    *(s16 *)(*(u8 **)(arg0 + 0x18) + 0xC) = 0x100;
    *(s16 *)(*(u8 **)(arg0 + 0x18) + 0xE) = 0x59;
    SetSemiTrans(*(u8 **)(arg0 + 0x18), 0);
    AddPrim(g_gpu_ot_ptr + 4, *(u8 **)(arg0 + 0x18));
    *(u8 **)(arg0 + 0x18) += 0x10;

    SetDrawMode(*(s32 *)(arg0 + 0x1C), 1, 0, func_8006E480(s.p0, 0), 0);
    AddPrim(g_gpu_ot_ptr, *(s32 *)(arg0 + 0x1C));
    *(s32 *)(arg0 + 0x1C) += 0xC;
}
