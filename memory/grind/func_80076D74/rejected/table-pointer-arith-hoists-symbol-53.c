/* REJECTED (s1): score 53: (&D_8009BCF9)[idx*2] and every pointer-arithmetic spelling force `reg=symbol` at expand (expr.c force_operand), loop.c hoists it (la $t3 outside loop) instead of target's lui $at/addu $at/lbu %lo form; raw-mask bitfield ops on *(u32*)(hdr+0x14); hdr+i*10+j*2 store address. */
s32 func_80076D74(s32 *arg0) {
    u8 *p;
    u8 *hdr;
    u16 *cnt;
    s16 v;
    s16 i;
    s16 j;
    s32 sel;
    s32 ret;

    ret = 0;
    cnt = (u16 *)(D_800A36A0 + 0x36);
    v = *cnt + 8;
    *cnt = v;
    if (v >= 0xFF) {
        *cnt = 0xFF;
        hdr = *(u8 **)D_800A36A0;
        *(u32 *)(hdr + 0x14) = (*(u32 *)(hdr + 0x14) & ~0xC00) | ((*(u8 *)(D_800A36A0 + 0x65) & 3) << 10);
        ret = 1;
        if (*(u8 *)(D_800A36A0 + 0x66) < 3) {
            sel = *(u8 *)(D_800A36A0 + 0x66) - 1;
        } else {
            sel = *(u8 *)(D_800A36A0 + 0x66) - 2;
        }
        *(u32 *)(hdr + 0x14) = (*(u32 *)(hdr + 0x14) & ~0x3000) | ((sel & 3) << 12);
        *(u32 *)(hdr + 0x14) = (*(u32 *)(hdr + 0x14) & ~0x4000) | ((*(u8 *)(D_800A36A0 + 0x67) & 1) << 14);
        *(u32 *)(hdr + 0x14) = (*(u32 *)(hdr + 0x14) & ~0x18000) | (((*(u8 *)(D_800A36A0 + 0x68) + *(u8 *)(D_800A36A0 + 0x69) * 2) & 3) << 15);
        for (i = 0; i < 2; i++) {
            for (j = 0; j < *(u8 *)(D_800A36A0 + 0x65) + 3; j++) {
                *(u8 *)(hdr + i * 10 + j * 2) = (&D_8009BCF9)[*(s16 *)(D_800A36A0 + i * 10 + j * 2 + 0x6A) * 2];
                *(u8 *)(hdr + i * 10 + j * 2 + 1) = *(u16 *)(D_800A36A0 + i * 10 + j * 2 + 0x7E);
            }
        }
    }
    p = (u8 *)arg0[5];
    SetTile((GameObj *)p);
    *(u8 *)(p + 4) = *cnt;
    *(u8 *)(p + 5) = *cnt;
    *(u8 *)(p + 6) = *cnt;
    *(s16 *)(p + 8) = 0;
    *(s16 *)(p + 0xA) = 0;
    *(s16 *)(p + 0xC) = 0x280;
    *(s16 *)(p + 0xE) = 0xF0;
    SetSemiTrans((GameObj *)p, 1);
    AddPrim(D_800A374C, (GameObj *)p);
    p += 0x10;
    arg0[5] = (s32)p;
    SetDrawMode(arg0[6], 1, 0, 0x40, 0);
    AddPrim(D_800A374C, (GameObj *)arg0[6]);
    arg0[6] += 0xC;
    return ret;
}
