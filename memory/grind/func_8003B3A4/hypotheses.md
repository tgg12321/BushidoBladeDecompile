# Hypothesis ledger — func_8003B3A4

- [s2 2026-07-13] H: judge-prescribed direct conditional write matches. Probe: sandbox. Result: distance 6. **KILLED** (rejected/direct-conditional-write-at-macro.c).
- [s2 2026-07-13] H: ternary single store matches. Probe: sandbox. Result: distance 6. **KILLED** (rejected/ternary-single-store.c).
- [s2 2026-07-13] H: value-diamond-into-local then direct store matches. Probe: sandbox. Result: distance 8 (worse). **KILLED** (rejected/diamond-local-then-store.c).
- [s2 2026-07-13] H: constant-offset store (&D_8010277C)[1] escapes the symbolic fold. Probe: sandbox. Result: distance 6 (folds back). **KILLED** (rejected/offset-neighbor-const-fold.c).
- [s2 2026-07-13] H: the pointer-variable alias is the ONLY C form producing the pre-branch $v1 address + register-indirect sb. Probe: the four kills above + s1 asm analysis. **CONFIRMED** — annotated /* FAKE */ alias re-measures 0; disposition awaits ruling (pointer-alias-fake-exception vs binding regression diagnosis).
- [s2b 2026-07-13] H: the annotated alias still measures 0 on the current tree (annotation restored after main dropped it). Probe: sandbox --disable all. Result: 0 (56/56). **CONFIRMED**.
- [s2b 2026-07-13] H: prerequisite 4 clears mechanically under the bd20027f Judge ruling (layer-1 PASS then layer-2). Probe: cheat-reviewer layer-1, twice (second invocation with the decisions.md ruling text after the first FAIL rested on a factually wrong premise). Result: NEEDS_USER — reviewer accepts prerequisites (a)-(d) intact but holds the Judge's "retroactive exhaustion cures first-reach" doctrine needs the human owner's sign-off. **KILLED** (does not clear mechanically; owner answer required before layer-2/candidate-ready).
- [s2d 2026-07-14] H: a MULTI-USE pointer over the D_8010277C cluster (p[0] diamond + p[2]=0) matches, mooting the store-only-alias question by making the pointer legitimately multi-use. Probe: sandbox --disable all. Result: distance 2 (55/56) — build emits register-indirect `sb $zero,2($v1)` where target has the two-insn $at macro store to D_8010277F. **KILLED** (rejected/multiuse-pointer-cluster.c) — the alias is irreducibly write-only single-target per target bytes.
- [s2d 2026-07-14] H: the annotated alias still measures 0 on the current tree (annotation restored after third hygiene drop). Probe: sandbox --disable all. Result: 0 (56/56). **CONFIRMED** (sandbox_annotated_alias_s2d_restore3.json).
- [s2e 2026-07-14] H: deref-of-address `*(&D_8010277D) = k;` escapes the symbolic-MEM fold. Probe: sandbox. Result: 6 (55/56) — folds instantly, identical to direct write. **KILLED** (rejected/deref-of-address-folds.c).
- [s2e 2026-07-14] H: the inner block around the alias is stylistic — a function-scope `u8 *p` declaration matches too. Probe: sandbox. Result: 7 (55/56) — address materializes into $a2 before the first diamond; block scope is LOAD-BEARING. **KILLED** (rejected/function-scope-pointer-early-materialization.c).
- [s2e 2026-07-14] H: the annotated alias still measures 0 after the fourth hygiene drop + restore. Probe: sandbox. Result: 0 (56/56). **CONFIRMED** (sandbox_annotated_alias_s2e_restore4.json).
- [s2c 2026-07-13] H: with the 23:04 Judge ruling (0e606ee0) in hand, prerequisite 4 clears via layer-1 PASS + fresh layer-2 PASS. Probe: both reviewers run this session, sandbox re-verified 0 first. Result: layer-1 PASS; layer-2 FAIL on process ground ONLY — the Judge (PASS/FAIL-only role) self-answered the layer-1 NEEDS_USER that hard rule #1 reserves for the human user; all technical prerequisites re-verified intact by both layers. **KILLED** — prerequisite 4 cannot clear until Trenton personally answers (or delegates NEEDS_USER authority to the Judge). The construct itself is fully vetted; only the sign-off channel is disputed.

## [s2] Deref-of-address spelling *(&D_8010277D) = k; escapes the symbolic-MEM fold and materializes the address pre-branch
- mechanism: If expand_expr kept the ADDR_EXPR as a computed address it would enter RA as a pseudo; instead GCC folds *&SYM to the SYMBOL_REF MEM immediately, so the store stays in the assembler sb macro ($at) form after the diamond
- probe: sandbox func_8003B3A4 --disable all with the deref form in src (tmp/grind/func_8003B3A4/s2/variantF_deref_of_address.txt)
- result: distance 6 (55/56) — byte-identical outcome to the direct conditional write; fold confirmed
- verdict: KILLED

## [s2] The inner block around the alias is stylistic; declaring u8 *p = &D_8010277D; at function scope with the other locals also matches
- mechanism: Initializer expansion happens at the declaration point: at function scope the address pseudo is materialized (lui/addiu into $a2) BEFORE the first value diamond (a1 && D_800A37A0==1) instead of into $v1 between the diamonds; scheduler additionally hoists li v0,0x1D into the first beqz delay slot, cascading through both diamonds
- probe: sandbox func_8003B3A4 --disable all with the flattened form; objdump of the disabled .o (variantG_function_scope_pointer.txt + variantG_objdump_full.txt)
- result: distance 7 (55/56) — worse than block-scoped 0 AND worse than direct-write 6; block scope is LOAD-BEARING
- verdict: KILLED

- [s3 2026-07-14] H: the annotated alias still measures 0 after the FIFTH hygiene drop + restore. Probe: sandbox --disable all. Result: 0 (56/56). **CONFIRMED** (s3/sandbox_annotated_alias_s3_restore5.json).
- [s3 2026-07-14] H: the decl-with-initializer spelling is load-bearing (split `u8 *p; p = &D_8010277D;` would diverge). Probe: sandbox --disable all with the split form. Result: 0 (56/56) — same bytes. **KILLED** — the spelling is an equivalence class; the load-bearing lever is statement position inside the inner block, not the decl form (s3/variantH_split_decl_init.txt). Committed shape unchanged per the Judge's exact-shape constraint.

## [s2] The annotated block-scoped alias still measures 0 on the current tree after the fourth hygiene drop of the /* FAKE */ annotation
- mechanism: Annotation is comment-only; codegen unchanged from the s1/s2 matched form
- probe: restore annotation at src/code6cac_c_ab.c:467-471, sandbox --disable all (sandbox_annotated_alias_s2e_restore4.json)
- result: distance 0 (56/56, rules_dropped 0)
- verdict: CONFIRMED

## [s3] The /* FAKE */-annotated block-scoped alias still measures 0 on the current tree after the fifth hygiene drop + restore
- mechanism: Annotation is comment-only; codegen unchanged from the s1/s2 matched form
- probe: Restore annotation at src/code6cac_c_ab.c:467-471 from candidate.c; sandbox func_8003B3A4 --disable all
- result: distance 0 (56/56, rules_dropped 0) — tmp/grind/func_8003B3A4/s3/sandbox_annotated_alias_s3_restore5.json
- verdict: CONFIRMED

## [s4] A clean alias-free pure-C chassis (direct conditional write) can reach the target's pre-branch $v1 address materialization via random permuter exploration
- mechanism: The hand-built s2/s2e exhaustion grid varied the store block locally (8 forms); a whole-function random permuter campaign mutates the ENTIRE function (temp vars, statement order, types, conditions), so if any global restructure lets a direct symbolic store match, it would surface here
- probe: TWO fresh-seed campaigns via tools/permuter_campaign.py (--stop-on-zero, --stack-diffs default): (1) `clean-direct-write-randbasin`, base = direct-conditional-write chassis (sandbox 6, permuter base 520) — 8,770 iters, 29 finds, best 315; (2) `goto-diamond-randbasin`, base = goto-CFG diamond chassis (never in the hand grid; permuter base 730) — 26 min, 37,903 iters, 84 finds, best 515
- result: ~46,700 combined iterations, ZERO score-0 finds, zero novel classes. Every find classified into known attractors: staged-flag local (`new_var = a1 != 0`), constant-holder (`new_var = 1/2`), param-alias pointer copy (`new_var = arg0`), volatile-local coercion, identity `inline_fn` wrapper, clean store reorders (best clean find: hoist `D_8010277F = 0` above the diamond, 330 — still $at macro stores). None produced the pre-branch lui/addiu-into-$v1 + register-indirect sb; a symbolic store cannot acquire that shape without an RA-visible address pseudo, which in C requires the pointer local. Harvest telemetry in metrics/events.jsonl; logs in tmp/grind/func_8003B3A4/s4/.
- verdict: KILLED (rejected/goto-diamond-chassis.c) — automated whole-function search corroborates the s2/s2e structural exhaustion grid; the annotated block-scoped alias remains the only known C form reaching the target bytes

## [s3] The decl-with-initializer spelling is load-bearing: splitting it (u8 *p; p = &D_8010277D;) inside the block would diverge
- mechanism: If initializer expansion at the declaration point differed from assignment-statement expansion, the address pseudo's materialization slot between the diamonds would move
- probe: sandbox func_8003B3A4 --disable all with the split decl/init form in src
- result: distance 0 (56/56) — byte-identical; the load-bearing lever is the statement's position INSIDE the inner block (after the first diamond), not the decl spelling. Consistent with s2e fn-scope=7. Committed shape restored to exact decl-with-init per the Judge's exact-shape constraint. Artifact: tmp/grind/func_8003B3A4/s3/variantH_split_decl_init.txt
- verdict: KILLED

## [s4] A clean alias-free pure-C chassis can reach the target's pre-branch $v1 address materialization via whole-function random permuter exploration (beyond the hand-built s2/s2e local exhaustion grid)
- mechanism: Random mutation of the ENTIRE function (temp vars, statement order, types, conditions, CFG) could in principle find a global restructure that lets a direct symbolic store schedule/allocate into the target shape
- probe: Two fresh-seed campaigns via tools/permuter_campaign.py (--stop-on-zero, --stack-diffs): clean-direct-write-randbasin (8,770 iters, base 520) and goto-diamond-randbasin (37,903 iters / 26 min, base 730, structurally different CFG never in the hand grid)
- result: Zero score-0 finds; best 315 / 515. All 113 finds classified into known attractor classes: staged-flag local, constant-holder, param-alias pointer copy, volatile-local coercion, identity inline_fn wrapper, clean store reorders (best clean 330, stores still in $at macro form). None produced the register-indirect sb through a pre-materialized $v1
- verdict: KILLED

## [s4] The /* FAKE */-annotated block-scoped alias still measures 0 on the current tree after the SIXTH hygiene drop + restore
- mechanism: Annotation is comment-only; codegen unchanged from the matched form
- probe: Restore annotation at src/code6cac_c_ab.c:467-471 from candidate.c; sandbox func_8003B3A4 --disable all
- result: distance 0 (56/56, rules_dropped 0) — tmp/grind/func_8003B3A4/s4/sandbox_annotated_alias_s4_restore6.json
- verdict: CONFIRMED
