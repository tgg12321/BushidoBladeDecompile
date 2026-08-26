/* REJECTED [s10 rederive] Declaring the walking pointer at the top and taking the base as a copy of it collapses back to the one-pointer body (the copy is propagated), so the s9 allocation win is lost.
 * sandbox --disable all = 13 (33 insns) vs the s10 candidate.c floor of 6.
 */
s32 func_80037A20(s32 arg0, s32 arg1)
{
  s32 *var_s0;
  s32 *p;
  s32 var_s1;
  s32 sp10[8];
  s32 v0_val;
  p = (s32 *)&D_80102810;
  var_s0 = p;
  func_80079A30(sp10, (s32) (&g_str_memcard_fmt), arg0, arg1);
  var_s1 = 0;
  if (bios_firstfile_B(sp10, var_s0) != 0)
  {
    var_s1++;
    do {
      p = (s32 *) (((u8 *) p) + 0x28);
      v0_val = bios_nextfile_B(p);
      var_s1 += 1;
    } while (v0_val);
    var_s1 -= 1;
  }
  D_800A38C8 = var_s1;
  return var_s1;
}
