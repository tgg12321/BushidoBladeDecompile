# SELF-VET — func_8002C22C

CONSTRUCTS: none (this session did not change candidate.c; the s3 candidate stands unmodified at floor 196). The permuter's best-scoring find (a fresh once-written/once-read `new_var` staging local for `*(s32*)0x1F80005C`) was test-spliced into src/code6cac_b.c for measurement only, measured codegen-neutral (196, no improvement), and was reverted before session end — it is not part of the banked candidate.c and is not being proposed.

## T1 semantic purpose: n/a — no construct proposed.
## T2 human-programmer: n/a
## T3 GCC-internals justification: n/a
## T4 permuter/search provenance: n/a — the one permuter find inspected was measured and rejected (no real-floor improvement), not adopted.
## T5 family check: n/a
## T6 naming-announces-intent: n/a

SANCTIONED-FAMILY-CLAIMS: none — this is not a candidate-ready session. The existing s3 candidate.c's own construct (duplicated-statement-into-arms, per-arm zero-init duplication) was already vetted and banked in prior sessions' self-vet history; unchanged this session.

ANNOTATION-CONFORMANCE: n/a — no FAKE construct in this session's diff (there is no diff; src/code6cac_b.c was reverted to HEAD before session end, confirmed via `git status --short`).
