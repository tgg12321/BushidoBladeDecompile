# SELF-VET — func_80074B18

Session s2 (2026-09-14), modality `structural`. The diff replaces
`INCLUDE_ASM("asm/funcs", func_80074B18);` in src/text1b.c with the body of
`memory/grind/func_80074B18/candidate.c` — the identical body banked by s1
(body sha1 a00d6744), unchanged. No new constructs were introduced this session;
the floor moved 1 -> 0 purely because the maspsx `.L`-label load-delay-nop gate was
retired 2026-09-14 and the nop is now emitted globally.

CONSTRUCTS: none

## T1 semantic purpose
Every declaration in the body carries a real consumed value. `p` is the emit cursor into
the primitive buffer (`arg0[5]`, written back at exit); `t` is the per-row source-tile
cursor; `i`/`j` are the two loop induction variables; `n` is the column count (5 or 8,
selected by `arg2`) and is read by the inner loop condition; `ot` is the ordering-table
index (0xB or 0x15, selected by `arg1`) and is read by the `AddPrim` argument. Removing
any one of them changes what the function does. There is no construct present whose
removal would leave behavior byte-identical — i.e. nothing here exists only to shape
codegen. `s16` on `n` (and on `i`/`j`) is a type choice, not an added construct: it is a
narrower integer type on a variable that genuinely holds a small count, which is ordinary
C and is the natural spelling for a loop counter over <= 8 columns / <= 258 rows.

## T2 human-programmer
Yes. Given the specification — "emit (rows x columns) tile sprites from a source tile
table into the primitive buffer, advancing both cursors, with position/OT selected by the
two flags" — this is the obvious C: two nested `for` loops, a cursor pair, a column count
picked up front, an OT index picked per iteration. A reader asks "why is this here?"
about nothing in the body. There is no statement whose only justification is codegen.

## T3 GCC-internals justification
No construct in the diff is justified by a GCC-internals mechanism. The one place the
ledger records a GCC-internals OBSERVATION is `s16 n` vs `s32 n`: with `s32 n` the entry
test of the inner loop is folded (combine.c nonzero_bits, evidence.md), which costs bytes.
That is an explanation of why the ordinary narrow type happens to be the one that matches
— not a construct invented to exploit a pass. `n` holds 5 or 8; `s16` is a truthful type
for it, and a programmer writing this function cold could plausibly pick either width.
Nothing in the diff is named after a lever, and no statement exists whose stated purpose
is to steer an allocator, scheduler, or DCE decision.

## T4 permuter/search provenance
None. No permuter campaign produced this body. It was derived by reading the target asm
(asm/funcs/func_80074B18.s) and writing the C the structure implies; the rejected forms in
`memory/grind/func_80074B18/rejected/` are hand-written variants that were measured and
discarded, not search output. Nothing here passes detectors because of a particular
spelling — there is nothing for a detector to catch.

## T5 family check
No forbidden family is matched, by analogy or otherwise: no register-asm pins, no
`__asm__` of any kind, no scheduling barriers, no volatile in any spelling, no dead or
unused locals, no dead stores or self-assigns, no constant-holder locals, no pointer
aliases to globals, no local arrays, no `do { } while (0)` wrap, no `if (1)` wrapper, no
goto, no dead-param assignment, no width-padding casts. The body is entirely ordinary C:
declarations, two `for` loops, member loads/stores, two `if`/`else` position arms, and
library calls.

## T6 naming-announces-intent
No name in the body announces coercion intent. Names are `p`, `t`, `i`, `j`, `n`, `ot` —
cursor, cursor, row index, column index, count, ordering-table index. None matches
`pad`/`dummy`/`unused`/`spill`/`slack`/`_buf`/`tail`, and every one of them is read as
well as written.

SANCTIONED-FAMILY-CLAIMS: none

ANNOTATION-CONFORMANCE: n/a — no FAKE construct

## Measurements backing this vet (this session)
- `& tools/wteng.ps1 main sandbox func_80074B18 --disable all` -> `"score": 0`,
  target_insns 133, build_insns 133, rules_dropped 0. The sandbox strips cheat-asm before
  scoring, so this is the honest cheat-free distance.
- `& tools/wteng.ps1 main build` (full clean-driver build + relink) ->
  sha1 `62efab4f73f992798c43e8c730aa43baa10bb4fa` == oracle, MATCH.
- No file outside `src/text1b.c` (plus the ledger/scratch surfaces this session is allowed
  to write) was touched. `maspsx_label_nop_funcs.txt` no longer exists — it was deleted by
  the 2026-09-14 retirement — so the s1 handoff's operator step is moot, not performed.
