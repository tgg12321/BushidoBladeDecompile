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
