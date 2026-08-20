# SELF-VET — func_80038170

CONSTRUCTS: none

The diff replaces `INCLUDE_ASM("asm/funcs", func_80038170);` in src/code6cac_c_mid.c
with an ordinary C body. It is the only file touched. Every statement in the body
computes or stores a value that the function's output buffer or a later statement
actually consumes. There is no local that is written and never read, no array that
exists only to occupy frame bytes, no `volatile`, no `__asm__` of any kind, no
`register ... asm("$N")`, no regfix/asmfix rule, no scheduling or ordering device,
and no annotation-bearing construct. Nothing in the diff is present for a
byte-shaping reason; the body is what the function does.

## T1 semantic purpose
Every construct in the body has observable effect on the function's output.
- `s32 s1, s2, s3;` + `s3 = 0; s2 = 0; s1 = 0;` — three counters, incremented in the
  `switch (v)` inside the scan loop and then read: `s1` at `out[0x22]/out[0x23]`,
  `s2` at `out[0x3C]/out[0x3D]`, `s3` both as the `if (s3 > 0)` predicate and as the
  index at `out[0x42]/out[0x43]`. Delete any one initialisation and the function reads
  an uninitialised counter — a behavioural change, not a byte-level one.
- The mask scan (`mask = D_80106A50;`, `bit = 1 << i`, `mask & bit`) selects which of
  the 0x1B `D_8008F204[]` entries are tallied. Removing it changes the counts.
- The two `do { ... } while` zero-fill loops clear `out[4..0x43]` and `out[0x44..0x5F]`;
  the `strcpy(out + 4, D_8008F1C0)` writes the name field; the two copy loops move
  0x10 halfwords from `D_800109EC` to `out+0x60` and 0x40 halfwords from `D_80010A2C`
  to `out+0x80`. All are the function's actual output.
- `(&D_8008F19C)[s3 * 2 + 0]` / `[s3 * 2 + 1]` read the two bytes of entry `s3` of a
  2-bytes-per-entry table whose base symbol is `D_8008F19C`. Both bytes are stored to
  `out[0x42]/out[0x43]`. Sharing one base for the two halves of one table entry is how
  the table is actually laid out; the parenthesised `&` is only what C requires to index
  from a symbol the header declares as a scalar byte (a splat per-byte auto-symbol), and
  the header is out of scope for this candidate so the declaration is left as-is.

## T2 human-programmer
Yes. Given the specification — tally three categories over a 27-bit mask, build a
0x100-byte record, copy two fixed tables into it — a human writes exactly this: three
counters zeroed and counted, a masked scan, memset-style clears, a strcpy, four
table-indexed byte stores, two copy loops. The only line a reader might query is the
`(&D_8008F19C)[...]` parenthesisation, and the answer is a plain C one (the symbol is
declared as a byte, the data is a table) with no reference to codegen. Nothing in the
body would draw a "why is this here?".

## T3 GCC-internals justification
No construct in the diff is justified by a compiler internal. The ledger DOES record a
compiler-side observation — that indexing both halves of the entry off one base is what
makes GCC 2.7.2 stage the address in a stack temp, which is why the frame is -0x38 — but
that is an explanation of an outcome, not the reason the code is written this way: one
table entry read through one table base is the semantically correct way to express the
access regardless of what the compiler then does with it. The alternative the ledger
rejects (two unrelated scalar symbols for the two halves of one entry) is the WORSE
description of the data, not merely the one that misses the frame. No pass name is load
bearing anywhere in this diff, and no construct was added, kept, or ordered to steer an
allocator, a scheduler, DCE, or RTL emission.

## T4 permuter/search provenance
No search tool produced any part of this body. No permuter campaign was ever run on this
function (recorded in evidence.md, s4a). The body was derived by reading the target asm
and writing the C it corresponds to; the two things earlier sessions had added as
deliberate devices — a statement-order hoist in the scan loop, and a companion header
retype — were each removed after being measured unnecessary, and the form still reaches
the same bytes. Nothing here survives because a detector missed it; there is nothing to
detect.

## T5 family check
No forbidden family is matched, by analogy or otherwise. Concretely: no register-asm
pin; no `__asm__` at all, with or without `$N`; no regfix/asmfix rule (the sandbox
reports `rules_dropped 0`); no scheduling barrier; no volatile of any kind, so no
alias-rename / cast / extern coercion; no local array, dead or otherwise; no
write-never-read local or parameter; no self-assign or same-value re-store; no
always-true wrapper, no `do { } while (0)`, no dead `goto` or label pad; no opaque
constant holder; no pointer alias to a global introduced for codegen (`u8 *p = out +
0x3F;` and the copy-loop cursors are ordinary walking pointers over the function's own
output buffer and source tables, each dereferenced every iteration); no aggregate merge;
no width cast added anywhere. The earlier sessions' two rejected devices (a chained
zeroing assignment, and reversing the declaration order of the three counters) are absent
— the declaration order is the natural one and the three initialisations are separate
statements in the order the counters are used for the record's fields.

## T6 naming-announces-intent
No name in the diff announces coercion intent. Locals are `s1`/`s2`/`s3` (the three
counters, names inherited from the existing ledger form), `i`, `j` (loop counters),
`mask`, `bit`, `v` (the scanned entry value), `p`, `src`, `dst`, `outer_src`,
`outer_dst` (cursors). None is a `pad`/`dummy`/`unused`/`spill`/`slack`/`tail` shape,
and every one of them is read after being written — there is no declaration whose only
uses are a discard, an address-of, or the declaration itself.

SANCTIONED-FAMILY-CLAIMS: none

ANNOTATION-CONFORMANCE: n/a — no FAKE construct
