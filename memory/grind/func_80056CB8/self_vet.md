# SELF-VET — func_80056CB8

CONSTRUCTS: none (this session made no net source change — three probed
respellings of the flags==4 threshold comparison were each measured and
reverted; the committed candidate.c body is byte-identical to the s11/s12
banked form: the s7 flags/ang/code variable-reuse merge + s11 r1/r2
variable-reuse merge, both SOTN-sanctioned variable-reuse-for-codegen-
control, unchanged this session)

## T1 semantic purpose: N/A — no construct in the diff (this session is a
pure pass-attribution + probe-and-revert session; src/text1b.c currently
carries the same s11/s12 body). The two banked variable-reuse merges
(flags/ang/code at s7; r1/r2 at s11) each borrow an EXISTING local for a
second unrelated but REAL value with observable effect (the merged
variable's final value is what gets stored to `*(s8*)(arg0+0x444+i)`).
## T2 human-programmer: N/A — no new construct. The banked merges read as
ordinary reuse of a status/flags accumulator across sequential stages of
one loop iteration, which is how a human decompiler naming from assembly
observation would write it once shown the target keeps one register for
all three roles.
## T3 GCC-internals justification: N/A — no new construct this session.
The banked merges' original justification (s7/s11 headers) cites program
logic (three non-overlapping-lifetime named quantities in the same loop
iteration), not a GCC pass, as the actual code change.
## T4 permuter/search provenance: N/A — no auto-search used this session;
all three probes were hand-derived from a loop.c source reading, each
independently measured via sandbox --disable all and reverted on no gain.
## T5 family check: N/A — no new construct in the diff.
## T6 naming-announces-intent: N/A — no new construct in the diff.

SANCTIONED-FAMILY-CLAIMS: none — this session's diff is empty (net) and no
family is being claimed. The pre-existing candidate.c body's two claims
(variable-reuse-for-codegen-control at s7 and s11) are unchanged from prior
sessions' self-vets; re-stated here for continuity, not re-claimed fresh:
  FAMILY: variable reuse for codegen control
  SCOPE: "reusing one C variable for two unrelated values to influence loop-invariant detection or RA. SOTN ships `idxSub = idxSub;` and `randy = basePoint.x; baseX = randy;` with \"FAKE but makes register allocation work\" comments."
  PRECEDENT: .claude/rules/no-new-park-categories.md:185

ANNOTATION-CONFORMANCE: n/a — no FAKE construct anywhere in the current
candidate.c body (the variable-reuse-for-codegen-control family is a
SOTN-accepted ordinary-C technique per Ruling 1 of
.claude/rules/ordinary-c-judge-decidable.md and does not require a FAKE
annotation — only the LAST-RESORT families listed in
no-new-park-categories.md's "2026-07-01 additions" carry that
prerequisite).
