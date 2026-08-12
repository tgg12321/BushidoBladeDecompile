/* REJECTED — `mask = 1 << idx` (or any spelling where the constant 1 lives in
 * its own single-set pseudo) makes the const-1 a loop.c MOVABLE, so
 * scan_loop/move_movables hoist it out of the inner loop into a fresh
 * callee-save ($s4). That adds an sw/lw save-restore pair: build_insns 81 vs
 * target 78, honest sandbox distance 17 (vs 0 for the reuse form).
 *
 * Also rejected here: declaring a SEPARATE `s32 one = 1;` — identical RTL,
 * identical hoist. The distinguishing property is not the variable's name but
 * whether the pseudo is set more than once inside the loop.
 *
 * Do NOT re-propose this shape. The working form routes the constant through
 * the same variable that later holds the D_800A3444 read (see candidate.c).
 */
s32 func_800645B0(void) {
    s32 i;
    s32 j;
    s32 idx;
    s32 idx2;
    s32 mask;
    D_800F10EC = 1;
    i = 0;
    do {
        j = 0;
        do {
            idx = i + j;
            mask = 1 << idx;            /* <-- hoisted into $s4 by loop.c */
            j += 1;
            if (!(D_800A3444 & mask)) {
                idx2 = idx << 1;
                *((s32 *)(((s32)(&D_800F0D78)) + ((idx2 + idx) << 2))) = (((s32 *)D_800A347C)[0] + (rand() & 0xFF)) - 0x7F;
                *((s32 *)(((s32)(&D_800F0D7C)) + ((idx2 + idx) << 2))) = (((s32 *)D_800A347C)[1] + (rand() & 0xFF)) - 0x7F;
                *((s32 *)(((s32)(&videoDec)) + ((idx2 + idx) << 2))) = (((s32 *)D_800A347C)[2] + (rand() & 0xFF)) - 0x7F;
                *((s16 *)(((s32)(&D_800F0BCC)) + idx2)) = rand() & 7;
                D_800A3444 |= mask;     /* fused RMW: emits lw; nop; or; sw */
                break;
            }
        } while (j < 4);
        i += 4;
    } while (i < 0xF);
    return 1;
}
