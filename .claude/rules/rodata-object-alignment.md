---
name: rodata-object-alignment
paths: ["Makefile", "bb2.ld", "engine/pipeline.py", "engine/buildconfig.py", "src/*rodata*.c"]
description: "Owner ruling 2026-09-30: every C object's .rodata is linked on a 4-byte boundary and its jump-table `.align 3` pads relative to the object's own start, as Sony ASPSX 2.34 + PSYLINK 2.37 do (measured). One uniform rule replaces the per-file `.align 3 -> .align 2` sed (RODATA_ALIGN2_FILES). New TU boundaries only from evidence: bytes prove existence, ownership fixes position, byte-equivalent alternatives are recorded, never chosen by fit."
metadata:
  type: rule
---

# Object-relative rodata alignment (owner ruling 2026-09-30)

> **Update 2026-09-30 — per-file gp model boundaries (owner ruling Q65).** Boundaries proven by the per-file
> gp model ([[per-file-gp-model]]) satisfy their existence by the gp split test in place of this rule's
> condition 1, and are placed by that rule's cut convention in place of condition 3's placement clause.
> Conditions 2-4 otherwise apply as that rule states. A boundary set under this rule may also be moved within its
> recorded window by per-file-gp-model's cut outcome (i) or by its Merge-bullet boundary move; that move's
> placement replaces condition 3's placement clause for that boundary, and its record in
> docs/grind/rodata-align-2026-09-30.md is updated in the same commit.

Owner (Trenton), verbatim, answering "do you want me to go ahead and adopt it on those terms?"
(the terms: ruling recorded first, adoption lands only after a byte-identical oracle check and a
layer-2 review): **"Yes go ahead"**. Evidence: `docs/grind/rodata-align-2026-09-30.md`
(sections 1-6). Record: docs/grind/decisions.md 2026-09-30 OWNER RULING — object-relative rodata
alignment.

## The rule (substrate)

- **One rule for every C object.** After `as`, the object's `.rodata` section alignment is set to 4
  (`objcopy --set-section-alignment .rodata=4`). GNU as already pads `.align 3` relative to the
  section start, so each jump table is 8-aligned relative to its OWN object's start, and the object
  sits on a 4-byte boundary. This is what Sony's ASPSX + PSYLINK do (t1/t3 tests, section 1 of the
  evidence doc). It applies to all objects alike; it is never per-file or per-function.
- **The per-file sed (`RODATA_ALIGN2_FILES`) is retired** and may not come back in any spelling.
  The Makefile and `engine/pipeline.py` apply the same uniform rule (pinned by `engine test`).

## New TU boundaries: evidence only

A table whose phase (address mod 8) differs from its object's start phase needs a TU boundary.
A boundary may be added only when ALL hold, recorded in the evidence doc per site:

1. **Existence from the original bytes**: two jump tables at different phases cannot share one
   original TU.
2. **Rodata position from evidence**: the new object starts at an item start whose phase equals its
   first table's phase, consistent with item ownership (a TU's rodata is contiguous; a string
   literal is TU-local; rodata follows function order). If more than one position survives, EVERY
   survivor must give identical bytes; the one used is stated with the others. A site where the
   surviving positions give different bytes is not decided by picking the one that matches: it is
   a policy question for the owner.
3. **Text cut**: splitting keeps every section's order, so the cut position inside the window is
   byte-neutral. The window (first/last possible function) is recorded, and the cut is placed by
   the stated convention: at the item that starts the new rodata (its function, INCLUDE_ASM or
   INCLUDE_RODATA line), or, for library code, at the PsyQ module start.
4. **Moves only**: a split moves source text verbatim; no C, data or declaration is changed except
   the declarations a part needs to compile (externs and includes).

Choosing a boundary, a reorder or an item grouping BECAUSE it makes bytes match, when evidence
does not force it and alternatives differ in bytes, stays the banned speculative rodata reorder
([[no-new-park-categories]], [[jtbl-rodata-split-infrastructure]]).

## Related

[[jtbl-rodata-split-infrastructure]] · [[no-compiler-divergence]] · [[no-new-park-categories]]
