# Hypothesis ledger — func_800203B4

## s1 (2026-09-01, recon)

- **H1 — CONFIRMED (measured, closes the function):** "The func_8002FDB0-authorized
  single-block island spelling (hardcoded $12-$15 inside the asm template, operand as `"r"`,
  explicit clobbers) transplanted onto the pre-migration pure-C head reproduces all 65
  instructions." Mechanism: $t4..$t7 ARE $12..$15, so the SDK-macro `move $12,%0` emits the
  target's `addu $t4,$rN,$zero`; GCC materializes the operand addresses (`addiu $v0,$sp,0x10`
  / `addiu $v0,$sp,0x30` / `addiu $s0,$s0,0x354` via `arg0 += 0x354`) exactly as the target.
  Probe: applied body, `sandbox func_800203B4 --disable all`. Result: **0, 65==65,
  rules_dropped 0, first measurement.** No permuter, no dumps needed.
- H2 (never needed): dead locals m0/m1/m2 from the historical body required for frame size —
  KILLED implicitly: frame is 0x50 without them (score 0 includes prologue/epilogue).

Frontier: empty — bytes proven; residual is the driver-side canonical-asm grant per the
2026-08-17 cluster ruling (see evidence.md fact 7).

## s1 retry (2026-09-01, recon)

- **H1 re-CONFIRMED this session:** candidate.c applied to src/code6cac.c scores
  sandbox --disable all == 0 (65/65, rules_dropped 0). Artifact:
  tmp/grind/func_800203B4/s1/code6cac_sandbox0_retry.o.
- **H3 — KILLED (measured):** "scan_hand_coded gives STRONG tier, opening the
  [[judge-sole-gate]] rule-3 grant path without cluster membership." Result: tier LOW,
  score 1 (s4 only). The scanner path is closed; cluster inheritance is the only route.
- **H4 — OPEN (the ruling question):** "The 2026-08-17 FDB0 cluster ruling covers the
  $v0/$s0-source spelling of the materialize-then-copy cop2 idiom." Cannot be resolved
  agent-side: the enumeration (.claude/rules/cop2-addressing-preamble-cluster.md) is
  count-anchored at 28 and omits this function (evidence.md fact 9). Emitted as
  ruling-request 2026-09-01.

Frontier: the H4 ruling. No codegen work remains — do NOT spend sessions re-measuring;
the C body is final in candidate.c. Next session: check whether the ruling landed
(docs/grind/decisions.md / an updated cluster-membership file / a grant line in
inline_asm_canonical.txt); if YES follow evidence.md fact 11's integration steps via
candidate-ready; if NO, foreclosure per fact 11.

## s2 (2026-09-01, recon — disposition)

- **H4 RESOLVED NO** by Judge FAIL ruling docs/grind/decisions.md:17546 (2026-09-01 09:01):
  cluster grant is count- and idiom-anchored; the $v0/$s0-source spelling is a family
  extension. Binding Judge constraint in the s2 brief mandated foreclosure per
  [[ordinary-c-judge-decidable]] Ruling 3.
- **H1 re-CONFIRMED on the current chassis** (dispatch chassis-check was "unavailable"):
  sandbox --disable all == 0, 65/65. Artifact:
  tmp/grind/func_800203B4/s1/code6cac_sandbox0_chassis_recheck_20260901.o.
- Foreclosure record filed at decisions.md tail; outcome owner-gated.

Frontier: EMPTY — foreclosed. Only the re-activation triggers matter (owner class grant for
non-$aN-source cop2 preamble sites / toolchain finding / unpark). On re-activation, apply
candidate.c and follow evidence.md fact 11's integration steps.

## s3 (2026-09-01, recon — modality-corrected re-baseline)

- **H1 re-CONFIRMED on the 2026-09-01 chassis (fourth independent measurement):**
  candidate.c applied → sandbox --disable all == 0, 65/65, rules_dropped 0. Artifact:
  tmp/grind/func_800203B4/s1/code6cac_sandbox0_s3_20260901.o. src reverted.
- No new hypotheses: every codegen question is closed (H1 CONFIRMED ×4, H2/H3 KILLED,
  H4 resolved NO by Judge FAIL decisions.md:17546). The residual is 11 cop2 insns with no
  C form and both authorization gates measured dead.

Frontier (for the driver's ladder, not for codegen work): the ONLY remaining action is the
standing-ruling terminal disposition, which is modality-gated — it must be emitted from a
driver-dispatched `escalation` session (s2's owner-gated from `recon` was discarded for
exactly this). An escalation-modality session should: verify no re-activation trigger has
landed (grep .claude/rules/cop2-addressing-preamble-cluster.md for a widened scan /
inline_asm_canonical.txt for a grant / decisions.md tail for an owner class grant), then
return owner-gated with escalation_ref = the already-filed record at
docs/grind/decisions.md:17550. Do NOT re-measure the floor beyond one confirming sandbox
run, do NOT re-file the record (it exists), do NOT re-argue cluster membership (Judge
constraint forbids it in any spelling).

## [s1] H1 (re-run): candidate.c (FDB0-authorized single-block island spelling on the pre-migration pure-C head) reproduces all 65 target instructions on the 2026-09-01 chassis
- mechanism: SDK-macro islands hardcode $12-$15 internally (no pins, no move-aliasing); GCC materializes operand addresses (addiu $v0,$sp,0x10/0x30; arg0+=0x354 into $s0) exactly as target; sandbox strips the 4 islands (25 insns) and scores the C chassis 0
- probe: dossier func_800203B4 (CONSISTENCY OK) -> applied memory/grind/func_800203B4/candidate.c to src/code6cac.c -> sandbox func_800203B4 --disable all -> reverted src to INCLUDE_ASM
- result: score 0, 65/65 insns, rules_dropped 0, cheat_asm_stripped 25 — identical to the three prior measurements (evidence.md facts 6, 8, 13)
- verdict: CONFIRMED

## s4 (2026-09-01, structural)

- **H5 � KILLED (measured):** "A structural rearrangement of the pure-C body (declaration
  order / block-local splits / re-association / type narrowing) can close part of the residual
  without inline asm." Mechanism proposed: frame-layout and expression-shape levers change
  which instructions GCC emits. Probe: islands deleted -> `sandbox --disable all` = 26 with
  build_insns 39 (score == exact 65-39 deficit, so ALL 39 emitted insns already match); then
  two structural perturbations of the full body. Result: the entire 26-insn residual is cop2
  transfers + the MVMVA word + their preamble/delay nops, which GCC 2.7.2 emits ONLY from
  inline asm. **Verdict: KILLED � the structural axis is bounded at 26 by expressiveness, not
  by spelling.** (evidence.md fact 17)
- **H6 � KILLED (measured, and it IMPROVED the candidate):** "The s1 body's block-local
  `new_var` named intermediate is load-bearing for the target's codegen." Probe: replaced with
  the plain nested expression -> still 0, 65/65. **Verdict: KILLED � codegen-neutral;
  candidate.c promoted to the simpler form** (evidence.md fact 18).
- **H7 � CONFIRMED (measured):** "Local declaration order is load-bearing for the frame
  layout." Probe: (src, vec, mat) instead of (mat, vec, src) -> score 25, build_insns 66.
  **Verdict: CONFIRMED � do not reorder** (evidence.md fact 19).

Frontier: UNCHANGED and still not codegen. Every C-side question is now closed on measurement
(H1 CONFIRMED x5, H2/H3/H5/H6 KILLED, H4 resolved NO by Judge FAIL decisions.md:17546,
H7 CONFIRMED). The only remaining action is the standing-ruling terminal disposition, which is
modality-gated to a driver-dispatched `escalation` session (s2's owner-gated from recon was
discarded for exactly this; s4 is `structural`, so this session returns `progress`). An
escalation session should: check the re-activation triggers (a widened scan in
.claude/rules/cop2-addressing-preamble-cluster.md / a grant line in inline_asm_canonical.txt /
an owner class grant in the decisions.md tail), then return owner-gated with
escalation_ref = docs/grind/decisions.md:17550. Do NOT re-measure beyond one confirming run,
do NOT re-file the foreclosure record, do NOT re-argue cluster membership in any spelling.

## [s2] A structural rearrangement of the pure-C body (declaration order / block-local splits / re-association / type narrowing) can close part of the residual without inline asm.
- mechanism: frame-layout and expression-shape levers change which instructions GCC 2.7.2 emits; if any part of the 65-insn target were reachable by C spelling, a structural perturbation would move the score.
- probe: deleted all four inline-asm islands from candidate.c -> applied to src/code6cac.c:1813 -> `sandbox func_800203B4 --disable all`
- result: score 26, build_insns 39, target_insns 65, rules_dropped 0 — score equals the EXACT deficit 65-39, so all 39 C-emitted instructions match; the whole residual is 5x ctc2 / mtc2 / lwc2 / 3x swc2 / MVMVA .word 0x4A486012 plus their addressing preamble and two unfilled cop2 load-delay nops, none of which GCC 2.7.2 can emit from C (the PsyQ gte_* macros are themselves inline asm).
- verdict: KILLED

## [s2] The s1 candidate's block-local `new_var` named intermediate around game_GetPlayerData is load-bearing for the target codegen.
- mechanism: named-intermediate declaration order affects GCC 2.7.2 temp allocation / evaluation order.
- probe: replaced the { s32 new_var; ... } block with the plain nested expression; sandbox --disable all
- result: score 0, 65/65, rules_dropped 0 — identical. Codegen-neutral. candidate.c PROMOTED to the simpler form, removing a construct that would otherwise need defending as a named-intermediate family claim.
- verdict: KILLED

## [s2] Local declaration order (mat[8], vec[3], src) is load-bearing for the target frame layout.
- mechanism: GCC 2.7.2 assigns frame slots in declaration order; target has mat @ sp+0x10 and vec @ sp+0x30.
- probe: reordered to (src, vec[3], mat[8]); sandbox --disable all
- result: score 25, build_insns 66 (one insn MORE than target) — order is required; banked as a rejected form so no future session 'tidies' it.
- verdict: CONFIRMED

## [s2] The banked floor of 0 for candidate.c still holds on the current chassis (dispatch chassis-check read 'measurement unavailable').
- mechanism: chassis-relative re-measurement per the dispatch contract.
- probe: applied memory/grind/func_800203B4/candidate.c (s1 form) -> sandbox --disable all
- result: score 0, 65/65, rules_dropped 0, cheat_asm_stripped 25 — fifth independent confirmation; src reverted to INCLUDE_ASM after every measurement (git diff clean).
- verdict: CONFIRMED

## s5 (2026-09-01, structural - second structural session)

- **H8 - KILLED (measured, and it IMPROVED the candidate):** "The two explicit load-delay
  `nop`s inside the gte_ldv0 island are load-bearing for the byte match." Mechanism proposed:
  the target's two unfilled cop2 load-delay slots are literal bytes that must come from
  somewhere in the source. Probe: deleted both `"nop\n"` lines -> `sandbox --disable all`.
  Result: **0, 65/65, rules_dropped 0, cheat_asm_stripped 25** - the assembler stage emits the
  load-delay padding itself. **Verdict: KILLED; candidate.c PROMOTED** to the nop-free form,
  which is the literal PsyQ SDK gte_ldv0 macro body (no hand-invented instruction remains in
  any island). (evidence.md fact 21)
- **H9 - CONFIRMED (measured; upgrades an s4 ARGUMENT into a MEASUREMENT):** "The pure-C bound
  of 26 is a minimum attained by the canonical body, not an artifact of one spelling."
  Mechanism: if any of the 39 C-emitted instructions were spelling-dependent, a structural
  perturbation could trade some of them; if the residual is purely cop2 expressiveness, a
  perturbation can only add mismatches. Probe: islands-deleted body with locals reordered
  (src, vec, mat) -> score 30, build_insns 39. **Verdict: CONFIRMED - the bound moves UP under
  perturbation; 26 is the floor and the structural axis stays DEAD** (H5, s4).
  (evidence.md fact 22)
- **H10 - CONFIRMED (measured):** "Statement order (the vec[] stores relative to the
  func_8002EECC call) is load-bearing." Probe: hoisted the three vec[] stores above the call ->
  score 17, build_insns 63. **Verdict: CONFIRMED - do not reorder statements.**
  (evidence.md fact 23; rejected/vec-stores-hoisted-above-call.c)
- **H11 - KILLED (measured):** "The `arg0 += 0x354;` param mutation is load-bearing." Probe:
  deleted it and re-associated into the stlvnl operand as `"r"(arg0 + 0x354)` -> 0, 65/65.
  **Verdict: KILLED - codegen-neutral, a free stylistic choice; the candidate keeps the `+=`
  form.** (evidence.md fact 24)

Frontier: UNCHANGED and still not codegen. Every C-side question on this function is closed on
measurement (H1 CONFIRMED x7, H2/H3/H5/H6/H8/H11 KILLED, H7/H9/H10 CONFIRMED, H4 resolved NO by
Judge FAIL decisions.md:17546). The body is byte-final at floor 0 and its authorization surface
is now minimal: four inline-asm islands, each a literal PsyQ SDK gte_* macro body, plus a pure-C
head that needs no sanctioned-family claim at all. The only remaining action is the
standing-ruling terminal disposition, which is modality-gated to a driver-dispatched
`escalation` session. An escalation session should: check the re-activation triggers (a widened
scan in .claude/rules/cop2-addressing-preamble-cluster.md / a grant line in
inline_asm_canonical.txt / an owner class grant in the decisions.md tail), then return
owner-gated with escalation_ref = docs/grind/decisions.md:17550. Do NOT re-measure beyond one
confirming run, do NOT re-file the foreclosure record, do NOT re-argue cluster membership in any
spelling.

## [s3] H8: the two explicit load-delay `nop` lines inside the gte_ldv0 island are load-bearing for the byte match (the target's two unfilled cop2 delay slots must come from somewhere in the source).
- mechanism: If the assembler stage (maspsx/as) supplies cop2 load-delay padding itself, the hand-written nops are redundant filler inside the asm template rather than required instructions.
- probe: Deleted both "nop\n" lines after `lwc2 $1, 8($12)` from candidate.c, applied to src/code6cac.c:1813, `sandbox func_800203B4 --disable all` (tmp/grind/func_800203B4/s3/varG_no_explicit_nops.c).
- result: score 0, build_insns 65, target_insns 65, rules_dropped 0, cheat_asm_stripped 25 - identical to the form with the nops. candidate.c PROMOTED to the nop-free form and re-measured at 0, 65/65 (artifact candidate_s5_score0.o).
- verdict: KILLED

## [s3] H9: the pure-C bound of 26 (islands deleted, build_insns 39 == all 39 C-emitted instructions matching) is a MINIMUM attained by the canonical body, not an artifact of one spelling - i.e. s4's structure-invariance claim is measurable, not merely argued.
- mechanism: If any of the 39 C-emitted instructions were spelling-dependent, a structural perturbation could trade some of them for target instructions; if the entire residual is cop2 expressiveness, a perturbation can only ADD mismatches to an already-fully-matching set.
- probe: Islands-deleted body with locals reordered to (src, vec[3], mat[8]) applied and scored: tmp/grind/func_800203B4/s3/varE_purec_declorder.c.
- result: score 30 with build_insns 39 - the same 39 instructions, four of which now mismatch (canonical order scores 26 == the exact 65-39 deficit). The bound moves UP under perturbation. Banked at memory/grind/func_800203B4/rejected/purec-declorder-swap-raises-bound-26-to-30.c.
- verdict: CONFIRMED

## [s3] H10: statement order - specifically the position of the three vec[n] = arg2[n] stores relative to the func_8002EECC(src, mat) call - is load-bearing for the target codegen.
- mechanism: GCC 2.7.2 schedules and merges stores differently across a call boundary (call-clobbered registers force different reload placement), so moving the stores past the call changes the emitted instruction stream.
- probe: Hoisted the three vec[] stores above the func_8002EECC call, applied, scored: tmp/grind/func_800203B4/s3/varF_vec_hoist.c.
- result: score 17 with build_insns 63 - two FEWER instructions than the 65-insn target plus 17 mismatches. The stores must stay between the gte_SetRotMatrix island and the gte_ldv0 island. Banked at memory/grind/func_800203B4/rejected/vec-stores-hoisted-above-call.c.
- verdict: CONFIRMED

## [s3] H11: the `arg0 += 0x354;` param mutation before the gte_stlvnl island is load-bearing (the target materializes the destination address in $s0 via that add).
- mechanism: Re-associating the offset into the asm operand expression would let GCC fold it elsewhere or emit the addiu at a different point in the stream.
- probe: Deleted `arg0 += 0x354;` and wrote the stlvnl operand as "r"(arg0 + 0x354), applied, scored: tmp/grind/func_800203B4/s3/varD_ptr_expr.c.
- result: score 0, build_insns 65, target_insns 65 - codegen-neutral. Both spellings are ordinary C with no family claim; candidate.c keeps the `+=` form because it mirrors the SDK call shape.
- verdict: KILLED

## s4 (2026-09-01, permuter)

## [s4] H12: an automated randomized search over the PURE-C chassis (all four cop2 islands deleted, base score 26) can find a semantically-valid C spelling that scores BELOW 26 - i.e. some C statement form emits one or more of the 26 residual instructions (the non-cop2 members of the islands: 4x `move $12,rN` (addu), 5x lw, 2x lhu, sll, or - 13 of the 26 are ordinary integer instructions that C *can* in principle emit).
- mechanism: The 26-instruction residual is not homogeneous. Only 11 of them are cop2 (5x ctc2, mtc2, lwc2, 3x swc2, the MVMVA .word); the other ~13-15 are the SDK macros' addressing preamble (register copy of the operand, the packed lhu/sll/or of VX0/VY0, the five matrix lw's) plus two load-delay slots. Those are ordinary MIPS integer instructions. If GCC 2.7.2 could be steered into emitting them - reading mat[0..4], packing vec[0]/vec[1] into a word, copying an array address into $12 - from ordinary C, the pure-C bound would be lower than 26 and the un-authorizable residual would shrink toward the 11 genuinely cop2-only instructions. A hand search cannot cover that space; a randomized permuter search over ~35k iterations can.
- probe: Built a purpose-made single-function permuter workspace (tmp/grind/func_800203B4/s4/perm) - target.o assembled from asm/funcs/func_800203B4.s; base.c = the cpp-preprocessed FULL code6cac.c TU (correct codegen context) with the islands-deleted body substituted and a `typedef struct GameObj GameObj;` prepended purely so pycparser can parse the TU (measured codegen-neutral: the extracted func_800203B4 region is byte-identical with and without it); compile.sh runs the exact engine pipeline (cc1 -mel | prologue_fix | maspsx | align sed | multu_pad) and extracts ONLY the .globl..`.end func_800203B4` region before assembling, so the permuter scores the function and not the whole TU. Campaign via tools/permuter_campaign.py launch -j 8, label purec-no-islands-fnonly.
- result: base score 3000 (asm-differ weighted; = the 26-instruction deficit plus alignment penalties). Over ~35k+ iterations the search produced only three novel outputs, best 2960 - a 40-point (sub-one-instruction) move, and INVALID: output-2960-1 hoists `arg0 += 0x354;` to the top of the body, which changes every subsequent `arg0 + 0xNNN` offset (semantics-breaking, as permuter randomizations are permitted to be). No find approached, let alone crossed, the 2600-equivalent that a single genuinely-recovered instruction would require. The randomized search cannot invent the statements (array reads feeding nothing, a packed lhu/sll/or of two array elements) that the preamble instructions would need - and any such statement would be dead code that GCC's DCE deletes, which is exactly why the residual is unreachable rather than merely unfound.
- verdict: KILLED

## [s4] An automated randomized search over the PURE-C chassis (all four cop2 islands deleted, sandbox score 26) can find a semantically-valid C spelling scoring below 26 - i.e. some C statement form emits one or more of the ~13-15 ORDINARY INTEGER instructions inside the 26-instruction residual (the SDK macros' addressing preamble: 4x 'move $12,rN' (addu), 5x matrix lw, the lhu/sll/or packing of VX0/VY0), shrinking the un-authorizable residual toward the 11 genuinely cop2-only instructions.
- mechanism: The residual is not homogeneous: only 11 of the 26 are cop2 (5x ctc2, mtc2, lwc2, 3x swc2, the MVMVA .word 0x4A486012). The rest are ordinary MIPS integer instructions that C can in principle express. A hand search cannot cover that spelling space; a randomized permuter search over tens of thousands of iterations can. If a valid basin existed it would show as a find at or below 2600 on asm-differ's weighted metric (one recovered instruction = 100 points off the 3000 base).
- probe: Built a purpose-made SINGLE-FUNCTION permuter workspace at tmp/grind/func_800203B4/s4/perm: target.o assembled from asm/funcs/func_800203B4.s (macro.inc wrapper); base.c = the cpp-preprocessed FULL code6cac.c TU (correct codegen context) with the islands-deleted body substituted, plus a prepended 'typedef struct GameObj GameObj;' needed only so pycparser parses the TU (verified codegen-neutral by byte-comparing the extracted function region with and without it); compile.sh runs the exact engine pipeline (cc1 -mel | prologue_fix | maspsx | align sed | multu_pad) and assembles ONLY the '.globl func_800203B4' .. '.end func_800203B4' region, so the scorer diffs the function and not the whole TU. Campaign launched via tools/permuter_campaign.py launch -j 8 (label purec-no-islands-fnonly), waited in-turn across five ~9-minute windows, harvested with --stop.
- result: base_score 3000; 67,817 iterations in 1,723 s; FOUR novel finds (2960, 2980, 2978, 2935). Best = 2935, i.e. 65 points off base - below the 100-point cost of a single recovered instruction, so no find recovered ANY residual instruction. All four are semantically invalid: output-2960-1 hoists 'arg0 += 0x354;' above the 0x350/0x352 stores (shifting every later offset); output-2935-1 re-assigns 'src' from 'new_var' after that block scope has closed and then dereferences it. Campaign stopped, 9 worker procs killed, 0 live campaigns at session end.
- verdict: KILLED

## [s4] The residual's ordinary-integer instructions are unreachable by C in principle, not merely unfound by this search.
- mechanism: The preamble instructions are only C-expressible as computations whose results are never consumed - reads of mat[0..4] and a word packed from vec[0]/vec[1] whose real consumers are cop2 registers. GCC 2.7.2's DCE deletes exactly that, and any source form written to survive DCE would be a dead-read / constant-holder coercion aimed at materializing bytes: a cheat, not a match. So no amount of search effort changes the bound.
- probe: Structural reasoning cross-checked against the campaign result (no find within 350 points of the one-instruction threshold despite 67,817 samples) and against the s3 measurement that the islands-deleted body scores EXACTLY the 65-39=26 deficit, i.e. all 39 C-emitted instructions already match.
- result: Consistent: the search behaves like one with no valid basin (finds cluster tightly just under base and are all semantics-breaking), which is exactly what an unreachable residual predicts.
- verdict: CONFIRMED

## [s4] The dispatch chassis-check read 'measurement unavailable', so both banked floors must be re-measured before being spent.
- mechanism: Ledger conclusions are chassis-relative and the driver's own measurement was missing this dispatch.
- probe: Applied memory/grind/func_800203B4/candidate.c to src/code6cac.c:1813 and ran 'sandbox func_800203B4 --disable all'; then applied the islands-deleted body (rejected/pure-c-no-islands-floor-26.c) and ran it again. src reverted after each run.
- result: candidate -> score 0, build_insns 65 == target_insns 65, rules_dropped 0, cheat_asm_stripped 25 (artifact tmp/grind/func_800203B4/s4/code6cac_sandbox0_s4.o). Islands-deleted -> score 26, build_insns 39. Eighth and ninth independent confirmations; both banked numbers hold on this chassis.
- verdict: CONFIRMED

## s6 (2026-09-01, SYNTHESIS - the merged attack)

### The merged picture (what five sessions add up to)

func_800203B4 is a 65-instruction PsyQ-SDK GTE wrapper: set the rotation matrix from a
caller-supplied 3x3 (gte_SetRotMatrix), load a short vector (gte_ldv0), run one MVMVA
(sf=1, rot . V0), store the result long-vector (gte_stlvnl) into `arg0 + 0x354`, around a
pure-C head that stamps 0x350/0x352, indexes D_8008D59E by `arg1 * 20`, and calls
game_GetPlayerData -> table -> func_8002EECC(src, mat).

Five sessions have MEASURED, not argued, the following partition of its 65 instructions:

- **39 instructions are emitted by C and are ALREADY EXACT.** Verified this session by
  disassembling the islands-deleted build and comparing instruction by instruction against
  asm/funcs/func_800203B4.s - not by the earlier score-arithmetic inference (evidence facts
  33, 36).
- **3 instructions are ordinary C that only appear when an island consumes them**
  (`&mat[0]`, `&vec[0]`, `arg0 += 0x354`) - free, not residual (fact 34).
- **25 instructions cannot come from C at all**: 11 cop2 opcodes (5x ctc2, mtc2, lwc2,
  3x swc2, the MVMVA word), 12 ordinary-integer instructions that live INSIDE the SDK macro
  bodies and are C-expressible only as DCE-deleted dead code, and 2 assembler-supplied cop2
  load-delay slots (facts 29, 34).

Everything above the C/asm line is closed: declaration order and statement order are
load-bearing and pinned (H7, H10); the named intermediate, the explicit delay nops and the
`arg0 += 0x354` spelling are all codegen-neutral and the candidate has been simplified to the
minimal surface (H6, H8, H11); the structural bound is a measured minimum that only rises
under perturbation (H5, H9); and 67,817 permuter iterations over the pure-C chassis produced
four semantically-invalid finds, none within 350 points of recovering a single instruction
(H12). The C body is byte-final: sandbox --disable all = 0, 65/65, now confirmed TEN times
across five sessions and every chassis re-check.

**So the function's residual is an AUTHORIZATION question, not a codegen question**, and both
authorization gates are measured dead: scan_hand_coded is LOW (score 1, s4 signal only - H3),
and the 2026-08-17 FDB0 cluster grant was ruled NOT to reach this function by Judge FAIL
(docs/grind/decisions.md:17546) because its idiom sites copy from $v0/$s0 rather than $aN.
The proof-of-foreclosure record is already filed at docs/grind/decisions.md:17550.

### The reset frontier (strongest three, for the next ladder pass)

- **F1 (terminal, modality-gated) - emit the standing-ruling disposition from an `escalation`
  session.** Mechanism: the driver validator accepts `owner-gated` only from a
  driver-dispatched `escalation` modality (s2's owner-gated from `recon` was discarded, marker
  decisions.md:17591). Everything the disposition needs already exists: the Judge FAIL
  (17546), the foreclosure record (17550), the byte-final candidate, and now the exact 25-insn
  residual anatomy (fact 34). Next probe: one confirming sandbox --disable all run on
  candidate.c, the three-way trigger presence check of fact 38, then `owner-gated` with
  escalation_ref docs/grind/decisions.md:17550. Do not re-file the record, do not re-argue
  membership in any spelling.
- **F2 - the only re-activation trigger worth a per-session check is a class grant covering
  non-$aN-source cop2 addressing-preamble sites.** Mechanism: such a grant would admit this
  function's three `addu $t4,$v0/$s0,$zero` sites into the sanctioned family, at which point
  the function is a PURE INTEGRATION HANDOFF - candidate.c applies unchanged, the driver writes
  the inline_asm_canonical.txt line and the docs/grind/borderline.md entry, then layer-2
  cheat-reviewer -> verify-oracle --rebuild -> queue done. Next probe (cheap, ~1 turn): grep
  inline_asm_canonical.txt for func_800203B4, grep
  .claude/rules/cop2-addressing-preamble-cluster.md for a widened scan/membership, tail
  docs/grind/decisions.md for an owner class grant. Checked 2026-09-01: none present (fact 38).
- **F3 - if the driver dispatches solver / forensics / rederive anyway, those axes are
  foreclosed A PRIORI by fact 33 and must not be spent searching.** Mechanism: ra_solver and
  sched_solver reason about register seats and emission order AMONG THE INSTRUCTIONS THE
  COMPILER ACTUALLY EMITS; forensic cc1 dumps attribute a divergence to a pass. This function
  has **zero divergent C-emitted instructions** - all 39 match exactly - so there is no seat to
  re-assign, no order to re-schedule and no pass to attribute. The gap is expressiveness (no
  cop2 intrinsic in GCC 2.7.2), which no solver models. Next probe for such a session: one
  confirming pair of runs (candidate -> 0/65; islands-deleted -> 26/39), re-state facts 33-34,
  return `progress` in under 5 turns. Do NOT re-launch a permuter campaign (H12/facts 28/32),
  do NOT reorder locals or statements (H7/H10), do NOT re-probe the nops, the named
  intermediate or the `+=` spelling (H6/H8/H11).

### Hypotheses measured this session

- **H13 - CONFIRMED (measured by disassembly, upgrading an inference):** "Every one of the 39
  instructions the pure-C chassis emits is already identical to its target counterpart, so the
  residual is exclusively instructions C cannot emit."
  - mechanism: If any C-emitted instruction diverged, the islands-deleted build would show a
    mnemonic/operand difference against asm/funcs/func_800203B4.s at that address, and a
    codegen lever (RA seat, schedule slot, expression shape) would exist to attack. The earlier
    sessions inferred "no divergence" from `score == target_insns - build_insns`, which is only
    valid if the scorer's counts are raw - and they are not (fact 36).
  - probe: Applied rejected/pure-c-no-islands-floor-26.c -> sandbox --disable all (26,
    build_insns 39) -> mipsel-linux-gnu-objdump -d on the resulting
    tmp/grind/func_800203B4/s5/code6cac_purec26_s5.o, region 00003564 func_800203B4, and
    compared all 39 instructions line-by-line against asm/funcs/func_800203B4.s.
  - result: All 39 match exactly (17 + 7 + 9 + 6 as itemized in evidence fact 33); the 28
    absent target instructions decompose into 11 cop2 opcodes + 12 in-island integer insns +
    2 assembler-supplied delay nops + 3 operand-address materializations that are ordinary C
    and return for free with the islands. Un-authorizable-by-C set = **25 instructions**.
  - verdict: CONFIRMED

- **H14 - KILLED (measured):** "cheat_asm_stripped in the sandbox JSON reports the number of
  inline-asm instructions stripped from the function under test, so it can be used as a check
  that the islands were really applied/removed."
  - mechanism: The field name and its stability at 25 across sessions suggested a per-function
    island count (25 == exactly this function's four islands).
  - probe: Ran sandbox func_800203B4 --disable all back-to-back on the island-bearing
    candidate and on the islands-deleted body in the same session.
  - result: **25 in BOTH cases**, while build_insns went 65 -> 39. It is not a per-function
    counter. Use build_insns (65 vs 39) as the apply/removal check, per the fact-27 tooling
    note.
  - verdict: KILLED

- **H15 - KILLED (checked, no measurement needed):** "A re-activation trigger has landed since
  s5 (a widened cluster enumeration, a grant line, or an owner class grant)."
  - mechanism: The foreclosure record's own re-activation triggers; a landed trigger would
    convert the function into an integration handoff immediately.
  - probe: grep func_800203B4 in inline_asm_canonical.txt; grep membership/scan definition in
    .claude/rules/cop2-addressing-preamble-cluster.md; presence check only, no membership
    argument (Judge constraint).
  - result: No grant line; the cluster file still enumerates the same 28 `addu $t4,$aN,$zero`
    members without this function; no class grant filed.
  - verdict: KILLED (for now - re-check is F2, ~1 turn)

Frontier: F1/F2/F3 above. The C body is final; no session should spend measurements on
spelling again.

## [s5] Every one of the 39 instructions the pure-C chassis emits is already identical to its target counterpart, so the residual is exclusively instructions C cannot emit (H13 - upgrading s4/s5's score-arithmetic inference to a direct instruction-level comparison).
- mechanism: If any C-emitted instruction diverged, the islands-deleted build would show a mnemonic/operand difference against asm/funcs/func_800203B4.s at that address and a codegen lever (RA seat, schedule slot, expression shape) would exist to attack. The prior sessions inferred 'no divergence' from score == target_insns - build_insns, which is only sound if the scorer's counts are raw instruction counts - and they are not (the .s carries 67 instruction words while the sandbox reports target_insns 65).
- probe: Applied memory/grind/func_800203B4/rejected/pure-c-no-islands-floor-26.c to src/code6cac.c, ran `sandbox func_800203B4 --disable all` (26, build_insns 39), then disassembled the resulting object (mipsel-linux-gnu-objdump -d tmp/grind/func_800203B4/s5/code6cac_purec26_s5.o, region 00003564 <func_800203B4>) and compared all 39 instructions line-by-line against asm/funcs/func_800203B4.s.
- result: All 39 match exactly: 17 prologue/lookup insns (800203B4-800203F4), 7 insns through the func_8002EECC call (800203F8-80020410), the 9-insn vec[] store block (80020444-80020464), and the 6-insn epilogue (800204A8-800204BC); `move s0,a0`/`move s1,a2` are the assembler spelling of the target's `addu $s0,$a0,$zero`/`addu $s1,$a2,$zero`. The 28 absent target instructions decompose into 11 cop2 opcodes, 12 ordinary-integer instructions inside the SDK macro bodies, 2 assembler-supplied cop2 load-delay nops, and 3 operand-address materializations (addiu $v0,$sp,0x10 / addiu $v0,$sp,0x30 / addiu $s0,$s0,0x354) that are ordinary C and reappear for free once an island consumes them.
- verdict: CONFIRMED

## [s5] cheat_asm_stripped in the sandbox JSON reports the number of inline-asm instructions stripped from the function under test, so it can be used as a check that the islands were really applied or removed (H14).
- mechanism: The field name plus its stability at exactly 25 across five sessions - which equals this function's four islands' instruction count - made it look like a per-function island counter, and facts 13/16/20/25/30 quote it as corroboration.
- probe: Ran `sandbox func_800203B4 --disable all` back-to-back in this session on the island-bearing candidate.c and on the islands-deleted body.
- result: cheat_asm_stripped read 25 in BOTH cases while build_insns went 65 -> 39. It is not a per-function counter and carries no information about the function under test; only score / build_insns / target_insns / rules_dropped do.
- verdict: KILLED

## [s5] A re-activation trigger has landed since s5 - a widened cluster enumeration, a grant line in inline_asm_canonical.txt, or an owner class grant covering non-$aN-source cop2 addressing-preamble sites (H15).
- mechanism: The filed foreclosure record (docs/grind/decisions.md:17550) names exactly these triggers; a landed trigger converts the function from foreclosed into a pure integration handoff, since candidate.c is already byte-final at floor 0.
- probe: Presence check only (no membership argument, per the binding Judge constraint): grep func_800203B4 in inline_asm_canonical.txt; grep the membership table and scan definition in .claude/rules/cop2-addressing-preamble-cluster.md.
- result: No grant line for func_800203B4 in inline_asm_canonical.txt; the cluster rule still enumerates the same 28 members derived by the `addu $t4, $aN, $zero` scan and still omits this function; no owner class grant filed.
- verdict: KILLED

## s6 (2026-09-01, SYNTHESIS - island-partition minimality)

- **H12 — KILLED (measured x3, new this session):** "The asm authorization surface can be
  shrunk below 25 instructions by moving the C-expressible parts of the PsyQ SDK macro bodies
  (fact 34 tier b: the `move $12,rN` addressing preambles, the five matrix `lw`s, the
  `lhu/lhu/sll/or` VX0/VY0 packing) out of the islands into C, leaving smaller cop2-only
  islands." Mechanism it assumed: those 12 instructions are ordinary integer code, so a C
  spelling that CONSUMES the values (feeding them to asm operands rather than dropping them,
  which is what makes fact 29's DCE argument bite) should survive and emit the same bytes.
  Probes (each applied to src/code6cac.c, `sandbox func_800203B4 --disable all`, src reverted
  after):
  - varH thin gte_SetRotMatrix -> **12 / build 63** (rejected/thin-setrotmatrix-c-loads-score12.c)
  - varI thin gte_stlvnl, no `move $12` preamble -> **4 / build 64** (rejected/thin-stlvnl-no-preamble-score4.c)
  - varJ thin gte_ldv0, packing in C -> **8 / build 64** (rejected/thin-ldv0-c-packing-score8.c)
  Result: GCC 2.7.2 chooses its own registers and its own emission order for the extracted
  computations, and the SDK macro bodies' fixed $12-$15 sequence and lw/ctc2 interleave are
  not reproducible from separated C. The residual is byte-forced INSIDE the asm, so 25 is the
  minimum authorizable surface for this function. This retires the last complementary
  question the ledger had never asked (s1-s5 all treated the islands as atomic).
- **H1 re-CONFIRMED (eleventh measurement):** candidate.c -> 0, 65/65, rules_dropped 0
  (tmp/grind/func_800203B4/s6/code6cac_sandbox0_s6.o).
- **Trigger presence check (H13, standing):** none of the three re-activation triggers has
  landed (evidence fact 43).

### Merged attack after s6 (the complete picture in one paragraph)
The function is 65 target instructions. 39 of them are emitted by the pure-C body and are
byte-exact against asm/funcs/func_800203B4.s, verified by disassembly (fact 33); 3 more are
operand-address materializations that reappear for free once an island consumes the address
(fact 34d); the remaining 25 are byte-forced to live inside inline asm — 11 are cop2 opcodes
GCC 2.7.2 cannot emit at all, 12 are ordinary-integer SDK-macro-body instructions that are
either DCE-deleted when written as C dead code (fact 29) or emitted in the wrong
registers/order when written as C live code feeding asm operands (fact 41, NEW), and 2 are
assembler-supplied cop2 load-delay nops that cost nothing (fact 21). Therefore the codegen
question is closed in both directions: the C part cannot be improved (it is exact) and the asm
part cannot be reduced (it is minimal). What remains is purely an AUTHORIZATION question, and
both authorization gates are measured shut — scan_hand_coded LOW (fact 10, re-run by the Judge
in fact 12) and cluster membership denied by Judge FAIL (decisions.md:17546) because this
function's three idiom sites copy from `$v0`/`$s0` rather than `$aN`. The proof-of-foreclosure
record is already filed at decisions.md:17550; the terminal disposition is modality-gated to a
driver-dispatched `escalation` session (decisions.md:17591).

Frontier reset (strongest 1-3, in order):
  F1 (terminal, modality-gated): emit `owner-gated` citing decisions.md:17550 — valid ONLY from
     an `escalation` session. It is a cite-and-return; every supporting fact is on disk
     (Judge FAIL 17546, foreclosure record 17550, residual anatomy fact 34, minimality fact 41,
     eleven floor confirmations). Do not re-file the record; do not re-argue cluster membership.
  F2 (standing, one turn): the three-grep trigger check (fact 43). If an owner class grant for
     non-$aN-source cop2 addressing-preamble sites lands, this function is a PURE INTEGRATION
     HANDOFF: apply candidate.c unchanged, confirm sandbox 0, hand off per evidence fact 11.
  F3 (a priori foreclosed): solver / forensics / rederive. There is no divergent C-emitted
     instruction to re-seat, re-order or attribute to a pass (fact 33), and the asm surface is
     minimal (fact 41). If dispatched, confirm candidate.c -> 0 and islands-deleted -> 26,
     re-state facts 33/34/41, return `progress` inside five turns. Do NOT re-run the permuter
     (facts 28/32), re-order locals or statements (H7/H10), re-probe the delay nops, the named
     intermediate or the `arg0 += 0x354` spelling (H6/H8/H11), or re-partition the islands (H12).

## [s6] H12: the asm authorization surface can be shrunk below 25 instructions by moving the C-expressible parts of the PsyQ SDK macro bodies (the 'move $12,rN' addressing preambles, the five matrix lw's, the lhu/lhu/sll/or VX0/VY0 packing) out of the islands into C, leaving smaller cop2-only islands.
- mechanism: Those 12 instructions are ordinary integer code. Fact 29's DCE argument only bites when they are written as C computations whose results are never consumed; written as C values that FEED asm operands they are live, so they should survive and could in principle emit the same bytes - which would shrink the surface any future authorization axis has to cover.
- probe: Three variants spelled and applied to src/code6cac.c, each measured with 'sandbox func_800203B4 --disable all' and reverted after: varH thin gte_SetRotMatrix (s32 r0..r4 = mat[0..4] in C, asm reduced to five ctc2 %n,$n); varI thin gte_stlvnl (swc2 $25/$26/$27 addressing the operand register directly, no 'move $12, %0'); varJ thin gte_ldv0 (u32 pack = ((u32)*(u16*)&vec[1] << 16) | *(u16*)&vec[0] in C, only mtc2/lwc2 in asm).
- result: varH = 12, build_insns 63; varI = 4, build_insns 64; varJ = 8, build_insns 64 (all rules_dropped 0, target_insns 65). Objects tmp/grind/func_800203B4/s6/varH_thin_setrotmatrix.o, varI_thin_stlvnl.o, varJ_thin_ldv0.o; forms banked as rejected/thin-setrotmatrix-c-loads-score12.c, rejected/thin-stlvnl-no-preamble-score4.c, rejected/thin-ldv0-c-packing-score8.c. GCC 2.7.2 picks its own registers and emission order for the extracted computations, and the SDK macro bodies' fixed $12-$15 sequence and lw/ctc2 interleave are not reproducible from separated C; dropping the stlvnl preamble additionally re-bases all three swc2 (4 mismatches for one removed instruction).
- verdict: KILLED

## [s6] H1 (standing): the candidate body (pure-C head + four literal PsyQ SDK gte_* macro islands) reproduces all 65 target instructions on the current chassis.
- mechanism: $t4..$t7 ARE $12..$15, so the SDK macro's 'move $12,%0' emits the target's 'addu $t4,rN,$zero'; GCC materializes the operand addresses exactly as the target does.
- probe: candidate.c applied to src/code6cac.c -> 'sandbox func_800203B4 --disable all' (run twice this session: before and after the header-comment update).
- result: 0, build_insns 65 == target_insns 65, rules_dropped 0 both times. Artifact tmp/grind/func_800203B4/s6/code6cac_sandbox0_s6.o. Eleventh independent confirmation; this dispatch's chassis-check again read 'measurement unavailable'.
- verdict: CONFIRMED

## [s6] H13 (standing): one of the three re-activation triggers has landed since s5.
- mechanism: An owner class grant covering non-$aN-source cop2 addressing-preamble sites, an updated cluster enumeration, or a grant line in inline_asm_canonical.txt would make this function a pure integration handoff with candidate.c unchanged.
- probe: Presence check only (no membership argument re-derived, per the binding Judge constraint): grep func_800203B4 in inline_asm_canonical.txt; grep func_800203B4 in .claude/rules/cop2-addressing-preamble-cluster.md; tail docs/grind/decisions.md.
- result: No match in inline_asm_canonical.txt; zero occurrences in the cluster enumeration (unchanged); decisions.md tail is still the 2026-09-01 09:05 discarded-session marker. No trigger has landed.
- verdict: KILLED

## [s7 / solver] H13 - KILLED (typed verdict, not inference)
**Statement:** the residual on this function is a register-allocation seat assignment or a
scheduler emission-order tie that tools/ra_solver or tools/sched_solver can type as REACHABLE
and convert into a ranked C-lever vector.
**Mechanism tested:** the solver stack's own triage. `inverse_compose.py classify` guards
itself off on zero-rule functions (it would report a fictitious PRE-RA verdict from an absent
src-derived tgt stream) and redirects to `goal_from_tgt.py classify`, which compares the two
OBJECT streams directly.
**Probe:** object-level classify on both chassis. (a) candidate.c applied: ours 65 / target 65,
**NO DIVERGENCE - the two streams are identical**. (b) the pure-C islands-deleted chassis
(rejected/pure-c-no-islands-floor-26.c, sandbox 26, build_insns 39): ours 39 / target 65,
**FIRST DIVERGENCE: PRE-RA**, with the tool's own next-step field reading
*"none - the residual is upstream of every model"*. Every one-stream-only shape is target-only
and belongs to an island (3x `move #,#`, five matrix `lw`, `lhu`, `addiu #,#,16`, `ctc2 $0..$3`).
**Verdict: KILLED.** A PRE-RA classification means the two streams differ in the instruction
MULTISET, which is built by the front end + cse/combine/loop. ra_solver permutes register
assignments over a FIXED multiset and sched_solver reorders a FIXED, already-allocated stream;
neither model can add or remove an instruction, so both are searching a space that provably
cannot contain the answer. No vector search was run - running one would have produced fiction
(the exact func_80072CD4 defect the classifier exists to prevent). Do not re-open the solver
axis on this function under any spelling: the only thing that could change the verdict is a
change to the instruction multiset, which is the expressiveness question (cop2 opcodes GCC
2.7.2 cannot emit) that facts 33/34/41 already measured shut in both directions.

## [s7] The residual on func_800203B4 is a register-allocation seat assignment or a scheduler emission-order tie that tools/ra_solver or tools/sched_solver can type as REACHABLE and convert into a ranked C-lever vector.
- mechanism: The solver stack's mandated triage. inverse_compose.py classify guards itself off on zero-rule functions (since rules-to-zero 2026-08-25 the src-derived code6cac.tgt.s cannot carry target's stream, so the text classifier would emit a FICTITIOUS PRE-RA verdict) and redirects to the object-level classifier tools/ra_solver/goal_from_tgt.py classify, which compares the two object streams directly.
- probe: goal_from_tgt.py classify code6cac func_800203B4 on BOTH chassis. (a) candidate.c applied (sandbox 0, 65/65): ours 65 insns / target 65 insns, 'NO DIVERGENCE: the two streams are identical' (tmp/grind/func_800203B4/s7/classify_candidate.txt). (b) the islands-deleted pure-C chassis rejected/pure-c-no-islands-floor-26.c (sandbox 26, build_insns 39): ours 39 / target 65, 'FIRST DIVERGENCE: PRE-RA', tool's own next-step field 'none - the residual is upstream of every model'; every one-stream-only shape is target-only and belongs to an island (3x move #,#, five matrix lw, lhu #,0(#), addiu #,#,16, ctc2 #,$0..$3) (tmp/grind/func_800203B4/s7/classify_purec.txt, object code6cac_purec39_s7.o).
- result: PRE-RA on the only chassis that has a residual at all; identical streams on the candidate chassis. A PRE-RA classification is an instruction-MULTISET difference, built by the front end + cse/combine/loop. ra_solver permutes register assignments over a FIXED multiset; sched_solver reorders a FIXED, already-allocated stream. Neither model can add or remove an instruction, so both would be searching a space that provably cannot contain the answer. No vector search was run - running one would have produced fiction (the func_80072CD4 baseline-routing defect this classifier exists to prevent).
- verdict: KILLED

## [s7] The candidate body is byte-final on the current chassis (floor 0) and the chassis has not drifted since s6.
- mechanism: Direct measurement, not inference - the dispatch chassis-check again read 'measurement unavailable', so the ledger floor had to be re-measured before anything could be spent on it.
- probe: memory/grind/func_800203B4/candidate.c applied to src/code6cac.c, `sandbox func_800203B4 --disable all`, run twice this session (once before and once after the header-comment update).
- result: 0, build_insns 65 == target_insns 65, rules_dropped 0, cheat_asm_stripped 25 - both times. Twelfth and thirteenth confirmation overall. Artifact tmp/grind/func_800203B4/s7/code6cac_sandbox0_s7.o. src reverted to INCLUDE_ASM after every build per asm-until-matched.
- verdict: CONFIRMED

## H14 [s8, forensics] — CONFIRMED
**Statement.** The 11 cop2 instructions in this function's residual are not "an unfound C
spelling" but a hard expressiveness limit of GCC 2.7.2's MIPS backend, provable from the
compiler's own machine description rather than from search.
**Mechanism.** An insn can only be emitted if some pattern in `config/mips/mips.md` produces the
mnemonic and some RTL register object can name the operand. Both are absent for coprocessor 2.
**Probe.** Grep the toolchain sources at tools/gcc-2.7.2/config/mips/ for every cop2 mnemonic;
contrast with cop1; read FIRST_PSEUDO_REGISTER, `mips_reg_names`, and `enum reg_class`.
**Result.** 0 hits for `ctc2|mtc2|cfc2|mfc2|lwc2|swc2|cop2` in mips.md, mips.c AND mips.h; 4 hits
for the cop1 equivalents in mips.md. FIRST_PSEUDO_REGISTER 68 (mips.h:1181) = 32 GPR + 32 FPR +
hi + lo + accum + $fcr31 (mips.c:240-251); `enum reg_class` (mips.h:1377) has no COP2 class.
**Verdict.** CONFIRMED. Evidence fact 50. The cop2 half of the residual is closed permanently -
no future session should search for a C form of it.

## H15 [s8, forensics] — CONFIRMED
**Statement.** The 12 ordinary-integer SDK-macro-body instructions cannot be written in C because
their target register seats ($12-$15) are unreachable by GCC 2.7.2's allocator at this function's
register pressure — a structural allocation-order fact, not a spelling problem.
**Mechanism.** mips.h defines no REG_ALLOC_ORDER, so local-alloc's `find_free_reg`
(local-alloc.c:2249-2273) and global-alloc's `find_reg` (global.c:1058-1082) both take the
`#else int regno = i;` branch: ascending regno scan, first free register wins. With $2 the first
allocatable GPR, a pseudo reaches $13/$14 only when $2..$12 are all simultaneously live.
**Probe.** Apply the s6 varJ (thin gte_ldv0) form — the one partition where the C-expressible work
is genuinely ordinary C — run `pwsh tools/grinder/dump.ps1 func_800203B4`, and read the .lreg/.greg
register dispositions plus the emitted .s.
**Result.** `.lreg` seats all 19 pseudos in $2 or $3 except the two call-crossing pointers ($16/$17,
the first free callee-saved regs); `.greg` dispositions are identical, so global-alloc revises
nothing; $12-$15 appear in "Hard regs used" solely as asm clobbers. The emitted body uses $2/$3
where the target uses $13/$14, and folds vec[] to $sp displacements instead of basing off one
pointer register. sandbox re-measured 8 / build 64, matching fact 41.
**Verdict.** CONFIRMED. Evidence facts 51-52. Note the corollary in fact 52: the ORDER matches the
target exactly, so this was never a scheduling divergence — consistent with s7's PRE-RA verdict.
**Explicitly NOT a lever:** raising register pressure to push a pseudo into $13/$14 would require
inventing live values with no semantic purpose — a register-pin by another spelling, forbidden by
the cheat checklist (T1/T2/T6). It is named here so no future session mistakes it for an idea.

## [s8] The 11 cop2 instructions in the residual are a hard expressiveness limit of GCC 2.7.2's MIPS backend, provable from the compiler's own machine description rather than from search.
- mechanism: An insn is emittable only if some mips.md pattern produces the mnemonic and some RTL register object can name the operand; both are absent for coprocessor 2, so no C expression can ever expand to one.
- probe: grep -cE 'ctc2|mtc2|cfc2|mfc2|lwc2|swc2|cop2' over tools/gcc-2.7.2/config/mips/{mips.md,mips.c,mips.h}; contrast with the cop1 mnemonics; read FIRST_PSEUDO_REGISTER, mips_reg_names and enum reg_class.
- result: 0 hits in all three files (cop1 equivalents mtc1|ctc1|lwc1|swc1: 4 hits in mips.md, so the backend does support coprocessor 1 and the zero is a real absence). FIRST_PSEUDO_REGISTER 68 (mips.h:1181); mips_reg_names (mips.c:240-251) = $0-$31, $f0-$f31, hi, lo, accum, $fcr31; enum reg_class (mips.h:1377-1389) = NO_REGS/GR_REGS/FP_REGS/HI_REG/LO_REG/HILO_REG/MD_REGS/ST_REGS/ALL_REGS with no COP2 class.
- verdict: CONFIRMED

## [s8] The 12 ordinary-integer SDK-macro-body instructions cannot be written in C because their target register seats ($12-$15) are unreachable by GCC 2.7.2's allocator at this function's register pressure.
- mechanism: mips.h defines no REG_ALLOC_ORDER, so local-alloc's find_free_reg (local-alloc.c:2249-2273) and global-alloc's find_reg (global.c:1058-1082) both take the '#else int regno = i;' branch - ascending regno scan, first free register wins. $2 is the first allocatable GPR under FIXED_REGISTERS, so a pseudo reaches $13/$14 only when $2..$12 are simultaneously live.
- probe: Apply the s6 varJ thin-gte_ldv0 form (the one partition whose C-expressible work is genuinely ordinary C), run 'pwsh tools/grinder/dump.ps1 func_800203B4', read the .lreg/.greg register dispositions and the emitted .s, and re-measure the sandbox score.
- result: '.lreg' seats all 19 pseudos in $2 or $3 except the two call-crossing pointers ($16/$17, the first free callee-saved regs); '.greg' dispositions are identical so global-alloc revises nothing; $12-$15 appear in 'Hard regs used' only as asm clobbers. The emitted body uses $2/$3 where the target uses $13/$14 and folds vec[] to $sp displacements instead of basing off one pointer register. sandbox re-measured 8 / build_insns 64, matching fact 41.
- verdict: CONFIRMED
