/* KILLED s2 (2026-07-19) — structural probe C: init-order permutation.
 * Initialize var_a3 at its declaration (`s32 var_a3 = 7;`) instead of the
 * later body-level `var_a3 = 7;` assignment. Measured floor WORSENED to
 * score=35, build_insns=166 (+2 vs target 164). Init-at-decl hoists the
 * `li $tN, 7` to prologue where target does it lazily just before the
 * `if (r->flag)` region, adding 2 spurious insns.
 *
 * Evidence for the H1 FAKE-alias dossier: structural exhaustion axis
 * "init-order permutation" (specifically for var_a3) is measured DEAD —
 * adds instructions target lacks AND does not shift prologue pair order. */
void func_8007C4B8(s32 *out, Rect *r)
{
  u16 buf[4];
  s16 var_v0;
  s16 var_v0_2;
  s16 new_var;
  s32 var_a3 = 7;
  out[1] = func_8007C7A0(r->x, r->y);
  /* ... rest identical to candidate.c, minus the `var_a3 = 7;` at line 449 ... */
}
