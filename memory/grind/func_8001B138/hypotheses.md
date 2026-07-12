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

- UNEXPLORED (low confidence, not fully closed): is there a *dual-purpose* variable already alive across this residual (e.g. hoisting the existing `v` local from the trailing rounding block earlier) that could legitimately carry -7168 for OTHER reasons and incidentally also feed the clamp store? s2's quick analysis found the obvious version (pre-init `v = -0x1C00` before its real use) is a dead-store-before-overwrite antipattern — same "no semantic purpose" objection. s3 confirmed the trailing block's read of `g_file_vram_timer` must happen AFTER the clamp per the target asm ordering, so there's no natural control-flow reshuffle that lets `v` legitimately hold the pre-clamp constant for a genuine second purpose without inventing an artificial one (itself a forbidden naming-around-cheat pattern). Effectively exhausted; no further avenue derived.
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
