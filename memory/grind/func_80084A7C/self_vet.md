# SELF-VET — func_80084A7C (grind session 3, structural, 2026-08-17)

Diff under vet: `src/main.c` func_80084A7C only (11 insertions / 11 deletions).
Honest floor with these edits: `sandbox func_80084A7C --disable all` = **score 0**,
target_insns 145, build_insns 145, 11 regfix rules dropped, 79 cheat-asm lines
stripped. The s1 normalized objdump diff (`tmp/grind/func_80084A7C/s1/diffit.py`)
shows the only remaining token difference is the unresolved `lui/addiu` relocation
pair for `D_80106F28` in the unlinked sandbox object (target `REL` vs `0`), i.e.
byte-clean at 145/145.

CONSTRUCTS: (1) two named entry intermediates `s32 shifted = a0 << 16;` and
`s32 *addr = (s32 *)&D_80106F28;` feeding `base_ptr` (inherited from session 2,
copied verbatim from the matched siblings in the same file); (2) REMOVAL of the
`s32 offset` local — the stride multiply `(s16)a1 * 0xB0` is now written out at
the `base` computation and at each of the eight `+0x98` flag-word sites, and cse
shares the one multiply chain. No construct was ADDED by this session; the net
change is the deletion of one local variable and the inlining of its real
arithmetic at its use sites.

## T1 semantic purpose
Both constructs carry the function's actual arithmetic.
(1) `shifted` holds `a0 << 16` and is consumed by `shifted >> 14` (the
    sign-extend-and-scale of the table index); `addr` holds the table base
    address. Both are read and both values appear in the emitted code.
(2) `(s16)a1 * 0xB0` at each site IS the entry's byte offset in the 0xB0-stride
    record table — the address the function must form to touch that record's
    flag word. Nothing is dead, nothing is a no-op, no arithmetic identity is
    inserted (contrast session 2's rejected `-((s16)a1 * -0xB0)`, which had no
    observable effect; that spelling is NOT in this diff).
Every construct here changes the function's computed addresses, so all have
observable effect.

## T2 human-programmer
Yes to both, and both are demonstrable in-tree idiom rather than my invention.
(1) The identical two-intermediate entry spelling appears in FOUR other
    already-matched functions in the same file: `src/main.c:304-305`
    (`spu_SetMotionState`), `596-597`, `615-616`, `628-629`. This is how this
    codebase spells a `D_80106F28` table access.
(2) Writing the stride expression at each use instead of caching it in a local
    is ordinary C, and the matched sibling `_SsSeqPlay` (`src/main.c:332`) spells
    the same table access with the multiply inline in the expression. A reader
    asks nothing about `*(s32 *)(*base_ptr + (s16)a1 * 0xB0 + 0x98)` — it reads
    as "the flag word of entry a1 of channel a0's record table", which is
    exactly what it is. If anything the new form is simpler than the old one: it
    has one fewer local.

## T3 GCC-internals justification
No. The constructs are justified by program logic alone (see T1/T2) and I can
describe each without naming a GCC pass: "form the record address from the table
entry and the channel's stride offset". GCC internals appear in this session's
ledger only as the *diagnosis* of why the PREVIOUS form missed — the `offset`
local's global allocno tying with base_ptr's on `allocno_compare`'s priority so
the hard-reg choice fell to the allocno-number tiebreak — but the submitted C is
not shaped around an internals trick: it is the un-cached spelling of the same
expression, which happens to remove the tied allocno entirely because the value
no longer needs a named holder. There is no construct here whose only
description is a compiler-pass effect.

## T4 permuter/search provenance
The form was found by a cc1-only structural sweep I wrote this session
(`tmp/grind/func_80084A7C/s3/sweep.py`, 50 variants across 9 batches) — not by
decomp-permuter, and not by a randomizer. Crucially the winning change is a
DELETION: no construct survives in the diff that exists only because a search
said so. The form stands on its own reading (T2) and would be the natural first
draft of this function had the `offset` handle never been introduced by the
original m2c-derived decompilation.

## T5 family check
No forbidden family is touched, by construction or by analogy: no register-asm
pin, no `__asm__` of any spelling, no scheduling barrier, no volatile of any
form, no alias rename, no dead store / self-assign / dead conditional store, no
unused or written-never-read local, no dead parameter assign, no `do{}while(0)`
or other no-semantic-purpose wrapper, no arithmetic no-op (the session-2
negation pair is explicitly absent), no goto/label restructuring, no width-cast
padding, no regfix/asmfix/linker/rules-file change of any kind. The `(s16)`
casts are the pre-existing sign-extension of an `s16` parameter used in an `s32`
address computation and are unchanged in count and meaning from HEAD.

## T6 naming-announces-intent
No new names. The diff DELETES a name (`offset`) and keeps two pre-existing ones
(`shifted`, `addr`) whose spelling matches four matched siblings in the same
file. Nothing is named `pad`, `dummy`, `unused`, `spill`, `tmp`, `fake` or
similar, and every remaining local is read.

SANCTIONED-FAMILY-CLAIMS:
  FAMILY: Named-intermediate declaration order
  SCOPE: "declare a sub-expression as a separately-named local to bias LUID."
  PRECEDENT: `.claude/rules/no-new-park-categories.md:189`
  (Claimed only defensively for construct (1), the inherited `shifted`/`addr`
  pair. I do not believe the claim is even needed: both locals hold live values
  that are read, and the spelling is copied from four matched functions in the
  same translation unit — `src/main.c:304` is the nearest. Construct (2) claims
  no family because it adds nothing; it removes a local.)

ANNOTATION-CONFORMANCE: n/a — no FAKE construct. Nothing in the diff has "no
observable effect", so no `/* FAKE: ... */` annotation is required or present.
