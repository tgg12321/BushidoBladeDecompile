# Hypothesis ledger — func_800288C8

## s1 (recon)

H1: "The body currently on main is already a byte-exact, policy-compliant match; no codegen
lever is needed." Mechanism: n/a (not a codegen question — a review-readiness question).
Probe: `sandbox func_800288C8 --disable all` + `verify-oracle`, read the body against
inline-asm-policy's 2026-09-26 ruling and ordinary-c-judge-decidable Rulings 4/11.
Result: CONFIRMED — sandbox 0/465, full-build SHA1 == oracle, asm island already carries an
`inline_asm_canonical.txt` row (line ~406) citing the exact ruling, and all three FAKE locals
cite specific admitted families (dist: Ruling 11 real-computation; tbl: Ruling 11 (C)(3) Q28;
shift: Ruling 4, arguably not even FAKE-worthy). Not a KILLED hypothesis (nothing disproven);
banked as the session's finding. See evidence.md s1.

## Frontier (if a review bounces this back)

F1: if layer-2/Judge FAILs `shift`'s FAKE label as unnecessary noise (Ruling 4 says no
annotation needed): drop the `/* FAKE: */` comment on `shift` only, re-measure sandbox 0,
resubmit. Mechanism: Ruling 4 compound-assignment-split needs no annotation; removing an
unneeded label never changes bytes. Next probe: `sandbox --disable all` after the one-comment
edit, confirm still 0, fresh layer-2.

F2: if layer-2/Judge wants the Ruling 11 (D) dump-proof dossier for `dist`/`tbl` (hygiene tier
under Q91, not blocking, but may be requested as hygiene debt): run
`pwsh tools/grinder/dump.ps1 func_800288C8`, read `.lreg`/`.greg` for the `dist`/`tbl` seats,
bank the dossier in evidence.md rather than re-deriving the construct.

F3: if layer-2/Judge wants independent confirmation of the inline_o.h macro-text match (already
asserted in the inline_asm_canonical.txt row): diff the six `__asm__` statements at
src/main/17AFC.c:947-952 against `engine/gtemacro.py` PINNED `gte_ldlzc`/`gte_nop`/`gte_stlzc`
text, character for character; this is a read-only confirmation, no src/ change expected.
