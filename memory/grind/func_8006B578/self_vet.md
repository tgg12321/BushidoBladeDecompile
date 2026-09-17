# SELF-VET — func_8006B578

CONSTRUCTS: none (this session's diff to src/text1b.c has been REVERTED to
the INCLUDE_ASM stub before finishing — the improved body lives only in
memory/grind/func_8006B578/candidate.c, which is not a committed candidate
this session; this is a `progress` outcome, not `candidate-ready`). The
candidate.c body itself contains ordinary C only: `switch` statements, `goto`
into a switch's own case body to reach a genuinely-shared consequence
(case 0/1/2 all execute the SAME `*arg1&0x400040` field-increment code —
this is real, observable program behavior, not a no-purpose construct),
casts (`(u32)`, `(s32 *)`), and local scratch variables holding real,
consumed intermediate values (`f2`, `m2`, `c2`, `a3`, `bit`, etc., each
written once and read at least once, each holding a value that appears in
the target's own instruction stream). No FAKE annotation is present because
no FAKE-requiring construct is present.

## T1 semantic purpose
Every construct in candidate.c has an observable effect on the function's
output: the `switch`/`goto shared_400040`/`goto tail` control flow implements
the real dispatch semantics of the FSM (confirmed instruction-for-instruction
against `asm/funcs/func_8006B578.s`); the local scratch variables (`f2`,
`m2`, `c2`) hold values that are read and used to compute the final stored
word. Nothing is dead; nothing would compile away under DCE while still
affecting codegen upstream of DCE.

## T2 human-programmer test
Yes. A programmer implementing "case 0/1/2 each toggle a bit, then ALL THREE
also apply a shared cooldown-flag check and field increment, while case 3/4/5
each apply their own single check without the shared step" would naturally
write case 2's body followed immediately by the shared check as its
fallthrough (or duplicate a `goto` to that point from case 0/1), because the
asm ITSELF shows that shape — this is not a spelling chosen to defeat a GCC
pass, it is the correct translation of the observed control flow.

## T3 GCC-internals justification test
No. The mechanism named in hypotheses.md H5 is GCC's *ordinary, source-order*
switch-body layout (an unremarkable, well-documented behavior, not an
allocator/scheduler/DCE internal being defeated) — and the actual JUSTIFICATION
for the fix is that it makes the C match the REAL control flow read directly
off the target asm (H4, H6), not "this changes GCC's internal priority/LUID/
allocno decision." No construct here is described in terms of defeating CSE,
changing allocno priority, or forcing insn scheduling.

## T4 permuter/search provenance
Not applicable this session — no permuter campaign was run (the residual
after direct asm reading + structural correction was a scorer-artifact, not
a search-shaped register/scheduling problem; see hypotheses.md H7). All three
structural fixes (H4/H5/H6) were derived by reading `asm/funcs/func_8006B578.s`
directly and matching the C to it, not proposed by an auto-search tool.

## T5 family check
No construct matches any forbidden family. No pins, no `__asm__`, no
volatile coercion, no dead stores/self-assigns, no unused arrays, no
alias renames, no scheduling barriers. The `goto` usage matches the
SOTN-sanctioned "mixed exit forms" precedent shape in spirit (goto used for
real control-flow merges, not for coercion) but doesn't even need that
carve-out since it's a plain semantically-required jump, not a no-purpose
wrapper.

## T6 naming-announces-intent test
No construct is named `pad`, `dummy`, `unused`, `spill`, `slack`, `_buf`,
`tail` (the `tail:` label names the function's real converged exit path,
matching the asm's own semantic role, not a coercion device), or any other
intent-announcing name. `shared_400040` names the real semantic role of the
label (the shared `*arg1 & 0x400040` check all three cases 0/1/2 execute).

SANCTIONED-FAMILY-CLAIMS: none — no construct in this session's candidate
requires a sanctioned-exception family; everything is ordinary,
semantically-motivated C.

ANNOTATION-CONFORMANCE: n/a — no FAKE construct anywhere in this session's
work.

## Outcome disposition note
This session did NOT reach `sandbox --disable all == 0` (floor is 2, from a
scorer-side cross-TU jump-table-symbol resolution artifact documented in
hypotheses.md H7/evidence.md Session 3 — not a source-level C defect). Per
contract, `src/text1b.c` has been REVERTED to its INCLUDE_ASM stub (git
status confirms zero diff on that file) and the improved body is saved only
to `memory/grind/func_8006B578/candidate.c`. This is reported as a
`progress` outcome, not `candidate-ready`.
