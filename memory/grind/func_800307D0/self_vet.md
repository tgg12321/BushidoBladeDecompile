# SELF-VET — func_800307D0 (s8, 2026-08-25)

The diff replaces `INCLUDE_ASM("asm/funcs", func_800307D0);` in
`src/code6cac_b.c` with the pure-C body in `memory/grind/func_800307D0/candidate.c`.
Nothing else in the repo is touched by this session's work (asmfix.txt and
src/text1b.c are dirty from a CONCURRENT operator session migrating
func_80060A68 — not mine, not part of this candidate).

CONSTRUCTS: none. (No FAKE construct, no sanctioned-family claim, no inline asm,
no register pins, no volatile, no aliases, no dead locals, no dead stores, no
do-while(0), no goto. Every local is written and read; every statement is live.
The only spellings worth naming are ordinary C idioms already used verbatim
elsewhere in this same function and file: byte-offset casts
`*(s16 *)((u8 *)a0 + 0xNNN)` and a scaled index inside a memory reference
`*(s16 *)((u8 *)a0 + s1 * 2 + 0x332)`.)

## T1 semantic purpose
Every construct is load-bearing. `count` is the queue length; the guarded block
computes `s1` (whether entry 0 is the already-hit weapon id); `s3` is the queue
slot the function returns and passes to `func_80030580`; the for-loop shifts the
queue down one slot; the tail decrements the count and dispatches one of two
effect calls. Delete any statement and the function is wrong. The one spelling
this session changed — reading slot `s1` as
`*(s16 *)((u8 *)a0 + s1 * 2 + 0x332)` instead of building a named pointer first
— has the same semantics as the old form and one fewer local; it is a
simplification, not an addition.

## T2 human-programmer
Yes. "Read the s16 at queue slot `s1` (queue base 0x332, stride 2)" written as a
scaled byte offset is the dominant idiom in this file, and the very next
statement in this same function (the copy loop, unchanged from the prior
reviewer-PASSed candidate) already writes `(u8 *)a0 + 0x332 + i * 2`. The
if-guard `if (count >= 2 && *(s16 *)((u8 *)a0 + 0x88) != -1)` replaced two
`goto`s with the natural conditional. A reader would not ask "why is this here?"
about any line.

## T3 GCC-internals justification
The body is fully describable as program logic with no reference to GCC. It is
true that this session's forensics identified WHY the previous spelling was one
instruction off (see below) — but the justification for the committed spelling is
"it is the simpler, more idiomatic way to read a scaled queue slot", and the
result is fewer locals and less code than the rejected forms. The pass mechanism
explains why the target's bytes are what they are (i.e. what the original source
must have looked like); it is not the semantic content of the code. Recorded for
the audit trail, not as a defense of an otherwise-purposeless construct:
GCC 2.7.2 expands a `+` inside a MEM address with `modifier == EXPAND_SUM`
(`expr.c:5238`), which reaches the `both_summands` canonicalization
`if (CONSTANT_P (op0) || GET_CODE (op1) == MULT) swap` (`expr.c:5289`), fed by
the MULT_EXPR indexed-address path (`expr.c:5362-5382`); an assignment RHS goes
to `binop` instead and keeps the front end's pointer-first order. That is the
whole 7-session "endgame lock": the target simply dereferences, we assigned.

## T4 permuter/search provenance
None. No permuter was run this session. The spelling was derived by reading
GCC 2.7.2's `expr.c` PLUS_EXPR/MULT_EXPR expansion and `optabs.c`
`expand_binop`, predicting which contexts can produce the target order, and then
confirming the prediction with a 7-row measured matrix
(`tmp/grind/func_800307D0/s8/variant_matrix.md`) in which every row behaves
exactly as the mechanism predicts (address-context + MULT index => 0; anything
else => 1). The prior sessions' permuter campaigns (~144k iters) never found it.

## T5 family check
No forbidden family. Specifically NOT or-tree-shape-shift: no operand of any
expression was reordered or reparenthesized to chase a score — the committed
expression is written in its natural reading order (base, then scaled index,
then field offset), and the address-vs-assignment difference is a structural
difference in the C, not a commutative shuffle. The reviewer-FAILED int-cast
form (`(s32 *)(v0 + (s32)a0)`) is NOT used and remains in
`rejected/int-cast-operand-swap.c`; no pointer is cast to an integer anywhere in
the body. No dead store, no dead local, no alias, no volatile, no wrapper.

## T6 naming-announces-intent
No intent-announcing names. Locals are `count`, `s1`, `v0`, `v1`, `s3`, `a2`,
`i` — the register-flavoured naming convention already used throughout
`src/code6cac_b.c` for not-yet-semantically-named values. No `pad`, `dummy`,
`spill`, `tmp_buf`, `_frame_pad`. Every one is read.

SANCTIONED-FAMILY-CLAIMS: none — the body is ordinary C and claims no exception.

ANNOTATION-CONFORMANCE: n/a — no FAKE construct in the diff.

## Bytes (measured this session)
- `tools/wteng.ps1 main sandbox func_800307D0 --disable all` -> **score 0**,
  target_insns 76 == build_insns 76, `rules_dropped: 0`, scorable true.
- `tools/wteng.ps1 main build` -> sha1
  `62efab4f73f992798c43e8c730aa43baa10bb4fa` == oracle, **MATCH**.
- `grep -c regfix/asmfix` for this function: zero rules; no
  `inline_asm_canonical.txt` entry; no `__asm__` anywhere in the body.
