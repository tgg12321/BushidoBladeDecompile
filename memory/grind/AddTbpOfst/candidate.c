/* AddTbpOfst (src/main.c) — best form after session 1 (recon).
 * Honest floor: 3 (sandbox --disable all, 49/49 insns).
 * Remaining diffs vs target:
 *   1. andi $v0,$7,0xffff  vs target andi $v0,$a0,0xffff (zero_extend reads
 *      HI var pseudo 72/$a3-copy instead of SI incoming pseudo 73/$a0).
 *   2+3. sll $2,$3,4 emitted at slot 36 (after both sb's) vs target slot 32
 *      (before them). Registers of sll/addu now MATCH target ($v0) — only
 *      linear placement differs (blocked by the do{}while(0) fence region).
 * This form supersedes the baseline in-place `sa1 = sa1 << 4;` (which had
 * correct placement but wrong regs: 3 subst rules).
 */
s32 AddTbpOfst(u16 a0, s16 a1) {
    s32 idx;
    s32 sa1;
    s32 v0;
    int v1;
    s32 v2;
    s32 ofs;
    s32 entry;
    if ((a0 & 0xFFFF) >= 0x10) goto fail;
    idx = (s16)a0;
    if (D_80102A68[idx] != 1) return -1;
    sa1 = (s16)a1;
    if (sa1 < D_800FF634) goto ok;
fail:
    return -1;
ok:
    v0 = D_800F66B8[idx];

    v1 = D_800F6660[idx];
    v2 = D_800F6700[idx];
    D_801027F1 = (u8) a0;
    do { } while (0);
    D_801027F6 = (u8) a1;
    entry = *((s32 *) (((sa1 << 4) + v1) + 8));
    D_80101BC4 = v0;
    D_800FF6A0 = v1;
    D_80101BC8 = (VagAtr *)v2;
    D_801027F7 = (u8)entry;
    return 0;
}
