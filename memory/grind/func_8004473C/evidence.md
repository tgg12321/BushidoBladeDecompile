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
