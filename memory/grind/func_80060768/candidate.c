/* func_80060768 — sandbox distance 0, s2 (2026-07-17), pure C, NO pointer aliases.
 * This exact text is live in src/text1b.c:13222-13303.
 * Key mechanism: the increment value is staged through t1/t2 (live both times —
 * store feed, then product), so CSE's mem(D_800A32B4/B6) == reg equivalence is
 * invalidated when the var is reassigned, forcing the natural lh reload at the
 * clamp compare. Direct-read WITHOUT the reuse folds to sll/sra (score 16). */
s32 func_80060768(s32 arg0, s32 arg1, s32 arg2) {
    s32 sp18;
    s32 sp1C;
    s32 t1;
    s32 t2;
    u16 cur1;
    u16 cur2;
    s32 end_off;
    s32 tile_off;

    tile_off = arg0 + 0x7D0;
    end_off = arg0 + 0xAC8;
    sp18 = arg0;
    sp1C = arg0 + 0x870;
    func_8006D808(&sp18, &sp1C, &D_8009B0C0, arg1, arg2);
    if ((u32)arg2 < 3U) {
        initTile((void *)tile_off);
        *(u8 *)(arg0 + 0x7D4) = 0xFF;
        *(s16 *)(arg0 + 0x7D8) = 0x6A;
        *(u8 *)(arg0 + 0x7D5) = 0;
        *(u8 *)(arg0 + 0x7D6) = 0;
        *(s16 *)(arg0 + 0x7DA) = (s16)(arg2 * 0x1A + 0x5B);
        cur1 = D_800A32B4;
        t1 = cur1 + 1;
        D_800A32B4 = t1;
        t1 = (s32)((s16)cur1) * 0x1AA;
        *(s16 *)(arg0 + 0x7DE) = 2;
        *(s16 *)(arg0 + 0x7DC) = (s16)(t1 / 0x1E);
        if ((s16)D_800A32B4 >= 0x1F) {
            D_800A32B4 = 0x1E;
        }
        gpu_SetSemiTransp((void *)tile_off, 0);
        ot_Link(D_800A374C + arg1 * 4, (void *)tile_off);
        tile_off = arg0 + 0x7E0;
    }
    initTile((void *)tile_off);
    *(u8 *)(tile_off + 4) = 0xFF;
    *(s16 *)(tile_off + 8) = 0x9E;
    *(u8 *)(tile_off + 5) = 0;
    *(u8 *)(tile_off + 6) = 0;
    *(s16 *)(tile_off + 0xA) = 0xBD;
    cur2 = D_800A32B6;
    t2 = cur2 + 1;
    D_800A32B6 = t2;
    t2 = (s32)((s16)cur2) * 0x144;
    *(s16 *)(tile_off + 0xE) = 2;
    *(s16 *)(tile_off + 0xC) = (s16)(t2 / 0x1E);
    if ((s16)D_800A32B6 >= 0x1F) {
        D_800A32B6 = 0x1E;
    }
    gpu_SetSemiTransp((void *)tile_off, 0);
    ot_Link(D_800A374C + arg1 * 4, (void *)tile_off);
    tile_off += 0x10;

    initTile((void *)tile_off);
    *(s16 *)(tile_off + 8) = 0x3F;
    *(s16 *)(tile_off + 0xA) = 0x2D;
    *(s16 *)(tile_off + 0xC) = 0x202;
    *(u8 *)(tile_off + 4) = 0;
    *(u8 *)(tile_off + 5) = 0;
    *(u8 *)(tile_off + 6) = 0;
    *(s16 *)(tile_off + 0xE) = 0x6C;
    gpu_SetSemiTransp((void *)tile_off, 1);
    ot_Link(D_800A374C + arg1 * 4, (void *)tile_off);
    tile_off += 0x10;

    initTile((void *)tile_off);
    *(s16 *)(tile_off + 8) = 0x92;
    *(s16 *)(tile_off + 0xA) = 0xAA;
    *(s16 *)(tile_off + 0xC) = 0x15C;
    *(u8 *)(tile_off + 4) = 0;
    *(u8 *)(tile_off + 5) = 0;
    *(u8 *)(tile_off + 6) = 0;
    *(s16 *)(tile_off + 0xE) = 0x1A;
    gpu_SetSemiTransp((void *)tile_off, 1);
    ot_Link(D_800A374C + arg1 * 4, (void *)tile_off);

    initTexPage((void *)sp1C, 1, 0, 0, 0);
    ot_Link(D_800A374C + arg1 * 4, (void *)sp1C);
    sp1C += 0xC;
    return end_off - arg0;
}
