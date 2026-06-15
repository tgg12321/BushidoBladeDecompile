/* func_800550E8 (text1b.c) — byte-array half-decay loop. HEAD body.
 * Matches ONLY with 3 regfix substs that re-encode the addressing mode:
 * target keeps `la(D_80101EC8)` (no addend) + `lbu/sb 1045(reg)`; cc1's combine
 * pass folds &D_80101EC8 + 0x415 into the la symbol+addend, emitting
 * `la(D_80101EC8+1045)` + `lbu/sb 0(reg)`. Same runtime bytes, different operand
 * encoding. SINGLE displacement => cc1's combine has no degree of freedom.
 * See notes.md. */
void func_800550E8(s32 arg0) {
    s32 i;
    u8 *p = (u8 *)&D_80101EC8 + arg0 * 0x44C;
    i = 0;
    do {
        i += 1;
        *(p + 0x415) = *(p + 0x415) >> 1;
        p += 2;
    } while (i < 8);
}
