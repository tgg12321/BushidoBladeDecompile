# SELF-VET — func_80038170

CONSTRUCTS: none

The diff touches exactly two files, both inside this function's granted scope
(`tools/grinder/scope_allow.txt:26`):

1. `src/code6cac_c_mid.c:281` — the `INCLUDE_ASM("asm/funcs", func_80038170);`
   line is replaced by an ordinary C function body (the banked
   `memory/grind/func_80038170/candidate.c` body, unchanged).
2. `include/code6cac.h:80-81` — a one-line type correction of a shared
   declaration: two splat per-byte auto-symbols standing for one stride-2 byte
   table are replaced by that table's own array declaration, so the function can
   index it in plain C.

Nothing else changes. The body contains no `volatile`, no `__asm__` of any kind,
no `register ... asm("$N")`, no regfix/asmfix rule (`rules_dropped 0` in the
sandbox result), no scheduling or ordering device, no frame-shaping local, no
`/* FAKE */` annotation, and claims no sanctioned family. Every statement in it
computes or stores a value the output buffer or a later statement consumes.

## T1 semantic purpose
Every construct in the body has an observable effect on the function's output.
- `s32 s1, s2, s3;` with `s3 = 0; s2 = 0; s1 = 0;` — three counters, incremented
  in the `switch (v)` inside the scan loop and then read: `s1` at
  `out[0x22]/out[0x23]`, `s2` at `out[0x3C]/out[0x3D]`, `s3` both as the
  `if (s3 > 0)` predicate and as the index for the final two record bytes. Delete
  any one initialisation and the function reads an uninitialised counter — a
  behavioural change, not a byte-level one.
- The mask scan (`mask = D_80106A50;`, `bit = 1 << i`, `mask & bit`) selects which
  of the 0x1B `D_8008F204[]` entries are tallied. Removing it changes the counts.
- The two `do { ... } while` zero-fill loops clear `out[4..0x43]` and
  `out[0x44..0x5F]`; `strcpy(out + 4, D_8008F1C0)` writes the name field; the two
  copy loops move 0x10 halfwords from `D_800109EC` to `out+0x60` and 0x40
  halfwords from `D_80010A2C` to `out+0x80`. All are the function's real output.
- The three table reads (`D_8008F1A8[s1*2+n]`, `D_8008F1A8[s2*2+n]`, and the
  0x8008F19C-based table at `[s3*2+n]`) fetch the two bytes of one entry of a
  2-bytes-per-entry byte table and store them into the record. Indexing an entry
  of a stride-2 table from that table's base is what the access IS.

The header line has no runtime semantics of its own; it makes the C declaration
agree with the object the data actually is, which is a prerequisite for writing
the read above in plain C.

## T2 human-programmer
Yes. Given the specification — tally three categories over a 27-bit mask, build a
0x100-byte record, copy two fixed tables into it — a human writes exactly this:
three counters zeroed and counted, a masked scan, memset-style clears, a strcpy,
three table-indexed entry reads, two copy loops. And a human declaring a
2-bytes-per-entry byte table in a shared header writes it as an array, exactly as
the sibling table one entry later (`D_8008F1A8[]`, same header, read by this same
function with the identical `[sN*2+0]`/`[sN*2+1]` shape) is already declared.
There is no line here a reader would ask "why is this here?" about.

## T3 GCC-internals justification
No part of the diff is justified by a compiler internal, and the diff would be
written the same way if the compiler did not exist. This session DID run the
instrumented cc1 and the `-da` dumps, and they do name a pass chain (expand
refuses `reg + CONST(PLUS(symbol_ref,1))` as a MIPS address → combine folds the
address back into the mem and orphans the setter → `global.c` forms no allocno
for the SET-less pseudo → `reload1.c` `alter_reg`'s `from_reg == -1` arm reserves
an 8-byte stack slot, giving `vars= 16` / `.frame $sp,56`). But that chain is
used here as EVIDENCE ABOUT THE ORIGINAL SOURCE, not as a mechanism the code
exploits: the target's own `addiu $sp,$sp,-0x38` prologue can only arise if the
original C expressed the odd byte as a source-level `+1` addend on one symbol,
i.e. the original indexed one stride-2 array. The forensics tell us which data
model the original had; the code then simply describes that data model. Nothing
was added, kept, reordered, or renamed to steer an allocator, a scheduler, DCE,
or RTL emission, and the declared type of the base is itself byte-inert (ledger
s4b: both spellings of the same read emit identical assembly).

## T4 permuter/search provenance
No search tool produced any part of this. No permuter campaign was ever run on
this function (`evidence.md`, s4a). The body was derived by reading the target
asm and writing the C it corresponds to. The two devices earlier sessions had
added deliberately — a statement-order hoist in the scan loop, and a chained
zeroing assignment — were each measured unnecessary and are gone; the form still
reaches the same bytes. Nothing here survives because a detector missed it.

## T5 family check
No forbidden family is matched, by analogy or otherwise. Concretely: no
register-asm pin; no inline asm, with or without `$N`; no regfix/asmfix rule; no
scheduling barrier; no `volatile` anywhere, hence no alias-rename / cast /
extern coercion of any kind; no local array, dead or otherwise; no
write-never-read local or parameter; no self-assign or same-value re-store; no
always-true wrapper, no `do { } while (0)`, no dead `goto` or label pad; no
opaque constant holder; no C-level pointer alias standing in for a global (the
`u8 *p = out + 0x3F;` cursor and the copy-loop cursors are ordinary walking
pointers over the function's own buffer and source tables, dereferenced every
iteration, advanced every iteration); no aggregate merge into a struct; no width
cast added anywhere.

The header line deserves its own answer, because the family question there is
real. It is NOT the aggregate-merge family: nothing is merged into a struct, no
new type is introduced, and no per-word splat scalars are gathered under a
fabricated object model. It is a declaration whose element type, base address,
and stride are all unchanged from what the linked image already contains — the
only thing that changes is that C now knows the object has more than one
element. The two supports are (a) the frame-size forensics above, which are
recovered evidence about the original source's data model rather than a
byte-shaping argument, and (b) the identically-shaped sibling table in the same
header that is already declared this way. This is the same reason the same
header already carries a scope grant for `replay_camera_Init`
(`tools/grinder/scope_allow.txt:23`). And it is byte-inert: the s4b measurement
shows the emitted assembly does not depend on how the base is declared, so the
correction cannot be a coercion device — it buys legitimacy, not bytes.

## T6 naming-announces-intent
No name in the diff announces coercion intent. Locals are `s1`/`s2`/`s3` (the
three counters, names inherited from the ledger form), `i`, `j` (loop counters),
`mask`, `bit`, `v` (the scanned entry value), `p`, `src`, `dst`, `outer_src`,
`outer_dst` (cursors). None is a `pad` / `dummy` / `unused` / `spill` / `slack` /
`tail` shape, and every one is read after being written — there is no
declaration whose only uses are a discard, an address-of, or the declaration
itself. No symbol is renamed and no alias name is introduced.

SANCTIONED-FAMILY-CLAIMS: none

ANNOTATION-CONFORMANCE: n/a — no FAKE construct
