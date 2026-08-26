/* REJECTED [s10 rederive] s9's counter partition (n = var_s1 + 1) re-measured on the do/while single-pointer chassis: n(p79) nrefs=6 livelen=7 pri=17142, still above the pointer's 14117, so the counter family still sorts first. Loop-body references are loop-depth weighted (count twice), so the loop-carried counter pseudo cannot get below 6 refs at livelen 7.
 * sandbox --disable all = 13 (33 insns) vs the s10 candidate.c floor of 6.
 */
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
    s32 n = var_s1 + 1;
    do {
      var_s0 = (s32 *) (((u8 *) var_s0) + 0x28);
      v0_val = bios_nextfile_B(var_s0);
      n += 1;
    } while (v0_val);
    var_s1 = n - 1;
  }
  D_800A38C8 = var_s1;
  return var_s1;
}
