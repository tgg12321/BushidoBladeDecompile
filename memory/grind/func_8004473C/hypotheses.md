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

## s2 (2026-09-06, structural)

### H4 — The residual on BOTH surviving chassis is a single `birthing_insn_p` priority boost in sched1 — CONFIRMED (mechanism, from dumps + a measured half-confirmation)
Mechanism: `adjust_priority` (tools/gcc-2.7.2/sched.c:2543-2597) raises a ready insn to
LAUNCH_PRIORITY 0x7f000001 when `birthing_insn_p` (sched.c:2505-2536) holds — dest is a REG,
its `bb_live_regs` bit is set, and `reg_n_sets[dest] == 1`. Equal-priority ties fall to
class-vs-last-scheduled and then to higher INSN_LUID (`rank_for_schedule`, sched.c:2418-2462).
Form C loses at T-2 (`src`'s copy insn 20 is a once-set live leaf and takes the branch delay
slot ahead of the store); form A loses at T-4 (the pre-check count load 124 is boosted and beats
the call-result copy 11 on LUID, so the copy is emitted BEFORE the count load, hard `$v0` dies
early, local-alloc's free-reg scan hands the single-block count temp `$v0`, and `src` — multi-block,
allocated later by global.c — is rotated into `$a0`, materialising `move a0,v0`).
Probe: `pwsh tools/grinder/dump.ps1 func_8004473C` on form A this session
(tmp/grind/func_8004473C/dumps/text1a_c.sched) plus s1's dumpsC; both block-0 traces read in
full; sched.c/jump.c sources read.
Result: form A's emitted block 0 is the target's order and delay slot exactly, with one extra
`move a0,v0` and a 3-cycle seat rotation; form C's emitted block 0 has the target's seats exactly
and only the order wrong. Predicted: killing either boost yields distance 0 on that chassis.
Verdict: CONFIRMED

### H5 — The local-alloc half of H4 is real: when the call-result copy is scheduled after the count load, `src` gets `$v0` and the count gets `$v1` — CONFIRMED by measurement
Mechanism: hard `$v0`'s live range runs from the call insn to the copy insn; local-alloc
(runs before global-alloc) only avoids `$v0` for the single-block count temp if that range
overlaps the count temp's range, i.e. only if the copy is scheduled after the count load.
Probe: form A3 (`src = call(); dst = (Rec *)D.unkC; D.unk10 = (s32)src;`) — moving the `dst` load
ahead of the store removes the store's separate address insn, so nothing competes with the copy
at T-3 and the copy lands after the count load. sandbox --disable all.
Result: 13 at **build_insns 49 = the target's instruction count**; the copy is deleted, and the
seats are the target's (`lh $v1,6`, `lw $a1`, `sw $v0`, `addiu $a0,$v0,52`).
Verdict: CONFIRMED

### H6 — Statement order chosen to fix the schedule moves the hoisted `&D_800A9CF8` anchor off `&D+0x10` — KILLED (instance)
Mechanism: cse/loop hoist `&D_800A9CF8` (needed for the in-loop `lhu 0($a3)` read of unk0) and
relate it to the first register-form address of the object materialised in the function. With the
store statement first the anchor is `&D+0x10` (target: `addiu $a0,$a0,0x10`, `addiu $a3,$a0,-0x10`,
`sw $v0,0($a0)`); with the `dst` load first it becomes `&D+0xC` and the store degenerates to a
symbol-direct `lui $at; sw $v0,0x10($at)`.
Probe: A3 vs A0, sandbox + objdump of the sandbox object.
Result: A3 13 (right seats, wrong anchor + wrong delay slot); A0 15 (right anchor + right order,
wrong seats). Statement order cannot satisfy both at once in the forms measured.
Verdict: KILLED
kill_scope: instance
measured_on: merged-struct chassis, forms A0/A3, no FAKE constructs, sandbox --disable all = 15 / 13

### H7 — Loop-shape and declaration-order rewrites can move `src` off its single-set state on the form-C chassis — KILLED (instance)
Mechanism: `reg_n_sets[src] >= 2` would defeat `birthing_insn_p`; the only candidate second set is
the biv increment, which flow.c deletes as a dead self-referential set. loop.c only leaves a biv
register live when some access has add_val 0 relative to the biv itself, and `src`'s accesses are
at +0x2C/+0x30/+0x34, so the giv absorbs all of them.
Probe: S1 (src[i], dst++) 18/49; S2 (both indexed) 34; S3 (`s32 *sp` anchored at +0x34 with
sp[-2..0], `sp += 0x1A`) 23/52; S4 (`src` typed `s32`) 13; S5 (declaration order dst/src/i) 13;
S6 (while + explicit byte add at body end) 13; B2 (increments in the body) 13; B4 (declaration
order i/dst/src) 15; B5 (`src++, dst++, i++`) 14; B6 (while + increments at end) 14.
Result: no shape produced a second surviving set; every form landed on 13/14/15 except the
indexed forms which changed the loop body and regressed. S3 shows the cost of forcing a
zero-offset biv use: two address adds (52 insns) instead of the target's one.
Verdict: KILLED
kill_scope: instance
measured_on: merged-struct chassis, forms S1-S6/B2/B4/B5/B6, no FAKE constructs, sandbox --disable all = 13..34

### H8 — Writing the count guard explicitly (so `jump.c` `duplicate_loop_exit_test` never fires) gives the pre-check count temp a second set on the form-A chassis — KILLED (instance)
Mechanism: `duplicate_loop_exit_test` (tools/gcc-2.7.2/jump.c:2163) allocates a FRESH pseudo via
`gen_reg_rtx` for any exit-test reg whose first uid is the exit-test insn and whose last uid is
inside the exit code; a source-level guard instead creates its own once-set pseudo, so the count
temp is a once-set live leaf either way and keeps the boost.
Probe: A1 (`if (unk6 > 0) { do ... while (i < unk6); }`) and B1 (`if (unk6 <= 0) return;` + do-while),
both on the form-A chassis; sandbox.
Result: 19 and 17 — both worse than plain form A (15); the explicit guard also perturbs the
preheader and the delay slot.
Verdict: KILLED
kill_scope: instance
measured_on: merged-struct chassis, forms A1/B1, no FAKE constructs, sandbox --disable all = 19 / 17

### Frontier for s3
1. Form C is the cheaper target: it needs ONLY `reg_n_sets[src] >= 2` at flow time (or `src` not
   live at block-0 exit). Everything else about form C is already byte-correct. Feed s1's
   dumpsC pre-sched chain to `tools/sched_solver` and ask it to enumerate the minimal insn-attribute
   change that flips T-2 to the store, then ask `tools/ra_solver inverse_compose.py classify`
   whether the resulting seat assignment survives. Do this BEFORE spelling more C — 20+ spellings
   across two sessions have now landed on the same 13.
2. Form A needs the mirror change: the pre-check count temp must not be boosted at T-4. Read
   `duplicate_loop_exit_test` (jump.c:2163) again with the loop body in hand and check whether any
   ordinary loop shape makes the exit-test reg's `regno_last_uid` fall OUTSIDE the exit code (that
   is the documented no-remap path, and it would give the pre-check load the SAME pseudo as the
   in-loop load, i.e. two sets). Verify in `.flow` that `reg_n_sets` really is 2 before spending a
   sandbox run.
3. A3 is 13 at the TARGET's instruction count (49) with the target's seats; its only defect is
   the `&D+0xC` anchor and the symbol-direct store. If any ordinary-C form keeps A3's schedule
   while materialising `&D+0x10` first (i.e. some other statement in the function that needs a
   register-form address of the object at offset 0x10 before the `dst` load), that closes it.
   Check the sibling functions in text1a_c.c for such a statement pattern.

## [s2] The entire remaining residual on both surviving chassis is a single birthing_insn_p LAUNCH_PRIORITY boost in sched1: form C loses T-2 to the src copy insn, form A loses T-4 to the pre-check count load.
- mechanism: adjust_priority (tools/gcc-2.7.2/sched.c:2543-2597) raises a ready insn to LAUNCH_PRIORITY 0x7f000001 when birthing_insn_p (sched.c:2505-2536) holds - dest is a REG whose bb_live_regs bit is set and whose reg_n_sets is 1. Equal-priority ties fall through class-vs-last-scheduled to higher INSN_LUID (rank_for_schedule, sched.c:2418-2462). In form C the once-set live copy 'src = <call temp>' (insn 20) is boosted and takes the blez delay slot ahead of the store; in form A the boosted pre-check count load (insn 124) beats the call-result copy (insn 11) on LUID at T-4, which emits the copy BEFORE the count load, so hard $v0 dies early, local-alloc's free-reg scan gives the single-block count temp $v0, and src (multi-block, allocated later by global.c) is rotated into $a0 and materialises 'move a0,v0'.
- probe: pwsh tools/grinder/dump.ps1 func_8004473C on the form-A body this session (tmp/grind/func_8004473C/s2/dumpsA0/text1a_c.sched, full block-0 trace read) plus s1's dumpsC block-0 trace; sched.c adjust_priority/birthing_insn_p/rank_for_schedule and jump.c duplicate_loop_exit_test read in source; objdump of both sandbox objects compared against asm/funcs/func_8004473C.s.
- result: Form A emits the target's block-0 ORDER and delay slot exactly (addu a2,zero / lw a1,12 / lh v0,6 / addiu v1,16 / blez / sw a0,0(v1)) with one extra 'move a0,v0' and a 3-cycle seat rotation; form C emits the target's SEATS exactly (dst $a1, addr $a0, src $v0, count $v1, i $a2, a3=a0-0x10, addiu a0,v0,0x34, addiu v1,a1,0x58) and only the order wrong. Killing either boost is predicted to give distance 0 on that chassis.
- verdict: CONFIRMED

## [s2] When the call-result copy is scheduled AFTER the pre-check count load, hard $v0's live range covers the count temp, local-alloc gives the count $v1, global.c gives src $v0, and the copy is deleted as a no-op.
- mechanism: local-alloc runs before global-alloc and only avoids $v0 for the single-block count qty if hard $v0's range (call insn to copy insn) overlaps the count temp's range (count load to branch); that overlap exists only when the copy is emitted after the count load.
- probe: Form A3 (src = (Rec *)game_GetCharData(); dst = (Rec *)D_800A9CF8.unkC; D_800A9CF8.unk10 = (s32)src;) - moving the dst load ahead of the store removes the store's separate address insn so nothing competes with the copy at T-3. sandbox --disable all plus objdump of tmp/sandbox/func_8004473C/text1a_c.o.
- result: 13 at build_insns 49, which is the TARGET's instruction count; the copy insn is gone and the seats are the target's (lh $v1,6 / lw $a1 / sw $v0 / addiu $a0,$v0,52). The allocation half of the mechanism above is therefore measured, not inferred.
- verdict: CONFIRMED

## [s2] Reordering the store statement and the dst load to fix the schedule keeps the hoisted &D_800A9CF8 anchor on &D+0x10 as the target needs.
- mechanism: cse/loop hoist &D_800A9CF8 for the in-loop lhu 0($a3) read of unk0 and relate it to the first register-form address of the object materialised in the function; the target anchors on &D+0x10 (addiu $a0,$a0,0x10 / addiu $a3,$a0,-0x10 / sw $v0,0($a0)).
- probe: A0 (store statement first) and A3 (dst load first), sandbox --disable all plus objdump of the sandbox object.
- result: A0 15 - right anchor and right order, wrong seats. A3 13 at 49 insns - right seats, but the anchor moves to &D+0xC (addiu $a0,$a0,12 / lw $a1,0($a0) / addiu $a3,$a0,-12) and the store degenerates to a symbol-direct lui $at; sw $v0,0x10($at) with the delay slot taken by move a2,zero. Statement order alone cannot satisfy both halves in the forms measured.
- verdict: KILLED
- kill_scope: instance
- measured_on: merged-struct chassis, forms A0/A3, no FAKE constructs, sandbox --disable all = 15 / 13

## [s2] Loop-shape, indexing and declaration-order rewrites give src a second surviving set on the form-C chassis and so defeat the birthing test.
- mechanism: reg_n_sets[src] >= 2 would defeat birthing_insn_p; the only candidate second set is the biv increment, which flow.c deletes as a dead self-referential set, and loop.c only leaves a biv register live when some access has add_val 0 relative to the biv itself while src's accesses sit at +0x2C/+0x30/+0x34.
- probe: Ten spellings measured with sandbox --disable all: S1 src[i] with dst++; S2 both indexed; S3 s32 *sp anchored at +0x34 with sp[-2..0] and sp += 0x1A; S4 src typed s32 with cast at use; S5 declaration order dst/src/i; S6 while with an explicit byte add at body end; B2 increments in the body; B4 declaration order i/dst/src; B5 for-increment src++, dst++, i++; B6 while with increments at body end.
- result: S1 18 (49 insns, loop body changed); S2 34; S3 23 (52 insns - forcing a zero-offset biv use costs two address adds instead of the target's one); S4 13; S5 13; S6 13; B2 13; B4 15; B5 14; B6 14. No shape produced a second surviving set.
- verdict: KILLED
- kill_scope: instance
- measured_on: merged-struct chassis, forms S1-S6/B2/B4/B5/B6, no FAKE constructs, sandbox --disable all = 13..34

## [s2] Writing the count guard explicitly so jump.c duplicate_loop_exit_test never fires gives the pre-check count temp a second set on the form-A chassis.
- mechanism: duplicate_loop_exit_test (tools/gcc-2.7.2/jump.c:2163) allocates a fresh pseudo via gen_reg_rtx for any exit-test reg whose first uid is the exit-test insn and whose last uid is inside the exit code; a source-level guard instead creates its own once-set pseudo, so the count temp stays a once-set live leaf and keeps the boost either way.
- probe: A1 (if (unk6 > 0) { do ... while (i < unk6); }) and B1 (if (unk6 <= 0) return; then do-while), both on the form-A chassis; sandbox --disable all.
- result: 19 and 17, both worse than plain form A (15); the explicit guard also perturbs the preheader ordering and the delay slot.
- verdict: KILLED
- kill_scope: instance
- measured_on: merged-struct chassis, forms A1/B1, no FAKE constructs, sandbox --disable all = 19 / 17
