/* KILLED s2 (2026-07-19) — structural probe A: K&R decl-block reversal.
 * Local decls listed in reversed order (s32/s16/s16/s16/u16 vs baseline
 * u16/s16/s16/s16/s32). Measured floor unchanged: score=4, 164/164 insns,
 * identical build.o. Confirms the mechanism note: `move sN, aN` LUIDs are
 * assigned during expand_function_start walking the PARAM decl list, not
 * the LOCAL decl block — so reordering locals cannot shift them.
 *
 * Evidence for the H1 FAKE-alias dossier: structural exhaustion axis "local
 * decl-order permutation" is measured DEAD. */
void func_8007C4B8(s32 *out, Rect *r)
{
  s32 var_a3;
  s16 new_var;
  s16 var_v0_2;
  s16 var_v0;
  u16 buf[4];
  /* ... body identical to candidate.c ... */
}
