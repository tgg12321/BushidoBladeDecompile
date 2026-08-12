/* REJECTED — func_800645B0, grind session 3 (2026-08-12).
 *
 * WHAT IT PROVES (positive result, kept for the mechanism): in the CA shape
 * (slot index recomputed at the loop top, *3 sum in its own `wid`) a second
 * set of `idx` whose VALUE IS GENUINELY NEW does remove sched.c's
 * birthing_insn_p bonus and fixes the loop-top emission order.  CA is 3/78 and
 * its three unmasked diffs are ALL the loop-top order; staging any fresh value
 * through `idx` removes all three:
 *   CA  control                                                     3 / 78
 *   MA  `idx = last & 7;` staged before the s16 store               2 / 78
 *   MB  same, staged before the D_800A3444 read                     2 / 78
 *   MD  `idx = val | mask; D_800A3444 = idx;`                       2 / 78
 *   MC  `idx = rand();` (value re-derived at the use site)          3 / 78
 *
 * WHY IT IS DEAD: the residual simply MOVES to the staged value's register.
 *   MA:  target `andi v0,v0,0x7` / `sh v0,0(at)`
 *        build  `andi s0,v0,0x7` / `sh s0,0(at)`
 *   MD:  target `or v1,v1,s2`    / `sw v1,0(gp)`
 *        build  `or s0,v1,s2`    / `sw s0,0(gp)`
 * `idx` is a multi-block pseudo allocated $s0, and GCC 2.7.2 has no live-range
 * splitting, so every value routed through `idx` is emitted in $s0.  The
 * target keeps EXACTLY three values in $s0 — `i + j`, the *3 sum, and the
 * 12-byte byte offset — so a legal second set of `idx` must be one of those:
 *   - the *3 sum      -> re-introduces optabs.c's `target == op1` commutative
 *                        swap (this is the shipped 1-point form),
 *   - the byte offset -> variant DA, 12/78: `idx` then carries the offset
 *                        across the four calls, overlaps `idx2`, and since
 *                        local_alloc runs before global_alloc the block-local
 *                        `idx2` claims $s0 first and `idx` is pushed to $s1,
 *   - `i + j` again   -> the maintained-index loop form (see
 *                        maintained-index-entry-copy-const-folds.c).
 * MC also shows the fold rule again: `idx = rand();` with the mask re-derived
 * at the use site leaves the bonus in place (3/78) because the staged value is
 * still available in the call-return pseudo.
 *
 * Sweep: tmp/grind/func_800645B0/s3/sweep15.py, diffs via diffvar15.py.
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
    i = 0;
    do {
        j = 0;
        do {
            idx = i + j;
            val = 1;
            mask = val << idx;
            j += 1;
            if (!(D_800A3444 & mask)) {
                idx2 = idx << 1;
                last = rand();
                wid = idx2 + idx;
                /* ... three s32 stores at (wid << 2) ... */
                last = rand();
                val = D_800A3444;
                idx = last & 7;   /* real second set -> loop top fixed,
                                     but emitted in $s0, target uses $v0 */
                *((s16 *)(((s32)(&D_800F0BCC)) + idx2)) = idx;
                val = val | mask;
                D_800A3444 = val;
                break;
            }
        } while (j < 4);
        i += 4;
    } while (i < 0xF);
    return 1;
}
