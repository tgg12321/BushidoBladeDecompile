# func_8003FA24 hypotheses

## H1 — packed data model (confirmed semantically, candidate floor 129)

The function consumes a point list selected through `D_80103608`, copies three halfwords per point into 8-byte output records, then walks two passes over a command stream. The first pass computes `point_count * 16 + vertex_count * 64`; the second emits type-3 or type-4 scratchpad packets using strides from `D_80094AEC = {12, 12, 18, 22}`. The candidate reproduces these data effects and reaches 262/263 instructions.

## H2 — phase-specific counter split (killed)

Splitting vertex, scan, and packet counters into separate semantic locals regressed the honest score from 129 to 137 and reduced the build to 261 instructions. Rejected.

## H3 — signed packet-stream pointer (killed)

Using a dedicated `s16 *` walker for the second pass regressed the score to 140 and reduced the build to 259 instructions. Rejected.

## H4 — named stride and field-selector intermediates (neutral)

Introducing truthful branch-local pointers to the stride-table entry and a named field selector compiled byte-identically to the 129 floor. They do not close the residual.

## H5 — generic permuter proposal (rejected as dishonest)

The lowest internal permuter proposal inserted `count = mode` inside the type-3 packet loop and then indexed the stride table through `count`. This changes the live loop count and therefore program behavior. It was rejected under the semantic-lie / carrier rules without integration or engine scoring.

## Frontier

1. Re-derive the packet loop from a source-level packet writer abstraction that keeps every store live while reproducing the target's non-hoisted 4/2/3 constants.
2. Establish the authentic full size of the initializer object passed to `func_80017D84`; do not fill the 0x48→0x50 frame gap with fabricated padding.
3. Once source-level instruction count reaches 263, address the three register cycles in the copy/scan phases through truthful declaration and lifetime changes.

