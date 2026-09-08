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
