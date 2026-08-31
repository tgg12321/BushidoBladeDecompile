# SELF-VET — func_80045878   (s10, 2026-08-30)

NOTE: this session's outcome is `ruling-request`, NOT `candidate-ready`. The
form is byte-exact (sandbox --disable all = 0, 108/108 insns, rules_dropped 0)
but construct (2) below has no citable sanctioned family, so it is not
submitted. This vet is written in full anyway, so that the ruling can be made
against the real diff and a later session can submit unchanged.

CONSTRUCTS: (1) `v0` (existing s16* local, the func_8004574C result pointer)
reused as the tail record base; (2) `c` (fresh s32 local) written three times —
the third-if condition read `(c = s1[3]) != -2`, then `a0 + 3`, then `0x8000`;
(3) `s3` — a single named local for `a0 + 3` in the join block (ordinary C, one
write, six reads, no exception claimed).

## T1 semantic purpose
(1) YES beyond bookkeeping only in codegen terms: `v0` genuinely holds the base
address the six tail stores write through; the value is real and read six times.
The behaviour is identical to using a separate pointer, so the CHOICE of `v0`
over a fresh pointer has no observable effect — that is exactly why it needs the
staged-value-reuse exception rather than passing as ordinary C.
(2) Same answer: each of `c`'s three values is real and consumed by the very
next statement (the `!= -2` comparison, `v0[11] =`, `*(v0+0x18) =`). Using ONE
variable rather than three has no observable effect. Needs an exception.
(3) PASSES as ordinary C: `s3` holds a real value read at six call sites; a
human writes this to avoid repeating `a0 + 3` six times.

## T2 human-programmer
(3) yes, unambiguously. (1) plausibly — reusing a already-finished pointer local
for the record being filled in is a shape the SOTN-era sources do ship. (2) NO:
a human writes `s1[3] != -2` and `v0[11] = a0 + 3;` directly; folding three
unrelated values through one scratch is not something a reader would explain
semantically. This is the construct that fails the test and drives the ruling
request.

## T3 GCC-internals justification
(1) and (2) are BOTH justified by GCC internals, not by program logic:
sched.c:2543 `adjust_priority` / sched.c:2570 `birthing_insn_p`
(`reg_n_sets[dest] == 1`) for (1), and local-alloc.c:472 (`reg_basic_block[i]
>= 0`) plus global.c's calls_crossed rule for (2). Per the checklist this is a
cheat SIGNAL, which is why both are declared here rather than glossed; the
project's sanctioned families are precisely the set where that signal has been
ruled acceptable with a FAKE annotation. (1) sits inside such a family.
(2) does not, on the current rule text.

## T4 permuter/search provenance
None. No permuter ran this session. Both constructs were derived from a read of
tools/gcc-2.7.2/sched.c and this session's cc1 -dS dumps
(tmp/grind/func_80045878/dumps/, block 13 ready-list trace), then spelled and
measured by hand. Eleven alternative spellings were measured and banked in
memory/grind/func_80045878/rejected/.

## T5 family check
(1) `.claude/rules/staged-value-reused-variable.md` — all six bounds hold; see
SANCTIONED-FAMILY-CLAIMS below.
(2) NO family covers it. It is explicitly excluded by bound 2 of the
staged-value rule ("Inventing a new variable just to have something to borrow
is NOT this rule"); it is not the named-intermediate family (which requires
once-written / once-read; `c` is written three times); it is not dead-store,
constant-holder, pointer-alias, duplicated-statement, dead-array, do-while(0),
sub-word read, mixed-exit or split-read. A grep of
docs/reference/sotn-construct-index.md for reuse constructs returns only the
five `// fake reuse of i?` cutscene sites, which are borrows of an EXISTING loop
counter — i.e. the same quadrant as (1), not (2).

## T6 naming-announces-intent
No `pad` / `dummy` / `unused` / `spill` / `_buf` / `tail` / `slack` names. Every
local is the m2c-derived register name already used throughout this file
(`v0`, `s1`, `s3`, `s0`) or a one-letter scratch (`c`). No declaration exists
whose only uses are discards, address-of, or nothing.

SANCTIONED-FAMILY-CLAIMS:
  FAMILY: staged-value-reused-variable  (construct (1) only)
  SCOPE: "SANCTIONED 2026-07-03 - a real, immediately-used value staged through an existing (currently-dead) local to fix instruction order; FAKE-annotated, lever-exhaustion required; zero dead code"
  PRECEDENT: .claude/rules/staged-value-reused-variable.md:46
  (that line is the SOTN-master citation block: src/st/{cen,lib,no3,st0,top,mar}/cutscene.c `// fake reuse of i?`)

  FAMILY: (none available for construct (2), the fresh carrier `c`) — this is
  the ruling request. No SCOPE/PRECEDENT is claimed, and none is invented.

ANNOTATION-CONFORMANCE: not yet emitted. The banked candidate.c carries the
mechanism, the liveness argument and the lever-exhaustion pointers in its header
comment but does NOT yet carry inline `/* FAKE: ... */` lines, because the
family for construct (2) is unresolved and a FAKE annotation naming a family
that has not been granted would itself be a false citation. On a favourable
ruling the submitting session must add, at the two sites:
  /* FAKE: tail record base staged through the finished `v0` lookup pointer,
     mechanism: sched.c:2543 adjust_priority -> sched.c:2570 birthing_insn_p
     (reg_n_sets[dest] == 1), lever-exhaustion: memory/grind/func_80045878/
     hypotheses.md [s10] + rejected/s10-base-second-set-*.c */
  /* FAKE: two tail scratch values carried in the s1[3] condition temp,
     mechanism: local-alloc.c:472 (reg_basic_block >= 0) + global.c
     calls_crossed callee-save rule, lever-exhaustion: memory/grind/
     func_80045878/hypotheses.md [s9]+[s10], rejected/s10-carrier-*.c */
