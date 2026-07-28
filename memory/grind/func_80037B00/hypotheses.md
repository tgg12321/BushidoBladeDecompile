# Hypothesis ledger — func_80037B00

## [s1] The +8 phantom frame is not independently addressable; it materializes only when register pressure keeps a local alive across the loop, so it's downstream of the register-rotation axis.
- mechanism: Target has addiu sp,-8/addiu sp,+8 with ZERO memory accesses at sp+0..7 and zero jals (outgoing-args=0), so the 8 bytes are a vars slot. GCC 2.7.2 provably never DCEs local-array stores (file_LoadSectors DCE tests), so any pure-C written-local emits stores that target lacks; unused-local is dead-vars-local-array (forbidden except the 2026-07-01 carve-out which requires target dead stores — target has none). Only surviving mechanism: pin-induced reload pressure keeps a coalesced-away scalar alive, producing the frame as a side-effect of the register axis.
- probe: Confirmed by ledger evidence (WIP note #2) + this session's asm read; no fresh measurement needed.
- result: Axis is contingent, not independent. Any register-axis fix that keeps a natural live-but-memory-dead local produces the frame implicitly.
- verdict: CONFIRMED

## [s1] do-while loop restructure lowers the score.
- mechanism: Move outer/inner test to bottom to match target's bnez-with-delay-slot-advance shape.
- probe: Ledger: prior session measured do-while pin-free → sandbox=17 weighted, build_insns=33.
- result: Score WORSE than goto form (17 > 15 weighted, 33 < 34 build_insns — lost one inner-loop insn). Do-while collapses one slt/bnez pair GCC produces in the goto form.
- verdict: KILLED

## [s2] Narrow-int retyping (var_t1:s32->s16, var_t2:s32->u8) shifts RA scavenger enough to lock at least one of the rotated regs.
- mechanism: register-alloc-pure-c Lever B: GCC 2.7.2 global.c breaks conflict-graph ties by n_refs and mode; narrower modes could flip the tiebreaker.
- probe: Retyped var_t1 to s16 and var_t2 to u8 in src/code6cac_c.c and ran sandbox --disable all.
- result: Score ROSE from 15 to 21; build_insns rose 34->38 due to GCC inserting sign/zero-extension pack instructions at load/compare sites (net loss, target has no extend ops).
- verdict: KILLED

## [s2] Shared-end-label restructure (return ret; joined) with s32 ret local rebinds v0 assignment order to match target's late-zero pattern.
- mechanism: shared-end-label rule: single trailing return via ret prevents per-case constant-fold of return values that drops target stores; also could bias RA on the returned scalar.
- probe: Introduced s32 ret; both exit paths set ret and goto end; end: return ret; measured sandbox.
- result: Score ROSE from 15 to 19 (+4 weighted). build_insns unchanged at 34. The added live scalar competes with existing rotation locals and made the scavenger tiebreak WORSE for the 5-way rotation, not better.
- verdict: KILLED

## [s2] Splitting var_v0 into distinct outer var_count + inner var_v0 narrows live-range and unblocks the rotation.
- mechanism: register-alloc-pure-c Lever A: block-local var split — two disjoint live ranges instead of one variable-reuse coalesce.
- probe: Renamed the outer D_800A38C8 reader to var_count while keeping inner byte-loads on var_v0; measured sandbox.
- result: Score UNCHANGED at 15, build_insns=34. GCC coalesced var_count and var_v0 back into the same pseudo (single-def-then-dead form) — the split had no effect on RA.
- verdict: KILLED

## [s2] Declaration order in target-register-hint sequence (t1,a1,a2,t0,v1 first) biases GCC's initial pseudo numbering / allocation preference toward the target rotation.
- mechanism: register-alloc-pure-c: decl order can influence pseudo-register numbering and thus tiebreak in allocation.
- probe: Reordered locals to var_t1,var_a1,var_a2,var_t0,var_v1 first; measured sandbox.
- result: Score UNCHANGED at 15, build_insns=34. Decl order not a live lever for this function's rotation — GCC's global.c re-sorts pseudos by n_refs+mode before allocation.
- verdict: KILLED

## [s2] Reading both bytes (var_v1=*var_a2; var_v0=*var_a1;) before either compare produces parallel loads that shift the scheduling and free the register rotation.
- mechanism: Load-order restructuring: moving var_v0 load ahead of the zero-terminator branch removes a branch-hop between the two loads.
- probe: Placed both u8 loads at top of loop_inner before both if-tests; measured sandbox.
- result: Score ROSE from 15 to 17; build_insns FELL from 34 to 32 (target has 36). GCC folded/scheduled the paired loads and DROPPED two instructions target keeps. Moves further from target shape.
- verdict: KILLED
