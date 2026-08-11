/* REJECTED — score 13 (vs 9 baseline at time of probe, best-known 2).
 * Folding the adj into the subtraction expression makes expand evaluate
 * tbl[idx] and the constant chain in the wrong order and loses the $a0
 * allocation of 0xFFFECC00.
 */
    {
        s32 remaining = (s32)tbl[idx] - (s32)(D_800A38B4 + 0xFFFECC00u);
        ...
    }
