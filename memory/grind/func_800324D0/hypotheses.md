# Hypothesis ledger — func_800324D0

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
