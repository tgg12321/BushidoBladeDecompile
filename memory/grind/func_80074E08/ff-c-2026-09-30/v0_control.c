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
