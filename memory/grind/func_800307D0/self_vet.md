# SELF-VET — func_800307D0 (s8 final, 2026-08-25)

The diff replaces `INCLUDE_ASM("asm/funcs", func_800307D0);` in
`src/code6cac_b.c` with the pure-C body in
`memory/grind/func_800307D0/candidate.c`. Nothing else in the repo is touched
(`git status --short` this session: only `src/code6cac_b.c`,
`memory/grind/func_800307D0/candidate.c`, and the engine's own
`metrics/events.jsonl`).

This session (s8, forensics) re-derived and re-measured the form from the
interrupted prior s8 run's artifacts, and independently re-verified the
mechanism against GCC 2.7.2 source and against fresh cc1 pass dumps of the
matched TU. Measurements are mine, taken this session.

CONSTRUCTS: none. (No FAKE construct, no sanctioned-family claim, no inline asm,
no register pins, no volatile, no alias, no dead local, no dead store, no
do-while(0), no goto, no operand shuffle. Every local is written and read; every
statement is live. The only spellings worth naming are ordinary C idioms already
used verbatim elsewhere in this same function and file: byte-offset casts
`*(s16 *)((u8 *)a0 + 0xNNN)` and a scaled index inside a memory reference
`*(s16 *)((u8 *)a0 + s1 * 2 + 0x332)`.)

## T1 semantic purpose
Every construct is load-bearing. `count` is the queue length; the guarded block
computes `s1` (whether queue entry 0 is the already-hit weapon id); `s3` is the
queue slot the function returns and passes to `func_80030580`; the for-loop
shifts the queue down one slot; the tail decrements the count and dispatches one
of two effect calls. Delete any statement and the function is wrong. Relative to
the prior floor-1 candidate this session's body has FEWER constructs: the named
byte offset `v0 = s1 << 1` and the named base pointer `s2 = (s32 *)((u8 *)a0 +
v0)` are gone, replaced by the direct dereference
`s3 = *(s16 *)((u8 *)a0 + s1 * 2 + 0x332)`; the two `goto do_sll` jumps are
replaced by the natural `if (count >= 2 && ... != -1)`; the two single-use
`a0_arg` temporaries are inlined into the calls. It is strictly a
simplification, not an addition.

## T2 human-programmer
Yes. "Read the s16 at queue slot `s1` (queue base 0x332, stride 2)" written as a
scaled byte offset is the dominant idiom in this file, and the very next
statement in this same function (the copy loop, unchanged from the earlier
reviewer-PASSed candidate) already writes `(u8 *)a0 + 0x332 + i * 2`. A reader
would not ask "why is this here?" about any line; the conditional-guard form is
what one would write from the spec without knowing anything about GCC.

## T3 GCC-internals justification
The body is fully describable as program logic with no reference to GCC, and the
justification for the committed spelling is that it is the simpler, more
idiomatic way to read a scaled queue slot (one local fewer, no goto). The pass
mechanism is recorded for the audit trail — it explains why the ORIGINAL source
must have been written this way, not what the code does:
- `expr.c:5237-5239` — a `+` is expanded through the address path only when
  `modifier == EXPAND_SUM` and `mode == ptr_mode`; otherwise `goto binop`. An
  assignment RHS takes `binop` and keeps the front end's pointer-first order
  (`c-typeck.c:2696`, the s6/s7 finding).
- `expr.c:5288-5290` — "Put a constant term last and put a multiplication
  first": `if (CONSTANT_P (op0) || GET_CODE (op1) == MULT) temp = op1, op1 = op0,
  op0 = temp;` — this is the swap that produces the target's index-first order.
- `expr.c:5362-5384` — the `MULT_EXPR` EXPAND_SUM indexed-address path returns a
  literal `gen_rtx (MULT, ...)`, which is the `GET_CODE (op1) == MULT` the swap
  above tests for. A named `v0 = s1 << 1` local destroys it (the operand is a
  plain REG by then), which is why variant E stayed base-first.
- `optabs.c:399-421` — `expand_binop`'s commutative swap fires only when
  `(op1 is REG && op0 is not) || target == op1 || op0 is CONST_INT`; two pseudo
  REGs never swap. That is the directive's "expand_binop canonicalization
  reachability" question, and the answer is NEGATIVE — the reachable canonicaliser
  is the `both_summands` one in `expr.c`, not `expand_binop`.
Verified in my own fresh dumps (`pwsh tools/grinder/dump.ps1 func_800307D0`,
`tmp/grind/func_800307D0/dumps/`): initial RTL insn 65 =
`(ashift (reg 93 = s1) 1)` with `REG_EQUAL (mult (reg 74) 2)`, insn 67 =
`(plus (reg 94 = index) (reg/v 72 = a0))` INDEX-FIRST, final asm
`addu $18,$2,$16` == target `addu $s2,$v0,$s0` (idx25).

## T4 permuter/search provenance
None. No permuter was run this session, and the form is not a permuter product —
the three prior campaigns (~144k iterations, s4/s5) never found it. It was
derived by reading GCC 2.7.2's `expr.c` PLUS_EXPR/MULT_EXPR expansion and
`optabs.c expand_binop`, predicting which C contexts can produce the target
order, and confirming the prediction with a 7-row measured matrix
(`tmp/grind/func_800307D0/s8/variant_matrix.md`) in which every row behaves
exactly as the mechanism predicts (address-context + MULT index => 0; anything
else => 1).

## T5 family check
No forbidden family, and no sanctioned family is claimed or needed.
Specifically NOT or-tree-shape-shift: no operand of any expression was reordered
or reparenthesized to chase a score — the committed expression is written in its
natural reading order (base, then scaled index, then field offset), and the
address-context-vs-assignment-context difference is a structural difference in
the C, not a commutative shuffle. The owner directive's "one bounded re-test
under the 2026-08-20 or-tree carve-out" was executed and the carve-out turned out
to be INERT here (all written orders in the pointer domain lower identically —
s7 type matrix, re-confirmed by variants B2/D/E this session); it is therefore
NOT invoked, and no `/* FAKE */` annotation is required or present. The
reviewer-FAILED int-cast form `(s32 *)(v0 + (s32)a0)` is NOT used and remains in
`rejected/int-cast-operand-swap.c`; no pointer is cast to an integer anywhere in
the body. No dead store, no dead local, no alias, no volatile, no wrapper.

## T6 naming-announces-intent
No intent-announcing names. Locals are `count`, `s1`, `v0`, `v1`, `s3`, `a2`,
`i` — the register-flavoured naming convention already used throughout
`src/code6cac_b.c` for values without a confirmed semantic name. No `pad`,
`dummy`, `spill`, `_buf`, `slack`, `_frame_pad`. Every one is written and read.

SANCTIONED-FAMILY-CLAIMS: none — the body is ordinary C and claims no exception.

ANNOTATION-CONFORMANCE: n/a — no FAKE construct in the diff.

## Bytes (measured by me, this session)
- `& tools/wteng.ps1 main sandbox func_800307D0 --disable all` -> **score 0**,
  `target_insns 76 == build_insns 76`, `rules_dropped: 0`, `scorable: true`.
- `& tools/wteng.ps1 main build` -> sha1
  `62efab4f73f992798c43e8c730aa43baa10bb4fa` == oracle, **MATCH**.
- Zero regfix/asmfix rules for this function (asmfix.txt is empty repo-wide since
  2026-08-25); no `inline_asm_canonical.txt` entry; no `__asm__` in the body.
