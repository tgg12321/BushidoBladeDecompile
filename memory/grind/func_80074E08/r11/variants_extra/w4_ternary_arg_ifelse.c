#include "gpu.h"
typedef struct EnvB {
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
} EnvB;
void func_80074E08(s32 *arg0, s32 arg1) {
    EnvB s;
    u16 rect[4];
    u16 offset[2];
    s32 *records;
    s32 prim;
    s32 ot_idx;
    s32 rect_x;
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
        AddPrim(g_gpu_ot_ptr + 0xE * 4 + 0x24, prim);
    } else {
        AddPrim(g_gpu_ot_ptr + 4 * 4 + 0x24, prim);
    }
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
    AddPrim(g_gpu_ot_ptr + ot_idx * 4, arg0[7]);
    arg0[7] += 0xC;

    offset[0] = ((DRAWENV *)SELWORK->f24)->ofs[0];
    offset[1] = ((DRAWENV *)SELWORK->f24)->ofs[1]
              - SELWORK->f08[arg1];
    SetDrawOffset(arg0[8], offset);
    AddPrim(g_gpu_ot_ptr + ot_idx * 4 + 0x24, arg0[8]);
    arg0[8] += 0xC;

    offset[0] = ((DRAWENV *)SELWORK->f24)->ofs[0];
    offset[1] = ((DRAWENV *)SELWORK->f24)->ofs[1];
    SetDrawOffset(arg0[8], offset);
    AddPrim(g_gpu_ot_ptr + ot_idx * 4, arg0[8]);
    arg0[8] += 0xC;
}
