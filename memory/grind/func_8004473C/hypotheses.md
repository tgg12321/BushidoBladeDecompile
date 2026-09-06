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

## s3 (2026-09-06, structural)

### H9 — Form C's block-0 defect is exactly ONE ready-list decision (insn 20 winning T-2) — CONFIRMED, now trace-complete
Mechanism: `schedule_block` (tools/gcc-2.7.2/sched.c:2674-2727) selects from the ready list in
groups of equal `INSN_PRIORITY`; `potential_hazard`'s memory-op preference and `actual_hazard`'s
queuing only reorder insns WITHIN a group, so an insn raised to LAUNCH_PRIORITY by
`adjust_priority` (sched.c:2584, via `birthing_insn_p` sched.c:2505) is a singleton group and is
taken unconditionally. `adjust_priority` runs once per insn at launch (sched.c:2645).
Probe: full `-da` capture of the form-C TU into tmp/grind/func_8004473C/s3/traceC; block-0
schedule read T-1..T-9 and reconciled instruction-for-instruction with the sandbox object.
Result: form C's forward block-0 order is `call, i=0, dst, reg75=$v0, addr(&D+0x10), store,
count, reg72=reg75, blez`; the target's is `i=0, dst, count, addr, store`. Every instruction
form C emits is already one of the target's (same anchor `addiu $a0,$a0,0x10`, same
`addiu $a3,$a0,-0x10`, same seats); the only extra byte is the load-delay `nop` forced by the
count load landing immediately before `blez`. Hand-replaying the trace with insn 20 unboosted
gives the target's order, puts the store last before the branch (so reorg fills the delay slot
with it) and removes the nop — 49 instructions, the target's bytes.
Verdict: CONFIRMED

### H10 — A walker pointer with a surviving second set removes the boost and yields the target's block-0 order — CONFIRMED by measurement (W1)
Mechanism: `birthing_insn_p` needs `reg_n_sets[dest] == 1`. loop.c leaves a biv's own register
live (instead of folding it entirely into givs) when some access has add_val 0 relative to the
biv, so a walker anchored at the LAST accessed field and read at `[-2]/[-1]/[0]` keeps both its
init and its increment, giving `reg_n_sets == 2`.
Probe: W1 — `s32 *sp = (s32 *)(D_800A9CF8.unk10 + 0x34); ... sp += 0x1A;` with the three source
reads spelled `sp[-2] / sp[-1] / sp[0]`. sandbox --disable all + objdump.
Result: 23 / 52. Block 0 emits `move a2,zero / lui a1; lw a1,12 / lui v0; lh v0,6 / lui a0;
addiu a0,a0,16 / blez v0 / sw v1,0(a0) / addiu t0,a0,-16` — the target's order, the target's
`&D+0x10` anchor, the store in the delay slot and NO load-delay nop, for the first time in this
grind. The boost is therefore confirmed as the entire ordering story.
Verdict: CONFIRMED

### H11 — The biv-survival walker (W-family) can be made byte-neutral — KILLED (instance)
Mechanism: if loop.c keeps the biv register live it must also keep a giv for any access whose
add_val is not 0; with reads at -8/-4/0 relative to the biv only the last is add_val 0, so the
loop carries the biv AND a giv, each with its own `addiu ...,0x68`. The walker's init also
becomes a source-level statement in block 0 rather than a loop.c preheader giv init, and the
call-result temp is then live into the preheader, so it is multi-block and global.c allocates
it after local-alloc has already given `$v0` to the block-0-local pre-check count temp.
Probe: W1 (sp anchored at +0x34), W2 (sp init inside the for-init), W3 (A-chassis: `sp =
(s32 *)game_GetCharData(); D.unk10 = (s32)sp; sp += 0xD;`), W9 (dst loaded before sp),
W13 (`src = (Rec4473C *)D_800A9CF8.unk10; sp = &src->unk34;` — combine merges the copy into
the add). sandbox --disable all on each, plus objdump of W1 and its `.lreg` register list.
Result: W1 / W2 / W9 / W13 all 23 at 52 instructions with byte-identical output (+3 insns: the
undeleted `move v1,v0` and the second walking pointer's increment pair); W3 32 / 52. `.lreg`
confirms the seat mechanism: `Register 95 used 2 times across 4 insns in block 0` (count, local)
vs `Register 75 used 4 times across 12 insns` listed live at the start of basic block 1.
Verdict: KILLED
kill_scope: instance
measured_on: merged-struct chassis (TU-local aggregate decl), forms W1/W2/W3/W9/W13, no FAKE constructs, sandbox --disable all = 23/23/32/23/23

### H12 — Declaration scope can renumber `src`'s pseudo so cse2 canonicalises the preheader giv onto the call-result temp and deletes the copy — KILLED (instance)
Mechanism: pseudo numbers are assigned in declaration-expansion order, and cse2 canonicalises an
equivalence class onto its representative register; if `src`'s pseudo were created after expand's
call-result temp, the preheader's `reg97 = reg_src + 52` might be rewritten onto the temp,
leaving `src` dead and the copy insn deleted by flow.
Probe: Z6 (`src` declared in an inner block placed after the store statement), Z7 (`src` and
`dst` both inner-block scoped); sandbox --disable all, plus the `.flow` RTL for the copy chain.
Result: both 13 / 50 with output byte-identical to plain form C. The `.flow` chain is unchanged:
`(insn 20 (set (reg/v:SI 72) (reg:SI 75)))` with `REG_DEAD reg 75`, feeding
`(insn 145 (set (reg:SI 97) (plus (reg/v:SI 72) (const_int 52))))` with `REG_DEAD reg/v 72` in
the preheader — the two registers die at each other's boundary, so neither direction of copy
propagation is available, and the two insns are in different basic blocks so combine cannot
merge them either.
Verdict: KILLED
kill_scope: instance
measured_on: merged-struct chassis (TU-local aggregate decl), forms Z6/Z7, no FAKE constructs, sandbox --disable all = 13 / 13

### H13 — Computing the source pointer inside the loop body (block-scoped, indexed) removes the block-0 copy without disturbing the loop — KILLED (instance)
Mechanism: a loop-body-local `Rec4473C *src = (Rec4473C *)D_800A9CF8.unk10 + i;` would be
LICM'd/strength-reduced by loop.c, so no user pseudo would need a block-0 copy at all.
Probe: Z1; sandbox --disable all.
Result: 30 / 51 — loop.c builds a different address chain and the loop body diverges. Worse than
the S1 indexed form (18 / 49) already banked in s2.
Verdict: KILLED
kill_scope: instance
measured_on: merged-struct chassis (TU-local aggregate decl), form Z1, no FAKE constructs, sandbox --disable all = 30

### Frontier for s4
1. The two halves are now separately MEASURED and, in every form tried, mutually exclusive:
   the target's SEATS need the stored value to be a block-0-local temp whose only consumers are
   the store and one copy (form C: reg75 local with a `$v0` copy suggestion, allocated first by
   local-alloc, pushing the count temp to `$v1`, then global.c gives the multi-block `src` `$v0`
   and the copy is deleted); the target's ORDER needs the walker's defining insn to be
   non-birthing (W1: `reg_n_sets == 2`, which costs a live biv plus a second walking register).
   The next probe should attack the ALLOCATION side of W1 rather than the order side of form C:
   find a spelling in which the walker has two sets AND the call-result temp still dies inside
   block 0. Concretely, check whether any ordinary form makes the loop's source giv init
   (`addiu $a0,$v0,0x34`) come from the call-result temp while a separately-set walker supplies
   the second set — e.g. read the `.loop` dump for which pseudo loop.c chooses as the giv base
   when two candidate pointers exist.
2. Un-probed disjunct of `birthing_insn_p`: the boost also requires the dest's bit in
   `bb_live_regs` at LAUNCH time. `src` is live-out of block 0 only because the preheader giv
   init (insn 145) reads it. If any ordinary form puts that giv init in block 0 without adding
   an instruction (i.e. loop.c emits it before the branch), `src` would be dead at block-0 exit
   and the boost would vanish with no second set required. Read the `.loop` dump to see what
   decides preheader vs pre-branch placement of the giv init before spelling anything.
3. Do NOT re-spell form C's statement order again: S0/C4-C7/D1-D3/F2/F4/S4/S5/S6/B2/Z6/Z7 —
   twelve orderings across three sessions — all emit byte-identical 13 / 50 output. The
   emitted instruction SET is already the target's; only the schedule differs.

## [s3] Form C's block-0 divergence is exactly one sched1 ready-list decision: insn 20 (the src copy) wins T-2 because birthing_insn_p raises it to LAUNCH_PRIORITY, and schedule_block selects in groups of equal INSN_PRIORITY so a boosted insn is an unbeatable singleton group.
- mechanism: adjust_priority (tools/gcc-2.7.2/sched.c:2584) raises a ready insn to LAUNCH_PRIORITY when birthing_insn_p (sched.c:2505) holds - dest is a REG whose bb_live_regs bit is set and whose reg_n_sets is 1 - and it is called exactly once per insn at launch (sched.c:2645). schedule_block's selection loop (sched.c:2674-2727) walks the ready list in groups of EQUAL priority, so potential_hazard's memory-op preference and actual_hazard's queuing only reorder insns WITHIN a group and can never outrank a boosted singleton.
- probe: Full -da capture of the form-C TU with the instrumented cc1 into tmp/grind/func_8004473C/s3/traceC; the whole block-0 backward schedule read T-1..T-9 and reconciled instruction-for-instruction against the sandbox object; sched.c 2400-2470 (rank_for_schedule), 2490-2600 (birthing_insn_p/adjust_priority) and 2670-2727 (selection loop) read in source; the .flow RTL for insns 11/13/15/20/25/28/145/151 extracted.
- result: Form C's forward block-0 order is call, i=0, dst-load, reg75=$v0, addr(&D+0x10), store, count-load, reg72=reg75, blez. The target's is i=0, dst, count, addr, store with both copies deleted. Every instruction form C emits is already one of the target's - same anchor, same addiu $a3,$a0,-0x10, same seats, byte-exact loop body - and the single extra instruction is the load-delay nop forced by the count load landing immediately before blez. Replaying the trace with insn 20 unboosted gives the target's order, makes the store the last real insn before the branch (so reorg fills the delay slot with it) and removes the nop: 49 instructions, the target's bytes.
- verdict: CONFIRMED

## [s3] Giving the source walker a surviving second set (reg_n_sets == 2) removes the birthing boost and makes sched1 emit the target's block-0 order, the &D_800A9CF8+0x10 anchor and the store in the blez delay slot.
- mechanism: birthing_insn_p requires reg_n_sets[dest] == 1. loop.c leaves a biv's own register live instead of folding it entirely into givs when some access has add_val 0 relative to the biv, so a walker anchored at the LAST accessed field and read at [-2]/[-1]/[0] keeps both its init and its increment.
- probe: Form W1: s32 *sp = (s32 *)(D_800A9CF8.unk10 + 0x34); the three source reads spelled sp[-2] / sp[-1] / sp[0]; sp += 0x1A in the for-increment. sandbox --disable all plus objdump of tmp/sandbox/func_8004473C/text1a_c.o and the .lreg register list from tmp/grind/func_8004473C/s3/traceW1.
- result: 23 at 52 instructions, but block 0 emits move a2,zero / lui a1; lw a1,12 / lui v0; lh v0,6 / lui a0; addiu a0,a0,16 / blez v0 / sw v1,0(a0) / addiu t0,a0,-16 - the target's order, the target's anchor, the store in the delay slot and NO load-delay nop, for the first time in this grind. The boost is therefore confirmed as the entire ordering story rather than an inference from one ready list.
- verdict: CONFIRMED

## [s3] The biv-survival walker family (W1/W2/W3/W9/W13) is byte-neutral apart from the scheduling effect it buys.
- mechanism: If loop.c keeps the biv register live it must still keep a giv for every access whose add_val is not 0, so with reads at -8/-4/0 the loop carries the biv AND a giv, each with its own addiu ...,0x68. The walker's init also becomes a source-level block-0 statement rather than a loop.c preheader giv init, which leaves the call-result temp live into the preheader; it is then multi-block, and global.c allocates it only after local-alloc has handed $v0 to the block-0-local pre-check count temp.
- probe: W1 (sp anchored at +0x34), W2 (sp init inside the for-init), W3 (A-chassis: sp = (s32 *)game_GetCharData(); D.unk10 = (s32)sp; sp += 0xD), W9 (dst loaded before sp), W13 (src = (Rec4473C *)D_800A9CF8.unk10; sp = &src->unk34, where combine merges the copy into the add). sandbox --disable all on each; objdump of W1; W1's .lreg register list.
- result: W1 / W2 / W9 / W13 all 23 at 52 instructions with byte-identical output - three extra instructions: the undeleted move v1,v0 plus the second walking pointer and its increment. W3 32 at 52. The .lreg confirms the seat mechanism directly: 'Register 95 used 2 times across 4 insns in block 0' (the count temp, local) versus 'Register 75 used 4 times across 12 insns' with no block tag and reg 75 listed in 'Basic block 1: Registers live at start: 29 30 72 73 74 75 76'.
- verdict: KILLED
- kill_scope: instance
- measured_on: merged-struct chassis with the aggregate declared TU-locally in src/text1a_c.c, forms W1/W2/W3/W9/W13, no FAKE constructs, sandbox --disable all = 23 / 23 / 32 / 23 / 23

## [s3] Declaring src in an inner block after the store statement renumbers its pseudo so that cse2 canonicalises the preheader giv init onto the call-result temp and the copy insn dies.
- mechanism: Pseudo numbers are assigned in declaration-expansion order and cse2 canonicalises an equivalence class onto its representative register, so a src pseudo created after expand's call-result temp might let the preheader's reg97 = reg_src + 52 be rewritten onto the temp, leaving src dead and the copy deleted by flow.
- probe: Z6 (src declared in an inner block placed after the store statement) and Z7 (src and dst both inner-block scoped); sandbox --disable all plus the .flow RTL for the copy chain.
- result: Both 13 at 50 instructions with output byte-identical to plain form C. The .flow chain is unchanged: (insn 20 (set (reg/v:SI 72) (reg:SI 75))) carries REG_DEAD reg 75 and feeds (insn 145 (set (reg:SI 97) (plus (reg/v:SI 72) (const_int 52)))) which carries REG_DEAD reg/v 72 - the two registers die at each other's boundary so neither direction of copy propagation is available, and the two insns sit in different basic blocks so combine cannot merge them.
- verdict: KILLED
- kill_scope: instance
- measured_on: merged-struct chassis with the aggregate declared TU-locally in src/text1a_c.c, forms Z6/Z7, no FAKE constructs, sandbox --disable all = 13 / 13

## [s3] Computing the source pointer inside the loop body as a block-scoped indexed expression removes the block-0 copy insn without disturbing the loop body.
- mechanism: A loop-body-local Rec4473C *src = (Rec4473C *)D_800A9CF8.unk10 + i would be LICM'd and strength-reduced by loop.c, so no user pseudo would need a copy in block 0 at all.
- probe: Form Z1; sandbox --disable all.
- result: 30 at 51 instructions - loop.c builds a different address chain and the loop body diverges. Worse than the s2-banked indexed form S1 (18 / 49).
- verdict: KILLED
- kill_scope: instance
- measured_on: merged-struct chassis with the aggregate declared TU-locally in src/text1a_c.c, form Z1, no FAKE constructs, sandbox --disable all = 30
