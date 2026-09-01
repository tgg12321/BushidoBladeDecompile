# Evidence bank — func_800203B4

## s1 (2026-09-01, recon)

1. **Baseline:** src/code6cac.c carried `INCLUDE_ASM` (asm-until-matched migration commit
   3a5882b2); sandbox floor with no C body = 65 (whole function, 65 target insns). Queue
   distance 33 was the stale pre-migration figure — ignore it.
2. **Canonical verdict:** ASM-PARTIAL, 11/65 insns cop2 (ctc2×5, mtc2, lwc2, swc2×3, mvmva
   `.word 0x4A486012`), four regions: [28-29],[33-35],[51-53],[56-58] — the classic
   `addu $t4,$rN,$zero` + cop2 island idiom.
3. **Cluster ruling applies (load-bearing).** The func_8002FF20 brief
   (memory/grind/func_8002FF20/brief-2026-08-18.md:11) names func_800203B4 explicitly as a
   cluster sibling of func_8002FDB0. Owner ruling 2026-08-17 (docs/grind/decisions.md, heading
   "func_8002FDB0 — OWNER RULING — GRANTED"; grant text at inline_asm_canonical.txt:268):
   the 26 queued siblings sharing the idiom inherit the canonical-asm disposition subject to
   the mechanical per-function check — sandbox --disable all == 0, zero
   pins/move-aliasing/barriers, in-island GPR limited to cop2 addressing preamble — "which the
   Judge may apply without re-escalation".
4. **Register identity proof:** the island scratch regs ARE $12-$15 ($t4=$12, $t5=$13,
   $t6=$14, $t7=$15), so target's `addu $t4,$v0,$zero` is byte-for-byte the SDK macro's
   `move $12, %0` with the compiler putting the operand in $v0. The FDB0 single-block
   spelling therefore reproduces the islands with NO pins and NO move-aliasing.
5. **Historical body:** pre-migration body (git 3a5882b2~1 src/code6cac.c:1994) matched via
   6 register-asm pins + 3 move-aliasing blocks + 2 free-standing nops (Wave 16, commit
   83dc0e5d). Its pure-C head (0x350/0x352 stores, D_8008D59E lookup with `arg1 * 20`,
   game_GetPlayerData → table index → func_8002EECC(src, mat)) is correct as-is and was
   reused verbatim. Its dead locals m0/m1/m2 were NOT needed — frame lands at 0x50 without
   them (mat[8] @ sp+0x10, vec[3] @ sp+0x30, s0/s1/ra @ 0x40/0x44/0x48).
6. **MEASURED s1: sandbox --disable all == 0, 65 == 65 insns, rules_dropped 0,** with the
   respelled body in place in src/code6cac.c (four `__asm__ volatile` islands: SetRotMatrix
   5-word ctc2 $0..$4; ldv0 pack lhu/sll/or + mtc2/lwc2 + 2 delay nops INSIDE the block;
   `.word 0x4A486012` (MVMVA sf=1 rot·V0) as its own block; stlvnl swc2 $25/$26/$27).
   Artifact: tmp/grind/func_800203B4/s1/code6cac_sandbox0.o.
7. **Remaining step is the grant, not codegen:** function is not yet in
   inline_asm_canonical.txt (dossier: canonical-listed False). Per [[judge-sole-gate]] rule 3
   the driver/Judge writes the grant line + docs/grind/borderline.md entry
   (category: canonical-asm-grant) citing the cluster ruling; then layer-2 cheat-reviewer,
   verify-oracle --rebuild, queue done. No owner wait.

## s1 retry (2026-09-01, recon — after driver discard of the first s1 for self-vet format)

8. **RE-MEASURED THIS SESSION: sandbox --disable all == 0, 65 == 65, rules_dropped 0**
   with candidate.c applied to src/code6cac.c (then reverted to INCLUDE_ASM — bytes-proven
   candidates stay in the ledger until authorization resolves; main carries no draft C per
   [[asm-until-matched]]). Artifact: tmp/grind/func_800203B4/s1/code6cac_sandbox0_retry.o.
9. **CLUSTER-MEMBERSHIP GAP (the load-bearing new finding, 2026-09-01).** The authoritative
   enumeration of the 2026-08-17 cluster ruling, `.claude/rules/cop2-addressing-preamble-cluster.md`,
   lists exactly 28 members derived by the mechanical scan "every asm/funcs/*.s line matching
   `addu $t4, $aN, $zero` whose next 6 lines contain a cop2 transfer referencing `$t4`" —
   and **func_800203B4 is NOT among them** (neither is func_8002FF20, whose 2026-08-18 brief
   called both functions cluster siblings; that brief is a session artifact, not a ruling).
   Cause: this function's three idiom sites copy from `$v0` and `$s0`, not `$aN` —
   `addu $t4,$v0,$zero` at .s lines 27 and 48 (source `$v0` = `addiu $v0,$sp,0x10/0x30`,
   compiler-materialized stack addresses) and `addu $t4,$s0,$zero` at line 59 (callee-save
   base after `addiu $s0,$s0,0x354`). The enumeration file's own idiom DEFINITION (a
   materialize-then-copy addressing preamble whose only consumer is a cop2 transfer — its
   primary example is a materialized address copied to $t4, with "$aN" given as the
   alternative) covers this shape; the SCAN REGEX under-matched it. Same SDK macro bodies
   (gte_SetRotMatrix / gte_ldv0 / MVMVA .word / gte_stlvnl), same $12-$15 footprint, same
   unfilled cop2 load-delay nops, same splat "handwritten instruction" tags as func_8002FDB0.
   But the ruling text and the enumeration are count-anchored (28 in-band / 26 queued), so
   treating this function as a member would be agent-side extension of a sanctioned
   exception — exactly what [[no-new-park-categories]] forbids agents to self-approve.
   Hence outcome = ruling-request, not candidate-ready.
10. **scan_hand_coded is LOW for this function (measured 2026-09-01):** score 1, only
   s4_front_loads fired; no S1/S2/S6. So the [[judge-sole-gate]] rule-3 pipeline grant path
   (STRONG scanner tier) is NOT available — cluster inheritance is the ONLY authorization
   route. (func_8002FDB0 itself was granted via owner ruling after escalation, not via the
   scanner path, so LOW is not evidence against hand-asm here — the islands carry the same
   per-instruction evidence the owner already accepted for FDB0.)
11. **If the ruling answers YES** (cluster covers the $v0/$s0-source spelling of the idiom),
   this function becomes a pure integration handoff: driver writes the
   inline_asm_canonical.txt grant line + borderline entry, then layer-2 cheat-reviewer on
   the applied diff, verify-oracle --rebuild, queue done. Nothing about the C body changes —
   candidate.c is final. **If NO**, the residual is 11 cop2 insns with no C form and no
   remaining sanctioned axis; the honest disposition would then be foreclosure per
   [[ordinary-c-judge-decidable]] Ruling 3 (re-activation trigger: a class grant covering
   non-$aN-source idiom sites).

## s2 (2026-09-01, recon — post-FAIL disposition session)

12. **The H4 ruling came back NO (Judge FAIL, docs/grind/decisions.md:17546, 2026-09-01
    09:01).** The FDB0 cluster grant is doubly anchored ($aN idiom text + 28/26 counts); the
    Judge's own band re-scan showed admitting the $v0/$s0-source spelling enlarges the grant
    by ~7 members — family extension, FAIL(CONSTRUCT). The Judge also independently re-ran
    scan_hand_coded: LOW, confirming fact 10. The dispatch brief's Judge constraint bound this
    session to take foreclosure per [[ordinary-c-judge-decidable]] Ruling 3, and explicitly
    forbade re-deriving/re-filing cluster membership in any spelling.
13. **RE-MEASURED ON THE CURRENT CHASSIS THIS SESSION (the dispatch chassis-check had "floor
    measurement unavailable"):** candidate.c applied to src/code6cac.c → `sandbox
    func_800203B4 --disable all` = **0, 65/65, rules_dropped 0, cheat_asm_stripped 25**
    (the four islands; identical to s1). Artifact:
    tmp/grind/func_800203B4/s1/code6cac_sandbox0_chassis_recheck_20260901.o. src reverted to
    INCLUDE_ASM after measurement per [[asm-until-matched]]. Every banked conclusion is
    therefore chassis-current as of 2026-09-01.
14. **FORECLOSURE RECORD FILED** at the tail of docs/grind/decisions.md ("2026-09-01 —
    func_800203B4 — RESOLVED BY STANDING RULING (2026-07-27): FORECLOSED"): both endgame-lock
    gates fail (scanner LOW; cluster/family NO with no other family applicable to 11 cop2
    insns). Re-activation triggers recorded there: an owner class grant covering non-$aN-source
    cop2 preamble sites (would admit ~7 functions incl. this one — then this function is a pure
    integration handoff, candidate.c byte-final), a toolchain-fidelity finding, or owner
    unpark. Session outcome: owner-gated citing that entry. **Do not re-grind this function**:
    the C body is final, the residual has no C form, and the authorization axes are measured
    dead — any future session should only check the re-activation triggers.

## s3 (2026-09-01, recon — modality-corrected re-baseline after the s2 discard)

15. **The s2 owner-gated outcome was DISCARDED by the driver validator** (marker at
    docs/grind/decisions.md:17591): the standing-ruling terminal disposition requires
    `escalation` modality (driver-declared exhaustion); in `recon` a dead axis is a
    `progress` outcome with kills banked. The foreclosure RECORD itself (decisions.md:17550)
    and the Judge FAIL (decisions.md:17546) remain filed and valid — only the session
    outcome shape was wrong. NOTE FOR FUTURE SESSIONS: do not emit owner-gated for this
    function unless your dispatched modality is `escalation`; until then the correct outcome
    is `progress` re-citing these facts.
16. **RE-MEASURED ON THE 2026-09-01 CHASSIS THIS SESSION (s3 dispatch chassis-check again
    "measurement unavailable"):** dossier CONSISTENCY OK; candidate.c applied to
    src/code6cac.c → `sandbox func_800203B4 --disable all` = **0, 65/65, rules_dropped 0,
    cheat_asm_stripped 25** — identical to facts 6/8/13. Artifact:
    tmp/grind/func_800203B4/s1/code6cac_sandbox0_s3_20260901.o. src reverted to INCLUDE_ASM
    after measurement per [[asm-until-matched]] (git diff clean). Queue distance 33 remains
    the stale pre-migration figure (fact 1); the measured floor is 0 with candidate.c.

- [s1] fact 15: s2's owner-gated was discarded ONLY for modality shape (recon vs escalation) — the Judge FAIL (decisions.md:17546) and the filed foreclosure record (decisions.md:17550) remain valid; future recon/other-modality sessions must return progress, never owner-gated, until the driver dispatches escalation

- [s1] fact 16: floor re-measured 0 on the 2026-09-01 chassis this session (dispatch chassis-check was 'measurement unavailable'); queue distance 33 is the stale pre-migration figure per fact 1; src reverted clean per asm-until-matched

- [s1] no codegen work remains: H1 CONFIRMED x4, H2/H3 KILLED, H4 resolved NO by Judge FAIL; residual = 11 cop2 insns with no C form; scanner tier LOW and cluster membership denied, so both authorization gates are measured dead

## s4 (2026-09-01, STRUCTURAL � the first non-recon modality on this function)

17. **THE PURE-C BOUND IS 26, AND IT IS STRUCTURE-INVARIANT (new, measured).** The candidate
    body with all four inline-asm islands deleted � the maximal pure-C form of this function �
    scores `sandbox --disable all` = **26, build_insns 39, target_insns 65, rules_dropped 0**
    (form banked at memory/grind/func_800203B4/rejected/pure-c-no-islands-floor-26.c). The
    score equals the EXACT instruction-count deficit (65 - 39 = 26): every one of the 39
    C-emitted instructions matches the target, so the residual is 100% instructions that no C
    construct can emit (5x ctc2, mtc2, lwc2, 3x swc2, the MVMVA `.word 0x4A486012`, their
    addressing preamble, and the two unfilled cop2 load-delay nops). GCC 2.7.2 has no cop2
    intrinsic � the PsyQ SDK's own gte_* macros are inline asm � so no structural lever
    (declaration order, block-local splits, type narrowing, statement re-association) can move
    26 toward 0. **The structural modality is bounded at 26 and is therefore measured DEAD**;
    it is not a spelling problem, it is an expressiveness one.
18. **The s1 candidate's block-local `new_var` named intermediate is CODEGEN-NEUTRAL
    (measured).** Replacing the `{ s32 new_var; new_var = game_GetPlayerData(...); src = ...; }`
    block with the plain nested expression `src = *(s32 *)((... << 2) + game_GetPlayerData(...))`
    still scores **0, 65/65, rules_dropped 0**. candidate.c has been PROMOTED to this simpler
    form (artifact tmp/grind/func_800203B4/s2/varB_no_newvar_score0.o): it removes a construct
    that would otherwise have to be defended as a named-intermediate under
    `.claude/rules/narrow-byte-args-packed-call.md`, shrinking the review surface to the four
    islands alone. Body is otherwise unchanged and still byte-final.
19. **Local DECLARATION ORDER is load-bearing (measured).** Reordering the locals from
    `s32 mat[8]; s32 vec[3]; s32 src;` to `s32 src; s32 vec[3]; s32 mat[8];` scores **25 with
    build_insns 66** � one instruction MORE than the target, i.e. the frame layout that puts
    `mat` at sp+0x10 and `vec` at sp+0x30 is a consequence of the declaration order and must
    not be "tidied". Banked at rejected/decl-order-swap-costs-one-insn.c, artifact
    tmp/grind/func_800203B4/s2/varC_declorder_score25.o.
20. **Floor re-confirmed a FIFTH time on the 2026-09-01 chassis** (dispatch chassis-check again
    read "measurement unavailable"): s1 candidate applied -> 0, 65/65, rules_dropped 0,
    cheat_asm_stripped 25; promoted varB form -> same. src/code6cac.c reverted to INCLUDE_ASM
    after every measurement (git diff clean) per [[asm-until-matched]]. Measurement table:
    tmp/grind/func_800203B4/s2/measurements.md.

- [s2] PURE-C BOUND = 26: candidate body with all four inline-asm islands deleted scores sandbox --disable all = 26 with build_insns 39 vs target 65; score == exact deficit, so zero mismatched instructions among the 39 C-emitted ones. The residual is exclusively cop2 traffic (5x ctc2, mtc2, lwc2, 3x swc2, MVMVA .word 0x4A486012) plus its addressing preamble and two unfilled cop2 load-delay nops. GCC 2.7.2 has no cop2 intrinsic, so the structural modality is bounded at 26 by expressiveness, not spelling — the axis is measured dead, not merely unexplored. Form banked: memory/grind/func_800203B4/rejected/pure-c-no-islands-floor-26.c

- [s2] candidate.c IMPROVED this session: the s1 body's block-local `new_var` named intermediate is codegen-neutral (0, 65/65 with and without it), so candidate.c now uses the plain nested expression. Net effect on any future authorization review: the only constructs left to defend are the four SDK-macro asm islands themselves — no C-side family claim is needed at all.

- [s2] Local declaration order (mat[8], vec[3], src) is LOAD-BEARING: swapping to (src, vec[3], mat[8]) gives score 25 / build_insns 66. Banked at rejected/decl-order-swap-costs-one-insn.c so it is never re-tried or 'cleaned up'.

- [s2] Floor 0 re-confirmed a fifth time on the 2026-09-01 chassis (0, 65/65, rules_dropped 0, cheat_asm_stripped 25), both for the s1 form and the promoted varB form. src/code6cac.c reverted to INCLUDE_ASM after each measurement per [[asm-until-matched]]; working tree clean apart from ledger/scratch.

- [s2] No new authorization argument was made and none was re-derived: the Judge FAIL (docs/grind/decisions.md:17546) and the filed foreclosure record (docs/grind/decisions.md:17550) stand untouched, per the binding Judge constraint in this brief. This session did not emit owner-gated because the mandated modality is `structural`, not `escalation` (s2's owner-gated from recon was discarded for exactly that reason, marker at decisions.md:17591).

## s5 (2026-09-01, STRUCTURAL - the second structural session; four NEW measured forms)

21. **THE TWO EXPLICIT LOAD-DELAY `nop`s IN THE gte_ldv0 ISLAND ARE NOT NEEDED (new, measured;
    candidate.c PROMOTED).** Deleting the two `"nop\n"` lines that followed `lwc2 $1, 8($12)`
    in the second island still scores `sandbox --disable all` = **0, 65/65, rules_dropped 0,
    cheat_asm_stripped 25** - the assembler stage supplies the cop2 load-delay padding. This is
    a pure authorization-surface win: the island is now the LITERAL PsyQ SDK gte_ldv0 macro
    body, so no instruction in ANY of the four islands is hand-invented filler. candidate.c is
    promoted to this form (artifact tmp/grind/func_800203B4/s3/candidate_s5_score0.o) and
    re-measured at 0, 65/65 after promotion.
22. **THE PURE-C BOUND OF 26 IS A MINIMUM, NOT AN ARTIFACT OF ONE SPELLING (new, measured).**
    s4 measured the islands-deleted body at 26 (build 39 == all 39 C-emitted insns matching) and
    ARGUED structure-invariance; s5 MEASURED a perturbation of it: the same islands-deleted body
    with locals reordered to (src, vec[3], mat[8]) scores **30 with build_insns 39** - the same
    39 instructions, but four of them now mismatch. Structural perturbation moves the pure-C
    bound UP, never down, so 26 is attained by the canonical form and the residual really is 26
    instructions that no C construct emits. Banked:
    rejected/purec-declorder-swap-raises-bound-26-to-30.c.
23. **STATEMENT ORDER IS LOAD-BEARING (new, measured).** Hoisting the three `vec[n] = arg2[n]`
    stores above the `func_8002EECC(src, mat)` call scores **17 with build_insns 63** - two
    FEWER instructions than the target plus 17 mismatches (GCC 2.7.2 schedules/merges the stores
    differently across the call boundary). The vec[] stores must stay between the
    gte_SetRotMatrix island and the gte_ldv0 island. Banked:
    rejected/vec-stores-hoisted-above-call.c.
24. **THE `arg0 += 0x354;` PARAM MUTATION IS A FREE CHOICE (new, measured).** Deleting it and
    re-associating the address into the stlvnl asm operand as `"r"(arg0 + 0x354)` also scores
    **0, 65/65**. Both spellings are ordinary C (no family claim either way); candidate.c keeps
    the `+=` form because it mirrors the SDK call shape. Form banked at
    tmp/grind/func_800203B4/s3/varD_ptr_expr.c so it is never re-probed.
25. **Floor re-confirmed a SIXTH and SEVENTH time on the 2026-09-01 chassis** (the s5 dispatch
    chassis-check again read "measurement unavailable"): s4 candidate -> 0, 65/65,
    rules_dropped 0, cheat_asm_stripped 25; promoted varG candidate -> identical.
    src/code6cac.c reverted to INCLUDE_ASM after every run (git diff clean) per
    [[asm-until-matched]]. Measurement table: tmp/grind/func_800203B4/s3/measurements.md.
26. **No authorization argument was made or re-derived this session.** The Judge FAIL
    (docs/grind/decisions.md:17546) and the filed foreclosure record
    (docs/grind/decisions.md:17550) stand untouched, per the binding Judge constraint. The
    outcome is `progress` because the mandated modality is `structural`, not `escalation` (the
    s2 owner-gated from `recon` was discarded for exactly that reason, marker at
    decisions.md:17591).
27. **TOOLING NOTE.** tmp/grind/func_800203B4/s2/apply.py is BROKEN on this machine (Windows
    python 3.9: `pathlib.write_text(newline=)` is 3.10+; it also choked on cp1252 bytes in the
    old candidate.c). Use tmp/grind/func_800203B4/s3/apply.py instead; candidate.c is now pure
    ASCII. A silently-failed apply reads as `score 65, build_insns 0` (INCLUDE_ASM still in
    place) or as `"score": null` with "func_800203B4 not found in ... code6cac.o" - treat both
    as apply failures, not codegen findings.

- [s3] fact 21: the two explicit load-delay nops in the gte_ldv0 island are unnecessary - deleting them still scores sandbox --disable all = 0, 65/65, rules_dropped 0, cheat_asm_stripped 25, because the assembler stage supplies cop2 load-delay padding. candidate.c PROMOTED to this form: every instruction in all four islands is now a literal PsyQ SDK gte_* macro instruction, with no hand-invented filler anywhere in the body. This is a strict authorization-surface reduction for whatever axis eventually re-activates the function.

- [s3] fact 22: the pure-C bound of 26 is now MEASURED to be a minimum, not an argued one - the islands-deleted body with locals reordered (src, vec, mat) scores 30 with the same build_insns 39, i.e. structural perturbation adds mismatches to an already-fully-matching 39-instruction set. The 26-instruction residual really is exclusively cop2 traffic (5x ctc2, mtc2, lwc2, 3x swc2, MVMVA .word 0x4A486012) plus its addressing preamble and load-delay slots, which GCC 2.7.2 cannot emit from C. The structural axis (H5, killed in s4) is confirmed dead by a second, independent kind of measurement.

- [s3] fact 23: statement order is load-bearing - hoisting the three vec[] stores above the func_8002EECC call scores 17 with build_insns 63 (two fewer instructions than target). Banked as a rejected form so no future session 'tidies' the ordering.

- [s3] fact 24: the arg0 += 0x354 param mutation is a free stylistic choice - the re-associated "r"(arg0 + 0x354) operand form also scores 0, 65/65. Banked so it is never re-probed.

- [s3] fact 25: floor re-confirmed a SIXTH and SEVENTH time on the 2026-09-01 chassis (the dispatch chassis-check again read 'measurement unavailable'): s4 candidate -> 0, 65/65, rules_dropped 0, cheat_asm_stripped 25; promoted candidate -> identical. src/code6cac.c reverted to INCLUDE_ASM after every run; working tree clean apart from ledger + scratch.

- [s3] fact 26: no authorization argument was made or re-derived - the Judge FAIL (docs/grind/decisions.md:17546) and the filed foreclosure record (docs/grind/decisions.md:17550) stand untouched per the binding Judge constraint. Outcome is `progress` and not `owner-gated` solely because the mandated modality is `structural`; the terminal disposition is modality-gated to a driver-dispatched `escalation` session (s2's owner-gated from recon was discarded for exactly that reason, marker at decisions.md:17591).

- [s3] fact 27 (tooling): tmp/grind/func_800203B4/s2/apply.py is broken on this machine (Windows python 3.9 has no pathlib.write_text(newline=), and the old candidate.c carried cp1252 bytes). Use tmp/grind/func_800203B4/s3/apply.py; candidate.c is now pure ASCII. A silently-failed apply reads as score 65 / build_insns 0, or as "score": null with 'func_800203B4 not found in ... code6cac.o' - both are apply failures, not codegen findings.
