# Hypothesis ledger — func_8002D780

## s1 (2026-09-08, recon) — floor 48 -> 47

### KILLED (instance kills; chassis = HEAD 543ed812 -mel -msoft-float, retired body with the sibling func_8002D320 cop2 spelling, no FAKE constructs present)
- H-s1-a: "Swapping the XOR operand order in the first triangle test alone reorders the
  block-5 multiplies toward the target." Measured v1 = 48 (no floor change; 70 vs 73
  differing insns). Instance kill.
- H-s1-b: "Reading px/pz through a pointer at the use sites (no locals) changes the
  test-1 schedule." Measured v2 = 48, no change. Instance kill.
- H-s1-c: "A plain `u32 ud = dist;` copy in the else arm materialises the target's
  `move a0,s1`." Measured v3 = 48, copy folded (cse make_regs_eqv), same as sibling
  func_8002D518's control. Instance kill.
- H-s1-d: "Writing the three-call tail as nested `== 0` ifs with a trailing `return 1`
  changes reorg's slot fill on the 2nd post-call branch." Measured v5 = 48; jump1
  produces the same relocated layout, so reorg sees the same liveness. Instance kill.

### CONFIRMED
- H-s1-e: XOR operand order `(cross_p ^ cross_c)` in all three tests is an ordinary-C
  lever on the sched1 order of block 5: v4 = 47 (tests 2/3 unchanged, test 1 reordered).

## Live frontier (for s2)
1. **Block-5 schedule (cluster 1, ~38 insns)** — mechanism: sched1 backward list
   scheduling, priority ties broken by dependence class then INSN_LUID
   (tools/gcc-2.7.2/sched.c rank_for_schedule). Target order: z0*cx, z0*px, x0*pz,
   cz-fixup, x0*cz, cross_p subu, cross_c subu. Next probe: run tools/sched_solver
   inverse on block 5 against the target order to get the emission order / dependency
   shape, then spell it: candidates are (a) cx/cz computed inline at their use points
   (`(z0 + z2) / 3` inside the cross expression, which would explain the DEFERRED cz
   sra/subu), (b) a per-edge helper macro `SIDE(ex,ez,px,pz)` evaluation order, (c) v4's
   operand order per test. Instrument with BB2_RANK_DEBUG=1 on the instrumented cc1
   (tools/gcc-2.7.2/cc1, s1/dbrdbg.py takes env names as args) to see class ties.
2. **`dist` copy into $a0 (cluster 2, ~5 insns)** — mechanism: second pseudo not
   coalesced because `dist` ($s1) stays live across the calls. Plain copy folds
   (H-s1-c). Next probe: natural forms first: the sqrt input computed as its own
   expression (`n = r_sq - y*y` for the sqrt while the call argument is a separate
   expression; cse may merge them), a u32-typed sqrt argument via a macro-style block;
   then, ONLY if the natural ladder is spent and banked here, the sibling's FAKE
   dead-store re-store (src/code6cac_b.c:1244-1265, dead-store-fake-exception) with
   this function's own lever-exhaustion cited. Also the sp+16 slot: target places the
   LZCR store at 16($sp), ours at 0($sp) — frame layout of `sp_var` vs other locals.
3. **Tail slot fill (cluster 3, 3 insns)** — mechanism: reorg.c fill_slots_from_thread
   refuses `li v0,1` for branch 381 because mark_target_live_regs reports $v0 live at
   the call-3 block entry (no label after jump1's relocation -> pending-dead never
   flushed). Next probe: shapes that change WHICH basic block / label chain
   mark_target_live_regs walks (give the call-3 block a label by making it a jump
   target, or place the shared `return 1` so no relocation happens); verify each with
   BB2_DBR_DEBUG=1 (s1/dbrdbg.py) — look for oppregs bit 2 on the `insn=381` line.

## [s1] Swapping the XOR operand order in the first triangle test alone reorders the block-5 multiplies toward the target.
- mechanism: sched1 backward list scheduling; priority ties broken by dependence class then INSN_LUID (tools/gcc-2.7.2/sched.c rank_for_schedule)
- probe: v1_xor_swap_t1.c via tools/sweep_variants.py; pairdiff_v1_xor_swap_t1.txt
- result: 48 (baseline 48); 70 differing insns vs 73 but no scored change
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 543ed812 (-mel -msoft-float), retired body with sibling func_8002D320 cop2 spelling, no FAKE constructs

## [s1] Reading px/pz through a pointer at the use sites (no px/pz locals) changes the test-1 schedule.
- mechanism: RTL emission order of the two loads relative to the multiplies (sched1 LUID tie-break)
- probe: v2_inline_loads.c via sweep_variants
- result: 48, no change; cse/sched already place the loads next to their multiplies
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 543ed812 (-mel -msoft-float), retired body with sibling cop2 spelling, no FAKE constructs

## [s1] A plain `u32 ud = dist;` copy in the else arm materialises the target's `move a0,s1` for the LZCS/shift path.
- mechanism: cse.c make_regs_eqv folds a single-def copy into the source pseudo (same finding as sibling func_8002D518 s8)
- probe: v3_ud_copy.c via sweep_variants
- result: 48, copy folded, no `move a0,s1` emitted
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 543ed812 (-mel -msoft-float), retired body with sibling cop2 spelling, no FAKE constructs

## [s1] Writing the three-call tail as nested `== 0` ifs with a trailing `return 1` changes reorg's delay-slot fill on the second post-call branch.
- mechanism: jump.c relocates the shared v0=1 block behind the sltu block either way; reorg.c mark_target_live_regs then reports $v0 live at the call-3 block entry (BB2_DBR_DEBUG trace insn=381 oppregs bit 2, setsopp=1)
- probe: v5_tail_nested.c via sweep_variants
- result: 48, identical layout and fill
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 543ed812 (-mel -msoft-float), retired body with sibling cop2 spelling, no FAKE constructs

## [s1] XOR operand order (cross_p ^ cross_c) in all three triangle tests is an ordinary-C lever on the sched1 order of block 5.
- mechanism: RTL emission order changes INSN_LUID tie-breaks in sched.c rank_for_schedule
- probe: v4_xor_swap_all.c via sweep_variants; pairdiff_v4_xor_swap_all.txt
- result: 47 (from 48); tests 2/3 still match in order, test 1 reordered but not yet to the target order
- verdict: CONFIRMED

## s2 (2026-09-08, structural) — floor 47 -> 10

Chassis for every measurement below: HEAD main @ 25c0188a (-mel -msoft-float), the s1
candidate applied to src/code6cac_b.c, NO FAKE construct present anywhere in the body.

### CONFIRMED
- H-s2-a: "Computing each triangle side test's two cross products into named locals
  (kc = centroid-side, kp = point-side) as separate statements, instead of one nested
  expression, gives sched1 the target's block-5 multiply order." Measured 23 (from 47).
- H-s2-b: "Writing the sign test with the centroid cross product as the XOR's first
  operand -- `(kc ^ kp) >= 0` -- matches the operand order the target's three tests
  share." Measured 14 (from 23); the reverse `(kp ^ kc)` measured 23.
- H-s2-c: "Reusing the same two cross-product locals across the three side tests, rather
  than declaring fresh ones per test, is load-bearing." Fresh locals per test measured
  52 vs 14 for reuse.
- H-s2-d: "The LZCS island's sibling clobber footprint $12-$15 (func_8002BC68 /
  func_8002BEA0, same TU, byte-matched, judge ruling 2026-07-28 recorded in-line at
  src/code6cac_b.c:751-758) moves our `mflo $t7` ($15) to the target's `mflo $t8`
  ($24)." Measured 10 (from 14), build_insns still 202.
- H-s2-e: "Dropping the redundant `(u8 *)` casts on `&D_8008D118` is byte-neutral."
  Measured 10 = control; removes the two declaration puns the brief flagged.
- H-s2-f: "Passing `\"r\"(&sp_var)` as a third asm operand reproduces the target's
  `addiu v0,sp,16; move t4,v0` LZCR slot addressing." Measured: the two insns appear,
  but build_insns goes 202 -> 203, because our tail still carries the extra `j` that
  cluster C accounts for. Confirmed as a mechanism, NOT yet adoptable alone.

### KILLED (all instance kills; chassis as stated above, no FAKE constructs present)
- H-s2-g: "Declaring px/pz before x0/x2/z0/z2, or z-before-x, reorders the block-5
  loads toward the target." Measured 48 and 49 (from 47).
- H-s2-h: "Reading px/pz through the object at the use sites, re-measured on the 47
  chassis (s1's v2 was measured on the 48 chassis), changes the test-1 schedule."
  Measured 47 = control, no change.
- H-s2-i: "Spelling the third (edge) test with the centroid difference as the multiply's
  first operand -- `(cx - x0) * dz` -- makes cc1 emit `subu v0,t2,t1` before
  `subu a0,t0,a3`, so reorg steals it into the test-2 bltz slot." Measured 27 (from 10);
  the kc-only variant 12; a `cx - x0` named local 17; inlining dz 10 (no change);
  declaring dx before dz 10; a split accumulation (`kc = dz*(cx-x0); kc -= ...`) 50;
  assigning kp before kc in test 3 37; fresh locals for test 3 only 10; hoisting dz/dx
  into the test-1 arm 47. Ten spellings, none moves the two-insn residual A.
- H-s2-j: "A copy of `dist` in the else arm survives cse on the 10-floor chassis (four
  spellings: u32 copy, s32 copy, copy declared in the outer block, copy used only as
  the srlv index)." All measured 10 = control; the copy is still folded
  (cse.c make_regs_eqv), reproducing s1's H-s1-c on the new chassis.
- H-s2-k: "One of six further tail shapes changes reorg's slot fill on the second
  post-call branch (goto-to-a-shared `yes:` label, nested `== 0` ifs re-measured, truthy
  `if (f(...))`, a `hit` local for the last call, `? 1 : 0`, `!!`)." All measured 10 =
  control. A seventh, a `s32 hit = 1;` result carrier returned from the early exits,
  measured 73 at 197 insns (structurally different function).
- H-s2-l: "Hoisting the p10C setup above the first call shortens the call-3 block enough
  to change reorg's liveness walk." Measured 94 at 200 insns.
- H-s2-m: "The sibling func_8002BC68's staged-u32 sqrt chain (`m = (u32)-2; m &= lzcr;
  sh = 0x16 - m; idx = dist >> sh; ...`, src/code6cac_b.c:769-780) closes part of
  cluster 2 here." Measured 10 = control, byte-neutral on this chassis.

## Live frontier (for s3) — 10 insns left, in three groups
1. **A, 2 insns — the edge test's delay-slot pick.** Target fills the test-2 `bltz` slot
   with `subu v0,t2,t1` (cx - x0); we fill it with `subu a0,t0,a3` (dz = z2 - z0).
   Mechanism: reorg.c fill_simple_delay_slots takes the first schedulable insn of the
   fall-through thread, so the question is sched1's order at the top of the test-3
   block. Ten source spellings measured (H-s2-i) without moving it. Next probe:
   `tools/sched_solver` inverse on the test-3 block against the target order (this is
   an order-exact model — it will say whether the pick is REACHABLE from any emission
   order, and if so which one), then spell that order; also `pwsh tools/grinder/dump.ps1
   func_8002D780` and read .sched for the priority/LUID of the two subus.
2. **B, 5 insns — cluster 2 (`dist` copy into $a0 + the LZCR slot address).** Two
   sub-parts, and s2 established they are coupled to C by the insn count: the
   `"r"(&sp_var)` operand DOES produce the target's `addiu v0,sp,16; move t4,v0` but
   costs a net insn while our tail still has the extra `j`. Nine natural copy spellings
   are now dead (s1 H-s1-c, s2 H-s2-j). Next probe: land C first, then re-measure the
   `&sp_var` operand; only if the natural ladder is then still short, the sibling
   func_8002D518's FAKE dead-store re-store (src/code6cac_b.c:1244-1265,
   .claude/rules/dead-store-fake-exception.md) with THIS function's exhaustion ledger
   (which is now nine measured spellings deep) cited.
3. **C, 3 insns — the tail slot fill.** Unchanged mechanism from s1 (reorg.c
   mark_target_live_regs reports $v0 live at the call-3 block entry because jump1
   relocated the shared `li v0,1` block and no CODE_LABEL flushes the pending-dead).
   Eleven tail shapes are now dead across s1 and s2. Next probe: attack the JUMP1
   relocation rather than the tail spelling — anything that keeps the three `return 1`
   sites from being cross-jumped into one shared block, e.g. giving the call-3 block a
   real label by making it a goto target from ABOVE the calls; verify each with
   BB2_DBR_DEBUG=1 (tmp/grind/func_8002D780/s1/dbrdbg.py) and look for oppregs bit 2 on
   the `insn=381` line.

## [s2] Computing each triangle side test's two cross products into named locals (kc = centroid-side, kp = point-side) as separate statements, instead of one nested expression, gives sched1 the target's block-5 multiply order.
- mechanism: separate RTL groups for the two multiply pairs change the INSN_LUID tie-break in sched1's backward list scheduler (tools/gcc-2.7.2/sched.c rank_for_schedule)
- probe: variant e_named_cross via tools/sweep_variants.py (tmp/grind/func_8002D780/s2/variants/)
- result: 23, from 47; build_insns stayed 202
- verdict: CONFIRMED

## [s2] Writing the sign test with the centroid cross product as the XOR's first operand -- (kc ^ kp) >= 0 -- matches the operand order the target's three tests share.
- mechanism: RTL emission order of the two cross-product groups feeds sched1's tie-break
- probe: variant e2_reuse_cfirst_xor vs e0_control (tmp/grind/func_8002D780/s2/variants2/)
- result: 14 with centroid first, 23 with point first
- verdict: CONFIRMED

## [s2] Reusing the same two cross-product locals across the three side tests, rather than declaring fresh ones per test, is load-bearing on this chassis.
- mechanism: local-alloc quantity numbering follows the number of live pseudos in the block; three live pairs permute the register assignment of the whole triangle region
- probe: variant e1_fresh_locals vs e2_reuse_cfirst_xor
- result: fresh locals 52, reuse 14
- verdict: CONFIRMED

## [s2] The LZCS island's sibling clobber footprint $12-$15 (func_8002BC68 / func_8002BEA0, same TU, byte-matched on main, judge ruling 2026-07-28 recorded in-line at src/code6cac_b.c:751-758) moves our mflo $t7 ($15) to the target's mflo $t8 ($24).
- mechanism: reload1.c bad_spill_regs: RTL mention of $13-$15 is the only route by which reload can pick $24 for the reload-emitted mflo
- probe: variant g1_clobbers (tmp/grind/func_8002D780/s2/variants4/), pairdiff tmp/grind/func_8002D780/s2/pairdiff_g1.txt
- result: 10, from 14; build_insns still 202; insns 116/117 (mflo t8 / subu s1,a1,t8) and two permutation insns close
- verdict: CONFIRMED

## [s2] Passing "r"(&sp_var) as a third asm operand reproduces the target's addiu v0,sp,16 / move t4,v0 LZCR slot addressing.
- mechanism: an r-constrained address operand forces cc1 to materialise the frame slot address into a pseudo instead of the hardcoded $sp in the asm text
- probe: variants g2_slot_addr / g3_both (tmp/grind/func_8002D780/s2/variants4/)
- result: the two target insns appear, but build_insns goes 202 -> 203 because our tail still carries the extra j from the cluster-C residual; score stays 10 — a real mechanism, not adoptable until the tail lands
- verdict: CONFIRMED

## [s2] Declaring px/pz before x0/x2/z0/z2, or declaring the z pair before the x pair, reorders the block-5 loads toward the target.
- mechanism: declaration order sets the RTL emission order of the four loads, which sched1 breaks ties on by INSN_LUID
- probe: variants f_decl_pfirst and h_decl_zfirst (tmp/grind/func_8002D780/s2/variants/)
- result: 48 and 49, both worse than the 47 control
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 25c0188a (-mel -msoft-float), s1 candidate body, no FAKE constructs present

## [s2] Reading px/pz through the object at the use sites (s1's v2 re-measured on the 47 chassis rather than the 48 chassis it was killed on) changes the test-1 schedule.
- mechanism: RTL emission order of the two loads relative to the multiplies
- probe: variant g_pz_at_use (tmp/grind/func_8002D780/s2/variants/)
- result: 47, identical to the control — s1's kill reproduces on the newer chassis
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 25c0188a (-mel -msoft-float), s1 candidate body, no FAKE constructs present

## [s2] One of ten spellings of the third (edge) side test makes cc1 emit subu v0,t2,t1 (cx - x0) before subu a0,t0,a3 (z2 - z0), so reorg steals the centroid difference into the test-2 bltz slot as the target does.
- mechanism: reorg.c fill_simple_delay_slots takes the first schedulable insn of the fall-through thread, so the pick follows sched1's order at the top of the test-3 block
- probe: variants f1-f6 (variants3/), m1-m6 (variants6/), n1-n5 (variants7/) via sweep_variants
- result: no spelling moved the two-insn residual: inline dz 10, dx-declared-first 10, fresh test-3 locals 10 (all = control); kc-only centroid-first operand 12, cx-x0 named local 17, centroid-first operand in both terms 27, kp-before-kc 37, split accumulation 50, dz/dx hoisted into the test-1 arm 47
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 25c0188a (-mel -msoft-float), the s2 10-floor body (named cross locals + sibling clobbers), no FAKE constructs present

## [s2] A copy of dist in the else arm survives cse on the 10-floor chassis in one of four spellings (u32 copy, s32 copy, copy declared in the outer block, copy used only as the srlv index), materialising the target's move a0,s1.
- mechanism: cse.c make_regs_eqv folds a single-def copy into its source pseudo
- probe: variants u_a..u_d (tmp/grind/func_8002D780/s2/variants5/) and q5_p1_plus_ud (variants9/)
- result: all 10, identical to the control; the copy is folded in every spelling, reproducing s1's H-s1-c on the new chassis
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 25c0188a (-mel -msoft-float), the s2 10-floor body, no FAKE constructs present

## [s2] One of six further tail shapes changes reorg's delay-slot fill on the second post-call branch (goto to a shared yes: label, nested == 0 ifs re-measured, truthy if, a result local for the last call, ? 1 : 0, and !!).
- mechanism: reorg.c mark_target_live_regs reports $v0 live at the call-3 block entry because jump1 relocated the shared li v0,1 block and no CODE_LABEL flushes the pending-dead bit
- probe: variants t_a..t_d (variants5/), q1/q2/q4 (variants9/)
- result: all 10, identical to the control; the seventh shape (a hit = 1 result carrier returned from the early exits) restructured the function to 197 insns and scored 73
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 25c0188a (-mel -msoft-float), the s2 10-floor body, no FAKE constructs present

## [s2] Hoisting the p10C setup above the first call shortens the call-3 block enough to change reorg's liveness walk.
- mechanism: fewer insns in the call-3 block change which insn reorg finds first in the opposite thread
- probe: variant q3_p10c_hoisted (tmp/grind/func_8002D780/s2/variants9/)
- result: 94 at 200 build insns — the hoist also changes what the two earlier calls see, so the whole region diverges
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 25c0188a (-mel -msoft-float), the s2 10-floor body, no FAKE constructs present

## [s2] The byte-matched sibling func_8002BC68's staged-u32 sqrt chain (m = (u32)-2; m &= lzcr; sh = 0x16 - m; idx = dist >> sh; ...) transplanted onto this chassis closes part of the LZCS residual.
- mechanism: sibling-ledger transplant mandate — the sibling ships this exact block byte-matched at src/code6cac_b.c:769-780
- probe: variant p2_sibling_chain (tmp/grind/func_8002D780/s2/variants8/)
- result: 10, byte-neutral against the simpler func_8002D320 spelling the candidate keeps
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 25c0188a (-mel -msoft-float), the s2 10-floor body, no FAKE constructs present

## [s2] Dropping the redundant (u8 *) casts on &D_8008D118 (already declared extern u8 at src/code6cac_b.c:279) is byte-neutral and removes the two declaration puns the brief flagged.
- mechanism: no codegen effect — the cast is a no-op on an already-u8 lvalue address
- probe: variant p1_no_pun_cast (tmp/grind/func_8002D780/s2/variants8/)
- result: 10, identical to the control; adopted into candidate.c
- verdict: CONFIRMED

## [s3] Writing the third func_8002D518 call as `if (... != 0) return 1; return 0;` instead of `return ... != 0;` changes the BLOCK STRUCTURE enough for reorg.c to steal `li v0,1` into the second post-call branch's delay slot.
- mechanism: all three post-call exits become the same shape, so jump.c's cross-jump merges them differently and the call-3 block no longer sits behind the relocated shared block; reorg.c fill_simple_delay_slots then wins the steal that s1's BB2_DBR_DEBUG trace showed it losing (oppregs bit 2 / $v0 reported live at insn 390)
- probe: variant c1_third_call_if_form (tmp/grind/func_8002D780/s3/variants/), pairdiff tmp/grind/func_8002D780/s3/pairdiff_d1.txt
- result: 7 from 10 at 201 insns; the standalone `.L303: li v0,1` block and the `j` over it are both gone -- the entire tail matches the target insn-for-insn
- verdict: CONFIRMED

## [s3] With the tail at 201 insns, the s2-banked `"r"(&sp_var)` LZCR slot-address operand pays for itself and restores the count to the target's 202.
- mechanism: an r-constrained address operand materialises the frame-slot address into a pseudo (`addiu v0,sp,16; move t4,v0`) instead of the asm text's hardcoded `$sp`, costing one net insn -- which is exactly the insn the tail change gave back
- probe: variant d1_c1_plus_slotaddr (tmp/grind/func_8002D780/s3/variants2/)
- result: 7 at 202 insns; s2 measured the same operand at 203 insns and could not adopt it. Residuals B and C were coupled by the insn count in BOTH directions.
- verdict: CONFIRMED

## [s3] Writing the third triangle side test as the cross product of two vertex-0-relative vectors (six named difference locals) gives the target's emission order for the first subu pair of the test-3 block.
- mechanism: declaration order sets INSN_LUID, and sched1's rank_for_schedule (tools/gcc-2.7.2/sched.c) falls through priority and dependence-class to the LUID tie-break for two same-unit subus; the lower-LUID insn of a pair lands first in the block
- probe: variant e2_full_relative_vectors (tmp/grind/func_8002D780/s3/variants3/), pairdiff tmp/grind/func_8002D780/s3/pairdiff_e2.txt
- result: 7 = control on score, but `subu v0,t2,t1` (cx - x0) now precedes `subu a0,t0,a3` (z2 - z0) insn-for-insn; the residual moves to the second pair (dx vs az)
- verdict: CONFIRMED

## [s3] Declaring dx ahead of az fixes residual A's remaining subu-pair order without a register cost.
- mechanism: the same LUID tie-break; dx at a lower LUID than az should put dx first in the block as the target has it
- probe: variants f1_ax_dz_dx_az_bx_bz and f3_ax_dx_dz_az_bx_bz (tmp/grind/func_8002D780/s3/variants4/), pairdiffs pairdiff_f1.txt / pairdiff_f3.txt
- result: both measured 14. The ORDER is fixed -- the whole test-3 block matches the target's emission order -- but dz and dx are permuted between $a0 and $v1 (ours dz to $v1 / dx to $a0; target dz to $a0 / dx to $v1) and the permutation propagates through the mult/mflo chain for 7 insns. Residual A is therefore an allocation residual, not a scheduling one.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 6d8020d9 (-mel -msoft-float), the s3 7-floor body (relative-vector test 3 + if/return-0 tail + &sp_var slot operand), no FAKE constructs present

## [s3] One of six declaration orders / inlinings of the six vertex-0-relative difference locals gives both the target's emission order and the target's dz/dx allocation.
- mechanism: declaration order drives both the sched1 LUID tie-break and local-alloc's quantity numbering, so a single order might satisfy both at once
- probe: variants f1/f2/f3/f4/f5/f6 (variants4/) and g1..g5 (variants5/: az inlined, az+bz inlined, az inlined with dx first, the mult terms' operands swapped, az/bz declared last)
- result: only ax,az,bx,bz,dz,dx (7) and ax,az,dz,dx,bx,bz (7) hold the floor; every other order and every inlining measured 14, the swapped-operand form 22, reassigning cx/cz in place 28, and fully inlining every difference 26/27
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 6d8020d9 (-mel -msoft-float), the s3 7-floor body, no FAKE constructs present

## [s3] A copy of dist survives cse on the 7-floor chassis in one of five further spellings (mag copy at the head of the else arm covering both uses, the same copy in the outer block, asm-input-only, srlv-index-only, u32 copy).
- mechanism: cse.c make_regs_eqv folds a single-def copy into its source pseudo
- probe: variants b1..b5 (tmp/grind/func_8002D780/s3/variants6/)
- result: all 7 = control. Fourteen natural copy spellings are now dead across s1-s3.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 6d8020d9 (-mel -msoft-float), the s3 7-floor body, no FAKE constructs present

## [s3] Fully inlining every difference in the third side test (no dz/dx locals at all), with each product written difference-from-origin first, reproduces the target's test-3 emission order.
- mechanism: expand_expr emits a binary operation's operands left-to-right, so the emission order is the written order
- probe: variants a1_expanded_origin_first, a2_expanded_cz_first, a3_dz_local_only, a4_expanded_dz_first_operand (tmp/grind/func_8002D780/s3/variants/)
- result: 26, 27, 25 and 10 respectively (control 10) -- the fully-inlined forms lose the cse'd dz/dx pseudos and repermute the whole block
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 6d8020d9 (-mel -msoft-float), the s2 10-floor body, no FAKE constructs present

## [s3] Giving the call-3 block a real CODE_LABEL from above (`if (f2 == 0) goto last; return 1; last: return f3 != 0;`) changes reorg's liveness walk on the second post-call branch.
- mechanism: a real CODE_LABEL at the call-3 block head flushes reorg.c mark_target_live_regs' pending-dead $v0
- probe: variant c4_goto_last_label (tmp/grind/func_8002D780/s3/variants/)
- result: 10 = control; jump1 deletes the goto and rebuilds the same relocation. The winning structural change was the `return 0` fall-out form, not a label.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 6d8020d9 (-mel -msoft-float), the s2 10-floor body, no FAKE constructs present

## Live frontier (for s4) -- 7 insns left, in two groups
1. **A, 2 insns -- the dz/dx seat, NOT a schedule.** f1/f3 already produce the target's
   emission order for the entire test-3 block; the only thing wrong is that dz lands in
   $v1 and dx in $a0 where the target has dz in $a0 and dx in $v1. Next probe:
   `tools/ra_solver/inverse_compose.py classify` on those two seats against the f1 body
   (the local-alloc/global.c models are order- and seat-exact and return
   REACHABLE/FORECLOSED plus a ranked C-lever vector); only spell a form after the
   solver names one. Do NOT re-sweep declaration orders -- twelve are measured this
   session, and the two that hold the floor are the two in candidate.c.
2. **B, 5 insns -- the `dist` copy into $a0.** Fourteen natural copy spellings are dead
   (s1 H-s1-c, s2 H-s2-j, s3 above); cse.c make_regs_eqv folds every one. Everything
   else in the sqrt region now matches (LZCR slot addressing, `mflo t8`, `lw 16($sp)`),
   so this is the LAST thing between the two sqrt blocks -- our `nop` in the beqz delay
   slot vs the target's `move a0,s1`. Next probe: `tools/ra_solver` on the $a0 seat
   (inverse_compose.py classify); if that returns FORECLOSED for every natural form,
   the sibling func_8002D518's FAKE dead-store re-store (src/code6cac_b.c:1244-1265,
   .claude/rules/dead-store-fake-exception.md) is the sanctioned last resort, and this
   function's exhaustion ledger is now FOURTEEN measured spellings deep.
3. **Vetting note for whoever reaches 0.** candidate.c's test 3 declares six fresh
   once-written locals (ax/az/bx/bz/dx/dz). They are real consumed values with a plain
   geometric reading (vertex-0-relative vectors) and each appears in the target's own
   bytes, but a candidate-ready self-vet should either classify them under the
   named-intermediate family (`.claude/rules/narrow-byte-args-packed-call.md` plus the
   2026-08-17 clarification in no-new-park-categories.md) and walk its six prongs, or
   argue them as ordinary C on the strength of the geometry. Decide that BEFORE
   submitting -- review verdicts are keyed by body, so do not respell after a layer-1
   verdict.

## [s3] Writing the third func_8002D518 call as `if (... != 0) return 1; return 0;` instead of `return ... != 0;` changes the block structure enough for reorg.c to steal `li v0,1` into the second post-call branch's delay slot.
- mechanism: All three post-call exits become the same shape, so jump.c's cross-jump merges them differently and the call-3 block no longer sits behind the relocated shared `li v0,1` block; reorg.c fill_simple_delay_slots then wins the steal that s1's BB2_DBR_DEBUG trace showed it losing (oppregs bit 2 / $v0 reported live at insn 390).
- probe: variant c1_third_call_if_form (tmp/grind/func_8002D780/s3/variants/), scored via tmp/grind/func_8002D780/s3/run.ps1; diff tmp/grind/func_8002D780/s3/pairdiff_d1.txt
- result: 7 from 10, at 201 build insns. The standalone .L303 li v0,1 block and the j over it are both gone and the whole tail matches the target insn-for-insn. Eleven tail SPELLINGS were dead across s1/s2; this is a block-structure change, which is exactly what the s2 frontier said to attack.
- verdict: CONFIRMED

## [s3] With the tail landing at 201 insns, the s2-banked "r"(&sp_var) LZCR slot-address operand pays for itself and restores the count to the target's 202.
- mechanism: An r-constrained address operand makes cc1 materialise the frame-slot address into a pseudo (addiu v0,sp,16; move t4,v0) instead of the asm text's hardcoded $sp, costing one net insn - exactly the insn the tail change gave back.
- probe: variant d1_c1_plus_slotaddr (tmp/grind/func_8002D780/s3/variants2/)
- result: 7 at exactly 202 build insns (target_insns 202). s2 measured the same operand at 203 insns and could not adopt it; residuals B and C were coupled by the insn count in BOTH directions.
- verdict: CONFIRMED

## [s3] Writing the third triangle side test as the cross product of two vertex-0-relative vectors (six named difference locals ax/az/bx/bz/dz/dx) gives the target's emission order for the first subu pair of the test-3 block.
- mechanism: Declaration order sets INSN_LUID, and sched1's rank_for_schedule (tools/gcc-2.7.2/sched.c) falls through the priority and dependence-class tests to the LUID tie-break for two same-unit subus; the lower-LUID insn of a pair lands first in the block.
- probe: variant e2_full_relative_vectors (tmp/grind/func_8002D780/s3/variants3/), diff tmp/grind/func_8002D780/s3/pairdiff_e2.txt
- result: 7 = control on score, but subu v0,t2,t1 (cx - x0) now precedes subu a0,t0,a3 (z2 - z0) insn-for-insn as the target has it; the two-insn residual moves to the second pair (dx vs az). Adopted into candidate.c.
- verdict: CONFIRMED

## [s3] Declaring dx ahead of az fixes residual A's remaining subu-pair order without a register cost.
- mechanism: The same sched1 LUID tie-break: dx at a lower LUID than az should put dx first in the block as the target has it.
- probe: variants f1_ax_dz_dx_az_bx_bz and f3_ax_dx_dz_az_bx_bz (tmp/grind/func_8002D780/s3/variants4/), diffs pairdiff_f1.txt / pairdiff_f3.txt
- result: Both measured 14. The order IS fixed - the whole test-3 block matches the target's emission order - but dz and dx are permuted between $a0 and $v1 (ours dz->$v1, dx->$a0; target dz->$a0, dx->$v1) and the permutation propagates through the mult/mflo chain for 7 insns. This reclassifies residual A from a scheduling residual to an allocation residual.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 6d8020d9 (-mel -msoft-float), the s3 7-floor body (relative-vector test 3 + if/return-0 tail + &sp_var slot operand), no FAKE constructs present

## [s3] One of six further declaration orders or inlinings of the six vertex-0-relative difference locals gives both the target's emission order and the target's dz/dx allocation.
- mechanism: Declaration order drives both the sched1 LUID tie-break and local-alloc's quantity numbering, so a single order might satisfy both at once.
- probe: variants f1/f2/f3/f4/f5/f6 (variants4/) and g1..g5 (variants5/: az inlined, az+bz inlined, az inlined with dx first, mult operands swapped, az/bz declared last)
- result: Only ax,az,bx,bz,dz,dx (7) and ax,az,dz,dx,bx,bz (7) hold the floor; every other order and every inlining measured 14, the swapped-operand form 22, reassigning cx/cz in place 28, and fully inlining every difference 26/27. Twelve orders/inlinings measured this session.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 6d8020d9 (-mel -msoft-float), the s3 7-floor body, no FAKE constructs present

## [s3] A copy of dist survives cse on the 7-floor chassis in one of five further spellings (mag copy at the head of the else arm covering both uses, the same copy in the outer block, asm-input-only, srlv-index-only, u32 copy).
- mechanism: cse.c make_regs_eqv folds a single-def copy into its source pseudo, so the copy insn dies and flow deletes it.
- probe: variants b1..b5 (tmp/grind/func_8002D780/s3/variants6/)
- result: All 7 = control. Fourteen natural copy spellings are now dead across s1-s3; re-measured here because the chassis changed materially (tail + slot operand).
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 6d8020d9 (-mel -msoft-float), the s3 7-floor body, no FAKE constructs present

## [s3] Fully inlining every difference in the third side test (no dz/dx locals at all), with each product written difference-from-origin first, reproduces the target's test-3 emission order.
- mechanism: expand_expr emits a binary operation's operands left-to-right, so emission order follows written order.
- probe: variants a1_expanded_origin_first, a2_expanded_cz_first, a3_dz_local_only, a4_expanded_dz_first_operand (tmp/grind/func_8002D780/s3/variants/)
- result: 26, 27, 25 and 10 respectively against a control of 10 - the fully-inlined forms lose the cse'd dz/dx pseudos and repermute the whole block. Banked as rejected/test3-fully-inlined-origin-first-26.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 6d8020d9 (-mel -msoft-float), the s2 10-floor body, no FAKE constructs present

## [s3] Giving the call-3 block a real CODE_LABEL from above (if (f2 == 0) goto last; return 1; last: return f3 != 0;) changes reorg's liveness walk on the second post-call branch.
- mechanism: A real CODE_LABEL at the call-3 block head would flush reorg.c mark_target_live_regs' pending-dead $v0 bit.
- probe: variant c4_goto_last_label (tmp/grind/func_8002D780/s3/variants/)
- result: 10 = control; jump1 deletes the goto and rebuilds the same relocation. The structural change that DID work was the return-0 fall-out form, not a label.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 6d8020d9 (-mel -msoft-float), the s2 10-floor body, no FAKE constructs present

## [s4] A decomp-permuter campaign on the s3 7-floor chassis finds a form that lowers the engine's honest floor below 7.
- mechanism: The permuter's random C mutations explore statement/declaration/expression rearrangements far faster than hand variants, and the two open residuals (a dz/dx seat permutation in test 3, a folded dist copy in the sqrt block) are both the kind of local rearrangement its pass set covers.
- probe: A faithful workspace was built and validated for this function for the first time (tmp/grind/func_8002D780/s4/nonmatchings/func_8002D780; see evidence.md [s4] for the four hurdles and the rebuild recipe), then `tools/permuter_campaign.py launch --func func_8002D780 -j 8 --stop-on-zero`, waited in-turn, harvested with --stop. 55,531 iterations, 12 finds, best permuter weighted score 110 from a base of 300. Every find at or below 125 was extracted and re-scored against the real engine sandbox via tmp/grind/func_8002D780/s3/run.ps1.
- result: No find improved the honest floor. output-110-1 = 8, output-115-1 = 8, output-115-2 = 8, output-115-3 = 9, output-125-1 = 9, output-210-1 = 7 at 201 insns; the regenerated control = 7 at 202. The permuter's weighted scorer (regs x5, reorderings x60, ins/del x100) is ANTI-CORRELATED with the engine's raw differing-instruction score on this chassis - it descended 300 -> 110 entirely by trading raw diffs for reordering credit. Banked: memory/grind/func_8002D780/rejected/permuter-best-110-find-engine-8.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 8cd6da6b (-mel -msoft-float), the s3 7-floor candidate.c body spliced into src/code6cac_b.c (score 7, build_insns 202), no FAKE constructs present

## [s4] The dist copy of residual B survives cse if it is spelled as an embedded assignment inside a condition (`if ((u32)(m = dist) < 0x400)`) rather than as a plain `s32 c = dist;` declaration.
- mechanism: cse.c make_regs_eqv folds a single-def copy INSN into its source pseudo. An assignment written inside a condition expression is emitted by expand_expr at a different point in the RTL stream and is also a use in a comparison, so the hope was that the copy would not present as a plain single-def move for make_regs_eqv to fold. This spelling is the one decomp-permuter itself converged on in output-110-1, and no prior session had tried it.
- probe: Nine arrangements generated by tmp/grind/func_8002D780/s4/gen_d.py and scored with tmp/grind/func_8002D780/s3/run.ps1 against the candidate.c control - m carrying (asm input + srlv), (asm input only), (srlv only), (asm + srlv + small-path LUT index), (asm + srlv with m assigned in the `dist >= 0` condition), (call args only), (small-path LUT + srlv + calls), (small-path LUT + calls), and the semantically-wrong else-arm-assignment form.
- result: All eight well-defined arrangements measured 7 = control at exactly 202 build insns - i.e. no `move` insn materialised at all, so cse folded every one. (The ninth, assigning m inside the else arm's `dist >= 0` test while reading it on the small path, is undefined on the small path and measured 57 at 203 insns; not a codegen result.) Note that the permuter's own output-110-1 also stayed at 202 insns, so its score of 8 came from its block restructuring, not from a surviving copy. Twenty-three dist-copy spellings are now dead across s1-s4. Banked: memory/grind/func_8002D780/rejected/embedded-assign-dist-copy-still-folded-7.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 8cd6da6b (-mel -msoft-float), the s3 7-floor candidate.c body spliced into src/code6cac_b.c, no FAKE constructs present

## [s4] decomp-permuter's context stripping (ast_util.extract_fn replaces every other function definition in base.c with a declaration) makes the permuter's compile context unfaithful to the engine sandbox's full-TU build for this function.
- mechanism: GCC 2.7.2 is compiled per translation unit; the memory `cc1-first-pass-scheduler-bug` records that ~1400 lines of preceding declarations changes cc1's behaviour, so losing 60-odd function bodies could plausibly move func_8002D780's codegen.
- probe: tmp/grind/func_8002D780/s4/validate3.sh - regenerate base.c through ast_util twice, once with the full TU (from_import path, bodies kept) and once through extract_fn (bodies stripped), compile both with the workspace compile.sh, and objdump-diff func_8002D780 out of each.
- result: 207 insns each side; the only differences are branch-target text (relative offsets, which both scorers normalise) and the `jal` relocation symbol names. Stripping is codegen-neutral for this function, so the campaign's null result is a real result about the search, not a workspace artifact.
- verdict: CONFIRMED

## [s4] A decomp-permuter campaign on the s3 7-floor chassis finds a form that lowers the engine's honest floor below 7.
- mechanism: The permuter's random C mutations explore statement/declaration/expression rearrangements far faster than hand variants, and both open residuals (a dz/dx seat permutation in test 3, a folded dist copy in the sqrt block) are the kind of local rearrangement its pass set covers.
- probe: Built and validated a faithful workspace (tmp/grind/func_8002D780/s4/nonmatchings/func_8002D780) via tools/decomp-permuter/import.py --no-prune plus four hand fixes, then tools/permuter_campaign.py launch --func func_8002D780 -j 8 --stop-on-zero, waited in-turn with the campaign wait command, harvested with --stop. 55,531 iterations, 12 finds, best permuter weighted score 110 from a base of 300. Every find at or below 125 was extracted and re-scored against the real engine sandbox.
- result: No find improved the honest floor. output-110-1 = 8, output-115-1 = 8, output-115-2 = 8, output-115-3 = 9, output-125-1 = 9, output-210-1 = 7 at 201 insns; the regenerated control = 7 at 202. The permuter's weighted scorer (regs x5, reorderings x60, ins/del x100) is anti-correlated with the engine's raw differing-instruction score on this chassis: it descended 300 -> 110 entirely by trading raw diffs for reordering credit. Banked as memory/grind/func_8002D780/rejected/permuter-best-110-find-engine-8.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 8cd6da6b (-mel -msoft-float), the s3 7-floor candidate.c body spliced into src/code6cac_b.c (sandbox score 7, build_insns 202), no FAKE constructs present

## [s4] The dist copy of residual B survives cse if it is spelled as an embedded assignment inside a condition (if ((u32)(m = dist) < 0x400)) rather than as a plain s32 c = dist; declaration.
- mechanism: cse.c make_regs_eqv folds a single-def copy insn into its source pseudo. An assignment written inside a condition expression is emitted by expand_expr at a different point in the RTL stream and is also a use in a comparison, so the copy might not present as a plain single-def move for make_regs_eqv to fold. This is the spelling decomp-permuter itself converged on in output-110-1, and no prior session had tried it.
- probe: Nine arrangements generated by tmp/grind/func_8002D780/s4/gen_d.py and scored with tmp/grind/func_8002D780/s3/run.ps1 against the candidate.c control — m carrying (asm input + srlv), (asm input only), (srlv only), (asm + srlv + small-path LUT index), (asm + srlv with m assigned in the dist >= 0 condition), (call args only), (small-path LUT + srlv + calls), (small-path LUT + calls), and the semantically-wrong else-arm-assignment form.
- result: All eight well-defined arrangements measured 7 = control at exactly 202 build insns — no move insn materialised at all, so cse folded every one. The ninth form is undefined on the small path and measured 57 at 203 insns (not a codegen result). The permuter's own output-110-1 also stayed at 202 insns, so its score of 8 came from its block restructuring, not from a surviving copy. Twenty-three dist-copy spellings are now dead across s1-s4. Banked as memory/grind/func_8002D780/rejected/embedded-assign-dist-copy-still-folded-7.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 8cd6da6b (-mel -msoft-float), the s3 7-floor candidate.c body spliced into src/code6cac_b.c, no FAKE constructs present

## [s4] decomp-permuter's context stripping (ast_util.extract_fn replaces every other function definition in base.c with a declaration) makes the permuter's compile context unfaithful to the engine sandbox's full-TU build for this function.
- mechanism: GCC 2.7.2 compiles per translation unit and the cc1-first-pass-scheduler-bug memory records that ~1400 lines of preceding declarations changes cc1's behaviour, so losing 60-odd function bodies could plausibly move func_8002D780's codegen.
- probe: tmp/grind/func_8002D780/s4/validate3.sh — regenerate base.c through ast_util twice, once with the full TU (bodies kept) and once through extract_fn (bodies stripped), compile both with the workspace compile.sh, and objdump-diff func_8002D780 out of each.
- result: 207 insns each side; the only differences are branch-target text (relative offsets, normalised by both scorers) and jal relocation symbol names. Stripping is codegen-neutral for this function, so the campaign's null result is a real result about the search and not a workspace artifact.
- verdict: CONFIRMED

## [s5] The carrier for residual B survives cse if it is MULTIPLY-DEFINED across the join, as the byte-matched sibling func_8002D518 does at src/code6cac_b.c:1244-1265.
- mechanism: cse.c make_regs_eqv puts a SINGLE-def copy and its source into one quantity and rewrites every later read to the source register, deleting the copy insn; a second definition makes the pseudo multiply-defined across the join and invalidates the equivalence, so the copy survives to reload and global_alloc lands it in the target's $a0.
- probe: batches D/E/F (19 variants, tmp/grind/func_8002D780/s5/gen_d.py / gen_e.py / gen_f.py), scored with tmp/grind/func_8002D780/s3/run.ps1; diffs tmp/grind/func_8002D780/s5/pairdiff_e1.txt and pairdiff_f4.txt.
- result: 7 -> 5 at exactly 202 insns (variant f4). The `move a0,s1` in the beqz delay slot, the `srlv v0,a0,v1` and the LUT reload all match the target insn-for-insn. Two extra conditions beyond the sibling's shape were load-bearing: the carrier must be DECLARED BEFORE `lzcr` (d1, carrier declared second, = 7 -- the copy folds), and the second definition must sit inside the same `dist >= 0` arm (e1, second def in an `else` arm, = 9 at 204 insns; d3/d7, one def per arm, = 10 at 206).
- verdict: CONFIRMED

## [s5] The target's LZCR frame-slot address is computed BETWEEN the mtc2 group and the swc2 group, which a single asm statement cannot emit.
- mechanism: an "r" operand's setup insn is emitted before the whole asm insn, so `"r"(&sp_var)` on a one-statement island always hoists `addiu $v0,$sp,0x10` above the `mtc2`. Two asm statements put the setup insn between them.
- probe: batch G (4 variants, tmp/grind/func_8002D780/s5/gen_g.py) on the f4 chassis
- result: 5 -> 2 at 202 insns (g1/g2/g3). The whole sqrt region now matches the target insn-for-insn. Clobber placement across the two statements is free ($12 then $12-$15, the reverse, or both = 2) but dropping $13-$15 from both statements costs 4 (g4 = 6).
- verdict: CONFIRMED

## [s5] Declaring the six test-3 difference locals in the floor-holding order while ASSIGNING az after dx gives the target's emission order without the dz/dx seat flip.
- mechanism: expand_decl assigns DECL_RTL for each local at block entry, so declaration order was expected to set the pseudo numbers that local-alloc's quantity numbering follows, while statement order sets INSN_LUID and hence sched1's tie-break -- decoupling them should let one form satisfy both.
- probe: batch A (8 variants, tmp/grind/func_8002D780/s5/gen_a.py): az declared in place and assigned last; all six split into declarations plus assignments in the candidate order and in the az-last order; az between dz and dx; declarations in the dx-first order with candidate statement order; az declared last but assigned first; the whole b-vector assigned late.
- result: the seat permutation follows the STATEMENT order alone. Every form whose az statement follows dx's measured 14 (a1/a3/a4/a5/a8), and every form whose az statement precedes dx's measured 7 = control no matter how the declarations were reshuffled (a2/a6/a7). Banked as rejected/decl-stmt-decoupled-seats-unchanged-7.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 994d8b2d (-mel -msoft-float), the s3/s4 7-floor candidate.c body spliced into src/code6cac_b.c, no FAKE constructs present

## [s5] Re-audit of the s3 instance kill on the fully-inlined test-3 forms (killed at 26/27 on the s2 10-floor body): one of them reaches the target's test-3 emission order AND seats on the newer chassis.
- mechanism: chassis-relative kills must be re-measured when the chassis changes materially, and the tail/slot-operand changes of s3 altered register pressure across the whole function.
- probe: batches B and C (19 variants, gen_b.py / gen_c.py) on the 7-floor chassis, plus batches H/I/J (20 variants) re-run on the 2-floor chassis after residual B closed.
- result: the fully-inlined EDGE-first form (b3/h5) is at the floor (7, then 2) but mis-orders the OTHER subu pair -- it emits dz before ax where the target has ax then dz. The fully-inlined ORIGIN-first form (b1) is the only measured form that gets BOTH pairs in the target's emission order, and it permutes dz/dx between $a0 and $v1 and repermutes tests 1-2 (23). Partial-local hybrids: ax/bx local with the edges inlined 9, az/bz local 4, dz/dx local with the origin components inlined 2, four locals with the edges inlined 2. Fresh test-3 cross locals 2, four further declaration orders 2, kp-assigned-first 35, az as a negated reverse difference 11.
- result-summary: 34 forms across s3 and s5 now agree that the target's emission order and the target's dz/dx seats are mutually exclusive on this chassis. Banked as rejected/inlined-origin-first-seat-flip-23.c and rejected/test3-dx-before-az-seat-flip-9.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 994d8b2d (-mel -msoft-float), the s3 7-floor body for batches B/C and the s5 2-floor body (one FAKE same-value re-store of the local `m` present) for batches H/I/J

## Live frontier (for s6) -- 2 insns left, all of them residual A
1. **Residual A, 2 insns -- the test-3 subu pair (dx before az) vs the dz/dx seats.**
   This is now the ONLY differing region: the sqrt block, the tail, tests 1 and 2 and
   the whole prologue match insn-for-insn. Thirty-four forms show the two properties to
   be mutually exclusive under source rearrangement. Next probe: this is exactly what
   `tools/ra_solver/inverse_compose.py classify` exists for -- run it on the dz and dx
   seats against BOTH 2-floor bodies (candidate.c, which has the seats right and the
   order wrong, and tmp/grind/func_8002D780/s5/variantsH/h2_ax_dx_dz_az.c, which has the
   order right and the seats wrong) and let it name the C-lever vector; also worth a
   `tools/sched_solver` order-exact pass on the test-3 block, since the pair is a
   priority-1 same-unit tie decided by schedule_select's largest-potential-hazard rule
   (tools/gcc-2.7.2/sched.c:2708-2723) and a dependence change -- not a statement
   reorder -- may break the tie the other way without touching allocation.
2. **A permuter campaign is now worth re-running.** The s4 campaign explored the
   7-floor chassis, whose search neighbourhood was dominated by the two dead residuals;
   the 2-floor chassis is a completely different neighbourhood and the workspace already
   exists and is validated (tmp/grind/func_8002D780/s4/nonmatchings/func_8002D780, plus
   the four hurdles documented in evidence.md [s4]). Regenerate base.c from the new
   candidate.c first. Note the s4 finding that the permuter's weighted metric is
   anti-correlated with the engine's here: re-score every find on the real sandbox.
3. **Family decision before any candidate-ready.** The body now carries ONE
   FAKE-annotated construct (the same-value re-store of the local `m`, dead-store
   family, .claude/rules/dead-store-fake-exception.md, in-TU byte-matched precedent at
   src/code6cac_b.c:1244-1265) plus the six test-3 difference locals whose
   named-intermediate-vs-ordinary-C classification the s3 frontier flagged. Both must be
   written into memory/grind/func_8002D780/self_vet.md BEFORE submitting, because review
   verdicts are keyed by body and respelling after a layer-1 verdict is not available.

## [s5] The carrier for residual B survives cse if it is MULTIPLY-DEFINED across the join, as the byte-matched sibling func_8002D518 does at src/code6cac_b.c:1244-1265.
- mechanism: cse.c make_regs_eqv puts a SINGLE-def copy and its source into one quantity and rewrites every later read to the source register, so the copy insn dies; a second definition makes the pseudo multiply-defined across the join, invalidates that equivalence, and the copy survives to reload, where global_alloc lands it in the target's $a0. All 23 copy spellings killed across s1-s4 were single-def, which is why every one of them folded.
- probe: Batches D/E/F, 19 variants (tmp/grind/func_8002D780/s5/gen_d.py, gen_e.py, gen_f.py), scored with tmp/grind/func_8002D780/s3/run.ps1; diffs tmp/grind/func_8002D780/s5/pairdiff_e1.txt and pairdiff_f4.txt.
- result: 7 -> 5 at exactly 202 insns (variant f4_carrier_decl_first_restore_in_arm). The target's `move a0,s1` in the beqz delay slot, `srlv v0,a0,v1` and the LUT reload all match insn-for-insn. Two conditions beyond the sibling's shape are load-bearing and cost two batches to find: (1) the carrier must be DECLARED BEFORE `lzcr` -- with `s32 lzcr = 0;` first the copy folds again (d1 = 7); declaration order decides which of the two stores reorg.c steals into the beqz delay slot; (2) the second definition must sit inside the same `dist >= 0` arm -- in an `else` arm the copy survives (first materialisation of `move a0,s1` in five sessions, e1) but costs an extra `j` plus a duplicated `li v0,-2` (204 insns, score 9), and one definition per arm emits both copies (205/206 insns, score 10, d2/d3/d5/d7/e4). Asm-input-only use of the carrier folds back to 7 (d4).
- verdict: CONFIRMED

## [s5] The target's LZCR frame-slot address is computed BETWEEN the mtc2 group and the swc2 group, which one asm statement cannot emit, so the LZCS island is two asm statements.
- mechanism: An "r" operand's setup insn is always emitted before the whole asm insn, so `"r"(&sp_var)` on the one-statement island hoists `addiu $v0,$sp,0x10` above the `mtc2`. Splitting into a mtc2 island and a swc2 island whose "r" operand is &sp_var puts the setup insn between them, exactly as asm/funcs/func_8002D780.s L141-147 has it.
- probe: Batch G, 4 variants (tmp/grind/func_8002D780/s5/gen_g.py) on the f4 chassis.
- result: 5 -> 2 at 202 insns (g1/g2/g3). The whole sqrt region now matches the target insn-for-insn; the remaining diff for the function is 2 instructions, both in test 3. Clobber placement across the two statements is free ($12 then $12-$15 = 2, reversed = 2, both = 2) but dropping $13-$15 from both statements costs 4 (g4 = 6), the same reload1.c bad_spill_regs effect the s2 sibling clobber footprint buys.
- verdict: CONFIRMED

## [s5] Declaring the six test-3 difference locals in the floor-holding order while assigning az after dx gives the target's emission order without the dz/dx seat flip.
- mechanism: expand_decl assigns DECL_RTL for each local at block entry, so declaration order was expected to set the pseudo numbers local-alloc's quantity numbering follows while statement order sets INSN_LUID and hence sched1's tie-break; decoupling them should satisfy both at once.
- probe: Batch A, 8 variants (tmp/grind/func_8002D780/s5/gen_a.py): az declared in place and assigned last; all six split into declarations plus assignments in the candidate order and in the az-last order; az between dz and dx; declarations in dx-first order with candidate statement order; az declared last but assigned first; the b-vector assigned late.
- result: The seat permutation follows the STATEMENT order alone. Every form whose az statement follows dx's measured 14 (a1/a3/a4/a5/a8); every form whose az statement precedes dx's measured 7 = control however the declarations were reshuffled (a2/a6/a7). Banked as memory/grind/func_8002D780/rejected/decl-stmt-decoupled-seats-unchanged-7.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 994d8b2d (-mel -msoft-float), the s3/s4 7-floor candidate.c body spliced into src/code6cac_b.c, no FAKE constructs present

## [s5] Kill re-audit: one of the fully-inlined or partial-local test-3 forms killed at 26/27 on the s2 10-floor body reaches the target's test-3 emission order and the target's dz/dx seats on the newer chassis.
- mechanism: Chassis-relative kills must be re-measured when the chassis changes materially, and s3's tail and slot-operand changes plus s5's carrier and split-island changes altered register pressure across the whole function.
- probe: Batches B and C (19 variants, gen_b.py / gen_c.py) on the 7-floor chassis, then batches H, I and J (20 variants, gen_h.py / gen_i.py / gen_j.py) re-run on the 2-floor chassis after residual B closed; diff tmp/grind/func_8002D780/s5/pairdiff_b3_inlined_edge_first_term_first.txt.
- result: The fully-inlined EDGE-first form (b3/h5) sits at the floor (7, then 2) but mis-orders the OTHER subu pair (it emits dz before ax where the target has ax then dz), so it trades one 2-insn residual for another. The fully-inlined ORIGIN-first form (b1) is the only measured form that puts BOTH pairs in the target's emission order, and it pays with the dz/dx seat flip (dz->$v1, dx->$a0) plus an upstream permutation in tests 1-2, scoring 23. Partial-local hybrids: ax/bx local with the edges inlined 9, az/bz local 4, dz/dx local with the origin components inlined 2, four origin locals with the edges inlined 2. Fresh test-3 cross locals 2, four further declaration orders 2, kp-assigned-first 35, az as a negated reverse difference 11. Banked as rejected/inlined-origin-first-seat-flip-23.c and rejected/test3-dx-before-az-seat-flip-9.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 994d8b2d (-mel -msoft-float); batches B/C on the s3 7-floor body with no FAKE constructs present, batches H/I/J on the s5 2-floor body with one FAKE same-value re-store of the local `m` present

## [s6] The residual-A seat flip is decided by local_alloc's qty_compare_1 priority (floor_log2(n_refs)*n_refs/live_length), and in the target's emission order dz and dx tie on both inputs.
- mechanism: dump_flow_info at the head of the .lreg dump prints qty_compare_1's two inputs per pseudo. Candidate order: dz 3 refs / 7 insns, dx 3 refs / 6 insns -> dx first -> $v1 (target seats). h1 order: both 3 refs / 7 insns -> the `*q1 - *q2` quantity-number fallback picks dz -> $v1 to dz (seats inverted).
- probe: pwsh tools/grinder/dump.ps1 func_8002D780 on both bodies; slice func_8002D780 out of code6cac_b.lreg (tmp/grind/func_8002D780/s6/slice.py) and read the `Register N used K times across M insns in block 7` lines.
- result: CONFIRMED. dumpsA/lreg.txt L111-113 (dz=133: 3/7, dx=134: 3/6) vs dumpsH1/lreg.txt L105-107 (dz=130: 3/7, dx=131: 3/7). h1 scores 9 with 14 differing insns (s6/pairdiff_h1.txt) and the diff is exactly the dz/dx seat swap and its downstream mflo permutation.
- verdict: CONFIRMED

## [s6] An anti-dependence (C-level variable reuse) on the preceding mult can push az into rank_for_schedule's class 2 so that dx wins the ready-list tie in the candidate's statement order.
- mechanism: rank_for_schedule (sched.c:2408-2464) prefers the highest dependence class w.r.t. last_scheduled_insn; class 2 = anti/output dependence with insn_cost > 1.
- probe: read ADJUST_COST for the target back end and insn_cost's handling of its result before spelling any reuse form.
- result: KILLED at the source. config/mips/mips.h:2946 defines ADJUST_COST to set COST = 0 for every dependence whose REG_NOTE_KIND != 0 (anti or output); insn_cost (sched.c:1409-1414) sees the adjusted cost <= 1, sets LINK_COST_FREE and returns 1; rank_for_schedule's `link == 0 || insn_cost (...) == 1` test then classifies the insn 3, identical to an independent insn. The class tie-break is unreachable by any WAR/WAW dependence on this target.
- verdict: KILLED
- kill_scope: class
- predicate_cite: tools/gcc-2.7.2/config/mips/mips.h:2946
- measured_on: read from the pinned toolchain source (tools/gcc-2.7.2), toolchain-invariant

## [s6] Swapping the product operand order, or negating both cross products (semantics-preserving under `(kc ^ kp) >= 0`), reaches the target's test-3 subu order.
- mechanism: the mult's rtx operand order feeds sched_analyze's dependence construction and local-alloc's scan order.
- probe: batch K, 11 variants on the 2-floor chassis (tmp/grind/func_8002D780/s6/gen_k.py, variantsK).
- result: KILLED. k1/k3 = 17, k2 = 19, k4 = 32, k5 = 43, k6 = 24, k8 = 33, k9 = 23; k7 (product locals) = 2 and k10 (dx inlined in both products) = 2 are byte-neutral alternates, not improvements.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 06451cd2 (-mel -msoft-float), the s5 2-floor candidate.c body spliced into src/code6cac_b.c, one FAKE construct present (the `m` re-store)

## [s6] Reordering the four test-3 mults so that dx dies before dz shortens dx's live range enough to break the qty_compare_1 tie in the target's emission order.
- mechanism: qty priority is n_refs-weighted and divided by (qty_death - qty_birth); moving the dx*bz product ahead of dz*bx makes dx die at the 7th insn of the block instead of the 9th.
- probe: batch L, 11 variants (named product locals p1/p2/q1/q2 in four orders, and a single `q2 = dx * bz` local, each against three difference-local statement orders).
- result: KILLED. 9, 25, 25, 32, 32, 32, 32, 33, 36, 36. Reordering the products necessarily reorders the bx/bz subus that feed them, so the two insns the seat would win are paid for four to seventeen times over.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 06451cd2 (-mel -msoft-float), the s5 2-floor candidate.c body, one FAKE construct present (the `m` re-store)

## [s6] Giving dx a fourth reference inside the block (carrying the point-side cross product kp in the dx variable) raises its qty priority past dz's, since floor_log2 steps at 4 refs.
- mechanism: qty_compare_1's numerator is floor_log2(n_refs)*n_refs, so 3 -> 4 refs takes it from 3 to 8 while the live length grows only from 7 to 10; the target itself shares $v1 between dx, the dx*bz product and kp (asm/funcs/func_8002D780.s L107/L119/L120).
- probe: batch N, 7 variants (kp carried in dx / bz / bx / dz, kc carried in az, against three statement orders).
- result: KILLED. n1/n2/n3 = 16 (identical for all three statement orders), n4 = 41, n5 = 46, n6 = 10, n7 = 10. The second definition merges kp into dx's quantity, and the cost at the join with the two earlier tests' kp exceeds the two insns the seat is worth.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 06451cd2 (-mel -msoft-float), the s5 2-floor candidate.c body, one FAKE construct present (the `m` re-store)

## [s6] Carrying dz or dx in a local that is already live in an earlier block (x2 / z2) takes it out of local-alloc entirely, so the qty_compare_1 tie never arises and global.c assigns the seat.
- mechanism: block_alloc only allocates pseudos whose live range lies inside one basic block; a multi-block pseudo is left to global_alloc, whose priority function is different.
- probe: batch O, 8 variants (dz in z2, dx in x2, both, each against the candidate and h1 statement orders, plus two split-declaration controls).
- result: KILLED. o1/o2 = 23, o3/o4 = 38, o5/o6 = 40 -- and each pair is IDENTICAL across the two statement orders, which independently confirms that the seat half of residual A belongs to local-alloc. Controls: o7 (h1, split decl/assign) = 9, o8 (candidate, split decl/assign) = 2, so splitting declaration from assignment is byte-neutral and is available free to future probes.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 06451cd2 (-mel -msoft-float), the s5 2-floor candidate.c body, one FAKE construct present (the `m` re-store)

## [s6] The s5 frontier's second 2-floor body (variantsH/h2, "order right, seats wrong") exists.
- mechanism: n/a -- a ledger claim inherited from s5's frontier text.
- probe: re-scored h1 and h2 on this chassis and pairdiffed h1.
- result: KILLED. Both h1 and h2 score 9 at 202 insns; h1's pairdiff shows 14 differing instructions (the dz/dx seat swap plus its downstream mflo/subu permutation). There is exactly one 2-floor family, the candidate's.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 06451cd2 (-mel -msoft-float), s5 variantsH bodies spliced into src/code6cac_b.c

## Live frontier (for s7) -- 2 insns left, all of them residual A, now fully attributed

1. **Raise dx's INSN_PRIORITY above az's in the CANDIDATE statement order (the only route
   left that keeps the winning seats).** Residual A is a two-sided constraint and s6
   proved both sides: the candidate order gives the target's SEATS (dx live 6, dz 7) but
   loses the sched1 LUID tie-break; every order that wins the tie-break ties the live
   lengths and loses the seats. The one axis that moves sched1 WITHOUT moving statement
   order is INSN_PRIORITY: rank_for_schedule tests priority BEFORE it falls through to
   LUID. All six subus are priority 1 only because every operand is computed in an
   earlier block; if one of dx's operands were produced inside block 7 by an insn with
   result_ready_cost > 1 (a load is the obvious candidate: priority(dx) = 1 + 2 - 1 = 2),
   dx would outrank az and be scheduled first with the candidate's statement order
   untouched. Next probe: find a spelling in which the x2 (or x0) LOAD lands inside the
   test-3 block without adding an insn -- e.g. read the vertex-2 x through the object at
   its two use sites (test 2 and test 3) instead of into an x2 local, and check in the
   .cse/.combine dumps whether the second read survives as its own load or is folded back
   to the test-2 pseudo. Keep build_insns == 202: if the earlier load stays live for test
   2 the body goes to 203 and the probe is dead, so pair it with a form where the test-2
   use is the one that reads through the object.
2. **Break the qty_compare_1 tie from the n_refs side without merging quantities.** The
   s6 batch-N failure was not the priority arithmetic -- it was that carrying kp in dx
   MERGES kp's cross-block quantity into dx's. A fourth reference to dx that stays inside
   block 7 and does not extend any other value's range would still work. Next probe:
   enumerate the block-7 values that die before dx is born or are born after dx dies
   (from s6/dumpsA/lreg.txt: ax dies at the first mult, az at the second, and the kc subu
   is born two insns after dx dies), and test a carrier whose OTHER uses are all inside
   block 7 -- the kc subtraction is the only candidate, and it was never measured (n6
   carried kc in az, not in dx).
3. **A decomp-permuter campaign on the 2-floor chassis** (inherited unspent from s5, and
   now better motivated: s6 has shown the residual is a single local-alloc tie, which is
   exactly the neighbourhood the permuter's statement-reordering passes explore). Next
   probe: regenerate base.c from candidate.c into the validated workspace
   tmp/grind/func_8002D780/s4/nonmatchings/func_8002D780 (four hurdles + rebuild recipe in
   evidence.md [s4]), launch with tools/permuter_campaign.py, wait IN-TURN, harvest with
   --stop, and re-score every find on the engine sandbox (the s4 finding that the
   permuter's weighted metric is anti-correlated with the engine's here still applies).
   NOTE: the family classification of the body (frontier item 3 from s5) is still unspent
   and must be settled before any candidate-ready submission.

## [s6] The residual-A seat/order coupling is decided by local_alloc's qty_compare_1 priority (floor_log2(n_refs)*n_refs*size / (qty_death - qty_birth)), and in the target's emission order the dz and dx quantities tie on both inputs.
- mechanism: dump_flow_info at the head of the .lreg dump prints qty_compare_1's two inputs per pseudo. In the 2-floor candidate body dz is 3 refs across 7 insns and dx is 3 refs across 6 insns, so dx's priority is strictly higher, dx is allocated first and takes $v1 under the default REG_ALLOC_ORDER ($v0 is already conflicted by ax/az/bx/bz) while dz takes $a0 -- the target's seats. In the h1 body (dx's statement moved ahead of az's) sched1 emits the target's subu order but dz and dx are BOTH 3 refs across 7 insns, an exact tie, so qty_compare_1 falls through to its quantity-number fallback, which favours the quantity born first (dz), and the seats invert.
- probe: pwsh tools/grinder/dump.ps1 func_8002D780 on the candidate body and on tmp/grind/func_8002D780/s5/variantsH/h1_ax_dz_dx_az.c; slice func_8002D780 out of code6cac_b.lreg with tmp/grind/func_8002D780/s6/slice.py and read the 'Register N used K times across M insns in block 7' lines; pairdiff both bodies.
- result: CONFIRMED. tmp/grind/func_8002D780/s6/dumpsA/lreg.txt L111-113 (dz=133: 3 refs/7 insns, dx=134: 3 refs/6 insns) vs s6/dumpsH1/lreg.txt L105-107 (dz=130: 3/7, dx=131: 3/7). The candidate scores 2 with the 2-insn subu-order residual; h1 scores 9 with 14 differing instructions whose content is exactly the dz/dx seat swap and its downstream mflo permutation (s6/pairdiff_h1.txt). Also measured on the scheduler side: all six test-3 difference subus carry INSN_PRIORITY 1 (s6/dumpsA/sched.txt L248-258) because every one of their operands is computed in an earlier basic block, so rank_for_schedule falls straight through to the INSN_LUID tie-break -- which is why 34 source rearrangements across s3/s5 could only trade the emission order against the seats.
- verdict: CONFIRMED

## [s6] An anti-dependence or output-dependence introduced from C (variable reuse) changes rank_for_schedule's dependence-class tie-break, so az can be demoted below dx while the candidate's statement order is kept.
- mechanism: rank_for_schedule (tools/gcc-2.7.2/sched.c:2408-2464) classifies each ready insn against last_scheduled_insn as class 1 (data dependence, cost > 1), class 2 (anti/output dependence, cost > 1) or class 3 (independent, or cost 1), and prefers the highest class before falling through to INSN_LUID.
- probe: Read the target back end's ADJUST_COST and insn_cost's handling of the adjusted value before spelling any reuse form.
- result: KILLED at the source. tools/gcc-2.7.2/config/mips/mips.h:2946 defines ADJUST_COST to set COST = 0 whenever REG_NOTE_KIND (LINK) != 0, i.e. for every anti- and output-dependence; insn_cost (sched.c:1409-1414) then sets LINK_COST_FREE and returns 1, and rank_for_schedule's 'link == 0 || insn_cost (...) == 1' test classifies the insn 3 -- identical to an independent insn. The class tie-break cannot be reached by a WAR/WAW dependence on this back end, so no C form whose only scheduling effect is such a dependence can move it.
- verdict: KILLED
- kill_scope: class
- measured_on: read from the pinned toolchain source under tools/gcc-2.7.2 (back-end macro, toolchain-invariant); no FAKE construct involved
- predicate_cite: tools/gcc-2.7.2/config/mips/mips.h:2946

## [s6] Swapping the product operand order (dx * az -> az * dx), or negating both cross products (kc = dx*az - dz*ax; kp = dx*bz - dz*bx, which preserves the (kc ^ kp) >= 0 test), reaches the target's test-3 subu emission order on this chassis.
- mechanism: The mult's rtx operand order feeds sched_analyze's dependence construction and the order in which local-alloc's block scan first sees each pseudo.
- probe: Batch K, 11 variants generated by tmp/grind/func_8002D780/s6/gen_k.py into s6/variantsK and scored with tmp/grind/func_8002D780/s3/run.ps1 (sandbox func_8002D780 --disable all).
- result: KILLED. k1/k3 = 17, k2 = 19, k4 = 32, k5 = 43, k6 = 24, k8 = 33, k9 = 23, all at 202 insns. Two byte-neutral alternates at the floor came out of the batch and are banked as future chassis: k7 (four named product locals p1/p2/q1/q2) = 2 and k10 (dx inlined as (x2 - x0) in both of its products, no dx local at all) = 2.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 06451cd2 (-mel -msoft-float), the s5 2-floor candidate.c body spliced into src/code6cac_b.c, one FAKE construct present (the same-value re-store of the local m)

## [s6] Reordering the four test-3 multiplies so that dx dies before dz shortens dx's live range enough to break the qty_compare_1 tie while the target's subu emission order is kept.
- mechanism: qty_compare_1's priority divides an n_refs-weighted numerator by (qty_death - qty_birth); moving the dx*bz product ahead of dz*bx makes dx die at the 7th insn of the block instead of the 9th, which would make dx's priority strictly higher than dz's.
- probe: Batch L, 11 variants (four named-product-local orders and a single 'q2 = dx * bz' local, each against three difference-local statement orders), tmp/grind/func_8002D780/s6/gen_l.py -> s6/variantsL.
- result: KILLED. Scores 9, 25, 25, 32, 32, 32, 32, 33, 36, 36 at 202 insns. Reordering the products necessarily reorders the bx/bz subus that feed them, so the two instructions the seat would win are paid for four to seventeen times over.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 06451cd2 (-mel -msoft-float), the s5 2-floor candidate.c body spliced into src/code6cac_b.c, one FAKE construct present (the same-value re-store of the local m)

## [s6] Giving dx a fourth reference inside the test-3 block by carrying the point-side cross product kp in the dx variable raises dx's qty priority past dz's, because floor_log2 steps at 4 references.
- mechanism: qty_compare_1's numerator is floor_log2(n_refs)*n_refs, so 3 -> 4 refs takes it from 3 to 8 while the live length only grows from 7 to 10; the target itself shares $v1 between dx, the dx*bz product and kp (asm/funcs/func_8002D780.s L107, L119, L120), which is what suggested the carrier.
- probe: Batch N, 7 variants (kp carried in dx / bz / bx / dz and kc carried in az, against the candidate, h1 and h2 statement orders), tmp/grind/func_8002D780/s6/gen_n.py -> s6/variantsN.
- result: KILLED. n1/n2/n3 = 16 -- identical for all three statement orders -- n4 = 41, n5 = 46, n6 = 10, n7 = 10, all at 202 insns. The second definition merges kp's cross-block quantity into dx's, and the cost at the join with the two earlier tests' kp exceeds the two instructions the seat is worth.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 06451cd2 (-mel -msoft-float), the s5 2-floor candidate.c body spliced into src/code6cac_b.c, one FAKE construct present (the same-value re-store of the local m)

## [s6] Carrying dz or dx in a local that is already live in an earlier block (the vertex-2 coordinates x2 / z2) takes that value out of local-alloc, so the qty_compare_1 tie never arises and global.c assigns the seat instead.
- mechanism: block_alloc only allocates pseudos whose live range lies inside a single basic block; a multi-block pseudo is left to global_alloc, whose priority function is different.
- probe: Batch O, 8 variants (dz in z2, dx in x2, both, each against the candidate and h1 statement orders, plus two split-declaration controls), tmp/grind/func_8002D780/s6/gen_o.py -> s6/variantsO.
- result: KILLED. o1/o2 = 23, o3/o4 = 38, o5/o6 = 40 at 202 insns -- and each pair scores IDENTICALLY across the two statement orders, which independently confirms that the seat half of residual A belongs to local-alloc rather than to sched1. The controls are useful: o7 (h1 order, split declaration/assignment) = 9 and o8 (candidate order, split declaration/assignment) = 2, so declaring the six difference locals uninitialised and assigning them in the same order is byte-neutral and is available free to future probes.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 06451cd2 (-mel -msoft-float), the s5 2-floor candidate.c body spliced into src/code6cac_b.c, one FAKE construct present (the same-value re-store of the local m)

## [s6] The s5 frontier's second 2-floor body -- tmp/grind/func_8002D780/s5/variantsH/h2_ax_dx_dz_az.c, described there as 'order right, seats wrong' at the floor -- scores 2 on this chassis.
- mechanism: n/a: a ledger claim inherited from the s5 frontier text, re-measured before it could be spent on an s6 probe.
- probe: Re-scored h1 and h2 with tmp/grind/func_8002D780/s5/pd.ps1 and pairdiffed h1 through tools/pairdiff.py.
- result: KILLED. Both h1 and h2 score 9 at 202 insns; h1's pairdiff shows 14 differing instructions (tmp/grind/func_8002D780/s6/pairdiff_h1.txt). There is one 2-floor family on this chassis, the candidate's, and an s7 plan built around a complementary 2-floor body would be built on a wrong inherited number.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 06451cd2 (-mel -msoft-float), s5 variantsH bodies spliced into src/code6cac_b.c, one FAKE construct present (the same-value re-store of the local m)

## [s7] The residual is a pure emission-order divergence, and the scheduler half of it is reachable from the source statement order alone.
- mechanism: goal_from_tgt.py's object-level classifier aligns our 202 instruction texts against the target's and reports the first pass at which the two streams diverge; sched_solver/simulate.py then replays sched.c's backward list scheduler for the block (validated 6978/6978 blocks, and self-checked here against this function's own dump).
- probe: `goal_from_tgt.py classify code6cac_b func_8002D780`; `sched_solver/extract.py code6cac_b`; `inverse_sched.py --block 7 --pass 1 --goal-order 179,191,197,194,182,199,185,204,188,206,201,208,210,212`; then tmp/grind/func_8002D780/s7/sim_luid.py over all 720 permutations of the six difference-local statements.
- result: CONFIRMED. classify prints `FIRST DIVERGENCE: SCHED -- texts match as a multiset, order differs`. inverse_sched returns NEGATIVE at depth 2 and perturb.py --atoms luid,luid_move at depth 3 also returns negative, but the direct simulation shows that is only a depth statement: 180 of the 720 statement permutations reach the target's emission order exactly, h1 (ax,dz,dx,az,bx,bz) among them. The scheduler half needs no further search.
- verdict: CONFIRMED

## [s7] On the target's own emission order the dz and dx quantities tie on every qty_compare_1 input, so local-alloc.c:1684's qty-number tie-break decides the seat and hands $v1 to whichever subu the target emits first (dz).
- mechanism: block_alloc sorts quantities by floor_log2(n_refs)*n_refs*size/(death-birth) and, when that is equal, by qty number; qty numbers are handed out in birth order by the forward block scan. The suggested-register pass (local-alloc.c:1509) runs first but only for quantities with a nonzero sugg/copysugg count.
- probe: `local_extract.py code6cac_b --func func_8002D780 --suggest` on the h1 body (the body whose sched1 output IS the target's stream), reading the complete BB2_SUGG_DEBUG table in tmp/ra_solver_work/code6cac_b.sugg.json block 7.
- result: CONFIRMED. qty 1 (dz, pseudo 130) = birth 4, death 16, refs 3, size 1, nsugg 0, ncopysugg 0; qty 4 (dx, pseudo 131) = birth 8, death 20, refs 3, size 1, nsugg 0, ncopysugg 0. Identical priority, so the comparator returns *q1 - *q2 and dz (lower qty) is allocated first. Since all 180 order-reaching permutations produce the identical scheduled stream, they all present this same tied table -- measured h1 = 9, p1 = 9, p7 = 9.
- verdict: CONFIRMED

## [s7] Six independent block-local difference pseudos, arranged into the target's sched1 emission order, produce the target's dz/dx register seats.
- mechanism: refs and live span are read by local-alloc off the scheduled stream; if that stream is the target's, both quantities necessarily have 3 refs and a 12-unit span, and neither carries a register suggestion, so the qty_compare_1 tie-break at local-alloc.c:1684 fixes the seat by birth order.
- probe: the BB2_SUGG_DEBUG table above, plus the exhaustive 720-permutation simulation, plus three direct measurements of order-reaching bodies (h1, p1, p7).
- result: KILLED. All three order-reaching bodies score 9 with the seats inverted, and the input table shows why: the two quantities are indistinguishable to the comparator. The corollary is the useful part -- the original compile must have presented local-alloc with an extra reference, an extra quantity, or a copy that is absent from the final 202 instructions (reload, flow2 delete_noop_moves and reorg all run after local-alloc).
- verdict: KILLED
- kill_scope: class
- measured_on: HEAD b3a8a19a (-mel -msoft-float), the s5 candidate body and the s5 h1 body spliced into src/code6cac_b.c, one FAKE construct present (the same-value re-store of the local m)
- predicate_cite: tools/gcc-2.7.2/local-alloc.c:1684

## [s7] The inverse RA solver's top spellable vector -- a fourth in-block reference to dx -- can be spelled by carrying the test-3 kp in dx once the test-3 cross products get FRESH block-local names (the repair of the s6 batch-N failure, which merged the outer cross-block kp into dx).
- mechanism: qty_compare_1's numerator steps at floor_log2(4)*4 = 8, so a fourth reference lifts dx's priority clear of dz's even with the longer span; local-alloc.c:1509's suggestion pass is not involved.
- probe: batch P (tmp/grind/func_8002D780/s7/gen_p.py -> s7/variantsP), 8 variants: fresh test-3 locals with kp carried in dx / in dz / not carried, against the h1 and candidate statement orders, plus outer-kc controls.
- result: KILLED. p2 = 28 and p3 = 28 (fresh c3 local, kp carried in dx, h1 and candidate orders); p4 = 16 and p8 = 16 (outer kc kept, reproducing batch N's number); p5 = 30 (kp carried in dz). Re-extracting the local-alloc table from p2 shows the reuse also moved the SCHEDULE -- dx's quantity is gone (merged with kp at birth 24) and dz's death moved 16 -> 10 -- so the variable-reuse spelling of a refs_up atom is not stream-neutral here. Byte-neutral by-product: giving test 3 its own cross-product locals costs nothing (p1 = 9 = p7 on the h1 order, p6 = 2 = the candidate on the candidate order).
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD b3a8a19a (-mel -msoft-float), the s5 candidate body spliced into src/code6cac_b.c, one FAKE construct present (the same-value re-store of the local m)

## [s7] A plain `dx2 = dx` copy makes local-alloc's combine_regs merge the two pseudos into one quantity whose n_refs is the sum, lifting dx clear of dz, while the copy itself ends as a same-register no-op that flow2's delete_noop_moves removes.
- mechanism: local-alloc combines the quantities of a copy whose source dies at the copy, summing qty_n_refs; a copy between two pseudos that end up in the same hard register is deleted after reload, so the final stream can stay at 202 instructions.
- probe: batch Q (tmp/grind/func_8002D780/s7/gen_q.py -> s7/variantsQ), 6 variants: the copy placed before the products, between them, feeding one product or both, and a dz-side control, against the h1 and candidate statement orders.
- result: KILLED. Every placement is byte-neutral -- q1/q3/q4/q5 = 9 on the h1 order, q2/q7 = 2 on the candidate order, all at 202 insns. cse.c make_regs_eqv puts the copy's source and destination into one quantity and deletes the copy insn long before local-alloc runs, which is the identical mechanism s5 documented for residual B. The route survives only with a MULTIPLY-DEFINED carrier, and this straight-line block offers no control-flow arm to hold the second definition. Useful by-product: a single-def copy of dz or dx is a free (byte-neutral) carrier slot on both statement orders.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD b3a8a19a (-mel -msoft-float), the s5 candidate body spliced into src/code6cac_b.c, one FAKE construct present (the same-value re-store of the local m)

## Live frontier (for s8) -- still 2 insns, and the search is now typed on both passes

1. **Find the extra local-alloc-time reference/quantity that the final 202 instructions do
   not show.** This is the corollary of the s7 class kill: with the target's emission order
   fixed, dz and dx tie on every comparator input, so the original compile must have had
   something MORE in block 7 at local-alloc time that was removed afterwards. Only three
   passes remove instructions after local_alloc: reload/flow2 `delete_noop_moves` (a copy
   whose two pseudos land in the same hard register), reorg's delay-slot steal + own-thread
   delete, and jump2/cross-jumping. The delay-slot route is already spoken for (`subu ax`
   fills the test-2 `bltz` slot). The no-op-move route is the one to attack, and s7 showed
   the blocker precisely: cse folds every SINGLE-DEF copy. Next probe: give the copy carrier
   a second definition the way s5's `m` carrier gets one -- e.g. hoist the test-3 difference
   computation so the carrier is assigned in the test-2 arm and re-assigned in the test-3
   block, and check tmp/grind/func_8002D780/dumps/code6cac_b.cse for whether the copy insn
   survives into .lreg before scoring anything.
2. **Spell the `[extra_qty] one more temp: refs=2 span=[2,4) pri=10000` vector.** It is the
   only 1-atom vector in the inverse.py report that does not touch dz's or dx's own
   columns: an additional short-lived quantity born at the top of block 7 changes what
   find_free_reg has already handed out by the time the tied pair is reached. It was not
   spelled this session. Next probe: a seventh difference local computed at the top of the
   test-3 block whose value is genuinely consumed (so it is not a dead store) -- the obvious
   candidate is splitting one of the four multiply operands into a named intermediate --
   measured against the h1 order, checking build_insns stays 202.
3. **A decomp-permuter campaign on the 2-floor chassis** (inherited unspent from s5/s6, and
   now sharper: s7 has typed the residual as a local-alloc tie whose only remaining C-side
   inputs are an extra quantity or a surviving copy, both of which are exactly the
   temporary-introduction moves the permuter's pass set explores). Next probe: regenerate
   base.c from candidate.c into the validated workspace
   tmp/grind/func_8002D780/s4/nonmatchings/func_8002D780 (four hurdles + rebuild recipe in
   evidence.md [s4]), launch with tools/permuter_campaign.py, wait IN-TURN, harvest with
   --stop, re-score every find on the engine sandbox. The family classification of the
   body (the FAKE `m` re-store plus the six difference locals) is still unsettled and must
   be recorded before any candidate-ready submission.

## [s7] The residual is a pure emission-order divergence, and the scheduler half of it is reachable from the source statement order alone.
- mechanism: goal_from_tgt.py's object-level classifier aligns our 202 instruction texts against the target's and reports the first pass at which the streams diverge; sched_solver/simulate.py then replays sched.c's backward list scheduler for the block (validated 6978/6978 blocks, and self-checked here against this function's own dump: 'model self-check: sim == dump (err=None)').
- probe: goal_from_tgt.py classify code6cac_b func_8002D780; sched_solver/extract.py code6cac_b -> s7/model.json; inverse_sched.py --func func_8002D780 --pass 1 --block 7 --goal-order 179,191,197,194,182,199,185,204,188,206,201,208,210,212 --depth 2; perturb.py --atoms luid,luid_move --depth 3; then tmp/grind/func_8002D780/s7/sim_luid.py over all 720 permutations of the six difference-local statements.
- result: CONFIRMED. classify prints 'FIRST DIVERGENCE: SCHED -- texts match as a multiset, order differs'. inverse_sched is NEGATIVE at depth 2 and the luid-only search is negative at depth 3, but the direct simulation shows that is only a depth statement: 180 of the 720 permutations reach the target's emission order exactly, and the s5 h1 body (ax,dz,dx,az,bx,bz) is one of them. No further scheduler search is warranted on this block.
- verdict: CONFIRMED

## [s7] On the target's own sched1 emission order the dz and dx quantities tie on every qty_compare_1 input, so local-alloc.c:1684's qty-number tie-break decides the seat and hands $v1 to the subu the target emits first (dz).
- mechanism: block_alloc sorts quantities by floor_log2(n_refs)*n_refs*size/(death-birth) and, when that is equal, by qty number; qty numbers are handed out in birth order by the forward block scan. The suggested-register pass (local-alloc.c:1509) runs first, but only for quantities with a nonzero sugg/copysugg count.
- probe: local_extract.py code6cac_b --func func_8002D780 --suggest on the h1 body (the body whose sched1 output IS the target's stream); read the complete BB2_SUGG_DEBUG table for block 7 (tmp/grind/func_8002D780/s7/sugg_block7.json).
- result: CONFIRMED. qty 1 (dz, pseudo 130) = birth 4, death 16, refs 3, size 1, nsugg 0, ncopysugg 0; qty 4 (dx, pseudo 131) = birth 8, death 20, refs 3, size 1, nsugg 0, ncopysugg 0 -- identical priority, so the comparator returns *q1 - *q2 and dz is allocated first and takes $v1. All 180 order-reaching permutations produce the identical scheduled stream and therefore this same tied table: h1 = 9, p1 = 9, p7 = 9.
- verdict: CONFIRMED

## [s7] Six independent block-local difference pseudos, arranged into the target's sched1 emission order, produce the target's dz/dx register seats.
- mechanism: local-alloc reads refs and live span off the scheduled stream; if that stream is the target's, both quantities necessarily carry 3 references and a 12-unit span and neither carries a register suggestion, so the qty_compare_1 tie-break at local-alloc.c:1684 fixes the seat by birth order.
- probe: The BB2_SUGG_DEBUG input table above, the exhaustive 720-permutation simulation in s7/sim_luid.py, and three direct sandbox measurements of order-reaching bodies (h1, p1 = h1 with fresh test-3 cross-product locals, p7 = h1 with the outer kc/kp).
- result: KILLED. All three order-reaching bodies score 9 at 202 insns with the dz/dx seats inverted, and the input table shows why: the two quantities are indistinguishable to the comparator. The useful corollary for s8 is that the original compile must have presented local-alloc with an extra reference, an extra quantity, or a copy that is absent from the final 202 instructions -- reload/flow2 delete_noop_moves, reorg's delay-slot steal and jump2 cross-jumping all run after local_alloc.
- verdict: KILLED
- kill_scope: class
- measured_on: HEAD b3a8a19a (-mel -msoft-float), the s5 candidate body and the s5 h1 body spliced into src/code6cac_b.c, one FAKE construct present (the same-value re-store of the local m)
- predicate_cite: tools/gcc-2.7.2/local-alloc.c:1684

## [s7] The inverse RA solver's top spellable vector -- a fourth in-block reference to dx -- can be spelled by carrying the test-3 kp in dx once the test-3 cross products get fresh block-local names, which is the repair of the s6 batch-N failure that merged the outer cross-block kp into dx.
- mechanism: qty_compare_1's numerator steps at floor_log2(4)*4 = 8, so a fourth reference lifts dx's priority clear of dz's even with the longer span; the suggestion pass is not involved.
- probe: Batch P, 8 variants (tmp/grind/func_8002D780/s7/gen_p.py -> s7/variantsP): fresh test-3 cross-product locals with kp carried in dx, in dz, or not carried, against the h1 and candidate statement orders, plus outer-kc controls; scored with tmp/grind/func_8002D780/s3/run.ps1.
- result: KILLED. p2 = 28 and p3 = 28 (fresh c3 local, kp carried in dx, h1 and candidate orders); p4 = 16 and p8 = 16 (outer kc kept, reproducing batch N's number); p5 = 30 (kp carried in dz); all at 202 insns. Re-extracting the local-alloc table from p2 shows the reuse also moved the SCHEDULE: dx's quantity is gone (merged with kp, birth 24) and dz's death moved 16 -> 10. Byte-neutral by-product: giving test 3 its own cross-product locals costs nothing (p1 = 9 = p7; p6 = 2 = the candidate).
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD b3a8a19a (-mel -msoft-float), the s5 candidate body spliced into src/code6cac_b.c, one FAKE construct present (the same-value re-store of the local m)

## [s7] A plain dx2 = dx copy makes local-alloc's combine_regs merge the two pseudos into one quantity whose n_refs is the sum, lifting dx clear of dz, while the copy itself ends as a same-register no-op that flow2's delete_noop_moves removes.
- mechanism: local-alloc combines the quantities of a copy whose source dies at the copy and sums qty_n_refs; a copy between two pseudos that land in the same hard register is deleted after reload, so the final stream can still be 202 instructions.
- probe: Batch Q, 6 variants (tmp/grind/func_8002D780/s7/gen_q.py -> s7/variantsQ): the copy placed before the products, between them, feeding one product or both, and a dz-side control, against the h1 and candidate statement orders.
- result: KILLED. Every placement is byte-neutral -- q1/q3/q4/q5 = 9 on the h1 order, q2/q7 = 2 on the candidate order, all at 202 insns. cse.c make_regs_eqv puts the copy's source and destination into one quantity and deletes the copy insn long before local-alloc runs, the identical mechanism s5 documented for residual B. The route survives only with a MULTIPLY-DEFINED carrier, and this straight-line block offers no control-flow arm to hold the second definition. By-product: a single-def copy of dz or dx is a free carrier slot on both statement orders.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD b3a8a19a (-mel -msoft-float), the s5 candidate body spliced into src/code6cac_b.c, one FAKE construct present (the same-value re-store of the local m)

## [s8] The sched1 half of residual A (the az/dx emission-order swap) has exactly ONE C-side input on this block shape: statement order (INSN_LUID). Neither INSN_PRIORITY nor rank_for_schedule's dependence-class tie-break can be moved by any source spelling that leaves block 7's four subus and four mults as its only insns.
- mechanism: GCC 2.7.2's sched.c list scheduler is BACKWARD (it picks insns from the ready list and lays them down from the end of the block towards the start), so the PREFERRED insn of a ready-list pair is the one emitted LATER. `priority()` (sched.c:1434) is a DEPTH function -- it walks LOG_LINKS, i.e. the insn's PREDECESSORS, and returns `max(priority(pred) + insn_cost(pred) - 1)`, floored at 1. Every one of block 7's six difference subus takes both of its operands from pseudos computed in an EARLIER basic block, so each has an empty in-block LOG_LINKS list and INSN_PRIORITY == 1, uniformly. rank_for_schedule (sched.c:2408) therefore falls through the priority test; its second test is the dependence class w.r.t. last_scheduled_insn, and at the decision that matters last_scheduled_insn is the `mult dx*az` (our insn 199 / target 8002D928), of which BOTH candidates -- az's subu and dx's subu -- are DATA predecessors with insn_cost == result_ready_cost(subu) == 1, so both classify 3 (sched.c:2429 `link == 0 || insn_cost (...) == 1`). Class 2 was already class-killed in s6 by mips.h:2946's ADJUST_COST. So the comparator reaches its last line, `INSN_LUID (tmp) - INSN_LUID (tmp2)`, and the higher-LUID insn is preferred, i.e. emitted later. Ours emits `subu az` then `subu dx` because dx's statement is last; the target emits `subu $v1,$t5,$t1` (dx) then `subu $v0,$a2,$a3` (az), which requires az to be PREFERRED, i.e. LUID(az) > LUID(dx), i.e. dx's statement before az's -- the h1 order and nothing else.
- probe: read sched.c:1372-1520 (insn_cost + priority) and sched.c:2400-2465 (rank_for_schedule) against the block-7 RTL slice of the 2-floor candidate's own .sched dump (tmp/grind/func_8002D780/s8/sched_fn.txt, insns 179/191/197/182/194/199/185/204/188/206/201/208/210/212, every subu carrying `(nil)` LOG_LINKS), and against the target's asm/funcs/func_8002D780.s L103-L123.
- result: KILLED. There is no in-block producer for any of ax/az/bx/bz/dz/dx, so no source spelling that keeps block 7 at these fourteen RTL insns can give az an INSN_PRIORITY of 2 (that needs an in-block predecessor whose result_ready_cost is 2 -- a load -- and every load of the six operands is hoisted into an earlier block by construction, since each operand is also read by test 1 or test 2), and no spelling can move dx out of class 3 (that needs dx's producer to be something other than a 1-cycle subu). The corollary corrects the s6 frontier's item 1, which proposed raising DX's priority: because the scheduler is backward, raising dx's priority would push dx LATER, which is the wrong direction; it is az's priority that would have to rise, and az is the one with no in-block operand at all.
- verdict: KILLED
- kill_scope: class
- measured_on: HEAD a91a7f44 (-mel -msoft-float), the s5 2-floor candidate.c body spliced into src/code6cac_b.c (sandbox re-measured 2/202 this session), one FAKE construct present (the same-value re-store of the local `m`)
- predicate_cite: tools/gcc-2.7.2/sched.c:2408

## [s8] A self-assign of a block-local (`dx = dx;`) is a free way to give dx a fourth local-alloc reference, because the dead-store family sanctions it and s5 showed a redundant same-value store is dropped before final output.
- mechanism: qty_compare_1's numerator steps at floor_log2(4)*4 = 8, so a fourth reference lifts dx's quantity clear of dz's; a store that is redundant is expected to be deleted late (flow2/jump2), i.e. after local_alloc has already counted the reference.
- probe: batch S (tmp/grind/func_8002D780/s8/gen_s.py -> s8/variantsS), 10 variants: `dx = dx;` after the difference declarations, between the two cross products, immediately before the products, doubled, plus `dz = dz;` and `az = az;` direction controls, on both the h1 and the candidate statement orders; then a full instrumented-cc1 dump of s1 and a regex scan of every pass dump for a `(set (reg N) (reg N))` pattern (tmp/grind/func_8002D780/s8/copyscan.py).
- result: KILLED, and the mechanism is much earlier than assumed. s1/s3/s4/s6/s8/s10 = 9 on h1 and s5/s9 = 2 on the candidate order, all at 202 insns -- i.e. completely byte-neutral. copyscan.py finds ZERO self-copies in EVERY dump including `.rtl`: the RTL expander emits no insn at all for `x = x` on a plain register local, so the construct never exists for any pass to count. A self-assign is therefore not a refs lever here at all (as distinct from s5's `m = dist;`, which is a copy between two DIFFERENT variables and does emit an insn). Incidental: s2 (a statement spliced between two declarations) fails C89 declaration-order and builds a 182-insn body -- generators for this function must keep all declarations first.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD a91a7f44 (-mel -msoft-float), the s5 candidate body and the s5 h1 body spliced into src/code6cac_b.c, one FAKE construct present (the `m` re-store)

## [s8] A copy carrier with TWO definitions inside block 7 survives cse the way s5's `m` does, so it can hold a fourth reference to dx (via combine_regs) or a spare insn between dz's and dx's births, and then vanish at jump2's delete_noop_moves.
- mechanism: s5/s7 established that cse folds every SINGLE-def copy, and that a multiply-defined destination defeats that; `noop_moves` is passed 1 only at the jump2 call (toplev.c:3142), so a copy whose two pseudos land in the same hard register is deleted AFTER local_alloc has already seen it, keeping build_insns at 202.
- probe: batch T (tmp/grind/func_8002D780/s8/gen_t.py -> s8/variantsT), 9 variants: a block-local `c` initialised from az / bz / dz / ax / bx and then re-assigned `c = dx;`, with the products reading c in both arms or only the second, plus a reversed `c = dx; c = az;` and two candidate-order controls; then a full instrumented dump of t1 and the pass-by-pass copy scan (tmp/grind/func_8002D780/s8/t1_{rtl,jump,cse,lreg}_slice.txt).
- result: KILLED. All nine are byte-neutral (t1/t2/t3/t5/t6/t7/t8 = 9 on h1, t4/t9 = 2 on the candidate order, every one at 202 insns). The dump says exactly why: t1's two copies are `(insn 197 (set (reg 135) (reg 132)))` and `(insn 200 (set (reg 135) (reg 131)))`, both PRESENT in `.rtl` and `.jump` and both ABSENT from `.cse` onward, while in the SAME dump the s5 `m` copies (insns 264 and 274, `(set (reg 155) (reg 143))`) survive all the way into `.lreg`. The discriminator is not the number of definitions, it is whether they sit in the same basic block: cse's per-block value table (insert_regs -> make_regs_eqv, cse.c:1032) sees a same-block re-definition, deletes the first copy as a dead store and then folds the second as a single-def copy; the `m` shape escapes only because its second definition is inside a conditional arm, so the value is unknown at the join. Block 7 is a single straight-line basic block with no internal control flow, so no carrier defined only inside it can reach local_alloc.
- verdict: KILLED
- kill_scope: class
- measured_on: HEAD a91a7f44 (-mel -msoft-float), the s5 candidate body and the s5 h1 body spliced into src/code6cac_b.c, one FAKE construct present (the `m` re-store)
- predicate_cite: tools/gcc-2.7.2/cse.c:1032

## [s8] Borrowing an EXISTING multiply-defined variable (test 2's `kp`) as the block-7 copy carrier gets the copy past cse and is therefore the cheap spelling of the surviving-copy route.
- mechanism: `kp` is written in the test-1 arm and again in test 3, so its pseudo is multiply-defined across a join exactly like `m`; using it to carry dx would put a real copy insn into block 7 without inventing a new variable.
- probe: batch R (tmp/grind/func_8002D780/s8/gen_r.py -> s8/variantsR), 8 variants: a block-scoped `c` that takes over test 2's `kp` computation (making c multiply-defined across the test-2/test-3 join) and is then re-assigned `c = dx;`, carrying dx into both products / the second product only / the first only, a `c = dz;` direction control, a single-def control, and candidate-order controls.
- result: KILLED as an improvement, informative as a cost measurement. r1/r2/r4/r8 = 16 on the h1 order and r3/r6 = 13 on the candidate order, all at 202 insns; the single-def control r5 = 9 is byte-neutral, which isolates the +7 / +11 entirely to the change in test 2's own codegen (moving test-2's kp out of `kp` and into `c`). The copies do vanish by final (202 insns), so the delete-before-final half of the route is confirmed working; what is not affordable is manufacturing the cross-block second definition out of an existing test-2 statement. r7 (a statement spliced between declarations) is a C89 build break at 184 insns, not a measurement.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD a91a7f44 (-mel -msoft-float), the s5 candidate body and the s5 h1 body spliced into src/code6cac_b.c, one FAKE construct present (the `m` re-store)

## Live frontier (for s9) -- still 2 insns, and the box around residual A is now closed on three sides

1. **The carrier paradox is the one thing left to break, and it has a named shape.** s7 proved that on the target's emission order dz and dx tie on every qty_compare_1 input, so the original compile saw something extra in block 7 at local-alloc time. s8 proved (a) that extra thing cannot be a self-assign (no RTL at all) and (b) it cannot be a copy whose definitions all live inside block 7 (cse deletes it). A carrier whose second definition lives in ANOTHER basic block does survive cse -- but a pseudo referenced in two basic blocks is REG_BLOCK_GLOBAL and is skipped by block_alloc, so it cannot merge quantities with dx and cannot give dx a fourth reference. That closes the reference-count route completely. What it does NOT close is the SPAN route: a cross-block carrier's copy insn is still a real insn sitting inside block 7 at local_alloc time, and if it schedules between dz's definition and dx's definition it lengthens dz's live range from 6 to 7 while dx's stays 6, which breaks the tie in dx's favour WITHOUT touching either quantity's reference count. Next probe: take the h1 body, add an `m`-shaped carrier (declared before the test-2 arm, assigned once inside the test-2 arm, re-assigned inside test 3 from a block-7 value that dies at the copy), and BEFORE scoring anything read tmp/grind/func_8002D780/dumps/code6cac_b.lreg to confirm (i) the copy insn is present in block 7, (ii) it sits between the `dz` subu and the `dx` subu in the scheduled stream, and (iii) dz's `used K times across M insns` line reports 7 where dx's reports 6. Only then score it; the copy must also be deleted by jump2's delete_noop_moves (toplev.c:3142) to keep build_insns at 202, which requires the two pseudos to land in the same hard register.
2. **Spell the `[extra_qty] one more temp: refs=2 span=[2,4) pri=10000` vector** (inherited unspent from s7 and untouched by every s8 kill, because it does not rely on a copy at all). It is the only 1-atom vector in the inverse.py report that leaves both tied quantities' own comparator columns alone: an additional short-lived quantity born at the top of block 7 changes what find_free_reg has already handed out by the time the tied pair is reached. Next probe: a seventh test-3 local whose value is genuinely consumed -- splitting one of the four multiply operands into a named intermediate is the obvious candidate -- measured against the h1 order with build_insns checked to stay at 202.
3. **A decomp-permuter campaign on the 2-floor chassis** (inherited unspent from s5/s6/s7). Now sharper still: s8 has class-killed the entire sched1 side, so the permuter should be pointed at the h1 (order-correct, seats-wrong) body as well as the candidate, and its temporary-introduction moves are exactly the extra-quantity search of item 2. Regenerate base.c into the validated workspace tmp/grind/func_8002D780/s4/nonmatchings/func_8002D780 (four hurdles + rebuild recipe in evidence.md [s4]), launch with tools/permuter_campaign.py, wait IN-TURN, harvest with --stop, re-score every find on the engine sandbox. The family classification of the body (the FAKE `m` re-store plus the six difference locals) is still unsettled and must be recorded before any candidate-ready submission.

## [s8] The sched1 half of residual A (the az/dx emission-order swap) has exactly one C-side input on this block shape -- statement order (INSN_LUID); neither INSN_PRIORITY nor rank_for_schedule's dependence-class tie-break can be moved by any source spelling that leaves block 7's four subus and four mults as its only insns.
- mechanism: GCC 2.7.2's sched.c list scheduler is BACKWARD, so the insn rank_for_schedule prefers is the one emitted LATER. priority() (sched.c:1434) is a depth function over LOG_LINKS (predecessors), and all six block-7 difference subus read both operands from an earlier basic block, so each has an empty in-block LOG_LINKS list and INSN_PRIORITY 1. At the decision that matters, last_scheduled_insn is the dx*az mult and both ready candidates are data predecessors of it with insn_cost 1, so both classify 3 (sched.c:2429); class 2 was already closed in s6 by mips.h:2946's ADJUST_COST. The comparator therefore falls through to INSN_LUID(tmp) - INSN_LUID(tmp2), and the higher-LUID insn is emitted later.
- probe: Read sched.c:1372-1520 (insn_cost + priority) and sched.c:2400-2465 (rank_for_schedule) against the block-7 RTL slice of the 2-floor candidate's own instrumented-cc1 .sched dump (tmp/grind/func_8002D780/s8/sched_fn.txt, insns 179/191/197/182/194/199/185/204/188/206/201/208/210/212, every subu carrying `(nil)` LOG_LINKS) and against the target's asm/funcs/func_8002D780.s L103-L123 with the operand map recovered from the four subus.
- result: KILLED. There is no in-block producer for any of ax/az/bx/bz/dz/dx, so nothing can give az an INSN_PRIORITY of 2 (that needs an in-block predecessor with result_ready_cost 2 -- a load -- and every one of the six operands is also read by test 1 or test 2, so its load is hoisted out of block 7 by construction), and nothing can move dx out of class 3 (that needs dx's producer to be something other than a 1-cycle subu). Important correction to the s6 frontier, which proposed raising DX's priority: with a backward scheduler that pushes dx LATER, the wrong direction; it is az's priority that would have to rise, and az is the one with no in-block operand at all. Combined with s7's local-alloc class kill this boxes residual A on both sides: the statement order that wins the sched pick (h1) is exactly the order that ties dz and dx in qty_compare_1.
- verdict: KILLED
- kill_scope: class
- measured_on: HEAD a91a7f44 (-mel -msoft-float), the s5 2-floor candidate.c body spliced into src/code6cac_b.c and re-measured at 2/202 this session, one FAKE construct present (the same-value re-store of the local m)
- predicate_cite: tools/gcc-2.7.2/sched.c:2408

## [s8] A self-assign of a block-local (`dx = dx;`) gives dx a fourth local-alloc reference that lifts its qty_compare_1 priority clear of dz's, and is dropped before final output the way s5's redundant `m` store is.
- mechanism: qty_compare_1's numerator steps at floor_log2(4)*4 = 8, so a fourth reference lifts dx's quantity clear of dz's; a redundant store was expected to be deleted late (flow2/jump2), i.e. after local_alloc had already counted the reference.
- probe: Batch S (tmp/grind/func_8002D780/s8/gen_s.py -> s8/variantsS), 10 variants: `dx = dx;` after the difference declarations, between the two cross products, immediately before the products, doubled, plus `dz = dz;` and `az = az;` direction controls, on both the h1 and the candidate statement orders; then a full instrumented-cc1 dump of s1 and a regex scan of every pass dump for a `(set (reg N) (reg N))` pattern (tmp/grind/func_8002D780/s8/copyscan.py).
- result: KILLED, with the mechanism much earlier than assumed. s1/s3/s4/s6/s8/s10 = 9 on h1 and s5/s9 = 2 on the candidate order, all at 202 insns -- completely byte-neutral. copyscan.py finds zero self-copies in EVERY dump including `.rtl`: the RTL expander emits no insn at all for `x = x` on a plain register local, so the construct never exists for any pass to count. That is the sharp distinction from s5's `m = dist;`, which is a copy between two DIFFERENT variables and does emit an insn. Incidental: variant s2 spliced a statement between two declarations, which violates C89 declaration order and silently builds a 182-insn body -- generators for this function must keep all declarations at the head of their block.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD a91a7f44 (-mel -msoft-float), the s5 candidate body and the s5 h1 body spliced into src/code6cac_b.c, one FAKE construct present (the m re-store)

## [s8] A copy carrier with two definitions inside block 7 survives cse the way s5's `m` does, so it can hold a fourth reference to dx via combine_regs or a spare insn between dz's and dx's births, and then vanish at jump2's delete_noop_moves.
- mechanism: s5/s7 established that cse folds every single-def copy and that a multiply-defined destination defeats that; jump_optimize is passed noop_moves == 1 only at the jump2 call (toplev.c:3142), so a copy whose two pseudos land in the same hard register is deleted after local_alloc has already seen it, keeping build_insns at 202.
- probe: Batch T (tmp/grind/func_8002D780/s8/gen_t.py -> s8/variantsT), 9 variants: a block-local `c` initialised from az / bz / dz / ax / bx and then re-assigned `c = dx;`, with the products reading c in both arms or only the second, plus a reversed `c = dx; c = az;` and two candidate-order controls; then a full instrumented dump of t1 and the pass-by-pass copy scan (tmp/grind/func_8002D780/s8/t1_{rtl,jump,cse,lreg}_slice.txt).
- result: KILLED. All nine are byte-neutral (t1/t2/t3/t5/t6/t7/t8 = 9 on h1, t4/t9 = 2 on the candidate order, every one at 202 insns). The dump says why: t1's two copies, `(insn 197 (set (reg 135) (reg 132)))` and `(insn 200 (set (reg 135) (reg 131)))`, are present in `.rtl` and `.jump` and absent from `.cse` onward, while in the SAME dump this body's own `m` copies (insns 264/274, `(set (reg 155) (reg 143))`) survive into `.lreg`. The discriminator is not the number of definitions but whether they share a basic block: cse's per-block value table (insert_regs -> make_regs_eqv) deletes the first copy as a dead store and folds the second as a single-def copy. Block 7 has no internal control flow, so no carrier confined to it can reach local_alloc. Corollary that closes a whole route: a carrier that survives cse needs a definition in another basic block, but a pseudo referenced in more than one block is skipped by block_alloc, so it can never lend dx a reference -- the reference-count route into qty_compare_1 is closed, and only the live-span route remains.
- verdict: KILLED
- kill_scope: class
- measured_on: HEAD a91a7f44 (-mel -msoft-float), the s5 candidate body and the s5 h1 body spliced into src/code6cac_b.c, one FAKE construct present (the m re-store)
- predicate_cite: tools/gcc-2.7.2/cse.c:1032

## [s8] Borrowing an existing multiply-defined variable (test 2's kp) as the block-7 copy carrier gets the copy past cse and is the cheap spelling of the surviving-copy route.
- mechanism: kp is written in the test-1 arm and again in test 3, so its pseudo is multiply-defined across a join exactly like m; using it to carry dx would put a real copy insn into block 7 without inventing a new variable.
- probe: Batch R (tmp/grind/func_8002D780/s8/gen_r.py -> s8/variantsR), 8 variants: a block-scoped `c` that takes over test 2's kp computation (making c multiply-defined across the test-2/test-3 join) and is then re-assigned `c = dx;`, carrying dx into both products / the second product only / the first only, a `c = dz;` direction control, a single-def control, and candidate-order controls.
- result: KILLED as an improvement, informative as a cost measurement. r1/r2/r4/r8 = 16 on the h1 order and r3/r6 = 13 on the candidate order, all at 202 insns; the single-def control r5 = 9 is byte-neutral, which isolates the whole +7 / +11 to the change in test 2's own codegen. The copies do vanish by final output (202 insns), so the delete-before-final half of the route is confirmed working; what is unaffordable is manufacturing the cross-block second definition out of an existing test-2 statement. Variant r7 is a C89 declaration-order build break at 184 insns, not a measurement.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD a91a7f44 (-mel -msoft-float), the s5 candidate body and the s5 h1 body spliced into src/code6cac_b.c, one FAKE construct present (the m re-store)

## [s9] Kill re-audit: the s6-s8 instance kills were measured with the FAKE `m` re-store occupying a pseudo that residual A needs, so at least one of them is void on the current chassis.
- mechanism: a lever measured inert while a FAKE carrier occupies its target pseudo is not a kill (func_8002EA24 s8). The FAKE here is a copy in the sqrt block; residual A is in test 3 (block 7).
- probe: re-measured the s5 candidate on HEAD c7aa37e7 (2/202) and ran `python3 tools/fake_ablate.py --func func_8002D780 --file code6cac_b --candidate memory/grind/func_8002D780/candidate.c`.
- result: KILLED -- the re-audit clears the earlier kills rather than voiding them. keep-all = 2/202, drop-1 = 6/202: the FAKE is worth 4 insns on this chassis (5 when s5 measured it), and residual A's two insns are present in both variants. The FAKE construct lives in a different basic block from block 7 and does not occupy dz's, dx's or ax's quantity, so the s6/s7/s8 measurements taken "with one FAKE construct present" stand as recorded and do not need re-running.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD c7aa37e7 (-mel -msoft-float), the s5 candidate.c body spliced into src/code6cac_b.c, measured both with the FAKE present and with it ablated by tools/fake_ablate.py

## [s9] On the h1 (target) emission order the dz/dx seat is decided by local-alloc's quantity-number fallback, and an extra short-lived block-7 quantity born between their births flips the seats to the target's.
- mechanism: qty_sugg_compare_1 (local-alloc.c:1725-1758) sorts by suggestion count, then by floor_log2(refs)*refs*size/(death-birth)*10000, then by quantity number. On the h1 order dz and dx are both refs 3 / span 12 = pri 2500, so the quantity-number line seats dz (born first) on $v1. Inserting a quantity that conflicts with dz but dies before dx's birth denies dz $v1 and leaves it free for dx.
- probe: instrumented cc1 (tools/gcc-2.7.2/cc1) with BB2_SUGG_DEBUG=1 BB2_QTY_DEBUG=1 via tmp/grind/func_8002D780/s9/dumpvar.py on the candidate body, the h1 body, and probe u3 (tmp/grind/func_8002D780/s9/variantsU/u3_probe_early_extra_qty.c), reading the SUGGDBG-QTY table and the QTYDBG allocation trace for blk=7.
- result: CONFIRMED. Candidate order: dz qty1 (birth 4 death 16 refs 3, pri 2500), dx qty5 (birth 10 death 20, pri 3000) -> dx allocated at ord=5 with got=3 ($v1), dz at ord=6 with got=4 ($a0). h1 order: dz qty1 (4,16), dx qty4 (8,20), both pri 2500 -> dz ord=5 got=3, dx ord=6 got=4, seats inverted, score 9. Probe u3 adds `e` (qty2, birth 6 death 8, refs 2, pri 10000): e is allocated first onto $v0, ax is pushed to $v1, dz gets $a0 and dx gets $v1 -- the target's seats on the target's emission order. The s7 inverse-solver [extra_qty] vector is therefore a real lever, demonstrated in the allocator's own trace rather than in a model.
- verdict: CONFIRMED

## [s9] The extra block-7 quantity that flips the dz/dx seats can be spelled without disturbing the seats of ax, az, bx and bz.
- mechanism: the target puts ax, az, bx and bz all in $v0 (asm/funcs/func_8002D780.s L110/L124/L130/L140). A new quantity only helps if it is allocated before dz; if it is also allocated before ax it takes $v0 first and ax is displaced.
- probe: probe u3 (an extra value born after dz and consumed by the first multiply) scored on the engine sandbox and read in the QTYDBG trace, plus the priority arithmetic worked through against the block-7 insn geometry read from s9/h1/code6cac_b.lreg.
- result: KILLED on this block geometry. u3 measures 13/203: the extra quantity has refs 2 over a span of 2, hence pri 10000 against ax's 5000, so it is allocated first, takes $v0, and ax moves to $v1. For a quantity B to fix the seats and leave ax alone it must satisfy four constraints at once -- pri < 5000 (below ax), pri > 2500 (above dz), a live range overlapping dz's [4,16], and death <= 8 so dx keeps $v1. With births at 2*insn-index over block 7's 14 insns the only triple satisfying all four is refs 2 / birth 4 / death 8, and birth 4 is the slot of dz's own defining insn, which already sets dz. Adding an insn to free that slot moves the body to 203 and reshuffles the schedule, as u3 shows.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD c7aa37e7 (-mel -msoft-float), the s5 h1 body (tmp/grind/func_8002D780/s5/variantsH/h1_ax_dz_dx_az.c) spliced into src/code6cac_b.c, one FAKE construct present (the `m` re-store)

## [s9] One of block 7's four multiply-result pseudos can be turned into the extra local-alloc quantity, since each is defined and dies inside block 7.
- mechanism: reg135-138 are set by the four mult insns and consumed by the two cross-product subtractions, all inside block 7, so on live-range grounds they are block-local and would supply an extra quantity for free.
- probe: read `dump_flow_info`'s register lines and the SUGGDBG-QTY table for blk=7 in tmp/grind/func_8002D780/s9/h1/{code6cac_b.lreg,qty.txt}, then read local-alloc.c:470-478.
- result: KILLED. The dump prints all four as `Register N used 2 times across K insns in block 7; pref LO_REG, else GR_REGS`, and none of them appears in the blk=7 quantity table (7 general quantities + 8 hi/accum clobber quantities = 15 total, no product among them). local-alloc.c:472 marks a pseudo allocatable-locally (`reg_qty[i] = -2`) only when `reg_alternate_class (i) == NO_REGS || ! CLASS_LIKELY_SPILLED_P (reg_preferred_class (i))`; a multiply result prefers LO_REG (likely spilled) and has GR_REGS as its alternate class, so it fails both disjuncts and block_alloc never sees it. Confirmed independently by probe u2 (naming the first product), which is byte-neutral at 9/202 and adds no quantity.
- verdict: KILLED
- kill_scope: class
- measured_on: HEAD c7aa37e7 (-mel -msoft-float), the s5 h1 body spliced into src/code6cac_b.c, one FAKE construct present (the `m` re-store)
- predicate_cite: tools/gcc-2.7.2/local-alloc.c:472

## [s9] Giving test 3 its own cross-product locals (instead of reusing the outer kc/kp) adds block-local quantities that break the dz/dx tie.
- mechanism: the outer `kc`/`kp` are multi-block and therefore invisible to block_alloc; fresh test-3 names would become block-7 quantities and change what find_free_reg has handed out by the time the tied pair is reached.
- probe: variant u1 (tmp/grind/func_8002D780/s9/variantsU/u1_fresh_kc_kp_locals.c) on the h1 order, scored on the sandbox and dumped with BB2_QTY_DEBUG.
- result: KILLED as a lever, banked as a free degree of freedom. u1 = 9/202, identical to the h1 baseline. The two new quantities do appear (qty14 reg141 birth 22 death 28 refs 4, qty15 reg136 birth 24 death 26 refs 2) but both are born after dx's death window, so they are seated on $v0 before the tied pair without ever conflicting with it; dz stays qty1 (4,16) and dx qty4 (8,20), still tied, still seated dz-first. Fresh test-3 cross-product locals are byte-neutral and available free to future probes.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD c7aa37e7 (-mel -msoft-float), the s5 h1 body spliced into src/code6cac_b.c, one FAKE construct present (the `m` re-store)

## Live frontier (for s10) -- 2 insns, and the seat flip now has a measured mechanism and a measured price

1. **Fill the [4,8] slot with an insn that disappears after local_alloc.** s9 pinned the
   unique admissible shape: a quantity with refs 2, birth 4 and death 8 -- born at block-7
   insn 2, dead at insn 4 -- which outranks dz (2500) but not ax (5000) and clears dx's
   birth. The only pass that can remove an insn after local_alloc has counted it is jump2's
   `delete_noop_moves` (toplev.c:3142), and s8 established that a copy surviving cse needs
   its second definition in a DIFFERENT basic block. So the shape to spell is an `m`-shaped
   cross-block carrier (declared before the test-2 arm, defined once inside that arm, and
   re-assigned inside test 3 from a value that dies at the copy) whose copy insn schedules
   between ax's subu and dz's subu. Next probe: build it on the h1 order and BEFORE scoring
   read the blk=7 SUGGDBG-QTY table produced by tmp/grind/func_8002D780/s9/dumpvar.py to
   confirm the new quantity reports birth 4 death 8 refs 2 and that ax still reports birth 2
   death 6; only then score, and require build_insns == 202. Note s8's batch R measured that
   manufacturing the cross-block second definition out of an existing test-2 statement costs
   +7/+11, so the second definition has to be a statement test 2 already needs.
2. **Take ax out of the competition instead of working around it.** The whole price of the
   u3 shape is that ax is displaced from $v0. If ax were a multi-block pseudo it would be
   seated by global_alloc rather than block_alloc, the extra quantity could take $v0 freely,
   and dz/dx would flip exactly as u3 already demonstrates. s6's batch O did this to dz and
   dx (23-40) but never to ax. Next probe: carry `ax` (= cx - x0) in a variable that is also
   live in the test-2 block, on the h1 order, and read the blk=7 table to confirm ax has left
   the quantity list before scoring.
3. **A decomp-permuter campaign on the 2-floor chassis** (inherited unspent from s5/s6/s7/s8,
   and now the best-motivated it has been: the target shape is a single extra short-lived
   temporary in one block, which is exactly the permuter's temporary-introduction move).
   Regenerate base.c from candidate.c AND from the h1 body into the validated workspace
   tmp/grind/func_8002D780/s4/nonmatchings/func_8002D780 (four hurdles + rebuild recipe in
   evidence.md [s4]), launch with tools/permuter_campaign.py, wait IN-TURN with
   `permuter_campaign.py wait --dir <ws>`, harvest with --stop before the turn ends, and
   re-score every find on the engine sandbox. The family classification of the body (the FAKE
   `m` re-store plus the six difference locals) is still unsettled and must be recorded before
   any candidate-ready submission.

## [s9] Kill re-audit: the s6-s8 instance kills were measured with the FAKE `m` re-store occupying a pseudo that residual A needs, so at least one of them is void on the current chassis.
- mechanism: A lever measured inert while a FAKE carrier occupies its target pseudo is not a kill (func_8002EA24 s8). The FAKE here is a copy in the sqrt block; residual A lives in test 3 (block 7).
- probe: Re-measured the s5 candidate on HEAD c7aa37e7 (2/202) and ran `python3 tools/fake_ablate.py --func func_8002D780 --file code6cac_b --candidate memory/grind/func_8002D780/candidate.c`.
- result: KILLED -- the re-audit clears the earlier kills rather than voiding them. keep-all = 2/202, drop-1 = 6/202, so the FAKE is worth 4 insns on this chassis (5 when s5 measured it), and residual A's two insns are present in both variants. The FAKE construct sits in a different basic block from block 7 and does not occupy dz's, dx's or ax's quantity, so the s6/s7/s8 measurements taken 'with one FAKE construct present' stand as recorded.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD c7aa37e7 (-mel -msoft-float), the s5 candidate.c body spliced into src/code6cac_b.c, measured both with the FAKE present and with it ablated by tools/fake_ablate.py

## [s9] On the h1 (target) emission order the dz/dx seat is decided by local-alloc's quantity-number fallback, and an extra short-lived block-7 quantity born between their births flips the seats to the target's.
- mechanism: qty_sugg_compare_1 (local-alloc.c:1725-1758) sorts by suggestion count, then by floor_log2(refs)*refs*size/(death-birth)*10000, then by quantity number. On the h1 order dz and dx are both refs 3 / span 12 = pri 2500, so the quantity-number line seats dz (born first) on $v1. A quantity that conflicts with dz but dies before dx's birth denies dz $v1 and leaves it free for dx.
- probe: Instrumented cc1 (tools/gcc-2.7.2/cc1) with BB2_SUGG_DEBUG=1 BB2_QTY_DEBUG=1 via tmp/grind/func_8002D780/s9/dumpvar.py on the candidate body, the h1 body and probe u3; read the SUGGDBG-QTY table (local-alloc.c:1447) and the QTYDBG find_free_reg trace (local-alloc.c:1585) for blk=7.
- result: CONFIRMED. Candidate order: dz qty1 (birth 4 death 16 refs 3, pri 2500), dx qty5 (birth 10 death 20, pri 3000) -> dx allocated at ord=5 got=3 ($v1), dz ord=6 got=4 ($a0) -- the target seats. h1 order: dz qty1 (4,16), dx qty4 (8,20), both pri 2500 -> dz ord=5 got=3, dx ord=6 got=4, seats inverted, score 9. Probe u3 adds `e` (qty2, birth 6 death 8, refs 2, pri 10000): e is allocated first onto $v0, ax is pushed to $v1, dz gets $a0 and dx gets $v1 -- the target's seats on the target's own emission order. The s7 inverse-solver [extra_qty] vector is a genuine lever, now demonstrated in the allocator's trace rather than in a model.
- verdict: CONFIRMED

## [s9] The extra block-7 quantity that flips the dz/dx seats can be spelled without disturbing the seats of ax, az, bx and bz.
- mechanism: The target puts ax, az, bx and bz all in $v0 (asm/funcs/func_8002D780.s L110/L124/L130/L140). A new quantity only helps if it is allocated before dz; if it is also allocated before ax it takes $v0 first and ax is displaced.
- probe: Probe u3 (tmp/grind/func_8002D780/s9/variantsU/u3_probe_early_extra_qty.c) scored on the engine sandbox and read in the QTYDBG trace, plus the qty-priority arithmetic worked through against the block-7 insn geometry sliced from tmp/grind/func_8002D780/s9/h1/code6cac_b.lreg.
- result: KILLED on this block geometry. u3 measures 13/203: the extra quantity has refs 2 over a span of 2, hence pri 10000 against ax's 5000, so it is allocated first, takes $v0, and ax moves to $v1. For a quantity B to fix the seats and leave ax alone it must satisfy four constraints at once -- pri < 5000 (below ax), pri > 2500 (above dz), a live range overlapping dz's [4,16], and death <= 8 so dx keeps $v1. With births at 2*insn-index over block 7's 14 insns the only triple satisfying all four is refs 2 / birth 4 / death 8, and birth 4 is the slot of dz's own defining insn, which already sets dz.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD c7aa37e7 (-mel -msoft-float), the s5 h1 body (tmp/grind/func_8002D780/s5/variantsH/h1_ax_dz_dx_az.c) spliced into src/code6cac_b.c, one FAKE construct present (the `m` re-store)

## [s9] One of block 7's four multiply-result pseudos can be turned into the extra local-alloc quantity, since each is defined and dies inside block 7.
- mechanism: reg135-138 are set by the four mult insns and consumed by the two cross-product subtractions, all inside block 7, so on live-range grounds they are block-local and would supply an extra quantity without adding an insn.
- probe: Read dump_flow_info's register lines and the SUGGDBG-QTY table for blk=7 in tmp/grind/func_8002D780/s9/h1/{code6cac_b.lreg,qty.txt}, then read tools/gcc-2.7.2/local-alloc.c:470-478; cross-checked with variant u2 (naming the first product).
- result: KILLED. The dump prints all four as `Register N used 2 times across K insns in block 7; pref LO_REG, else GR_REGS`, and none appears in the blk=7 quantity table (7 general quantities + 8 hi/accum clobber quantities = 15, no product among them). local-alloc.c:472 marks a pseudo allocatable-locally only when `reg_alternate_class (i) == NO_REGS || ! CLASS_LIKELY_SPILLED_P (reg_preferred_class (i))`; a multiply result prefers LO_REG (likely spilled) and has GR_REGS as its alternate class, failing both disjuncts, so block_alloc never sees it. Variant u2 is byte-neutral at 9/202 and adds no quantity, as predicted.
- verdict: KILLED
- kill_scope: class
- measured_on: HEAD c7aa37e7 (-mel -msoft-float), the s5 h1 body spliced into src/code6cac_b.c, one FAKE construct present (the `m` re-store)
- predicate_cite: tools/gcc-2.7.2/local-alloc.c:472

## [s9] Giving test 3 its own cross-product locals instead of reusing the outer kc/kp adds block-local quantities that break the dz/dx tie.
- mechanism: The outer `kc`/`kp` are multi-block and therefore invisible to block_alloc; fresh test-3 names become block-7 quantities and change what find_free_reg has handed out by the time the tied pair is reached.
- probe: Variant u1 (tmp/grind/func_8002D780/s9/variantsU/u1_fresh_kc_kp_locals.c) on the h1 order, scored with tools/sweep_variants.py and dumped with BB2_QTY_DEBUG.
- result: KILLED as a lever, banked as a free degree of freedom. u1 = 9/202, identical to the h1 baseline. The two new quantities do appear (qty14 reg141 birth 22 death 28 refs 4, qty15 reg136 birth 24 death 26 refs 2) but both are born after dx's death window, so they are seated on $v0 before the tied pair without ever conflicting with it; dz stays qty1 (4,16) and dx qty4 (8,20), still tied, still seated dz-first.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD c7aa37e7 (-mel -msoft-float), the s5 h1 body spliced into src/code6cac_b.c, one FAKE construct present (the `m` re-store)

## s10 (2026-09-08, rederive) — floor 2, residual relocated

## [s10] Test 3 can be written with NO named difference locals at all and still hold the 2/202 floor, and the resulting body's two residual instructions are a different pair with every register already correct.
- mechanism: the six differences are ordinary subexpressions; with them inline, RTL expansion creates the same six pseudos but in the operand-evaluation order of the two product expressions, which is what sets INSN_LUID and therefore (s8, sched.c:2408) the emission order of block 7.
- probe: batch W (tmp/grind/func_8002D780/s10/gen_w.py -> s10/variantsW) scored with tools/sweep_variants.py, then tools/pairdiff.py code6cac_b func_8002D780 on the s5 body, on w2 (fully inline) and on x4 (only dx and az named) via tmp/grind/func_8002D780/s10/showdiff.py.
- result: CONFIRMED. w2 = 2/202, w3 (edge named only) = 2, w4 (four offsets named) = 2, x4 (dx+az named) = 2 — all with the same diff: ours emits `subu $a0,$t0,$a3` (dz) then `subu $v0,$t2,$t1` (ax) where the target emits ax in the `bltz` delay slot and dz after it. The s5 body's residual is instead the `az`/`dx` transposition. In BOTH bodies all six values are already in the target's registers and all four multiplies match. Consequence for the eventual submission: the six named test-3 locals that s3-s9 flagged as an unresolved named-intermediate-vs-ordinary-C question are unnecessary, and memory/grind/func_8002D780/candidate.c no longer contains them (the s5 body is preserved verbatim as candidate_alt_s5_named_locals.c).
- verdict: CONFIRMED

## [s10] The dz/dx seat inversion is decided purely by which of the two has the longer local-alloc live range, and evaluating `(z2 - z0)` before `(cx - x0)` is the one-token source lever that decides it.
- mechanism: qty_compare_1 (local-alloc.c:1725-1758) ranks by floor_log2(refs)*refs*size/(death-birth)*10000 and falls back to quantity number. Moving dz's subu from block-7 index 1 to index 0 changes its span from 12 to 14 without changing its 3 references, dropping its priority from 2500 to 2142 and below dx's 2500.
- probe: instrumented cc1 (tools/gcc-2.7.2/cc1) with BB2_SUGG_DEBUG=1 BB2_QTY_DEBUG=1 through tmp/grind/func_8002D780/s10/dumpvar.py on the new body (tag dw2) and on y1, the same body with only the first product's operands flipped (tag dy1, 9/202); compared the blk=7 SUGGDBG-QTY tables.
- result: CONFIRMED. dw2: dz qty0 birth 2 death 16 refs 3 (span 14, pri 2142), ax qty1 birth 4 death 6 refs 2 (pri 10000), dx qty4 birth 8 death 20 refs 3 (span 12, pri 2500) -> dx seated first onto $v1, the target's seats. dy1: ax qty0 birth 2 death 6 (pri 5000), dz qty1 birth 4 death 16 (span 12, pri 2500), dx qty4 birth 8 death 20 (span 12, pri 2500) -> exact tie, quantity-number fallback seats dz on $v1, seats inverted. Every other row of the two tables (qty2/3 and qty5-14: the four multiply HI/LO scratch pairs and az/bx/bz) is identical, so the ax/dz evaluation order is the ONLY input that differs.
- verdict: CONFIRMED

## [s10] Flipping the first product's operand order so that ax is evaluated before dz reaches the target's block-7 emission order at no cost.
- mechanism: RTL expansion evaluates an `A * B` operand pair left to right, so `(cx - x0) * (z2 - z0)` emits ax's subu first, which is the target's order; the s8 class kill says emission order == LUID order for these six subus.
- probe: y1 (tmp/grind/func_8002D780/s10/variantsY/y1_w2_kc_first_flip.c), a one-token change from the 2-floor body, scored on the sandbox and dumped as tag dy1.
- result: KILLED. y1 = 9/202. The flip does deliver the target's ax-then-dz emission order, but by moving dz's def from block-7 index 0 to index 1 it restores the exact dz/dx priority tie (both refs 3, span 12, pri 2500) that s7/s9 typed, so the seats invert and the block costs 9 instructions instead of 2. The two effects are the same token: on this block geometry the target's emission order and the target's register seats were not both reachable by any of the 35 spellings measured this session.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD ce1ed95f (-mel -msoft-float), the s10 inline test-3 body spliced into src/code6cac_b.c, one FAKE construct present (the `m` re-store in the sqrt block)

## [s10] Flipping the C operand order of the four multiplications is a seat lever independent of evaluation order.
- mechanism: the RTL `(mult (reg A) (reg B))` operand order reaches the emitted `mult $rs,$rt`, and local-alloc's combine_regs walks an insn's operands in order, so a flip might change which quantity is examined first.
- probe: batch Z (tmp/grind/func_8002D780/s10/gen_z.py -> s10/variantsZ), 9 variants: the h1 named-local body with product 1, product 2, both kc products, both dz products, both dx products and all four flipped, plus three spellings that name ax alone and leave the rest inline.
- result: KILLED. In the ax-first regime a single flip is completely byte-neutral (z4 product-1 flipped = 9, z5 product-2 flipped = 9, z3 = 9, identical to the unflipped h1 body); flipping a PAIR costs 9-13 more (z6 = 18, z2 = 22, z7 = 22) by disturbing which pseudo cse keeps. Naming ax and leaving dz/dx/az/bx/bz inline (z1, z8, z9) lands in the same 9-regime as h1. No spelling in the batch reaches the target's seats with the target's emission order.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD ce1ed95f (-mel -msoft-float), the s5 h1 body and the s10 inline body spliced into src/code6cac_b.c, one FAKE construct present (the `m` re-store)

## [s10] Hoisting one of the six test-3 differences into the test-2 arm shortens block 7 and breaks the dz/dx tie without changing the emitted instruction set.
- mechanism: a value computed in block 6 becomes a multi-block pseudo, is skipped by block_alloc (local-alloc.c:470-478) and no longer occupies a block-7 slot, so every later birth/death index shifts and the two tied spans separate. This is the untried half of the s9 frontier item 2 ("take ax out of the competition").
- probe: batch B (tmp/grind/func_8002D780/s10/gen_b.py -> s10/variantsB), 8 variants: ax, az, bx, bz, dz, dx and the dz/dx pair each hoisted into the test-2 arm after its two cross products, plus one spelling with ax hoisted ahead of test 2's own arithmetic.
- result: KILLED, and expensively. b5 (az) = 20 at 203 insns, b2 (dz) = 21, b1 (ax) = 26, b7 = 26, b8 (bx) = 28 at 201, b4 (bz) = 32 at 201, b3 (dx) = 38 at 203, b6 (edge pair) = 39 at 203. Half the batch does not even keep build_insns at 202: a hoisted difference needs a register that lives across test 2's four multiplies, which forces block 6's own allocation to change and adds or drops instructions. The cheapest hoist is 18 instructions worse than doing nothing.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD ce1ed95f (-mel -msoft-float), the s10 inline test-3 body spliced into src/code6cac_b.c, one FAKE construct present (the `m` re-store)

## [s10] Rewriting the triangle tests around translated coordinates — in-place translation of cx/cz/px/pz or of x2/z2, or collapsing the four $v0-hosted offsets into one or two reused temps — reaches the target's block-7 register pattern more directly than six separate locals do.
- mechanism: the target keeps ax, az, bx and bz all in $v0 and x0/z0/cx/cz/px/pz/x2/z2 all live in $t0-$t5/$a2/$a3 across the whole test region, which reads like one temporary being rewritten four times; in-place translation would make the offsets reuse the pseudos they are computed from.
- probe: batch W variants w1/w1b (cx -= x0, cz -= z0, px -= x0, pz -= z0), w5/w5b (a single reused `t`), w6 (two reused temps `u`/`v`), and batch X variant x8 (x2 -= x0, z2 -= z0).
- result: KILLED. w1/w1b = 44, w5/w5b = 41, w6 = 26, x8 = 40, every one at 202 insns. In-place translation destroys the seats outright: the offsets inherit the multi-block pseudos' globally allocated $t2/$a2/$t3/$t4 instead of taking $v0, so all four subus and all four multiplies differ. A single reused temp does produce ONE $v0-hosted pseudo spanning the whole block, but its priority (refs 8 over span 18, pri 13333) puts it first and the dz/dx pair still ties behind it. The target's $v0 reuse is the allocator's doing, not the source's.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD ce1ed95f (-mel -msoft-float), the s5 h1 body and the s10 inline body spliced into src/code6cac_b.c, one FAKE construct present (the `m` re-store)

## [s10] The basic-block structure of the three triangle tests is a lever on which insns land in block 7, so a different control-flow spelling can change the block's shape without changing the instruction set.
- mechanism: frontier item 3 for s11 rests on the possibility that the original source put block 7's fourteen insns into a differently-shaped CFG; jump1 normalises `if`/`goto`/`&&` spellings, but only up to the point where the arms differ.
- probe: batch C (tmp/grind/func_8002D780/s10/gen_c.py -> s10/variantsC), 5 variants on the s10 inline body: (c1) each test inverted into a `goto sqrt_path` early exit with the tests un-nested, (c2) test 3 written entirely inside its `if` condition with no kc/kp assignment at all, (c3) tests 2 and 3 chained with `&&` using embedded assignments, (c4) the innermost `if` inverted with a `goto` and a bare `return 1`, (c5) fresh `c3`/`p3` locals with inline differences.
- result: KILLED as a lever, banked as a free degree of freedom. All five score 2/202 with build_insns 202 -- byte-identical output to the candidate. jump1 collapses every one of these spellings onto the same CFG, so control-flow shape in the test region costs nothing and buys nothing. A future probe may therefore pick whichever of these shapes it likes (including c2, which removes the kc/kp assignments from test 3 entirely) without paying for it, but block structure is not the missing input to the dz/dx tie.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD ce1ed95f (-mel -msoft-float), the s10 inline test-3 body spliced into src/code6cac_b.c, one FAKE construct present (the `m` re-store)

## Live frontier (for s11) — still 2 insns, but the residual is now a single adjacent-pair emission swap with every register already correct

1. **One extra instruction between dz's definition and dx's definition, deleted after
   local_alloc.** This is the arithmetic s10 pinned: on the target's emission order dz
   spans block-7 indices i1->i7 and dx spans i3->i9, six slots each, refs 3 each, hence
   the tie. One extra insn in the window between dz's def and dx's def — the single slot
   i2 — makes dz span 14 against dx's 12 and seats dx first, exactly as the s10 body's
   earlier dz birth does, but WITHOUT costing the ax/dz order. The insn must vanish after
   local_alloc, so it must be a reg-reg copy removed by jump2's delete_noop_moves
   (toplev.c:3142). s8's class kill (cse.c:1032) says such a copy cannot survive if both
   of its definitions sit inside block 7, and s8's batch R priced a cross-block carrier
   manufactured out of test 2's own arithmetic at +7/+11. What has NOT been tried is a
   carrier whose second definition sits in the TEST-1 arm or in the outer block (the two
   blocks that already assign `kc` and `kp`), so that test 2's codegen is untouched.
   Next probe: on the ax-first (y1) body add a carrier declared in the outer test block,
   defined once there or in the test-1 arm, and re-assigned inside test 3 from a block-7
   value that dies at the copy; BEFORE scoring run tmp/grind/func_8002D780/s10/dumpvar.py
   and require the blk=7 SUGGDBG-QTY table to show dz death 18 (span 14) with dx still
   birth 8 / death 20 (span 12); only then score, requiring build_insns == 202.
2. **A decomp-permuter campaign on the s10 chassis.** Inherited unspent since s5, and it
   has never been this well posed: the residual is now ONE transposition of two adjacent
   `subu` instructions with every register, every multiply and the whole rest of the
   function already byte-correct, which is precisely the permuter's statement-reordering
   move set. Regenerate base.c from the new memory/grind/func_8002D780/candidate.c AND
   from tmp/grind/func_8002D780/s10/variantsY/y1_w2_kc_first_flip.c into the validated
   workspace tmp/grind/func_8002D780/s4/nonmatchings/func_8002D780 (four hurdles + rebuild
   recipe in evidence.md [s4]), launch with tools/permuter_campaign.py, wait IN-TURN with
   `permuter_campaign.py wait --dir <ws>`, harvest with --stop before the turn ends, and
   re-score every find on the engine sandbox (s4's finding that the permuter's weighted
   metric is anti-correlated with the engine's here still applies).
3. **Re-examine whether block 7 really is fourteen RTL insns in the original compile.**
   Every kill from s6 onward assumes the target's block 7 holds exactly the fourteen insns
   we emit, which forces the dz/dx tie and leaves the original's dx-first seat unexplained.
   The alternative is that the original source put one of the six differences, or some
   other value, somewhere that changes the block's SHAPE at local-alloc time without
   changing the final 202 instructions. s10 killed the obvious version (hoisting a
   difference into the test-2 arm, 20-39). The untried version changes which insns are in
   which basic block rather than which insns exist: the three side tests written as one
   `&&`-chained condition, an early-return inversion of each `if`, or the test-2 and
   test-3 arms merged. Next probe: sweep that block-structure axis and read the .lreg
   dump's basic-block boundaries, not only the score.

## [s10] Test 3 can be written with no named difference locals at all and still hold the 2/202 floor, and the resulting body's two residual instructions are a different pair with every register already correct.
- mechanism: The six differences are ordinary subexpressions; inline, RTL expansion creates the same six pseudos but in the operand-evaluation order of the two product expressions, which sets INSN_LUID and therefore (s8, sched.c:2408) block 7's emission order.
- probe: Batch W (tmp/grind/func_8002D780/s10/gen_w.py -> s10/variantsW) scored with tools/sweep_variants.py, then tools/pairdiff.py code6cac_b func_8002D780 on the s5 body, on w2 (fully inline) and on x4 (only dx and az named) via s10/showdiff.py.
- result: CONFIRMED. w2 fully inline = 2/202, w3 (only dz/dx named) = 2, w4 (only the four $v0 offsets named) = 2, x4 (only dx and az named) = 2. All four show the SAME diff, and it is not the s5 body's diff: ours emits `subu $a0,$t0,$a3` (dz) then `subu $v0,$t2,$t1` (ax) where the target emits ax in the bltz delay slot at 8002D910 and dz right after; the dx/az pair, all four multiplies and every register assignment already match. The s5 body's residual is instead the az/dx transposition. Consequence: the six named test-3 locals that s3-s9 flagged as needing a named-intermediate-vs-ordinary-C ruling are unnecessary, and the new candidate.c does not contain them, so that family question no longer blocks a candidate-ready submission.
- verdict: CONFIRMED

## [s10] The dz/dx seat assignment is decided purely by which of the two has the longer local-alloc live range, and evaluating (z2 - z0) before (cx - x0) is the one-token source lever that decides it.
- mechanism: qty_compare_1 (tools/gcc-2.7.2/local-alloc.c:1725-1758) ranks block-7 quantities by floor_log2(refs)*refs*size/(death-birth)*10000 and falls back to quantity number. Moving dz's subu from block-7 index 1 to index 0 changes its span from 12 to 14 without changing its 3 references, dropping its priority from 2500 to 2142, below dx's 2500.
- probe: Instrumented cc1 (tools/gcc-2.7.2/cc1) with BB2_SUGG_DEBUG=1 BB2_QTY_DEBUG=1 through tmp/grind/func_8002D780/s10/dumpvar.py on the new body (tag dw2) and on y1, the same body with only the first product's operands flipped (tag dy1, 9/202); compared the blk=7 SUGGDBG-QTY tables.
- result: CONFIRMED. dw2: dz qty0 birth 2 death 16 refs 3 (span 14, pri 2142), ax qty1 birth 4 death 6 refs 2 (pri 10000), dx qty4 birth 8 death 20 refs 3 (span 12, pri 2500) -> dx seated first onto $v1, the target's seats. dy1: ax qty0 birth 2 death 6 (pri 5000), dz qty1 birth 4 death 16 (span 12, pri 2500), dx qty4 birth 8 death 20 (span 12, pri 2500) -> exact tie, quantity-number fallback seats dz on $v1, seats inverted. Every other row of the two tables (qty2/3 and qty5-14: the four multiply HI/LO scratch pairs and az/bx/bz) is identical, so the ax/dz evaluation order is the ONLY differing input.
- verdict: CONFIRMED

## [s10] Flipping the first product's operand order so that ax is evaluated before dz reaches the target's block-7 emission order at no cost.
- mechanism: RTL expansion evaluates an A * B operand pair left to right, so (cx - x0) * (z2 - z0) emits ax's subu first, which is the target's order; the s8 class kill establishes that emission order equals LUID order for these six subus.
- probe: y1 (tmp/grind/func_8002D780/s10/variantsY/y1_w2_kc_first_flip.c), a one-token change from the 2-floor body, scored on the engine sandbox and dumped as tag dy1.
- result: KILLED. y1 = 9/202 at 202 insns. The flip does deliver the target's ax-then-dz emission order, but by moving dz's def from block-7 index 0 to index 1 it restores the exact dz/dx priority tie (both refs 3, span 12, pri 2500) that s7/s9 typed, so the seats invert and the block costs 9 instructions instead of 2. Emission order and register seats are driven by the same token: across the 35 spellings measured this session, none reached both.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD ce1ed95f (-mel -msoft-float), the s10 inline test-3 body spliced into src/code6cac_b.c, one FAKE construct present (the m re-store in the sqrt block)

## [s10] Flipping the C operand order of the four multiplications is a seat lever independent of evaluation order.
- mechanism: The RTL (mult (reg A) (reg B)) operand order reaches the emitted mult $rs,$rt, and local-alloc's combine_regs walks an insn's operands in order, so a flip might change which quantity is examined first.
- probe: Batch Z (tmp/grind/func_8002D780/s10/gen_z.py -> s10/variantsZ), 9 variants: the h1 named-local body with product 1, product 2, both kc products, both dz products, both dx products and all four flipped, plus three spellings naming ax alone with the rest inline.
- result: KILLED. In the ax-first regime a single flip is byte-neutral (z4 = 9, z5 = 9, z3 = 9, identical to the unflipped h1 body); flipping a pair costs 9-13 more (z6 = 18, z2 = 22, z7 = 22) by disturbing which pseudo cse keeps. Naming ax and leaving dz/dx/az/bx/bz inline (z1, z8, z9) lands in the same 9-regime as h1. No spelling in the batch reaches the target's seats with the target's emission order.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD ce1ed95f (-mel -msoft-float), the s5 h1 body and the s10 inline body spliced into src/code6cac_b.c, one FAKE construct present (the m re-store)

## [s10] Hoisting one of the six test-3 differences into the test-2 arm shortens block 7 and breaks the dz/dx tie without changing the emitted instruction set.
- mechanism: A value computed in block 6 becomes a multi-block pseudo, is skipped by block_alloc (local-alloc.c:470-478) and no longer occupies a block-7 slot, so every later birth/death index shifts and the two tied spans separate. This is the untried half of the s9 frontier item 2.
- probe: Batch B (tmp/grind/func_8002D780/s10/gen_b.py -> s10/variantsB), 8 variants: ax, az, bx, bz, dz, dx and the dz/dx pair each hoisted into the test-2 arm after its two cross products, plus one spelling with ax hoisted ahead of test 2's own arithmetic.
- result: KILLED, and expensively. b5 (az) = 20 at 203 insns, b2 (dz) = 21, b1 (ax) = 26, b7 = 26, b8 (bx) = 28 at 201, b4 (bz) = 32 at 201, b3 (dx) = 38 at 203, b6 (edge pair) = 39 at 203. Half the batch does not even keep build_insns at 202: a hoisted difference needs a register living across test 2's four multiplies, which forces block 6's own allocation to change and adds or drops instructions. The cheapest hoist is 18 instructions worse than doing nothing.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD ce1ed95f (-mel -msoft-float), the s10 inline test-3 body spliced into src/code6cac_b.c, one FAKE construct present (the m re-store)

## [s10] Rewriting the triangle tests around translated coordinates -- in-place translation of cx/cz/px/pz or of x2/z2, or collapsing the four $v0-hosted offsets into one or two reused temps -- reaches the target's block-7 register pattern more directly than six separate locals do.
- mechanism: The target keeps ax, az, bx and bz all in $v0 and x0/z0/cx/cz/px/pz/x2/z2 all live in $t0-$t5/$a2/$a3 across the whole test region, which reads like one temporary rewritten four times; in-place translation would make the offsets reuse the pseudos they are computed from.
- probe: Batch W variants w1/w1b (cx -= x0, cz -= z0, px -= x0, pz -= z0), w5/w5b (a single reused t), w6 (two reused temps u/v), and batch X variant x8 (x2 -= x0, z2 -= z0).
- result: KILLED. w1/w1b = 44, w5/w5b = 41, w6 = 26, x8 = 40, every one at 202 insns. In-place translation destroys the seats outright: the offsets inherit the multi-block pseudos' globally allocated $t2/$a2/$t3/$t4 instead of taking $v0, so all four subus and all four multiplies differ. A single reused temp does produce one $v0-hosted pseudo spanning the whole block, but its priority (refs 8 over span 18, pri 13333) puts it first and the dz/dx pair still ties behind it. The target's $v0 reuse is the allocator's doing, not the source's.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD ce1ed95f (-mel -msoft-float), the s5 h1 body and the s10 inline body spliced into src/code6cac_b.c, one FAKE construct present (the m re-store)

## [s10] The basic-block structure of the three triangle tests is a lever on which insns land in block 7, so a different control-flow spelling can change the block's shape without changing the instruction set.
- mechanism: Frontier item 3 rests on the possibility that the original source put block 7's fourteen insns into a differently-shaped CFG; jump1 normalises if/goto/&& spellings, but only up to the point where the arms differ.
- probe: Batch C (tmp/grind/func_8002D780/s10/gen_c.py -> s10/variantsC), 5 variants on the s10 inline body: each test inverted into a goto-based early exit with the tests un-nested; test 3 written entirely inside its if condition with no kc/kp assignment; tests 2 and 3 chained with && using embedded assignments; the innermost if inverted with a goto and a bare return 1; and fresh c3/p3 locals.
- result: KILLED as a lever, banked as a free degree of freedom. All five score 2/202 at 202 insns, byte-identical to the candidate. jump1 collapses every one of these spellings onto the same CFG, so control-flow shape in the test region costs nothing and buys nothing. A future probe may pick whichever of these shapes it likes (including the one that removes the kc/kp assignments from test 3 entirely) without paying for it.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD ce1ed95f (-mel -msoft-float), the s10 inline test-3 body spliced into src/code6cac_b.c, one FAKE construct present (the m re-store)

# s11 (2026-09-08, rederive) — floor 2/202, residual re-attributed to reorg.c

## [s11] Kill re-audit: the s10 candidate's `m` re-store is un-annotated, and ablating it re-measures the carrier's worth on the current chassis.
- mechanism: tools/fake_ablate.py keys on the `/* FAKE: ... */` marker; the s10 body claimed the construct in its header comment but never emitted the annotation, so the ablation tool reported nothing to ablate and every s6-s10 "FAKE present" caveat was unverifiable by tooling.
- probe: ran fake_ablate.py on memory/grind/func_8002D780/candidate.c (reported "no FAKE-annotated constructs found"), then hand-built two variants — a1_no_m_restore.c (the inner `m = dist;` deleted) and a2_annotated_m.c (re-store plus the annotation) — and scored both on the sandbox.
- result: a1 = 6/202, a2 = 2/202, candidate = 2/202. The re-store is still worth 4 insns on HEAD 4e56c8b9 and the annotation is codegen-neutral. a1 banked as rejected/m-restore-ablated-still-load-bearing-6.c; candidate.c now ships the annotation.
- verdict: CONFIRMED

## [s11] The last two instructions are decided by reorg.c's delay-slot fill for the test-2 exit branch, not by local-alloc.
- mechanism: the two transposed insns straddle the delay slot of `bltz $v0,.L8002D964` at 8002D90C. reorg.c's fill_slots_from_thread scans the fall-through thread and takes the first insn clearing five gates (references-set / sets-set / sets-needed / sets-a-resource-needed-at-the-opposite-thread / may_trap_p), then goes to `winner` (tools/gcc-2.7.2/reorg.c:3480).
- probe: instrumented cc1 with BB2_DBR_DEBUG=1 on the candidate (tmp/grind/func_8002D780/s11/dbr1/stderr_full.txt) and read the trace for the branch, uid 175.
- result: `DBRDBG thr insn=175 trial=179 refset=0 setset=0 setneed=0 setsopp=0 trap=0` followed immediately by `DBRDBG thr WINNER insn=175 trial=179 annul=0`. Uid 179 is dz's subu, the first insn of block 7; it clears every gate and takes the slot with no competition. `oppregs=20010020_00000000` decodes to $a1/$s0/$s1/$sp live at the branch target, so $a0 is dead there and `setsopp` can never be 1 for dz.
- verdict: CONFIRMED

## [s11] The ax/dz emission order and the dz/dx register seat are two readings of the same INSN_LUID, so no re-ordering of the six test-3 difference expressions satisfies both.
- mechanism: sched.c rank_for_schedule compares INSN_PRIORITY, then dependence class, then falls to `INSN_LUID (tmp) - INSN_LUID (tmp2)` (tools/gcc-2.7.2/sched.c:2464). dz's subu and ax's subu are both inputs of the same mult, so their longest-path priorities are equal by construction and both classify as 3; the LUID line therefore decides in both sched1 and sched2, and sched2's LUIDs come from sched1's output stream. local-alloc then reads that same order as its birth order, and on the target's block-7 geometry dz and dx have 3 refs and an eight-slot span each, so qty_compare_1 ties them and the quantity-number fallback (tools/gcc-2.7.2/local-alloc.c:1684) seats whichever was born first.
- probe: BB2_RANK_DEBUG=1 trace (s11/rank2/stderr_full.txt) for the pair; uid-window comparison of the .lreg / .greg / .jump2 / .sched2 streams for the candidate; and the cc1 .s output of both the dz-first candidate (s11/cand) and the ax-first body y1_w2_kc_first_flip.c (s11/y1).
- result: `RANKDBG last=183 y=181 cls=3 x=179 cls2=3 val=0` — the print is only reached when the priority comparison did not return, so the pair is priority-tied and class-tied. All four dumps show the same relative order `[175, 179, 181, ...]`, so sched2 never decouples the emission order from the order local-alloc saw. The two cc1 outputs are the two halves of the contradiction: dz-first puts dz in the slot (wrong) with dz in $a0 and dx in $v1 (right); ax-first puts ax in the slot (right) with dz in $v1 and dx in $a0 (wrong, 9/202).
- verdict: KILLED
- kill_scope: class
- predicate_cite: tools/gcc-2.7.2/sched.c:2464
- measured_on: HEAD 4e56c8b9 (-mel -msoft-float), the s10/s11 candidate body and y1_w2_kc_first_flip.c spliced into src/code6cac_b.c, one FAKE construct present (the `m` re-store)

## [s11] The extra local-alloc insn residual A needs can be paid for out of ASPSX mflo->mult hazard-pad slack, so it costs no machine instruction.
- mechanism: ASPSX inserts a nop when fewer than two instructions separate an `mflo` from the next `mult`, so an RTL insn dropped into such a gap displaces the pad and leaves the machine-instruction count unchanged. The target's block 7 is 20 machine instructions of which two (8002D934, 8002D944) are exactly those pads, i.e. 18 RTL insns.
- probe: mapped the target's block 7 to RTL indices (i0 ax, i1 dz, i2 mult, i3 mflo, i4 dx, i5 az, i6 mult, i7 mflo, i8 bx, i9 mult, i10 mflo, i11 bz, i12 mult, i13 kc, i14 mflo, i15 kp, i16 xor, i17 branch), located the two pad gaps (i8->i9 and i11->i12), and recomputed the qty_compare_1 spans for an insn inserted at each gap and at both.
- result: both pads lie AFTER dx's birth at i4, so spending them moves the spans the wrong way — before i9 gives dz 9 / dx 9 (still a tie, dz seated first), before i12 gives dz 8 / dx 9 (dz priority higher), both gives dz 9 / dx 10 (dz priority higher). The gap that would work, strictly between i1 and i4, has no pad (mflo i3 already has two insns before mult i6), so an insn there costs +1 and produces 203.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 4e56c8b9 (-mel -msoft-float), the s11 candidate body in src/code6cac_b.c, one FAKE construct present (the `m` re-store); pad positions read from asm/funcs/func_8002D780.s and from the `#nop` markers in tmp/grind/func_8002D780/s11/cand/code6cac_b.s

## [s11] reorg can be made to pass over dz's subu and take ax for the slot instead, by making one of its five gates fail.
- mechanism: fill_slots_from_thread does not stop at an unusable trial when it owns the thread (`! stop_search_p (...) && (! lose || own_thread)`, reorg.c:3399-3401); it records the loss and continues, so if dz's subu failed a gate the loop would reach ax's subu next and fill the slot with it — which, on the dz-first body that already has the target's seats, is exactly the target.
- probe: read the five gate values reorg actually computed for the candidate (BB2_DBR_DEBUG trace, s11/dbr1) and checked each against what C could change. refset/setset/setneed are all vacuous for the first insn of a thread. may_trap_p is 1 only for a MEM, and dz's insn is a register subtract. setsopp needs dz's destination live at the branch target, and the target's own sqrt block at .L8002D964 writes $a0 at 8002D980 before any read of it.
- result: all five gates are structurally clear for a register-to-register subtract at the head of the thread; the trace shows `refset=0 setset=0 setneed=0 setsopp=0 trap=0` and an immediate WINNER. No C spelling of the six test-3 differences makes dz's subu fail a gate without changing the instruction that lands there.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 4e56c8b9 (-mel -msoft-float), the s11 candidate body in src/code6cac_b.c, one FAKE construct present (the `m` re-store)

## Live frontier (for s12) — still 2 insns, and the door is now named exactly

1. **Make one of dz / dx a GLOBAL (cross-block) quantity so global.c seats it instead of local-alloc.**
   mechanism: local-alloc only forms a quantity for a pseudo whose entire live range sits inside one
   basic block; anything live across a block boundary is left to global_alloc, which ranks by a
   completely different allocno priority and never consults qty_compare_1 or its quantity-number
   fallback. That is the only measured escape from the s11 contradiction, because it removes one of
   the two tied quantities from the tie altogether while leaving its defining subu inside block 7.
   next probe: spell test 3 so that one of `(z2 - z0)` / `(x2 - x0)` is ALSO consumed after the three
   triangle tests (for example as the value stored to obj+0x118 / obj+0x124 in the sqrt block, which
   already computes `x0 - px` and `x2 - px` style differences), so the pseudo is live at the join
   while its subu still sits at block-7 index 1 or 4. Read the .lreg dump FIRST: require the blk=7
   QTYDBG table to show only ONE of dz/dx present (the other having been handed to global_alloc) and
   the .greg dump to show the missing one allocated; only then score, requiring build_insns == 202.

2. **An RTL insn born strictly between dz's def and dx's def that disappears between local_alloc and final.**
   mechanism: unchanged from s10 frontier 1 and still the arithmetic that works (dz span 14 against
   dx's 12), but s11 adds two facts. First, the ASPSX pad route is closed (both pads are on the wrong
   side of dx's birth, s11 kill above), so the insn really must vanish rather than be absorbed.
   Second, reorg's `redundant_insn` deletion (reorg.c:3439-3462) is a SECOND late deleter beside
   jump2's delete_noop_moves — but it only fires on insns the thread scan actually reaches, and the
   scan stops at the first winner, so it cannot reach index 2 or 3. That leaves delete_noop_moves,
   whose cse survival problem s8 class-killed for copies defined wholly inside block 7.
   next probe: only worth spending after frontier 1; if it is spent, the shape needed is a copy whose
   two definitions straddle the test-2 branch AND whose block-7 definition lands at index 2 or 3.

3. **A decomp-permuter campaign on the s11 chassis.**
   mechanism: unchanged from s10 frontier 2 — the residual is one transposition of two adjacent subus
   with every register already correct, which is squarely in the permuter's move set, and the s4
   campaign ran on a 7-floor chassis dominated by residuals B and C. s11 lowers the prior: the
   transposition is now known to be a single LUID whose two consumers pull opposite ways, so a
   permuter win would have to arrive via frontier 1's shape rather than by shuffling statements.
   next probe: regenerate base.c from memory/grind/func_8002D780/candidate.c into the validated
   workspace tmp/grind/func_8002D780/s4/nonmatchings/func_8002D780 (four hurdles + rebuild recipe in
   evidence.md [s4]), launch with tools/permuter_campaign.py, wait IN-TURN with
   `permuter_campaign.py wait --dir <ws>`, harvest with --stop before the turn ends, and re-score
   every find on the engine sandbox.

## [s11] Splitting test 3 on the sign of kc makes dz and dx cross-block quantities, handing their seats to global_alloc instead of local-alloc's tied qty_compare_1.
- mechanism: local-alloc only forms a quantity for a pseudo whose whole live range sits in one basic block (the s11 contradiction is entirely a local-alloc phenomenon), so branching on `kc < 0` between the two cross products leaves dz and dx live across the join and hands them to global.c's allocno priority model, which never consults qty_compare_1 or its quantity-number fallback.
- probe: two spellings measured on the sandbox and dumped — g1_split_block_global_dz_dx.c (named dz/dx locals, the kp product duplicated into both arms of an `if (kc < 0)`) and g2_split_block_inline_dup.c (the same shape with the differences left inline). Register dispositions read from the .greg dumps (tmp/grind/func_8002D780/s11/cand vs s11/g1).
- result: KILLED as spelled. g1 = 36/212, g2 = 37/212 — jump2 does NOT cross-jump the duplicated kp arithmetic back together, so the split costs 10 machine instructions before any seat question is reached. The seats do move (candidate: reg129 dz in $a0, reg130 ax in $v0, reg132 dx in $v1; g1: reg129 in $v1, reg130 in $a0, reg132 in $t1), which confirms the mechanism is live, but they move the wrong way and the +10 insn cost is fatal. Any future attempt at this route must find a split that jump2 tail-merges back to 202.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 4e56c8b9 (-mel -msoft-float), g1/g2 spliced into src/code6cac_b.c, one FAKE construct present (the `m` re-store)

## [s11] The ax/dz emission order and the dz/dx register seat are two readings of the same INSN_LUID, so re-ordering the six test-3 difference expressions cannot satisfy both.
- mechanism: sched.c rank_for_schedule compares INSN_PRIORITY, then dependence class, then falls to INSN_LUID. dz's subu and ax's subu are both inputs of the same mult, so their longest-path priorities are equal by construction and both classify as 3; the LUID line decides in sched1 and again in sched2 (whose LUIDs come from sched1's output stream). local-alloc reads that same order as its birth order, and on the target's block-7 geometry dz and dx have 3 refs and an eight-slot span each, so qty_compare_1 ties them and the quantity-number fallback at local-alloc.c:1684 seats whichever was born first.
- probe: BB2_RANK_DEBUG=1 instrumented cc1 trace on the candidate (tmp/grind/func_8002D780/s11/rank2/stderr_full.txt); uid-window comparison of the .lreg / .greg / .jump2 / .sched2 streams; cc1 .s output of the dz-first candidate (s11/cand) and the ax-first body y1_w2_kc_first_flip.c (s11/y1).
- result: RANKDBG last=183 y=181 cls=3 x=179 cls2=3 val=0 - the class print is only reached when the priority comparison did not return, so uids 179 (dz) and 181 (ax) are priority-tied and class-tied. All four dumps show the same relative order [175, 179, 181, ...], so sched2 never decouples the post-reload emission order from the order local-alloc saw. The two cc1 outputs are the two halves of the contradiction: dz-first puts dz in the delay slot (wrong) with dz in $a0 and dx in $v1 (right, 2/202); ax-first puts ax in the slot (right, matching the target) with dz in $v1 and dx in $a0 (wrong, 9/202).
- verdict: KILLED
- kill_scope: class
- measured_on: HEAD 4e56c8b9 (-mel -msoft-float), the s10/s11 candidate body and s10/variantsY/y1_w2_kc_first_flip.c spliced into src/code6cac_b.c, one FAKE construct present (the `m` re-store)
- predicate_cite: tools/gcc-2.7.2/sched.c:2464

## [s11] The last two instructions are decided by reorg.c's delay-slot fill for the test-2 exit branch, not by a local-alloc seat as sessions s6-s10 assumed.
- mechanism: The two transposed insns straddle the delay slot of `bltz $v0,.L8002D964` at 8002D90C. reorg.c's fill_slots_from_thread walks the fall-through thread and takes the first insn clearing five gates (references-set, sets-set, sets-needed, sets-a-resource-needed-at-the-opposite-thread, may_trap_p), then goes to the winner label at reorg.c:3480.
- probe: Instrumented cc1 with BB2_DBR_DEBUG=1 on the candidate; read the trace for the branch (uid 175) in tmp/grind/func_8002D780/s11/dbr1/stderr_full.txt.
- result: CONFIRMED. `DBRDBG thr insn=175 trial=179 refset=0 setset=0 setneed=0 setsopp=0 trap=0` followed immediately by `DBRDBG thr WINNER insn=175 trial=179 annul=0`. Uid 179 is dz's subu, the head of block 7; it clears every gate and takes the slot uncontested. oppregs=20010020_00000000 decodes to $a1/$s0/$s1/$sp live at the branch target, so $a0 is dead there and setsopp can never be 1 for dz. The target's slot holds ax, so the target's pre-reorg block 7 began with ax.
- verdict: CONFIRMED

## [s11] The extra local-alloc insn that residual A needs can be paid for out of ASPSX mflo->mult hazard-pad slack, so it costs no machine instruction.
- mechanism: ASPSX inserts a nop when fewer than two instructions separate an mflo from the next mult, so an RTL insn dropped into such a gap displaces the pad and leaves the machine-instruction count unchanged. The target's block 7 is 20 machine instructions of which two (8002D934, 8002D944) are exactly those pads, i.e. 18 RTL insns.
- probe: Mapped the target's block 7 to RTL indices (i0 ax, i1 dz, i2 mult, i3 mflo, i4 dx, i5 az, i6 mult, i7 mflo, i8 bx, i9 mult, i10 mflo, i11 bz, i12 mult, i13 kc, i14 mflo, i15 kp, i16 xor, i17 branch), located the two pad gaps (i8->i9 and i11->i12) from asm/funcs/func_8002D780.s and the `#nop` markers in the cc1 .s output, and recomputed the qty_compare_1 spans for an insn inserted at each gap and at both.
- result: KILLED. Both pads lie after dx's birth at i4, so spending them moves the spans the wrong way: before i9 gives dz span 9 / dx span 9 (still a tie, dz seated first), before i12 gives dz 8 / dx 9 (dz priority higher), both gives dz 9 / dx 10 (dz priority higher). The gap that would work, strictly between i1 and i4, has no pad (mflo i3 already has two insns before mult i6), so an insn there costs +1 and leaves 203.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 4e56c8b9 (-mel -msoft-float), the s11 candidate body in src/code6cac_b.c, one FAKE construct present (the `m` re-store)

## [s11] reorg can be made to pass over dz's subu and take ax for the slot instead, by making one of its five gates fail on the dz-first body that already has the target's seats.
- mechanism: fill_slots_from_thread does not stop at an unusable trial when it owns the thread (`! stop_search_p (...) && (! lose || own_thread)`); it records the loss and continues, so if dz's subu failed a gate the loop would reach ax's subu next and fill the slot with it - which on the dz-first body is exactly the target's byte sequence.
- probe: Read the five gate values reorg actually computed for the candidate (BB2_DBR_DEBUG trace, s11/dbr1) and checked each against what C can change; checked $a0's liveness at .L8002D964 in the target's own asm.
- result: KILLED. refset/setset/setneed are vacuous for the first insn of a thread; may_trap_p is 1 only for a MEM and dz's insn is a register subtract; setsopp needs dz's destination live at the branch target, and the target's own sqrt block writes $a0 at 8002D980 before any read of it. The trace shows all five gates clear and an immediate WINNER, so no C spelling of the six test-3 differences makes dz's subu fail a gate without changing which instruction lands there.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 4e56c8b9 (-mel -msoft-float), the s11 candidate body in src/code6cac_b.c, one FAKE construct present (the `m` re-store)

## [s11] Splitting test 3 on the sign of kc makes dz and dx cross-block quantities, handing their seats to global_alloc instead of local-alloc's tied qty_compare_1.
- mechanism: local-alloc only forms a quantity for a pseudo whose whole live range sits in one basic block, so branching on kc < 0 between the two cross products leaves dz and dx live across the join and hands them to global.c's allocno priority model, which never consults qty_compare_1 or its quantity-number fallback.
- probe: Two spellings measured on the sandbox and dumped: g1_split_block_global_dz_dx.c (named dz/dx locals, the kp product duplicated into both arms of an `if (kc < 0)`) and g2_split_block_inline_dup.c (same shape, differences inline). Register dispositions read from the .greg dumps (s11/cand vs s11/g1).
- result: KILLED as spelled. g1 = 36/212, g2 = 37/212 - jump2 does not cross-jump the duplicated kp arithmetic back together, so the split costs 10 machine instructions before any seat question is reached. The seats do move (candidate: reg129 dz in $a0, reg130 ax in $v0, reg132 dx in $v1; g1: reg129 in $v1, reg130 in $a0, reg132 in $t1), confirming the mechanism is live, but they move the wrong way. Banked as rejected/split-block-kc-sign-no-crossjump-36.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD 4e56c8b9 (-mel -msoft-float), g1/g2 spliced into src/code6cac_b.c, one FAKE construct present (the `m` re-store)

## [s11] Kill re-audit: the s10 candidate's `m` same-value re-store shipped without its FAKE annotation, and ablating it re-measures the carrier's worth on the current chassis.
- mechanism: tools/fake_ablate.py keys on the /* FAKE: ... */ marker; the s10 body claimed the construct in its header comment but never emitted the annotation, so the ablation tool reported nothing to ablate and every s6-s10 'FAKE present' caveat was unverifiable by tooling. An un-annotated FAKE is also an automatic Judge FAIL, so this was a latent blocker on any future candidate-ready.
- probe: Ran fake_ablate.py on memory/grind/func_8002D780/candidate.c, then hand-built a1_no_m_restore.c (inner `m = dist;` deleted) and a2_annotated_m.c (re-store plus the annotation) and scored both on the sandbox.
- result: CONFIRMED that the carrier is still load-bearing: fake_ablate.py reported 'no FAKE-annotated constructs found'; a1 = 6/202, a2 = 2/202, candidate = 2/202. The re-store is worth 4 insns on HEAD 4e56c8b9 and the annotation itself is codegen-neutral. a1 banked as rejected/m-restore-ablated-still-load-bearing-6.c; candidate.c now ships the annotation.
- verdict: CONFIRMED

## [s12] Emitting `ax` first (as a named local at the top of test 3) hands reorg the target's delay-slot insn.
- mechanism: reorg.c's fill_slots_from_thread takes the first insn of the fall-through thread that clears its five gates, and the head of block 7 is whichever difference the source evaluates first. Declaring `s32 ax = cx - x0;` before the first cross product emits ax's subu at block-7 index 0 while leaving the mult operand order `mult $a0,$v0` (dz * ax) intact, unlike the s10 y1 variant which flipped the product's operands.
- probe: variant a1_ax_local_in_t3.c scored on the sandbox and dumped with the instrumented cc1 (BB2_QTY_DEBUG/BB2_SUGG_DEBUG/BB2_DBR_DEBUG, tmp/grind/func_8002D780/s12/a1/), then pairdiffed.
- result: 9/202 at 202 insns. pairdiff shows ours[92] = target[92] = `subu v0,t2,t1` (ax in the slot) — the delay slot is CORRECT for the first time — and the only cluster left is the dz/dx seat swap (ours dz $v1 / dx $a0, target dz $a0 / dx $v1). Banked as rejected/axfirst-slot-right-seats-swapped-9.c.
- verdict: CONFIRMED

## [s12] On an ax-first body, one extra RTL insn strictly between dz's definition and dx's definition breaks the qty_compare_1 tie in dx's favour and restores the target's seats at zero instruction cost.
- mechanism: local-alloc.c:1725-1758 ranks by floor_log2(refs)*refs*size/(death-birth)*10000 with a quantity-number tie-break at local-alloc.c:1684. An insn inserted between dz's birth and dx's birth shifts dz's death by +2 (span 12 -> 14) while shifting dx's birth AND death together (span unchanged at 12), so dx's priority 2500 beats dz's 2142, dx is seated first and takes $v1, and dz takes $a0. The insn is free when it is a subu the target emits anyway, merely scheduled earlier by sched1 than it is emitted by sched2.
- probe: hoisted each of the four remaining test-3 differences to a named local declared immediately after `dz` and scored all of them (d1 az, d2 bz, d3 bx, d4 az+bz, d5 dx), then read the block-7 QTYDBG tables for a1 and c1 to check the span arithmetic against the model.
- result: CONFIRMED for `az`. d1_az_hoisted = 2/202 at 202 insns with every register in the function correct, the delay slot correct, and the residual reduced to the adjacent az/dx transposition at indices 96/97. d4 (az+bz) = 2/202 with the same residual. d2 = d3 = d5 = 9/202. Saved as memory/grind/func_8002D780/candidate.c; the s11 dz-first body is kept as candidate_alt_s11_dzfirst.c.
- verdict: CONFIRMED

## [s12] The dz/dx register seat and the ax/dz delay slot are independently controllable, so the s11 "one LUID, two contradictory requirements" fork is not the real wall.
- mechanism: s11 read the fork off two bodies that differed only in the order of the two subus feeding the first product. The seat is a function of the two quantities' SPANS, not of their birth order as such, and the span is controllable by what sits between the two births — a second, independent degree of freedom that the s11 pair of bodies held constant.
- probe: variant c1_kp_second_product_first.c (kp's second product evaluated into a named temp before its first) built on the ax-first chassis, scored, and dumped; block-7 QTYDBG table compared against a1's.
- result: CONFIRMED. c1 gives dz birth 4 death 20 (span 16, pri 1875) and dx birth 8 death 16 (span 8, pri 3750); the printed allocation order is ord5 qty4 (dx) got=3 = $v1 and ord6 qty1 (dz) got=4 = $a0 — the target's seats on a body that also holds the target's delay slot. c1 itself scores 32/202 because reordering kp's two products scrambles six emitted instructions, so it is banked as rejected/kp-products-reordered-seats-right-order-wrong-32.c, but it is the measurement that proves the fork is escapable. d1 then realises the same seats without disturbing the emission order.
- verdict: CONFIRMED

## [s12] A multiply-defined reg-reg copy carrier can supply the free RTL insn between dz's def and dx's def, the way `m = dist` supplies one in the sqrt block.
- mechanism: s11's frontier 2 asked for an insn that reaches local_alloc and then disappears; a copy whose two definitions straddle a branch was supposed to survive cse (cse.c cannot form a single-definition equivalence for a multiply-defined pseudo) and then be deleted by jump2's delete_noop_moves once local-alloc's combine_regs gives both pseudos the same hard register.
- probe: four carriers built on the ax-first chassis and scored — b1 (`s32 c = x2;` in the outer block, `c = x2;` at block-7 index 3, dx read as `c - x0`), b2 (same with def 1 moved into the test-1 arm), b3 (carrier for z0, consumed by az and bz), b4 (carrier for cz, consumed by az) — plus a full instrumented-cc1 dump of b1.
- result: KILLED as spelled. All four score 9/202 at 202 insns, i.e. identical to the carrier-free a1 body, and b1's block-7 QTYDBG table is byte-identical to a1's (dz birth 4 death 16 refs 3, dx birth 8 death 20 refs 3) — the copy is gone before local-alloc ever sees it. cse propagates the carrier's source into the ordinary-arithmetic use and DCE deletes the now-dead copy. The `m = dist` precedent does not transfer because m's only use is an `__asm__` "r" operand, which cse will not substitute through.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD d9de5549 (-mel -msoft-float), b1/b2/b3/b4 spliced into src/code6cac_b.c on the ax-first (a1) chassis, one FAKE construct present (the `m` re-store in the sqrt block)

## [s12] Hoisting `bz` or `bx` (rather than `az`) supplies the tie-breaking insn between dz's def and dx's def.
- mechanism: identical positional mechanism to the `az` result — any RTL insn between the two definitions lengthens dz's span without lengthening dx's — so any of the four remaining test-3 differences should serve.
- probe: d2_bz_hoisted.c and d3_bx_hoisted.c, each declaring the difference as a named local immediately after `dz`, scored on the sandbox.
- result: KILLED. Both score 9/202 at 202 insns, i.e. no change from the carrier-free ax-first body. sched1 sinks bz and bx back down to their consumers (mult4 and mult3), so neither actually lands between dz's def and dx's def and the span tie is untouched. Only `az` stays high, because its consumer mult2 is the earliest mult after the first product. Banked as rejected/bz-hoist-sinks-back-no-span-shift-9.c and rejected/bx-hoist-sinks-back-no-span-shift-9.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD d9de5549 (-mel -msoft-float), d2/d3 spliced into src/code6cac_b.c, one FAKE construct present (the `m` re-store in the sqrt block)

## [s12] Naming `dx` as the third declaration (so the source order is ax, dz, dx) gives dx the lower sched LUID and therefore the target's dx-before-az emission order.
- mechanism: sched.c rank_for_schedule falls to INSN_LUID (sched.c:2464) for the priority- and class-tied pair dx/az, so putting dx's subu earlier in the source should put it earlier in the emitted stream.
- probe: d5_dx_last_decl.c (`s32 ax; s32 dz; s32 dx;` then `kc = dz * ax - dx * (cz - z0)`), scored on the sandbox.
- result: KILLED, and the failure is instructive: 9/202. Giving dx the lower LUID also gives dx the earlier BIRTH, which shortens nothing and restores the dz/dx span tie, so the seats invert again. The emission order and the seat are coupled through the birth position, which is why the fix has to come from an insn placed between the two births rather than from re-ordering the births themselves. Banked as rejected/dx-named-third-span-tie-restored-9.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD d9de5549 (-mel -msoft-float), d5 spliced into src/code6cac_b.c, one FAKE construct present (the `m` re-store in the sqrt block)

## [s12] Hoisting `ax` out of block 7 into the test-2 block (so fill_simple_delay_slots pulls it into the slot from before the branch) reaches the target's slot without disturbing the seats.
- mechanism: reorg tries fill_simple_delay_slots (insns before the branch) before fill_slots_from_thread, so an ax computed in the test-2 block could be moved into the slot while block 7 still begins with dz, preserving the dz-first spans that already give the target's seats.
- probe: a2_ax_hoist_after_t2.c and a3_ax_hoist_before_t2.c, scored, and a3 pairdiffed.
- result: KILLED. Both score 26/202 at 202 insns. The pairdiff of a3 shows the damage is not in block 7 at all: making ax live across the test-2 branch adds a global allocno and cascades a register renaming through the whole triangle-test region (`lw t2,168(s0)` for ours against `lw t1,168(s0)` for the target, and so on from index 48 onward). Banked as rejected/ax-hoisted-global-allocno-cascade-26.c. This is the same failure mode s10 recorded for hoisting any single difference into the test-2 arm (20-39).
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD d9de5549 (-mel -msoft-float), a2/a3 spliced into src/code6cac_b.c, one FAKE construct present (the `m` re-store in the sqrt block)

## Live frontier (for s13) — 2 insns, no delay slot, no allocator: one scheduler tie

1. **Make sched2 emit `dx` before `az` on the d1 candidate without moving dx's birth earlier.**
   mechanism: sched.c rank_for_schedule compares INSN_PRIORITY, then dependence class, then
   INSN_LUID (tools/gcc-2.7.2/sched.c:2464). dx and az are independent subus that both feed
   mult2, so their longest-path priorities are equal and their classes are equal, and az wins
   on LUID because its declaration puts its subu earlier in sched1's output — which is exactly
   the property that lengthens dz's live range and buys the target's register seats. The naive
   fix is self-cancelling and measured dead (d5). What is needed is an asymmetry in the first
   two comparisons, or a post-reload hard-register dependence that blocks az until after dx.
   next probe: run tools/sched_solver on the d1 stream (both passes are modelled order- and
   clock-exactly). Ask it, over the sched1 statement orders reachable from ordinary C spellings
   of test 3, for the set that simultaneously (a) puts at least one insn between dz's def and
   dx's def, verified against the .lreg blk=7 QTYDBG table as dz span > dx span, and (b) has a
   sched2 output equal to the target's block-7 emission order ax, dz, mult1, dx, az, mult2, bx,
   mult3, bz, mult4, kc, kp, xor, branch. Only score bodies the solver types REACHABLE, and
   require build_insns == 202.

2. **A tie-breaking insn between dz's def and dx's def that sched2 places AFTER dx.**
   mechanism: same positional mechanism as `az`, but with an insn whose INSN_PRIORITY is lower
   than dx's (a longer dependence chain to the end of block 7) or which acquires a WAR/WAW
   dependence on a hard register written by dx's insn after reload. `az` has equal priority and
   loses only on LUID; bz and bx have the right chain length on paper but sched1 sinks them
   before they can serve (s12 kill), so the candidate insn must be needed early and consumed
   late — a shape that does not exist among the six differences.
   next probe: only after frontier 1. If spent, enumerate with sched_solver rather than by hand;
   the s12 measurements show hand-guessing which difference "stays high" is unreliable.

3. **A decomp-permuter campaign on the d1 chassis.**
   mechanism: the residual is now a single adjacent transposition of two independent
   statements with every register in the function already correct — the smallest and most
   permuter-shaped residual this function has ever had, and strictly easier than the s4 campaign
   (which ran on a 7-floor chassis dominated by residuals B and C) or the s11 framing (which was
   a reorg delay-slot decision the permuter cannot address).
   next probe: regenerate base.c from the NEW memory/grind/func_8002D780/candidate.c into the
   validated workspace tmp/grind/func_8002D780/s4/nonmatchings/func_8002D780 (four hurdles +
   rebuild recipe in evidence.md [s4]), launch with tools/permuter_campaign.py, wait IN-TURN
   with `permuter_campaign.py wait --dir <ws>`, harvest with --stop before the turn ends, and
   re-score every find on the engine sandbox (s4's finding that the permuter's weighted metric
   is anti-correlated with the engine's here still applies).

## [s12] Emitting `ax` first, as a named local declared at the top of the test-3 block, hands reorg.c's fill_slots_from_thread the target's delay-slot instruction without flipping the first product's mult operand order.
- mechanism: reorg.c fill_slots_from_thread takes the first insn of the fall-through thread that clears its five gates, and the head of block 7 is whichever difference the source evaluates first. Declaring `s32 ax = cx - x0;` ahead of the first cross product emits ax's subu at block-7 index 0 while `kc = dz * ax - ...` keeps mult1 as `mult $a0,$v0`, unlike the s10 y1 variant which achieved ax-first only by flipping the product's operands.
- probe: Variant a1_ax_local_in_t3.c scored on the sandbox, dumped with the instrumented cc1 (BB2_CC1=tools/gcc-2.7.2/cc1, BB2_QTY_DEBUG/BB2_SUGG_DEBUG/BB2_DBR_DEBUG) into tmp/grind/func_8002D780/s12/a1/, then pairdiffed.
- result: 9/202 at 202 insns. pairdiff shows ours[92] == target[92] == `subu v0,t2,t1` (ax in the delay slot) for the first time in twelve sessions; the only remaining cluster is the dz/dx seat swap (ours dz $v1 / dx $a0, target dz $a0 / dx $v1). Banked as rejected/axfirst-slot-right-seats-swapped-9.c.
- verdict: CONFIRMED

## [s12] On an ax-first body, one extra RTL insn placed strictly between dz's definition and dx's definition breaks the local-alloc qty_compare_1 tie in dx's favour and restores the target's register seats at zero instruction cost.
- mechanism: local-alloc.c:1725-1758 ranks block-7 quantities by floor_log2(refs)*refs*size/(death-birth)*10000 with a quantity-number tie-break at local-alloc.c:1684; whichever of dz/dx is seated first takes $v1 and the other takes $a0. An insn between the two births shifts dz's death by +2 (span 12 -> 14) while shifting dx's birth and death together (span unchanged at 12), so dx's priority 2500 beats dz's 2142. The insn is free when it is a subu the target emits anyway, merely scheduled earlier by sched1 than it is emitted by sched2.
- probe: Hoisted each remaining test-3 difference to a named local declared immediately after `dz` and scored all five bodies (d1 az, d2 bz, d3 bx, d4 az+bz, d5 dx); read the block-7 QTYDBG tables of a1 and c1 to check the span arithmetic against the model.
- result: CONFIRMED for `az`. d1_az_hoisted = 2/202 at 202 insns with every register in the function correct, the delay-slot fill correct, and the residual reduced to the adjacent az/dx transposition at indices 96/97 (tmp/grind/func_8002D780/s12/pairdiff_d1.txt). d4 (az+bz) = 2/202 with the same residual. Saved as memory/grind/func_8002D780/candidate.c; the s11 dz-first body kept as candidate_alt_s11_dzfirst.c.
- verdict: CONFIRMED

## [s12] The dz/dx register seat and the ax/dz delay-slot fill are independently controllable, because the seat depends on the two quantities' live-range SPANS and the span is set by what sits between the two births, not by the birth order alone.
- mechanism: s11 read its fork off two bodies that differed only in the order of the two subus feeding the first product, holding the intervening insn count constant. Varying that count is a second degree of freedom on qty_compare_1's divisor.
- probe: Variant c1_kp_second_product_first.c (kp's second product evaluated into a named temp before its first) built on the ax-first chassis, scored, and dumped; its block-7 QTYDBG table compared against a1's.
- result: CONFIRMED. c1 gives dz birth 4 death 20 (span 16, pri 1875) and dx birth 8 death 16 (span 8, pri 3750); the printed allocation order is ord5 qty4 (dx) got=3 = $v1 and ord6 qty1 (dz) got=4 = $a0 - the target's seats on a body that also holds the target's delay slot. c1 itself scores 32/202 because reordering kp's two products scrambles six emitted instructions, so it is banked as rejected/kp-products-reordered-seats-right-order-wrong-32.c, but it is the measurement that proves the s11 fork is escapable.
- verdict: CONFIRMED

## [s12] A multiply-defined reg-reg copy carrier supplies the free RTL insn between dz's definition and dx's definition, the way `m = dist` supplies one in the sqrt block.
- mechanism: s11's frontier 2 asked for an insn that reaches local_alloc and then disappears. A copy whose two definitions straddle a branch was expected to survive cse (no single-definition equivalence for a multiply-defined pseudo) and then be deleted by jump2's delete_noop_moves once local-alloc's combine_regs gives both pseudos the same hard register.
- probe: Four carriers built on the ax-first chassis and scored: b1 (`s32 c = x2;` in the outer block, `c = x2;` at block-7 index 3, dx read as `c - x0`), b2 (same with def 1 in the test-1 arm), b3 (carrier for z0, consumed by az and bz), b4 (carrier for cz, consumed by az); plus a full instrumented-cc1 dump of b1.
- result: KILLED as spelled. All four score 9/202 at 202 insns, identical to the carrier-free a1 body, and b1's block-7 QTYDBG table is byte-identical to a1's (dz birth 4 death 16 refs 3, dx birth 8 death 20 refs 3) - the copy is gone before local-alloc sees it. cse propagates the carrier's source into the ordinary-arithmetic use and DCE deletes the dead copy. The `m = dist` precedent does not transfer because m's only use is an `__asm__` "r" operand, which cse will not substitute through.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD d9de5549 (-mel -msoft-float), b1/b2/b3/b4 spliced into src/code6cac_b.c on the ax-first (a1) chassis, one FAKE construct present (the `m` re-store in the sqrt block)

## [s12] Hoisting `bz` or `bx` rather than `az` supplies the tie-breaking insn between dz's definition and dx's definition.
- mechanism: Identical positional mechanism to the `az` result - any RTL insn between the two definitions lengthens dz's span without lengthening dx's - so any of the remaining test-3 differences should serve.
- probe: d2_bz_hoisted.c and d3_bx_hoisted.c, each declaring the difference as a named local immediately after `dz`, scored on the sandbox.
- result: KILLED. Both score 9/202 at 202 insns, no change from the carrier-free ax-first body. sched1 sinks bz and bx back down to their consumers (mult4 and mult3), so neither lands between dz's def and dx's def and the span tie is untouched. Only `az` stays high, because its consumer mult2 is the earliest mult after the first product. Banked as rejected/bz-hoist-sinks-back-no-span-shift-9.c and rejected/bx-hoist-sinks-back-no-span-shift-9.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD d9de5549 (-mel -msoft-float), d2/d3 spliced into src/code6cac_b.c, one FAKE construct present (the `m` re-store in the sqrt block)

## [s12] Naming `dx` as the third declaration, so the source order is ax, dz, dx, gives dx the lower sched LUID and therefore the target's dx-before-az emission order.
- mechanism: sched.c rank_for_schedule falls to INSN_LUID (sched.c:2464) for the priority- and class-tied pair dx/az, so putting dx's subu earlier in the source should put it earlier in the emitted stream.
- probe: d5_dx_last_decl.c (`s32 ax; s32 dz; s32 dx;` then `kc = dz * ax - dx * (cz - z0)`), scored on the sandbox.
- result: KILLED, and instructively so: 9/202. Giving dx the lower LUID also gives dx the earlier birth, which restores the dz/dx span tie and inverts the seats again. The emission order and the seat are coupled through the birth position, which is why the fix has to come from an insn placed BETWEEN the two births rather than from re-ordering the births. Banked as rejected/dx-named-third-span-tie-restored-9.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD d9de5549 (-mel -msoft-float), d5 spliced into src/code6cac_b.c, one FAKE construct present (the `m` re-store in the sqrt block)

## [s12] Hoisting `ax` out of block 7 into the test-2 block, so that fill_simple_delay_slots pulls it into the slot from before the branch, reaches the target's slot while leaving the dz-first spans that already give the target's seats.
- mechanism: reorg tries fill_simple_delay_slots (insns before the branch) before fill_slots_from_thread, so an ax computed in the test-2 block could be moved into the slot while block 7 still begins with dz.
- probe: a2_ax_hoist_after_t2.c and a3_ax_hoist_before_t2.c scored on the sandbox; a3 pairdiffed.
- result: KILLED. Both score 26/202 at 202 insns. The pairdiff of a3 shows the damage is not in block 7 at all: making ax live across the test-2 branch adds a global allocno and cascades a register renaming through the whole triangle-test region from index 48 onward (`lw t2,168(s0)` ours against `lw t1,168(s0)` target). Same failure mode s10 recorded for hoisting any single difference into the test-2 arm (20-39). Banked as rejected/ax-hoisted-global-allocno-cascade-26.c.
- verdict: KILLED
- kill_scope: instance
- measured_on: HEAD d9de5549 (-mel -msoft-float), a2/a3 spliced into src/code6cac_b.c, one FAKE construct present (the `m` re-store in the sqrt block)
