/* REJECTED [s10 rederive] Reusing the pointer local to first hold the format-string address (aiming to give it reg_n_sets==2 so sched.c birthing_insn_p would stop boosting the la) does NOT work: cse1 propagates the symbol_ref straight into a1 and DELETES the first set, so reg_n_sets falls back to 1 and the la still sinks past the jal.
 * sandbox --disable all = 8 (33 insns) vs the s10 candidate.c floor of 6.
 */
s32 func_80037A20(s32 arg0, s32 arg1)
{
  s32 *var_s0;
  s32 *p;
  s32 var_s1;
  s32 sp10[8];
  s32 v0_val;
  var_s0 = (s32 *)&g_str_memcard_fmt;
  func_80079A30(sp10, (s32) var_s0, arg0, arg1);
  var_s0 = (s32 *)&D_80102810;
  var_s1 = 0;
  if (bios_firstfile_B(sp10, var_s0) != 0)
  {
    var_s1++;
    p = var_s0;
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
