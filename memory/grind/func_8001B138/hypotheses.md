# Hypothesis ledger — func_8001B138

- [s2, KILLED] Any literal-RHS respelling (plain `-0x1C00`, `(u16)(s16)-0x1C00` cast-on-literal, `0xE400` unsigned hex) closes the addiu/ori encoding gap without a typed intermediate variable. Measured: all three score 1 (ori). The front-end always reconverts a literal through the u16 lvalue's type at fold time regardless of spelling/cast; only a separately-declared variable (whose own INTEGER_CST is fixed before the assignment) escapes this reconversion. No compliant literal-spelling lever exists.

- [s2, KILLED] The two clamp checks (negative bound, positive bound) can be merged into an if/else-if since they're mutually exclusive. Measured: score 11, build_insns 82 vs target 87 — target unconditionally re-checks the positive bound after the negative clamp fires (two independent ifs), not else-if. The existing two-if structure is required.

- [s2, CONFIRMED mechanism, but FORBIDDEN] A typed local variable (s16 OR s32) initialized directly from the clamp literal and then assigned to g_file_vram_timer produces addiu (score 0) because the variable's own tree-level constant is fixed at its own type before the store's implicit conversion runs, so GCC never re-derives the unsigned CST. This is the exact "typed-literal coercion" family the Judge banned (previously only demonstrated for s16; s2 confirms it generalizes to s32 — width is not the discriminator, "separately-typed holder used solely to steer store encoding" is). Do not resubmit under any type/width without a genuine non-coercion use for the variable.

## Open frontier (unmeasured, for a future session)

- UNEXPLORED: is there a *dual-purpose* variable already alive across this residual (e.g. hoisting the existing `v` local from the trailing rounding block earlier) that could legitimately carry -7168 for OTHER reasons and incidentally also feed the clamp store? Quick analysis this session found the obvious version (pre-init `v = -0x1C00` before its real use) is a dead-store-before-overwrite antipattern — same "no semantic purpose" objection — so this looked like a dead end, but a full sweep of dual-use variable placements was not exhaustively measured.
- UNEXPLORED: whether the ORIGINAL source represented the vram timer clamp bounds as `s16`-typed named constants (`#define`/`enum` won't change type) declared at file scope (not function-local) — a file-scope `static const s16 VRAM_TIMER_MIN = -0x1C00;` participates in genuine "named constant" semantics (not a bare literal-holder local) and might dodge the Judge's specific objection (which targeted a *local*, block-scoped, single-use variable). Untested this session — plausible next probe, but must be checked against whether a file-scope const changes codegen at all (constants folded at compile time typically fold the same way regardless of storage duration, so this may also fail — should be measured, not assumed).
- Both frontier items above are LOW confidence; the mechanism evidence (s2) suggests the encoding is determined purely by "does a separately-typed tree node exist for the value before the u16-conversion," which is true for any named object regardless of scope/duration — so file-scope constants likely fall in the same forbidden bucket. Worth ONE quick measurement to close it out, but do not assume it will pass Judge review even if bytes match — vet first.

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
