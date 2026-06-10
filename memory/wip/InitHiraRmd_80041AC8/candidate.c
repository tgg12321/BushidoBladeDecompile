/* candidate for InitHiraRmd_80041AC8 (src/text1a.c) — floor 2 (HEAD floor 6)
 *
 * Lever (cheat-reviewer PASS 2026-06-10): named loop-invariant locals
 * `w`/`h` (rect width 16 / height 1) declared BEFORE the destination-pointer
 * init. With the constants inline in the loop body, loop.c hoists them as
 * movables placed AFTER the explicit `var_s1 = &D_800A9A24` statement
 * (preheader order la,li,li); target has li,li,la. Named locals make the
 * materialization follow source order. Retires regfix rule
 * `reorder 44,45,43 @ 43-45`.
 *
 * REMAINING GAP (the floor-2 residual, 3 index positions): cc1 sched1
 * hoists the post-store reload `lh v0,8(a0)` above `sh a1,D_800A9A20`;
 * target has the store first. See notes.md for the full MEM_IN_STRUCT_P
 * mechanism + rejected/ for the two sandbox-0 forms that FAILED review.
 * The remaining regfix rule `reorder 22,21 @ 21-22` covers this cluster.
 *
 * NOTE: this candidate CANNOT be committed alone — the existing preheader
 * rule `reorder 44,45,43` is index-calibrated against HEAD's emission
 * order; applying this body with that rule still present breaks the oracle.
 * Either close the lh/sh cluster too (retire both rules) or rewrite the
 * remaining rule for the new ordering (not done; partial-cheat commits are
 * not a sanctioned end state anyway).
 */
void InitHiraRmd_80041AC8(s16 *arg0)
{
  s16 rect[4];
  s16 *var_s0;
  u16 *var_s1;
  s32 var_s2;
  s32 var_s3;
  u16 v1_val;
  if (arg0[2] != 1)
  {
    return;
  }
  if (D_80094E08[arg0[4]] == 0xFF)
  {
    return;
  }
  D_800A9A20 = arg0[4];
  var_s0 = (s16 *) D_80094DF0[D_80094E08[arg0[4]]];
  if (single_game_SetStageId() != 1)
  {
    goto else_lbl;
  }
  var_s3 = -0x140;
  var_s2 = 0xF0;
  goto after_if;
  else_lbl:
  var_s3 = 0x80;

  var_s2 = 0;
  after_if:
  v1_val = (u16) (*var_s0);

  if ((*var_s0) >= 0)
  {
    s32 w = 0x10;
    s32 h = 1;
    var_s1 = &D_800A9A24;
    do
    {
      u16 v0_val;
      rect[0] = v1_val + var_s3;
      v0_val = (u16) var_s0[1];
      rect[2] = w;
      rect[3] = h;
      rect[1] = v0_val + var_s2;
      gpu_StoreImage(rect, var_s1);
      var_s0 += 2;
      var_s1 += 0x10;
      v1_val = (u16) (*var_s0);
    }
    while ((*var_s0) >= 0);
  }
  gpu_DrawSync(0);
}
