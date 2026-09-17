# SELF-VET — func_80073200
CONSTRUCTS: address-taken local struct `S73200 s;` (from s2, unchanged this session); named intermediate `s32 tbl;` (from s2, unchanged this session); named intermediate `s32 v12 = 0x12;` (new this session, H5)

## T1 semantic purpose
`S73200 s` / `tbl`: unchanged from s2 — each carries a real value consumed
by `func_80073728`/`func_8007352C` through `&s`/`&s.sp18`, and the aggregate
grouping matches the identical layout already on main for
func_80069AE4/func_80069F80/func_8005D46C/func_8005FA98.
`v12`: holds the literal `0x12` that the function genuinely stores into
`s.sp2C` (a real field consumed by the following `AddPrim(D_800A374C +
s.sp2C*4, ...)` call) at TWO real call sites. The value is not decorative —
removing `v12` and inlining `0x12` at both sites produces byte-DIFFERENT
(worse) output, so the construct has an observable effect through the
compiled bytes, not just through "steering an analysis pass" with no
output trace. It is a name for a value the function actually uses twice.

## T2 human-programmer test
A human writing this function from a spec ("draw two rects at index 0x12
into the OT") would very plausibly factor the repeated `0x12` sub-index out
into a single named constant/variable used at both call sites — this reads
as ordinary refactoring-for-clarity, not as an unexplained "why is this
here" construct. No naming (`pad`, `dummy`, `unused`, etc.) that announces
coercion intent.

## T3 GCC-internals justification
The MECHANISM by which this reproduces target's bytes is a GCC-internals
fact (global register allocation choosing a persistent callee-saved
register for a value with a long live range established from a top-of-body
initializer) — but the construct itself is not justified ONLY by that
internal detail. `v12`'s reason to exist is that the ORIGINAL SOURCE
plainly held this index in a named quantity too (target's own asm computes
`addiu $s3,$zero,0x12` ONCE and reuses the register at the second site —
i.e., target's original C almost certainly held this same value in one
local variable). Observing GCC's allocator to CONFIRM this is the same
method sanctioned by the 2026-08-31 ordinary-c-judge-decidable ruling
("choosing among semantically-truthful C spellings by observing codegen is
the METHOD of matching decompilation, not a cheat signal").

## T4 permuter/search provenance
Not permuter-found. Derived from reading the target asm directly (target's
own `addiu $s3,$zero,0x12` / register reuse across both call blocks) plus
the s2 ledger's frontier item 1 (frame-size/callee-save gap), then verified
by measurement (sandbox --disable all --diff, three variant placements
tested and two of them KILLED as instance results).

## T5 family check
Matches the SOTN-sanctioned "named-intermediate declaration order" /
`new_var_temp` family (no-new-park-categories.md § SOTN-accepted
techniques, "Named-intermediate declaration order... Clarification (owner
ruling 2026-08-17)..."), relaxed from once-written/once-read to
once-written/many-read by the 2026-08-31 ordinary-c-judge-decidable
ruling. All prongs: (1) once-written (v12 is assigned exactly once, at
declaration) — satisfies even the STRICTER pre-2026-08-31 prong; (2) real
value — the `0x12` literal is genuinely stored into `s.sp2C` and consumed
by the following `AddPrim` call at both sites; (3) byte-neutral for the
frame-size hunks it targets (target_insns stays 203; the construct closes
real hunks, though one unrelated residual insn remains elsewhere — see
frontier); (4) fresh local, not a borrow — `v12` is a brand-new declaration,
not reusing an existing dead variable ([[staged-value-reused-variable]]
territory, not applicable here); (5) destination not live-pre-initialized —
`v12` has no prior value before its own initializer. No forbidden-family
construct present: no register pins, no `__asm__`, no dead stores, no
unused arrays, no volatile coercion.

## T6 naming-announces-intent
`v12` names the value it holds (a straightforward "value twelve"-style
label, matching this file's existing convention of value-derived names like
`var_v0`, `tbl`, `idx`). It is not named `pad`/`dummy`/`unused`/`spill`/
`slack`/`_buf`/`tail` or any other coercion-announcing token, and it is used
(read) at two real call sites — never discarded, never merely
address-of'd, never `(void)`-cast away.

SANCTIONED-FAMILY-CLAIMS:
  FAMILY: named-intermediate declaration order (SOTN new_var_temp class)
  SCOPE: "A fresh local holding a real, consumed value may be read any number of times." (the 2026-08-31 once-written relaxation of the named-intermediate prong, as stated in .claude/rules/no-new-park-categories.md and restated in .claude/rules/ordinary-c-judge-decidable.md § "Class amendment ratified by this ruling")
  PRECEDENT: .claude/rules/ordinary-c-judge-decidable.md:34 (class amendment paragraph citing docs/reference/sotn-construct-index.md:649)

  FAMILY: address-taken local struct (S73200) — ordinary C, not a listed forbidden/sanctioned family; same shape as func_80069AE4/func_80069F80/func_8005D46C/func_8005FA98 already on main (unchanged from s2, re-stated for completeness)
  SCOPE: n/a — ordinary struct declaration + address-of, no no-semantic-purpose construct involved, not from the frozen-family list at all
  PRECEDENT: src/text1b.c (func_80069AE4/func_80069F80 declarations, already on main; see s2 evidence.md entry for line numbers)

ANNOTATION-CONFORMANCE: n/a — no FAKE construct. Every construct in this diff (the S73200 struct, the `tbl` intermediate, and this session's `v12` intermediate) is ordinary semantically-truthful C carrying a real, consumed value; none requires a `/* FAKE */` annotation under its family's rule.
