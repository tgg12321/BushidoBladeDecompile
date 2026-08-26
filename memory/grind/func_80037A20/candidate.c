/* [s12, structural] BODY UNCHANGED; re-verified this session at
 * sandbox --disable all = 6, 33/33 insns, 0 rules.  READ THIS BEFORE GRINDING:
 * s12 proved that NO two-pseudo body (this one included) can ever reach target,
 * and found a THREE-pseudo chassis at sandbox 8 that already carries target's
 * $s0/$s1 assignment with a STRICT margin and, under -fno-schedule-insns, is
 * target line-for-line except the peel fold.  See
 * memory/grind/func_80037A20/chassis_s12_vJ_basewalk.c and hypotheses.md [s12].
 * This file is retained only because it is the lower SANDBOX number (6 vs 8);
 * the vJ chassis is the one with a live route to zero.
 */
/* candidate [s11, rederive modality] - UNCHANGED body from s10; floor still 6.
 *
 * s11 re-measured this exact body at sandbox --disable all = 6 (33/33 insns,
 * 0 rules) and then closed the s10 frontier with measurements:
 *
 *  - The $s0/$s1 disposition is decided ONLY by global.c allocno order (MIPS has
 *    no REG_ALLOC_ORDER in 2.7.2 and every hard-reg preference is pruned:
 *    prefs == {} on every body measured).  First allocno takes $s0.
 *  - Weighted ref counts are fixed by target's own 33 insns: Rp = 8, Rc = 10
 *    (11 with target's un-folded `addiu $s1,$s1,1`).  So the pointer wins the
 *    $s0 seat iff Lc >= 1.25 * Lp (1.375 un-folded).
 *  - Full 2x2 statement-position matrix (sandbox / ALLOCDBG):
 *       ptr before, zero before : 15000 vs 15000 tie -> ptr $s0 : 6   (this file)
 *       ptr after,  zero before :                                 : 7
 *       ptr after,  zero after  : 21818 vs 20000    -> ptr $s0 : 8
 *       ptr before, zero after  : 14117 vs 21428    -> ctr $s0 : 13
 *    Target needs Lp~17 with Lc~14 (ratio 0.82) against a bar of 1.25:
 *    THE STATEMENT-POSITION ROUTE TO TARGET IS FORECLOSED.
 *  - sched1 does NOT reorder this entry block (all insns tie at priority 1;
 *    emission == RTL order), and the callee-saved `sw $sN` saves are TEXT emitted
 *    by mips.c immediately before the first def of $sN - not RTL insns.
 *  - Therefore the remaining 6 diffs reduce to ONE reorg decision: reorg fills the
 *    sprintf jal delay slot with `addiu $a0,$sp,0x10` (the nearest length-4
 *    eligible insn; both `la`s are length-8 macros).  If that slot were left
 *    empty, mips.c's lazy `sw $s1` + `move $s1,$zero` would land after the jal and
 *    `addiu $a0,$sp,0x10` would stay before `la $a1` - bit-for-bit target, with
 *    the 15000/15000 tie untouched.  See hypotheses.md [s11] OPEN item.
 *
 * HEAD is INCLUDE_ASM (asm-until-matched); paste this body over the INCLUDE_ASM
 * line in src/code6cac_c.c before any sandbox re-measure.
 * Every construct is ordinary C: no coercion construct, no FAKE annotation,
 * nothing from any sanctioned-exception family.  The declaration order
 * `s32 *var_s0;` before `s32 var_s1;` is load-bearing (it is the allocno tie-break),
 * and `var_s1 = 0;` must precede `var_s0 = ...` which must precede the sprintf call.
 *
 * ---- s10 header retained below ----
 candidate [s10, rederive modality] - honest, pin-free, cheat-free.
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
