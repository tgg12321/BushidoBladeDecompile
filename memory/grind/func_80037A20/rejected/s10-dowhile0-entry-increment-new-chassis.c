/* REJECTED [s10 rederive] The do-while(0) fold defeat re-measured on the s10 chassis (frontier said prior kills were chassis-stale): still does not defeat the cse1 REG_WAS_0 fold, and it costs the allocation.
 * sandbox --disable all = 14 (33 insns) vs the s10 candidate.c floor of 6.
 */
s32 func_80037A20(s32 arg0, s32 arg1)
{
  s32 *var_s0;
  s32 var_s1;
  s32 sp10[8];
  s32 v0_val;
  var_s1 = 0;
  var_s0 = (s32 *)&D_80102810;
  func_80079A30(sp10, (s32) (&g_str_memcard_fmt), arg0, arg1);
  if (bios_firstfile_B(sp10, var_s0) != 0)
  {
    do { var_s1++; } while (0);
    do {
      var_s0 = (s32 *) (((u8 *) var_s0) + 0x28);
      v0_val = bios_nextfile_B(var_s0);
      var_s1 += 1;
    } while (v0_val);
    var_s1 -= 1;
  }
  D_800A38C8 = var_s1;
  return var_s1;
}
