# Hypothesis ledger — func_800324D0

## [s5] 2026-08-20 (permuter, brief-session 4; scratch tmp/grind/func_800324D0/s4/)

14. **H14 — a second campaign from a genuinely different C text at floor 27
    (the combined s2/s3 flat respellings) finds an honest closure the first
    basin could not.** Probe: campaign tmp/perm_324d0_s4, ~12k iters, vetted
    every sub-120 find. Result: zero score-0; 110/115 finds are forbidden
    families (invented locals, empty-if dead-read, if(1){} wrap); 120 is the
    banned invented-carrier direction again. BUT output-105-1 is a
    semantics-preserving staged tail read that hand-measures 15 (H15).
    **KILLED as a closer; CONFIRMED as a lever source. R3 now 2/2 — permuter
    exhausted.**
15. **H15 — staging the loop-tail read through the existing dead u32 cmd
    (`cmd = *ptr; c = cmd;`) re-seeds the find_reg census toward the target
    rotation.** Probe: sandbox + BB2_FINDREG_DEBUG + objdump side-by-side.
    Result: **27 -> 15, 68/68** — val/byte/holders all land in TARGET regs;
    residual is the exact walker<->cmd 2-swap ($6<->$3). Construct =
    staged-value-reused-variable family (bounds walked in evidence.md [s5];
    FAKE-annotated; SOTN precedent sotn-construct-index.md:51 et al.).
    **CONFIRMED (floor lever, not yet a closer).**
16. **H16 — micro-variants of the staging close the remaining swap.** Probe:
    u8 carrier (27), placement after ptr++ (28/69), preheader staged (flat),
    arm-load staged via c (flat), decl order (flat), do-while (flat),
    0xFF-arm pointer-through-cmd borrow (flat; copy cse-coalesced).
    **KILLED — the swap does not move by any measured staging variant.**

## Frontier (for s6+)
1. **The residual-15 wall is s1's wall in a new coat: walker must get $3.**
   find_reg ground truth on the 15-chassis (evidence.md [s5]): all three of
   75/76/85 must skip $3 -> needs walker-pref-$3 (s2 def-analysis proof of
   impossibility carries over) or walker-first allocation order (s1
   arithmetic kill carries over) or an invented conflicting pref-carrier
   (BANNED family). Mechanism: global.c find_reg pass 0. Next probe: a
   rederive/synthesis modality searching for a statement geometry where the
   cmd head web's live length grows (10 refs across >=10 insns drops 75
   below val AND walker... note: order change alone measured insufficient —
   re-derive the full cascade for any new census before spending a build).
2. **The submitting session must resolve the staged-value citation question**
   (rule origin names sched.c; our mechanism is global-RA census; bounds are
   mechanism-silent). If irresolvable from the rule text + precedent index,
   emit ruling-request BEFORE any candidate-ready carrying the construct.
3. **Permuter is exhausted (R3 2/2).** Do not propose further campaigns.

## [s4] 2026-08-20 (structural — permuter; scratch tmp/grind/func_800324D0/s3/)

13. **H13 — the permuter finds a structural spelling outside the
    hand-enumerated space that closes the rotation honestly** (frontier 1).
    Probe: campaign tmp/perm_324d0 from the 27-floor pin-free seed, ~109k
    iterations, 8 jobs, --stop-on-zero, --stack-diffs. Result: zero score-0
    finds; best find 105/170 IS the banned invented-intermediate family's
    direction (pointer-carrier `new_var` copy of the base-pointer load); every
    other sub-170 class sampled is semantics-breaking (case-constant stored
    instead of val, coincidentally matching target's register). The frontier-1
    caveat is now measured fact: the only byte-moving lever in this basin is
    the banned plant. **KILLED (for this seed/basin; R3 now 1 of 2 used).**

## Frontier (for s5+)
1. **Ladder modalities (synthesis/rederive/forensics-2):** in-shape honest
   routes are exhausted ([s1]-[s3]) and the seed's permuter basin is measured
   dry ([s4]). A genuinely NEW chassis (different CFG that still emits 68
   matching insns) is the only remaining structural object; deriving one is
   rederive/synthesis work, not mutation search.
2. **Second permuter session (R3 1 of 2 remaining) — ONLY with a new seed.**
   Re-running from the same 27-floor chassis is measured waste. If a future
   modality produces a different-shape 27-or-better chassis, spend the last
   permuter session on it; otherwise escalation per the driver's ladder.


## [s3] 2026-08-20 (structural)

9. **H9 — a naturally-live local-allocated $3 pseudo overlapping val AND cmd
   excludes $3 in find_reg pass 0 by CONFLICT, yielding the target cascade**
   (the [s2] frontier-1 hypothesis). Probe: fresh .lreg census on the pin-free
   chassis. Result: walker (73) dies in 0 places and its loop live range is a
   strict SUPERSET of val's and cmd's — any $3 holder overlapping both also
   conflicts with the walker, which then cannot take $3 (cascade val→5,
   cmd→6, walker→7 ≠ target). Structurally impossible for every statement
   arrangement, not just the ones tried. **KILLED (proof, not sample).**
10. **H10 — a type-axis change perturbs the lreg census enough to shift
    find_reg** ([s2] frontier 3). Probe: T1 cmd-u8 (38/69, WORSE —
    rejected/cmd-u8-type.c), T2 val-u32 (flat 27), T3 c-u32+mask (flat 27).
    **KILLED.**
11. **H11 — a switch-guard / loop-form / decl-order respelling changes the
    pseudo census toward the rotation.** Probe: bare-switch (flat 27),
    while-form loop (flat 27), reversed decls (flat 27). **KILLED.**

## Frontier (for s4+)
1. **Permuter campaign** (R3 cap: 2 sessions, 0 used): structural mutation
   search from the 27-floor seed — the only remaining route to a shape
   OUTSIDE the hand-enumerated space. NOTE for the vetter: within the current
   matching shape, ALL three find_reg routes to the rotation are dead
   (preference = banned family only, walker defs cannot plant a pref;
   conflict = liveness-impossible, evidence.md [s3]; priority = arithmetic,
   [s1]) — so any permuter find that closes to 0 while KEEPING the 68-insn
   shape almost certainly acts through an invented-intermediate preference
   plant and must be vetted against the banned base/ff family with extreme
   suspicion. A find that closes via a genuinely DIFFERENT shape (different
   insn count folding back to 68, different block structure) is the
   interesting case.
2. Ladder: after permuter, this function's honest in-shape space is exhausted
   → forensics/rederive/synthesis modalities per the driver, then escalation.

12. **H12 — merged single-variable loop spelling (c and cmd as ONE u32, no
    copy at loop head) changes the census toward the rotation.** Probe:
    sandbox on the merged spelling ([s3]). Result: 37, build 67 — the andi
    (u8→u32 promotion at the cmd copy) disappears, one insn SHORT, shape
    breaks. Banked at rejected/merged-c-cmd.c. **KILLED.**

5. **H5 — a matched sibling/duplicate exhibits the natural closing spelling.**
   Probe: tmp/duplicates_leads.txt + git history of cpu_get_dist_2. Result:
   the 1.000-similarity lead IS func_800324D0 under its retired duplicate
   name (deleted in 2651e2e5). **KILLED.**
6. **H6 — the cmd-copy statement pins the rotation; removing it changes the
   census enough to flip allocation.** Probe: no-copy spelling (test `c`
   directly). Result: flat 27, 68/68. **KILLED as closer** (valid alternative
   spelling, same floor).
7. **H7 — a literal-0xFF CSE scratch (block-local pseudo) perturbs local-alloc
   enough to re-seed the cascade.** Probe: literal stores, no holder. Result:
   flat 27, 68/68. **KILLED.**
8. **H8 — payload-arm increment placement changes loop scratch liveness.**
   Probe: ptr++ after switch. Result: 30, 66 insns — shape breaks. **KILLED.**

## Frontier (for s3+; the banned base/ff family and all its respellings are OFF the table)
1. **Honest $3-conflict route (find_reg exclusion by CONFLICT, not preference):**
   read the pin-free chassis .lreg/.greg loop-body block-pseudo census
   (`pwsh tools/grinder/dump.ps1 func_800324D0`); enumerate every natural
   statement arrangement that changes which scratch pseudos are live across
   the payload arm (val/cmd live ranges). Mechanism: find_reg pass-0 excludes
   conflicting hard regs; a $3-holding local pseudo overlapping val AND cmd
   yields the target cascade without any preference construct. Only if a
   NATURAL arrangement produces it — inventing a local for this purpose is the
   banned construct again.
2. **Permuter campaign (R3 cap: 2 sessions max, none used yet):** seed from
   the 27-floor pin-free chassis; structural mutations only. Any closing find
   must clear the 6-test vet AND must not be a respelling of the banned
   block-0 overlapping-intermediate family — vet before surfacing.
3. **Type-axis sweep (cheap, unprobed):** cmd as u8 (QImode subtract), val as
   u32, c as u32 with explicit `& 0xFF` — each changes the lreg census
   slightly; measure before dismissing. Low prior, but unmeasured.


## [s1] 2026-08-20 (recon)

1. **H1 — the 27 is a pure 3-cycle register rotation (walker/cmd/val), no
   shape or schedule residual.** Probe: objdump diff of pin-free sandbox .o vs
   target. Result: confirmed — 27 rename diffs, 68/68 insns, andi + single
   sltiu present. **CONFIRMED.**
2. **H2 — the rotation is a global-RA allocation-order effect, not a tie.**
   Probe: .lreg/.greg + allocno_compare arithmetic. Result: order is
   val(4.95) > cmd(4.2) > byte(3.05) > pad(2.98) > walker(1.55); inversion by
   ref/live tweaks needs ~61 weighted walker refs. **CONFIRMED (and the
   priority-lever class KILLED).**
3. **H3 — a bare pointer-alias copy can plant a walker preference.** Probe:
   `q = v1` alias in the ≥0x80 arm, sandbox + .greg census. Result: flat 27,
   census unchanged (cse coalesces copies pre-RA). **KILLED.**
4. **H4 — walker full-pref $3 via an overlapping block-0 `base`/`ff` pair
   closes the function.** Mechanism: local-alloc ff→$2, base→$3;
   set_preference PLUS-lookthrough on `ptr = base + 5`; prune_preferences →
   regs_someone_prefers {3} on val/cmd; find_reg pass-0 cascade. Probe:
   the base/ff spelling in src, sandbox. Result: **score 0, 68/68, measured
   twice (before and after semantic renames). CONFIRMED — candidate-ready.**

## [s1] Frontier — SUPERSEDED 2026-08-20 by the layer-1 FAIL
The s1 candidate WAS bounced on construct grounds (Test-3, decisions.md:9542)
and the named fallback axis ("other natural two-consumer intermediates in
block 0") is EXPLICITLY BANNED by the driver's banned-constructs list — do
not pursue it in any spelling. Current frontier: see [s2] above.

## [s1] A matched sibling (cpu_get_dist_2, similarity 1.000) exhibits the natural closing spelling
- mechanism: duplicate-lead transplant
- probe: tmp/duplicates_leads.txt + git history
- result: cpu_get_dist_2 is func_800324D0's own retired duplicate-address name (asm file deleted in commit 2651e2e5); lead is self-vs-self
- verdict: KILLED

## [s1] Removing the cmd-copy statement (test c directly, cmd = c - 0x80 only in payload arm) flips the register rotation
- mechanism: different allocno census changes allocno_compare order / find_reg seeding
- probe: sandbox --disable all on the no-copy spelling
- result: flat 27, 68/68
- verdict: KILLED

## [s1] Literal 0xFF per store (no named holder) re-seeds local-alloc via the CSE scratch pseudo
- mechanism: block-local CSE pseudo changes local-alloc assignments feeding set_preference
- probe: sandbox on literal-constant spelling
- result: flat 27, 68/68; walker def src is mem(reg pad), untouched by the scratch
- verdict: KILLED

## [s1] Moving the payload arm's ptr++ after the switch changes loop scratch liveness toward target
- mechanism: statement order changes block-pseudo live ranges in the payload arm
- probe: sandbox on reordered spelling
- result: WORSE: 30, build_insns 66 (increment merged, shape broke)
- verdict: KILLED

## [s2] A naturally-live local-allocated $3 pseudo overlapping val AND cmd excludes $3 in find_reg pass 0 by CONFLICT, yielding the target rotation
- mechanism: global.c find_reg pass-0 conflict exclusion; local-alloc block-pseudo liveness
- probe: Fresh .lreg/.greg dumps on the pin-free 27-floor chassis; walker (73) dies in 0 places and is live at the start of every loop block where val (76) or cmd (75) is live, so any $3 holder overlapping both also conflicts with the walker, which then cannot take $3 (cascade val->5, cmd->6, walker->7, not target). All current block scratches (79/80/84/85/86) are die-at-def chain links local-allocated to $2.
- result: Structurally impossible for every statement arrangement; combined with s1 (preference route = banned family only, walker defs are all self-increments so set_preference can never plant a pref; priority route arithmetically dead), all three honest find_reg routes within the matching 68-insn shape are closed
- verdict: KILLED

## [s2] A type-axis change (cmd u8 / val u32 / c u32 with explicit mask) perturbs the lreg census enough to shift find_reg
- mechanism: mode changes alter refs/live density and scratch pseudos
- probe: sandbox --disable all on each of the three variants
- result: cmd-u8: 38, build 69 (WORSE, banked rejected/cmd-u8-type.c); val-u32: flat 27 68/68; c-u32 + cmd = c & 0xFF: flat 27 68/68
- verdict: KILLED

## [s2] A switch-guard / loop-form / declaration-order respelling changes the pseudo census toward the rotation
- mechanism: flow-graph and RTL-emission-order perturbation of the allocno census
- probe: sandbox on bare switch (no if(cmd<12)), while-form loop replacing guard+do-while, reversed decl order
- result: All three flat 27, 68/68 - valid alternative spellings, same floor; allocno_compare order is strictly priority-sorted with no ties
- verdict: KILLED

## [s2] Merging c and cmd into one u32 loop variable (no copy at loop head) changes the census toward the rotation
- mechanism: fewer loop-carried pseudos re-seeds allocation
- probe: sandbox on the merged spelling
- result: 37, build 67 - the andi from the u8->u32 promotion at cmd=c disappears, one insn short, shape breaks; banked rejected/merged-c-cmd.c
- verdict: KILLED

## [s3] H13 — the permuter finds a structural spelling outside the hand-enumerated space that closes the 3-cycle register rotation honestly (ledger frontier 1)
- mechanism: structural mutation search (decomp-permuter, weighted byte-diff scorer) from the pin-free 27-floor candidate.c seed
- probe: campaign tmp/perm_324d0 via tools/permuter_campaign.py (label s3-pinfree-27, 8 jobs, --stop-on-zero, --stack-diffs), base permuter score 170, ~109k iterations over ~55 min, harvested and stopped in-session; best-find and exemplar diffs vetted by hand against the 6-test checklist and the banned base/ff family
- result: Zero score-0 finds and zero finds below 105 across ~400 outputs. Best find (105) is an invented block-0 pointer intermediate 'new_var = *(u8**)(pad+0x58); ptr = new_var;' — the banned base/ff invented-intermediate family's direction, partially cse-coalesced so it cannot close without the full banned overlapping-pair construct. All other sampled sub-170 classes are semantics-breaking mutations (case-constant stored instead of val) that only score better because the constant's register coincidentally matches target at that store.
- verdict: KILLED

## [s4] H14: a second campaign from a genuinely different C text at floor 27 (the four s2/s3 flat-27 respellings combined, measured flat 27 this session) finds an honest closure the first basin could not
- mechanism: permuter mutation space is over SOURCE text, so a same-score different-text seed is a different basin
- probe: campaign tmp/perm_324d0_s4 (label s4-combined-respelling-27, 8 jobs, --stop-on-zero, base 170, ~12k iters, ~25 min, harvested + stopped in-session); every sub-120 find hand-vetted against the cheat catalog
- result: zero score-0; 110/115/120 finds are forbidden families (invented locals, empty-if dead-read, if(1){} wrap, invented pointer carrier); output-105-1 is a semantics-preserving staged tail read that hand-measures 15
- verdict: KILLED

## [s4] H15: staging the loop-tail stream-byte read through the existing currently-dead u32 cmd re-seeds the find_reg census toward the target rotation
- mechanism: global.c allocno census: the extra cmd set splits cmd into head web (75) and arm web (85); val/byte/0xFF-holder/jtbl-base all land in target registers
- probe: sandbox --disable all + BB2_FINDREG_DEBUG sweep (s4/findreg*.log) + objdump side-by-side (s4/residual15_sbs.txt)
- result: 27 -> 15, build 68 == target 68, re-verified on the final annotated text; residual is exactly walker ours $6/target $3 and cmd(both webs) ours $3/target $6; construct classified staged-value-reused-variable (bounds walked in evidence.md [s5], SOTN precedent sotn-construct-index.md:51,81,92,97,109), FAKE-annotated in src
- verdict: CONFIRMED

## [s4] H16: micro-variants of the staging close the remaining 2-swap
- mechanism: carrier mode / placement / guard form / decl order perturb the census further
- probe: seven variants measured: u8 carrier (27), ptr++ between read and copy (28/69), preheader staged (flat 15), arm load staged via c (flat 15), decl-order (flat 15), do-while guard (flat 15), 0xFF-arm pointer-through-cmd borrow (flat 15, copy cse-coalesced)
- result: no variant moves the swap; the SImode borrow and its exact placement are load-bearing for the 15
- verdict: KILLED
