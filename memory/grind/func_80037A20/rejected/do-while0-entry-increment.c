/* REJECTED (s2, structural). sandbox --disable all = 13 (UNCHANGED).
 * Hypothesis: wrapping the entry increment in a sanctioned `do { } while(0)`
 * introduces a loop-note / basic-block boundary that cse2 will not const-propagate
 * the dominating `var_s1 = 0` across, so the entry `var_s1++` would emit
 * `addiu` (reading 0) like the target instead of our folded `li 1` (diff #13).
 * FAILED: the do-while(0) COLLAPSED (documented in the lost-codegen WIP notes:
 * "Per-case do { } while (0) wrapping collapses to nothing useful"). out.s entry
 * increment STILL `li $16,1` (fold persists); dispositions STILL 74->s1(17),
 * 75->s0(16) (swap persists); score identical 13. No BB boundary survived to
 * block the propagation. KILLED. */
s32 func_80037A20(s32 arg0, s32 arg1)
{
  s32 *var_s0;
  s32 var_s1;
  s32 sp10[8];
  s32 v0_val;
  var_s0 = (s32 *)&D_80102810;
  func_80079A30(sp10, (s32) (&g_str_memcard_fmt), arg0, arg1);
  var_s1 = 0;
  if (bios_firstfile_B(sp10, var_s0) != 0)
  {
    do { var_s1++; } while (0);
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
