# Hypothesis ledger — func_800324D0

## [s13] 2026-08-20 (escalation, brief-session 12; scratch tmp/grind/func_800324D0/s12/) — DISPOSITION FILED

26. **H26 — disposition session bookkeeping, no new probe.** Chassis
    re-verified: candidate.c applied to src measured **15, build 68 ==
    target 68** (s12/sandbox_floor15_reverify.log) — unchanged since s4,
    all banked kills current. Gate (a): `scan_hand_coded --single` =
    **LOW 0/8** (s12/scan_hand_coded.log) — canonical-asm refused. Gate
    (b): zero hits in docs/reference/sotn-construct-index.md (1,365
    entries) for register pins / overlapping-live-range pairs / any
    RA-steering family; the only measured closer is the layer-1-FAILed,
    driver-banned base/ff pair (decisions.md:9542) — no citable
    precedent. **Both AND-gates FAIL → standing ruling 2026-07-27
    auto-applied: REFUSED / OWNER-ACCEPTED INCOMPLETE, entry appended at
    docs/grind/decisions.md tail (2026-08-20 func_800324D0
    OWNER-ESCALATION — RESOLVED BY STANDING RULING). Terminal park; the
    HEAD 4-pin body stays only to hold the oracle. Src restored to HEAD
    after measurement.**

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

## [s12] H26 - the chassis is unchanged: the banked candidate still measures floor 15 on current HEAD
- mechanism: chassis re-verification mandated by the dispatch brief (HEAD floor was 'measurement unavailable' because the committed body is the legacy 4-pin cheat-asm form)
- probe: candidate.c applied to src/code6cac_b.c, sandbox func_800324D0 --disable all; src restored from HEAD afterward
- result: score 15, build_insns 68 == target_insns 68 (tmp/grind/func_800324D0/s12/sandbox_floor15_reverify.log) - identical to the s4-s12 ledger floor; every banked axis-kill is current
- verdict: CONFIRMED

## [s12] H27 - gate (a): the function shows hand-coded-asm evidence supporting the canonical-asm grant path
- mechanism: endgame-lock-disposition.md gate 1: STRONG S1/S2/S6 tier required
- probe: python3 tools/scan_hand_coded.py --single func_800324D0 (tmp/grind/func_800324D0/s12/scan_hand_coded.log)
- result: tier=LOW score=0/8, 'no strong hand-coded indicators' - all eight signals absent; the residual is a pure register-allocation tiebreak, ordinary GCC output
- verdict: KILLED

## [s12] H28 - gate (b): an in-hand SOTN-master precedent exists for a construct that closes the residual 15
- mechanism: endgame-lock-disposition.md gate 2: file+line citation required; the only measured closer is the invented overlapping base/ff local pair (layer-1 FAILed decisions.md:9542, driver-banned in all spellings), and the HEAD body's closer is 4 register-asm pins
- probe: grep of docs/reference/sotn-construct-index.md (1,365 entries) for register pins / overlapping-live-range pairs / set_preference-find_reg steering families, this session; plus the banked s10 decomp.me corpus sweep (3,754 scratches, best sim 0.090) and the 2026-07-01 community research (zero precedent for register-asm pins)
- result: ZERO hits - no citable file+line exists for either closing construct; a completed negative search, not an open question
- verdict: KILLED

## [s13] H29 — the owner-directed solver modality (tools/ra_solver, never run on this function) finds a modelled input perturbation that reaches the target walker/cmd assignment
- mechanism: global.c allocno-priority + find_reg inverse solve — search minimal perturbations of refs / live length / birth order / conflicts / preferences / calls-crossed that flip pseudo 73 to $v1 and pseudos 75+85 to $a2, each mapped to a C-level lever
- probe: full solver chain, first ever run on func_800324D0, candidate.c applied to src (chassis re-verified 15, 68/68 first): `extract.py func_800324D0 code6cac_b` -> model.json; `simulate.py` forward check; `mkasm_honest.sh` run TWICE (once with HEAD src to capture the TRUE post-regfix/asmfix target stream, once with candidate src for the honest stream — mkasm_honest builds .tgt.s from whatever src holds, so the naive single run produced a fiction that classified IDENTICAL/PRE-RA; banked TRUE.tgt.s); `goal_from_asm.py` -> goal.json; `inverse.py global --goal {"73":3,"75":6,"85":6} --depth 3 --top 10`
- result: forward model is EXACT (sort order MATCH, dispositions 8/8). goal_from_asm independently re-derives the ledger's residual from the two asm streams: 15 substituted operands over 11 instructions, $a2->$v1 x12 and $v1->$a2 x3 — the same uniform 2-swap s4–s12 measured by hand. inverse.py returns **NEGATIVE / FORECLOSED**: 199 single perturbation atoms over 7 classes, refs delta +12/-6, live length +/-2,4,8, depth 3 — no vector reaches the goal. 16 preference atoms are not even emitted, on a mechanical ground the hand analysis never stated this crisply: `prera_hard = [4]` — $v1 and $a2 NEVER appear as hard registers in this function's pre-RA RTL (the only pre-RA hard reg is the incoming parameter $a0), so global.c set_preference can never record a preference for either register, for ANY C spelling. The function is a leaf with no calls, so no argument/return hard reg can be introduced without breaking the 68-insn shape.
- verdict: KILLED (mechanically typed FORECLOSED, replacing s1/s2/s6's hand arithmetic with a tool verdict)

## [s13] H30 — the three mechanisms the solver names as OUTSIDE its global model (local-alloc suggested-register pass, qty_size/DImode mispricing, reload spill-retry) hold the residual and are worth instrumenting
- mechanism: inverse.py's own escape hatch — a NEGATIVE global result means the flip was forced by an input the global model does not carry; the README names exactly three such mechanisms
- probe: `local_extract.py code6cac_b --func func_800324D0` + `local_alloc.py code6cac_b --func func_800324D0`; model.json `modes`/`sizes` inspection for DImode; model.json retry-block scan (`grep -c retry` = 0) cross-checked against scan_hand_coded's spill count
- result: ALL THREE MEASURED INERT for this function. (1) local-alloc: exactly 5 quantities, all in different blocks, and EVERY ONE gets `got=2` ($v0) — no local quantity ever touches $v1 or $a2, so neither the main pass nor the suggested-register pass can seed or exclude the two contested registers (order 4/4 blocks, assign 4/5 qtys; the single unscored miss is a live-hard-reg block, still $v0). (2) DImode: `modes` are SI/QI only and `sizes` is empty — no DImode quantity exists to misprice. (3) reload retry: zero retry blocks in the model, 0 spills per scan_hand_coded — the function never enters reload's spill loop, so retry_global_alloc is never called. The solver's "extend the instrumentation before spending another spelling search" lead is therefore closed by measurement rather than left open.
- verdict: KILLED

## [s13] H31 — gate (a) re-verified this session: canonical-asm grant path
- mechanism: endgame-lock-disposition.md gate 1 (STRONG S1/S2/S6 tier required)
- probe: `python3 tools/scan_hand_coded.py --single func_800324D0` (tmp/grind/func_800324D0/s13/scan_hand_coded.log)
- result: tier=LOW score=0/8 on all eight signals (80 insns, 0 spills, 7 distinct regs) — unchanged from s12. Ordinary GCC output; asm refused.
- verdict: KILLED

## [s13] The owner-directed solver modality (tools/ra_solver, never run on this function in 12 sessions) finds a modelled input perturbation that reaches the target walker/cmd assignment.
- mechanism: global.c allocno-priority + find_reg inverse solve over refs / live length / birth order / conflicts / preferences / calls-crossed, each atom mapped to a C-level lever
- probe: extract.py func_800324D0 code6cac_b -> model.json; simulate.py forward check; mkasm_honest.sh run twice (HEAD src for the TRUE target stream, candidate src for the honest stream); goal_from_asm.py -> goal.json; inverse.py global --goal {73:3,75:6,85:6} --depth 3 --top 10
- result: Forward model EXACT (sort order MATCH, dispositions 8/8). goal_from_asm independently re-derives the residual: 15 substituted operands over 11 instructions, $a2->$v1 x12 / $v1->$a2 x3 - identical to the hand-measured 2-swap. inverse.py returns NEGATIVE/FORECLOSED: 199 atoms over 7 input classes, refs delta +12/-6, live length +/-2,4,8, depth 3, no vector reaches the goal. Mechanical reason newly surfaced: prera_hard=[4] - $v1 and $a2 never appear as hard regs in this function pre-RA RTL, so global.c set_preference can never record a preference for either under ANY C spelling (16 preference atoms not emitted). Leaf function, so introducing those hard regs pre-RA via a call site would break the 68-insn shape.
- verdict: KILLED

## [s13] The three mechanisms inverse.py names as OUTSIDE its global model (local-alloc suggested-register pass, qty_size/DImode mispricing, reload spill-retry) hold the residual and are worth instrumenting.
- mechanism: the solver escape hatch: a NEGATIVE global result normally means the flip was forced by an unmodelled input, and the README names exactly these three
- probe: local_extract.py + local_alloc.py --func func_800324D0; model.json modes/sizes inspection for DImode; retry-block scan cross-checked against the scan_hand_coded spill count
- result: ALL THREE INERT. local-alloc: exactly 5 quantities, EVERY one assigned got=2 ($v0) - neither the main nor the suggested-register pass can seed or exclude $v1/$a2 (order 4/4 blocks, assign 4/5 qtys; the single miss is a live-hard-reg block still landing $v0). DImode: modes are SI/QI only and the sizes map is empty - nothing to misprice. Reload: zero retry blocks, 0 spills - retry_global_alloc is never called. No instrumentation lead remains to buy.
- verdict: KILLED

## [s13] Gate (a): the function shows hand-coded-asm evidence supporting the canonical-asm grant path.
- mechanism: endgame-lock-disposition.md gate 1, STRONG S1/S2/S6 tier required
- probe: python3 tools/scan_hand_coded.py --single func_800324D0
- result: tier=LOW score=0/8, all eight signals absent (80 insns, 0 spills, 7 distinct regs) - unchanged from s12; ordinary GCC output, asm refused
- verdict: KILLED

## [s13] The chassis is unchanged: the banked candidate still measures floor 15 on current HEAD.
- mechanism: chassis re-verification mandated by the dispatch brief (HEAD floor unavailable because main carries the legacy 4-pin body)
- probe: candidate.c applied to src/code6cac_b.c; sandbox func_800324D0 --disable all; src restored to HEAD afterwards (git status --porcelain src/ clean)
- result: score 15, build_insns 68 == target_insns 68 - identical to the s4-s12 floor, so every banked axis-kill is current
- verdict: CONFIRMED

## [s14] H32 — the owner's 2026-08-30 ruling 5 (jtbl_800105A0 re-wiring GRANTED) can be executed WITHOUT a bb2.ld edit, entirely inside a grind session's src surface
- mechanism: `INCLUDE_RODATA(FOLDER, NAME)` (include/include_asm.h:18) emits `.section .rodata` + `.include "<folder>/<name>.s"` at the point of the toplevel `__asm__` statement. Placing it immediately after `INCLUDE_ASM("asm/funcs", func_800324D0)` in the SAME translation unit means (a) the table lands in code6cac_b.o(.rodata) at exactly the slot the compiler-generated switch table used to occupy (between jtbl_80010548 and jtbl_800105D0 — 0x800105A0 + 12*4 == 0x800105D0), and (b) the `.L8003256C…L800325C4` local labels the table references are DEFINED by the func_800324D0.s the neighbouring INCLUDE_ASM pulls in, so no symbol has to be globalised and no linker-script line is needed. The s13 packet's premise ("routing it back requires editing bb2.ld / the splat inputs, outside a grind session's allowed surface") was wrong — it assumed the jtbl had to be a separate object file, as in the func_80036940 / replay_camera_rob_back_loose2 jtbl-infra cases where the table lives in a DIFFERENT rodata TU that links far earlier.
- probe: replaced src/code6cac_b.c:1698-1752 (the four-`register asm("$N")`-pin body) with the two lines `INCLUDE_ASM("asm/funcs", func_800324D0);` / `INCLUDE_RODATA("asm/rodata", jtbl_800105A0);`, then `& tools/wteng.ps1 main build` (tmp/grind/func_800324D0/s14/build_sha1_migrated.log), then `python3 tools/check_completion_integrity.py` under WSL.
- result: FIRST-TRY SUCCESS. Full clean-driver build links and `sha1 62efab4f73f992798c43e8c730aa43baa10bb4fa == want` **MATCH**; integrity audit prints "OK: all completed functions satisfy their category's invariants" with func_800324D0 no longer carrying any cheat-class construct. The four register-asm pins (`v1`/`v0`/`a2`/`a1`) are DELETED from main; src/code6cac_b.c now has zero `register … asm("…")` declarations. (Project-wide, pins still exist in src/config.c ×16, src/text1a_c.c ×26, src/code6cac_c2.c ×6 — those are other functions' items, so the s13 packet's "the last cheat-class constructs on main" phrasing was over-broad and is corrected here.) The function's representation is now honest INCLUDE_ASM per asm-until-matched; the pure-C residual is untouched and the item stays ACTIVE.
- verdict: CONFIRMED

## [s14] H33 — the ledger's floor of 15 is still the current chassis number
- mechanism: chassis re-verification mandated by the dispatch brief (driver reported HEAD floor "measurement unavailable"); every banked spelling conclusion is chassis-relative
- probe: candidate.c applied to src/code6cac_b.c TWO ways and sandboxed both times — (1) on top of this session's migrated file (tmp/grind/func_800324D0/s14/sandbox_chassis.log), and (2) via an exact replication of the s13 procedure from a pristine `git checkout -- src/code6cac_b.c` (tmp/grind/func_800324D0/s14/sandbox_chassis_headbased.log)
- result: KILLED — **both runs score 17**, not 15 (target_insns 68 == build_insns 68, scorable, rules_dropped 0, cheat_asm_stripped 39, identical in both runs). The chassis has drifted +2 since the s13 measurement of 2026-08-26 with NO change to this function's C: `git log --since=2026-08-25` over engine/, src/code6cac_b.c, include/, tools/maspsx, tools/gcc-2.7.2 shows only unrelated completions (func_80078654, special_camera_get_rot_dir) and disposition commits. Most likely mechanism is the known [[sandbox-lo16-text-addend-false-distance]] artifact — engine/score.py masks branch/jump targets but NOT section-relative R_MIPS_LO16 addends, and this function takes the address of jtbl_800105A0, so a neighbouring completion that shifted the code6cac_b rodata stream can move the score without moving the codegen. NOT yet distinguished from a real +2 regression; the next session should diff the two disassemblies before spending any spelling work, because a chassis whose absolute number is addend-contaminated makes small floor deltas unreadable.
- verdict: CONFIRMED (drift measured), with the CAUSE open

## [s14] The owner's 2026-08-30 ruling 5 (jtbl_800105A0 re-wiring GRANTED) can be executed WITHOUT a bb2.ld/splat edit, entirely inside a grind session's src surface, by splicing asm/rodata/jtbl_800105A0.s into the same translation unit with INCLUDE_RODATA.
- mechanism: INCLUDE_RODATA (include/include_asm.h:18) emits .section .rodata + .include at the toplevel __asm__ statement's position, so the table lands in code6cac_b.o(.rodata) in exactly the slot the compiler-generated switch table occupied (0x800105A0, between jtbl_80010548 and jtbl_800105D0; 0x800105A0 + 12*4 == 0x800105D0), and its .L8003256C..L800325C4 local labels resolve against the func_800324D0.s spliced in by the INCLUDE_ASM one line above. The s13 packet's bb2.ld premise assumed the func_80036940 jtbl-rodata-split shape, where the table lives in a DIFFERENT, earlier-linking rodata TU; that does not apply here.
- probe: Replaced src/code6cac_b.c:1698-1752 (the 4-pin body) with INCLUDE_ASM("asm/funcs", func_800324D0); + INCLUDE_RODATA("asm/rodata", jtbl_800105A0); then ran `& tools/wteng.ps1 main build` and `python3 tools/check_completion_integrity.py` (WSL).
- result: First-try success. sha1 62efab4f73f992798c43e8c730aa43baa10bb4fa == want, MATCH (verified twice; tmp/grind/func_800324D0/s14/build_sha1_migrated.log). Integrity audit: 'OK: all completed functions satisfy their category's invariants.' src/code6cac_b.c now has zero `register ... asm(...)` declarations; the function carries zero regfix/asmfix rules and is in no maspsx gate list.
- verdict: CONFIRMED

## [s14] The ledger's floor of 15 is still the current chassis number for the banked candidate.
- mechanism: Dispatch-brief chassis re-verification: every banked spelling conclusion is chassis-relative, and the driver reported the HEAD floor as 'measurement unavailable'.
- probe: candidate.c sandboxed twice with `sandbox func_800324D0 --disable all`: (1) applied over this session's migrated file (s14/sandbox_chassis.log), (2) applied over a pristine `git checkout -- src/code6cac_b.c` replication of the s13 procedure (s14/sandbox_chassis_headbased.log).
- result: Both runs score 17, not 15 (target_insns 68 == build_insns 68, rules_dropped 0, scorable; identical logs). No commit since 2026-08-25 touched this function's C, engine/, include/, tools/maspsx or tools/gcc-2.7.2. Prime suspect is the known lo16-addend false-distance artifact (engine/score.py masks branch/jump targets but not section-relative R_MIPS_LO16 addends; this function takes %hi/%lo of jtbl_800105A0). Cause not yet separated from a genuine +2 regression.
- verdict: KILLED

## [s14] The four register-asm pins on func_800324D0 were the last cheat-class constructs on main (the s13 packet's claim).
- mechanism: Post-migration project-wide re-audit of register-asm pins in src/.
- probe: grep -rn 'register .* asm(' src/*.c after the migration landed.
- result: FALSE as stated: pins remain in src/config.c (16), src/text1a_c.c (26) and src/code6cac_c2.c (6), each under its own queue item. The migration's benefit is real but local: code6cac_b.c is now pin-free.
- verdict: KILLED

## [s15] 2026-08-31 (synthesis, brief-session 15; scratch tmp/grind/func_800324D0/s15/)

34. **H34 — the s14 "+2 chassis drift" (15 -> 17) is the
    [[sandbox-lo16-text-addend-false-distance]] artifact rather than a codegen
    regression.** mechanism (as inherited): engine/score.py masks branch/jump
    targets but not section-relative R_MIPS_LO16 addends, and this function takes
    `%hi/%lo` of jtbl_800105A0. probe: reproduce the 17 (candidate applied over a
    pristine HEAD checkout -> `sandbox --disable all` = **17**,
    s15/sandbox_candidate.log), then diff the two normalized instruction streams
    that score.py itself compares (`engine.score.normalized_insns` over
    build/src/code6cac_b.o vs tmp/sandbox/func_800324D0/code6cac_b.o,
    s15/insn_diff_candidate.log). result: **KILLED — the drift is real, its cause
    is now positively identified, and it is NOT the lo16 artifact.**
    score.py has masked section-relative HI16/LO16 addends since that memory note
    was written (engine/score.py:63 `_SECTION_ADDEND_RELOCS = {"R_MIPS_HI16",
    "R_MIPS_LO16"}`, plus `_mask_section_addend`), so the candidate's jtbl load
    already normalizes to `lui a3,@.rodata` / `addiu a3,a3,@.rodata`. The two
    extra diffs came from the **REFERENCE** side: `sandbox` scores against
    `build/src/<stem>.o` (engine/sandbox.py:72), and s14 left that object built
    from **its own INCLUDE_ASM/INCLUDE_RODATA migration of this TU** (mtime
    18:59, exactly the s14 migrated build). In the migrated object the table is
    reached through `%hi/%lo(jtbl_800105A0)` — relocations against a NAMED GLOBAL
    symbol, which `_mask_section_addend` deliberately does not mask — so the
    reference stream read `lui a3,0x0` / `addiu a3,a3,0` against our masked
    `@.rodata`: two instructions of pure false distance, 15 -> 17.
35. **H35 — with the reference object rebuilt from pristine HEAD the floor is
    still 15 and the residual is still the uniform 2-swap.** mechanism: the
    reference must be a byte-correct build of the PINNED source, not of any
    session's edit; `sandbox` never rebuilds it. probe: `git checkout --
    src/code6cac_b.c` -> `& tools/wteng.ps1 main build` (sha1
    62efab4f73f992798c43e8c730aa43baa10bb4fa == want, MATCH,
    s15/build_head_reference.log) -> re-apply candidate.c -> `sandbox --disable
    all`. result: **CONFIRMED — score 15, target_insns 68 == build_insns 68,
    rules_dropped 0** (s15/sandbox_candidate_freshref.log). The re-diff
    (s15/insn_diff_candidate_freshref.log) is exactly 15 substituted operands over
    11 instructions and is a **pure `v1` <-> `a2` rename**: zero insertions, zero
    deletions, zero reorderings. Every chassis-relative kill banked in s1-s13 is
    therefore current, and the s14 "17" must not be quoted again. **Standing
    procedure for this function: if the floor does not read 15, rebuild
    `build/src/code6cac_b.o` from a pristine checkout BEFORE concluding
    anything.**
36. **H36 — the owner-ruling-5 INCLUDE_ASM migration can be landed by a grind
    session "through the normal candidate path" (the remedy prescribed by the
    2026-08-31 Judge FAIL, decisions.md:16725).** mechanism:
    `Invoke-CandidatePath` requires the driver's own `sandbox --disable all`
    re-check to print 0 (grind.ps1:561); any non-candidate outcome ends with
    `git checkout -- src include` (grind.ps1:886), which reverts every src edit a
    `progress` session makes. probe: applied the exact s14 migration
    (`INCLUDE_ASM("asm/funcs", func_800324D0);` +
    `INCLUDE_RODATA("asm/rodata", jtbl_800105A0);`) and ran `sandbox
    func_800324D0 --disable all`. result: **KILLED —
    `{"score": 68, "build_insns": 0, "no_c_body": true, "target_insns": 68}`**
    (s15/sandbox_include_asm_form.log). An INCLUDE_ASM body has no C body to
    score, so the sandbox reports the FULL 68 rather than 0; the candidate path
    is mechanically unreachable for a representation-only change, and a
    `progress` session's src edit is mechanically reverted. **The migration is
    therefore not landable by ANY grind session under the current driver** — it
    needs the operator lane (an ordinary `cheat-cleanup:` commit) or a driver
    change. Recorded as a mechanical fact only: it is NOT re-filed as an
    integration handoff (that framing already drew a Judge FAIL) and no session
    should spend turns re-attempting it.
37. **H37 — the closure argument for the residual is a bounded search (s13's
    depth-3 inverse.py NEGATIVE) and therefore leaves unexamined input space.**
    mechanism: re-derive the requirement directly from the exact forward model
    instead of searching perturbations of it. probe:
    `tools/ra_solver/extract.py func_800324D0 code6cac_b` +
    `simulate.py --trace` re-run this session on the fresh chassis
    (s15/simulate_trace.log): sort order MATCH, dispositions **8/8**. result:
    **KILLED — the closure is a PROOF over the whole input space, not a bounded
    search.** Exact model numbers (allocation order 75, 76, 85, 72, 74, 73, 91,
    86):
      - pseudo 73 = the walker (`ptr`), pri **15483**, gets `$6`; the target
        wants `$3`.
      - pseudo 75 = the `c & 0xff` web, pri **75000**, gets `$3`; 85 = the
        `-0x80` web, pri 34285, `$3`; 76 = `val` (QI), pri 47272, `$5`;
        72 = `pad`, 29838, `$4`; 74 = `c`, 26666, `$2`.
      - `conflicts[73] = [72, 73, 74, 75, 76, 85, 86, 91]` — **73 conflicts with
        every other allocno in the function**, because the walker is live from
        the first instruction to the last.
    Two exhaustive cases for "73 receives $3":
    **(a) 73 is allocated before 75, 76 and 85.** It must out-prioritise 75 at
    75000, a **4.84x** lift of 15483. With
    `pri = floor_log2(n)*n/livelen * 10000`, 73's live length cannot fall below
    the loop body (~54 of the 68 instructions), so the only free variable is n:
    `floor_log2(n)*n > 465` needs ~80+ weighted refs against today's 24 — and
    every extra reference is an extra instruction in a shape fixed at 68.
    Symmetrically, pushing 75, 76 and 85 all below 15483 requires 76 (`val`) to
    be live across roughly the whole loop, but `val` is defined and consumed
    inside the payload arm by the function's own semantics. **Arithmetically
    foreclosed.**
    **(b) $3 is excluded for 75/76/85 by a conflict.** Any such exclusion comes
    either from an earlier allocno already holding $3 or from a hard `$v1` live
    range. Because 73 conflicts with EVERY allocno and is live at EVERY
    instruction, either source excludes $3 for 73 as well. **Structurally
    foreclosed — case (b) can never help, for any C spelling, at any depth.**
    Case (b) is new this session and is what upgrades s13's "no vector found at
    depth 3" to "no vector exists". The residual is unreachable by ANY
    allocation-input perturbation of this instruction shape; only a different
    68-insn shape in which the walker is not live across the loop could move it,
    and the function's semantics (one forward stream pointer) forbid such a
    shape.

## Frontier (for s16+ — RESET by this synthesis pass)
1. **The floor is 15 and the chassis-drift question is CLOSED.** Do not
   re-measure the drift; do re-run `build` from a pristine checkout first if any
   future sandbox prints something other than 15 (H34/H35).
2. **The RA residual is FORECLOSED BY PROOF, not by search** — H37 case (b) is
   the new decisive leg. No refs / live-length / birth-order / conflict /
   preference perturbation of this 68-insn shape can hand `$3` to the walker.
   The only theoretically open door is a different 68-instruction shape in which
   the walker's live range does not span the loop, which the function's
   semantics forbid. Any session that wants to re-open this must first exhibit
   such a shape, not another spelling.
3. **Representation debt, operator-only:** main still carries the four
   `register asm("$N")` pins; the owner-ruling-5 migration is verified correct
   and oracle-green (s14) but is mechanically unlandable from a grind session
   (H36). It needs the operator lane. Do not spend session turns on it, and do
   not re-file it as an integration handoff.

## [s15] The s14 '+2 chassis drift' (15 -> 17) is the known sandbox-lo16-text-addend-false-distance artifact rather than a codegen regression.
- mechanism: engine/score.py was believed to mask branch/jump targets but not section-relative R_MIPS_LO16 addends, and this function takes %hi/%lo of jtbl_800105A0, so a rodata shift could move the score without moving the codegen.
- probe: Reproduced the 17 (candidate.c applied over a pristine HEAD checkout; tmp/grind/func_800324D0/s15/sandbox_candidate.log), then diffed the two normalized instruction streams score.py itself compares via engine.score.normalized_insns over build/src/code6cac_b.o and tmp/sandbox/func_800324D0/code6cac_b.o (s15/insn_diff_candidate.log).
- result: The drift is real but the attributed cause is wrong. score.py ALREADY masks section-relative HI16 and LO16 addends (engine/score.py:63 _SECTION_ADDEND_RELOCS plus _mask_section_addend), so our jtbl load normalizes to `lui a3,@.rodata` / `addiu a3,a3,@.rodata`. The two extra diffs came from the REFERENCE side: sandbox scores against build/src/<stem>.o (engine/sandbox.py:72) and never rebuilds it, and s14 left that object built from its own INCLUDE_ASM/INCLUDE_RODATA migration of this TU (mtime matching the s14 migrated build). In that object the table is reached through %hi/%lo(jtbl_800105A0) — relocations against a NAMED GLOBAL symbol, which _mask_section_addend deliberately does not mask — so the reference stream read `lui a3,0x0` / `addiu a3,a3,0`. Exactly two instructions of pure false distance.
- verdict: KILLED

## [s15] With the reference object rebuilt from pristine HEAD the floor is still 15 and the residual is still the uniform walker/cmd 2-swap.
- mechanism: The sandbox reference must be a byte-correct build of the PINNED source, not of any session's edit; the sandbox command never regenerates it.
- probe: `git checkout -- src/code6cac_b.c` -> `& tools/wteng.ps1 main build` (s15/build_head_reference.log) -> re-apply candidate.c -> `& tools/wteng.ps1 main sandbox func_800324D0 --disable all` (s15/sandbox_candidate_freshref.log) -> re-diff the normalized streams (s15/insn_diff_candidate_freshref.log).
- result: Build sha1 62efab4f73f992798c43e8c730aa43baa10bb4fa == want, MATCH. Sandbox: score 15, target_insns 68 == build_insns 68, rules_dropped 0. The diff is exactly 15 substituted operands over 11 instructions and is a PURE v1 <-> a2 rename: zero insertions, zero deletions, zero reorderings. Every chassis-relative kill banked in s1-s13 is therefore current and the s14 '17' must never be quoted again.
- verdict: CONFIRMED

## [s15] The owner-ruling-5 INCLUDE_ASM migration can be landed by a grind session 'through the normal candidate path', which is the remedy the 2026-08-31 Judge FAIL prescribed (docs/grind/decisions.md:16725).
- mechanism: Invoke-CandidatePath requires the driver's own `sandbox --disable all` re-check to print 0 (tools/grinder/grind.ps1:561); every non-candidate outcome ends with `git checkout -- src include` (grind.ps1:886), which reverts a progress session's src edit.
- probe: Applied the exact s14 migration (INCLUDE_ASM("asm/funcs", func_800324D0); + INCLUDE_RODATA("asm/rodata", jtbl_800105A0);) to src/code6cac_b.c and ran `& tools/wteng.ps1 main sandbox func_800324D0 --disable all` (s15/sandbox_include_asm_form.log).
- result: The sandbox returns {"score": 68, "build_insns": 0, "no_c_body": true, "target_insns": 68}. An INCLUDE_ASM body has no C body to score, so the sandbox reports the FULL 68 rather than 0 — the candidate path can never pass for a representation-only change, and a progress session's src edit is mechanically reverted. The migration is not landable by ANY grind session under the current driver; it needs the operator lane (an ordinary cheat-cleanup: commit) or a driver change. Recorded as a mechanical fact only — deliberately NOT re-filed as an integration handoff, since that framing already drew a Judge FAIL.
- verdict: KILLED

## [s15] The closure argument for the residual is a bounded search (s13's depth-3 inverse.py NEGATIVE) and therefore leaves unexamined allocation-input space.
- mechanism: Rather than searching perturbations of the forward model, derive the requirement directly from it: global.c allocno_compare priority plus find_reg's first-free scan over the conflict sets.
- probe: Re-ran tools/ra_solver/extract.py func_800324D0 code6cac_b and simulate.py --trace on the corrected chassis (s15/simulate_trace.log), then hand-derived the exhaustive case split for 'pseudo 73 receives $3'.
- result: Forward model still exact (sort order MATCH, dispositions 8/8). Order 75,76,85,72,74,73,91,86 with priorities 75000, 47272, 34285, 29838, 26666, 15483 (=73, the walker), 333, 326; conflicts[73] = [72,73,74,75,76,85,86,91] — the walker conflicts with EVERY other allocno because it is live at every instruction. Case (a) 73 allocated first: needs a 4.84x priority lift; pri = floor_log2(n)*n/livelen*10000, livelen cannot drop below the ~54-insn loop body, so floor_log2(n)*n > 465 requires ~80+ weighted refs against today's 24, each ref costing an instruction in a shape fixed at 68; symmetrically, driving 75/76/85 all below 15483 needs val (76) live across the whole loop, which the payload arm's semantics forbid. Case (b) $3 excluded for 75/76/85 by conflict: any earlier allocno holding $3, or any hard $v1 live range, also excludes $3 for 73, because 73 conflicts with everything and is live everywhere — so case (b) can never help at any depth, for any C spelling. Case (b) is new this session and upgrades 'no vector found at depth 3' to 'no vector exists'.
- verdict: KILLED

## [s16] 2026-08-31 (synthesis, brief-session 16; scratch tmp/grind/func_800324D0/s16/)

## [s16] H38 — the s15 foreclosure proof is incomplete: find_reg has non-conflict pass-0 exclusion channels that a C spelling could use to push $3 away from allocnos 75/76/85 without also pushing it away from the walker (73)
- mechanism: s15's decisive case (b) argues only about CONFLICTS ("73 conflicts with every allocno, so any $3-excluder for 75/76/85 excludes it for 73 too"). But global.c:998-1001 builds find_reg's pass-0 `used` set from THREE sources: hard_reg_conflicts, the complement of regs_used_so_far, and regs_someone_prefers[allocno]. The latter two are not conflicts, and regs_someone_prefers is explicitly asymmetric (prune_preferences merges only LOWER-priority conflicting allocnos' preferences), so it can in principle exclude a register for a high-priority allocno while leaving it available to a lower-priority one — exactly the asymmetry the residual needs.
- probe: Read tools/gcc-2.7.2/global.c:344-375 (regs_used_so_far seed), :877-935 (prune_preferences), :990-1010 (the pass-0 `used` construction), then measured the actual preference sets on the live chassis with tools/ra_solver/extract.py (which harvests hard_reg_copy_preferences / hard_reg_full_preferences through the instrumented cc1's BB2_FINDREG_DEBUG hook) — tmp/grind/func_800324D0/s16/model.json.
- result: BOTH non-conflict channels are closed. Leg 2 (`~regs_used_so_far`): global.c:363-368 seeds the set with every register satisfying `regs_ever_live[i] || call_used_regs[i]`; $v1/$3 is call-used on MIPS and this is a leaf function, so $3 sits in the seed before the first find_reg call and the complement (callee-saved registers only) can never exclude $3 for anybody, in any spelling. Leg 3 (`regs_someone_prefers`): measured EMPTY for every allocno that matters — full_prefs = {75:[], 76:[], 85:[], 72:[4], 74:[2], 73:[], 91:[], 86:[]}, copy_prefs = {72:[4], rest []}. The only hard-register preferences that exist in this function are $a0 (the incoming `pad` parameter copy) and $2, because set_preference needs a copy insn between a pseudo and a hard register, and a `void` leaf with one pointer parameter and no calls never binds $v1 under the MIPS ABI — no C spelling can create that site. And even granting one hypothetically, prune_preferences makes the channel self-defeating: a $3-preferring allocno must conflict with 75/76/85 and rank below them, which puts it either above 73 in allocno_order (it then takes $3 itself before the walker's turn) or below 73 (regs_someone_prefers[73] then inherits $3 and the walker is excluded too). No rank helps.
- verdict: KILLED (the gap is real but empty; the foreclosure now covers find_reg's complete pass-0 input space)

## [s16] H39 — the one door s15 left open (a 68-instruction shape whose walker web does not span the loop) can be written for a single forward stream pointer
- mechanism: Both of s15's closure legs depend on the walker being live at every instruction. A shape breaking its function-spanning live range invalidates both simultaneously.
- probe: Semantic derivation plus the one source-level move that could shorten the range without changing the instruction count — sinking `ptr = *(u8 **)(pad + 0x58);` below the 11 pad stores (probe P1; tmp/grind/func_800324D0/s16/sandbox_p1.log) — followed by a fresh RA-model extraction on that exact source (s16/model_p1_identical_to_candidate.json) to see whether the walker's livelen actually moved.
- result: The door does not exist. (i) Semantically, the walker is loop-carried — iteration N+1's read depends on iteration N's advance — so it is live-in at the loop head and live-out on the back edge, hence live at every instruction of the loop body in EVERY C spelling of a single forward stream pointer; allocnos 75/76/85 all live inside that body, so any $3-excluder for them necessarily conflicts with the walker. Splitting the walker into multiple webs is strictly worse, not better: the target holds it in ONE register ($v1) across the whole loop, so every split web would have to receive $3, and the measured instance of that family (s10's index-walk) lands at 69 insns, off-shape. (ii) Empirically, P1 measured FLAT 15, 68 == 68, and its RA model is byte-identical to the candidate's — same allocation order [75,76,85,72,74,73,91,86], same priorities [75000,47272,34285,29838,26666,15483,333,326], same walker livelen 62. cc1 re-establishes the live range regardless of statement position (the `lw` has no dependence on the stores), so the live-length term of allocno_compare's priority is not source-order-controllable for this pointer. That kills the whole "raise the walker's priority by shrinking its live range" family, not merely this spelling. Banked as rejected/walker-load-below-stores.c.
- verdict: KILLED

## [s16] H40 — the chassis is unchanged and the standing measurement procedure is correct as written
- mechanism: sandbox scores against build/src/<stem>.o and never rebuilds it (engine/sandbox.py:72), so a stale reference object poisons the absolute score while target_insns/build_insns stay a healthy-looking 68/68.
- probe: `git checkout -- src/code6cac_b.c` -> `& tools/wteng.ps1 main build` (s16/build_head_reference.log) -> apply candidate.c -> `sandbox func_800324D0 --disable all` (s16/sandbox_candidate_freshref.log).
- result: Build sha1 62efab4f73f992798c43e8c730aa43baa10bb4fa == want, MATCH. Sandbox: score 15, target_insns 68 == build_insns 68, rules_dropped 0. Floor 15 confirmed for the thirteenth consecutive session; the s15 procedure works exactly as documented and should be followed verbatim by any future session.
- verdict: CONFIRMED

## Frontier (for s17+ — RESET by this synthesis pass; supersedes the s15 frontier)
1. **The pure-C match is FORECLOSED, and the proof is now closed over
   find_reg's entire pass-0 input space.** Three legs: (a) priority — the
   walker cannot out-rank allocno 75 (needs a 4.84x lift; and s16 P1 shows the
   live-length term is not source-controllable at all); (b) conflicts — any
   allocno excluding `$3` from 75/76/85 also excludes it from the walker, which
   is live everywhere; (c) the two NON-conflict channels — `~regs_used_so_far`
   is saturated ($3 is call-used, leaf function) and `regs_someone_prefers` is
   structurally empty ($v1 has no copy site in a void leaf with one pointer
   param) and self-defeating by rank even if it were not. Do NOT spend another
   session on any register-seat spelling. A future session may only re-open this
   by falsifying one of the three legs with a measurement, not with a new
   spelling.
2. **The disposition question is representation, and it is already ANSWERED —
   only the landing is missing.** Owner ruling 5 (docs/grind/decisions.md:14829)
   grants the `INCLUDE_ASM`/`INCLUDE_RODATA` migration; s14 executed it and
   verified a full-build SHA1 == oracle; s15 measured that no grind session can
   land it (an INCLUDE_ASM body makes `sandbox` return score 68 / build_insns 0
   / no_c_body true, so grind.ps1:561's `score == 0` gate can never pass, and
   grind.ps1:886 reverts a progress session's src edit). This is a routing
   question for the owner/operator lane, NOT an integration handoff (that
   framing already drew a Judge FAIL on 2026-08-31) and NOT a grindable axis.
3. **Nothing else remains to measure.** Both endgame-lock gates FAIL and are not
   worth re-running: scan_hand_coded tier=LOW score=0/8 (s12 and s13), and zero
   SOTN-master file+line precedent for any closing construct (s12, checked
   against docs/reference/sotn-construct-index.md). Re-running either is the
   spinning the brief warns about.

## [s16] The s15 foreclosure proof is incomplete: find_reg has NON-conflict pass-0 exclusion channels a C spelling could use to push $3 away from allocnos 75/76/85 without also pushing it away from the walker (73).
- mechanism: s15's decisive case (b) argues only about conflicts. But tools/gcc-2.7.2/global.c:998-1001 builds the pass-0 `used` set from THREE sources: hard_reg_conflicts, the complement of regs_used_so_far, and regs_someone_prefers[allocno]. The last two are not conflicts, and regs_someone_prefers is explicitly ASYMMETRIC (prune_preferences merges only lower-priority conflicting allocnos' preferences), so in principle it can exclude a register for a high-priority allocno while leaving it free for a lower-priority one — exactly the asymmetry the residual needs.
- probe: Read global.c:344-375 (regs_used_so_far seed), :877-935 (prune_preferences), :990-1010 (pass-0 `used` construction); then measured the real preference sets on the live chassis with tools/ra_solver/extract.py, which harvests hard_reg_copy_preferences / hard_reg_full_preferences through the instrumented cc1's BB2_FINDREG_DEBUG hook (tmp/grind/func_800324D0/s16/model.json).
- result: Both non-conflict channels are closed. LEG 2 (~regs_used_so_far): global.c:363-368 seeds the set with every register satisfying regs_ever_live[i] || call_used_regs[i]; $v1/$3 is call-used on MIPS and this is a leaf, so $3 is in the seed before the first find_reg call and the complement (callee-saved only) can never exclude $3 for anyone, in any spelling. LEG 3 (regs_someone_prefers): measured EMPTY — full_prefs = {75:[],76:[],85:[],72:[4],74:[2],73:[],91:[],86:[]}, copy_prefs = {72:[4], rest []}. The only hard-register preferences in the function are $a0 (the incoming pad parameter copy) and $2, because set_preference needs a copy insn between a pseudo and a hard register, and a void leaf with one pointer parameter and no calls never binds $v1 under the MIPS ABI — no C spelling can create that site. Even granting one hypothetically, prune_preferences makes it self-defeating: a $3-preferring allocno must conflict with 75/76/85 and rank below them, which puts it either ABOVE 73 in allocno_order (it then consumes $3 itself before the walker's turn) or BELOW 73 (regs_someone_prefers[73] then inherits $3 and the walker is excluded too). No rank helps.
- verdict: KILLED

## [s16] The one door s15 left open — a 68-instruction shape in which the walker's live range does not span the loop — can be written for a single forward stream pointer.
- mechanism: Both of s15's closure legs depend on the walker being live at every instruction; a shape breaking its loop-spanning live range invalidates both at once. The only source-level move that could shorten that range without changing the instruction count is sinking the walker load below the 11 pad stores.
- probe: Semantic derivation, plus probe P1: `ptr = *(u8 **)(pad + 0x58);` moved below the store block, measured with `sandbox func_800324D0 --disable all` (s16/sandbox_p1.log), followed by a fresh tools/ra_solver/extract.py run on that exact source to see whether the walker's livelen actually moved (s16/model_p1_identical_to_candidate.json).
- result: The door does not exist. (i) Semantically the walker is loop-carried — iteration N+1's read depends on iteration N's advance — so it is live-in at the loop head and live-out on the back edge, hence live at every instruction of the loop body in EVERY C spelling of a single forward stream pointer; 75/76/85 all live inside that body, so any $3-excluder for them necessarily conflicts with the walker. Splitting the walker into multiple webs is strictly worse: the target holds it in ONE register ($v1) across the whole loop, so every split web would need $3, and the measured instance of that family (s10's index-walk) is off-shape at 69 insns. (ii) Empirically P1 is FLAT 15, 68 == 68, and its RA model is BYTE-IDENTICAL to the candidate's — same order [75,76,85,72,74,73,91,86], same priorities [75000,47272,34285,29838,26666,15483,333,326], same walker livelen 62. cc1 re-establishes the live range regardless of statement position (the lw has no dependence on the stores), so the live-length term of allocno_compare's priority is not source-order-controllable here. That kills the whole 'raise the walker's priority by shrinking its live range' family, not just this spelling.
- verdict: KILLED

## [s16] The chassis is unchanged and the s15 standing measurement procedure is correct as written.
- mechanism: sandbox scores against build/src/<stem>.o and never rebuilds it (engine/sandbox.py:72), so a stale reference poisons the absolute score while target_insns/build_insns stay a healthy-looking 68/68.
- probe: `git checkout -- src/code6cac_b.c` -> `& tools/wteng.ps1 main build` (s16/build_head_reference.log) -> apply candidate.c -> `& tools/wteng.ps1 main sandbox func_800324D0 --disable all` (s16/sandbox_candidate_freshref.log).
- result: Build sha1 62efab4f73f992798c43e8c730aa43baa10bb4fa == want, MATCH. Sandbox: score 15, target_insns 68 == build_insns 68, rules_dropped 0. Floor 15 confirmed for the thirteenth consecutive session; the procedure works verbatim and future sessions should follow it before quoting any number.
- verdict: CONFIRMED

## [s17] 2026-08-31 (synthesis, brief-session 17; scratch tmp/grind/func_800324D0/s17/) — MERGED-ATTACK PASS + DISPOSITION

## [s17] H41 — the chassis is still the 15-chassis on today's HEAD, and every chassis-relative kill banked in s1-s16 is therefore still current
- mechanism: `sandbox` scores the candidate against build/src/code6cac_b.o and never rebuilds it (engine/sandbox.py:72), so the ONLY way to quote an absolute floor honestly is the s15 standing procedure: pristine `git checkout -- src/code6cac_b.c` -> full build (reference object rebuilt, SHA1 checked) -> apply candidate.c -> sandbox. The driver's dispatch brief for this session reported the HEAD floor as "measurement unavailable", so nothing could be inherited.
- probe: `git checkout -- src/code6cac_b.c`; `& tools/wteng.ps1 main build` (s17/build_head_reference.log); `python tmp/grind/func_800324D0/s17/apply.py candidate`; `& tools/wteng.ps1 main sandbox func_800324D0 --disable all` (s17/sandbox_candidate_freshref.log).
- result: Build sha1 62efab4f73f992798c43e8c730aa43baa10bb4fa == want, MATCH. Sandbox: score 15, target_insns 68 == build_insns 68, rules_dropped 0, cheat_asm_stripped 38 (TU-wide, none from this function's candidate body). Floor 15 for the FOURTEENTH consecutive session across eight modalities. The standing procedure is confirmed correct a third time (s15, s16, s17) and should be run verbatim before any future session quotes a number.
- verdict: CONFIRMED

## [s17] H42 — the owner-ruling-5 INCLUDE_ASM migration may have rotted since s14 (HEAD has advanced: func_80045878 completed, func_80062020 foreclosed), so the operator record cannot be trusted without re-measurement
- mechanism: The s14 oracle-green proof was taken on a 2026-08-30 tree. Between then and now main gained b94a65de / 549970f8 / ef962c5f / 54313cae / dde89e31, at least one of which rewrote a sibling TU (src/text1a_c.c) and the rodata surface. If the INCLUDE_RODATA("asm/rodata", jtbl_800105A0) line collided with any of that, the granted remedy would be stale and the operator steps in the record would be wrong.
- probe: Applied the exact two-line migration to src/code6cac_b.c (deleting the legacy four-pin body) and ran a FULL build: `& tools/wteng.ps1 main build` (s17/build_sha1_migrated.log). Diff banked verbatim at s17/migration.diff (68 lines).
- result: sha1 62efab4f73f992798c43e8c730aa43baa10bb4fa == want, MATCH. The migration is oracle-green on today's HEAD (b94a65de), not merely on s14's tree; the local labels the jump table references are supplied by the same asm/funcs/func_800324D0.s that INCLUDE_ASM pulls in, so no bb2.ld edit and no scope grant are involved. The operator record filed this session is therefore actionable as written. Also measured: `python tools/check_completion_integrity.py` cannot run from the Windows interpreter (FileNotFoundError on OBJDUMP, s17/integrity_migrated.log) — the operator must run it under WSL. Reference object restored to pristine HEAD afterwards (s17/build_reference_restored.log, SHA1 MATCH) so the next session does not inherit the s14-shaped stale-reference artifact.
- verdict: CONFIRMED (the remedy is current; only the landing lane is missing)

## [s17] H43 — synthesis over the whole ledger leaves an unexamined axis somewhere in s1-s16 that a merged attack could still reach
- mechanism: The point of a synthesis pass is to look for a lever that only appears when the kills are read together rather than one modality at a time: the union of (structural respellings, two dry permuter campaigns, forensics byte-identity, three rederive inputs, the solver's exact forward model, and s15/s16's find_reg closure).
- probe: Full re-read of evidence.md (1272 lines), hypotheses.md (855 lines), state.json's 16-entry floor history, the 11-form rejected/ bank, and the six cross-knowledge decisions.md entries; then a cross-check of the one remaining structural question the merged view raises — whether the walker's seat could be decided by a pass OTHER than global.c (local-alloc qty assignment or reload), which no single-modality session had to answer.
- result: No axis survives. The seat is decided in global alloc and nowhere else: the walker spans basic blocks (loop-carried), so it is an allocno and never a local-alloc quantity, and tools/ra_solver's forward model reproduces the REAL allocation exactly (sort order MATCH, dispositions 8/8) — a model that predicts every seat correctly leaves no room for a later pass to be the cause. Independently, s16's leg-3 result is an ABI fact rather than a search result: `set_preference` cannot bind $v1 in a void leaf with one pointer parameter and no calls, and the signature is fixed by the target, so the one re-opening route the s16 frontier named ("exhibit a C form whose hard_reg_full_preferences contains 3") is closed by the function's own prototype, not by how hard anyone searched. The three foreclosure legs are therefore jointly exhaustive over source-controllable inputs.
- verdict: KILLED (no merged-attack lever exists; the pure-C residual is foreclosed)

## Frontier (for s18+ — RESET by this synthesis pass; supersedes the s16 frontier)
1. **DISPOSITION FILED — do not re-grind.** docs/grind/decisions.md:17155 records
   `OWNER-ESCALATION — RESOLVED BY STANDING RULING (2026-07-27): FORECLOSED` for this function, with
   the gate evidence (scan_hand_coded LOW 0/8; zero SOTN precedent), the three-leg find_reg closure,
   and the operator steps. If the item is ever re-activated WITHOUT one of the recorded triggers, the
   correct session action is to re-verify the chassis by the standing procedure and return
   owner-gated citing that entry — not to spend measurements.
2. **The only remaining work is an operator-lane commit, and its diff is banked.**
   `tmp/grind/func_800324D0/s17/migration.diff` (re-verified oracle-green on HEAD b94a65de this
   session) migrates the function to INCLUDE_ASM/INCLUDE_RODATA and deletes the four legacy register
   pins that main still carries at src/code6cac_b.c:1802. No grind session can land it: an
   INCLUDE_ASM body makes sandbox report score 68 / build_insns 0 / no_c_body true, so grind.ps1:561
   can never pass, and grind.ps1:886 reverts the edit on every other path. Do NOT re-file it as an
   integration handoff (Judge FAIL 2026-08-31, decisions.md:16725) and do NOT re-attempt it as a
   candidate.
3. **Re-opening the pure-C question requires falsifying a leg, not writing a spelling.** Concretely:
   a C form whose tools/ra_solver/extract.py model shows a non-empty hard_reg_full_preferences
   containing 3, or a walker livelen other than 62. Both are checkable in ONE extract run before any
   sandbox measurement is spent. s17 additionally showed the first is closed by the function's
   prototype (void leaf, one pointer param, no calls => no $v1 copy site can exist), so in practice
   only a toolchain/model finding or a new class grant can re-open this.

> [s17 addendum — MODALITY GATE] `grindlib.py validate` (grindlib.py:569) rejects an
> owner-gated outcome whose escalation_ref contains 'RESOLVED BY STANDING RULING' in any
> modality other than `escalation` — exhaustion is the driver's call, not a session's. That is
> the ONLY reason s17 returned `progress` rather than the disposition its own evidence
> supports. The foreclosure record is already filed and complete at
> docs/grind/decisions.md:17155, so the first session dispatched in `escalation` modality can
> return owner-gated citing it verbatim on turn one, after nothing more than the standing
> chassis re-verify (one build + one sandbox, expect 15 / 68 == 68).

## [s17] The chassis is still the 15-chassis on today's HEAD, so every chassis-relative kill banked in s1-s16 remains current (the driver's dispatch reported the HEAD floor as 'measurement unavailable', so nothing could be inherited).
- mechanism: sandbox scores the candidate against build/src/code6cac_b.o and never rebuilds it (engine/sandbox.py:72), so the only honest way to quote an absolute floor is the s15 standing procedure: pristine checkout -> full build (reference object rebuilt, SHA1 checked) -> apply candidate.c -> sandbox.
- probe: git checkout -- src/code6cac_b.c; `& tools/wteng.ps1 main build` (s17/build_head_reference.log); python tmp/grind/func_800324D0/s17/apply.py candidate; `& tools/wteng.ps1 main sandbox func_800324D0 --disable all` (s17/sandbox_candidate_freshref.log).
- result: Build sha1 62efab4f73f992798c43e8c730aa43baa10bb4fa == want, MATCH. Sandbox: score 15, target_insns 68 == build_insns 68, rules_dropped 0. Floor 15 for the fourteenth consecutive session across eight distinct modalities; the residual is still the pure v1<->a2 rename over 11 instructions with zero insertions/deletions/reorderings.
- verdict: CONFIRMED

## [s17] The owner-ruling-5 INCLUDE_ASM migration may have rotted since s14 (main has advanced five commits, including a sibling-TU completion and a rodata-touching change), so the operator record could not be filed on inherited evidence.
- mechanism: s14's oracle-green proof was taken on a 2026-08-30 tree. If the INCLUDE_RODATA("asm/rodata", jtbl_800105A0) line collided with any later change, the granted remedy would be stale and the filed operator steps would be wrong.
- probe: Applied the exact two-line migration (deleting the legacy four-pin body) and ran a full build: `& tools/wteng.ps1 main build` (s17/build_sha1_migrated.log); diff banked verbatim at s17/migration.diff; reference object then rebuilt from pristine HEAD (s17/build_reference_restored.log).
- result: sha1 62efab4f73f992798c43e8c730aa43baa10bb4fa == want, MATCH on today's HEAD (b94a65de). The remedy is current and needs no bb2.ld edit and no scope grant. It remains unlandable from a grind session: with it applied sandbox returns score 68 / build_insns 0 / no_c_body true, so grind.ps1:561's score==0 candidate gate can never pass and grind.ps1:886 reverts src on every other path. Also measured: tools/check_completion_integrity.py cannot run from the Windows interpreter (FileNotFoundError on OBJDUMP, s17/integrity_migrated.log) — the operator must run it under WSL.
- verdict: CONFIRMED

## [s17] Reading the whole ledger together (the union of structural, permuter, forensics, rederive and solver kills) exposes a merged-attack lever that no single-modality session could reach.
- mechanism: A synthesis pass exists to find a lever visible only across modalities; the one structural question the merged view raises that no prior session had to answer is whether the walker's seat could be decided by a pass other than global.c (local-alloc quantity assignment or reload).
- probe: Full re-read of evidence.md (1272 lines), hypotheses.md (855 lines), the 16-entry floor history, the 11-form rejected/ bank and the six cross-knowledge decisions.md entries; then the pass-attribution cross-check plus a prototype-level check of s16's leg-3 emptiness claim.
- result: No lever exists. The walker is loop-carried, hence an allocno and never a local-alloc quantity, and tools/ra_solver's forward model already reproduces the real allocation exactly (sort order MATCH, dispositions 8/8) — a model that predicts every seat correctly leaves no room for a later pass to be the cause. New this session: s16's leg-3 emptiness is an ABI/prototype fact, not a search result — set_preference needs a copy insn binding a pseudo to $v1, and a void leaf with one pointer parameter and no calls can never contain one, and the signature is fixed by the target. So the re-opening route the s16 frontier named ('exhibit a C form whose hard_reg_full_preferences contains 3') is closed by the function's own prototype. The three foreclosure legs are jointly exhaustive over source-controllable inputs.
- verdict: KILLED

## [s18] H44 — the s15/s16/s17 foreclosure closes find_reg's pass-0 input space for a FIXED allocno set, but never closed the allocno SET itself; an upstream (cse.c) unification could reach the target seats
- mechanism: `tools/ra_solver/inverse_compose.py hypothesis --merge P,Q` models exactly what cse.c does when it unifies two materialisations of one value: the survivor takes the SUM of the references, the UNION of the live range and the UNION of the conflicts, and the exact forward RA model is then replayed. That input class is upstream of every leg the foreclosure argues (priority / conflicts / ~regs_used_so_far / regs_someone_prefers), and no session had ever run it on this function.
- probe: tmp/grind/func_800324D0/s18/merge_sweep.py — all 56 ordered pairs of the 8 allocnos, via the tool, goal {73:$v1, 75:$a2, 85:$a2} (s13, dispositions 8/8 MATCH). Then tmp/grind/func_800324D0/s18/merge_depth2.py — all 2408 merge CHAINS of depth <= 2, in-process (merge_pseudos + Sim).
- result: 56/56 negative at depth 1; 2408/2408 negative at depth <= 2 (875 of them faithful, i.e. not deleting a goal pseudo). ZERO chains reach the full goal and ZERO chains put the walker (73) in $v1 at all — not even unfaithfully. Logs: s18/merge_sweep.log, s18/merge_depth2.log; JSON: s18/merge_sweep.json, s18/merge_depth2.json.
- verdict: KILLED (the CSE / allocno-set-shrink axis is closed to depth 2; it is NOT the missing input)

## [s18] H45 — THE FORECLOSURE'S PRIORITY LEG IS FALSE. The target disposition IS reachable by allocation ORDER alone, and the walker does not need a priority lift at all
- mechanism: The filed record (docs/grind/decisions.md:17155) argues "lifting the walker above 75 needs a 4.84x priority lift — arithmetically impossible". That is a claim about ONE pairwise race (73 vs 75). It was never checked against the full order space, so it never established that beating 75 is what the target actually requires.
- probe: tmp/grind/func_800324D0/s18/order_closure.py — enumerate ALL 8! = 40320 permutations of the allocation order and replay the exact find_reg forward model (Sim.simulate(order=...)) on each, everything else held at the measured model; count those reaching the FULL target disposition {72:$a0, 73:$v1, 74:$v0, 75:$a2, 76:$a1, 85:$a2, 86:$t0, 91:$a3}.
- result: **336 of 40320 orders reach the full 8/8 target disposition** (6720 seat the walker in $v1; 2016 reach the 3-seat goal). The necessary precedences, extracted over all 336 (s18/order_constraints.log), are exactly: 73 before {75,76,85,91,86}; 76 before {75,85,91,86}; 91 before 86 — and **72 and 74 are completely unconstrained** (they may occupy any position). So the requirement is NOT "beat 75"; it is the three-way relation pri(73) > pri(76) > {pri(75), pri(85)}.
- verdict: CONFIRMED (the order channel is OPEN — the recorded priority leg overstates the requirement and is falsified as written)

## [s18] H46 — with the correct constraint set, the walker's live length does not have to change AT ALL; the reachable region is a demotion of the three short-lived allocnos
- mechanism: global.c sorts by priority = floor_log2(nrefs)*nrefs/livelen*10000*size, so an order is reachable iff some (nrefs, livelen) assignment induces it. Live length is the source-controllable term (compute a value earlier / consume it later). Measured ALLOCDBG inputs (fresh extract from candidate.c THIS session, s18/model_candidate.json): 73 nrefs=24 livelen=62 pri=15483 | 75 nrefs=10 livelen=4 pri=75000 | 76 nrefs=26 livelen=22 pri=47272 | 85 nrefs=8 livelen=7 pri=34285 | fixed 72=29838, 74=26666, 91=333, 86=326.
- probe: tmp/grind/func_800324D0/s18/livelen_feasibility.py (6,447,060 live-length tuples land in the 336-order hit set), s18/walker_relief.py (rank by demand on the walker instead of total distance), s18/relief_curve.py (the Pareto curve: cheapest demotions for every walker live length 1..70).
- result: **At the MEASURED walker live length of 62 — the walker untouched — a target-reaching order exists**, requiring only livelen(75) >= 20 (from 4), livelen(76) >= 68 (from 22), livelen(85) >= 16 (from 7). The curve trades off smoothly: at l73=20 the requirement collapses to l75>=7, l76>=22 (UNCHANGED), l85>=7 (UNCHANGED) — i.e. shortening the walker to 20 needs a single +3 lengthening of pseudo 75. Full curve in s18/relief_curve.json / .log.
- verdict: CONFIRMED (a concrete, quantified, source-side lever map now exists where the record said "arithmetically dead")

## [s18] H47 — allocno live length IS source-order-controllable (the s16 blanket claim was generalised from one probe on the one allocno that cannot move)
- mechanism: s16's P1 moved the WALKER's load below the 11 pad stores, got a byte-identical RA model, and the record generalised that to "the live-length term is not source-order-controllable at all". The walker is loop-carried and pinned by construction, so it is the worst possible witness for a general claim.
- probe: hoisted `c = ptr[4]; ptr += 5;` above the 11 pad stores (s18/body_v1_hoist_c.c), fresh extract (s18/model_v1_hoist_c.json), compared against the candidate model extracted the same way this session.
- result: The models DIFFER. Pseudo 74's live length moved 9 -> 21 (priority 26666 -> 11428) and the allocation order changed from [75,76,85,72,74,73,91,86] to [75,76,85,72,73,74,91,86]. So an ordinary source-order edit does move live lengths and does move the order. (Pseudo 76 was unmoved at 22 by THIS particular edit, and the form is off-shape: sandbox 21, build_insns 67 != target 68 — banked at rejected/hoist-c-load-above-pad-stores-67insns.c.)
- verdict: CONFIRMED (the lever exists and is measurable; the blanket claim is retired — what remains is finding an edit that moves 75/76/85 by the amounts H46 quantifies while holding 68 insns)

## Frontier (for s19+ — REPLACES the s17 frontier, which rested on a leg that s18 falsified)
1. **The function is NOT foreclosed. Do not return owner-gated on decisions.md:17155.**
   s18 filed a correction entry against that record. The order channel is open (336/40320
   target-reaching orders), the walker needs no priority lift, and the constraint set is
   pri(73) > pri(76) > {pri(75), pri(85)} with 72/74 free.
2. **Attack the live lengths of pseudos 75, 76 and 85 — not the walker.** Targets from the
   relief curve (s18/relief_curve.json), pick any point on it: with the walker unchanged at 62,
   need livelen 75 >= 20 (from 4), 76 >= 68 (from 22), 85 >= 16 (from 7); each unit the walker
   comes down relaxes all three. Identity from the s5 chassis note: 75 = the cmd HEAD web (the
   staged loop-tail `cmd = *ptr`), 85 = the cmd ARM web (`cmd = c - 0x80`), 76 = the stream byte
   `c`, 73 = the walker `ptr`, 72 = `pad`, 74 = `val`. The lever is ordinary C: compute a value
   earlier / consume it later so its live range spans more insns, WITHOUT changing the 68-insn
   shape. Measure with `tools/ra_solver/extract.py func_800324D0 code6cac_b` (one run, no
   sandbox needed) and read allocdbg livelens BEFORE spending a sandbox measurement.
3. **Two model traps s18 hit — do not repeat them.** (a) `Sim.__init__` re-seeds
   `model["flow"]` from the ALLOCDBG rows, so editing `model["flow"]` before constructing a Sim
   is a silent NO-OP; use `Sim.simulate(overrides={pseudo: {"nrefs":..,"livelen":..}})` or
   `simulate(order=[...])`. (b) `Sim.flow` is keyed by INT, not str.

## [s18] The upstream CSE / allocno-set-shrink class — the one input class sitting above every leg of the filed foreclosure — can reach the target seats.
- mechanism: inverse_compose.py hypothesis --merge P,Q models cse.c unifying two materialisations: the survivor takes the SUM of references, the UNION of the live range and the UNION of the conflicts, and the exact find_reg forward model is replayed. Never run on this function in 17 prior sessions.
- probe: tmp/grind/func_800324D0/s18/merge_sweep.py (all 56 ordered pairs, via the tool) and s18/merge_depth2.py (all 2408 merge chains of depth <= 2, in-process merge_pseudos + Sim), goal {73:$v1, 75:$a2, 85:$a2} (s13, dispositions 8/8 MATCH).
- result: 56/56 negative at depth 1; 2408/2408 negative at depth <= 2 (875 of them faithful). ZERO chains reach the goal and ZERO put the walker (allocno 73) in $v1 at all.
- verdict: KILLED

## [s18] The filed foreclosure's priority leg ('lifting the walker above allocno 75 needs a 4.84x priority lift — arithmetically impossible in a shape fixed at 68 instructions') establishes that the target allocation is unreachable by ordering.
- mechanism: That leg reasons about a single pairwise race (73 vs 75) and never checks what the target disposition actually demands of the allocation ORDER, so it cannot support the general claim it is used for.
- probe: s18/order_closure.py — enumerate all 8! = 40320 permutations of the allocation order and replay the exact ra_solver find_reg model (Sim.simulate(order=...)) on each, everything else held at the model re-extracted THIS session from the candidate body (s18/model_candidate.json; sort order MATCH, dispositions 8/8).
- result: 336 of 40320 orders reach the FULL 8/8 target disposition; 6720 seat the walker in $v1; 2016 reach the 3-seat goal. Necessary precedences over all 336 (s18/order_constraints.log): 73 before {75,76,85,91,86}, 76 before {75,85,91,86}, 91 before 86, with 72 and 74 UNCONSTRAINED. The requirement is pri(73) > pri(76) > {pri(75), pri(85)} — not 'beat 75'.
- verdict: KILLED

## [s18] Reaching a target-producing order requires a large, source-impossible change to the loop-carried walker's live length.
- mechanism: global.c sorts allocnos by priority = floor_log2(nrefs)*nrefs/livelen*10000*size, so an order is reachable iff some (nrefs, livelen) assignment induces it, and live length is the source-controllable term. Measured ALLOCDBG inputs this session: 73(24,62) pri 15483 | 75(10,4) pri 75000 | 76(26,22) pri 47272 | 85(8,7) pri 34285 | fixed 72=29838, 74=26666, 91=333, 86=326.
- probe: s18/livelen_feasibility.py (6447060 live-length tuples landing in the 336-order hit set), s18/walker_relief.py (ranked by demand on the walker rather than total distance), s18/relief_curve.py (Pareto curve for walker live length 1..70).
- result: At the walker's MEASURED live length of 62 — walker untouched — a target-reaching order exists, needing only livelen(75) >= 20 (from 4), livelen(76) >= 68 (from 22), livelen(85) >= 16 (from 7). At walker livelen 20 the requirement collapses to livelen(75) >= 7 with 76 and 85 UNCHANGED. Full curve banked at s18/relief_curve.json.
- verdict: CONFIRMED

## [s18] Allocno live length is not source-order-controllable (the s16 blanket claim carried into the foreclosure record).
- mechanism: s16 generalised from one probe that moved the WALKER — loop-carried and pinned by construction, the worst possible witness for a claim about every allocno.
- probe: Hoisted `c = ptr[4]; ptr += 5;` above the 11 pad stores (s18/body_v1_hoist_c.c), ran a fresh tools/ra_solver/extract.py, and compared against the candidate model extracted the same way this session.
- result: The models DIFFER: pseudo 74's live length moves 9 -> 21 (priority 26666 -> 11428) and the allocation order changes from [75,76,85,72,74,73,91,86] to [75,76,85,72,73,74,91,86]. The form itself is off-shape (sandbox 21, build_insns 67 != target 68) and is banked as rejected.
- verdict: KILLED

## [s18] The chassis is still the 15-chassis on today's HEAD, so every chassis-relative conclusion in the ledger remains current.
- mechanism: sandbox scores against build/src/code6cac_b.o and never rebuilds it, so an absolute floor is only honest against a pristine reference object; s17 left that reference rebuilt from pristine HEAD and this session built nothing before measuring.
- probe: Applied memory/grind/func_800324D0/candidate.c to src/code6cac_b.c and ran `& tools/wteng.ps1 main sandbox func_800324D0 --disable all` (s18/sandbox_candidate.log).
- result: score 15, target_insns 68 == build_insns 68, rules_dropped 0.
- verdict: CONFIRMED

## [s19] The pseudo identity carried since s5 (74 = val, 76 = stream byte c, 75 = cmd head web, 85 = cmd arm web) is correct, so the s18 relief curve names the right source objects.
- mechanism: Every probe design since s5 has been aimed by that map; if it is wrong, the s18 frontier ("move the staged `cmd = *ptr` read earlier", "lengthen the stream byte 76 to livelen 68") aims at the wrong variables.
- probe: Generated the instrumented-cc1 dumps (`pwsh tools/grinder/dump.ps1 func_800324D0`) with the candidate body applied and read the `.lreg` segment `;; Function func_800324D0` insn by insn, attributing every pseudo to its defining and using RTL insns.
- result: THREE OF FIVE ARE WRONG. 74 = stream byte c (insn 61 `c = mem(ptr+4)`, insn 217 tail load, insn 70 loop test); 76 = operand byte val (insn 107 load, the twelve arm stores 117..183, "dies in 12 places"); 75 = the biased command (insn 104 `75 = 85 - 128`); 85 = the compiler's `zero_extend(c)` temp (insn 76), not any source variable. The staged tail read has NO allocno - it is collapsed into insn 217 before .lreg.
- verdict: KILLED

## [s19] The nrefs term (s18 frontier item 3, never swept) offers a cheaper route into the 336-order hit set than the live-length term.
- mechanism: priority = floor_log2(nrefs)*nrefs/livelen*10000*size makes nrefs a step function, so crossing a power of two moves priority discontinuously and might satisfy the constraint set far more cheaply than a 46-unit live-length change.
- probe: s19/joint_sweep.py - enumerate nrefs in [2,64] x livelen in [2,130] for each of 73/75/76/85, find the minimum total |dnrefs| + |dlivelen| satisfying pri(73) > pri(76) > {pri(75), pri(85)}; plus a pure-nrefs variant with all live lengths frozen.
- result: CONFIRMED in the abstract and USELESS in practice. Cheapest joint solution costs 25 units (76 nrefs 26->11, 75 nrefs 10->3, 85 nrefs 8->5, walker untouched) against 71 for the pure-live-length route. But this session also decoded the nrefs weighting (1 per out-of-loop reference, 2 per in-loop reference, verified exactly on 72 = 13 + 12x2 = 37 and 76 = 13x2 = 26), and in this function every reference of the constrained pseudos occupies an instruction of its own, so |dnrefs| converts 1:2 into instruction count and the 68-insn budget has zero slack.
- verdict: KILLED

## [s19] In-body hoisting (computing cmd and/or val earlier in the loop body) can deliver the live-length increases the s18 relief curve requires.
- mechanism: The relief curve at the measured walker live length of 62 needs livelen(75) >= 20 (from 4), livelen(76) >= 68 (from 22), livelen(85) >= 16 (from 7); moving a definition earlier in the body lengthens its live range without changing the instruction count.
- probe: Probes A (`cmd = c - 0x80` at the loop head), B (`val = *ptr` at the loop head) and C (both), each swapped into src/code6cac_b.c and measured with a fresh `tools/ra_solver/extract.py` (s19/model_A_cmd_early.json, model_B_val_early.json, model_C_both_early.json); A also sandboxed.
- result: The lever is real but roughly 2x too weak, and it is ANTI-CORRELATED across the two short webs. A: livelen(75) 4 -> 9 only (pri 75000 -> 33333, needs < 15483) while livelen(85) SHORTENS 7 -> 4, so pri(85) rises 34285 -> 60000 and 85 becomes the new head of the allocation order. B: livelen(76) 22 -> 26 (needs 68). C: both, same ceilings. A sandboxes 28 with build_insns 68 == 68.
- verdict: KILLED

## [s19] Head-test ORDER (`c < 0x80` before `c != 0xFF`) is a live-range lever on the zero-extend temp 85.
- mechanism: 85's live range runs from its definition (insn 76) to its last comparison use, so moving the last comparison later should lengthen it.
- probe: Probe F - the two head tests swapped, fresh extract (s19/model_F_test_reorder.json).
- result: The model is BIT-IDENTICAL to the baseline in every field (order, nrefs, livelen, pri for all eight allocnos). jump.c normalises the comparison order before flow computes liveness.
- verdict: KILLED

## [s19] Staging a value across the loop back edge is a strong enough lever to invert the walker-vs-cmd priority race that s1 called arithmetically dead.
- mechanism: A value computed in the preheader and re-computed at the loop latch is live over the whole loop body, which lengthens its live range and (because the in-loop definition replaces several in-loop uses) can lower its weighted reference count; priority falls on both terms at once.
- probe: Probe D (val staged), probe H (`cmd = c - 0x80` staged), probe I (both), each extracted and sandboxed (s19/model_*.json, s19/sandbox_*.log).
- result: CONFIRMED for the biased command and REFUTED for the operand byte. H: pseudo 75 nrefs 10 -> 7, livelen 4 -> 15, pri 75000 -> 9333, i.e. BELOW the walker's 14769 - the first measured spelling in which the walker is allocated before the cmd web, and 75 lands in its target seat $a2; allocno 85 disappears entirely. D: pseudo 76 reaches only livelen 31 (pri 34838) against the required 68, because val dies in 12 places (one per switch arm) so staging can only add the head+dispatch prefix, never the arm bodies. Cost: H is 69 insns, I is 70, both off-shape.
- verdict: CONFIRMED

## [s19] The instruction that back-edge staging costs can be paid for elsewhere in the body, giving a 68-insn spelling of the loop-carried cmd that keeps H's demotion.
- mechanism: H is exactly one instruction over budget (69 vs 68). Two candidate payments: widen `c` to u32 (s3's merged-c-cmd measured the u8->u32 promotion at -1 insn), or re-express the two head tests on the biased value so the zero-extend and one compare disappear.
- probe: Probe K (H with `u32 c`) and probe L (H with `cmd == 0x7F` / `cmd > 0x7F` head tests - exact unsigned algebra), both sandboxed; L also extracted (s19/sandbox_K_cmd_staged_u32c.log, s19/sandbox_L_staged_tests_on_cmd.log, s19/model_L_staged_tests_on_cmd.json).
- result: The 68-insn spelling EXISTS - L is build_insns 68 == target_insns 68, rules_dropped 0, score 33 - but it does not keep H's win. The two head tests add two in-loop references to cmd (nrefs 7 -> 11), and pri(75) goes back up to 23571, above the walker's 15000; the allocno set drops to seven and the order is [76,72,75,74,73,89,86]. K overshoots the other way: `u32 c` removes THREE instructions from the H shape (66 != 68), not the one s3 measured on the unstaged chassis.
- verdict: KILLED for these two payments; the underlying claim stays open - see the s20 frontier.

## [s19] Back-edge damping: any source edit that lengthens a non-walker allocno's live range also lengthens the walker's, so pairwise priority ratios move far less than single-allocno arithmetic suggests.
- mechanism: The walker is live over the entire loop body, so it accumulates every instruction added to or stretched inside that body.
- probe: Compare s19/model_D_val_staged.json and s19/model_I_cmd_val_staged.json against s19/model_base.json.
- result: CONFIRMED. Staging val moved 76 from livelen 22 to 31 (+9) and simultaneously moved 73 from 62 to 63 and 72 from 62 to 63. Probe I (two staging edits) moved the walker to livelen 66. Every demotion lever partially demotes the walker with it.
- verdict: CONFIRMED

## [s19] The pseudo identity carried since s5 (74 = val, 76 = stream byte c, 75 = cmd head web, 85 = cmd arm web) is correct, so the s18 relief curve names the right source objects.
- mechanism: Every probe design since s5 has been aimed by that map; if it is wrong, the s18 frontier ('move the staged cmd = *ptr read earlier', 'lengthen the stream byte 76') aims at the wrong variables.
- probe: pwsh tools/grinder/dump.ps1 func_800324D0 with the candidate body applied, then read the .lreg segment ';; Function func_800324D0' insn by insn and attribute every pseudo to its defining and using RTL insns.
- result: THREE OF FIVE WRONG. 74 = stream byte c (insn 61 c = mem(ptr+4), tail insn 217, loop test insn 70); 76 = operand byte val (insn 107 load + the twelve arm stores 117..183, 'dies in 12 places'); 75 = the biased command (insn 104, 75 = 85 - 128); 85 = the compiler's zero_extend(c) temp (insn 76), not a source variable at all. 72 = pad and 73 = walker are correct. The staged tail read owns NO allocno - combine collapses it into insn 217 before .lreg.
- verdict: KILLED

## [s19] The nrefs term (s18 frontier item 3, never swept) offers a cheaper route into the 336-order hit set than the live-length term.
- mechanism: priority = floor_log2(nrefs)*nrefs/livelen*10000*size makes nrefs a step function, so crossing a power of two moves priority discontinuously.
- probe: tmp/grind/func_800324D0/s19/joint_sweep.py - nrefs in [2,64] x livelen in [2,130] for each of 73/75/76/85, minimising total |dnrefs|+|dlivelen| subject to pri(73) > pri(76) > {pri(75), pri(85)}; plus a pure-nrefs variant with live lengths frozen.
- result: Cheapest joint solution costs 25 units (76 nrefs 26->11, 75 nrefs 10->3, 85 nrefs 8->5, walker untouched) vs 71 for the pure-live-length route - ~3x cheaper in the abstract. Unusable in practice: the same session decoded the nrefs weighting (1 per out-of-loop reference, 2 per in-loop reference, verified exactly on 72 = 13 + 12x2 = 37 and 76 = 13x2 = 26), and every reference of these pseudos occupies an instruction of its own, so |dnrefs| converts 1:2 into instruction count against a budget with zero slack.
- verdict: KILLED

## [s19] In-body hoisting (computing cmd and/or val earlier in the loop body) can deliver the live-length increases the s18 relief curve requires.
- mechanism: Moving a definition earlier lengthens its live range without changing the instruction count; the curve needs livelen(75) >= 20, livelen(76) >= 68, livelen(85) >= 16 at the measured walker live length of 62.
- probe: Probes A (cmd at loop head), B (val at loop head), C (both), each swapped into src/code6cac_b.c and measured with a fresh tools/ra_solver/extract.py; A also sandboxed.
- result: Roughly 2x too weak AND anti-correlated. A: livelen(75) 4 -> 9 only (pri 75000 -> 33333) while livelen(85) SHORTENS 7 -> 4, so pri(85) rises 34285 -> 60000 and 85 becomes the new order head. B: livelen(76) 22 -> 26. C: same ceilings. A sandboxes 28 at 68 == 68. No in-body hoist demotes both short webs, because 75 and 85 are chained (insn 76 zero_extend -> insn 104 subtract).
- verdict: KILLED

## [s19] Head-test ORDER (c < 0x80 before c != 0xFF) is a live-range lever on the zero-extend temp 85.
- mechanism: 85 lives from its definition to its last comparison use, so moving that use later should lengthen it.
- probe: Probe F - the two head tests swapped, fresh extract (s19/model_F_test_reorder.json vs s19/model_base.json).
- result: The model is BIT-IDENTICAL in every field - order [75,76,85,72,74,73,...], nrefs/livelen/pri 75(10,4,75000) 76(26,22,47272) 85(8,7,34285) 73(24,62,15483). jump.c normalises comparison order before flow computes liveness. Not sandboxed: an RA-identical, insn-identical form cannot move the score.
- verdict: KILLED

## [s19] Staging a value across the loop back edge is a strong enough lever to invert the walker-vs-cmd priority race that s1 called arithmetically dead (a 4.84x lift).
- mechanism: A value computed in the preheader and re-computed at the latch is live over the whole loop body, lengthening its live range while its in-loop definition replaces in-loop uses - priority falls on both terms of floor_log2(nrefs)*nrefs/livelen at once.
- probe: Probe D (val staged), probe H (cmd = c - 0x80 staged), probe I (both) - each extracted with tools/ra_solver/extract.py and sandboxed.
- result: CONFIRMED for the biased command, refuted for the operand byte. H: pseudo 75 nrefs 10 -> 7, livelen 4 -> 15, pri 75000 -> 9333, BELOW the walker's 14769 - the first spelling in 19 sessions where the walker is allocated before the cmd web, with 75 landing in its target seat $a2 and allocno 85 disappearing entirely. D: pseudo 76 reaches only livelen 31 (pri 34838) against the required 68, because val dies in 12 places so staging can only add the head+dispatch prefix, never the arm bodies. Shape cost: H = 69 insns, I = 70, D = 68 (score 19).
- verdict: CONFIRMED

## [s19] H's one-instruction cost can be paid elsewhere in the body, giving a 68-insn spelling of the loop-carried cmd that keeps the demotion.
- mechanism: Two candidate payments: widen c to u32 (s3 measured the u8->u32 promotion at -1 insn), or re-express the head tests on the biased value so the zero-extend and a compare disappear.
- probe: Probe K (H with u32 c) and probe L (H with cmd == 0x7F / cmd > 0x7F head tests, exact unsigned algebra), both sandboxed; L also extracted.
- result: The 68-insn spelling EXISTS - L is build_insns 68 == target_insns 68, rules_dropped 0, score 33 - but does not keep the win: the head tests add two in-loop references to cmd (nrefs 7 -> 11), pri(75) returns to 23571 above the walker's 15000, and the allocno set drops to seven (order [76,72,75,74,73,89,86]). K overshoots the other way: u32 c removes THREE instructions from the H shape (66 != 68). The instruction H spends and the references H saves are the same resource.
- verdict: KILLED

## [s19] Back-edge damping: any source edit that lengthens a non-walker allocno's live range also lengthens the walker's, so pairwise priority ratios move far less than single-allocno arithmetic predicts.
- mechanism: The walker is live over the entire loop body, so it accumulates every instruction added to or stretched inside that body.
- probe: Compare s19/model_D_val_staged.json and s19/model_I_cmd_val_staged.json against s19/model_base.json.
- result: CONFIRMED. Staging val moved 76 from livelen 22 to 31 and simultaneously moved 73 from 62 to 63 and 72 from 62 to 63; probe I moved the walker to livelen 66. Every demotion lever partially demotes the walker with it - this is why 18 sessions of pairwise arithmetic looked flat.
- verdict: CONFIRMED

## [s20] The single-store switch shape (arms select a destination, one store after the switch) demotes the operand byte below the walker and opens the target allocation order.
- mechanism: pri = floor_log2(nrefs)*nrefs*size/live_length*10000 (GCC 2.7.2 global.c, mirrored at tools/ra_solver/simulate.py:43), and in-loop references are weighted x2 by loop depth. val's 26 weighted refs are the 12 arm stores (24) plus its load (2). Replacing the twelve `pad[X] = val` stores with twelve destination assignments plus a single `pad[off] = val` / `*dst = val` after the switch collapses val to nrefs 4, which should drop it from pri ~47000 to ~2200, far below the walker's ~15000.
- probe: Two bodies built and measured this session - M (arms assign a u32 offset, `pad[off] = val` after the switch) and N (arms assign a `u8 *dst`, `*dst = val` after the switch). Sandboxed (tmp/grind/func_800324D0/s20/sandbox_M_off_single_store.log, sandbox_N_dst_single_store.log) and extracted with the instrumented cc1 via tools/ra_solver/extract.py (s20/model_M.json, s20/model_N.json).
- result: The demotion of val is REAL and exactly as predicted - pseudo 76 falls to nrefs 4, livelen 36/35, pri 2222/2285, a 21x demotion and the largest single-allocno demotion ever measured on this function - and it is WORTHLESS. The twelve arm references do not disappear, they move: a new pseudo 77 (the offset / destination pointer) appears carrying nrefs 28, livelen 28, pri 40000, and it takes $v1 in exactly the seat val used to occupy. Disposition 73 -> 6 in both, identical to base. Cost: M = 71 insns score 34, N = 70 insns score 33 (target 68).
- verdict: KILLED (bodies banked: rejected/switch-single-store-offset-71insns.c, rejected/switch-single-store-dstptr-70insns.c)

## [s20] The 12-arm jump-table switch imposes a conservation law on allocno priority that closes the find_reg priority channel for every spelling: some pseudo always carries >= 26 weighted references inside a live range of <= 34 instructions, so its priority is always at least 2.5x the walker's structural ceiling.
- mechanism: The walker (pseudo 73) is live-in and live-out of every basic block of the loop, so reg_live_length(73) >= 62 in every body ever measured and its reference count has been 24 or 25 in all thirteen; its priority ceiling is floor_log2(24)*24*10000/62 = 15873. The twelve switch arms each reference the stored value once inside the loop (weight 2), so whichever pseudo the arms name carries >= 24 weighted refs plus its own definition, and that pseudo is born in the payload arm and dies at the last arm store, bounding its live length by the dispatch+arm region. To fall below 15873 it would need live_length > 65.5 at nrefs 26, or > 70.6 at nrefs 28 (the M/N destination pseudo).
- probe: tmp/grind/func_800324D0/s20/priority_bound.py and s20/simultaneity.py over all thirteen instrumented-cc1 models measured to date (s19 A,B,C,D,E,F,G,H,I,L,base + s20 M,N). Outputs: s20/priority_bound.json/.log, s20/simultaneity.json/.log.
- result: CONFIRMED with no exception in thirteen bodies. The walker is never first: measured walker priority 14769-15873; the arm-carrier pseudo (76 in eleven bodies, 77 in M and N) measured 31764-49523 at live length 21-34, and would need live length 67.2-75.8 to fall below the walker. Its observed ceiling is 34 (probe I) - short by a factor of two, and that 34 was bought by staging BOTH the command and the operand across the back edge at a cost of two instructions. The other direction is worse: lifting the walker above the lowest carrier priority ever measured (40000) at live length 62 needs nrefs 50 (pri 40322), i.e. +26 weighted references = +13 RTL references = at least +6 instructions on a budget that is exact at 68. Every s19 demotion lever (staging, hoisting, test re-expression) moves the other short webs (75, 85/86) and leaves the arm carrier untouched, which is why probe H fixes 75 (75000 -> 9333) and is still blocked by 76 (49523 vs 14769).
- verdict: CONFIRMED

## [s20] The -1-instruction levers already banked (c-load hoisted above the pad stores, 67 insns; u32 c) can pay for probe H's one extra instruction, giving a 68-insn spelling of the loop-carried biased command.
- mechanism: H costs exactly +1 (69 vs 68). Two independent -1 edits exist that do not touch cmd's reference set - hoisting `c = ptr[4]; ptr += 5;` above the eleven pad stores (rejected/hoist-c-load-above-pad-stores-67insns.c), and widening c to u32. Either should net 68 while preserving H's demotion of pseudo 75.
- probe: Body P1 (hoist + H) sandboxed at s20/sandbox_P1_hoist_plus_H.log; body Q (base + u32 c) sandboxed at s20/sandbox_Q_base_u32c.log.
- result: KILLED - the savings are not additive with H. P1 measures 69 insns / score 31: the hoist's -1 evaporates once the biased command is computed in the preheader, because both edits compete for the same preheader scheduling slot. Q confirms the widening is worth only -1 on the base chassis (67 insns, score 29) while it is worth -3 on the H chassis (s19 probe K, 66 insns) - the levers interact, and no combination lands on 68 with H's allocation.
- verdict: KILLED

## [s20] The single-store switch shape (each arm selects a destination, one store after the switch) demotes the operand byte below the walker and opens the target allocation order.
- mechanism: GCC 2.7.2 global.c priority is floor_log2(nrefs)*nrefs*size/reg_live_length*10000 (mirrored at tools/ra_solver/simulate.py:43) with in-loop references weighted x2 by loop depth. val's 26 weighted refs are the 12 arm stores (24) plus its load (2); collapsing the twelve stores into one should drop val to nrefs 4 and pri ~2200, far below the walker's ~15000.
- probe: Bodies M (arms assign a u32 offset, then pad[off] = val) and N (arms assign a u8 *dst, then *dst = val) built, sandboxed (s20/sandbox_M_off_single_store.log, s20/sandbox_N_dst_single_store.log) and extracted through the instrumented cc1 with tools/ra_solver/extract.py (s20/model_M.json, s20/model_N.json).
- result: The demotion is real and exact - pseudo 76 falls to nrefs 4, livelen 36/35, pri 2222/2285, a 21x demotion, the largest single-allocno demotion ever measured on this function - and it changes nothing: a new pseudo 77 (the offset / destination pointer) appears with nrefs 28, livelen 28, pri 40000 and takes $v1 in the same seat. Walker disposition 73 -> 6 in both, identical to base. Cost: M = 71 insns score 34, N = 70 insns score 33 against target 68.
- verdict: KILLED

## [s20] The 12-arm jump-table switch imposes a conservation law on allocno priority that closes the find_reg priority channel for every spelling: some pseudo always carries >=26 weighted references inside a live range of <=34 instructions, so its priority is always at least 2.5x the walker's structural ceiling.
- mechanism: The walker (pseudo 73) is live-in and live-out of every block of the loop, so reg_live_length(73) >= 62 and nrefs(73) has been 24 or 25 in all thirteen measured bodies; its priority ceiling is floor_log2(24)*24*10000/62 = 15873. The twelve arms each reference the stored value once inside the loop (weight 2), so whichever pseudo they name carries >= 24 weighted refs plus its definition, and it is born in the payload arm and dies at the last arm store, which bounds its live length by the dispatch+arm region.
- probe: tmp/grind/func_800324D0/s20/priority_bound.py and s20/simultaneity.py over all thirteen instrumented-cc1 ALLOCDBG models to date (s19 A,B,C,D,E,F,G,H,I,L,base + s20 M,N).
- result: CONFIRMED with no exception in thirteen bodies. Walker priority measured 14769-15873; the arm carrier (pseudo 76 in eleven bodies, 77 in M and N) measured 31764-49523 at live length 21-34, and would need live length 67.2 (nrefs 26) to 75.8 (nrefs 28) to fall below the walker - its observed ceiling is 34 (probe I, bought with two extra instructions). The other direction needs walker nrefs 50 to beat the lowest carrier priority ever measured (40000) at live length 62, i.e. +26 weighted = +13 RTL references = at least +6 instructions on a budget that is exact at 68. This bounds the s18 enumeration: its 336 target-reaching orders are abstract permutations, but the order is the descending sort of a function of (nrefs, livelen) whose measured reachable domain excludes all of them.
- verdict: CONFIRMED

## [s20] The banked -1-instruction levers (c-load hoisted above the pad stores, 67 insns; u32 c) can pay for probe H's one extra instruction, yielding a 68-insn spelling of the loop-carried biased command that keeps H's demotion of pseudo 75.
- mechanism: H costs exactly +1 (69 vs 68) and both -1 edits leave cmd's reference set untouched, so either should net 68.
- probe: Body P1 (hoist + H) and body Q (base + u32 c) sandboxed: s20/sandbox_P1_hoist_plus_H.log, s20/sandbox_Q_base_u32c.log.
- result: KILLED - not additive. P1 measures 69 insns / score 31: the hoist's -1 evaporates once the biased command is computed in the preheader, because both edits compete for the same preheader slot. Q shows the widening is worth -1 on the base chassis (67, score 29) but -3 on the H chassis (s19 probe K, 66). The levers interact and nothing lands on 68 with H's allocation.
- verdict: KILLED

## [s21] 2026-09-01 — rederive

**H41 — KILLED.** *A spelling exists that splits the twelve arm references
across TWO carriers of <= 15 weighted refs each while preserving a single
12-entry jump table and 68 instructions* (the s20 frontier's only unmeasured
axis). Probe: build the nested-dispatch shape (outer `if (cmd < 6)` over two
6-arm switches), sandbox it, extract the RA model.
**Result:** score 28, build_insns **79** vs target 68
(`s21/sandbox_A_nested_dispatch.log`), and pseudo 76 stays at **nrefs 26** —
identical to the base body — with only livelen moving 22 → 30
(`s21/model_A_nested_dispatch.json`). **Mechanism of the kill:** `reg_n_refs`
counts references to a C VARIABLE, not to a dispatch region; twelve
`pad[X] = val;` arms are twelve references to `val` regardless of how many jump
tables sit above them. Splitting the carrier therefore requires two C variables
holding the same loaded byte, which is the BANNED base/ff overlapping-live-range
family respelled and has no semantic reading. The axis is closed, not just this
spelling.

**H42 — KILLED (and it supersedes the s20 framing).** *The binding requirement
is a competition between the walker and the arm carrier alone.* Probe:
re-derive find_reg's outcome from this body's own ALLOCDBG + conflict/pref sets
rather than from the s18 abstract order enumeration.
**Result:** pad (72) and c (74) are NOT competitors — 72 carries `prefs=[4]` and
takes `$a0` from any position, and 74 is the only allocno without a hard
conflict on reg 2, so `$v0` is reserved for it by exclusion. Hand-simulating the
order `[72,74,73,76,85,75,91,86]` against the model's conflict sets yields the
**FULL 8/8 target disposition**. The residual is therefore exactly three
demotions below the walker's invariant pri 15483: `livelen(75) >= 20` (from 4),
`livelen(85) >= 16` (from 7), `livelen(76) >= 68` (from 22). This confirms
s18's relief curve from an independent direction and REMOVES pad and c from
the constraint set.

**H43 — KILLED with a measured ceiling.** *`livelen(76)` can be lifted to the
required 68 by an honest C shape at 68 instructions.* Probe: hoist the operand
read out of the payload branch to the top of the loop body (probe D), the
shape that maximises val's live range without a preheader definition; also read
livelen(76) off every model built for this function.
**Result:** D gives livelen(76) = 26 at 68 insns, score 16
(`s21/sandbox_D_val_hoisted_head.log`, `s21/model_D_val_hoisted_head.json`) —
and it is semantically wrong anyway (it reads the operand byte on the 0xFF and
short-command paths, which the target bytes prove the original does not do).
Ceiling across all NINETEEN bodies ever built: **30** (probe A, +11 insns).
Required: **68**, which is longer than the entire loop (62) and therefore
demands that `val` be live across the back edge with a preheader definition.
No such definition has a semantic reading — every arm consumes the operand byte
on the next instruction — so it is a dead store (cheat family) AND costs an
instruction the exact-68 budget does not have.

**H44 — KILLED.** *Loop syntax or head-branch topology is a lever on the allocno
census.* Probes: `for` form with the stream advance in the third clause (C);
head tests re-nested with the payload test outermost (B).
**Result:** C is 15/68 with a **bit-identical** RA model to the while form
(same order, same nrefs/livelen/pri for all eight allocnos); B is 17/69, also
bit-identical in the model. Both banked in `rejected/`. Syntax-level loop and
branch restructuring is measured inert for this function.

**H45 — KILLED by record (rederive transplant leg).** *A Kengo or corpus
transplant can supply a different source shape.* There is no Kengo C anywhere:
`docs/grind/decisions.md:5937-5939` records `Kengo/` as names + sizes only,
"never a source shape to transplant"; the `kengo:HIGH | is_pad/Pad_Prs` comment
above the function is a naming attribution. m2c refuses the function without
the jump table and would in any case only reproduce the shape already read
insn-by-insn from `asm/funcs/func_800324D0.s` and already built at 68/68.

### Frontier after s21
The s20 frontier is now empty: item 1 (two-carrier split) is H41-KILLED,
item 2 (walker live-length reduction) is subsumed — the walker's (nrefs 24,
livelen 62-71, pri 13521-15483) triple is invariant across all nineteen bodies
and the requirement has been restated as three demotions that do not involve
it — and item 3 was already answered a priori. What remains, stated as the
single open question a future session would have to answer:

- **Is there a 68-instruction C body in which `val`'s live range crosses the
  loop back edge without a dead preheader store?** Everything else in the
  residual is bought: s19's probe H buys `livelen(75) >= 20` for +1 instruction
  and makes 85 vanish. If some shape pays for H's instruction AND puts a real,
  consumed value in `val` before the loop, the disposition closes. Nineteen
  bodies say the second half has no semantic reading; that is an argument, not
  a measurement, and it is the only place left to look.

## [s21] A spelling exists that splits the twelve arm references across TWO carriers of <= 15 weighted refs each while preserving a single 12-entry jump table and 68 instructions (the s20 frontier's only unmeasured axis).
- mechanism: At nrefs 15 the floor_log2 tier drops to 3 and the demotion requirement collapses to live_length > 28.4, which is inside the measured 21-34 envelope. Probe: nested dispatch - an outer `if (cmd < 6)` over two 6-arm switches - as the strongest available approximation (it splits the DISPATCH).
- probe: Built tmp/grind/func_800324D0/s21/body_A_nested_dispatch.c, sandboxed it, and extracted the RA model with tools/ra_solver/extract.py.
- result: score 28, build_insns 79 vs target 68 (s21/sandbox_A_nested_dispatch.log). Decisively: pseudo 76 stays at nrefs 26 - byte-for-byte the base body's count - with only livelen moving 22 -> 30 (pri 47272 -> 34666); a NEW allocno 97 appears for the half-select temp (nrefs 6, livelen 4, pri 30000). reg_n_refs counts references to a C VARIABLE, not to a dispatch region, so twelve `pad[X] = val;` arms are twelve references to `val` however many jump tables sit above them. Two carriers would require two C variables holding the same loaded byte - the BANNED base/ff overlapping-live-range family respelled, with no semantic reading.
- verdict: KILLED

## [s21] The binding competition for $v1 involves pad (72) and c (74) as blockers that must also be demoted, as earlier sessions assumed.
- mechanism: Re-derive find_reg's outcome directly from this body's own ALLOCDBG plus the model's conflict/pref/hard_conflict sets, instead of from s18's abstract 40320-order enumeration.
- probe: Hand-simulated find_reg over the order [72,74,73,76,85,75,91,86] against s19/model_base.json conflicts, prefs and hard_conflicts.
- result: FALSE, and the requirement shrinks. pad (72) carries prefs=[4] and takes $a0 from ANY position in the order, so it never competes for $v1; c (74) is the only allocno without a hard conflict on reg 2 (all of 72,73,75,76,85,86,91 list hard_conflicts [2,29]; 74 lists only [29]), so $v0 is reserved for it by exclusion and its position is free. The simulated order yields 72->$a0, 74->$v0, 73->$v1, 76->$a1, 85->$a2, 75->$a2, 91->$a3, 86->$t0 - the FULL 8/8 target disposition. The residual therefore reduces to exactly three demotions below the walker's invariant pri 15483: livelen(75) >= 20 (from 4), livelen(85) >= 16 (from 7), livelen(76) >= 68 (from 22). This reproduces s18's relief curve from an independent direction and REMOVES pad and c from the constraint set.
- verdict: KILLED

## [s21] reg_live_length(76) can be lifted to the required 68 by an honest C shape at 68 instructions.
- mechanism: 76's live range starts at the operand load inside the payload branch and ends at the last arm store. Hoisting the load to the top of the loop body is the shape that maximises the range without inventing a preheader definition.
- probe: Built s21/body_D_val_hoisted_head.c (operand read hoisted out of the payload branch), sandboxed it, extracted its model; and read livelen(76) off every model built for this function to date.
- result: D gives livelen(76) = 26 at 68 insns, score 16 (s21/sandbox_D_val_hoisted_head.log, s21/model_D_val_hoisted_head.json) - and it is semantically wrong anyway, reading the operand byte on the 0xFF and short-command paths the target bytes prove the original does not read. Ceiling across all NINETEEN bodies ever built for this function: 30 (probe A, +11 insns). Required: 68, which is LONGER THAN THE ENTIRE LOOP (62), so `val` would have to be live across the back edge with a preheader definition. No such definition has a semantic reading - every arm consumes the operand byte on the next instruction - so it is a dead store (cheat family) AND costs an instruction the exact-68 budget does not have.
- verdict: KILLED

## [s21] Loop syntax or head-branch topology is a lever on the allocno census.
- mechanism: A `for` form with the stream advance in the third clause changes where the increment is emitted; re-nesting the head tests (payload test outermost) changes the CFG shape, distinct from s19's probe F order-swap which jump.c normalised.
- probe: Built and measured s21/body_C_for_form.c and s21/body_B_nested_head.c; extracted RA models for both.
- result: C: score 15, build_insns 68, and a BIT-IDENTICAL RA model to the while form (same order [75,76,85,72,74,73,91,86], same nrefs/livelen/pri for all eight allocnos). B: score 17, build_insns 69, RA model also bit-identical to base - the extra instruction buys literally nothing. Syntax-level loop and branch restructuring is measured inert for this function.
- verdict: KILLED

## [s21] The rederive modality's transplant leg (Kengo / m2c / decomp.me corpus) can supply a structurally different source shape.
- mechanism: The function carries a `kengo:HIGH | is_pad/Pad_Prs` tag in src/code6cac_b.c, which has invited speculation about a Kengo transplant for several sessions.
- probe: Checked the repo record for Kengo C availability; ran m2c on asm/funcs/func_800324D0.s.
- result: docs/grind/decisions.md:5937-5939 records that `Kengo/` holds names + sizes + source paths ONLY - 'There is no Kengo C ... never a source shape to transplant' - so the tag is a naming attribution and carries no shape. m2c refuses the function without the jump table ('Found jr instruction ... but the corresponding jump table is not provided') and would in any case only reproduce the shape already read insn-by-insn from the target asm and already built at 68/68. The decomp.me corpus leg is non-applicable: the residual is a register-seat swap inside a body that already matches instruction-for-instruction.
- verdict: KILLED

## [s22] H55: the walker's allocno priority can be lifted to the target-reaching rank at ZERO instruction cost, because cross-jump-remerged references are free
- mechanism: flow.c's reg_n_refs census counts references on the pre-RA RTL; jump2's cross-jump pass runs AFTER reload. Duplicating the loop tail (`c = *ptr; ptr++;` + back-transfer) into all twelve command arms therefore multiplies the walker pseudo's reference count without emitting a single instruction. s20's foreclosure leg (b) — "promoting the walker needs nrefs ~50, which costs >= +6 instructions" — assumed refs cost instructions and is thereby falsified.
- probe: three bodies measured against a pristine-HEAD reference object. P1 = 12-arm duplication WITH the s5 staged tail; P2 = same without staging; P3 = same, `u32 cmd` local dropped, `switch (c - 0x80)`.
- result: P1 = 27 (68/68), P2 = **0** (68/68), P3 = **0** (68/68, rules_dropped 0), full build SHA1 == oracle. Walker pseudo 73's reg_n_refs is 96 in the winning body vs 24 in the base body, while the operand carrier stays at 26 in both (s22/lreg_seg.txt, s22/lreg_seg_p3.txt) — so the walker overtakes the carrier in allocno_compare and allocation lands 8/8 on target.
- verdict: CONFIRMED — **the function is MATCHED in pure C.**

## [s22] H56: the s5 staged-value-reused-variable FAKE is antagonistic with the walker reference lift
- mechanism: staging the tail read through the existing `cmd` variable moves the duplicated references onto the command web instead of the stream walker, so the walker still loses the $v1 seat.
- probe: P1 vs P2 above — identical bodies except for the staged tail.
- result: 27 with staging, 0 without. The staged read (load-bearing since s5, worth 27 -> 15 on the shared-tail chassis) must be DELETED for the duplication to work.
- verdict: CONFIRMED. Banked as rejected/tail-dup-12arms-with-staged-read-27.c.

## [s22] H57: nrefs(operand carrier) cannot be reduced below twelve (s21 frontier axis 2)
- mechanism: reg_n_refs counts references to a C variable; the target mandates twelve `sb $a1` arms, each of which is a reference to the operand byte.
- probe: reference census on the s21 body's .lreg RTL segment (s22/lreg_seg.txt).
- result: pseudo 76 has exactly 13 references — 1 def (insn 107) + the twelve arm stores at 0xA1,0xA3,0xA7,0xA8,0xA9,0xA5,0xA6,0xA2,0xA4,0xAA,0xAB,0xAC. No spelling reaches fewer.
- verdict: KILLED (axis closed in closed form; moot now that H55 closed the function from the other side).

## [s22] H58: the original had extra incoming parameters whose entry live ranges reshuffled allocation
- mechanism: params in $a1/$a2/$a3 carry hard-reg preferences and are live at function entry, which would change find_reg's conflict and preference sets without changing the body's instruction stream.
- probe: read the only caller's bytes, asm/funcs/func_80021A98.s:96-99.
- result: the caller sets `$a0` alone before `jal func_800324D0` (`addu $a0, $s0, $zero` in the delay slot); no $a1/$a2/$a3 setup exists. The signature is one pointer parameter.
- verdict: KILLED.

## [s22] H59: a fresh m2c derivation of the target yields a shape not yet tried
- mechanism: rederive modality's first leg — decompile asm/funcs/func_800324D0.s together with asm/rodata/jtbl_800105A0.s and read the shape m2c reconstructs.
- probe: `python3 tools/m2c/m2c.py --target mipsel-gcc-c --valid-syntax -f func_800324D0 asm/funcs/func_800324D0.s asm/rodata/jtbl_800105A0.s`.
- result: same control flow as the existing candidate, with the -0x80 bias folded into `case 0x80..0x8B` and the walker rendered as two variables (`var_v1`/`var_v1_2`). Both already banked kills (switch-folded-subtract-0x80-cases.c; q-alias-copy.c — cse1 canon_reg coalesces the copy pre-RA). m2c cannot express the winning shape: it never duplicates a shared tail into arms.
- verdict: KILLED as a source of new shape (the leg is closed, not the function).

### [s22 re-run] H43 — the s22 match survives a cold re-application to a pristine src
- statement: the 12-arm tail-duplication body is a genuine byte match, not an artifact of the discarded session's tree state.
- mechanism: the driver reverted the discarded session's src edits, so the body could be re-applied to the pre-migration pin-carrying src and measured from cold.
- probe: `s22/apply.py` onto pristine `src/code6cac_b.c`, then `sandbox func_800324D0 --disable all`, then full `build`.
- result: score 0, 68/68, rules_dropped 0 (`s22/sandbox_verify.log`); full build SHA1 == 62efab4f73f992798c43e8c730aa43baa10bb4fa (`s22/build_sha1_verify.log`). Repeated after the annotation correction: `s22/sandbox_final_s22b.log`, `s22/build_sha1_final_s22b.log`.
- verdict: CONFIRMED.

### [s22 re-run] H44 — the reference lift is carrier-neutral
- statement: the duplication promotes the walker WITHOUT perturbing the operand carrier, so allocno_compare's flip is attributable to one term only.
- mechanism: flow.c's reg_n_refs census is per-pseudo; duplicating the tail touches only the walker's and command byte's references, not the operand byte's.
- probe: compare the `.lreg` register census lines between `s22/lreg_seg.txt` (base) and `s22/lreg_seg_p3.txt` (winning body).
- result: walker pseudo 73 goes 24 -> 96; the operand carrier is 26 in BOTH (pseudo 76 base, renumbered 75 in the winning body). The s20 "demote the carrier" channel is untouched and remains closed — the match came entirely from the promote leg s20 had priced out.
- verdict: CONFIRMED.
