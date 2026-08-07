/* KILLED s3 — named intermediate rg for the r/g sub-tree.
 * Score stayed at 2. Emission still [r,g,b].
 * Mechanism dead: CSE/combine folded `rg` back into (sll r 16) | (sll g 8);
 * `rg` never materialized as a live pseudo across a scheduling boundary, so
 * sched1's dependence DAG for the FALSE block is unchanged (chains still
 * length 4 for r/g, length 2 for b). LUID/decl order of `rg` had no effect
 * because it was folded before sched1.
 */
} else {
    s32 rg;
    r = *((u8 *)player + 0x18);
    g = *((u8 *)player + 0x19);
    b = *((u8 *)player + 0x1A);
    rg = (r << 16) | (g << 8);
    gnd_load_tex(b | rg);
}
