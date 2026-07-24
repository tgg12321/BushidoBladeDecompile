/* REJECTED (s3, structural). sandbox --disable all = 13 UNCHANGED.
 * Hypothesis: split the store+return refs off the counter pseudo onto a
 * separate `result` local, reducing the counter's n_refs (~6 -> ~4) below the
 * pointer's so the pointer sorts first in global.c allocno order and grabs s0
 * (fixing the s0<->s1 swap). KILLED: greg identical (order still "75 74",
 * 74/ptr -> s1(17), 75/counter -> s0(16)); `result` copy-propagated away (no
 * new pseudo in dispositions). ROOT MECHANISM: GCC 2.7.2 allocno_compare uses
 * floor_log2(n_refs); floor_log2(4)==floor_log2(6)==2, so the counter-vs-ptr
 * ref-count gap the ledger flagged as the swap driver CANNOT discriminate them.
 * The real discriminator is live_length (s1/s2 already proved unmovable). */
s32 func_80037A20(s32 arg0, s32 arg1)
{
  s32 *var_s0;
  s32 var_s1;
  s32 result;
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
  result = var_s1;
  D_800A38C8 = result;
  return result;
}
