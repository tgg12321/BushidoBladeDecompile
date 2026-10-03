---
name: rodata-object-alignment
paths: ["Makefile", "bb2.ld", "engine/pipeline.py", "engine/buildconfig.py", "src/**/*rodata*.c", "src/display.c", "src/main/64FD8.c", "src/main/3AB48.c"]
description: "Every C object's .rodata links 4-aligned with jump-table .align 3 relative to the object's start (as ASPSX+PSYLINK do). Per-file align sed retired. New TU boundaries only from evidence."
metadata:
  type: rule
  tier: hygiene
---

# Object-relative rodata alignment (owner ruling 2026-09-30)

Record: docs/grind/decisions.md 2026-09-30 OWNER RULING — object-relative rodata alignment; evidence
`docs/grind/rodata-align-2026-09-30.md`.

## The rule (substrate)

- **One rule for every C object.** After `as`, the object's `.rodata` alignment is set to 4
  (`objcopy --set-section-alignment .rodata=4`); GNU as pads `.align 3` relative to the section start, so each
  jump table is 8-aligned relative to its OWN object's start. Never per-file or per-function.
- **The per-file sed (`RODATA_ALIGN2_FILES`) is retired** and may not come back in any spelling. Makefile and
  `engine/pipeline.py` apply the same rule (pinned by `engine test`).

## New TU boundaries: evidence only

A table whose phase (address mod 8) differs from its object's start phase needs a TU boundary. A boundary may
be added only when ALL hold, recorded in the evidence doc per site:

1. **Existence from the original bytes**: two jump tables at different phases cannot share one original TU.
2. **Rodata position from evidence**: the new object starts at an item start whose phase equals its first
   table's phase, consistent with item ownership (a TU's rodata is contiguous; a string literal is TU-local;
   rodata follows function order). If several positions survive, EVERY survivor must give identical bytes;
   the one used is stated with the others. Surviving positions with different bytes → owner policy question,
   never pick the one that matches.
3. **Text cut**: the window (first/last possible function) is recorded; the cut sits at the item that starts
   the new rodata (its function, INCLUDE_ASM or INCLUDE_RODATA line), or for library code at the PsyQ module
   start.
4. **Moves only**: source text moves verbatim; only the externs/includes a part needs to compile change.

Library code (owner ruling Q106 D3): a LIBSCAN verbatim module span (docs/naming/libscan/) is existence and
cut evidence for a boundary at that module's start, including re-cuts of a current mid-module cut; an
unidentified gap between placed modules stays one file; conditions 2 and 4 above still apply. A data-only rodata file folds into its proven owning
TU (Q106 D4: sole referrer, contiguous, in link order).

Boundaries proven by the per-file gp model ([[per-file-gp-model]]) take existence from its split test instead
of condition 1 and its cut convention instead of condition 3's placement; a boundary set here may move within
its recorded window under that rule, updating its record in docs/grind/rodata-align-2026-09-30.md in the same
commit.

Choosing a boundary, reorder or item grouping BECAUSE it makes bytes match, when evidence does not force it,
is the banned speculative rodata reorder ([[no-new-park-categories]]).

## Inter-object padding (owner rulings Q101, Q102, Q104, 2026-10-03)

The `PAD_NOPS_n` macros (`__asm__` nops between functions) and file-scope `.section .rodata`
`.word 0` blocks are not admitted constructs: they are layout debt to retire, each change
byte-neutral and oracle-green. Representation, first that the evidence supports:
1. **Module end**: where the gap ends a Sony module (LIBSCAN span; Sony's assembler padded each
   hand-written asm module's `.text` to 16, the link added nothing), the nops go into that
   module's own asm (its last `asm/funcs/*.s`), recorded per site in
   docs/grind/rodata-align-2026-09-30.md. ReadGeomScreen, SetGeomOffset and SetGeomScreen (LIBGTE
   REG09/12/13) become whole-body canonical asm, pad included (owner ruling Q104; these three only).
2. **Inside hand-written asm**: nops trailing a canonical asm function with no module evidence go
   into that function's `asm/funcs/*.s` (splat's convention).
3. **Rodata words**: a TU boundary under "New TU boundaries" above.
Do not add new uses.

## Related

[[per-file-gp-model]] · [[no-compiler-divergence]] · [[no-new-park-categories]]
