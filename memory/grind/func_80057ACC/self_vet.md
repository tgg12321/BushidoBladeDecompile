# SELF-VET — func_80057ACC

CONSTRUCTS: none (ordinary C only — a `for` loop nest over a polygon list, `short`
locals for the two loop counters and the two derived indices, pointer-arithmetic
casts in the file's established style, and a repeated `*(s32 *)(poly + 4)`
dereference written out at each of its four call-argument use sites).

## T1 semantic purpose
Every construct in the diff computes a value the function returns or stores.
- `best` (init 100000) is the returned minimum distance.
- `i`/`j` are the two loop counters; `k` is the wrap-around next-vertex index
  (`j + 1`, or 0 when it reaches the vertex count) and is dereferenced.
- `n` is the edge count for the polygon (vertex count, minus one when the 0x80
  "open polyline" flag is set) and bounds the inner loop.
- `poly` is the current 8-byte polygon record; `sp28`/`sp2C` receive the
  intersection point from func_8005763C via their addresses and are then read.
- `dx`/`dy`/`d` are the components and the length of the hit-point offset.
Removing any of them changes what the function computes or returns. No construct
is behaviour-neutral, so none is a codegen-only carrier.

## T2 human-programmer
Yes. Given the specification — "walk every polygon in the list; for each edge,
ask func_8005763C whether the segment (arg2,arg3)→(owner x,y) crosses it; keep
the nearest hit and record which polygon/edge it was" — this is the obvious
implementation. The `short` loop counters are the natural spelling for indices
into u8-counted tables, not a codegen device; the game's own index fields are
sub-word (the winning pair is stored back as two bytes at +0x360/+0x361).

## T3 GCC-internals justification
No. No construct in the diff is justified by a GCC pass. The only place I
reasoned about GCC at all was in DIAGNOSING the rejected first attempt
(int-typed counters with explicit `(s32)(s16)x` casts, score 91), where loop
strength reduction materialised shifted induction variables that consumed two
callee-saved registers and forced arg2/arg3 to memory. The fix was not to add a
construct but to declare the counters with the type the data model implies
(`short`), which is a source-level simplification, not a lever. The final body
would be written the same way by someone who had never looked at a dump.

## T4 permuter/search provenance
No search was run this session. The body was written by hand from the target
assembly's control flow and the callee's stack-argument layout; two iterations,
both hand-authored.

## T5 family check
No construct matches any forbidden family, by analogy or otherwise: no register
pins, no `__asm__`, no volatile, no barriers, no dead stores or self-assigns, no
dead or unused locals, no unused arrays or frame padding, no empty or
always-true conditionals, no gotos, no aliases of globals, no opaque
constant-holder variables, no redundant width casts inserted for codegen. The
repeated `*(s32 *)(poly + 4)` load in the call argument list is a plain source
expression evaluated where it is used (the value is genuinely re-read four times
in the target); it is not a coercion construct and needs no family grant.

## T6 naming-announces-intent
No name in the diff is `pad`, `dummy`, `unused`, `spill`, `slack`, `tail`, or
similar. `sp28`/`sp2C` are frame-offset-derived names inherited from the m2c
convention used throughout src/text1b.c; both are real out-parameters whose
values are read on the next three lines. Every other name (`best`, `poly`, `n`,
`i`, `j`, `k`, `dx`, `dy`, `d`) states what the value is.

SANCTIONED-FAMILY-CLAIMS: none — the body is ordinary C and claims no exception.

ANNOTATION-CONFORMANCE: n/a — no FAKE construct in the diff.
