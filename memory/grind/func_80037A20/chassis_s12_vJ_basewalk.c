/* CHASSIS vJ [s12, structural modality] - sandbox --disable all = 8, 33/33 insns,
 * 0 rules.  NOT the lowest-scoring form (candidate.c is 6) but the ONLY form with
 * a live route to zero, because it is the first THREE-pseudo decomposition:
 *   74 base    (la D_80102810)          3 refs / len 12 -> $s0 (copy preference)
 *   75 walking (p, loop-carried)        7 refs / len  6 -> $s0  pri 23333
 *   76 counter (var_s1)                10 refs / len 16 -> $s1  pri 18750
 * That is TARGET's assignment with a strict margin, and the `p = var_s0` copy is
 * byte-free (deleted at reload as a no-op move $s0,$s0).  Everything is ordinary C:
 * no coercion construct, no FAKE family, no pin, no barrier.
 *
 * WHY IT IS 8 AND NOT 0 - exactly two named cc1 gates remain:
 *  GATE 1  sched1 sinks `la $s0` past the sprintf jal.  sched.c:2504
 *          birthing_insn_p returns reg_n_sets[i]==1 for a live SET-of-REG dest and
 *          sched.c:2584 adjust_priority boosts it to max_priority; sched1 schedules
 *          BACKWARD so the boost emits it late.  Built with -fno-schedule-insns this
 *          body is TARGET LINE FOR LINE (delay slots included) except gate 2.
 *          Falls iff the base pseudo gets a byte-free, non-dead, cse1-surviving
 *          SECOND SET.  Three spellings measured and killed in s12 (see rejected/).
 *  GATE 2  cse1 REG_WAS_0 fold: our peel is `li $s1,1`, target's is `addiu $s1,$s1,1`.
 *          On THIS chassis the fold is DECOUPLED from the allocation for the first
 *          time (unfolding gives the counter 20625 vs the pointer's 23333), so every
 *          s2/s6/s10 fold-defeat kill is chassis-stale here and must be re-tried.
 *
 * HEAD is INCLUDE_ASM (asm-until-matched); paste this body over the INCLUDE_ASM line
 * in src/code6cac_c.c before any sandbox re-measure.
 */
s32 func_80037A20(s32 arg0, s32 arg1)
{
  s32 *var_s0;
  s32 *p;
  s32 var_s1;
  s32 sp10[8];
  s32 v0_val;
  var_s0 = (s32 *)&D_80102810;
  func_80079A30(sp10, (s32) (&g_str_memcard_fmt), arg0, arg1);
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
