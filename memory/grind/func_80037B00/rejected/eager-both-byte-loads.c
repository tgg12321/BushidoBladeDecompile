/* REJECTED s2 2026-07-28 — sandbox score 17 (baseline 15), +2 weighted;
 * build_insns=32 (target 36, LOST 2 vs baseline 34).
 * Structural: read both bytes (var_v1=(u8)*var_a2; var_v0=(u8)*var_a1;) BEFORE
 * the two if-tests, instead of interleaving the load with the zero-terminator
 * check. Intended to force parallel loads on separate registers.
 * Effect: GCC folded/scheduled the loads together and DROPPED two insns
 * (target has them). Structurally worse — moves further from target insn count.
 * KILLED as a load-order lever. */
s32 func_80037B00(u8 *arg0) {
    /* ... same decls ... */
loop_inner:
    var_v1 = (u8)*var_a2;
    var_v0 = (u8)*var_a1;
    if (var_v1 == 0) goto block_5c;
    if (var_v1 != var_v0) goto block_6c;
    /* ... */
}
