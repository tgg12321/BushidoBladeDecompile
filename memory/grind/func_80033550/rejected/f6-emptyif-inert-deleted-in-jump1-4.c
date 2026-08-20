/* s8-postunpark (2026-08-20) REJECTED (honest sandbox 4, 34 insns - INERT):
 * F6 empty-if / fabricated-redundant-condition shape (.claude/rules/
 * no-new-park-categories.md:371, F6 survey ESTABLISHED 2026-08-18), the
 * newly-sanctioned family that caused this function's 2026-08-19 unpark.
 * MEASURED DEAD, pass named from dumps: the compare+branch is present in
 * code6cac_b.rtl (35 insns vs baseline 34) and is GONE by code6cac_b.jump
 * (24 == baseline 24) - jump_optimize pass 1 (jump.c, jump-to-next-insn
 * deletion + dead compare) removes it BEFORE any allocation pass. Every
 * dump from .jump onward, including .lreg/.greg, is byte-identical to the
 * baseline candidate: same 2 allocnos (74 72), same conflict sets
 * (72: 72 74 2 3 4 29), same dispositions (72 in 5 = $a1). Zero RA effect.
 * All five empty-if placements measured identical (after `found`, after
 * new_var, after w2, on !arg0, and the `arg0 && arg0` redundant-condition
 * spelling): tmp/grind/func_80033550/s8/variants/f6_emptyif_*.c.
 * Dumps: tmp/grind/func_80033550/s8/dumps_f6_emptyif_i_afterfound/. */
void func_80033550(s32 *arg0)
{
  s32 new_var;
  s32 i;
  s32 w0;
  s32 w1;
  s32 w2;
  s32 idx;
  i = 0;
  loop:;
  if ((*(&D_800A3918 + i)) == 0) goto found;
  i++;
  if (i < 6) goto loop;
  found:;
  if (i == 6) return;
  if (!i) { }
  *(&D_800A3918 + i) = 1;
  new_var = i * 12;
  /* FAKE: single-level wrap; loop notes fence sched1 so the final sll of
   * idx stays ahead of the three lw's, matching target order. */
  do { idx = new_var; w0 = arg0[0]; } while (0);
  w1 = arg0[1];
  w2 = arg0[2];
  *((s32 *) (((u8 *) (&D_80107850)) + idx)) = w0;
  *((s32 *) (((u8 *) (&D_80107854)) + idx)) = w1;
  *((s32 *) (((u8 *) (&D_80107858)) + idx)) = w2;
}
