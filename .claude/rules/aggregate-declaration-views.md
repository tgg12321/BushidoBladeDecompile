---
name: aggregate-declaration-views
paths: [".claude/rules/aggregate-declaration-views.md"]
description: "Frozen-list exceptions to aggregate-merge prongs (c)/(d): per-file declarations of the same bytes (Q21/Q23-Q25), union word views (Q33/Q46/Q57), one cast store on a local array (Q36). Manual-path layer-2 only."
metadata:
  type: rules
---

# Second views of the same bytes

Exceptions to [[aggregate-merge-family]] prongs (c)/(d). Each lands only through a fresh
layer-2 on the manual path; a Judge PASS is not enough.

## Per-file declarations of the same bytes (Q21)

Two TUs may declare the same bytes with different C types ONLY when ALL hold:
1. **No single declaration compiles both files.** Under Q25 ("mechanism + search"): (a) the
   compiler mechanism by which each file's target depends on its declaration is named from
   banked dumps (pass + location); (b) every single-declaration spelling proposed by anyone
   (at least each file's own form as the shared one, plus every admissible aggregate) is
   banked, with at least one counting spelling per covered file, measured under cc1 and
   cc1psx, and its hit/miss of the file's full target recorded; (c) no declaration is
   DEFEATING (for every covered file, some counting spelling under it hits the full target).
   Spellings relying on refused/banned constructs (pun, F4, F5, alias rename, Q22 dummies) are
   set aside; under **Q23** so are spellings needing any FAKE/!FAKE-annotated construct (the
   ledger names the family and quotes its requirement), and the per-file form itself carries
   no FAKE construct in the covered functions. **Q24 cc1psx agreement:** for every counting
   spelling, cc1psx hits/misses the non-exempt governed instructions exactly as cc1 does. The
   reference form is the exact body/TU that will be on main; take cc1's and cc1psx's `-S` of it
   (before maspsx), strip directives/labels/comments, canonicalise symbol+offset by address,
   placeholder every register (except `$0`) and label, align with
   `difflib.SequenceMatcher(autojunk=False)`; branches compare by aligned destination. A
   governed instruction is EXEMPT only if it lies in a changed run with no equal cc1psx
   counterpart (then cc1's dumps decide). For mechanism parts that do not decide position,
   presence with multiplicity counts. Bank the script, alignment, governed instructions (by
   address and mechanism part) and exemptions; registers chosen outside the mechanism are
   compared under cc1 only.
2. **Same bytes, same accesses**: identical bytes, differing only in grouping (array vs
   scalars, struct vs member scalars); widths equal every access in that file's original
   instructions; no cast/pun/union/alias reach. Exception (Q52): a SOTN-cited local cast
   re-view `(T *)&D_...` marked `/* FAKE */` (with Q53 paperwork) is not refused.
3. **File-local**: each declaration in its own `.c`, none in a shared header; only the proven
   files; one handle per location within each.
4. **Annotated**: each names the other's file:line, states what the banked measurements show,
   cites the ledger and this ruling.
5. **Bytes**: `verify-oracle --rebuild` and sandbox 0 for functions landing with it.
6. **Layer-2** walks (1)-(5).

## Union word view over small fields (Q33; struct members Q46; trailing padding Q57)

An object (global, local, or struct member) may be a union of the real object and one
`s32`/`u32` member ONLY when: (1) at every word-member site the original shows ONE `lw`/`sw`
spanning exactly the word (cited), and the build emits it; (2) exactly two members, the word at
offset 0 (object ≥4 bytes, access at its first byte), no filler/third member/nesting; word
signedness follows any revealing instruction; the name claims only "word"; (3) the word member
is named only at those sites, all other accesses through the real members; (4) no cast
to/from/through the union, no cast-to-union constructor, no union-typed param/return; (5) a
global's union is its canonical header declaration under the aggregate prongs (the word member
is not object-model evidence); a struct member's union replaces the members it spans at the
same offsets, every other offset and the size unchanged, except the compiler's own trailing
round-up when the word raises alignment (Q57: no offset moves, no filler, SHA1 byte-identical,
and a whole-program search of C and target bytes for containing layouts, size/stride uses and
anything inside the added bytes; added-bytes / containing-layout / old-size hits FAIL,
unrelated hits are ignored with the reason, new-size uses pass); (6) layer-2 on every commit
adding a union or a word site; (7) the F5 CLOBBER union, single-member unions and two-view
(Silent Hill) unions stay refused.

## One cast store on a local array (Q36, locals only)

Exactly one `*(s32 *)arr = v;` (or `u32`) when: `arr` is a local array of a sub-word element
type, total 4 bytes, with consumed element reads elsewhere; the target has ONE `sw` at exactly
its frame offset there (cited) and the build emits it; no other cast of the array's address;
the Q33 union spelling was banked and missed first; inline annotation (address, ledger,
ruling). Globals, statics, members, merged aggregates and pointed-to objects never qualify.

Records: docs/grind/decisions.md OWNER RULING entries 2026-09-27 (Q21, Q23-Q25), 2026-09-29
(Q33, Q36), 2026-09-30 (Q46, Q52, Q57).
