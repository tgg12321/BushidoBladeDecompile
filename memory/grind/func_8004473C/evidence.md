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
