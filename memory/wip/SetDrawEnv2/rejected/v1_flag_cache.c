/* REJECTED: cache r->flag into local at function entry
 *
 * Hypothesis: introduce an early SEMANTIC reference to r (caching r->flag
 * into a local) before any function call, on the theory that it might
 * shift cc1's `move s0, a1` LUID earlier and thus flip the prologue
 * save+def pair emit order.
 *
 * Measured: sandbox --disable all => 52  (HEAD baseline = 4)
 * Build insns: 165 vs target 164  (+1 extra `lbu r->flag` early)
 *
 * Disproof: cc1's expand_function_start emits the `move sN, aN` insns at
 * function entry, BEFORE body code RTL is generated. Adding a body-level
 * early use of r does NOT shift the LUID of those move insns; it only
 * adds an additional `lbu` for r->flag that target does not have (target
 * loads r->flag lazily right before the if-test). Body codegen
 * sensitivity therefore RULES OUT cache-early-then-use-later structural
 * variants as a lever for this wall: any change that adds an early r
 * reference adds an instruction the target lacks.
 *
 * Conclusion: this confirms the wall is genuinely at the prologue surface
 * (LUID assignment in expand_function_start), not at any body-level
 * structural choice. The only known levers (param-local-alias,
 * prologue_fix per-function migration) are forbidden per
 * .claude/rules/no-new-park-categories.md (rulings 2026-06-02 and
 * 2026-06-15). No new lever found.
 */
void func_8007C4B8(s32 *out, Rect *r)
{
  s32 flag = r->flag;
  u16 buf[4];
  s16 var_v0;
  s16 var_v0_2;
  s16 new_var;
  s32 var_a3;
  out[1] = func_8007C7A0(r->x, r->y);
  /* ... rest unchanged from HEAD ... */
  if (flag != 0) {
    /* ... block uses cached flag ... */
  }
  *(((s8 *) out) + 3) = (s8) (var_a3 - 1);
}
