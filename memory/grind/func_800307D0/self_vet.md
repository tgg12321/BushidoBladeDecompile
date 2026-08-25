# SELF-VET — func_800307D0 (s10, 2026-08-25)

Diff: `src/code6cac_b.c:1159` `INCLUDE_ASM("asm/funcs", func_800307D0);` replaced by an
ordinary C body. Nothing else in the tree is touched. sandbox --disable all = **0**
(76/76, rules_dropped 0, scorable), full `build` SHA1 == oracle
`62efab4f73f992798c43e8c730aa43baa10bb4fa` — both measured THIS session with the edit in place.

CONSTRUCTS: (1) parameter typed `u8 *a0`; (2) inline scaled-index dereference
`*(s16 *)(a0 + s1 * 2 + 0x332)`; (3) forward `goto do_sll;` from two early guards to a shared
label; (4) index-off-a0 `for` copy-down loop `*(u16*)(a0+0x332+i*2) = *(u16*)(a0+0x334+i*2)`;
(5) `s1 = (u32)v0 < 1;` boolean-to-int; (6) per-arm block-local `s32 a0_arg = ... ^ ...;`
feeding `func_80032854(a0_arg != 0, ...)`.

## T1 semantic purpose
Every construct carries the function's real observable behaviour and none is removable:
(1) `u8 *` is the actual pointee width of every access in the body (all offsets are byte
offsets into the same object the siblings take as `u8 *`); (2) reads the queue head element
that is both returned and passed to `func_80030580` — delete it and the function has no
return value; (3) the two guards genuinely skip the xor/selection block, which is real
control flow present in the target; (4) the loop is the queue compaction (shifts every
remaining entry down one slot) — the whole point of the function; (5) computes the selected
slot index from the xor comparison; (6) computes the flag argument actually passed to
`func_80032854`. No construct is behaviourally inert.

## T2 human-programmer
Yes to all six. A programmer writing "pop the front of a 16-bit queue at +0x330/+0x332,
shifting the rest down, then notify" writes exactly this. Construct (2) is not merely
human-writable in the abstract — it is **this translation unit's own dominant spelling for
this exact array**, present verbatim in byte-matched zero-rule COMPLETED-C code:
`src/code6cac_b.c:1123` (func_8003047C, `*(s16 *)(a0 + 0x332 + i * 2)`),
`src/code6cac_b.c:1226` and `:1239` (func_80030B10, same `u8 *` param, same 0x332 array),
and `src/code6cac.c:2002` (func_80021904, identical read-into-a-named-local shape). Construct
(4) mirrors the direct sibling `func_80030900` loop that the s2 cheat-reviewer already PASSED
inside this function's own candidate. A reader would ask "why is this here?" about none of it.

## T3 GCC-internals justification
No. The C stands on its own program logic — I can describe every line without naming a GCC
pass, and did so under T1/T2. Compiler-internals material exists in this function's ledger
(the s8/s9 `expr.c` EXPAND_SUM finding, the `optabs.c:399-421` negative check the owner
directive asked for), but it is **diagnostic history, not the justification**: the reason the
head read is spelled this way in the submitted body is that it is the file's idiom for this
array, transplanted from the byte-matched siblings above, and the same shape is what an
independent mechanical decompiler (`tools/m2c/m2c.py`, fresh run in s9) reconstructs from the
target asm with no knowledge of this grind. This is exactly the distinction the 2026-08-25
15:45 ruling drew when it narrowed the s8-derived ban.

## T4 permuter/search provenance
No construct here came from auto-search. The permuter axis was measured DEAD across three
structurally-distinct basins (~144k iterations, s4/s5) and never produced any of this. The
body is the s2–s7 reviewer-PASSed floor-1 candidate with the head read and the parameter type
transplanted from named sibling functions. Banned construct #2 (the s8 7-row enumerated
"variant matrix" offered as justification) is NOT invoked, cited, or relied on anywhere in
this session's reasoning or records.

## T5 family check
No forbidden family is matched, by analogy or otherwise. Specifically NOT the banned
`v0 + (s32)a0` int-cast operand-order coercion: there is **no cast of the pointer to an
integer type anywhere in the diff** — `a0` is a `u8 *` from parameter to last use, and the
add is ordinary pointer arithmetic in a dereference. No register pins, no `__asm__`, no
volatile, no aliases, no dead stores, no dead locals, no pad arrays, no `do {} while (0)`,
no scheduling barriers, no build-time rewriting. Banned construct #4 is likewise not invoked:
I make no mootness declaration about the 2026-07-22 OWNER-ESCALATION — it was returned to
active grinding by the owner's own 2026-08-24 ruling (`.claude/rules/escalation-not-parked.md`),
and the s8-derived line ban was narrowed by the 2026-08-25 15:45 ruling
(`docs/grind/decisions.md:11151`), not by me.

## T6 naming-announces-intent
No name in the diff is a coercion tell. There is no `pad`/`dummy`/`unused`/`spill`/`slack`/
`_buf`/`tail`. The locals `count`, `s1`, `v0`, `v1`, `s3`, `a2`, `i`, `a0_arg` are plain
value-carrying names (several are register-derived placeholders in the decomp's usual style,
matching neighbouring functions in this file); every one is written AND read, and none exists
solely to be discarded, address-taken, or declared.

SANCTIONED-FAMILY-CLAIMS: none — this is ordinary C requiring no exception, per the
2026-07-19/21 initDrawMode precedent (`docs/grind/decisions.md:1156`) and the 2026-08-25 15:45
ruling on this function (`docs/grind/decisions.md:11151`).

ANNOTATION-CONFORMANCE: n/a — no FAKE construct in the diff.
