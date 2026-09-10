/* REJECTED (s1): first form. Sandbox 46. Ternary OT index inside the AddPrim argument
 * (D_800A374C + (arg1 ? 0x15 : 0xB) * 4) makes jump/cse fold the add into both arms
 * (two lui/lw/addiu blocks); target selects the INDEX in a register (li 0xB / li 0x15)
 * then sll+addu once. Fix: 'ot = 0xB; if (arg1) ot = 0x15;' — the same idiom as n. */
void func_80074B18(s32 *arg0, s32 arg1, s32 arg2) {
    u8 *p; u8 *t; s16 i; s16 j; s32 n;
    n = 5;
    if (arg2 != 0) { n = 8; }
    p = (u8 *)arg0[5];
    for (i = 0; i < *(u8 *)(D_800A36A0 + 0x65) + 3; i++) {
        t = *(u8 **)(*(u8 **)(D_800A36A0 + 4) + 0x3C);
        for (j = 0; j < n; j++) {
            SetTile((GameObj *)p);
            *(u8 *)(p + 4) = *(u8 *)(t + 8); *(u8 *)(p + 5) = *(u8 *)(t + 9); *(u8 *)(p + 6) = *(u8 *)(t + 0xA);
            *(u16 *)(p + 0xC) = *(u16 *)(t + 4); *(u16 *)(p + 0xE) = *(u16 *)(t + 6);
            SetSemiTrans((GameObj *)p, 0);
            if (arg2 != 0) { *(s16 *)(p + 8) = *(u16 *)(t + 0) + arg1 * 240; *(s16 *)(p + 0xA) = *(u16 *)(t + 2) + i * 34 + 0x2B; }
            else { *(s16 *)(p + 8) = *(u16 *)(t + 0) + arg1 * 240; *(s16 *)(p + 0xA) = *(u16 *)(t + 2) + i * 17 + 0x7C; }
            AddPrim(D_800A374C + (arg1 != 0 ? 0x15 : 0xB) * 4, (GameObj *)p);
            p += 0x10; t += 0xC;
        }
    }
    arg0[5] = (s32)p;
}
