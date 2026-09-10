# SELF-VET — func_8005C6D0

CONSTRUCTS: one — `nv`, a fresh `s32` local declared in the function's declaration
block, written exactly once (`nv = off;`, the first statement of the voice-scan body)
and read at the two volume-byte argument sites of the `SsUtKeyOnV` call. It is a
second C name for the pool byte offset `i * 8`, carries a `/* FAKE: ... */`
annotation at its single write, and is claimed under the named-intermediate family.
Everything else in the diff is ordinary C: a top-tested `for` over the 24 pool slots,
a top-tested `for` scanning voices from the running `next` cursor, the `(u8 *)&SYM +
offset` addressing that the byte-matching sibling `func_8005C650` already ships on
main directly above this function, a `u16 *p` pool-entry pointer, a `u32 *ev` pointer
to the packed note word, and a `u16 vab`. No guard block, no dead store, no pad, no
array, no volatile, no inline asm, no register pin.

## T1 semantic purpose
`nv` holds a real value that is consumed: it is the byte offset of the current pool
entry, and the two `*((u8 *)&D_800EFB7{C,D} + nv)` reads are two of the eight
arguments actually passed to `SsUtKeyOnV`. Deleting the name does not delete the
value — the reads would simply have to be spelled `off` instead, so the construct's
observable effect is not "nothing" but "which pseudo carries the offset into the
volume reads". That relocation is visible in the shipped bytes: the target keeps a
second, callee-saved copy of the offset alive across the `SpuGetKeyStatus` call
(`addu $s2,$v1,$zero`, asm/funcs/func_8005C6D0.s:41, address 0x8005C768). The
construct is byte-neutral in count (118 target insns == 118 build insns) — it adds no
surplus instruction; it decides which register holds an instruction the target
already contains.

## T2 human-programmer
Partly. A human writing this function would naturally compute `i * 8` once and reuse
one name; the second name is not what a specification-only reader would produce, and
that is exactly why it is FAKE-annotated rather than presented as ordinary C. What a
reader would NOT ask "why is this here?" about is the value itself — `nv` is the pool
offset, used to read the pool entry's stored volumes, which is a self-explanatory
thing for a local to hold. The construct fails the naive form of this test and is
therefore submitted under a sanctioned family with the family's full prerequisite set
(annotation, named mechanism, documented exhaustion, layer-1 + Judge review), not as
ordinary C.

## T3 GCC-internals justification
Yes, and it is stated openly rather than hidden: the mechanism is that GCC 2.7.2
gives one C name one pseudo, so the offset can only occupy two registers
simultaneously — one caller-saved across the call ($v1, recomputed per pool slot) and
one callee-saved that survives it ($s2) — if the C names it twice; local-alloc /
global.c then assigns the second pseudo a callee-saved hard register because its live
range crosses `SpuGetKeyStatus`, and loop.c LICM sinks the copy into the voice-scan
preheader, which is exactly where the target emits it. This is the named-intermediate
family's own declared mechanism class, and the 2026-08-17 clarification in
.claude/rules/no-new-park-categories.md:208-216 states in terms that the family
qualifies "whatever GCC pass it acts through". The rule's prong (2) requires the
value to appear in the target's own bytes and only RELOCATE where it is named — which
is the case here (the copy insn is in the shipped bytes; it is not conjured by the
name), and that is what separates this from a no-op-copy dead store.

## T4 permuter/search provenance
The construct was NOT found by search and does not depend on any detector's blind
spot. It was hand-derived in s1/s2 from the target's own disassembly (the unexplained
`addu $s2,$v1,$zero` at 0x8005C768) before any campaign ran. A 6,562-iteration
decomp-permuter campaign was subsequently run from the score-8 single-name chassis and
converged independently on the same shape (`new_var = off;` as the first statement of
the voice-scan body) — corroboration that the bytes demand the second pseudo, not the
provenance of the construct. Nine hand-written single-name spellings were measured on
this chassis (scores 8, 8, 16, 18, 21, 22, 33, 35, 39; best 8 at 114 insns) and none
reaches 118 or emits the copy — the exhaustion is measured, not asserted.

## T5 family check
Named-intermediate declaration order, as amended. Checked against every neighbouring
forbidden family: it is not a variable REUSE / borrow (`nv` is fresh, declared for
this purpose, never carries another value — .claude/rules/staged-value-reused-variable.md
is not the governing file); not a dead store or self-assign (the write is consumed by
two live argument reads, so it is not store-level dead); not a constant holder (`off`
is a loop-varying value, not a literal); not a pointer alias to a global; not a
duplicated statement into arms (there are no arms); not an unused/written-never-read
local array or frame pad (`nv` is a scalar and it is read); not volatile coercion of
any kind; not a scheduling barrier, register pin, or asm injection. The layer-1 ban
recorded for this function names a semantically-null duplicated loop-condition guard
block — that block is ABSENT from this body (measured: the guard-free chassis reaches
distance 0 without it, hypotheses.md H8), and the Judge narrowed that ban accordingly
in the 2026-09-10 09:49 ruling.

## T6 naming-announces-intent
`nv` is not `pad`, `dummy`, `unused`, `spill`, `slack`, `tail`, or `_buf`; it does not
announce coercion intent, and its uses are neither discards, nor address-of, nor
declaration-only — it is read twice as call arguments. It is a short name in the style
of the surrounding file (`off`, `ev`, `p`, `vab` in the neighbouring already-matched
bodies). Its purpose is not concealed either: the `/* FAKE: ... */` annotation
immediately above the write states what it is and why.

SANCTIONED-FAMILY-CLAIMS:
  FAMILY: named-intermediate declaration order (as amended by owner ruling 2026-08-17
    and relaxed to once-written / any-number-of-reads by owner ruling 2026-08-31)
  SCOPE: "**Named-intermediate declaration order** ([[narrow-byte-args-packed-call]] hi/lo sub-trick): declare a sub-expression as a separately-named local to bias LUID."
  PRECEDENT: .claude/rules/no-new-park-categories.md:204
  PRECEDENT: docs/reference/sotn-construct-index.md:1427
  PRECEDENT: docs/grind/decisions.md:26644

ANNOTATION-CONFORMANCE: present at the construct's single write (src/text1b.c, the
first statement of the voice-scan body). Emitted text:
  /* FAKE: second name for the pool byte offset i*8, feeding only the
   * two volume-byte reads (named-intermediate family, .claude/rules/
   * no-new-park-categories.md SOTN-accepted list as amended by
   * .claude/rules/ordinary-c-judge-decidable.md Ruling 1);
   * mechanism: GCC 2.7.2 local-alloc/global.c gives one C name one
   * pseudo, so a single name can never produce the target's second,
   * callee-saved copy of the offset that survives the SpuGetKeyStatus
   * call (`addu $s2,$v1,$zero`, asm/funcs/func_8005C6D0.s:41,
   * 0x8005C768); loop.c LICM hoists this copy into the scan preheader
   * exactly where the target emits it;
   * lever-exhaustion: memory/grind/func_8005C6D0/hypotheses.md H9 +
   * evidence.md s2 - nine single-name spellings (8..39, all short of
   * 118 insns), fifteen guard-free arrangements, and a 6,562-iteration
   * decomp-permuter campaign that converged independently on this form. */
It carries all three required elements: WHAT (a second name for the pool byte offset
i*8, feeding only the volume reads), MECHANISM (named GCC passes — local-alloc /
global.c one-name-one-pseudo allocation across the call, loop.c LICM placement), and
LEVER-EXHAUSTION (an explicit pointer to hypotheses.md H9 and evidence.md s2, where
the nine single-name measurements and the campaign are recorded).
