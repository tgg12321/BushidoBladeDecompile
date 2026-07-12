# Hypothesis ledger — func_8001B138

- [s2, KILLED] Any literal-RHS respelling (plain `-0x1C00`, `(u16)(s16)-0x1C00` cast-on-literal, `0xE400` unsigned hex) closes the addiu/ori encoding gap without a typed intermediate variable. Measured: all three score 1 (ori). The front-end always reconverts a literal through the u16 lvalue's type at fold time regardless of spelling/cast; only a separately-declared variable (whose own INTEGER_CST is fixed before the assignment) escapes this reconversion. No compliant literal-spelling lever exists.

- [s2, KILLED] The two clamp checks (negative bound, positive bound) can be merged into an if/else-if since they're mutually exclusive. Measured: score 11, build_insns 82 vs target 87 — target unconditionally re-checks the positive bound after the negative clamp fires (two independent ifs), not else-if. The existing two-if structure is required.

- [s2, CONFIRMED mechanism, but FORBIDDEN] A typed local variable (s16 OR s32) initialized directly from the clamp literal and then assigned to g_file_vram_timer produces addiu (score 0) because the variable's own tree-level constant is fixed at its own type before the store's implicit conversion runs, so GCC never re-derives the unsigned CST. This is the exact "typed-literal coercion" family the Judge banned (previously only demonstrated for s16; s2 confirms it generalizes to s32 — width is not the discriminator, "separately-typed holder used solely to steer store encoding" is). Do not resubmit under any type/width without a genuine non-coercion use for the variable.

## [s4] A random-mode permuter search (no directed PERM_* macros) over the whole function will find a score-0 closing form outside the already-identified forbidden typed-holder family.
- mechanism: black-box mutation (temp-for-expr, split-assignment, reordering, etc.) explores a much larger combinatorial space than hand-derivation and might surface a legitimate lever s2/s3 missed.
- probe: built a clean single-function permuter workspace (target.s at offset 0, symbol-name-fixed target.o), ran default-randomization campaign with --stop-on-zero, -j 8, harvested at first zero-score find
- result: found score=0 at iteration 337 — `int new_var; ...; new_var = 0x1C00; ...; g_file_vram_timer = -new_var;` — the SAME forbidden typed-literal-holder family already identified in s2 (separately-typed local variable escapes the u16-target-type reconversion), just a new surface spelling. No novel non-cheat lever surfaced.
- verdict: KILLED (as a source of a *legitimate* closing form; CONFIRMS the mechanism identified in s2 generalizes to yet another spelling, all still forbidden)

## Open frontier (unmeasured, for a future session)

- CLOSED [s5]: UNEXPLORED (low confidence, not fully closed): is there a *dual-purpose* variable already alive across this residual (e.g. hoisting the existing `v` local from the trailing rounding block earlier) that could legitimately carry -7168 for OTHER reasons and incidentally also feed the clamp store? s2's quick analysis found the obvious version (pre-init `v = -0x1C00` before its real use) is a dead-store-before-overwrite antipattern — same "no semantic purpose" objection. s3 confirmed the trailing block's read of `g_file_vram_timer` must happen AFTER the clamp per the target asm ordering, so there's no natural control-flow reshuffle that lets `v` legitimately hold the pre-clamp constant for a genuine second purpose without inventing an artificial one (itself a forbidden naming-around-cheat pattern). s5 measured the mechanical version of this question directly: a full PERM_LINESWAP sweep (6/6 orderings) of the trailing block's three statements found zero score improvements — no reordering flips the clamp-store's encoding. Fully exhausted, no further avenue derived.

## [s5] A directed PERM_LINESWAP sweep over the trailing v>>4 rounding block's 3 statements will surface a statement ordering that changes the constant-pool/scheduling context enough to flip the clamp-store's ori->addiu encoding.
- mechanism: speculative — if the trailing block's codegen shares any RTL/scheduling window with the clamp-store region, reordering it might perturb constant materialization for the earlier store.
- probe: built a fresh clean single-function permuter workspace (validated honest: base vs target objdump diff = exactly the 1 known residual instruction, 87/87 insns), applied PERM_LINESWAP to the 3 statements, ran a directed campaign (bounded 3!=6-permutation search space, not random sampling) via tools/permuter_campaign.py, harvested after completion.
- result: 6/6 permutations enumerated (complete coverage of the search space), 0 finds, best_new_score never beat base_score=5 (the known floor=1 residual). No ordering closes or even perturbs the gap.
- verdict: KILLED
- CLOSED [s3]: whether the ORIGINAL source represented the vram timer clamp bound as a file-scope named constant (`static const s16 D_VRAM_TIMER_MIN = -0x1C00;`) — measured score 1 (identical `ori` encoding via objdump), same as the bare literal. Storage duration/linkage does NOT escape the reconversion; only a block-local variable declaration does. See evidence.md [s3].
- CLOSED [s3]: arithmetic-expression RHS (`0 - 0x1C00`) and enum-constant RHS both measured score 1 — same fold path as the bare literal.

## [s2] Any pure literal-RHS respelling of the negative-clamp store (plain -0x1C00, (u16)(s16)-0x1C00 cast-on-literal, 0xE400 unsigned hex) reaches the target's addiu encoding without a typed intermediate variable.
- mechanism: GCC 2.7.2's c-typeck fold converts any literal RHS through the u16 lvalue's type at tree-fold time, always producing the unsigned CST 58368 regardless of spelling/cast, which materializes as ori not addiu.
- probe: measured sandbox --disable all for 3 literal spellings
- result: all 3 scored 1 (identical ori encoding, 87/87 insns)
- verdict: KILLED

## [s2] The two clamp checks (negative bound, positive bound) can be merged into if/else-if since they are mutually exclusive, matching target with fewer instructions.
- mechanism: if the target compiled the pair as one decision tree, else-if would reduce a redundant re-check
- probe: measured sandbox --disable all on the else-if merged form
- result: score jumped to 11 (build_insns 82 vs target 87) - target unconditionally re-checks the positive bound after the negative clamp fires; confirms the two-if structure is required, not optional
- verdict: KILLED

## [s2] A typed local variable (any integer width, not just s16) initialized from the clamp literal and then assigned to g_file_vram_timer produces the target's addiu encoding because the variable's own tree-level constant is fixed before the store's implicit conversion runs.
- mechanism: converting a variable of a signed type to the u16 global is a bit-truncating register move at RTL expand time, not a re-fold through the target type, so the variable's own materialized constant (-7168, addiu-encodable) survives instead of being re-derived as the unsigned CST (58368, ori-encodable)
- probe: measured sandbox --disable all with `s32 lo = -0x1C00; g_file_vram_timer = lo;` (s32, not s16)
- result: score 0 (byte-identical) - confirms the mechanism generalizes across integer width, but this is squarely the typed-literal-holder family the Judge already forbade for the s16 case; banked as rejected, not proposed
- verdict: CONFIRMED

## [s3] A file-scope `static const s16` named constant (not a block-local variable) reaches the target's addiu encoding for the clamp store, escaping the reconversion via storage-duration/linkage rather than the forbidden local-variable-holder mechanism.
- mechanism: speculative — if cc1's expand_expr treats a file-scope const object's initializer the same way as a local variable's (fixing its own INTEGER_CST before the store's u16-conversion), it would produce addiu without being the specific "local block-scoped single-use variable" construct the Judge named.
- probe: measured sandbox --disable all with `static const s16 D_VRAM_TIMER_MIN = -0x1C00;` declared at file scope, referenced as the clamp-store RHS; confirmed via objdump disassembly of the resulting store instruction
- result: score 1 (87/87 insns) — store still assembles to `li v0,0xe400` (ori $v0,$zero,0xe400), identical to the plain-literal form, not the target's addiu
- verdict: KILLED

## [s3] Arithmetic-expression RHS (`0 - 0x1C00`) or an enum-constant RHS (`enum { VRAM_TIMER_MIN = -0x1C00 };`) reach the target's addiu encoding as alternate non-local-variable spellings of the same constant.
- mechanism: an enumerator's CONST_DECL or a compile-time-foldable arithmetic expression might create a distinct tree node from a bare literal, potentially dodging the u16-target-type reconversion.
- probe: measured sandbox --disable all for both forms independently
- result: both score 1 (identical ori encoding) — GCC 2.7.2 folds both to the same INTEGER_CST as the bare literal before the target-type conversion runs; neither creates a distinct code path
- verdict: KILLED

## [s3] A file-scope static const s16 named constant (not a block-local variable) reaches the target's addiu encoding for the clamp store, escaping the reconversion via storage-duration/linkage rather than the forbidden local-variable-holder mechanism.
- mechanism: speculative: if cc1's expand_expr treats a file-scope const object's initializer the same way as a local variable's (fixing its own INTEGER_CST before the u16-store conversion), it would produce addiu without being the specific local-variable construct the Judge named
- probe: measured sandbox --disable all with `static const s16 D_VRAM_TIMER_MIN = -0x1C00;` at file scope as the clamp-store RHS; confirmed via objdump disassembly of the emitted store instruction
- result: score 1 (87/87 insns) - store assembles to `li v0,0xe400` (ori $v0,$zero,0xe400), identical to the plain-literal form, not target's addiu $v0,$zero,-0x1C00
- verdict: KILLED

## [s3] An arithmetic-expression RHS (0 - 0x1C00) reaches the target's addiu encoding as an alternate non-local-variable spelling of the clamp constant.
- mechanism: GCC 2.7.2 might treat a foldable arithmetic expression as a distinct tree node from a bare literal, potentially dodging the u16-target-type reconversion
- probe: measured sandbox --disable all with `g_file_vram_timer = 0 - 0x1C00;`
- result: score 1, identical ori encoding - GCC folds the subtraction to the same INTEGER_CST as the bare literal before the target-type conversion runs
- verdict: KILLED

## [s3] An enum-constant RHS reaches the target's addiu encoding as an alternate non-local-variable spelling of the clamp constant.
- mechanism: an enumerator's CONST_DECL might create a distinct tree node from a bare literal, potentially dodging the u16-target-type reconversion
- probe: measured sandbox --disable all with `enum { VRAM_TIMER_MIN = -0x1C00 };` then `g_file_vram_timer = VRAM_TIMER_MIN;`
- result: score 1, identical ori encoding - CONST_DECL substitutes its INTEGER_CST at reference time exactly like a macro/literal
- verdict: KILLED

## [s4] A random-mode permuter search (no directed PERM_* macros) over the whole function will find a score-0 closing form outside the already-identified forbidden typed-holder family.
- mechanism: Black-box mutation (temp-for-expr, split-assignment, reordering, etc.) explores a much larger combinatorial space than hand-derivation and might surface a legitimate lever s2/s3 missed.
- probe: Built a clean single-function permuter workspace (target.s at offset 0, symbol-name-fixed target.o so the score is honest), ran default-randomization campaign with --stop-on-zero -j 8, harvested at first zero-score find.
- result: Found score=0 at iteration 337 (~50s after seed): `int new_var; ...; new_var = 0x1C00; ...; g_file_vram_timer = -new_var;` -- the SAME forbidden typed-literal-holder family s2 already identified and the Judge already banned (separately-typed local variable escapes the u16-target-type reconversion), just a new surface spelling. No novel non-cheat lever surfaced.
- verdict: KILLED

## [s5] A directed PERM_LINESWAP sweep over the trailing v>>4 rounding block's 3 statements will surface a statement ordering that changes the constant-pool/scheduling context enough to flip the clamp-store's ori->addiu encoding.
- mechanism: speculative -- if the trailing block's codegen shares any RTL/scheduling window with the clamp-store region, reordering it might perturb constant materialization for the earlier store
- probe: built a fresh clean single-function permuter workspace (validated honest: base vs target objdump diff = exactly the 1 known residual instruction, 87/87 insns), applied PERM_LINESWAP to the 3 trailing-block statements, ran a directed campaign (bounded 3!=6-permutation search space) via tools/permuter_campaign.py launch/harvest, harvested after completion
- result: 6/6 permutations enumerated (complete coverage), 0 finds, best_new_score never beat base_score=5 (the known floor=1 residual)
- verdict: KILLED

## [s6] An RTL-level optimization pass (jump/cse/loop/combine/schedule/local-alloc/global-alloc) makes the ori-vs-addiu decision, and instrumenting cc1's per-pass dumps (-da) will identify a specific pass whose behavior could be steered by a legitimate (non-typed-holder) C-level lever.
- mechanism: speculative -- if some RTL pass (e.g. GREG global register allocation, or a scheduling/CSE step) chose the constant's representation, a legitimate RTL-shape change (not a typed literal holder) might influence it.
- probe: built minimal standalone repros of the literal-RHS form and the typed-tmp form; compiled both with `cc1 -da` (same flags as the sandbox pipeline) to dump RTL after every pass (rtl/jump/cse/loop/cse2/flow/combine/sched/lreg/greg/sched2/dbr); grepped `const_int` in every stage for both variants.
- result: the divergence (`const_int 58368` vs `const_int -7168`) is already fully present in the FIRST dump (immediately post RTL-expand, pre-optimization) and is byte-identical through all 11 subsequent stages to final. No RTL pass reads, rewrites, or has any opportunity to affect this constant -- it is fixed by the C front end's tree-level type conversion (`convert()` in `build_modify_expr`) before RTL is even generated.
- verdict: KILLED -- there is no RTL-pass-level lever. The mechanism is a front-end (pre-RTL) tree fold, confirmed by direct dump inspection, not a GREG/scheduling/CSE decision. This forecloses any future forensics probe of this residual via RTL-pass instrumentation; the only remaining variable is the C source's type structure of the assignment RHS, already exhaustively explored (s2-s5) and confined to the Judge-forbidden typed-holder family.

## [s6] An RTL-level optimization pass (jump/cse/loop/combine/schedule/local-alloc/global-alloc) makes the ori-vs-addiu decision, and instrumenting cc1's per-pass dumps (-da) will identify a specific pass whose behavior could be steered by a legitimate (non-typed-holder) C-level lever.
- mechanism: If some RTL pass (e.g. GREG global register allocation, CSE, or scheduling) chose the constant's representation, a legitimate RTL-shape change might influence it without needing a forbidden typed literal holder.
- probe: Built minimal standalone repros (tmp/grind/func_8001B138/s6/lit.c = clean literal-RHS spelling, var.c = Judge-forbidden typed-tmp spelling); compiled both with the sandbox pipeline's exact cc1 flags plus -da (dumps RTL after every pass: rtl/jump/cse/loop/cse2/flow/combine/sched/lreg/greg/sched2/dbr); grepped const_int across all 11 stages for both variants.
- result: The divergence (const_int 58368 vs const_int -7168) is already fully present in the FIRST dump (immediately post RTL-expand, before any optimization pass runs) and is byte-identical through all subsequent stages to final assembly. No RTL pass touches this constant at any point.
- verdict: KILLED

## [s7] s6 inferred (from RTL-dump behavior) that a separately-declared local VAR_DECL's own INTEGER_CST escapes the front-end's target-type reconversion of a literal RHS, while any literal/const-expr spelling does not.
- mechanism: Read the actual c-typeck.c source (tools/gcc-2.7.2/, the project's compiler fork). build_modify_expr (line 3794) calls convert_for_assignment (line 3933), which at line 3987 does: `else if (optimize && TREE_CODE (rhs) == VAR_DECL) rhs = decl_constant_value (rhs);` -- i.e. under -O2, ANY bare VAR_DECL RHS is first passed through decl_constant_value BEFORE the arithmetic convert-and-check fold. decl_constant_value (line 976) substitutes the variable's DECL_INITIAL back in (collapsing it to the literal, which then DOES get refolded through the lvalue's u16 type) only when TREE_READONLY(decl) is true (plus not-TREE_PUBLIC, not volatile, constant initializer, non-BLKmode). The controlling predicate is literally 'is this variable const-qualified', not merely 'is it a separately-typed tree node'.
- probe: Predict: if the gate is TREE_READONLY, then making the local holder `const`-qualified should re-enable the fold (score back to 1, same as bare literal) even though it is still a separately-declared local VAR_DECL. Tested `{ const s16 tmp = -0x1C00; g_file_vram_timer = tmp; }` via sandbox --disable all.
- result: Measured score = 1 (87/87 insns) -- identical to the plain-literal clean floor, NOT the 0 that the non-const `s16 tmp` holder scores. Prediction confirmed exactly.
- verdict: CONFIRMED

## [s8] A named bound variable used in BOTH the clamp comparison and the clamp store has genuine dual semantic purpose (unlike the Judge-forbidden store-only holder) and might legitimately reach the target's addiu encoding without failing the human-programmer test.
- mechanism: If the variable's own INTEGER_CST survives the front-end's u16-target-type reconversion (per s7's decl_constant_value/TREE_READONLY finding) while also serving a real comparison role, it would not be 'a holder with no semantic purpose' -- a different, non-forbidden shape than the s2-s7 rejected family.
- probe: Rewrote the negative clamp as `{ s16 min = -0x1C00; if ((s16)g_file_vram_timer < min) { g_file_vram_timer = min; } }` and measured sandbox --disable all.
- result: score=2 (regression from the clean floor's 1) -- WORSE, not better. Using the variable as the comparison operand defeats the target's `slti $v0,$v0,-7168` immediate-compare encoding (the compiler must load `min` into a register for the branch), changing the branch's instruction shape entirely on top of the pre-existing store-constant residual.
- verdict: KILLED

## [s8] A fresh, unbiased m2c decompile of the target asm (not derived from or influenced by 7 sessions of prior hand-analysis) will surface a structurally different C chassis for this residual.
- mechanism: m2c reconstructs plausible original-C structure directly from the target assembly, independent of this ledger's prior reasoning -- a genuine second opinion on the source shape.
- probe: Ran `python3 tools/m2c/m2c.py --context include/m2c_context.h asm/funcs/func_8001B138.s` (fresh, from the raw target asm only).
- result: m2c's reconstruction is structurally IDENTICAL to the existing clean-floor candidate (same two-if clamp structure, same v>>4 rounding block, same *arg0 masking) and independently emits the negative bound as a plain literal assignment (`D_800A3710 = -0x1C00U;`) -- no local-holder trick, no alternate control flow. This is strong independent corroboration (via an unbiased method) that no alternative structural shape exists in this residual's search space.
- verdict: KILLED

## [s8] The decomp.me corpus (BB2 toolchain class: gcc2.7.2-psx / psyq3.5) contains a scratch whose target asm overlaps this function's clamp-store shape closely enough to suggest a legitimate closing C form.
- mechanism: Coarse pre-filter over solved scratches with similar target bytes; a close analog could reveal how a genuine (non-cheat) closing form was written elsewhere for the same ori-vs-addiu constant-materialization choice.
- probe: Ran `tools/decomp_me_scrape.py search --asm-file asm/funcs/func_8001B138.s` against the downloaded corpus index.
- result: Best match similarity 0.074 (essentially noise for a 7-instruction zero-init + tiny-clamp function); no scratch in the corpus has a comparable target shape. No actionable lead.
- verdict: KILLED

## [s8] A Kengo (PS2 successor) source-level analog exists for this vram-timer clamp logic and could be transplanted as a structurally different, still-correct C shape.
- mechanism: Kengo reuses Lightweight's engine; prior sessions' successful transplants relied on a decompiled Kengo C source tree for comparable functions.
- probe: Checked the repo for a Kengo C source tree (only `Kengo/` disc images + function/global NAME-matching CSVs exist -- tools/kengo_match.py, kengo_matches.csv -- no decompiled C body available for any function).
- result: No Kengo C source exists to transplant from for this or any function; only binary-level name-matching artifacts are present. Not a viable avenue for this residual (or likely any BB2 function without a separate from-scratch Kengo decomp effort).
- verdict: KILLED

## [s9] The in-codebase corpus of already-COMPLETED-C functions contains a precedent for a negative-literal-to-`u16`-lvalue store that reaches `addiu` without the Judge-forbidden typed-holder construct.
- mechanism: if some other solved BB2 function assigns a negative literal to a `u16` global and matches target bytes with a plain-literal spelling, it would prove a legitimate lever this residual's search missed.
- probe: grepped all `src/*.c` for negative-literal-RHS assignments and cross-referenced each target lvalue's declared type against `include/*.h`.
- result: zero `u16`-typed targets among any negative-literal assignment in the codebase — every hit resolves to `s8`/`s16`/`s32`. No in-codebase precedent exists.
- verdict: KILLED

## [s9] A second, independently-invoked fresh m2c decompile (re-run this session, not reusing s8's output) will diverge from s8's reconstruction and surface a different chassis.
- mechanism: m2c's reconstruction could vary run-to-run if any part of its heuristics has non-determinism, or if context files changed since s8.
- probe: re-ran `python3 tools/m2c/m2c.py --context include/m2c_context.h asm/funcs/func_8001B138.s` fresh.
- result: byte-for-byte structurally identical to s8's reconstruction — same two-if clamp, same v>>4 block, same plain-literal spellings for both residual constructs (`D_800A3710 = -0x1C00U;`, `*arg0 &= 0xFFFEFFFE;`). Deterministic, no new chassis.
- verdict: KILLED

## [s9] A second, independently-invoked decomp.me corpus search (re-run this session) will surface a newly-added scratch with a comparable target shape.
- mechanism: the corpus is a live, growing dataset; a fresh query could return different results than s8's if new scratches were added since.
- probe: re-ran `tools/decomp_me_scrape.py search --asm-file asm/funcs/func_8001B138.s`.
- result: identical noise-floor result set (similarity 0.071-0.074, same three low-relevance matches as s8). No new corpus entries with a comparable shape.
- verdict: KILLED

## [s9] Re-running the rederive modality a further time (m2c, decomp.me corpus, in-codebase precedent grep) will surface a legitimate non-typed-holder closing form for the addiu-vs-ori residual.
- mechanism: N/A -- this was already independently run this same modality slot (see evidence.md [s9] entries: zero u16-negative-literal precedent anywhere in src/*.c; fresh m2c re-decompile structurally identical to s8 and to the clean floor; fresh decomp.me re-search returns the same noise-floor 0.071-0.074 matches). Re-running it again would reproduce the same KILLED result a third time.
- probe: None run this session -- declined per judge_constraints[3]/[4] ('treat any further modality re-run of the exhausted ladder as invalid work and discard/respawn'; 'do NOT re-request this ruling').
- result: Not re-tested; already KILLED twice on record (s8, s9) via three independent evidence channels.
- verdict: KILLED

## [s9] The Grinder pipeline has an escalation path that will eventually reach the actual owner (Trenton) for a policy decision on this residual.
- mechanism: Three ruling-requests were already submitted to the Judge (docs/grind/decisions.md 2026-07-12 05:16, 05:25, 05:29). The Judge correctly identifies each time that it is not the owner and cannot self-authorize a SOTN amendment or a park decision -- and judge_constraints now explicitly forbid a 4th ruling-request. There is no other automated channel from the Grinder driver to the human owner.
- probe: Reviewed docs/grind/decisions.md full history for this function and cross-checked against state.json judge_constraints.
- result: Confirmed: the automated loop (rederive-modality respawn <-> Judge ruling-request) is closed on both ends. Genuine forward motion requires an out-of-band owner action (queue park with explicit reason, or a direct SOTN-list amendment commit) that only a human operator can perform -- not something a worker session inside this modality ladder can produce.
- verdict: CONFIRMED
