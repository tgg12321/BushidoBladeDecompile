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

## s1 cont. — H1 was already Judge-FAILed; real blocker is a rule conflict, not a C question

H2: "The identical body (current src/, == candidate.c == rejected/judge-fail-1015-1452.c) can
pass by resubmission." KILLED — `state.json.review_ledger` shows this exact body already got a
Judge FAIL at the "final call" scope (hash `19de43ee8845301d`, 2026-10-15 19:52,
docs/grind/decisions.md:1304). Per review-discipline-before-commit.md's body-hash-keyed loop
rule, a Judge FAIL at final call is terminal on resubmission with no re-review; only a NEW
owner ruling reopens it. kill_scope: instance (one body, one Judge cycle); measured_on: the
exact body in src/ this session, no FAKE change, review_ledger entry above.

H3: "The Judge's demanded macro-call form (`gte_Lzc(tbl,&lzc_out);` via `#include` of a
verbatim `inline_o.h`/`gtemac.h` under include/) is buildable this session without touching
rules/engine." KILLED — `find include -iname "*inline*" -o -iname "*gtemac*"` returns nothing;
no such header exists anywhere in the tree. `engine/gtemacro.py`'s `PINNED` table is a Python
string match-target for the detector (`match_unit`/`unit_spans` recognize written-OUT `__asm__`
statement runs), not a C macro definition a call site could expand through — there is no
`#define gte_Lzc(...)` reachable from `src/**/*.c` today. Building one (new header file under
include/ + gtemacro.py support for a macro-call island shape) is an `engine:`/infra change, out
of a grind session's writable surface (CLAUDE.md). kill_scope: instance (this session, this
toolchain state — a future session after an `engine:` infra commit lands could retest); no
predicate_cite needed (instance kill).

Frontier (for the next session, if a ruling resolves the conflict either way):

F4: if the owner ruling says "land the macro-call form, and the infra lands separately as an
`engine:` commit first" — re-measure sandbox 0 with the new header's macro, confirm
gtemacro.py's detector recognizes the call-site form (or is updated to), fresh layer-2.

F5: if the owner ruling says "inline-asm-policy.md's written-out-only text stands; the
regression's reopen_reason / Judge final-call constraint was itself wrong and is overridden" —
resubmit the EXACT current body (no src/ change) once the ruling lands; do not touch src/ in
the meantime since the body is already byte-exact and policy-conformant to the written rule.

F6 (should not be needed): if neither F4 nor F5 — i.e. the ruling instead narrows the GTE
island requirements to some third spelling — re-derive only the island per the new text; the
surrounding ordinary C (dist/tbl/shift, all other locals) is unaffected either way.

## [s1] The identical body currently in src/ (== candidate.c == rejected/judge-fail-1015-1452.c) can pass by resubmission without any change.
- mechanism: n/a - review-loop body-hash-keyed closure, not codegen
- probe: state.json review_ledger lookup for this body's hash
- result: review_ledger shows this exact body (hash 19de43ee8845301d) already received a Judge FAIL at final-call scope, 2026-10-15 19:52 (docs/grind/decisions.md:1304-1310, including the follow-up ruling at 14:57 answering 'option (a)'). Per review-discipline-before-commit.md's body-hash-keyed loop rule, a Judge FAIL at final call is terminal on resubmission with no re-review.
- verdict: KILLED
- kill_scope: instance
- measured_on: exact body in src/main/17AFC.c this session (candidate.c == rejected/judge-fail-1015-1452.c), no FAKE change from the Judge-FAILed body; review_ledger entry in state.json

## [s1] The Judge's demanded macro-call form (gte_Lzc(tbl,&lzc_out); via #include of a verbatim inline_o.h/gtemac.h under include/) is buildable this session without touching rules/engine/.
- mechanism: n/a - infra-availability question, not a codegen lever
- probe: find include -iname "*inline*" -o -iname "*gtemac*"; read engine/gtemacro.py PINNED table's role
- result: No such header exists anywhere under include/. engine/gtemacro.py's PINNED table is a Python string match-target used by the detector to recognize written-OUT __asm__ statement runs (match_unit/unit_spans), not a C preprocessor macro definition; there is no #define gte_Lzc(...) reachable from src/**/*.c today. Building the header + teaching gtemacro.py to recognize a macro-call island shape is an engine:/infra change, outside a grind session's writable surface per CLAUDE.md.
- verdict: KILLED
- kill_scope: instance
- measured_on: this session's toolchain/tree state (2026-10-15); a future session after an engine: infra commit lands could retest
