/* REJECTED (s17b, 2026-09-02, structural) -- costs two instructions.
 *
 * `for (j = 0; j != 4; j++)` instead of `j < 4` on the WD chassis:
 * 10 / 78 at build_insns 80 (control W0 = 3 / 78 at 78).  GCC 2.7.2 loses the
 * `slti` exit test and emits a compare-and-branch pair, and the extra pressure
 * perturbs the inner-loop head as well.  The target's exit test is
 * `slti $v0,$a0,0x4` + `bnez`, i.e. the `<` spelling.  Banked so the bound
 * spelling is not re-swept.
 */
s32 func_800645B0(void) {
    s32 i;
    s32 j;
    s32 idx;
    s32 idx2;
    s32 mask;
    s32 val;
    s32 last;
    s32 wid;
    D_800F10EC = 1;
    for (i = 0; i < 0xF; i += 4) {
        for (j = 0; j != 4; j++) {
            idx = i + j;
            val = 1;
            mask = val << idx;
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
