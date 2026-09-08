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
