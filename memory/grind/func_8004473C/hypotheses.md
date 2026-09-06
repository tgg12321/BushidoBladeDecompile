# Hypothesis ledger — func_8004473C

## s1 (2026-09-06, recon)

### H1 — Object model: D_800A9CF8..D_800A9D0B is one struct (aggregate merge) — CONFIRMED
Mechanism: cse.c related-value addressing (`$a3 = $a0 - 0x10` serving +0/+6) is only emitted
for offsets of a single symbol. Probe: header-canonical `Unk800A9CF8Header D_800A9CF8`
(include/game.h), all sibling uses rewritten to members, this function rewritten as plain
struct/record C. Result: 40 -> 15 (form A) -> 13 (form C); loop body byte-exact.

### H2 — Pre-loop seating: the call result must stay in $v0 across the count load — CONFIRMED (mechanism), instance-level
Mechanism: local-alloc's copy-suggestion pass (local-alloc.c ~1512) seats a SINGLE-BLOCK
call-result temp in $v0 first; a multi-block user var goes to global.c after the count temp
has taken $v0. Probe: form A (user var directly from the call) -> `move a0,v0` (15); form C
(assignment to the struct member first, then `src = (Rec*)D.unk10`) -> correct seats (13).
Kill (instance): form A on this chassis, no FAKE constructs — measured 15.

### H3 — Pre-loop order: count load must precede the address/store pair — OPEN
Mechanism: sched.c backward list scheduling; the forwarded reload copy `src = p75` is a
once-set leaf (`birthing_insn_p`) and is raised to LAUNCH_PRIORITY, so it is scheduled next
to the branch and the count load is launched without the store hazard. Requirement derived:
keep form C's temp/store shape but make `src`'s copy non-birthing (`reg_n_sets >= 2` at
flow time) or give it a block-0 consumer other than the store. Killed instances (this
chassis, no FAKE): C4-C7 load-order permutations 13; D1-D3 loop-shape variants 13; F1 15;
F2 13; F4 13; F5 17; F6/F7 19; N1 29.

### Frontier for s2
1. Find an ordinary-C spelling in which `src`'s pseudo is set twice with no extra output
   insn (both sets live at flow time; second set a no-op after allocation). Candidates to
   measure: `src` initialised on both arms of the count check (e.g. computing a second
   pointer variable from the same temp on the fallthrough), or a loop shape where the biv
   increment of `src` is not deleted by flow but is absorbed into the giv (check .flow/.lreg).
   Read tmp/grind/func_8004473C/s1/dumpsC/text1a_c.sched block 0 trace before spelling.
2. Use tools/sched_solver on block 0 of form C (pre-sched chain in dumpsC .combine) to
   enumerate which insn-attribute changes (priority boost removal / a block-0 consumer)
   yield the target order, then map the winner back to C. Then classify the seat with
   tools/ra_solver `inverse_compose.py classify` to confirm the local-temp shape.
3. If both are dead: measure whether the store address can be made to depend on the count
   load (a form where the count is read through the same base register as the store) — the
   target's pre-check load is symbol-direct, so this is a low-probability probe; kill it
   with one measurement rather than theorising.

## [s1] D_800A9CF8/CFA/CFC/CFE/D00/D04/D08 are one 0x18-byte struct at 0x800A9CF8; declaring it as an aggregate reproduces the base-register loop reads and the member store.
- mechanism: cse.c related-value addressing on (const (plus sym N)) only exists for offsets of a single symbol; separate extern scalars can never share $a3 = $a0 - 0x10.
- probe: Header-canonical Unk800A9CF8Header in include/game.h, all sibling uses in text1a_c.c rewritten to members, function rewritten as plain struct C; sandbox --disable all.
- result: 40 -> 15 (form A) -> 13 (form C); loop body has zero diffs; lhu 0($a3)/lh 6($a3)/sw $v0,0($a0) all reproduced.
- verdict: CONFIRMED

## [s1] Form A (src = game_GetCharData(); D.unk10 = src; for-loop) on the merged-struct chassis with no FAKE constructs seats the call result in $a0 instead of $v0 and measures 15.
- mechanism: src is a multi-block user pseudo (used by the loop preheader giv init) so it is allocated by global.c after local-alloc has given the single-block count temp $v0 (REG_ALLOC_ORDER); local-alloc's copy-suggestion pass (local-alloc.c ~1512) only helps single-block qtys.
- probe: sandbox --disable all on form A; .lreg/.greg dumps (tmp/grind/func_8004473C/s1/dumpsA): 'Register 92 in 2', '72 preferences: 2' with conflict on hard reg 2.
- result: 15; sched1 order already matches the target, only the seats differ (move a0,v0 / lh v0 / addiu v1).
- verdict: KILLED
- kill_scope: instance
- measured_on: merged-struct chassis, form A, no FAKE constructs, sandbox --disable all = 15

## [s1] Form C (D.unk10 = (s32)game_GetCharData(); src = (Rec*)D.unk10; for-loop) and its load-order permutations C4-C7, loop-shape variants D1-D3, and F2/F4 all measure 13 with the count load scheduled after the store and i=0 in the delay slot.
- mechanism: cse1 forwards the reload into a once-set copy insn (src = p75) that sched.c adjust_priority marks birthing (reg_n_sets==1, live) and raises to LAUNCH_PRIORITY; scheduled backward it sits next to the branch, so the count load is launched at T-3 with no store hazard and lands before the addr/store pair.
- probe: sandbox on each; dumpsC/text1a_c.sched block-0 trace ('ready list at T-2: 15 (1) 20 (7f000001) 25 (1) 129 (1)'); sched.c:2418-2460 rank_for_schedule and adjust_priority/birthing_insn_p read.
- result: All 13. Allocation is correct in this family (p75 -> $v0 by copy suggestion, count -> $v1, src -> $v0 by global preference); only the two-insn order plus the delay-slot filler differ.
- verdict: KILLED
- kill_scope: instance
- measured_on: merged-struct chassis, forms C/C4-C7/D1-D3/F2/F4, no FAKE constructs, sandbox --disable all = 13 each

## [s1] Reading the count into a shared local n (pre-check and loop test, n re-read at body end) makes the pre-check load non-birthing without changing the loop.
- mechanism: jump.c duplicate_loop_exit_test keeps the pseudo when regno_first_uid predates the exit test, so reg_n_sets would be 2.
- probe: Form N1, sandbox; N1.dis and dumpsN1.
- result: 29: n becomes a loop-carried global pseudo seated in $a0 (lh a0 / slt v0,a3,a0) and the pre-loop seats shift; the target's count temps are fresh per load.
- verdict: KILLED
- kill_scope: instance
- measured_on: merged-struct chassis, form N1, no FAKE constructs, sandbox --disable all = 29

## [s1] Placing the reload/copy of src inside the count guard (F5/F6/F7) keeps the call temp local and fixes the order.
- mechanism: cse forwards across the fallthrough extended basic block, so the copy lands in block 1 and the temp p75 becomes live across the branch -> global.c -> count temp takes $v0.
- probe: sandbox on F5, F6, F7.
- result: 17 / 19 / 19 — allocation regresses exactly as predicted.
- verdict: KILLED
- kill_scope: instance
- measured_on: merged-struct chassis, forms F5/F6/F7, no FAKE constructs, sandbox --disable all = 17/19/19
