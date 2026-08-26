/* candidate [s10, rederive modality] - honest, pin-free, cheat-free.
 * sandbox --disable all = 6  (s9 was 8, s1..s8 were 13; 33/33 insns).  FLOOR DROPPED.
 *
 * HEAD is INCLUDE_ASM (asm-until-matched); paste this body over the
 * INCLUDE_ASM line in src/code6cac_c.c before any sandbox re-measure.
 *
 * WHAT CHANGED vs the s9 candidate: the s9 base+walking POINTER SPLIT is GONE.
 * This is the ordinary single-walking-pointer body (the natural way to write a
 * PsyQ firstfile/nextfile directory scan) with ONE statement-order change from
 * the s1..s8 body: `var_s1 = 0;` is hoisted above the sprintf call, ahead of the
 * pointer initialisation.  Every construct is ordinary C; no coercion construct,
 * no FAKE annotation, nothing from any sanctioned-exception family.
 *
 * WHY IT WORKS - two independent cc1 mechanisms, both first-hand attributed in
 * s10 from tmp/grind/func_80037A20/dumps/ and tools/gcc-2.7.2/sched.c:
 *
 * (1) `la D_80102810` PLACEMENT is gated by sched.c birthing_insn_p().
 *     adjust_priority() raises an insn to max_priority (0x7f000001) iff it has
 *     zero REG_DEAD notes AND birthing_insn_p() - which returns
 *     `reg_n_sets[REGNO(dest)] == 1`.  sched1 schedules BACKWARD, so the boost
 *     pulls the insn LATE in emission (that is the pass's register-lifetime
 *     shortening heuristic).  The s9 split gave the base pointer exactly ONE set,
 *     so its `la` was boosted (sched1 dump: `13 (7f000001)`) and sank past the
 *     sprintf jal, wrecking target's prologue schedule.  A single walking pointer
 *     is set TWICE (the la + the `+= 0x28` in the loop), reg_n_sets == 2, no
 *     boost (`13 (1)`), and the la drifts to the top of the entry block exactly
 *     as target has it.
 *
 * (2) THE ALLOCATION is a global.c allocno_compare TIE, broken by pseudo number.
 *     pri = floor_log2(nrefs)*nrefs*size/live_length*10000; reg_n_refs is
 *     LOOP-DEPTH WEIGHTED (flow.c adds loop_depth per reference, so a loop-body
 *     reference counts twice).  Measured with tools/ra_solver/extract.py:
 *       s1..s8 goto chassis   : ptr 5/17 = 5882   counter 8/14 = 17142  [ledger]
 *       s10 do/while, init after call:
 *                               ptr 8/17 = 14117  counter 10/14 = 21428 -> counter
 *                               sorts first and takes $s0.  WRONG (score 13).
 *       s10 do/while, init BEFORE the call (this candidate):
 *                               ptr(74) 8/16 = 15000
 *                               counter(75) 10/20 = 15000   ** EXACT TIE **
 *                               tie -> lower pseudo number wins -> 74 (the
 *                               pointer, declared first) is allocated FIRST and
 *                               takes $s0; the counter is forced to $s1.
 *                               TARGET's assignment.  objdump confirms.
 *     Hoisting the zero-init above the call lengthens the counter's live range
 *     (14 -> 20) and shortens the pointer's (17 -> 16) at the same time, which is
 *     what lands both allocnos on 15000.  The declaration order of var_s0 before
 *     var_s1 is therefore load-bearing: it is what breaks the tie.
 *
 * REMAINING 6 diffs, all in the ENTRY BASIC BLOCK, for the next session:
 *   ours  : [sp] a2 a3 | sw s1 | s1=0 | sw s0 | la s0 | la a1 | sw ra | jal |
 *           (delay) addiu a0
 *   target: [sp] a2 a3 | sw s0 | la s0 | addiu a0 | la a1 | sw ra | jal |
 *           (delay) sw s1 | s1=0
 *   i.e. our `move s1,zero` is emitted at index 4 and target's at index 12, and
 *   the three register saves + the a0 setup redistribute around it (which insn
 *   fills the sprintf jal delay slot changes).  ROOT: sched1 schedules backward
 *   and `s1 = 0` has no consumer inside the entry block, so it is ready from the
 *   first cycle and rank_for_schedule falls through to INSN_LUID (sorted
 *   DESCENDING - the largest LUID is emitted last).  Hoisting the statement gave
 *   it a LOW luid, so it is emitted FIRST.  Target needs it emitted LAST in the
 *   block while still carrying the long live range that wins the tie.  Plus one
 *   pre-RA diff: entry increment `li $s1,1` vs target `addiu $s1,$s1,1`
 *   (cse.c FIRST-pass REG_WAS_0 const-prop; see hypotheses.md [s6]).
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
    var_s1++;
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
