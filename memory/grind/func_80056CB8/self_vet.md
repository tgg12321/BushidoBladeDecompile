# SELF-VET — func_80056CB8

CONSTRUCTS: none (this session's diff is a pure statement reorder of pre-existing, already-vetted real statements; no new local, no dead code, no annotation-requiring construct, no asm, no volatile, no pins)

## T1 semantic purpose
Every statement in the diff has an observable effect on the function's output: `sin_p`, `cos_p`, `scale`, `x`, `z` are all real values consumed downstream (by the `func_80053614` calls and the `if (r1 != 0)` adjustment). The ONLY change this session is WHERE the `pt0[0..2]`/`pt1[0..2]` array stores sit relative to those computations — moved from interleaved-with-x/z to after-all-of-sin_p/cos_p/scale/x/z. Both orderings compute and store identical values; removing the reorder would not remove any statement, only its position. No construct in the diff is behavior-free.

## T2 human-programmer test
Yes. A programmer filling in a struct/array of computed point coordinates from several intermediate values would naturally either interleave the stores with the computations or batch them — both are ordinary, unremarkable C style choices a human would make without needing any special justification. Nothing here reads as "why is this here."

## T3 GCC-internals justification test
The justification for WHY this ordering was tried is register-allocation-motivated (finding the spelling that produces the target's register assignment) — but the construct itself has a plain semantic reading with no dependence on the justification. Per [[ordinary-c-judge-decidable]] Ruling 1 (owner ruling 2026-08-31): "a construct with a real semantic reading ... is NEVER a cheat merely because the agent chose it after observing the scheduler — 'scheduling-motivated respelling' is not a FAIL ground when the spelling is semantically truthful." This diff has no construct that lacks a semantic reading, so this test does not indict anything here.

## T4 permuter/search provenance
The winning form was found via `tools/spelling_enum.py` + `tools/sweep_variants.py` (a systematic ordering enumerator, not a random-mutation permuter) sweeping legal reorderings of REAL, already-present statements — it did not invent any new construct, only reordered existing ones. Per the auto-search vetting checklist in [[no-new-park-categories]]: the found form does not match any forbidden-family construct (no dead code, no unused locals, no address coercion, no padding); it is simplest-known-form ordinary C. This is not a case of "necessary only because the search found it" — the reordering is a real, independently-explainable structural choice (batch the output writes after the value computations), not an opaque search artifact.

## T5 family check
No construct in the diff matches any entry in the forbidden-family catalog (register pins, hardcoded-$N asm, scheduling barriers, volatile coercion, dead locals, alias renames, dead conditional stores, empty-body wrappers, etc.) — there is no new declaration, no dead statement, no annotation-requiring construct of any kind. This is plain statement reordering of code that was already in the candidate.

## T6 naming-announces-intent test
No new names were introduced. No `pad`, `dummy`, `unused`, `spill`, `_buf`, `tail`, `slack` or similar coercion-announcing identifiers appear anywhere in the diff.

SANCTIONED-FAMILY-CLAIMS: none — the diff needs no sanctioned-family exception because it introduces no no-semantic-purpose construct. It is ordinary compilable C under [[ordinary-c-judge-decidable]] Ruling 1: "Choosing among semantically-truthful C spellings by observing codegen is the METHOD of matching decompilation, not a cheat signal."

ANNOTATION-CONFORMANCE: n/a — no FAKE construct anywhere in the diff.
