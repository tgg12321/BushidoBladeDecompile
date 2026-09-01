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

- [s4] fact 28: the PERMUTER axis is measured DEAD. A purpose-built single-function permuter
  workspace (tmp/grind/func_800203B4/s4/perm) was run against the pure-C (islands-deleted)
  chassis for ~35k+ iterations at -j 8. Base score 3000 on asm-differ's weighted metric; the
  best novel find was 2960 and it is SEMANTICALLY INVALID (it hoists `arg0 += 0x354;` above the
  0x350/0x352 stores, shifting every later offset). Nothing came within ~350 points of the
  ~2600 that recovering even one residual instruction would require. The permuter modality has
  now been spent on this function and produced zero usable proposals; do not re-run it.

- [s4] fact 29 (why the permuter cannot help here, mechanistically): the 26-instruction residual
  splits into ~11 genuinely cop2-only instructions (5x ctc2, mtc2, lwc2, 3x swc2, the MVMVA
  .word 0x4A486012) and ~13-15 ordinary integer instructions that make up the SDK macros'
  addressing preamble (`move $12,rN`, five matrix lw's, the lhu/sll/or packing of VX0/VY0) plus
  two load-delay slots. The preamble instructions are individually C-expressible, but ONLY as
  computations whose results are never consumed (reads of mat[0..4] and a packed word built from
  vec[0]/vec[1] that feed nothing, since their real consumers are cop2 registers). GCC 2.7.2's
  DCE deletes exactly that, and any source form written to survive DCE would be a dead-read /
  constant-holder coercion aimed at materializing bytes - a cheat, not a match. So the residual
  is UNREACHABLE by C rather than merely unfound, and this is true independent of search effort.

- [s4] fact 30: floor re-confirmed an EIGHTH and NINTH time on the 2026-09-01 chassis (dispatch
  chassis-check again read "measurement unavailable"): candidate.c applied -> sandbox
  --disable all = 0, build_insns 65 == target_insns 65, rules_dropped 0, cheat_asm_stripped 25
  (artifact tmp/grind/func_800203B4/s4/code6cac_sandbox0_s4.o); islands-deleted body -> 26,
  build_insns 39. src/code6cac.c reverted to INCLUDE_ASM after both runs; tree clean.

- [s4] fact 31 (tooling, reusable): the permuter workspace recipe that WORKS for this project is
  banked at tmp/grind/func_800203B4/s4/ (mkws.sh, mkws2.sh, perm/compile.sh). Two traps cost
  real time and will cost the next session the same unless read: (a) base.c must be the
  cpp-preprocessed TU AND needs `typedef struct GameObj GameObj;` prepended or pycparser aborts
  with "Syntax error in base.c ... before: GameObj" (code6cac.c carries GameObj prototypes with
  no visible typedef - cc1 tolerates the parse error, pycparser does not; the typedef is
  codegen-neutral, verified by byte-comparing the extracted function region); (b) compile.sh
  MUST extract only the `.globl func_800203B4` .. `.end func_800203B4` region and assemble that
  alone - assembling the whole TU makes the scorer diff every other function in code6cac.c
  against a single-function target.o and yields a nonsense base score of ~1,017,894.

- [s4] fact 32 (final campaign numbers, superseding the "~35k" figure quoted in fact 28): the
  campaign ran 67,817 iterations over 1,723 s at -j 8 and was harvested with --stop (0 live
  campaigns at session end; 9 worker procs killed). FOUR novel finds total, best 2935 vs base
  3000 - every one of them below the 100-point cost of a single recovered instruction, and every
  one of them semantically invalid: output-2960-1 hoists `arg0 += 0x354;` above the 0x350/0x352
  stores; output-2935-1 re-assigns `src` from `new_var` AFTER new_var's block scope has closed
  and then dereferences it, and drops the value of the arg0 mutation entirely. This is the
  signature of a search with no valid basin to find, not of an under-sampled one.

- [s4] fact 28: the PERMUTER axis is measured DEAD - 67,817 iterations at -j 8 over the pure-C chassis, base score 3000, best novel find 2935, all four finds semantically invalid. The permuter modality has now been spent on this function and produced zero usable proposals; do not re-run it.

- [s4] fact 29: the mechanistic reason the permuter cannot help - the 26-instruction residual is ~11 cop2-only instructions plus ~13-15 ordinary integer preamble instructions that are C-expressible ONLY as DCE-deleted dead computations; surviving DCE would require a dead-read / constant-holder coercion, i.e. a cheat. The residual is unreachable by C, not merely unfound.

- [s4] fact 30: floor re-confirmed an eighth and ninth time on the 2026-09-01 chassis (dispatch chassis-check again read 'measurement unavailable'): candidate.c -> 0, 65/65, rules_dropped 0, cheat_asm_stripped 25; islands-deleted -> 26, build_insns 39. src/code6cac.c reverted to INCLUDE_ASM after every run; working tree clean apart from ledger files + scratch.

- [s4] fact 31 (tooling, reusable project-wide): the permuter-workspace recipe that WORKS on this project is banked at tmp/grind/func_800203B4/s4/ (mkws.sh, mkws2.sh, perm/compile.sh). Two traps: (a) base.c must be the cpp-preprocessed TU AND needs 'typedef struct GameObj GameObj;' prepended or pycparser aborts with 'Syntax error in base.c ... before: GameObj' (code6cac.c carries GameObj prototypes with no visible typedef - cc1 tolerates the parse error, pycparser does not); the typedef is codegen-neutral, verified by byte-comparing the extracted function region. (b) compile.sh MUST extract only the '.globl' .. '.end' region of the target function and assemble that alone - assembling the whole TU makes the scorer diff every other function in code6cac.c against a single-function target.o, yielding a nonsense base score of ~1,017,894.

- [s4] fact 32: final campaign numbers - 67,817 iterations / 1,723 s / 4 novel finds / best 2935 / harvested with --stop, 9 worker procs killed, 0 live campaigns at session end. The best find is banked at memory/grind/func_800203B4/rejected/permuter-best-find-2960-semantics-broken.c so no future session mistakes the 2960 datapoint for a lead.

## s5 (2026-09-01, SYNTHESIS - merged attack; two banked inferences upgraded to direct measurement)

33. **THE "ALL 39 C-EMITTED INSTRUCTIONS ALREADY MATCH" CLAIM IS NOW DIRECTLY VERIFIED BY
    DISASSEMBLY, not inferred from arithmetic (new).** s4/s5 established it by the argument
    "score 26 == the 65-39 deficit, therefore zero mismatches among the 39". This session
    disassembled the islands-deleted build itself
    (`mipsel-linux-gnu-objdump -d tmp/grind/func_800203B4/s5/code6cac_purec26_s5.o`, region
    `00003564 <func_800203B4>`) and compared all 39 instructions against
    asm/funcs/func_800203B4.s one by one. **Every one of the 39 is mnemonic- and
    operand-identical to its target counterpart** (17 prologue/lookup insns 800203B4-800203F4;
    7 insns 800203F8-80020410 through the func_8002EECC call; the 9-insn vec[] store block
    80020444-80020464; the 6-insn epilogue 800204A8-800204BC). `move s0,a0` / `move s1,a2` are
    the assembler's spelling of the target's `addu $s0,$a0,$zero` / `addu $s1,$a2,$zero`. The
    pure-C chassis is therefore not "close" - it is EXACT for everything C can emit, and the
    only gap is instructions C cannot emit at all. Artifact:
    tmp/grind/func_800203B4/s5/code6cac_purec26_s5.o.
34. **EXACT ANATOMY OF THE UN-REACHABLE RESIDUAL (new; refines fact 29's "~13-15 ordinary
    integer" estimate into an enumeration).** The islands-deleted build is missing 28 target
    instructions relative to asm/funcs/func_800203B4.s, in three tiers:
    - **11 cop2-only opcodes** - `ctc2` x5 (80020424, 80020428, 80020438, 8002043C, 80020440),
      `mtc2` (80020480), `lwc2` (80020484), `swc2` x3 (8002049C, 800204A0, 800204A4), and the
      MVMVA word `.word 0x4A486012` (80020490). GCC 2.7.2 has no cop2 intrinsic; the PsyQ SDK's
      own gte_* macros are inline asm. Unreachable, categorically.
    - **12 ordinary-integer instructions INSIDE the SDK macro bodies** - `addu $t4,rN,$zero` x3
      (80020418, 8002046C, 80020498), `lw` x5 (8002041C, 80020420, 8002042C, 80020430,
      80020434), `lhu` x2 (80020470, 80020474), `sll` (80020478), `or` (8002047C). Individually
      C-expressible, but only as computations whose sole consumers are cop2 registers - i.e. as
      dead code that GCC's DCE removes (fact 29). Reaching them from C would require a
      dead-read / constant-holder coercion, which is a cheat, not a match.
    - **2 cop2 load-delay `nop`s** (80020488, 8002048C) - supplied by the assembler stage when
      the island is present (fact 21), so they cost nothing in the source.
    - **PLUS 3 operand-address materializations that are NOT residual at all**:
      `addiu $v0,$sp,0x10` (80020414), `addiu $v0,$sp,0x30` (80020468),
      `addiu $s0,$s0,0x354` (80020494). These are ordinary C (`&mat[0]`, `&vec[0]`,
      `arg0 += 0x354`) and vanish from the islands-deleted build only because nothing consumes
      them once the islands are gone; they reappear automatically the moment an island uses the
      address. **So the genuinely un-authorizable-by-C set is 25 instructions, not 26-28.**
      This is the number any future class-grant record should quote for this function.
35. **TOOLING CAVEAT A (new, corrects an implicit assumption in facts 13/16/20/25/30):**
    `cheat_asm_stripped` in the sandbox JSON is NOT a per-function island counter. It reads
    **25 for BOTH** the island-bearing candidate (build_insns 65) and the islands-deleted body
    (build_insns 39) measured back-to-back this session. Do not read it as evidence about the
    function under test; only `score` / `build_insns` / `target_insns` / `rules_dropped` are.
36. **TOOLING CAVEAT B (new; softens the *form* of the s4/s5 argument without changing its
    conclusion):** `asm/funcs/func_800203B4.s` contains **67 instruction words**
    (800203B4..800204BC inclusive) while the sandbox reports `target_insns` **65**. The scorer's
    counts are therefore normalized, not raw, so the tidy identity "score 26 == 65 - 39, hence
    zero mismatches among the 39" is not airtight arithmetic. Fact 33's direct disassembly
    comparison replaces it and reaches the same conclusion by stronger evidence. Future sessions
    on any function: do not build a proof out of `target_insns - build_insns == score`; diff the
    disassembly.
37. **Floor re-confirmed a TENTH time on the 2026-09-01 chassis** (this dispatch's chassis-check
    again read "measurement unavailable"): candidate.c applied -> `sandbox --disable all` =
    **0, build_insns 65 == target_insns 65, rules_dropped 0** (artifact
    tmp/grind/func_800203B4/s5/code6cac_sandbox0_s5.o); islands-deleted body -> **26,
    build_insns 39** (artifact tmp/grind/func_800203B4/s5/code6cac_purec26_s5.o).
    src/code6cac.c reverted to INCLUDE_ASM after each run; working tree clean apart from
    ledger + scratch.
38. **Re-activation triggers re-checked this session and NONE has landed:**
    `inline_asm_canonical.txt` has **no** func_800203B4 line (grep: no match);
    `.claude/rules/cop2-addressing-preamble-cluster.md` still enumerates the same 28 members
    derived by the `addu $t4, $aN, $zero` scan and still does **not** list func_800203B4;
    no owner class grant covering non-$aN-source cop2 preamble sites has been filed. No
    membership argument was made or re-derived - this was a presence check only, per the
    binding Judge constraint.
39. **candidate.c is UNCHANGED this session** (the s5-promoted nop-free form remains the best
    known and is byte-final at floor 0). No new rejected form was produced: the synthesis found
    no untried C-side lever to spell.

- [s5] fact 33: the 'all 39 C-emitted instructions already match' claim is now DIRECTLY VERIFIED by disassembly rather than inferred from score arithmetic - objdump of the islands-deleted build (tmp/grind/func_800203B4/s5/code6cac_purec26_s5.o, region 00003564 <func_800203B4>) shows all 39 instructions mnemonic- and operand-identical to their counterparts in asm/funcs/func_800203B4.s. The pure-C chassis is not 'close', it is exact for everything C can emit.

- [s5] fact 34: exact anatomy of the un-reachable residual, refining s4's '~13-15 ordinary integer' estimate into an enumeration. The islands-deleted build is missing 28 target instructions in four tiers: (a) 11 cop2-only opcodes - ctc2 x5 at 80020424/80020428/80020438/8002043C/80020440, mtc2 at 80020480, lwc2 at 80020484, swc2 x3 at 8002049C/800204A0/800204A4, and the MVMVA word 0x4A486012 at 80020490; (b) 12 ordinary-integer instructions INSIDE the SDK macro bodies - addu $t4,rN,$zero x3 at 80020418/8002046C/80020498, lw x5 at 8002041C/80020420/8002042C/80020430/80020434, lhu x2 at 80020470/80020474, sll at 80020478, or at 8002047C, all C-expressible only as DCE-deleted dead code; (c) 2 assembler-supplied cop2 load-delay nops at 80020488/8002048C (free, per fact 21); (d) 3 operand-address materializations at 80020414/80020468/80020494 that are ordinary C (&mat[0], &vec[0], arg0 += 0x354) and return automatically once an island consumes them. Therefore the genuinely un-authorizable-by-C set is 25 instructions, not 26-28 - this is the number any future class-grant record should quote.

- [s5] fact 35 (tooling): cheat_asm_stripped is NOT a per-function island counter - it reads 25 for both the island-bearing candidate (build_insns 65) and the islands-deleted body (build_insns 39), measured back-to-back this session. Facts 13/16/20/25/30 quote it as corroboration; that corroboration is void. Use build_insns as the apply/removal check.

- [s5] fact 36 (tooling, methodological): asm/funcs/func_800203B4.s contains 67 instruction words (800203B4..800204BC inclusive) while the sandbox reports target_insns 65, so the scorer's counts are normalized rather than raw and the tidy identity 'score 26 == 65 - 39, hence zero mismatches among the 39' is not airtight arithmetic. Fact 33's disassembly comparison replaces it and reaches the same conclusion by stronger evidence. Project-wide lesson: do not build a proof out of target_insns - build_insns == score; diff the disassembly.

- [s5] fact 37: floor re-confirmed a TENTH time on the 2026-09-01 chassis (this dispatch's chassis-check again read 'measurement unavailable') - candidate.c applied -> sandbox --disable all = 0, build_insns 65 == target_insns 65, rules_dropped 0 (tmp/grind/func_800203B4/s5/code6cac_sandbox0_s5.o); islands-deleted body -> 26, build_insns 39 (tmp/grind/func_800203B4/s5/code6cac_purec26_s5.o). src/code6cac.c reverted to INCLUDE_ASM after each run per [[asm-until-matched]]; working tree clean apart from the two ledger files and scratch.

- [s5] fact 38: re-activation triggers re-checked and NONE has landed - no func_800203B4 line in inline_asm_canonical.txt, .claude/rules/cop2-addressing-preamble-cluster.md still enumerates the same 28 `addu $t4, $aN, $zero` members without this function, and no owner class grant for non-$aN-source cop2 preamble sites is filed. Presence check only; no membership argument was made or re-derived, per the binding Judge constraint.

- [s5] fact 39: candidate.c is UNCHANGED this session (the s5-promoted nop-free form remains best-known and byte-final at floor 0) and no new rejected form was produced - the synthesis found no untried C-side lever to spell. The merged attack is written up at the head of the s6 section of memory/grind/func_800203B4/hypotheses.md: 39 exact C insns + 3 free operand materializations + 25 un-C-expressible insns = the whole 65-instruction function.

- [s5] fact 40: the disposition state is unchanged and remains modality-gated - the Judge FAIL (docs/grind/decisions.md:17546) and the filed proof-of-foreclosure record (docs/grind/decisions.md:17550) stand untouched; this session returns `progress` and not `owner-gated` solely because the mandated modality is `synthesis`, not `escalation` (s2's owner-gated from `recon` was discarded for exactly that reason, marker at decisions.md:17591).

## s6 (2026-09-01, SYNTHESIS - the second synthesis session; the asm surface is now MEASURED minimal)

41. **THE 25-INSTRUCTION ASM AUTHORIZATION SURFACE IS MINIMAL (new, measured three ways -
    the one lever s1-s5 never spelled).** Every prior session treated the four islands as
    atomic SDK macro bodies and asked only "is the whole island authorizable?". s6 asked the
    complementary question: can the C-expressible parts of the macro bodies (fact 34 tier b -
    the `move $12,rN` preambles, the five matrix `lw`s, the `lhu/lhu/sll/or` VX0/VY0 packing)
    be moved OUT of the asm into C, leaving smaller cop2-only islands? All three partitions
    were spelled and measured; ALL are non-zero:
    - **varH (thin gte_SetRotMatrix)** - `s32 r0..r4 = mat[0..4];` in C, asm reduced to five
      `ctc2 %n,$n`: **score 12, build_insns 63** (artifact
      tmp/grind/func_800203B4/s6/varH_thin_setrotmatrix.o, form
      rejected/thin-setrotmatrix-c-loads-score12.c). GCC issues the five loads in its own
      order/registers and drops the `move $12` + one load; the target's interleave
      (lw,lw,ctc2,ctc2,lw,lw,lw,ctc2,ctc2,ctc2) is not reproducible from separated C loads.
    - **varI (thin gte_stlvnl)** - the three `swc2` addressing the operand register directly
      (`swc2 $25, 0(%0)`), no `move $12, %0`: **score 4, build_insns 64** (artifact
      varI_thin_stlvnl.o, form rejected/thin-stlvnl-no-preamble-score4.c). Losing the
      preamble costs the `addu $t4,$s0,$zero` at 80020498 AND re-bases all three swc2 off the
      wrong register - 4 mismatches for one "removed" instruction.
    - **varJ (thin gte_ldv0)** - the packing written in C as
      `u32 pack = ((u32)*(u16*)&vec[1] << 16) | *(u16*)&vec[0];` with only `mtc2`/`lwc2` in
      asm: **score 8, build_insns 64** (artifact varJ_thin_ldv0.o, form
      rejected/thin-ldv0-c-packing-score8.c). This was the most promising partition (the
      packing is genuinely ordinary C) and it still fails: GCC materializes the halfword
      reads and the shift/or in different registers and a different order from the SDK
      macro's fixed $13/$14 sequence.
    **Consequence for the ledger:** the 25 instructions named in fact 34 are not merely
    "currently inside asm" - they are byte-forced to be inside asm. There is no partition of
    this function into (more C, less asm) that reaches 0, so no future session and no future
    authorization reviewer can shrink the surface below 25. Any class-grant record may state
    minimality as MEASURED, not argued.
42. **Floor re-confirmed an ELEVENTH time on the 2026-09-01 chassis** (this dispatch's
    chassis-check again read "measurement unavailable"): candidate.c applied to
    src/code6cac.c -> `sandbox func_800203B4 --disable all` = **0, build_insns 65 ==
    target_insns 65, rules_dropped 0** (artifact
    tmp/grind/func_800203B4/s6/code6cac_sandbox0_s6.o). src/code6cac.c reverted to
    INCLUDE_ASM after every one of the four builds this session (`git status` clean apart
    from metrics/events.jsonl + ledger + scratch) per [[asm-until-matched]].
43. **Re-activation triggers re-checked (fact 38 repeat) and NONE has landed:**
    `grep func_800203B4 inline_asm_canonical.txt` - no match;
    `.claude/rules/cop2-addressing-preamble-cluster.md` - zero occurrences of func_800203B4,
    enumeration unchanged; `docs/grind/decisions.md` tail is still the 09:05 discarded-session
    marker, i.e. no owner class grant for non-$aN-source cop2 preamble sites has been filed.
    Presence check only - no membership argument was made or re-derived, per the binding
    Judge constraint.
44. **candidate.c is UNCHANGED as code** (the s5 nop-free form remains byte-final at floor 0);
    only its header comment gained the fact-41 minimality note. No C-side lever remains
    unspelled that any session has been able to name: structural (s4), permuter (s5 predecessor
    s4), synthesis-by-disassembly (s5) and now island-partition (s6) are all measured dead.

- [s6] fact 41: the 25-instruction asm authorization surface is MEASURED minimal. Three island-partition variants that move the C-expressible parts of the SDK macro bodies out of asm into C all score non-zero: varH thin gte_SetRotMatrix (C-loaded matrix words, ctc2-only asm) = 12 / build 63; varI thin gte_stlvnl (swc2 addressing the operand register, no `move $12` preamble) = 4 / build 64; varJ thin gte_ldv0 (VX0/VY0 packing written in C, mtc2+lwc2 only in asm) = 8 / build 64. Forms banked in memory/grind/func_800203B4/rejected/, objects in tmp/grind/func_800203B4/s6/. The 25 instructions of fact 34 are byte-forced to live inside asm; minimality is now measured, not argued.

- [s6] fact 42: floor re-confirmed an eleventh time on the 2026-09-01 chassis (dispatch chassis-check again read 'measurement unavailable') - candidate.c -> sandbox --disable all = 0, build_insns 65 == target_insns 65, rules_dropped 0 (tmp/grind/func_800203B4/s6/code6cac_sandbox0_s6.o). src reverted to INCLUDE_ASM after all four builds; tree clean apart from metrics/events.jsonl, ledger and scratch.

- [s6] fact 43: re-activation triggers re-checked and none has landed - no func_800203B4 line in inline_asm_canonical.txt, zero occurrences in .claude/rules/cop2-addressing-preamble-cluster.md, decisions.md tail still the 09:05 discarded-session marker. Presence check only.

- [s6] fact 44: candidate.c unchanged as code (s5 nop-free form, byte-final at 0); header comment updated with the minimality note. Four distinct axes are now measured dead - structural, permuter, disassembly-synthesis and island-partition.

- [s6] fact 41: the 25-instruction asm authorization surface is MEASURED minimal - varH thin gte_SetRotMatrix = 12/build 63, varI thin gte_stlvnl (no 'move $12' preamble) = 4/build 64, varJ thin gte_ldv0 (packing in C) = 8/build 64. The 12 ordinary-integer SDK-macro-body instructions of fact 34 tier (b) are byte-forced to live inside asm: as C dead code GCC's DCE deletes them (fact 29), and as C live code feeding asm operands GCC emits them in the wrong registers and order. No partition of this function into (more C, less asm) reaches 0, so 25 is the minimum authorizable surface and a future class-grant record may state minimality as measured rather than argued.

- [s6] fact 42: floor re-confirmed an ELEVENTH time on the 2026-09-01 chassis (dispatch chassis-check again read 'measurement unavailable') - candidate.c -> sandbox --disable all = 0, build_insns 65 == target_insns 65, rules_dropped 0 (tmp/grind/func_800203B4/s6/code6cac_sandbox0_s6.o). src/code6cac.c reverted to INCLUDE_ASM after all five builds this session per [[asm-until-matched]]; working tree clean apart from metrics/events.jsonl, the ledger files and scratch.

- [s6] fact 43: re-activation triggers re-checked and NONE has landed - no func_800203B4 line in inline_asm_canonical.txt, zero occurrences in .claude/rules/cop2-addressing-preamble-cluster.md, decisions.md tail still the 09:05 discarded-session marker. Presence check only; no membership argument made or re-derived.

- [s6] fact 44: candidate.c is unchanged as code (the s5 nop-free form remains byte-final at floor 0); only its header comment gained the fact-41 minimality note. Four distinct axes are now measured dead on this function - structural (s4), permuter (s4/s5), disassembly-synthesis (s5) and island-partition (s6).

- [s6] merged attack (written into hypotheses.md): 39 C-emitted instructions are byte-exact by disassembly (fact 33) + 3 operand-address materializations that return for free once an island consumes them (fact 34d) + 25 byte-forced asm instructions (11 cop2 opcodes GCC 2.7.2 cannot emit, 12 SDK-macro integer instructions proven irreducible by fact 41, 2 assembler-supplied cop2 load-delay nops) = the whole 65-instruction function. The codegen question is closed in BOTH directions; what remains is purely authorization, and both gates are measured shut (scan_hand_coded LOW, cluster membership denied by Judge FAIL decisions.md:17546).

- [s6] modality note: this session returns 'progress' and not 'owner-gated' solely because the mandated modality is synthesis, not escalation - s2's owner-gated from recon was discarded for exactly that reason (marker docs/grind/decisions.md:17591). The proof-of-foreclosure record at docs/grind/decisions.md:17550 stands filed and untouched; nothing was re-filed or re-argued this session.

- [s7] fact 45: **the SOLVER axis is now mechanically FORECLOSED by a typed verdict, not by
  inference.** The mandated first solver step (`inverse_compose.py classify code6cac
  func_800203B4`) refuses on this function by design - it is zero-rule since rules-to-zero
  2026-08-25, so the src-derived `code6cac.tgt.s` cannot carry target's stream and the text
  classifier would emit a FICTITIOUS PRE-RA verdict; the tool itself redirects to the
  object-level classifier `tools/ra_solver/goal_from_tgt.py classify` (guard text quoted in
  tmp/grind/func_800203B4/s7/classify_purec.txt provenance). Run object-level against the
  pure-C (islands-deleted) chassis - the only chassis on which a residual exists at all -
  the verdict is: **FIRST DIVERGENCE: PRE-RA / "next tool: none - the residual is upstream of
  every model"**, ours 39 insns vs target 65. The shapes present in ONE stream only are all
  target-only and all island instructions (3x `move #,#`, the five `lw #,N(#)` matrix loads,
  `lhu #,0(#)`, `addiu #,#,16`, and the `ctc2 #,$0..$3` cop2 writes). Artifacts:
  tmp/grind/func_800203B4/s7/classify_purec.txt, object
  tmp/grind/func_800203B4/s7/code6cac_purec39_s7.o (sandbox = 26, build_insns 39). This is the
  same conclusion facts 33/34/41 reached by disassembly and by partition, but it is now the
  solver stack's OWN typed answer: a PRE-RA residual is an instruction-MULTISET difference,
  and neither ra_solver (which renames a fixed multiset) nor sched_solver (which reorders a
  fixed, already-allocated stream) can reach it. No RA seat exists to re-assign and no
  emission order exists to permute, so no vector search was run and none should ever be run
  on this function.
- [s7] fact 46: **the candidate chassis classifies as NO DIVERGENCE** - with
  memory/grind/func_800203B4/candidate.c applied to src/code6cac.c, `goal_from_tgt.py classify
  code6cac func_800203B4` reports "ours 65 insns, target 65 insns ... NO DIVERGENCE: the two
  streams are identical" (tmp/grind/func_800203B4/s7/classify_candidate.txt). Two independent
  tools now agree the body is byte-final: the engine sandbox (score 0) and the solver stack's
  object-level stream comparator. There is no residual for a solver to own on the candidate
  chassis and a PRE-RA (expressiveness) residual on the pure-C chassis - the two ends of the
  same fact.
- [s7] fact 47: **floor re-confirmed a TWELFTH and THIRTEENTH time on the 2026-09-01 chassis**
  (this dispatch's chassis-check again read "measurement unavailable"): candidate.c applied ->
  `sandbox func_800203B4 --disable all` = **0, build_insns 65 == target_insns 65,
  rules_dropped 0, cheat_asm_stripped 25**, measured twice this session (artifact
  tmp/grind/func_800203B4/s7/code6cac_sandbox0_s7.o). src/code6cac.c reverted to INCLUDE_ASM
  after every build per [[asm-until-matched]]; working tree clean apart from
  metrics/events.jsonl and the ledger.
- [s7] fact 48: **re-activation triggers re-checked (fact 43 repeat) - NONE has landed.**
  `grep -c func_800203B4 inline_asm_canonical.txt` = 0;
  `grep -c func_800203B4 .claude/rules/cop2-addressing-preamble-cluster.md` = 0;
  `tail -3 docs/grind/decisions.md` is still the 2026-09-01 09:05 discarded-session marker,
  i.e. no owner class grant for non-$aN-source cop2 addressing-preamble sites has been filed.
  Presence check only - no membership argument was made or re-derived, per the binding Judge
  constraint.
- [s7] fact 49: **tooling note (repo hygiene, not codegen)** -
  memory/grind/func_800203B4/rejected/pure-c-no-islands-floor-26.c carried CP1252/UTF-8-mixed
  bytes (0x97, 0x94, 0x80, 0xe2) in its header comment, which made
  `engine/inlineasm.py:write_stripped` (a strict `read_text(encoding="utf-8")`) raise
  UnicodeDecodeError the moment the form was applied to src - i.e. the banked pure-C form was
  UNBUILDABLE as saved. Sanitized to pure ASCII this session; the C body is byte-unchanged.
  Any future session re-spending a banked rejected/*.c should expect the same trap in other
  ledgers.

- [s7] fact 45: the SOLVER axis is mechanically FORECLOSED by a typed verdict. inverse_compose.py classify refuses on this zero-rule function by design and redirects to the object-level classifier; goal_from_tgt.py classify on the islands-deleted pure-C chassis returns FIRST DIVERGENCE: PRE-RA with 'next tool: none - the residual is upstream of every model' (ours 39 insns vs target 65). All one-stream-only shapes are target-only island instructions. Neither ra_solver nor sched_solver can reach a multiset difference, so no RA seat exists to re-assign and no emission order exists to permute. This is the same conclusion facts 33/34/41 reached by disassembly and by partition, now stated by the solver stack itself.

- [s7] fact 46: the candidate chassis classifies as NO DIVERGENCE - ours 65 insns, target 65 insns, streams identical. Two independent tools now agree the body is byte-final (engine sandbox score 0 and the solver stack's object-level stream comparator). No residual for a solver to own on the candidate chassis; a PRE-RA expressiveness residual on the pure-C chassis. Two ends of the same fact.

- [s7] fact 47: floor re-confirmed a twelfth and thirteenth time on the 2026-09-01 chassis (dispatch chassis-check again read 'measurement unavailable') - candidate.c -> sandbox --disable all = 0, build_insns 65 == target_insns 65, rules_dropped 0, cheat_asm_stripped 25. src/code6cac.c reverted to INCLUDE_ASM after every build; tree clean apart from metrics/events.jsonl and the ledger.

- [s7] fact 48: re-activation triggers re-checked and NONE has landed - grep -c func_800203B4 inline_asm_canonical.txt = 0; grep -c func_800203B4 .claude/rules/cop2-addressing-preamble-cluster.md = 0; docs/grind/decisions.md tail is still the 2026-09-01 09:05 discarded-session marker, i.e. no owner class grant for non-$aN-source cop2 addressing-preamble sites has been filed. Presence check only - no membership argument made or re-derived, per the binding Judge constraint.

- [s7] fact 49: tooling/hygiene - memory/grind/func_800203B4/rejected/pure-c-no-islands-floor-26.c carried CP1252/UTF-8-mixed bytes (0x97, 0x94, 0x80, 0xe2) in its header comment, which made engine/inlineasm.py:write_stripped (strict read_text(encoding='utf-8')) raise UnicodeDecodeError as soon as the form was applied to src - the banked pure-C form was UNBUILDABLE as saved. Sanitized to pure ASCII this session; the C body is byte-unchanged. Future sessions re-spending a banked rejected/*.c should expect the same trap in other ledgers.

- [s7] no docs/grind/decisions.md entry was filed and no cluster-membership argument was made this session, per the binding Judge constraint and the s1 discard marker (docs/grind/decisions.md:17591): the proof-of-foreclosure record at docs/grind/decisions.md:17550 stands filed and untouched.
