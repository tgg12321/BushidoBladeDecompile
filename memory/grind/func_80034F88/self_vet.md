# SELF-VET — func_80034F88 (session 66, solver)

Measured this session: `sandbox func_80034F88 --disable all` = **score 0**,
target_insns 49, build_insns 49; and a full clean-driver `build` =
**SHA1 62efab4f73f992798c43e8c730aa43baa10bb4fa == oracle**.

CONSTRUCTS: (1) `u8 *q = &D_80106A73;` local pointer alias; (2) `u8 *r = &D_80106A73;` local pointer alias, reassigned once; (3) `u = 0;` dead store to a local (cse2 value invalidator); (4) NEW THIS SESSION — the existing local `q` reused as the copy loop's counter (`for (q = 0; (s32)q < 3; q++)`); (5) declaration change `extern u8 D_80106A70[3];` in include/code6cac.h with its two consumers in src/code6cac.c converted to element form.

## T1 semantic purpose
(1)(2) Both pointers are the address through which every read and write of the
flag byte in their block is performed; delete them and the function does not
compile. They have a real semantic purpose, but their SPLIT into two objects is
codegen-motivated, which is why both carry FAKE annotations (the Judge granted
exactly this two-object shape on 2026-09-05).
(3) `u = 0;` is genuinely dead — its stored value is never read; it exists only
so cse2 does not forward the preceding `sb` into the following `lbu`. It is
annotated FAKE and claimed under the dead-store family, not passed off as logic.
(4) The reuse is not dead: q's second life IS the loop counter, read three times
per iteration (source offset, destination index, bound test) and incremented.
Removing the reuse means introducing a second variable, i.e. the construct is a
choice of WHICH variable holds a real, live, used value — the value itself is
load-bearing.
(5) The array declaration is the DATA MODEL's own instruction for this symbol
(the target indexes D_80106A70 with a computed register). It is measured
byte-neutral project-wide and the full-build SHA1 proves it.

## T2 human-programmer
(1)(2)(4) are the ones a reader would question. (1)/(2): a human writes one
pointer, not two — this is why the two-object form is FAKE-annotated and was put
to the Judge, who PASSed it on 2026-09-05 after verifying in the target asm that
80034FC8 loads a SECOND copy of &D_80106A73 into $a0 while $v1 still holds the
first and is read by the store at 80034FD0. The target's own instruction stream
carries two live address registers; the two C objects are the transcription of
that, not an invention.
(4) Reusing a pointer local as an integer counter is NOT what a human would write
from the specification, and I do not claim otherwise. It is claimed as a
match-hack under the variable-reuse family, annotated FAKE, with the mechanism
and the exhaustion ledger named at the declaration. It is behaviourally exact:
the counter values 0, 1, 2 are the only values q holds in its second life, and q
is never dereferenced there.
(3) A human would not write `u = 0;` there either; annotated FAKE, dead-store
family.
(5) A human absolutely writes this: the symbol is a 3-element table copied by a
loop, and declaring it as an array is more honest than three splat scalars.

## T3 GCC-internals justification
Yes, and stated openly rather than hidden. (4) works because global.c's
allocation priority is floor_log2(nrefs)*nrefs*10000/live_length, and merging the
address object with the loop counter takes that allocno from 5 refs / pri 3571 to
16 refs / pri 30476 (measured, tmp/grind/func_80034F88/s66/z2.model.json), so it
is seated in $v1 before block 0's value allocno (pri 17500) is considered, and
the value then scans on to $a0 as the target has it. (3) works because of cse2
(cse.c) store-to-load forwarding. This is precisely why both carry
`/* FAKE: ... */` annotations naming the pass — the policy's required response to
a GCC-internals mechanism is to annotate it and claim a sanctioned family, which
is what is done here, not to present the construct as program logic.

## T4 permuter/search provenance
No permuter or auto-search was run this session. The construct came from the s65
ledger's own arithmetic (branch (A): the address object needs at least 16
references) plus the mandated solver reading of the target asm; it was spelled by
hand and then confirmed by extracting the real RA model with
tools/ra_solver/extract.py. It does not depend on any detector missing a
spelling.

## T5 family check
(1)(2) pointer-alias-fake-exception, and additionally covered by a dated Judge
PASS ruling on the two-object shape (2026-09-05, docs/grind/decisions.md).
(3) dead-store-fake-exception (a dead store to a LOCAL).
(4) variable reuse for codegen control — the first entry on the frozen
SOTN-accepted list, whose text names RA explicitly. It is a borrow of an EXISTING
local (bound 2 of staged-value-reused-variable): q exists for block 0's flag-byte
accesses and is not invented in order to be borrowed.
(5) ordinary C — an array declared as an array.
No construct in the diff matches a forbidden-family entry: there is no register
pin, no `__asm__`, no scheduling barrier, no volatile, no unused local or array,
no dead parameter assignment, no alias rename, no `if (1)` wrapper, no goto pad,
no redundant width cast, and no third C object aliasing &D_80106A73.
STANDING BAN CHECK: the banned construct recorded for this function is "the same
four `u8 *q = &D_80106A73;` handles, treated as ordinary program logic rather
than a register-allocation lever". This body has TWO such objects, not four, and
both are annotated as register-allocation levers — the shape the Judge granted on
2026-09-05.

## T6 naming-announces-intent
Names are `p` (the record pointer returned by func_80077D00), `q` and `r` (the
two address objects), and `u`, `v`, `c` (values). None is `pad`, `dummy`,
`unused`, `spill`, `slack` or similar, and every one of them is read.

SANCTIONED-FAMILY-CLAIMS:
  FAMILY: variable reuse for codegen control
  SCOPE: "reusing one C variable for two unrelated values to influence loop-invariant detection or RA."
  PRECEDENT: .claude/rules/no-new-park-categories.md:185

  FAMILY: pointer alias (second handle to a global)
  SCOPE: "NARROW SANCTIONED EXCEPTION (owner ruling 2026-07-01): a C-level local pointer alias (second handle) to a global — `Type* t = &g_Thing;`, `s16 (*p)[] = &D_xxx;`, FakePrim-style reinterpret views — is allowed as a LAST-RESORT lever with `/* FAKE: ... */` annotation + lever-exhaustion."
  PRECEDENT: .claude/rules/pointer-alias-fake-exception.md:5

  FAMILY: dead store / self-assignment to a LOCAL
  SCOPE: "NARROW SANCTIONED EXCEPTION (owner ruling 2026-07-01): a dead store / self-assignment to a LOCAL or PARAMETER, annotated `/* FAKE: ... */`, is allowed as a LAST-RESORT codegen lever after documented lever-exhaustion."
  PRECEDENT: .claude/rules/dead-store-fake-exception.md:6

ANNOTATION-CONFORMANCE:
  /* FAKE: block-0's own address object, also carrying the copy loop's counter below, mechanism: global.c allocation priority floor_log2(nrefs)*nrefs*10000/live_length -- the loop's eleven counter references lift this allocno from 5 refs / pri 3571 to 16 refs / pri 30476, so it is seated in $v1 before block 0's value allocno (pri 17500) is considered and that value scans on to $a0. lever-exhaustion: hypotheses.md s53-s65 (the reference-lift branch is s65's branch (A); its other spellings are banked dead). */
  /* FAKE: cse2 value invalidator, mechanism: cse2 (cse.c) forwards the sb into the following lbu only while the stored value's pseudo still holds it. lever-exhaustion: hypotheses.md s57-s62. */
  /* FAKE: the address object for flag blocks 1 and 2, mechanism: global.c:1275 assigns exactly one hard register per allocno and GCC 2.7.2 does no live-range splitting, so blocks 1/2 cannot be reached from the block-0 object. lever-exhaustion: as above. */
  All three carry what + a named GCC pass + a lever-exhaustion pointer. The
  variable-reuse construct (4) is annotated inside the first of these, at the
  declaration of the variable it reuses.

## RE-VERIFICATION STAMP (session 66, second dispatch)

The first s66 process wrote this vet and the ledger but exited without an
outcome JSON, so the driver discarded it and re-dispatched session 66 with
`src/` reverted to HEAD. This session re-installed the banked body
(`python3 tmp/grind/func_80034F88/s63/apply.py memory/grind/func_80034F88/candidate.c`)
and re-measured every claim above from scratch rather than trusting the
uncommitted files:

- `& tools/wteng.ps1 main sandbox func_80034F88 --disable all` →
  **score 0, target_insns 49, build_insns 49, scorable true, strip_cheat_asm true.**
- `& tools/wteng.ps1 main build` → SHA1
  **62efab4f73f992798c43e8c730aa43baa10bb4fa == oracle, MATCH.**
- All three cited rule locations were re-read and resolve:
  `.claude/rules/no-new-park-categories.md:185` is inside the
  "Variable reuse for codegen control" ALLOWED bullet and carries the quoted
  scope sentence verbatim; `.claude/rules/pointer-alias-fake-exception.md:5`
  and `.claude/rules/dead-store-fake-exception.md:6` are the two rules'
  `description:` lines and carry their quoted scope sentences verbatim.

Nothing in the vet above was changed; the measurements are re-derived, not
inherited.
