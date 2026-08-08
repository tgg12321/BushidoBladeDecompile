/* REJECTED (session 2, probe P10): reusing the outer-scope v1 variable for the
 * second 0x58 load (v1_58 role), predicted to create a v1/arg1 conflict that
 * kills the inherited $5 preference (both target regs are $3, ranges disjoint).
 * Measured: 15 / 159 insns (vs 2/158 for the banked candidate) - WORSE, +1 insn.
 * Widening v1's pseudo across the join block perturbs the block-1 structure
 * (delay-slot fill lost, join-block reshuffle).  KILLED as spelled.  A future
 * session could retry a narrower spelling only if it keeps v1's block-1 web
 * byte-identical first.  Shape (deltas only): declare `s32 v1;` at function
 * scope; `v1 = *(u16*)(arg1+4);` in block 1 unchanged; in the join block
 * replace `s32 v1_58 = *(s32*)(s0+0x58);` with `v1 = *(s32*)(s0+0x58);` and
 * deref `*(u8*)(v1+2)`. */
