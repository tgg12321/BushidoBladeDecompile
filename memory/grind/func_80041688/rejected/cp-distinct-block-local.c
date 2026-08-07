/* KILLED s3 — distinct block-local pointer cp = (u8*)player.
 * Score stayed 2. Emission still [r,g,b] at base s0.
 * Mechanism dead: cp is a pure cast; GCC's local CSE/coalescing merged
 * cp's pseudo with player's before sched1 saw any alias-set difference.
 * All three lbu's still hit base=s0 with the SAME (plus base K) MEM,
 * identical dependence DAG. Statement-order b-first was ignored by
 * sched1 (chain-length priority still dominates).
 */
} else {
    u8 *cp = (u8 *)player;
    b = cp[0x1A];
    r = cp[0x18];
    g = cp[0x19];
    gnd_load_tex(b | ((r << 16) | (g << 8)));
}
