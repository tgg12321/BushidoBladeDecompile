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
