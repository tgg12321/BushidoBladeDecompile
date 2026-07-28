/* REJECTED s2: adding explicit `var_v1 = a0 & 0xFFFF1FFF;` in each else arm and
 * using `var_v1 | ((var_v0 & 7) << 13)` in the shared complete_store REGRESSES
 * from 10 -> 15. Reason: with var_v1 lifted out, GCC allocates the final OR
 * result to $v0 in all four paths (matching var_v1 (v1) | shift (v0) -> v0),
 * so the terminating `sw $v0,gp` becomes rtx_equal across all 4 store sites;
 * jump2 find_cross_jump merges them ALL back to a single shared sw. In H1d
 * (this-rejected's parent) the shared block's OR result lands in $v1 while the
 * two then-arms' stores use $v0, so the sw source register differs (v0 vs v1)
 * and the merge is blocked. Coupling: forcing per-arm mask compute reintroduces
 * the store-tail merge trilemma discussed in [[cross-jump-store-tail-deep-dive]]. */
