# SELF-VET — func_80042874

CONSTRUCTS: (1) `u16 rawA;` staging local for the cosA table read + `cosA = (s16)rawA;`
sign-interpreting cast; (2) `s32 idxB = (s16)angB + 0x400;` named intermediate for the
cosB table index; (3) statement-order changes only — the real store `a1[7] = sinA;` moved
to sit between the rawA load and the (s16) cast, and the real reads
`angC = a0[2]; sinC = Judge[angC & 0xFFF];` moved to after the
`negSinAxsinB_12 = (sinA * -sinB) >> 12;` statement. The diff additionally DELETES two
cheats that HEAD carried: a `register s32 angC asm("$3")` pin and a
`(s16)*(volatile u16 *)(&Judge[...])` volatile coercion. No construct was added that HEAD
did not already have in a worse (cheating) form.

## T1 semantic purpose
- (1) `rawA`: YES, observable in the emitted code, and the target's own bytes are the
  evidence. With the staging local the function emits target's three-insn read
  `lhu v1,%lo(Judge)(at); sll v1,16; sra v1,16` (target insns 57/59/61) and the whole
  function is byte-identical (532/532 bytes, 0 non-reloc mismatches). Written as the
  direct `cosA = Judge[...]` read GCC emits a single `lh`, the function is 116 insns
  instead of 119, and the score is 6 (measured this session, both directions). So the
  construct is not behaviour-neutral decoration: it selects which of two genuinely
  different machine reads the compiler performs, and the ORIGINAL binary performs the
  u16-then-sign-extend one. The natural reading is that the original source read this
  table through an unsigned 16-bit value and interpreted it as signed — exactly what
  `u16 rawA; ... (s16)rawA` says.
- (2) `idxB`: YES — it names the cosB table index, which is a real sub-expression that is
  computed and used. Naming it is ordinary C authoring.
- (3) Statement order: the statements themselves are all load-bearing program logic
  (a real store to a1[7], the real read of a0[2] and the sinC lookup). Nothing was added
  or duplicated; only the order in which real work is written changed.

## T2 human-programmer
- (1) Yes. A programmer reading a hardware-ish `s16 Judge[]` sine table and wanting the
  signed cosine writes either the direct read or the "load raw, then interpret" pair; the
  latter is exactly what the immediately adjacent, already-accepted sibling
  func_80042A88 does (src/text1a_c.c:374-376) for the same table, same index expression.
- (2) Yes — hoisting a repeated-shape index expression into a named local is the most
  ordinary refactor there is, and the sibling does the same (`idxB` at
  src/text1a_c.c:359).
- (3) Yes. Nobody would ask "why is this store here?" — it stores a matrix element that
  the function must store; the only question a reader could ask is why it is written
  before the other eight, and the answer (it is the one element that does not depend on
  cosA) is a perfectly readable ordering.

## T3 GCC-internals justification
Partly, and I am flagging it rather than hiding it. My *derivation* of construct (1) cites
combine's `can_combine_p` refusing to combine a MEM across a possible memory write, so
`simplify_shift_const` never folds the zero_extend/ashift/ashiftrt chain into a
sign_extend. That reasoning came from the sibling's ledger and is why the store sits
between the load and the cast. What keeps this on the right side of the line is that the
mechanism explains an ordering choice among real statements, and the construct it protects
(the u16 read) is itself justified by the target bytes — the original binary contains the
lhu+sll+sra, so reproducing it is reproducing the original program, not defeating an
optimizer for its own sake. Construct (2) needs no GCC-internals story at all beyond
"name the sub-expression"; construct (3) adds no code.

## T4 permuter/search provenance
No permuter, no auto-search, no directed sweep was run this session. Every form was
hand-derived from (a) the target listing asm/funcs/func_80042874.s and (b) the accepted
COMPLETED-C body of the sibling func_80042A88 in the same file. Four forms were measured
by hand: HEAD-stripped 21, staging-only + split multiply 14, staging + combined multiply +
idxB 0, and no-staging 6.

## T5 family check
- (1) and (2) are the frozen-list family **Named-intermediate declaration order**
  (declare a sub-expression as a separately-named local). (1) additionally sits in the
  spirit of **Sub-word param reads** (a narrow-typed view of a value). Neither is a
  forbidden family: there is no register pin, no `$N` asm, no volatile, no alias rename,
  no dead store, no dead local (both new locals are written AND read), no wrapper with an
  empty body, no `if (1)`, no goto scaffolding, no frame coercion, no build-time rewriting.
  The diff REMOVES two members of forbidden families (register-asm pin,
  volatile-coercion cast).
- (3) is not a construct at all; "write the statements in this order" is not on any list,
  forbidden or sanctioned, because there is nothing to sanction — no code exists that
  would not exist otherwise.

## T6 naming-announces-intent
`rawA` and `idxB` name what they hold (the raw table word for angle A; the cos index for
angle B). Neither is `pad`/`dummy`/`unused`/`spill`/`tmp`/`slack`. Both are read: `rawA`
feeds `cosA`, `idxB` feeds the cosB lookup. No declaration is address-taken, discarded,
or unused.

SANCTIONED-FAMILY-CLAIMS:
  FAMILY: Named-intermediate declaration order
  SCOPE: "**Named-intermediate declaration order** ([[narrow-byte-args-packed-call]] hi/lo sub-trick): declare a sub-expression as a separately-named local to bias LUID. SOTN's `randy` chain in `src/weapon/w_037.c` is the same mechanism."
  PRECEDENT: .claude/rules/no-new-park-categories.md:189

  FAMILY: Named-intermediate declaration order (in-repo application of the identical
    construct, same file, same table, accepted as COMPLETED-C by this pipeline)
  SCOPE: "**Named-intermediate declaration order** ([[narrow-byte-args-packed-call]] hi/lo sub-trick): declare a sub-expression as a separately-named local to bias LUID. SOTN's `randy` chain in `src/weapon/w_037.c` is the same mechanism."
  PRECEDENT: 11ecbfa8

  FAMILY: Sub-word param reads
  SCOPE: "**Sub-word param reads** ([[narrow-stack-param-subword-offset]]): `*(u16 *)&local` cast to read a specific half-word. Standard C usage in SOTN."
  PRECEDENT: .claude/rules/no-new-park-categories.md:179

ANNOTATION-CONFORMANCE: n/a — no FAKE construct. Nothing in the diff is dead, empty, or
behaviour-neutral: both new locals are written and read, the relocated store is a required
matrix write, and every emitted instruction the constructs produce is present in the
target bytes.
