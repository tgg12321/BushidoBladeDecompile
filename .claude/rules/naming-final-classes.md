---
name: naming-final-classes
paths: [".claude/rules/naming-final-classes.md", "docs/naming/phase3/no_name.tsv", "tools/naming_noname.py", "tools/test_naming_noname.py"]
description: "Owner rulings Phase 5 and Q136 (2026-11-06): the FINAL classes of naming-bar N7. A placeholder is marked permanently neutral in docs/naming/phase3/no_name.tsv only when a fresh naming-reviewer re-derives one structural class test from the code; a name that meets N1-N4 always wins."
metadata:
  type: rule
  tier: hygiene
---

# FINAL classes (naming-bar N7)

A class is a test on the code that leaves N1-N4 nothing to bear a name out, whatever evidence
comes. A placeholder meeting the letter of a test but holding a name that meets N1-N4 is not
final: the name wins, and the reviewer FAILs the row. Tokens per kind are `FINAL_CLASSES` in
`tools/naming_noname.py`, which refuses any other. Rows marked final before Q136 stand. Classes
apply to game code and game types only: Sony library code (`src/main/psxsdk`, `include/psxsdk`)
keeps its placeholders open for Sony's names (N3.1).

## func

- `empty`: the body is `jr ra` (and its delay slot) only.
- `dead`: no caller, no address use (C, asm, data), and the body returns nothing or one literal
  constant, and writes and calls only final placeholders: an uncalled body that computes,
  writes or calls anything else is nameable from that effect.
- `forwarder`: one call of a named callee, arguments passed through unchanged, its result
  returned or dropped; a name would restate the callee.

## data and member

- `gap-filler` (data): no reference anywhere beyond its definition, and its contents are none,
  zero fill or padding (describable contents name it).
- `never-accessed` (member): no access in C or in any INCLUDE_ASM body, through any alias,
  and no disc-asset layout or initialised instance gives it describable bytes.
- `write-only` (data, member): no read anywhere (C, asm, asm/data, uncalled code included; a
  copy or I/O of the enclosing block, or a disc-asset layout, counts as a read),
  and every store writes the object's initial value, so it never changes. A store that changes
  it (a named value, a counter, a set flag, a span or other describable literal) records
  something and is nameable by it: not final.

## local

- `const-holder`, `fake-copy`: FAKE-labelled non-parameter locals holding only a literal, or
  only a copy. A parameter is never `write-only`, `const-holder` or `fake-copy`.
- `unused-param`: a parameter never read, or read only to pass it unchanged to an
  `unused-param`; and, tracing every caller (function-pointer dispatch included) through each
  unchanged pass-through and forwarder up to where the value is made, the callers do not all
  supply one quantity, named or still placeholder (N3.3 names it, now or once named).
- `write-only` (local): a local, not a parameter, never read and its address never taken.

## two-role (data, member, local)

A FAKE-labelled local whose label says it holds two values (codegen register reuse) is final on
the label: N1 keeps a FAKE local neutral. Otherwise the slot or binding holds values of
different kinds, differing in type, unit or kind of object (not another instance of one kind:
`&fighters[0]`, then `&fighters[1]` is one role), with one read cited per role. A refinement of
one quantity (x squared, then x; a clamped or rounded value; an index, then its element's
address; a cast of one value), a cursor that advances, a counter, or values read by one use
alike are one role: not two-role. (Splitting a two-role slot into a union is a type change,
outside identifier-only waves.)
