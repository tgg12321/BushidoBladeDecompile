/* KILLED s2 (2026-07-19) — structural probe B: do-while(0) entry wrap.
 * Wrap ENTIRE function body in `do { ... } while (0);`. Measured floor
 * WORSENED to score=27 (from 4), same 164/164 build vs target insns.
 * do-while(0) emits NOTE_INSN_LOOP_BEG at function entry, which the
 * scheduler treats as a scheduling fence — reallocating body pseudos to
 * different callee-saves and reshuffling later loads. It does NOT touch
 * the prologue save+def pair emit order (still s1-pair first) because
 * expand_function_start runs BEFORE loop-note insertion.
 *
 * Evidence for the H1 FAKE-alias dossier: structural exhaustion axis
 * "do-while(0) entry wrap" is measured DEAD — actively degrades floor. */
void func_8007C4B8(s32 *out, Rect *r)
{
  u16 buf[4];
  s16 var_v0;
  s16 var_v0_2;
  s16 new_var;
  s32 var_a3;
  do {
  /* ... body identical to candidate.c ... */
  } while (0);
}
