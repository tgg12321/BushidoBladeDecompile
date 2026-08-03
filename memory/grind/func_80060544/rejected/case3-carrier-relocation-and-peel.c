/* REJECTED FORMS — func_80060544, grind session s7 (2026-08-03, modality: forensics)
 *
 * Everything in this file is a DIAGNOSTIC, never a candidate.  All rows were
 * measured with the sub-second instrumented-cc1 gate (BB2_SCHED_DEBUG=1 on the
 * reduced TU tmp/perm_60544/v_base.c), not with the sandbox: the gate reports
 * (a) the emitted order of `move $5,$0` vs the `la D_8009B7D0` in the Case3
 * block, (b) the la's destination pseudo and its set count in the .combine RTL
 * (the last dump before sched1), (c) whether that uid ever appears in a sched1
 * ready list at LAUNCH_PRIORITY (p=2130706433), (d) whether the `la` is still
 * INSIDE loop 1 in the emitted asm, and (e) the function's asm line count
 * (the base form == 117 lines == target's 133 instructions).
 *
 * Scripts: tmp/grind/func_80060544/s7/reloc_sweep.py and s7/peel_sweep.py.
 *
 * ============================================================ GROUP 1
 * CARRIER-RELOCATION (the s6 frontier): keep `end_off` (or `new_var3`) as the
 * Case3 staging carrier but MOVE its other definition, so that the carrier is
 * "genuinely live in the function" while being dead at the s.p_static store.
 *
 *   variant                    order     la dest     promo     hoist  size
 *   X0_base (control)          a1-after  r75/3sets   launch=0  in     117
 *   X2_endoff_after_loops      a1-after  r77/2sets   launch=0  in     117
 *   X3_endoff_between_loops    A1-FIRST  r77/1sets   launch=1  in     115
 *   X4_endoff_after_nostage    a1-after  r75/3sets   launch=0  in     115
 *   X5_newvar3_late            a1-after  r87/2sets   launch=0  in     117
 *   X9_stage_ctl (judge-FAIL)  A1-FIRST  r77/1sets   launch=1  in     117
 *
 * X2 — `end_off = arg0 + 0x5F4; new_var6 = end_off;` moved out of the prologue
 * to just before the initTexPage tail, with the Case3 staging kept:
 *
 *       Case3:
 *           stat = (s32)(&D_8009B7D0);
 *           end_off = stat;                  // staging copy
 *           s.p_static = (s32 *)end_off;
 *       ...
 *       end_off = arg0 + 0x5F4;              // moved down here
 *       new_var6 = end_off;
 *       initTexPage(...);
 *
 *   The copy is NOT deleted by combine (the la still has a 2-set destination)
 *   and the promotion does not fire.  Moving the carrier's other definition
 *   DOWNSTREAM of the store therefore breaks the retarget outright.
 *
 * X3 — the same two statements moved to sit BETWEEN loop 1 and loop 2 instead.
 *   This one DOES flip (1-set destination, LAUNCH_PRIORITY, A1-FIRST) — but the
 *   relocation itself costs the function two instructions (115 asm lines, and
 *   X4 shows the -2 comes from the relocation, not from the staging), so it can
 *   never reach target's 133.  And the Case3 statement is still a dead store, so
 *   it is in the judge-FAILed family regardless.
 *
 * X5 — `new_var3 = arg0 + 0x5DC;` moved down to just before its own uses
 *   (initTexPage / ot_Link) and used as the Case3 staging carrier: 2-set
 *   destination, no promotion.
 *
 * CONCLUSION FOR GROUP 1: relocating the carrier's other definition either
 * destroys the copy-retarget (X2, X5) or moves the function off target's size
 * (X3/X4).  There is no relocation that makes a genuinely-read variable dead at
 * exactly the s.p_static store AND leaves the rest of the function intact.
 *
 * ============================================================ GROUP 2
 * PEELING the Case3 iteration out of loop 1.  Motivated by the s7 mechanism
 * finding (see below): a dedicated single-set address local for Case3 is
 * LOOP-INVARIANT, so loop.c hoists the `la` into the preheader and there is
 * nothing left in the Case3 block to promote (this is the explanation of the s6
 * arm_sweep anomaly, where four single-set-destination forms all measured
 * launch=0).  Peeling the i == 3 iteration out of the loop removes the loop that
 * the hoist happens out of, so a dedicated local can stay single-set AND stay in
 * the block — a clean route with NO dead store anywhere.
 *
 *   variant                order     la dest     promo     size
 *   Y1_peel_own_local      A1-FIRST  r75/1sets   launch=1  112
 *   Y2_peel_shared_stat    a1-after  r75/3sets   launch=0  112
 *
 * Y1's shape (loop 1 runs i = 0..2 with the two ordinary arms; the i == 3
 * iteration is written out after the loop with its own address local `c3`):
 *
 *       do {
 *           geom = (s32)(&D_8009B770); geom += idx; s.p_geom = (s32 *)geom;
 *           if (i > 0) goto S800;
 *           if (i == 0) goto S7D8;
 *           goto Skip;
 *       S7D8: stat = (s32)(&D_8009B7D8); s.p_static = (s32 *)stat; goto Skip;
 *       S800: stat = (s32)(&D_8009B800); s.p_static = (s32 *)stat;
 *       Skip: s.arg1_field = prev; prev = func_8007352C(&s);
 *           i += 1; idx += 0xC;
 *       } while (i < last);
 *       geom = (s32)(&D_8009B770); geom += idx; s.p_geom = (s32 *)geom;
 *       c3 = (s32)(&D_8009B7D0); s.p_static = (s32 *)c3;
 *       s.pad0C = mid_off; mid_off = func_80073728(&s, 0);
 *
 * Y1 IS the first form ever measured that reproduces target's Case3 block order
 * with no dead store and no coercion — the promotion fires exactly as the model
 * predicts.  It is nonetheless REJECTED: peeling costs five instructions
 * (112 asm lines against the base's 117), so the function as a whole is now
 * further from target than the floor-2 form, and s3 had already established by
 * instruction count that the four-way in-loop ladder IS the original's block
 * structure.  Y2 (the same peel keeping the shared `stat`) is also 112 and does
 * not fire, which isolates the -5 as the peel's own cost.
 *
 * ============================================================ WHY THIS CLOSES
 * The promotion needs TWO things at once and they pull against each other:
 *   (R1) the `la` must still be in the Case3 block when sched1 runs.  With the
 *        Case3 arm inside loop 1 that requires its destination pseudo to be set
 *        more than once inside the loop — i.e. the shared `stat` carrier — or
 *        loop.c hoists the (loop-invariant) address out of the loop.
 *   (R2) at sched1 the la's destination must have reg_n_sets == 1, which with a
 *        multiply-set `stat` can only happen if combine RETARGETS the la onto
 *        another pseudo, which requires deleting a copy, which requires the
 *        copied value never to be read — a dead store.
 * Peeling (Group 2) is the only measured way to satisfy both, and it changes the
 * loop structure that target's instruction count pins down.  Relocation
 * (Group 1) does not help.  See hypotheses.md s7 for the full derivation.
 */
