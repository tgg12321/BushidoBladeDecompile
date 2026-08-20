/* s8-postunpark (2026-08-20) REJECTED (honest sandbox 6-7, 33 insns):
 * F6 cancellation-pair shape (`x++; x--;`), the second half of the
 * newly-sanctioned F6 family (.claude/rules/no-new-park-categories.md:371).
 * MEASURED DEAD as a byte-free conflict-injection lever, with a hard
 * dichotomy proven over 16 placements (variants2/, variants3/):
 *  (i) pair on a variable whose live range does NOT overlap the arg0
 *      pointer pseudo 72 (i, idx, new_var, a fresh staged local, w1/w2
 *      after the last arg0 read): deleted by flow.c life-analysis DCE
 *      pre-RA - insn counts .cse2=24 -> .flow=21 == baseline 21; .greg
 *      identical to baseline (2 allocnos, 72 in $a1). Byte-free but INERT.
 *  (ii) pair on a value that IS consumed across 72's live range (w0 before
 *      w1, w0 between w1/w2, w1 before w2, and the 2-pair combinations):
 *      the pair SURVIVES flow and combine into global.c's conflict graph
 *      (3-4 allocnos; the new pseudo conflicts with 72 AND with hard 2,3,4)
 *      - real conflict injection, the thing s1-s7 never achieved byte-free.
 *      BUT the same liveness extension makes the entry copy `move a1,a0`
 *      coalescable: .greg prints `72 preferences: 4` (absent in baseline)
 *      and 72 LOSES its hard-reg-4 conflict, so global.c find_reg takes the
 *      PREFERRED reg $a0 before the numeric v0,v1,a0,a1,a2,a3 scan ever
 *      runs. Result: 33 insns (the entry move is deleted), distance 6-7.
 * Consequence (theorem, s8-postunpark): pseudo 72 is the destination of the
 * sole a0 entry copy, so it is either coalesced into $a0 (preference wins,
 * pre-empting the scan) or it conflicts with $a0 and no injected allocno
 * that could seat $a1/$a2 exists byte-free. Conflict injection via F6 can
 * therefore never route 72 to $a3 in this function.
 * Variants + dumps: tmp/grind/func_80033550/s8/variants{2,3}/,
 * dumps_c1_i_after_w2/, dumps_d1_w0pair_before_w1/. */
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
  *(&D_800A3918 + i) = 1;
  new_var = i * 12;
  /* FAKE: single-level wrap; loop notes fence sched1 so the final sll of
   * idx stays ahead of the three lw's, matching target order. */
  do { idx = new_var; w0 = arg0[0]; } while (0);
  w0++;
  w0--;
  w1 = arg0[1];
  w2 = arg0[2];
  *((s32 *) (((u8 *) (&D_80107850)) + idx)) = w0;
  *((s32 *) (((u8 *) (&D_80107854)) + idx)) = w1;
  *((s32 *) (((u8 *) (&D_80107858)) + idx)) = w2;
}
