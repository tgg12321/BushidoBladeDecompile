/* REJECTED (s2, structural). sandbox --disable all = 15 (WORSE than 13).
 * Hypothesis: moving `var_s1 = 0;` ABOVE the func_80079A30 call would (a) extend
 * the counter pseudo's live range across the call so it loses the priority edge
 * over the pointer (fixing the s0<->s1 swap), and (b) put a call boundary between
 * the 0-init and the entry `var_s1++` so cse2 cannot const-propagate 0+1->1
 * (fixing the fold, diff #13). BOTH failed:
 *   greg: now BOTH pseudo 74 (ptr) and 75 (counter) conflict 6,7 (a2/a3) i.e.
 *         equal live ranges, but allocation order STILL "75 74" and dispositions
 *         STILL 74->s1(17), 75->s0(16). Equalizing live length did NOT flip the
 *         tiebreak: the counter now wins purely on ref-count (~6 refs vs ptr ~4).
 *   out.s: entry increment STILL `li $16,1` (fold persists across the call
 *         boundary); plus a redundant `addu $4,$sp,16` recompute after the call
 *         => +2 insns of scheduling damage.
 * KILLED. */
s32 func_80037A20(s32 arg0, s32 arg1)
{
  s32 *var_s0;
  s32 var_s1;
  s32 sp10[8];
  s32 v0_val;
  var_s0 = (s32 *)&D_80102810;
  var_s1 = 0;
  func_80079A30(sp10, (s32) (&g_str_memcard_fmt), arg0, arg1);
  if (bios_firstfile_B(sp10, var_s0) != 0)
  {
    var_s1++;
    loop:
    var_s0 = (s32 *) (((u8 *) var_s0) + 0x28);
    v0_val = bios_nextfile_B(var_s0);
    var_s1 += 1;
    if (v0_val) goto loop;
    var_s1 -= 1;
  }
  D_800A38C8 = var_s1;
  return var_s1;
}
