/* REJECTED (s3, structural / type-narrowing lever). sandbox --disable all = 13,
 * 33 insns, UNCHANGED. Hypothesis: changing the counter's type (u32) might alter
 * allocation or disrupt the cse2 0+1 fold. KILLED: register width is unchanged
 * (both s32/u32 occupy one word reg) so allocation is identical, and the fold is
 * a cse2 const-prop that is type-independent. No codegen change. Type narrowing
 * is inert for both diffs. */
s32 func_80037A20(s32 arg0, s32 arg1)
{
  s32 *var_s0;
  u32 var_s1;
  s32 sp10[8];
  s32 v0_val;
  var_s0 = (s32 *)&D_80102810;
  func_80079A30(sp10, (s32) (&g_str_memcard_fmt), arg0, arg1);
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
