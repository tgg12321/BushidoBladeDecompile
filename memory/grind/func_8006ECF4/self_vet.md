# SELF-VET — func_8006ECF4 (s4, permuter modality)

CONSTRUCTS: none (this session did not produce a candidate-ready submission; no construct is proposed for landing)

## T1 semantic purpose: n/a — no construct submitted. (The permuter's `new_var` dead-store family, examined and rejected this session, has NO semantic purpose: it is written once inside an unrelated comparison/arithmetic expression and never read.)
## T2 human-programmer: n/a. (The rejected `new_var` family: no programmer would write `if (sel < (new_var = 15))` from a spec of this function's behavior.)
## T3 GCC-internals justification: n/a. (The rejected family's only justification is that materializing a literal into a fresh pseudo changes local-alloc/cse register-choice for surrounding code — a GCC-internals mechanism, not program logic — which is exactly why it was rejected.)
## T4 permuter/search provenance: n/a for submission. This session ran a directed permuter campaign (tmp/perm_ecf4/, 15,077+ iterations, base score 725, best find 470) as the mandated modality. Every closing-score find across ~10 harvested outputs used the SAME `new_var` dead-write family (plus one co-occurring redundant `& 0xFFFFu` width mask in output-470-1) — i.e., the search converged entirely on a forbidden-family construct, which is the correct outcome to report and reject, not adopt.
## T5 family check: n/a for submission. The rejected `new_var` finds match dead-store-fake-exception / named-local-fake-exception (.claude/rules/dead-store-fake-exception.md, .claude/rules/named-local-fake-exception.md) — both LAST-RESORT sanctions requiring documented lever-exhaustion + named GCC-pass mechanism + mandatory `/* FAKE */` annotation. Prerequisites are NOT met this session (only 2 honest structural levers were tried before the permuter found this family), so it is not eligible even though the family itself is nominally sanctioned. See memory/grind/func_8006ECF4/rejected/permuter-new_var-dead-store-cheat.c for full reasoning.
## T6 naming-announces-intent: n/a for submission. (`new_var` is decomp-permuter's own auto-generated fresh-variable name; it was never renamed to anything that would try to disguise it, and it is not being submitted.)

SANCTIONED-FAMILY-CLAIMS: none — no construct submitted this session, so no family claim is made.

ANNOTATION-CONFORMANCE: n/a — no FAKE construct submitted; the honest floor-11 candidate.c banked this session carries no cheat construct of any kind (same posture as the s3 header: plain statement-order / duplicated-real-statement / control-flow-topology C only).
