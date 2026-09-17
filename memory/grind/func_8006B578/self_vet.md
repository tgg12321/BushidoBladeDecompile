# SELF-VET — func_8006B578

CONSTRUCTS: none (this session made no candidate-ready submission; no diff is proposed to land. candidate.c is unchanged ordinary C from session 3, already vetted in prior self-vets — no new construct was introduced this session. The 10 enumerated spelling variants tested (v01, v02) are ordinary C: local-declaration reordering and CSE-inlining of a shared sub-expression, no FAKE/cheat constructs in either.)

## T1 semantic purpose: n/a — no construct proposed. The tested variants (v01/v02) are ordinary respellings of a real bit-toggle computation; every sub-expression is read and consumed.
## T2 human-programmer: n/a — same reasoning; a human could write any of candidate.c, v01, or v02 from the function's specification.
## T3 GCC-internals justification: n/a — no construct's presence in the code is justified by a GCC-internals mechanism; the choice between candidate.c's form and v01/v02 is a measured codegen-neutral (v02) or codegen-negative (v01) respelling, not a coercion construct.
## T4 permuter/search provenance: the spelling_enum tool is an exhaustive structured search, not a permuter proposal-acceptance path; its output (v01, v02) was REJECTED as a submission (v01 regresses, v02 is score-neutral so there is no reason to prefer it over the banked candidate) — nothing from this search is proposed for candidate-ready.
## T5 family check: n/a — no construct proposed.
## T6 naming-announces-intent: n/a — no construct proposed.

SANCTIONED-FAMILY-CLAIMS: none — this session's result is "progress" (hypotheses banked, no candidate-ready submission). No family claim is being made.

ANNOTATION-CONFORMANCE: n/a — no FAKE construct anywhere in this session's work.
