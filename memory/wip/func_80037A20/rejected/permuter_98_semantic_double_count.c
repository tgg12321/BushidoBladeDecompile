/*
 * Rejected 2026-07-06 by fleet-blk (blocked-worker).
 *
 * Source: decomp-permuter, base score 298 -> this variant score 98.
 * This is the LOWEST-permuter-score variant found in a 300s -j 4 search
 * over candidate.c. It is NOT semantically equivalent to the target and
 * MUST NOT be used as a COMPLETED-C candidate.
 *
 * Rejection reason: SEMANTIC — double-counts files in the multi-file case.
 *   Target counts N files as N (final value of s1 = N).
 *   This variant counts N files as N+1 (via the second s1++ in loop body
 *   combined with the entry s1++ being folded into loop-head placement).
 *   For a 2-file save, target returns 2; this variant returns 3.
 *
 * DO NOT re-derive this shape in a future permuter run.
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
    if ((!sp10) && (!sp10))
    {
    }
    loop:
    var_s0 = (s32 *) (((u8 *) var_s0) + 0x28);

    var_s1++;                    /* MOVED — was at entry (before loop:) */
    v0_val = bios_nextfile_B(var_s0);
    var_s1 += 1;
    if (v0_val) goto loop;
    var_s1 -= 1;
  }
  D_800A38C8 = var_s1;
  return var_s1;
}
