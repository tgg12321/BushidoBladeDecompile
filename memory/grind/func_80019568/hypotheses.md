# Hypothesis ledger — func_80019568

## [s1] 2026-08-25 — recon

H1 (CONFIRMED, mechanism-level): the committed rule-era body's structure (register pins,
explicit walking pointers, pointer-alias tail) is NOT the original shape. Fresh indexed
derivation (v2) reaches 34 honest vs the chassis's 42, with build insn count == target.

H2 (CONFIRMED): loop 1's target shape is INDEXED accesses over one counter i —
`sp.output[i]`, `sp.output[i+2]`, `packets[i*8+k]` — giving loop.c combined givs whose
reduced walkers match target a2/a3 exactly, and preserving the sh->lhu store-reload
(pointer-deref spelling folds the reload; indexed spelling does not). Probe: v1 (49) vs
v2 (34) + diff. 

H3 (CONFIRMED): the single dominant residual mechanism at v2 is loop.c biv elimination
of counter i (final-value path + compare rewrite via an always-computable out-class giv).
Blocking it (diagnostic `while (i+1<3)`) drops 34 -> 28 and aligns the entire register
file (i->t0, mask->t1, const4->t2, jtbl->t3, loop-2 counter->t0). Probe: dumps + score.
The diagnostic spelling itself is NOT submittable (2 extra insns; coercion-flavored).

H4 (KILLED): s16-typed counter to fail the giv `v->mode == mode` check — score 74,
build 159, sign-extends everywhere.

## Frontier for s2+

F1 (primary): find the HONEST spelling under which `while (i < 2)` survives biv
elimination emitting plain `slti v0,t0,2`. Mechanism-grounded angles, in order:
  a. Identify WHICH giv qualified for the compare rewrite (out-class, mult 2,
     add reg191, always_computable despite all output sites being inside arms).
     Read loop.c record_giv/not_every_iteration handling, or use the instrumented cc1
     (tools/gcc-2.7.2/cc1, BB2_*_DEBUG hooks) to log the qualifying giv's insn. Then
     find the C statement it comes from and see whether an honest respell of THAT
     statement makes it !always_computable (every giv ineligible -> i survives).
  b. Alternate axis: make one giv of i fail the worthwhile test (loop.c:3824
     lifetime*threshold*benefit < insn_count) -> all_reduced=0 -> no elimination.
     Check whether that ALSO suppresses the a2/a3 walkers (would break H2's match —
     measure before investing).
  c. Census the OTHER solved jtbl-cluster members (owner directive: "same pure-C route
     as the other jtbl members") — grep docs/grind/decisions.md and closed ledgers for
     how their loop counters survived; a proven sibling idiom beats derivation.
F2: keep `li 1` inside the loop (target: addiu v0,zero,1 between lhu and addiu -1,
    filling the load-delay slot; ours hoists it as a movable). Suspect coupled to F1's
    register pressure; re-measure after F1. If independent: loop.c scan_loop movables
    ("move-insn savings 1") — find what makes the original's 1-pseudo unmovable
    (e.g. conditional-arm placement that scan_loop refuses; ours moved regardless).
F3: tail block — honest producer for the materialized &D_80102790 (lui+addiu, lw/sw via
    0(v0)) + early D_80102790 store + store order 9C,94,98. Investigate the aggregate
    hypothesis (D_80102788..D_8010279C one struct) with independent evidence
    (split-scalars-hide-aggregate recipe: adjacent .data words, sibling functions,
    cross-TU stride) BEFORE any pointer spelling; the rule-era pointer local is the
    FAKE-family fallback, last resort only.

## [s1] The committed rule-era body (pins, hardcoded asm, pointer-alias tail) reflects rule calibration, not original structure
- mechanism: cheat-asm stripping leaves a structurally broken object (extra callee-save s1, split output[2] pointer, lbu through sp+43)
- probe: sandbox --disable all on committed body (42) vs fresh clean derivations (v1 49, v2 34 with build_insns==target_insns==141)
- result: fresh v2 beats chassis and matches insn count exactly
- verdict: CONFIRMED

## [s1] Loop 1's original shape is indexed accesses (sp.output[i], sp.output[i+2], packets[i*8+k]) over one counter, not walking pointers
- mechanism: loop.c combine_givs merges index-derived givs into one walker per array (a2 offsets 0/4, a3 offsets 0..3) matching target; pointer-deref spelling instead splits per-offset walkers AND lets cse fold the sh->lhu store-reload that target keeps
- probe: v1 (explicit pointers) 49 with split walkers and folded reload; v2 (indexed) 34 with target-shaped walkers and surviving lhu
- result: v2 matches target addressing and reload
- verdict: CONFIRMED

## [s1] The dominant v2 residual is loop.c eliminating biv i (exit test rewritten to slt a2,limit)
- mechanism: strength_reduce final-value path (dumps/code6cac.loop:885-886) + compare rewrite via an always-computable out-class giv (loop.c:6127+); blocking the rewrite cascades the entire register file to target (i->t0, mask->t1, const4->t2, jtbl->t3, loop-2 counter->t0)
- probe: diagnostic exit test `while (i + 1 < 3)` (REG nested in PLUS fails maybe_eliminate_biv_1) scored 28 vs 34
- result: elimination blocked, registers align; diagnostic itself costs +2 insns (combine will not fold signed < across PLUS) and is not submittable
- verdict: CONFIRMED

## [s1] A narrow (s16) counter blocks elimination via the giv mode check honestly
- mechanism: loop.c:6131 requires v->mode == biv mode
- probe: s16 i measured
- result: 74 (build 159), sign-extends everywhere
- verdict: KILLED
