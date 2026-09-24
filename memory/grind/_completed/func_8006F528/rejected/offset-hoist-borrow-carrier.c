/* REJECTED 2026-09-23 layer-2 r2: offset hoisted to fn scope to borrow = invented borrow (func_800460E4 lineage). 0/277. */
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
    u8 *offset;

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
        /* FAKE: base + 0xC staged through `offset` (real job: the switch's
         * D_800A35C4 draw-offset pointer; dead at every staging point, first
         * really assigned inside the switch), mechanism: global.c gives the one
         * multi-block pseudo a1 as in target; a fresh single-write local per
         * block measures 14/277. lever-exhaustion: memory/grind/func_8006F528/
         * hypotheses.md */
        offset = (u8 *)(base + 0xC);
        if (state >= 3) {
            s.zero18 = D_800A3570;
        } else {
            s.zero18 = 0;
        }
        s.p1 = (s32 *)offset;
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
        offset = (u8 *)(base + 0xC); /* FAKE: staged via offset, see first block */
        s.p1 = (s32 *)offset;
    }
    s.ret = arg0[1];
    arg0[1] = func_80073728((s32)&s, 0);

    {
        s32 base = ctx[0];

        s.p0 = (void *)base;
        offset = (u8 *)(base + 0xC); /* FAKE: staged via offset, see first block */
        if (state < 3) {
            s.zero18 = -D_800A3570 + 0x200;
        } else {
            s.zero18 = 0x200;
        }
        s.p1 = (s32 *)offset;
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
        offset = (u8 *)(base + 0xC); /* FAKE: staged via offset, see first block */
        s.p1 = (s32 *)offset;
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
    AddPrim(D_800A374C + 0x3C, arg0[7]);
    arg0[7] += 0xC;

    rect.x = ((u16 *)D_800A35C0)[0];
    rect.y = ((u16 *)D_800A35C0)[1];
    rect.w = ((u16 *)D_800A35C0)[2];
    rect.h = ((u16 *)D_800A35C0)[3];
    SetDrawArea(arg0[7], &rect);
    AddPrim(D_800A374C + 0x18, arg0[7]);
    arg0[7] += 0xC;

    switch (state) {
    case 2:
    case 4: {
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
        AddPrim(D_800A374C + 0x3C, arg0[8]);
        arg0[8] += 0xC;

        *(u16 *)(D_800A35C4 + 0x10) = *(u16 *)(D_800A35C0 + 8);
        *(u16 *)(D_800A35C4 + 0x12) = *(u16 *)(D_800A35C0 + 0xA);
        SetDrawOffset(arg0[8], D_800A35C4 + 0x10);
        AddPrim(D_800A374C + 0x18, arg0[8]);
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

        ot = D_800A374C + 0x38;
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
        offset = (u8 *)(base + 0xC); /* FAKE: staged via offset, see first block */
        s.p1 = (s32 *)offset;
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
