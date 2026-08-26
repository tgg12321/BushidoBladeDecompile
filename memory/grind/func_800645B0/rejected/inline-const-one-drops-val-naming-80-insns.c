/* REJECTED (session 12, 2026-08-25) — dropping the `val = 1;` naming.
 * `mask = 1 << idx;` in place of `val = 1; mask = val << idx;` measures
 * 12 / 78 at 80 build insns (two insns MORE than target).  The named `val`
 * local is load-bearing for the whole chassis, not cosmetic: giving the
 * constant 1 its own name is what keeps the shift's operand in a pseudo that
 * survives to the target's `sllv $s2,$v1,$s0`.  Do not re-propose inlining it.
 * (Applied on top of the WD chassis; the regression is far larger than WD's
 * own 3, so the interaction is not the cause.)
 */
s32 func_800645B0(void) {
    s32 i;
    s32 j;
    s32 idx;
    s32 idx2;
    s32 wid;
    s32 mask;
    s32 val;
    s32 last;
    D_800F10EC = 1;
    for (i = 0; i < 0xF; i += 4) {
        for (j = 0; j < 4; j++) {
            idx = i + j;
            mask = 1 << idx;
            if (!(D_800A3444 & mask)) {
                idx2 = idx << 1;
                last = rand();
                wid = idx2 + idx;
                *((s32 *)(((s32)(&D_800F0D78)) + (wid << 2))) = (((s32 *)D_800A347C)[0] + (last & 0xFF)) - 0x7F;
                *((s32 *)(((s32)(&D_800F0D7C)) + (wid << 2))) = (((s32 *)D_800A347C)[1] + (rand() & 0xFF)) - 0x7F;
                *((s32 *)(((s32)(&videoDec)) + (wid << 2))) = (((s32 *)D_800A347C)[2] + (rand() & 0xFF)) - 0x7F;
                last = rand();
                val = D_800A3444;
                *((s16 *)(((s32)(&D_800F0BCC)) + idx2)) = last & 7;
                val = val | mask;
                D_800A3444 = val;
                break;
            }
        }
    }
    return 1;
}
