/* REJECTED (s7, 2026-08-26) — hoisting the temp_a0_2 abs ABOVE the temp_v1_5
 * product (declaring `s32 temp_v1_5;` and assigning it after the if) measures
 * 44 / 214 insns: one instruction FEWER than target's 215 and a 16-point
 * regression.  Tried as the "shrink qty0's span" lever for the tail a0/a1
 * local-alloc cluster (solver vector: span(qty0) 30 -> <=24).  Sinking the abs
 * BELOW was already dead (s3, tail-abs-sunk-below-shift-adds-insn.c, 31/216);
 * this closes the other direction, so the abs is pinned where it is.
 *
 *     s32 temp_a0_2 = *(s32 *)(temp_a0 + 0x118);
 *     s32 var_a1 = temp_a0_2;
 *     s32 temp_v1_5;
 *     s32 var_v0_3;
 *     if (temp_a0_2 < 0) { var_a1 = -temp_a0_2; }
 *     temp_v1_5 = (s32)((&Judge)[...] * ... + (&Judge)[...] * ...) >> 0xC;
 *     var_v0_3 = temp_v1_5;
 *
 * The C_both variant (this PLUS the B_ptr_late reorder) measures the same
 * 44 / 214, so the loss is owned by the abs hoist alone.
 */
