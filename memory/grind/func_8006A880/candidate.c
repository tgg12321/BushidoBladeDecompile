/* The 0x2C-byte sprite draw descriptor func_8007352C (SPRT walker, EnvA) and
   func_80073728 (POLY_FT4 walker, EnvF) consume; same layout as EnvF. */
typedef struct {
    s32 header;
    s32 table;
    s32 sprt_out;
    s32 ft4_out;
    s32 semi;
    s32 ot_idx;
    s32 x;
    s32 y;
    s32 scale_x;
    s32 scale_y;
    u8 has_color;
    u8 col_r, col_g, col_b;
} S_6A880;

typedef struct {
    s16 x, y, w, h;
} Rect6A880;

extern s32 D_8009BC04;
extern s32 D_8009BC08;
extern s32 SetDrawOffset();

/* Option-menu HUD painter (MOD.BIN resource at ctx+4). arg0 = the draw
 * context func_8006E390 fills (+0x8 POLY_FT4 cursor, +0x14 SPRT cursor,
 * +0x18 TILE cursor, +0x1C DR_MODE cursor, +0x20 DR_AREA cursor,
 * +0x24 DR_OFFSET cursor), accessed as byte offsets like func_8006A564;
 * arg1 = the display environment's clip RECT and ofs[2]; arg2 (passed by
 * func_800693CC) is unused. */
void func_8006A880(u8 *arg0, u16 *arg1, s32 arg2) {
    S_6A880 s;
    Rect6A880 rect;
    s16 ofs[2];
    /* Ruling 11 (ordinary-c-judge-decidable.md): holds five values, each a
       sprite-sheet table of the MOD.BIN root: +0x18 (option rows; first sheet,
       the row loop and row 7), +0x40 (counter frames), +0x24 twice (icon
       frames at [8 + frame], then FT4 frames at [frame]). Necessity proof
       (allocator dumps): memory/grind/func_8006A880/ruling11.md. */
    s32 *sheets;
    /* Ruling 11: holds two values, each a mask with one bit per option row:
       D_8009BC08 (scanned for the first row drawn) and D_8009BC04 (rows
       switched on). Necessity proof: memory/grind/func_8006A880/ruling11.md. */
    u32 row_mask;
    /* Ruling 9: the sheet's cell array. Every sheet drawn here is one 12-byte
       header followed by its 8-byte cells, so the cells always start at
       +0xC (MOD.BIN census: memory/grind/func_8006A880/evidence.md). */
    s32 cells;
    s32 yofs;
    s32 bit;
    s32 i;

    sheets = *(s32 **)(*(s32 *)(arg0 + 4) + 0x18);
    row_mask = D_8009BC08;
    bit = 0;
    s.header = sheets[0];
    SetDrawMode(*(s32 *)(arg0 + 0x1C), 1, 0, func_8006E480(s.header, 0), 0);
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
    for (; !(row_mask & (1 << bit)); bit++) {
        yofs -= 0x18;
    }
    ofs[0] = arg1[4];
    ofs[1] = arg1[5] + yofs;
    SetDrawOffset(*(s32 *)(arg0 + 0x24), ofs);
    AddPrim(g_gpu_ot_ptr + 0x30, *(s32 *)(arg0 + 0x24));
    *(s32 *)(arg0 + 0x24) += 0xC;

    row_mask = D_8009BC04;
    s.ot_idx = 0xB;
    for (i = 0; i < 7; i++) {
        s32 y = i * 0x18 + 0x3F;

        s.x = 0x76;
        s.has_color = 1;
        if ((D_800A34F8 & 0xF) == i) {
            s.semi = 0;
            s.col_r = ((rsin(((D_800A3514 & 0x1F) << 7) + 0x1FF) * 63) >> 12) - 0x61;
            s.col_g = ((rsin(((D_800A3514 & 0x1F) << 7) + 0x1FF) * 45) >> 12) - 0x7D;
            s.col_b = 0x30;
            s.y = *(s16 *)(D_800A34FC + 0xE) + y;
            D_800A3514++;
        } else if ((row_mask >> i) & 1) {
            s.col_r = 0x80;
            s.col_g = 0x6C;
            s.col_b = 0x30;
            s.semi = 1;
            s.y = y;
            if (((s32 *)D_800A3524)[8] & 1) {
                s.has_color = 0;
            }
        } else {
            s.col_r = 0x30;
            s.col_g = 0x30;
            s.col_b = 0x30;
            s.semi = 1;
            s.y = y;
        }
        s.header = sheets[i];
        cells = s.header + 0xC;
        s.table = cells;
        s.sprt_out = *(s32 *)(arg0 + 0x14);
        *(s32 *)(arg0 + 0x14) = func_8007352C((s32)&s);
        SetDrawMode(*(s32 *)(arg0 + 0x1C), 1, 0, func_8006E480(s.header, 0), 0);
        AddPrim(g_gpu_ot_ptr + s.ot_idx * 4, *(s32 *)(arg0 + 0x1C));
        *(s32 *)(arg0 + 0x1C) += 0xC;
        s.has_color = 1;
        func_8006A564(arg0, (u8 *)&s, i);
    }

    sheets = *(s32 **)(*(s32 *)(arg0 + 4) + 0x40);
    s.ot_idx = 0xA;
    s.has_color = 0;
    s.semi = 0;
    s.x = 0;
    s.y = 0x41;
    s.header = sheets[D_800A34F8 & 0xF];
    cells = s.header + 0xC;
    s.table = cells;
    s.sprt_out = *(s32 *)(arg0 + 0x14);
    *(s32 *)(arg0 + 0x14) = func_8007352C((s32)&s);
    SetDrawMode(*(s32 *)(arg0 + 0x1C), 1, 0, func_8006E480(s.header, 0), 0);
    AddPrim(g_gpu_ot_ptr + s.ot_idx * 4, *(s32 *)(arg0 + 0x1C));
    *(s32 *)(arg0 + 0x1C) += 0xC;

    sheets = *(s32 **)(*(s32 *)(arg0 + 4) + 0x18);
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
    s.has_color = 1;
    if ((D_800A34F8 & 0xF) == 7) {
        s.semi = 0;
        s.col_r = ((rsin(((D_800A3514 & 0x1F) << 7) + 0x1FF) * 63) >> 12) - 0x61;
        s.col_g = ((rsin(((D_800A3514 & 0x1F) << 7) + 0x1FF) * 45) >> 12) - 0x7D;
        s.col_b = 0x30;
        s.y = *(s16 *)(D_800A34FC + 0xE) + 0xCF;
        D_800A3514++;
    } else if ((row_mask >> 7) & 1) {
        s.col_r = 0x80;
        s.col_g = 0x6C;
        s.col_b = 0x30;
        s.semi = 1;
        s.y = 0xCF;
        if (((s32 *)D_800A3524)[8] & 1) {
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
    s.header = sheets[7];
    cells = s.header + 0xC;
    s.table = cells;
    s.sprt_out = *(s32 *)(arg0 + 0x14);
    *(s32 *)(arg0 + 0x14) = func_8007352C((s32)&s);
    SetDrawMode(*(s32 *)(arg0 + 0x1C), 1, 0, func_8006E480(s.header, 0), 0);
    AddPrim(g_gpu_ot_ptr + s.ot_idx * 4, *(s32 *)(arg0 + 0x1C));
    *(s32 *)(arg0 + 0x1C) += 0xC;
    s.has_color = 1;
    func_8006A564(arg0, (u8 *)&s, 7);
    func_8006A494((s32 *)arg0, (u8 *)&s);

    s.has_color = 0;
    SetDrawMode(*(s32 *)(arg0 + 0x1C), 1, 0, func_8006E480(s.header, 0), 0);
    AddPrim(g_gpu_ot_ptr + 4, *(s32 *)(arg0 + 0x1C));
    *(s32 *)(arg0 + 0x1C) += 0xC;
    SetDrawMode(*(s32 *)(arg0 + 0x1C), 1, 0, func_8006E480(s.header, 0), 0);
    AddPrim(g_gpu_ot_ptr + 4, *(s32 *)(arg0 + 0x1C));
    *(s32 *)(arg0 + 0x1C) += 0xC;

    sheets = *(s32 **)(*(s32 *)(arg0 + 4) + 0x24);
    s.header = sheets[(D_800A34F8 & 0xF) + 8];
    s.x = 0;
    s.y = 0x19;
    s.has_color = 0;
    s.semi = 0;
    s.ot_idx = 0;
    cells = s.header + 0xC;
    s.table = cells;
    s.sprt_out = *(s32 *)(arg0 + 0x14);
    *(s32 *)(arg0 + 0x14) = func_8007352C((s32)&s);
    SetDrawMode(*(s32 *)(arg0 + 0x1C), 1, 0, func_8006E480(s.header, 0), 0);
    AddPrim(g_gpu_ot_ptr, *(s32 *)(arg0 + 0x1C));
    *(s32 *)(arg0 + 0x1C) += 0xC;

    sheets = *(s32 **)(*(s32 *)(arg0 + 4) + 0x24);
    s.header = sheets[D_800A34F8 & 0xF];
    s.scale_x = 0x200;
    s.x = 0;
    s.y = 0;
    s.scale_y = 0x100;
    s.has_color = 0;
    s.semi = 0;
    s.ot_idx = 0;
    cells = s.header + 0xC;
    s.table = cells;
    s.ft4_out = *(s32 *)(arg0 + 8);
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

    SetDrawMode(*(s32 *)(arg0 + 0x1C), 1, 0, func_8006E480(s.header, 0), 0);
    AddPrim(g_gpu_ot_ptr, *(s32 *)(arg0 + 0x1C));
    *(s32 *)(arg0 + 0x1C) += 0xC;
}
