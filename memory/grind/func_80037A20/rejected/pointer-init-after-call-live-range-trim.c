/* REJECTED (s1, recon): move the pointer init AFTER the func_80079A30 call to
 * trim var_s0's live range so its global.c priority rises above the counter and
 * it grabs s0 (matching target pointer->s0).
 *
 * RESULT: sandbox 16 (WORSE than the 13 floor). KILLED.
 * greg dump (tmp/grind/func_80037A20/s1/dump/pre.i.greg) is IDENTICAL to the
 * candidate baseline: allocation order still "75 74", pointer=pseudo74->s1(17),
 * counter=pseudo75->s0(16), and pseudo 74 STILL conflicts with a2/a3 (6,7).
 * GCC hoists the loop-invariant symbol-address load (la D_80102810) to the
 * function top regardless of C statement position, so the pointer live range /
 * a2-a3 conflict is not controllable by reordering. The reposition only worsened
 * instruction scheduling (+3 diffs). Confirms the swap is a robust priority wall.
 */
s32 func_80037A20(s32 arg0, s32 arg1)
{
  s32 *var_s0;
  s32 var_s1;
  s32 sp10[8];
  s32 v0_val;
  func_80079A30(sp10, (s32) (&g_str_memcard_fmt), arg0, arg1);
  var_s0 = (s32 *)&D_80102810;
  var_s1 = 0;
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
