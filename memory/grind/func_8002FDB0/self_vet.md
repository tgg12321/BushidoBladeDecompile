# SELF-VET — func_8002FDB0

CONSTRUCTS: (1) three multi-instruction canonical GTE `__asm__ volatile` islands
reproducing the PsyQ libgte macro bodies gte_SetRotMatrix / gte_ldlvl /
gte_stlvnl (each opens `move $12, %0` and then uses hardcoded $12-$15 around
ctc2 / lwc2 / swc2); (2) one single-instruction `__asm__ volatile(".word
0x4B70000C")` cop2 OP command; (3) two additional ordinary locals `w1`, `w2`
used for the sixth subtract block.

Constructs REMOVED by this diff (all cheats, none re-added): every
`register T x asm("$N")` pin in the function, every `__asm__ volatile("move %0,
%1" ...)` INLINE_MOVE_ALIASING block, and the `__asm__ volatile("" ::: "$5")`
scheduling barrier.

## T1 semantic purpose
(1) and (2): yes. These islands ARE the computation — they load the rotation
matrix into cop2 control registers, load the difference vector into IR1-IR3, run
the GTE outer-product command, and store MAC1-MAC3 back to scratchpad. Delete
them and the function computes nothing and returns garbage. cop2 register
transfers and cop2 commands have no C form at all.
(3): yes. `w1`/`w2` hold the two loaded words of the sixth difference component
and are read by the `w1 - w2` that is stored to SCR[0x378]. Both are live, both
are read, neither is a dead store or a declaration-only holder.

## T2 human-programmer
(1) and (2): yes, emphatically — a PS1 programmer writes `gte_SetRotMatrix(m);
gte_ldlvl(v); gte_op0(); gte_stlvnl(out);`. What is in the source here is those
SDK macros' own bodies written out, because this project deliberately does not
carry Sony's named-helper macro set (see the comment at `include/gte.h:91-96`).
A reader asking "why is this here?" gets the answer "because that is how you
drive the GTE", not a codegen answer.
(3): yes. Using distinct names for a distinct pair of values is the DEFAULT
spelling; reusing one variable for six unrelated pairs is the unusual choice.
Nothing about `w1`/`w2` announces a codegen purpose.

## T3 GCC-internals justification
The justification for (1) and (2) references no GCC pass. It is a claim about
the ORIGINAL SOURCE: the target bytes contain three redundant
materialize-then-copy sequences, a fixed $12-$15 footprint repeated three times,
and two unfilled cop2 load-delay nops — shapes no GCC pass emits, and which
match the published PsyQ inline_c.h macro text instruction for instruction. That
is hand-written-SDK-asm evidence, not an allocator or scheduler argument.
For (3) the honest answer is that the mechanism IS register allocation: `stride`
takes its last use as the sixth block's index, freeing $v0, and a fresh allocno
takes it where the five-block-old recycled pseudo would not. But (3) is not a
no-semantic-purpose construct being justified by that mechanism — it is two
ordinary live locals whose only "trick" is that they are not reused. The
mechanism explains why the natural spelling is also the matching one; it is not
load-bearing for the construct's admissibility.

## T4 permuter/search provenance
No permuter, no directed search, no automated sweep was run this session. (1)
and (2) were derived by reading `asm/funcs/func_8002FDB0.s` and recognizing the
PsyQ macro bodies. (3) was derived from the 3-instruction residual's register
pattern by inspection of the objdump. Nothing here exists only because a search
found it.

## T5 family check
The forbidden family in scope is "hardcoded-`$N` `__asm__` injection". Its
defining intent (per `.claude/rules/inline-asm-injection.md`) is emitting a
general-purpose instruction that GCC's optimizer ate, with the exact registers
target uses, to close a codegen shortfall — a regfix `insert_after` in a
different file. That is NOT what these islands are: they emit cop2 transfers and
commands that have no C form, and the `move $12,%0` / `lw $13,0($12)` scaffolding
is the SDK macro's own addressing preamble for those transfers, not a
reconstruction of instructions GCC dropped. The distinguishing test is whether a
pure-C form of the SAME operation exists — for `addu $8,$3,$zero` it does
(`a = b;`); for `ctc2 $13,$0` it does not.
`w1`/`w2` match no forbidden family: not a dead store, not a self-assign, not a
constant holder, not an unused declaration, not an alias, not a pin.
I record the residual risk honestly: the scaffolding instructions inside the
islands (`move`, `lw`) are general-purpose opcodes, and that is exactly why this
session's disposition is an owner-gated integration handoff rather than a
self-approved completion.

## T6 naming-announces-intent
No construct is named `pad`, `dummy`, `unused`, `spill`, `tail`, `slack`, `_buf`
or similar. `w1`/`w2` are value names in the same style as the pre-existing
`v1`/`v2`, and both are read. No address-of, no `(void)` discard, no
declaration-only local anywhere in the diff.

SANCTIONED-FAMILY-CLAIMS:
  FAMILY: canonical inline asm (GTE/cop2 ops with no C form)
  SCOPE: "Inline `__asm__` in a C function body using opcodes that ONLY EXIST in asm form: GTE coprocessor ops (`ctc2`/`mtc2`/`mfc2`/`lwc2`/`swc2`/`.word 0x4XXXXXXX`), BIOS vector jumps (`j 0xA0`/`B0`/`C0`), cache/DMA register pokes (`0x1F8003xx`). Authentic — original devs wrote these."
  PRECEDENT: inline_asm_canonical.txt:263

ANNOTATION-CONFORMANCE: n/a — no FAKE construct. Nothing in this diff is a
no-semantic-purpose coercion, so no `/* FAKE: ... */` annotation applies; every
construct is either the GTE computation itself or an ordinary live local.
