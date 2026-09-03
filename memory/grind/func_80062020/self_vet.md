# SELF-VET — func_80062020

**STATUS: NOT A CANDIDATE. This session (s13, synthesis, 2026-09-03, second run) submits no
candidate-ready outcome and leaves NO C in src/text1b.c — `git status --porcelain src/` is clean
and `INCLUDE_ASM("asm/funcs", func_80062020);` remains at src/text1b.c:3932.**

This file previously held the vet written by the s13 run that the driver DISCARDED, because that
vet re-declared a construct banned for this function in `state.json`. It has been rewritten so no
future session can inherit a banned construct as its starting point. The discarded body and its
full six-test adjudication are banked at
`rejected/epilogue-deadcond-identical-arms-crossjump-score0-s13.c`.

CONSTRUCTS: none — this session produced no diff to src/ and no candidate body. The best form on
record remains the honest floor-4 uniform body in `candidate.c` (one row-pointer local,
`p[2] = 0; p[1] = 0; p[0] = 0;`), which contains no FAKE construct of any kind.

## T1 semantic purpose: n/a — no construct submitted.
## T2 human-programmer: n/a — no construct submitted.
## T3 GCC-internals justification: n/a — no construct submitted.
## T4 permuter/search provenance: n/a — no construct submitted. The 117 shapes compiled this
   session are MEASUREMENTS banked in the ledger, not proposals; none of them is being advanced.
## T5 family check: n/a — no construct submitted.
## T6 naming-announces-intent: n/a — no construct submitted.

SANCTIONED-FAMILY-CLAIMS: none.

ANNOTATION-CONFORMANCE: n/a — no FAKE construct. `candidate.c` is plain C with no annotation
because it carries no coercion construct; it simply does not reach distance 0 (honest floor 4,
re-measured this session).
