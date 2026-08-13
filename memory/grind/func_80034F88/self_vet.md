# SELF-VET — func_80034F88

Diff scope: `src/code6cac_b.c` lines 3899-3956 (the whole body of
func_80034F88) — nothing else in the tree is touched.  Honest score measured
this session: `sandbox func_80034F88 --disable all` → **score 0**, 49 build
insns vs 49 target insns, `rules_dropped: 30`, `cheat_asm_stripped: 317`.

The new body, in full:

```c
void func_80034F88(void) {
    s32 *p;
    s32 i;

    p = func_80077D00();
    {
        u8 *q = &D_80106A73;

        *q &= 0xF8;
    }

    {
        u8 *q = &D_80106A73;
        s32 v;
        s32 c;

        c = p[8] & 1;
        v = *q;
        if (c) {
            c = v | 1;
        } else {
            c = v;
        }
        *q = c;
    }
    /* ... the same block again for bit 2 and for bit 4 ... */

    for (i = 0; i < 3; i++) {
        *(&D_80106A70 + i) = *((u8 *)p + i + 0x17);
    }
}
```

The previous body in `src/` carried THREE `asm volatile("" ::: "memory")`
scheduling barriers (a forbidden family).  This diff DELETES all three; it adds
no inline asm, no register pin, no volatile qualifier, no cast, no dead store,
no unused declaration and no re-assignment of a variable to a value it already
holds.  It also removes the construct s10 and s11 both flagged as unvetted (the
second `ptr2 = &D_80106A73;` redundant re-assignment) — that construct is NOT
present in this form and the 9- and 13-point bodies that carried it are
superseded.

CONSTRUCTS: block-scoped `u8 *q = &D_80106A73;` local pointer in each of the
four flag blocks (read and written through); block-scoped `s32 v` holding the
loaded flag byte; block-scoped `s32 c` holding first the tested flag bit and
then the byte value to be stored (a REUSE of one local for two values);
two-arm `if (c) { c = v | K; } else { c = v; }` select; `*q &= 0xF8;` mask;
the pre-existing copy loop `*(&D_80106A70 + i) = *((u8 *)p + i + 0x17);`
(unchanged from the s3-era body, not introduced here).

## T1 semantic purpose
- `q`: yes.  It is the address the block's `lbu` and `sb` both use; delete it
  and the block cannot read or write the byte.  Observable.
- `v`: yes.  It holds the loaded byte and is consumed in BOTH arms of the
  if/else (`v | K` and `v`).  Delete it and the value being stored is wrong.
- `c`: yes, in both of its roles.  Its first value (`p[8] & K`) decides the
  branch; its second value is the byte that is stored.  Both are read.  There
  is no assignment to `c` whose value is never used, and no path on which `c`
  is written and then not stored.
- The `if/else` with an explicit else arm: yes — the else arm supplies the
  stored value on the not-set path.  Removing it changes what is stored.
- `*q &= 0xF8;`: yes — it clears the three flag bits before they are recomputed.
- Block scoping: it has no effect on the function's OUTPUT, but it is not a
  construct in the coercion sense — it declares no object that is unused and
  emits nothing.  It is the ordinary C spelling of four self-contained,
  copy-pasted flag blocks.  Every object it declares is read and written.

## T2 human-programmer
Yes for every construct.  The function's specification is "clear the three
flag bits of D_80106A73, then set each of them from the corresponding bit of
p[8], then copy three bytes from p+0x17 to D_80106A70".  Three near-identical
copy-pasted blocks, each with its own local pointer to the byte it is editing
and its own scratch locals, is the ordinary way that gets written in this
codebase's era and style; `src/code6cac_b.c` already contains sibling
functions written as repeated per-flag blocks with local pointers into the
same region (e.g. func_8003504C immediately below, which declares `u8 *src;
u8 *ptr; s8 *base;` and walks the same bytes).  A reader would not ask "why is
this here?" about any line: every declaration is used, every statement's value
is stored or branched on.
The one line a reader might question is the double duty of `c` — it is a
condition, then it is a byte value.  That is the reuse declared under T5
below; it is a legitimate scratch-variable idiom (the value is still live and
read in both roles), not a dead construct, but I am claiming it as a
sanctioned family rather than asserting it is beyond question.

## T3 GCC-internals justification
The body's justification is program logic; it does not need a GCC-internals
story to read correctly, and nothing in it is present only for an internals
reason.  For completeness, the MEASURED explanation for why this spelling
reaches the target where the s1-s11 forms did not (recorded in evidence.md, and
offered as evidence rather than as a justification) is: giving each block its
own byte local stops one pseudo's live range from spanning block 2's base, so
the allocator can reuse block 1's byte register for block 2's base exactly as
the target does.  That is a description of WHY the natural spelling turns out
to be the right one — it is not a construct that exists for the sake of the
allocator.  No line here was added to defeat a pass; the diff is strictly
SMALLER in construct count than the bodies it replaces (three scheduling
barriers deleted, one redundant re-assignment deleted).

## T4 permuter/search provenance
No permuter.  The form came from reading the side-by-side instruction stream
(tmp/grind/func_80034F88/s12/sbs_d3.txt) against the target, identifying that
every INSTRUCTION was already in the target's position and all residual points
were register naming in blocks 2/3, and deriving the live-range explanation
from that.  The structural sweeps that followed (waves F/G/H/I/J, 57 forms)
were CONFIRMATIONS of that derivation and then a search for the most natural
spelling that still scores 0 — i.e. search was used to make the body simpler,
not to make it match.  The submitted form is the simplest of the seven
zero-scoring spellings found, not one that survives only because a detector
misses it.

## T5 family check
Two constructs touch the frozen SOTN-accepted list; both are claimed
explicitly below rather than argued around.  Nothing in the body matches any
entry of the forbidden-family catalog: no register-asm pin, no hardcoded-`$N`
asm, no scheduling barrier (three were REMOVED), no volatile of any spelling,
no alias rename, no unused local or array, no dead store or self-assign, no
dead conditional store, no empty-body `if`, no `if (1)`, no dead goto or label
pad, no DImode chain, no goto-end accumulator, no opaque `one` variable, no
`do { } while (0)`, no redundant width cast, no `.ld` change.

## T6 naming-announces-intent
No name in the diff announces coercion intent.  The identifiers are `p`, `i`,
`q`, `v`, `c` — a pointer, a loop index, an address handle, a value and a
condition/result scratch.  There is no `pad`, `dummy`, `unused`, `spill`,
`fake`, `tmp`, `slack` or `_buf`.  Every one of them is read after it is
written.

SANCTIONED-FAMILY-CLAIMS:
  FAMILY: Variable reuse for codegen control
  SCOPE: "reusing one C variable for two unrelated values to influence loop-invariant detection or RA"
  PRECEDENT: `.claude/rules/no-new-park-categories.md:170`
  (This covers `c` carrying the tested flag bit and then the byte value.  Both
  values are live and consumed; this is the "reuse", not a dead store.
  Measured load-bearing: the same body with a separate result local scores 30
  at 44 insns — tmp/grind/func_80034F88/s12/variants/j1.c.)

  FAMILY: C-level pointer alias to a global
  SCOPE: "C-level pointer aliases to globals"
  PRECEDENT: `.claude/rules/no-new-park-categories.md:233`
  (This covers the four `u8 *q = &D_80106A73;` handles.  Note the carve-out as
  written is for UNUSED aliases and requires a FAKE annotation for those; the
  handles here are each dereferenced for a real load and a real store, so they
  are ordinary live C pointers and are a fortiori inside the family's scope.
  I claim it anyway because four handles to one byte is the construct s10 and
  s11 flagged as needing classification, and I would rather declare it than
  rely on it reading as unremarkable.)

ANNOTATION-CONFORMANCE: n/a — no FAKE construct.  Neither claimed family
requires a `/* FAKE */` annotation under its rule (the 2026-06-02 resolutions
attach the annotation requirement to the `do { } while (0)` carve-out and the
2026-07-01 last-resort additions; the variable-reuse and live-pointer-alias
spellings used here are not last-resort dead constructs and carry no
annotation requirement).  No `/* FAKE */` line is emitted.
