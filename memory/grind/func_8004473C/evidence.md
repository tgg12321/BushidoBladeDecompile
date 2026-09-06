# Evidence bank — func_8004473C

## s1 (2026-09-06, recon) — baseline, object model, full pass attribution

- **Baseline.** `canonical` verdict C (49 target insns, distance 40). The committed body in
  src/text1a_c.c was an m2c dump carrying register pins, an `__asm__ volatile` barrier and an
  `_sp_pad[2]` frame coercion; sandbox --disable all stripped 20 cheat lines and scored 40.
- **OBJECT MODEL:** the three flagged globals are ONE object. Evidence in the original binary
  (independent of and predating this session):
  - func_8004473C: `lui/addiu $a0, %lo(D_800A9D08)` then `$a3 = $a0 - 0x10`, and inside the
    loop `lhu $v0, 0($a3)` (= 0x800A9CF8) and `lh $v0, 6($a3)` (= 0x800A9CFE) — three storage
    locations addressed off ONE base register. GCC 2.7.2 can only emit that for offsets of a
    single symbol (cse.c related-value on `(const (plus sym N))`); separate extern scalars never
    share a base.
  - func_80044800 (INCLUDE_ASM sibling): `addiu $v1, %lo(D_800A9D04)` then `$s5 = $v1 - 0xA`
    (= 0x800A9CFA) read via `lh 0($s5)` — a second base+offset instance from a different
    function.
  - 0x800A9D10 is a separate object (address-taken by func_800451D0 / func_80045230 /
    func_80045814 via `addiu %lo(D_800A9D10)`), so the aggregate spans 0x800A9CF8..0x800A9D0F
    (0x18 bytes): s16 @0, s16 @2 (D_800A9CFA), s16 @4 (D_800A9CFC = stage id), s16 @6
    (D_800A9CFE = entry count), s32 @8 (D_800A9D00), s32 @0xC (D_800A9D04 = entry table
    ptr), s32 @0x10 (D_800A9D08 = game_GetCharData() table ptr), s32 @0x14 (unreferenced).
  - Per symbol: D_800A9CFE MISMATCH (measured: scalar model cannot produce `lh 6($a3)`; merged
    struct produces it byte-exact — loop body 0 diff); D_800A9D04 MISMATCH (same measurement,
    loop biv seats from the struct member load match); D_800A9D08 MISMATCH (store
    `sw $v0,0($a0)` with $a0 = &struct+0x10 reproduced only by the struct member store).
    With the merge (header-canonical `Unk800A9CF8Header D_800A9CF8` in include/game.h,
    precedent style `include/game.h:27-33` D_800F1198 merge) the honest floor dropped 40 -> 15
    (form A) -> 13 (form C). The merge is complete in C (all sibling uses in text1a_c.c
    rewritten to members: func_80044670, func_80044B30, func_80044C70); the splat rows
    D_800A9CFA/D_800A9CFC/D_800A9D08 still need deletion from undefined_syms_auto.txt and
    D_800A9CFE/D_800A9D00/D_800A9D04 need the `/* alias of D_800A9CF8+N; retire with
    func_80044800 */` suffix (prong (c) sibling carve-out, no-new-park-categories.md:238-262)
    — NOT done this session because the function is not at 0 yet; the full diff is banked as
    candidate_merge.patch. A candidate-ready will need scope_allow for include/game.h +
    undefined_syms_auto.txt (integration-handoff path; grind.ps1 single-stem gate).
- **Record type.** Both tables are arrays of 0x68-byte records (sibling func_80044B30 walks
  both with stride 0x68 and the same field offsets 0x0/0x2/0x4/0x6/0x8/0xC/0x10/0x12/0x14 and
  copies 0x2C..0x34 -> 0x4C..0x54). Modelled TU-locally as `Rec4473C` (ordinary typing).
- **Loop body is byte-exact** under form A and form C: two bivs (dst `$a1`, i `$a2`) + two
  reduced givs (src+0x34 `$a0`, dst+0x58 `$v1`) with negative offsets — loop.c's giv list is
  prepended, so the LAST field access becomes the giv base; `dst->unk0` is the biv's direct
  use. Store order/scheduling in the loop matches with the natural field order 0,1,2,4,6,8,A,C,
  10,12,14,4C,50,54,58.
- **Residual = pre-loop block only (13).** Target: `addu a2,0; lw a1,D+0xC; lh v1,D+6;
  lui/addiu a0,D+0x10; blez v1; [slot] sw v0,0(a0)`, then `addiu a0,v0,0x34`. Two coupled
  facts: (a) the call result stays in $v0 through the count load (so the count temp seats
  in $v1); (b) the count load precedes the address/store pair.
- **Pass attribution (dumps in tmp/grind/func_8004473C/s1/dumpsA, dumpsC, dumpsN1):**
  - sched1 (sched.c) schedules BACKWARD from the jump. All block-0 insns have priority 1; the
    order is decided by `adjust_priority` (sched.c ~2700): a predecessor whose dest pseudo is
    live and has `reg_n_sets == 1` ("birthing", `birthing_insn_p`) is raised to
    LAUNCH_PRIORITY 0x7f000001 and picked first; among equals, class-vs-last-scheduled then
    higher LUID (`rank_for_schedule` sched.c:2418-2460) wins; a memory op is preferred among
    priority-1 ties (`potential_hazard`, sched.c ~2709); a load queued off the branch is
    blocked one cycle after a store (`actual_hazard`, form A trace "blocking insn 124 for 1
    cycles").
  - local-alloc (local-alloc.c ~1512): qtys with a hard-reg copy suggestion
    (`qty_phys_copy_sugg`) are allocated FIRST, before the priority-sorted pass, so the
    call-result temp gets $v0 whenever it is a single-block qty; the count temp then takes
    the next free reg in REG_ALLOC_ORDER ($v1). A multi-block pseudo (used in the loop
    preheader) is global.c's, which runs after local-alloc — then the count temp takes $v0.
  - Form A (`src = call(); D.unk10 = src;`): copy `p72 = v0` (user var, global). Sched1 gives
    the target ORDER (trace: T-2 store by hazard, T-3 addr, T-4 count vs copy tie -> count by
    LUID), but local-alloc seats count in $v0 -> src gets $a0 (`move a0,v0`). Floor 15.
  - Form C (`D.unk10 = (s32)call(); src = (Rec*)D.unk10;`): expand makes a local temp p75
    (`copy_to_reg` because the assignment target is a MEM); cse1 forwards the reload into
    `p72 = p75` (insn 20). Allocation is now right (p75 -> $v0 by suggestion, its last use is
    the store scheduled last; count -> $v1; src -> $v0 by global copy preference). But insn 20
    is a once-set leaf -> birthing -> LAUNCH priority -> scheduled adjacent to the branch at
    T-2; the count load is then launched at T-3 with no store hazard and lands BEFORE the
    addr/store pair (forward: store before `lh`), and `i = 0` fills the delay slot. Floor 13.
  - Exit-test duplication (jump.c duplicate_loop_exit_test, `regno_first_uid == INSN_UID`
    test): the pre-check count load is always a FRESH once-set pseudo, so it is always
    birthing/boosted; making the count a shared user variable (N1) makes it loop-carried and
    global (floor 29).
- **Measured spellings (all ordinary C):** A 15; C 13; C4/C5/C6/C7 (load-order permutations of
  C) 13; D1/D2/D3 (explicit `if (n>0)` + do/while, `while`) 13; F1 (`tmp=call; src=tmp;
  D.unk10=src`) 15 — cse canonicalizes the store operand to the older temp, so src's copy is a
  leaf again but the temp dies early; F2 (`tmp=call; D.unk10=tmp; src=tmp`) 13 (= C); F4
  (double reload) 13 (second reload folded); F5 17 / F6 19 / F7 19 (reload or tmp copy inside
  the count guard: the temp escapes block 0 -> global -> count takes $v0); N1 29.
- **What the target needs (derived, not yet spelled):** form C's allocation shape (a local
  call temp whose last use is the store) AND a reload/copy into `src` that is NOT a birthing
  leaf — i.e. `src`'s pseudo must have `reg_n_sets >= 2` at flow time, or the copy must have
  a block-0 consumer that the store cannot be (cse rewrites the store operand to the older
  reg). `src++`'s biv increment survives loop.c and cse2 but flow.c deletes it as a
  self-referential dead set BEFORE counting (dumpsC .loop/.cse2 have insn 112, .flow does
  not), so the loop increment does not supply the second set.

- [s1] OBJECT MODEL: D_800A9CFE MISMATCH (measured: scalar model cannot emit lh 6($a3); struct member emits it exact, floor 40 -> 13 across the merge); D_800A9D04 MISMATCH (measured, same merge: biv seat from the member load matches); D_800A9D08 MISMATCH (measured: sw $v0,0($a0) with $a0 = &D_800A9CF8+0x10 reproduced only by the member store). Independent evidence: func_8004473C $a3 = $a0-0x10 base+offset reads; func_80044800 $s5 = $v1-0xA reading D_800A9CFA; 0x800A9D10 is a separate address-taken object, so the aggregate is 0x800A9CF8..0x800A9D0F.

- [s1] Aggregate merge applied header-canonically (include/game.h Unk800A9CF8Header D_800A9CF8, precedent include/game.h:27-33) with every C sibling use rewritten; splat-row retirement (prong c) deferred until a 0 is in hand; full diff banked as memory/grind/func_8004473C/candidate_merge.patch. A candidate-ready will need scope_allow for include/game.h + undefined_syms_auto.txt (integration-handoff).

- [s1] Loop body byte-exact in forms A and C: bivs dst/$a1 and i/$a2, givs src+0x34/$a0 and dst+0x58/$v1 (loop.c prepends givs so the last field becomes the base; negative offsets are natural).

- [s1] sched1 is a backward list scheduler; block-0 order is decided by adjust_priority's birthing boost (sched.c), higher-LUID tie-break, potential_hazard memory-op preference and the one-cycle load-after-store block. local-alloc allocates copy-suggested single-block qtys first (local-alloc.c ~1512) — that is why form C seats correctly and form A does not.

- [s1] src++'s biv increment survives loop.c and cse2 (insn 112 in dumpsC .loop/.cse2) and is deleted by flow.c as a self-referential dead set before reg_n_sets is counted, so it cannot supply the second set needed to demote the reload copy.

- [s1] Derived requirement for the match: form C's shape (single-block call temp whose last use is the store) plus a src copy that is not a birthing leaf (reg_n_sets >= 2 at flow time, or a block-0 consumer the store cannot be because cse rewrites the store operand to the older temp).

## s2 (2026-09-06, structural) — the residual is ONE scheduler tie, fully localised

Chassis re-measured at session start: the s1 merge patch (`candidate_merge.patch`) applies
cleanly to HEAD and the form-C body measures **13** (build_insns 50 vs target 49 — the single
extra instruction is the load-delay `nop` forced by `lh $v1,6(...)` landing immediately before
the `blez $v1`). Every s1 conclusion is therefore still chassis-valid.

### The two chassis and their single blocking decision each (READ THIS FIRST)

Both surviving families are **one `birthing_insn_p` boost away from a byte match**. The boost is
`sched.c:2584` (`adjust_priority` raises a ready insn to LAUNCH_PRIORITY 0x7f000001 when
`birthing_insn_p` holds: dest is a REG, `bb_live_regs` has its bit, and `reg_n_sets[dest] == 1`,
`sched.c:2505-2536`). Ties at equal priority fall through to class-vs-last-scheduled and then to
**higher INSN_LUID wins** (`rank_for_schedule`, `sched.c:2418-2462`).

**Form C (`D.unk10 = (s32)game_GetCharData(); src = (Rec *)D.unk10;`) — floor 13, seats already
perfect.** Verified from `tmp/grind/func_8004473C/s1/dumpsC/text1a_c.sched`: block 0 is
{9 call, 11 p75=$v0, 13 addr=&D+0x10, 15 store, 20 p72=p75, 25 dst load, 28 i=0, 128 count load,
129 slt, 130 jump}. At T-2 the ready list is `15 (1) 20 (7f000001) 25 (1) 129 (1)` and insn 20
(the `src` copy, a once-set live leaf) takes the delay slot. Disassembly confirms the seats are
already exactly the target's (dst $a1, addr $a0, src $v0, count $v1, i $a2, a3 = a0-0x10,
`addiu $a0,$v0,0x34`, `addiu $v1,$a1,0x58`); ONLY the block-0 order differs. **If insn 20 were not
boosted**, the store wins T-2 by `potential_hazard`, insn 20 then beats the addr insn at T-3 on
LUID (it is the highest-LUID block-0 insn), the addr insn takes T-4 and the count load T-5 —
forward `count, addr, copy, store` — and the copy is a `$v0<-$v0` no-op that is deleted. That is
the target byte-for-byte.

**Form A (`src = (Rec *)game_GetCharData(); D.unk10 = (s32)src;`) — floor 15, order already
perfect.** Re-dumped this session (`tmp/grind/func_8004473C/dumps/text1a_c.sched`, form A0 in
src at dump time). Block 0 = {9 call, 11 p72=$v0, 14 addr, 16 store, 21 dst, 124 count, 125 i=0,
126 jump}. Trace: T-2 store (potential_hazard), T-3 addr (beats the copy on LUID), **T-4 ready
`11 (7f000001) 125 (1) 21 (1) 124 (7f000001)` -> 124 wins on LUID**, T-5 copy, T-6 dst, T-7 i=0.
Emitted: `move a2,zero / move a0,v0 / lui a1;lw a1,12 / lui v0;lh v0,6 / lui v1;addiu v1,16 /
blez v0 / sw a0,0(v1)` — the target's ORDER and delay slot exactly, plus one extra `move a0,v0`,
with a 3-cycle seat rotation (src $a0, count $v0, addr $v1 instead of $v0/$v1/$a0).

**Why the rotation happens, and the exact fix.** `local-alloc` runs before `global-alloc` and
allocates single-block qtys; the pre-check count temp is single-block, hard `$v0`'s live range
runs only from the call to the copy insn, and in form A sched1 places the copy BEFORE the count
load, so the count qty does not conflict with hard `$v0` and takes it (`local-alloc.c` free-reg
scan in REG_ALLOC_ORDER). `src` is then multi-block, goes to global.c, finds `$v0` taken, and
lands in `$a0` — hence `move a0,v0` and the rotation. **If insn 124 (the count load) lost the
birthing boost**, the T-4 group would be `{11}` alone (11 wins T-4), 124 would fall to T-5 in the
priority-1 group and still win it on `potential_hazard` (it is the only load), giving forward
`call, i=0, dst, count, copy, addr, store`. The copy then sits AFTER the count load, hard `$v0`
covers the count temp's range, local-alloc gives the count `$v1`, global.c gives `src` `$v0`, and
the copy becomes a deleted no-op. That is again the target byte-for-byte.

### Independent confirmation of the allocation half (form A3, measured)

`A3` = form A with `dst = (Rec *)D_800A9CF8.unkC;` moved BEFORE the store statement. Measured 13
with **build_insns 49 = the target's count** — the copy insn is gone and the seats are exactly
right (`sw v0,...`, `lh v1,6`, `lw a1`, `addiu a0,v0,52`). It works precisely because the store's
LHS needed no separate address insn there, so nothing competed with the copy at T-3, the copy
landed after the count load, and the predicted local-alloc conflict materialised. A3's residual is
purely the ADDRESSING ANCHOR: with `dst` first, cse/loop anchor the hoisted `&D_800A9CF8` on
`&D+0xC` (`addiu $a0,$a0,12` / `lw $a1,0($a0)` / `addiu $a3,$a0,-12`) and the store degenerates to
a symbol-direct `lui $at; sw $v0,16($at)`. The target anchors on `&D+0x10` (`addiu $a0,$a0,16`,
`addiu $a3,$a0,-0x10`, `sw $v0,0($a0)`) — i.e. the anchor follows the FIRST statement in the
function that needs a register-form address of the object, so the store statement must stay ahead
of the `dst` load. **A3 is the proof that the local-alloc half of the mechanism above is real, not
inferred.**

### Why the boost cannot be removed by the spellings tried

`birthing_insn_p` needs `reg_n_sets[dest] == 1`. For form C's `src` the second set would have to
be the biv increment, and s1 already measured that flow.c deletes it as a dead self-referential
set before counting (still true: every loop shape re-tested this session lands on 13/14/15). For
form A's count temp, the pre-check load comes from `jump.c` `duplicate_loop_exit_test`
(`tools/gcc-2.7.2/jump.c:2163`), which allocates a FRESH pseudo via `gen_reg_rtx` whenever the
exit-test reg's first uid is the exit-test insn AND its last uid is inside the exit code — which
is always true for a compiler-generated loop-bound temp. Making it a user variable instead is N1
(29, s1).

### Measurements this session (all ordinary C, merged-struct chassis, no FAKE constructs)

form C control (S0) 13 · S1 `src[i]` indexed, dst++ **18 / 49 insns** · S2 both indexed 34 ·
S3 `s32 *sp` at +0x34 with sp[-2..0] and `sp += 0x1A` **23 / 52 insns** (two address adds:
one in block 0, one in the preheader) · S4 `src` typed `s32` + cast at use 13 · S5 declaration
order dst/src/i 13 · S6 while-loop, `src` advanced by explicit byte add at body end 13 ·
A0 form A 15 · A1 form A + `if (unk6>0){do..while}` 19 · A2 form A + while 15 · **A3 form A with
dst before the store 13 / 49 insns** · A4 i=0 hoisted out of the for 15 · A5 declaration
initialiser 15 · A6 reversed test `unk6 > i` 15 · B1 `if (unk6<=0) return;` + do-while 17 ·
B2 increments in the body 13 · B3 i=0 separate 15 · B4 declaration order i/dst/src 15 ·
B5 for-increment reordered `src++, dst++, i++` 14 · B6 while + increments at body end 14.

- [s2] The whole residual is ONE `sched.c` birthing boost. Form C fails at T-2 (the `src` copy, insn 20, is boosted and steals the branch delay slot); form A fails at T-4 (the pre-check count load, insn 124, is boosted and beats the call-result copy on LUID, which puts the copy before the count load and lets local-alloc hand the count temp hard `$v0`). Removing either boost is predicted to yield distance 0 on that chassis.
- [s2] Form A3 (`src = call(); dst = ...; D.unk10 = (s32)src;`) MEASURES the allocation half of that prediction: 13 at build_insns 49, copy deleted, seats exactly the target's. Its only residual is that the hoisted `&D_800A9CF8` anchor follows the first register-form address in the function, so it lands on `&D+0xC` instead of `&D+0x10` and the store degenerates to symbol-direct `sw $v0,16($at)`.
- [s2] The addressing anchor is order-sensitive: the store statement must precede the `dst` load to get the target's `addiu $a0,$a0,0x10` / `addiu $a3,$a0,-0x10` pair.
- [s2] Making `src` the loop biv with a zero-offset use (S3) costs two address adds instead of one (52 insns) because loop.c only leaves a biv register in place when an access has add_val 0 relative to the biv itself.

- [s2] Chassis re-verified: candidate_merge.patch applies cleanly to HEAD and the form-C body measures 13 (build_insns 50 vs target 49). Every s1 conclusion is still chassis-valid. The tree was reverted to HEAD before finishing; the merge lives only in candidate_merge.patch.

- [s2] Form C's register seats are ALREADY exactly the target's - dst $a1, store address $a0 (= &D_800A9CF8+0x10), src $v0, count $v1, i $a2, a3 = a0-0x10, addiu $a0,$v0,0x34, addiu $v1,$a1,0x58. Only the block-0 instruction order differs, and the one extra instruction is the load-delay nop forced by lh $v1,6 landing immediately before blez $v1.

- [s2] Form A's block-0 ORDER is already exactly the target's including the store in the blez delay slot; its defect is one extra 'move a0,v0' plus a 3-cycle register rotation (src $a0, count $v0, addr $v1 instead of $v0/$v1/$a0).

- [s2] sched.c block-0 trace for form A (tmp/grind/func_8004473C/s2/dumpsA0/text1a_c.sched): 'ready list at T-4: 11 (7f000001) 125 (1) 21 (1) 124 (7f000001), now 124 11 125 21' - the pre-check count load 124 and the call-result copy 11 are tied at LAUNCH_PRIORITY and the tie is broken by INSN_LUID in favour of 124, which is what emits the copy before the count load and rotates the seats.

- [s2] sched.c block-0 trace for form C (s1 dumpsC): 'ready list at T-2: 15 (1) 20 (7f000001) 25 (1) 129 (1), now 20 129 25 15' - the src copy insn 20 is the only boosted insn and takes the branch delay slot ahead of the store 15.

- [s2] Form A3 (src = call(); dst = (Rec *)D.unkC; D.unk10 = (s32)src) measures 13 at build_insns 49 - the target's instruction count - with the copy insn deleted and the target's seats. Its whole residual is that the hoisted &D_800A9CF8 anchor follows the first register-form address materialised in the function, so it lands on &D+0xC and the store degenerates to a symbol-direct lui $at; sw $v0,0x10($at).

- [s2] The addressing anchor is order-sensitive: the store statement must precede the dst load to get the target's addiu $a0,$a0,0x10 / addiu $a3,$a0,-0x10 pair.

- [s2] birthing_insn_p (sched.c:2505) requires reg_n_sets[dest] == 1 AND the dest bit set in bb_live_regs; adjust_priority (sched.c:2584) then raises the insn to max_priority, observed as 0x7f000001. Nineteen ordinary-C spellings across s1 and s2 have failed to produce a second set for either the src pseudo or the pre-check count pseudo.

- [s2] duplicate_loop_exit_test (jump.c:2163) remaps an exit-test pseudo to a FRESH register only when regno_first_uid[reg] == INSN_UID(exit-test insn) AND the reg's last uid falls inside the exit code; if either fails the copy reuses the SAME pseudo, which would make reg_n_sets 2 and remove the boost. That no-remap path is the one untried structural route to killing form A's boost.

- [s2] New measurements this session (all ordinary C, merged-struct chassis, no FAKE constructs): S0 13, S1 18/49, S2 34, S3 23/52, S4 13, S5 13, S6 13, A0 15, A1 19, A2 15, A3 13/49, A4 15, A5 15, A6 15, B1 17, B2 13, B3 15, B4 15, B5 14, B6 14.

## s3 (2026-09-06, structural) — the block-0 schedule is now EXACT, and the lever is split in two

Chassis re-verified at session start: `candidate_merge.patch` applies to HEAD; the aggregate
declaration was placed TU-LOCALLY in src/text1a_c.c this session (typedef + `extern
Unk800A9CF8Header D_800A9CF8;` immediately before `func_80044670`) instead of include/game.h,
because include/game.h is outside a grind session's allowed edit surface. **Measured
identical: form C = 13 at build_insns 50.** The header-canonical placement is therefore a
packaging decision for the eventual candidate, not a codegen input; a future session can
measure entirely inside src/text1a_c.c and stay in scope.

### The complete sched1 block-0 trace for form C (first time captured end-to-end)

`tmp/grind/func_8004473C/s3/traceC/text1a_c.sched`, lines 11824-11852. The dump gives the
whole backward schedule, not just one ready list:

    ;; ready list at T-1: 130 (7fffff9b), now 130                                 -> 130 jump
    ;; ready list at T-2: 15 (1) 20 (7f000001) 25 (1) 129 (1), now 20 129 25 15   -> 20
    ;; launching 128 before 20 with no stalls at T-3
    ;; ready list at T-3: 129 (1) 25 (1) 15 (1) 128 (7f000001), now 128 129 25 15 -> 128
    ;; ready list at T-4: 129 (1) 25 (1) 15 (1), now 129 25 15
    ;; insn 15 has a greater potential hazard, now 15 129 25                      -> 15
    ;; ready list at T-5: 129 (1) 25 (1) 11 (7f000001) 13 (7f000001), now 13 11 . -> 13
    ;; ready list at T-6: 11 (7f000001) 129 (1) 25 (1), now 11 129 25             -> 11
    ;; ready list at T-7: 129 (1) 25 (1), now 129 25
    ;; insn 25 has a greater potential hazard, now 25 129                         -> 25
    ;; ready list at T-8: 129 (1), now 129                                        -> 129
    ;; ready list at T-9: 9 (7f000001), now 9                                     -> 9

Reading rule (established this session, verified against the emitted bytes): the scheduler
takes ready[0] AFTER the potential-hazard re-sort, and T-1 is the LAST insn in forward order.
So form C's forward block-0 order is

    9 call, 129 i=0, 25 dst-load, 11 (reg75 = $v0), 13 addr(&D+0x10), 15 store,
    128 count-load, 20 (reg72 = reg75), 130 blez

and after both copies are deleted as no-ops that is exactly what the sandbox object contains:
`lui a1; lw a1,12 / lui a0; addiu a0,a0,16 / sw v0,0(a0) / lui v1; lh v1,6 / nop / blez v1 /
move a2,zero`.

**The target's forward order is `129 i=0, 25 dst, 128 count, 13 addr, 15 store` with the two
copies anywhere before their uses.** Diffed against form C, the ONLY thing that has to change
is that insn 20 must not win T-2. Re-running the trace by hand with insn 20 unboosted gives:
T-2 store 15 (potential_hazard promotes it inside the priority-1 group), T-3 addr 13 (boosted,
`&D+0x10` is once-set and live-out), T-4 count 128 (boosted), T-5/T-6 the two copies, T-7 dst,
T-8 i=0 — i.e. forward `i=0, dst, count, addr, store`, the store becomes the last real insn
before the branch so reorg fills the delay slot with it, and the count load is no longer
adjacent to `blez` so the load-delay nop disappears. That is 49 instructions and the target's
bytes.

### The exact RTL (from tmp/grind/func_8004473C/s3/traceC/text1a_c.flow)

    (insn 11  (set (reg:SI 75) (reg:SI 2 v0))                        REG_DEAD $v0
    (insn 13  (set (reg:SI 76) (const (plus (symbol_ref "D_800A9CF8") 16)))
    (insn 15  (set (mem/s:SI (reg:SI 76)) (reg:SI 75))
    (insn 20  (set (reg/v:SI 72) (reg:SI 75))                        REG_DEAD reg 75
    (insn 25  (set (reg/v:SI 73) (mem/s:SI (const (plus (symbol_ref "D_800A9CF8") 12))))
    (insn 28  (set (reg/v:SI 74) (const_int 0))
    (insn 145 (set (reg:SI 97) (plus (reg/v:SI 72) (const_int 52)))  REG_DEAD reg/v 72  [preheader]
    (insn 151 (set (reg:SI 98) (plus (reg/v:SI 73) (const_int 88)))                     [preheader]

reg72 = `src`, reg73 = `dst`, reg74 = `i`, reg75 = expand's call-result temp, reg76 = the
`&D_800A9CF8+0x10` anchor. reg75 is DEAD at insn 20 and reg72 is DEAD at insn 145, so the
copy cannot be propagated away by cse2 in either direction, and insn 20 and insn 145 sit in
different basic blocks so combine cannot merge them either.

### Mechanism confirmations read out of sched.c this session

- `adjust_priority` is called exactly ONCE per insn, at launch time (`sched.c:2645`, inside
  the launch loop) — the boost is not re-evaluated, so `bb_live_regs` at launch is what
  decides it.
- `schedule_block`'s selection loop (`sched.c:2674-2727`) walks the ready list in groups of
  EQUAL `INSN_PRIORITY`. A boosted insn is a singleton group and is taken immediately; the
  `potential_hazard` "prefer the memory op" rule and `actual_hazard` queuing only ever apply
  WITHIN a group. So a LAUNCH_PRIORITY insn cannot be beaten by anything except another
  LAUNCH_PRIORITY insn with a higher LUID, or by being queued by `actual_hazard`
  (function-unit busy), which no ordinary reg-reg move in this block can be.
- `duplicate_loop_exit_test` (`jump.c:2228-2256`) re-read and confirmed: the remap to a fresh
  `gen_reg_rtx` fires iff `regno_first_uid[reg] == INSN_UID(exit-test insn)` AND some insn
  strictly inside the exit code is `regno_last_uid[reg]`. For this loop's bound temp both
  always hold.

### W1 — the first form that reproduces the target's block-0 order, anchor AND delay slot

`W1` replaces the `Rec4473C *src` walker with `s32 *sp` anchored at +0x34 and read as
`sp[-2] / sp[-1] / sp[0]`, advanced `sp += 0x1A`. The add_val-0 access (`sp[0]`) is what makes
loop.c leave the biv register live, so `reg_n_sets[sp] == 2`, `birthing_insn_p` fails, and the
schedule comes out as predicted:

    move v1,v0             <- the call-result copy, NOT deleted
    addiu a3,v1,52         <- sp init, in block 0 (target has this add in the preheader)
    move a2,zero           <- i = 0             }
    lui a1; lw a1,12       <- dst               }  the target's
    lui v0; lh v0,6        <- count (no nop!)   }  block-0 order,
    lui a0; addiu a0,a0,16 <- anchor +0x10      }  anchor and
    blez v0                                     }  delay slot,
     sw v1,0(a0)           <- store in the slot }  verbatim
    addiu t0,a0,-16

This is the measured proof that the birthing boost is the whole ordering story. Its own
residual (23 / 52 insns) is structural to the biv-survival trick and is NOT a near miss:
because the surviving biv does not absorb the two remaining reads, the loop carries TWO
walking pointers with TWO increments (`$a3` the biv read at 0, `$a0 = $a3+48` the giv read at
-4/0), and the call-result temp stays live into the preheader, so it is multi-block, global.c
seats it in `$v1` and the block-0-local pre-check count temp takes `$v0` — the exact mirror of
the target. W9 (dst loaded before sp) and W13 (`src = (Rec*)D.unk10; sp = &src->unk34;`, where
combine merges the copy into the add) both emit byte-identical 23/52 output.

### The allocation half, now stated precisely (from the .lreg register lists)

Form C works on seats because reg75 is a **block-0-local** qty carrying a `$v0` hard-reg copy
suggestion from insn 11, so local-alloc seats it first ($v0), the block-0-local count temp is
pushed to `$v1`, and global.c then gives the multi-block reg72 `$v0` by copy preference, which
is why insn 20 becomes a deleted `$v0 <- $v0`. In W1 (and in A0) the value that survives into
the preheader IS the call-result temp itself, so it is multi-block, global.c runs after
local-alloc has already handed `$v0` to the count temp, and the seats rotate. W1's `.lreg`
shows this directly: `Register 95 used 2 times across 4 insns in block 0` (the count, local)
vs `Register 75 used 4 times across 12 insns` with no block tag, and reg 75 listed in
`Basic block 1: Registers live at start: 29 30 72 73 74 75 76`.

**So the two halves are now separately measured and mutually exclusive in every form tried:**
(a) the seats need the stored value to be a block-0-local temp whose only consumers are the
store and one copy — that forces the walker to be a fresh once-set pseudo, i.e. birthing;
(b) the order needs the walker's defining insn to be non-birthing — which so far always means
giving the walker a second set, which makes it the live biv and costs a second walking
register plus its increment.

### Measurements this session (all ordinary C, merged-struct chassis, no FAKE constructs)

S0 form C control **13 / 50** - A3 **13 / 49** (re-measured; anchor `&D+0xC`, store degenerates
to `lui $at; sw $v0,16($at)`, delay slot `move a2,zero`) - W1 **23 / 52** - W2 (sp init in the
for-init) **23 / 52** - W3 (A-chassis sp, `sp += 0xD` after the store) **32 / 52** -
W9 (dst before sp) **23 / 52** - W13 (`src` reload then `sp = &src->unk34`) **23 / 52** -
Z1 (block-scoped `src = (Rec*)D.unk10 + i` inside the loop) **30 / 51** -
Z6 (src declared in an inner block after the store) **13 / 50** -
Z7 (src and dst both inner-block scoped) **13 / 50**.

- [s3] The TU-local aggregate declaration (typedef + extern in src/text1a_c.c) measures identically to the include/game.h placement (form C = 13 / 50), so the merge can be measured entirely inside a grind session's allowed edit surface; header-canonical placement is a packaging step for the final candidate only.
- [s3] Form C's complete sched1 block-0 schedule is captured in tmp/grind/func_8004473C/s3/traceC/text1a_c.sched (T-1..T-9). Forward order: call, i=0, dst-load, reg75=$v0, addr &D+0x10, store, count-load, reg72=reg75, blez. The target's is call, i=0, dst-load, count-load, addr, store (plus the two copies, both deleted). The single divergence is insn 20 winning T-2 on its birthing LAUNCH_PRIORITY.
- [s3] sched.c's selection loop (2674-2727) processes the ready list in groups of EQUAL INSN_PRIORITY; potential_hazard's memory-op preference and actual_hazard's queuing only reorder WITHIN a group. A LAUNCH_PRIORITY insn is therefore a singleton group and is unbeatable except by another boosted insn with a higher LUID. adjust_priority is called once, at launch (sched.c:2645), so bb_live_regs at launch time is decisive.
- [s3] In form C, reg75 is REG_DEAD at insn 20 and reg72 is REG_DEAD at insn 145 (the preheader giv init reg97 = reg72 + 52), and insns 20 and 145 are in different basic blocks. cse2 cannot propagate the copy away in either direction and combine cannot merge it, so the copy insn is structurally present in block 0 for every form-C spelling measured.
- [s3] W1 (s32 *sp anchored at +0x34, read sp[-2]/sp[-1]/sp[0], sp += 0x1A) is the FIRST measured form that reproduces the target's block-0 order, the &D+0x10 anchor and the store-in-the-blez-delay-slot all at once, because the add_val-0 access keeps the biv live (reg_n_sets == 2) and kills the birthing boost. Its residual is 23 / 52: the surviving biv does not absorb the other two reads, so the loop carries two walking pointers with two increments, and the call-result temp becomes multi-block so the seats rotate ($v1 for src, $v0 for the count).
- [s3] Declaration scope does not move pseudo numbering in a way that helps: Z6/Z7 (src, and src+dst, declared in an inner block after the store) emit output byte-identical to plain form C at 13 / 50.
- [s3] The seat half and the order half are mutually exclusive in every form measured across s1-s3: the seats require the stored value to be a block-0-local temp consumed only by the store and one copy (which forces the walker to be a fresh once-set = birthing pseudo), while the order requires the walker's defining insn to be non-birthing (which so far always means a second set, i.e. a live biv costing an extra walking register and increment).

- [s3] The aggregate merge can be declared TU-LOCALLY in src/text1a_c.c (typedef plus extern Unk800A9CF8Header D_800A9CF8 immediately before func_80044670) with byte-identical results to the include/game.h placement - form C measures 13 at build_insns 50 either way. That keeps every future measurement inside a grind session's allowed edit surface; header-canonical placement is a packaging step for the final candidate only. (The previous session was discarded for a scope violation on undefined_syms_auto.txt; this route removes the need to touch anything outside src/text1a_c.c.)

- [s3] Form C's complete sched1 block-0 schedule is now on record (tmp/grind/func_8004473C/s3/traceC/text1a_c.sched, T-1..T-9). Forward order: call, i=0, dst-load, reg75=$v0, addr &D+0x10, store, count-load, reg72=reg75, blez. The reading rule - the scheduler takes ready[0] after the potential-hazard re-sort, and T-1 is the LAST insn in forward order - was verified by reconciling the schedule against the emitted bytes.

- [s3] sched.c's selection loop (2674-2727) processes the ready list in groups of EQUAL INSN_PRIORITY; potential_hazard's memory-op preference and actual_hazard's queuing only reorder within a group. A LAUNCH_PRIORITY insn is a singleton group and is unbeatable except by another boosted insn with a higher LUID. adjust_priority is called once, at launch (sched.c:2645), so bb_live_regs at launch time is decisive.

- [s3] In form C the copy chain is structurally locked: (insn 20 (set (reg/v:SI 72) (reg:SI 75))) carries REG_DEAD reg 75, and the preheader's (insn 145 (set (reg:SI 97) (plus (reg/v:SI 72) (const_int 52)))) carries REG_DEAD reg/v 72. Each register dies at the other's boundary so cse2 cannot propagate the copy away in either direction, and the two insns are in different basic blocks so combine cannot merge them.

- [s3] W1 (s32 *sp anchored at +0x34, read sp[-2]/sp[-1]/sp[0], advanced sp += 0x1A) is the first measured form that reproduces the target's block-0 ORDER, the &D_800A9CF8+0x10 anchor and the store-in-the-blez-delay-slot simultaneously, because the add_val-0 access keeps the biv live (reg_n_sets == 2) and defeats the birthing boost.

- [s3] The seat half and the order half are mutually exclusive in every form measured across s1-s3. The target's SEATS need the stored value to be a block-0-local temp consumed only by the store and one copy (form C: reg75 local with a $v0 copy suggestion, allocated first by local-alloc, which pushes the count temp to $v1; global.c then gives the multi-block src $v0 and the copy is deleted). The target's ORDER needs the walker's defining insn to be non-birthing, which so far always means a second set - i.e. a live biv costing an extra walking register and its increment.

- [s3] Twelve form-C statement orderings across three sessions (S0, C4-C7, D1-D3, F2, F4, S4, S5, S6, B2, Z6, Z7) emit byte-identical 13 / 50 output. The emitted instruction SET is already the target's; re-spelling statement order on that chassis is exhausted.

## s4 (2026-09-06) — MATCHED, honest floor 13 -> 0

- **The matching body** (memory/grind/func_8004473C/candidate.c, applied in src/text1a_c.c):
  `D_800A9CF8.unk10 = (s32)(src = (Rec4473C *)game_GetCharData());` then
  `dst = (Rec4473C *)D_800A9CF8.unkC;` then
  `for (i = 0; i < D_800A9CF8.unk6; dst++, src++, i++)` with the 15 stores in the order
  unk0, unk1, unk2, unk4, unk8, unkC, unkA, unk10, unk12, unk14, unk4C, unk50, unk54, unk6, unk58.
- **Measurements:** `sandbox func_8004473C --disable all` = score 0, build_insns 49,
  target_insns 49. `verify-oracle` = ok true, build_sha1 == 62efab4f73f992798c43e8c730aa43baa10bb4fa
  == original_sha1_locked (whole tree, all siblings byte-neutral under the aggregate merge).
- **Corrected inheritance:** s1-s3 recorded the loop body as "byte-exact" and therefore stopped
  permuting it. That was wrong and it cost three sessions: `dst->unk6 = 0;` at position 13 plus
  `dst->unkC = 0;` before `dst->unkA = 4;` is worth 4 points on its own (13 -> 9), and the
  comma order `dst++, src++, i++` a further 2 (9 -> 7).
- **Tooling that made it cheap** (reusable for any function in this TU): a standalone scorer
  `tmp/grind/func_8004473C/s4b/sc.py` calling `engine.score.score_func` against
  `tmp/perm_4473C_s4a/target.o` through `tmp/perm_4473C_s4a/compile.sh`. ~0.4 s per candidate
  versus a full sandbox run, and validated to agree with `sandbox --disable all` at 13/50 and
  again at 9/50 and 0/49. The greedy searchers built on it are greedy_fc.py, greedy2.py,
  greedy3.py, search2.py..search6.py in the same directory.
- **HEAD context:** HEAD carried a register-pinned m2c body for func_8004473C
  (`register ... asm("v0")`, `s32 _sp_pad[2]`, `__asm__ volatile("" : "=m"(_sp_pad[0]))`).
  The s4 diff deletes all of it; the function is now pure C.
- **Packaging still open (operator step, outside a grind session's edit surface):** the
  `Unk800A9CF8Header` / `Rec4473C` typedefs and `extern Unk800A9CF8Header D_800A9CF8;` sit
  TU-locally in src/text1a_c.c. `candidate_merge.patch` moves them to include/game.h
  (prong (d) of the aggregate-merge family); s3 measured that variant byte-identical. The
  per-word rows in undefined_syms_auto.txt stay while `func_80044800` is still INCLUDE_ASM,
  per the 2026-09-03 amendment, and want the `/* alias of D_800A9CF8+N; retire with
  func_80044800 */` suffix.

## s5 (2026-09-06, synthesis) — bytes RE-PROVEN in the HEADER-CANONICAL configuration

The s4 session reached distance 0 but was layer-1 FAILed on ONE ground: the aggregate
declaration sat TU-locally in src/text1a_c.c, which is an unmet mandatory prong of the
aggregate-merge family (header-canonical, complete, never TU-local — `no-new-park-categories.md`
2026-08-17 entry). That exact TU-local spelling is now a driver-enforced BANNED construct for
this function. This session closed the remaining question: does the header-canonical placement
still produce the bytes?

**Measured this session, from a clean HEAD, with `candidate_merge.patch` applied (which puts
`Unk800A9CF8Header` + `extern Unk800A9CF8Header D_800A9CF8;` in include/game.h, deletes the six
per-word `extern` scalars from src/text1a_c.c and rewrites the three C sibling users) and the s4
matched body in place:**

- `sandbox func_8004473C --disable all` -> `{"score": 0, "target_insns": 49, "build_insns": 49,
  "scorable": true, "rules_dropped": 0}`. Honest, cheat-invisible, zero rules. (The run reports
  `cheat_asm_stripped: 6`; those six lines belong to OTHER functions still carrying m2c cheat
  bodies in the same TU, not to func_8004473C — the s4/s5 body has no `__asm__`, no register
  pins, no pads, no FAKE construct.)
- Full-tree `verify-oracle --rebuild --allow-dirty` -> `"ok": true`,
  `"build_sha1": "62efab4f73f992798c43e8c730aa43baa10bb4fa"` ==
  `"original_sha1_now"` == `"original_sha1_locked"`. Every sibling TU and every other user of the
  seven per-word symbols is byte-neutral under the merge.

So the header-canonical variant is byte-identical to the TU-local variant AT ZERO, not merely at
the old floor of 13 (s3 had only shown the equivalence at 13). **The function is SOLVED.** The
sole remaining obstacle is that `include/game.h` is outside a grind session's allowed edit
surface, so no session can stage the fix; the tree was reverted to clean after measurement.

Artifacts: `tmp/grind/func_8004473C/s4/text1a_c.c.matched` (the complete matched TU),
`tmp/grind/func_8004473C/s4/HANDOFF-include-game-h.diff` (the header hunk alone),
`memory/grind/func_8004473C/candidate_merge.patch` (REGENERATED this session — it now carries
BOTH the header-canonical merge AND the s4 matched body, so the next session applies exactly one
patch and is at 0).

- [s5] Header-canonical placement of the D_800A9CF8 aggregate (include/game.h) measures score 0 / 49 of 49 instructions AND full-tree oracle SHA1 62efab4f73f992798c43e8c730aa43baa10bb4fa. The layer-1 FAIL ground (TU-local declaration) is fixed by a one-hunk move that costs nothing in bytes.
- [s5] The ONLY blocker left is scope: `include/game.h` is not in this function's `tools/grinder/scope_allow.txt` line. The remedy is the pipeline-executable integration handoff (.claude/rules/integration-handoff-self-serve.md, owner ruling 2026-08-19) — a Judge ESCALATE with escalate_kind=integration-handoff and scope_paths=["include/game.h"]. No new construct, no new family, no owner question.
- [s5] KILL RE-AUDIT resolved by supersession: every instance kill in state.json was measured on a floor-13 chassis whose residual s4 dissolved (the loop body was never byte-exact — `dst->unk6 = 0` late plus `dst->unkC` before `dst->unkA` is worth 4, and the comma order `dst++, src++, i++` a further 2). Re-measuring those kills has no value: the chassis they constrain no longer exists, because the function measures 0.

- [s6 / driver session 4, synthesis] INDEPENDENT RE-VERIFICATION of the s5 bytes claim, from a
  clean HEAD, by a session that did not author it: `git apply memory/grind/func_8004473C/candidate_merge.patch`
  (clean apply, 2 files, include/game.h + src/text1a_c.c) then
  `sandbox func_8004473C --disable all` -> `"score": 0, "target_insns": 49, "build_insns": 49,
  "scorable": true, "rules_dropped": 0` (artifact tmp/grind/func_8004473C/s4/sandbox_merged_s4.json),
  then `verify-oracle --rebuild --allow-dirty` -> `"ok": true`,
  `build_sha1 == original_sha1_now == original_sha1_locked == 62efab4f73f992798c43e8c730aa43baa10bb4fa`
  (artifact tmp/grind/func_8004473C/s4/verify_oracle_s4.txt). The claim holds. The tree was
  reverted (`git checkout -- include/game.h src/text1a_c.c`) before this session ended; only
  ledger files are dirty.
- [s6] ROUTING DEFECT that discarded s5, diagnosed and avoided: s5 returned `owner-gated` with an
  `INTEGRATION HANDOFF`-titled entry. `tools/grinder/grind.ps1:1288` gates EVERY `owner-gated`
  outcome on a decisions.md LINE that matches `OWNER-ESCALATION|CANONICAL-ASM GRANT PATH` *and*
  contains the function name — so the `INTEGRATION HANDOFF` branch at grind.ps1:1409 is
  unreachable for a function that has no legacy OWNER-ESCALATION history. func_8004473C has none
  (it is 4 sessions old), so `owner-gated` can NEVER validate here regardless of merit. The
  precedents that did route this way (func_80022F34, func_80034F88, func_80033550, func_80062020)
  all carry pre-2026-08-19 OWNER-ESCALATION entries that satisfy the line regex incidentally.
- [s6] The routing that DOES work for a first-time handoff: `result: "ruling-request"`.
  grind.ps1:1371 -> Invoke-JudgeRuling -> on an `ESCALATE` verdict, Invoke-JudgeEscalation
  (grind.ps1:469) executes `escalate_kind=integration-handoff` exactly as the owner-gated branch
  would have: it appends the `scope_paths` line to tools/grinder/scope_allow.txt, applies
  `unban_construct`, constrains the function to land the banked form, and keeps it ACTIVE. No
  decisions.md line-regex is consulted anywhere on that path.
- [s6] The standing ban MUST be cleared as part of the grant, or the compliant next session is
  auto-discarded too. `check_banned_constructs` (grindlib.py:316) trips when >=50% of the ban
  phrase's content words appear in the vet's CONSTRUCTS: block. The banked ban is "typedef struct
  { ... } Unk800A9CF8Header; extern Unk800A9CF8Header D_800A9CF8; declared at file scope in
  src/text1a_c.c (not in include/game.h)"; a self-vet for the header-canonical form necessarily
  declares `typedef`, `struct`, `Unk800A9CF8Header`, `extern`, `D_800A9CF8`, `include/game.h` —
  well past the threshold. `unban_construct=Unk800A9CF8Header` is therefore part of the remedy,
  not a nicety.
