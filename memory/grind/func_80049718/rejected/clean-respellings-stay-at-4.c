/* REJECTED (s2) — func_80049718 Cluster A prologue `p_anim = &D_800EF980[arg0]`.
 * These CLEAN structural respellings (no base-holding pseudo) ALL measured
 * floor 4 (no change) or WORSE. They do NOT flip the addu operand order,
 * because GCC's `fold` canonicalizes the address PLUS with the symbol_ref
 * (constant) as operand-2, so the index ashift is expanded/born first and
 * local-alloc coalesces the base directly into $s0 (built in place), while
 * target keeps the base in its own temp $v0 (base born first).
 *
 *   p_anim = (s16 *)((s32)D_800EF980 + arg0 * 2);              -> 4  (fold-identical)
 *   { s32 idx = arg0 * 2; p_anim = (s16 *)((s32)D_800EF980 + idx); } -> 4  (index precompute; index still first)
 *   p_anim = D_800EF980; p_anim += arg0;                       -> 9  (WORSE; cse2 re-folds, reusing the real var collapses the split)
 *   p_anim = D_800EF980 + arg0;   (s1)                         -> 11 (WORSE; see rejected/pointer-add-no-local.c)
 *   (s16*)((s32)&D_800EF980[0] + arg0*2)  (s1)                 -> no change
 *
 * The ONLY forms that reach floor 0 give the base its OWN named pseudo
 * (a separate variable, so cse keeps it live and local-alloc births it first):
 *   { s16 *tbl = D_800EF980; p_anim = tbl + arg0; }            -> 0  (pointer spelling)
 *   { s32 base = (s32)D_800EF980; p_anim = (s16*)(base + arg0*2); } -> 0 (integer spelling)
 * Both are the SAME construct = the base-pointer local (pointer-alias family /
 * tslPolyF4Init "ordinary pointer arithmetic" lever). See candidate_gated_alias.c
 * and the s2 ruling-request. Clean pure-C (no base-holding pseudo) is EXHAUSTED
 * at floor 4. */
