# Hypothesis ledger — func_800324D0

## [s12] 2026-08-20 (structural, brief-session 11; scratch tmp/grind/func_800324D0/s11/)

25. **H25 — one of the two structural axes never measured on any chassis
    (plain ISO `register` storage class; block-scope placement of a local)
    perturbs the find_reg census toward the target rotation** (expected
    FALSE per the s6 cascade partition — but both were genuine coverage
    holes: the s2-s3 structural sweeps predate the 15-chassis and never
    touched storage class or scope nesting). Probe: (a) `register` on all
    four locals (no asm() pins), (b) `u8 val;` moved into the payload-arm
    block (the only liveness-narrowable local). Both sandbox-measured this
    session: **FLAT 15, 68/68 each** (probe log
    tmp/grind/func_800324D0/s11/probe_log.md). Neither axis touches
    refs/live census or webs; allocno_compare never consults
    REG_USERVAR_P. **KILLED — both axes measured shut; the partition's
    coverage now includes them explicitly.**

## Frontier (for the driver — the ladder is spent, flat in every modality ON the 15-chassis)
1. **Every honest axis is measured dead**, and with this session the
   post-s4-drop cycle is flat in every modality on the 15-chassis itself:
   permuter (s5/s6, R3 tripped both ways), synthesis (s7), forensics
   (s8), rederive (s9/s10/s11 — all three inputs + index-walk family),
   structural (s12). The ladder's R1 escalation disposition applies — the
   DRIVER's call, not a session's. No escalation entry exists yet in
   docs/grind/decisions.md (verified s12; sole hit is the layer-1 FAIL at
   line 9542), so the escalation-modality session must FILE it. Evidence
   spine for that session: H20 citation resolution (s7) + s6 cascade
   partition + s7/s8/s9 byte-identical-spellings uniformity proof + s9
   m2c closure + s10 corpus/sibling closure + s11 index-walk closure +
   s12 storage-class/scope closure.
2. No unmeasured honest probe remains on this chassis. Do not re-measure
   dead axes; do not propose permuter campaigns (R3 tripped both ways).

## [s11] 2026-08-20 (rederive, brief-session 10; scratch tmp/grind/func_800324D0/s10/)

24. **H24 — the index-based walk (fixed base + integer index, the one
    structurally different derivation of the stream walk never measured in
    s1-s10) produces a different find_reg census that reaches the target
    rotation** (expected FALSE — but it was a genuine hole in the
    "structurally different C shape" coverage, not a respelling). Probe:
    three spellings sandbox-measured this session — plain index (26,
    build 69), staged tail (14, build 69), biased base + staged tail (13,
    build 69; side-by-sides s10/sbs_index_staged14.txt /
    s10/sbs_biased_index13.txt). Result: ALL shape-broken at 69 insns.
    loop.c strength-reduces the index into a walking-pointer giv — GCC
    rebuilds the pointer form internally — and the register residual is
    the SAME walker↔cmd 2-swap in every spelling. The biased-base form
    matches the entire head including the walker-init `addiu $3,$3,5` in
    the beqz delay slot (target's exact geometry, first time reached); its
    extra insn is the giv header copy `move $6,$3`, deletable only if the
    giv gets $3 — the s2/s6-proven-impossible cmd-webs-skip-$3 condition —
    and the copy cannot plant a $3 pref because base is a global pseudo
    (live across the beqz edge; set_preference sees no renumbered hard
    reg). The sub-15 masked scores (14/13) are alignment-shift accounting
    on mis-shaped bodies, not floor progress. Banked:
    rejected/index-walk-family.c. **KILLED — the index-walk derivation
    axis is measured shut; the uniformity proof now covers the only
    structurally different walk derivation.**

## Frontier (for the driver — the ladder is spent, no caveats, no holes)
1. **Every honest axis is measured dead** (recon s1, structural s2-s3,
   permuter s4-s6 R3-exhausted, synthesis s7, forensics s8, rederive
   s9+s10+s11 — all three named rederive inputs run AND the index-walk
   derivation family closed). The ladder's escalation disposition applies
   — the DRIVER's call, not a session's. Evidence spine for the
   escalating/submitting session: H20 citation resolution + s6 cascade
   partition + s7/s8/s9 uniformity proof (four byte-identical flat-15
   spellings) + s9 m2c closure + s10 corpus/sibling closure + s11
   index-walk closure.
2. No unmeasured honest probe remains on this chassis. Do not re-measure
   dead axes; do not propose permuter campaigns (R3 tripped both ways).

## [s10] 2026-08-20 (rederive, brief-session 9; scratch tmp/grind/func_800324D0/s9/)

23. **H23 — an external sibling (decomp.me corpus scratch or Kengo
    equivalent) exhibits a structurally different C shape for this parser
    that re-opens the closed partition** (the last never-run rederive
    inputs; expected FALSE). Probe: (a) decomp_me_scrape.py shingle
    search over the full 3,754-scratch corpus (all three BB2-relevant
    compilers) — best similarity 0.090 noise floor, zero matching
    scratches resembling the function; (b) kengo_matches.csv row =
    size-only-ambiguous among 83 candidates at combined_score 0.00, the
    "is_pad/Pad_Prs" src banner proven misattributed (Pad_Prs = BB2
    func_80057CC8 / 0x80032314, 111 insns, not our 68), full is_pad.c
    family scan finds no leaf stream-parser body; (c) the LIBSND
    memory-resident walk idiom (`ptr = *base; *base = ptr + 1;`) is dead
    a priori — it emits store-backs the 68-insn target lacks. Artifacts:
    s9/corpus_search_results.txt, s9/kengo_family_notes.txt. **KILLED —
    no external derivation input exists; the rederive modality is closed
    on all three of its inputs (m2c s9, corpus s10, sibling s1+s10).**

## Frontier (for the driver — the ladder is spent, now with no caveats)
1. **Every honest axis is measured dead** (recon s1, structural s2-s3,
   permuter s4-s6 R3-exhausted, synthesis s7, forensics s8, rederive
   s9+s10 — all three rederive inputs now explicitly run). The ladder's
   escalation disposition applies — the DRIVER's call, not a session's.
   Evidence spine for the escalating/submitting session: H20 citation
   resolution + s6 cascade partition + s7/s8/s9 uniformity proof (four
   byte-identical flat-15 spellings) + s9 m2c closure + s10
   corpus/sibling closure.
2. No unmeasured honest probe remains on this chassis. Do not re-measure
   dead axes; do not propose permuter campaigns (R3 tripped both ways).

## [s9] 2026-08-20 (forensics/rederive, brief-session 8; scratch tmp/grind/func_800324D0/s8/)

22. **H22 — m2c re-derivation of the TARGET asm exposes an original-source
    structural signal (different CFG / dataflow) that re-opens the s6
    cascade partition** (the [s8] frontier 1; expected FALSE). Probe: m2c
    run on asm/funcs/func_800324D0.s + asm/rodata/jtbl_800105A0.s (first
    explicit m2c run on this function; output s8/m2c_target.c); structure
    compared against candidate.c; the one unmeasured respelling axis it
    surfaced (switch-folded -0x80: casesi emits the subtraction into its
    own scratch instead of the cmd web) measured three ways. Result: same
    guard + do-while + 3-arm + jtbl-switch CFG; R1 (switch on c, 0x80
    cases) = 27 68/68, R2 (cmd = c; switch(cmd), 0x80 cases) = 27 68/68 —
    both collapse the s4 allocno-85 arm web and restore the 27 rotation;
    R3 (m2c-verbatim head-copy + RMW subtract + staged tail, target's
    exact one-web $a2 dataflow) = 15 68/68 and BYTE-IDENTICAL to the
    candidate's .o (s8/sbs_r3.txt diff-clean vs s4/residual15_sbs.txt).
    **KILLED — the rederive axis is closed; no structural signal exists;
    `cmd = c - 0x80` carrying the subtracted value in the cmd web is
    load-bearing for the 15.**

## Frontier (for the driver — the ladder is spent)
1. **Every honest axis is measured dead** (recon s1, structural s2-s3,
   permuter s4-s6 R3-exhausted, synthesis s7, forensics s8, rederive s9).
   The ladder's escalation disposition applies — the DRIVER's call, not a
   session's. Evidence spine for the escalating/submitting session: H20
   citation resolution + s6 cascade partition + s7/s8/s9 uniformity proof
   (four byte-identical flat-15 spellings) + the s9 rederive closure.
2. No unmeasured honest probe remains on this chassis. Do not re-measure
   dead axes; do not propose permuter campaigns (R3 tripped both ways).

## [s8] 2026-08-20 (forensics, brief-session 7; scratch tmp/grind/func_800324D0/s7/)

21. **H21 — the residual-15 diff COMPOSITION differs across the flat-15
    spellings (candidate vs s5 basin2 vs G4-dup), exposing an unexplored
    seam** (the [s7] frontier 2; expected FALSE). Probe: each spelling
    applied to src, sandbox-measured (all 15, 68/68), objdump side-by-side
    via s4/sbs.py, full-file `diff` against s4/residual15_sbs.txt. Result:
    all three side-by-sides are IDENTICAL over the entire 68-line listing —
    the three spellings emit byte-identical machine code, so the residual
    is the SAME single walker↔cmd 2-swap everywhere and no seam exists
    among known flat-15 spellings. **KILLED (and the wall proven fully
    uniform — stronger than the expected same-diff-set outcome).**

## Frontier (for s9+ — one honest input remains)
1. **Rederive: m2c the TARGET asm** (unchanged from [s7] frontier 1 —
   still never run on this function). Mechanism: the cascade partition
   enumerates orders within OUR build's structure; the target's own
   decompiled shape is the one remaining independent derivation input.
   Next probe: m2c asm/funcs/func_800324D0.s, compare structure against
   candidate.c; any genuinely different semantics-equivalent CFG gets ONE
   sandbox measurement.
2. **If rederive confirms the same CFG (expected): every honest axis is
   measured dead.** The ladder's escalation disposition applies (driver's
   call, not a session's). Evidence spine for the escalating/submitting
   session: H20 citation resolution + s6 cascade partition + s7/s8
   uniformity proof (byte-identical flat-15 spellings).

## [s7] 2026-08-20 (synthesis, brief-session 6; scratch tmp/grind/func_800324D0/s6/)

18. **H18 — a statement geometry exists where the cmd head web's live length
    grows enough to drop allocno 75 below val, opening a different find_reg
    cascade to the target rotation** (the [s6] frontier 1). Probe: G1 (head
    test on cmd with preheader staging) = 30/70; G2 (loop condition on cmd)
    = 32/69 — every head-web-lengthening spelling breaks the 68-insn shape
    (keeping cmd live across the loop-back edge costs 1-2 insns). AND the
    cascade arithmetic shows the frontier was ill-posed: any order where val
    reaches find_reg with $3 unheld gives val→$3 (someone_prefers {4} only),
    not walker→$3 — dropping 75 below val hands $3 to val, never to the
    walker. **KILLED (measured + arithmetic; the full partition is banked at
    tmp/grind/func_800324D0/s6/probes_and_partition.md).**
19. **H19 — the strongest natural reg_n_refs lift (staged tail duplicated
    into all 3 arms, duplicated-statement-into-arms shape) inverts the
    allocation order toward walker-first.** Probe: G4 measured FLAT 15,
    68/68 (cross-jump re-merges byte-neutrally); the lift raises cmd's refs
    proportionally, so the order never inverts. Walker-first needs ~78
    weighted walker refs vs 24 — no natural geometry approaches it. G5a/G5b
    arm reorderings also flat. **KILLED.**
20. **H20 — the staged-value-reused-variable citation question is resolvable
    from the rule text without an owner ruling** (the [s6] frontier 2).
    Probe: read the rule + the 2026-08-17 clarification
    (no-new-park-categories.md:193-214). The bounds are mechanism-silent,
    bound 4 asks for the ACTUAL pass (ours names global.c), the sched.c
    reference is descriptive Origin text, the clarification's interpretive
    principle is that SOTN acceptance is shape-based, and the SOTN exemplar
    is literally our shape (`i = *scriptCur++` through an existing
    variable). **CONFIRMED — citation clean; no ruling-request needed on
    citation grounds; full argument banked for the eventual self-vet.**

## Frontier (for s8+ — RESET by the synthesis pass)
1. **Rederive: m2c the TARGET asm** (never explicitly run on this function)
   and check the CFG/dataflow assumptions the cascade partition rests on.
   Mechanism: the partition enumerates orders within OUR build's structure;
   an original-source structural signal (different guard nesting, different
   walk idiom) is the one remaining honest input that could re-open it.
   Next probe: m2c asm/funcs/func_800324D0.s, compare structure against
   candidate.c; any genuinely different semantics-equivalent CFG gets ONE
   sandbox measurement.
2. **Forensics-2: verify the residual-15 diff COMPOSITION is identical
   across the flat-15 spellings** (candidate vs G4-dup vs s5 basin2 text).
   Mechanism: a differing swap composition at the same score would expose an
   unexplored seam; expected identical (then the wall is fully uniform).
   Next probe: objdump side-by-side per spelling, diff the diff-sets against
   s4/residual15_sbs.txt.
3. **If 1 and 2 confirm (expected): every honest axis is measured dead.**
   The ladder's escalation disposition applies (driver's call, not a
   session's). The submitting/escalating session should carry the H20
   citation resolution and the s6 partition as its evidence spine.

## [s6] 2026-08-20 (permuter, brief-session 5; scratch tmp/grind/func_800324D0/s5/)

17. **H17 — the 15-floor staged chassis, never before permuted, has a
    permuter-reachable lever for the residual 2-swap (walker $6 <-> cmd-webs
    $3).** Probe: campaign #1 from candidate.c verbatim (tmp/perm_324d0_s5,
    ~118k iters, ~31 min, base 105) and campaign #2 from a genuinely
    different flat-15 text (s4's flat micro-variants combined:
    preheader-staged read + guard+do-while + cmd-first decls, measured flat
    15 before seeding; tmp/perm_324d0_s5b, ~106k iters, ~29 min). Result:
    ZERO improving finds across ~224k iterations in two basins; the only
    output in EACH basin is the identical score-neutral invented
    constant-holder `unsigned short new_var = 0x80;` (noise, and a forbidden
    invented-local direction besides). Unlike the 27-chassis basins, not even
    a banned-family carrier scored below base. **KILLED — the 2-swap is not
    mutation-reachable; permuter exhausted under both R3 counting
    conventions (2 permuter-modality sessions AND flat yield).**

## Frontier (for s7+)
1. **Unchanged from [s5] frontier 1: the walker-$3 wall needs a rederive/
   synthesis-derived statement geometry** (lengthen the cmd head web's live
   range so allocno 75 drops below val in allocno_compare; re-derive the
   full find_reg cascade from exclusion sets before spending a build).
   Mechanism: global.c allocno_compare + find_reg pass 0. Next probe: m2c
   re-derivation + hand-synthesis of head-web-lengthening geometries,
   measured against s4/findreg exclusion arithmetic.
2. **Unchanged from [s5] frontier 2: the staged-value-reused-variable
   citation question** (rule exemplar mechanism sched.c vs our global-RA
   mechanism; bounds are mechanism-silent) must be resolved from the rule
   text + no-new-park-categories 2026-08-17 clarification, or
   ruling-requested, BEFORE any candidate-ready.
3. **Permuter is now mechanically closed** (two permuter-modality
   floor_history entries + flat second yield trips grindlib.py's R3 gate).
   Do not propose campaigns; the driver will not mandate the modality again.

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

## [s5] H17 - the 15-floor staged chassis, never before permuted (both prior campaigns seeded from 27-floor texts), has a permuter-reachable lever for the residual walker $6 <-> cmd-webs $3 2-swap
- mechanism: decomp-permuter structural mutation search over source text; a different C text at the same floor is a different mutation basin (the s4-proven trick)
- probe: Campaign #1 from candidate.c verbatim (tmp/perm_324d0_s5, label s5-staged-15, 8 jobs, --stop-on-zero, base permuter score 105, ~118k iterations over ~31 min); campaign #2 from a genuinely different flat-15 text (s4's flat micro-variants combined: preheader read staged through cmd, guard+do-while loop form, cmd-first decl order; combination measured flat 15 68/68 this session before seeding; tmp/perm_324d0_s5b, ~106k iterations over ~29 min). Both harvested + stopped in-session.
- result: ZERO improving finds in either basin. The single output in EACH basin is the identical score-neutral (105==base) invented constant-holder 'unsigned short new_var = 0x80;' - noise, and a forbidden invented-local direction besides. Unlike the 27-chassis basins (s3/s4), not even a banned-family carrier scored below base.
- verdict: KILLED

## [s6] H18: a statement geometry lengthening the cmd head web drops allocno 75 below val and opens a different find_reg cascade to the target rotation
- mechanism: global.c allocno_compare priority ordering + find_reg pass-0 cascade
- probe: G1 head test on cmd (preheader staged): sandbox 30, build 70; G2 loop condition on cmd: 32, build 69; plus cascade arithmetic: any order where val reaches find_reg with $3 unheld gives val->$3 (someone_prefers {4} only), never walker->$3
- result: every head-web-lengthening spelling breaks the 68-insn shape (cmd live across the loop-back edge costs 1-2 insns), and the frontier was arithmetically ill-posed: dropping 75 below val hands $3 to val, not the walker; full partition banked in s6/probes_and_partition.md
- verdict: KILLED

## [s6] H19: the strongest natural reg_n_refs lift (staged tail duplicated into all 3 arms) inverts allocation order toward walker-first
- mechanism: flow.c reg_n_refs counting before jump2 cross-jump re-merge (duplicated-statement-into-arms shape)
- probe: G4 duplication: sandbox 15, build 68 (byte-neutral re-merge confirmed); G5a/G5b payload-arm reorderings: both 15, 68/68
- result: flat - the lift raises cmd refs proportionally so the order never inverts; walker-first needs ~78 weighted walker refs vs actual 24, unreachable by any natural geometry
- verdict: KILLED

## [s6] H20: the staged-value-reused-variable citation question (sched.c origin exemplar vs our global-RA mechanism) is resolvable from the rule text without an owner ruling
- mechanism: the rule's bounds 1-6 are mechanism-silent; the 2026-08-17 named-intermediate clarification establishes SOTN acceptance is shape-based
- probe: read staged-value-reused-variable.md full text + no-new-park-categories.md:193-214 this session
- result: citation is CLEAN: bound 4 requires naming the ACTUAL pass (our annotation names global.c), sched.c sits in descriptive Origin text not the bounds, clarification prong (4) confirms the borrow family keeps its own mechanism-silent bounds, and the SOTN exemplar (i = *scriptCur++ staged through an existing variable) is literally our shape; full argument banked for the eventual self-vet
- verdict: CONFIRMED

## [s7] H21 - the residual-15 diff composition differs across the flat-15 spellings (candidate vs s5 basin2 vs G4-dup), exposing an unexplored seam
- mechanism: the same sandbox score can decompose into different swap sets; a differing composition at equal score would mark a different find_reg outcome reachable by spelling
- probe: each spelling applied to src and sandbox-measured this session (all 15, build 68 == target 68), then objdump side-by-side via tmp/grind/func_800324D0/s4/sbs.py and full-file diff against the banked s4/residual15_sbs.txt
- result: all three side-by-sides are IDENTICAL over the entire 68-line listing - the spellings emit byte-identical .o code; the residual is the exact same walker ours-$6/target-$3, cmd-webs ours-$3/target-$6 swap at 15 instruction sites in every case
- verdict: KILLED

## [s8] H22 - m2c re-derivation of the TARGET asm exposes an original-source structural signal (different CFG/dataflow) that re-opens the s6 cascade partition
- mechanism: the s6 cascade partition enumerates allocation orders within OUR build's structure; the target's own decompiled shape was the one remaining independent derivation input
- probe: first explicit m2c run on this function (tools/m2c/m2c.py --valid-syntax --target mipsel-gcc-c asm/funcs/func_800324D0.s asm/rodata/jtbl_800105A0.s, output s8/m2c_target.c); CFG compared to candidate.c; the surfaced switch-folded-subtract respelling axis measured 3 ways in the sandbox: R1 switch-on-c 0x80-cases = 27 68/68, R2 cmd=c + switch(cmd) 0x80-cases = 27 68/68, R3 m2c-verbatim head-copy + RMW cmd-=0x80 + staged tail = 15 68/68 with s4/sbs.py side-by-side byte-IDENTICAL to s4/residual15_sbs.txt
- result: same guard+do-while+3-arm+jtbl-switch CFG as candidate.c; R1/R2 collapse the allocno-85 arm web (casesi puts the -0x80 in a die-at-def scratch) and restore the full 27 rotation; R3 is a fourth member of the uniform flat-15 basin, not a new seam; cmd = c - 0x80 carrying the subtracted value in the cmd web is load-bearing for the 15
- verdict: KILLED

## [s9] H23 - an external sibling (decomp.me corpus scratch or Kengo equivalent) exhibits a structurally different C shape for this parser that re-opens the closed partition
- mechanism: rederive modality's two never-run inputs: corpus transplant and Kengo transplant; a genuinely different semantics-equivalent CFG would get one sandbox measurement
- probe: decomp_me_scrape.py shingle search over the full 3,754-scratch corpus (907 gcc2.7.2-cdk + 1,554 gcc2.7.2-psx + 1,293 psyq3.5); kengo_matches.csv row audit + kengo_ref.py full is_pad.c family scan + Pad_Prs banner attribution check; LIBSND (src/main.c verbatim-Sony region + sotn-decomp libsnd) walk-idiom comparison
- result: Corpus: best similarity 0.090 (noise floor; s1's self-duplicate scored 1.000), and those hits are themselves non-matching scratches - no transplantable sibling exists. Kengo: csv row is size-only-ambiguous among 83 candidates at combined_score 0.00; the src 'kengo:HIGH is_pad/Pad_Prs' banner is misattributed legacy residue (Pad_Prs = BB2 func_80057CC8/0x80032314, 111 insns vs our 68); no is_pad.c body is a leaf 12-case-jtbl stream parser. LIBSND's memory-resident walk idiom (ptr = *base; *base = ptr + 1) emits store-backs the 68-insn target provably lacks - dead a priori.
- verdict: KILLED

## [s10] H24 - the index-based walk (fixed base pointer + integer index, the one structurally different derivation of the stream walk never measured in s1-s10) produces a different find_reg census that reaches the target rotation
- mechanism: loop.c strength reduction turns base[i]+increments into a walking-pointer giv with its own header copy; a different pseudo census could re-seed global.c find_reg pass 0
- probe: three spellings sandbox-measured: plain index (i=5), staged tail (cmd=base[i]; c=cmd), biased base (base+=5; i=0) + staged tail; objdump side-by-sides banked at tmp/grind/func_800324D0/s10/sbs_index_staged14.txt and sbs_biased_index13.txt
- result: 26/69, 14/69, 13/69 - ALL shape-broken at build 69 vs target 68. GCC rebuilds the pointer form internally (giv), register residual is the SAME walker-cmd 2-swap everywhere. Biased-base form matches the ENTIRE head incl. walker-init addiu $3,$3,5 in the beqz delay slot (target's exact geometry, first spelling ever to reach it); the extra insn is the giv header copy move $6,$3, deletable only if the giv takes $3 (the s2/s6-proven-impossible cmd-webs-skip-$3 condition), and the copy cannot plant a $3 preference because base is a global pseudo live across the beqz edge (set_preference sees no renumbered hard reg). Sub-15 masked scores are alignment-shift accounting on mis-shaped 69-insn bodies, not floor progress.
- verdict: KILLED

## [s11] H25 — one of the two structural axes never measured on any chassis (plain ISO register storage class on the locals; block-scope placement of the payload local val) perturbs the find_reg census toward the target walker-cmd rotation
- mechanism: global.c allocno_compare / find_reg pass-0 cascade: a census or web change from storage class or scope nesting could re-seed the allocation order
- probe: sandbox --disable all on (a) register on all four locals, no asm() pins; (b) u8 val declared inside the payload-arm block (the only liveness-narrowable local; c/cmd are loop-tail-live, ptr loop-carried); chassis verified 15 68/68 before, between, and after (probe log tmp/grind/func_800324D0/s11/probe_log.md)
- result: Both FLAT 15, build 68 == target 68. Storage class: GCC 2.7.2 allocno_compare consults only priority (floor_log2(refs)*refs/live), never REG_USERVAR_P. Scope nesting: neither refs/live census nor webs change. The s6 cascade partition's prediction confirmed on both previously-unmeasured axes.
- verdict: KILLED
