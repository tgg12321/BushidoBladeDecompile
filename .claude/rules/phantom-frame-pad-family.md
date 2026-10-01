---
name: phantom-frame-pad-family
paths: [".claude/rules/phantom-frame-pad-family.md"]
description: "Frozen-list family (2026-08-18): an unreferenced `volatile` pad array reserving target-untouched frame bytes — first-local form, plus the Q35 trailing sibling-evidence array. FAKE-annotated, engine allowlist row, layer-2."
metadata:
  type: rules
---

# Phantom-frame-slot volatile pad (owner ruling 2026-08-18)

Frozen-list entry of [[no-new-park-categories]] (SOTN `src/st/sel/2C048.c:564`
`volatile u32 pad; // !FAKE:`). Standard prerequisites (exhaustion, named mechanism,
annotation, layer-1 + layer-2) plus:
- ARRAY form `volatile u32 pad[N];`, the FIRST local, never referenced, no `(void)pad;` shims;
- `// !FAKE` annotation;
- a per-function row in `engine/volatile_cheats.py _SANCTIONED_UNWRITTEN_PADS` (the sandbox
  strips every never-referenced local array without one; the allowlist admits only a
  `volatile`-qualified declaration);
- ledger frame forensics showing the target slot genuinely untouched
  ([[phantom-slot-frame-lever]] procedure), honest producers measured inert first.

Earlier per-function grants stand: func_8001E404, func_8001E6E4, func_8003CF84 (leading pads),
func_8003CF84 `pad2[2]` (trailing, that function only).

## Trailing array with sibling evidence (Q35, 2026-09-29)

Relaxes only position, name and element type. ALL of:
1. Frame forensics list every `$sp` access and show the region untouched, in the locals area
   (not outgoing-args, callee-save or spill).
2. At least two COMPLETED-C functions in the same file declare a real, used array of the same
   type and count at the same offset after an object of the same layout, at least one as a
   separate local right after it (cited by file:line, use cited).
3. The declaration copies that sibling array's type and count as a separate local right after
   the object, and the frame then equals the target's (size, locals region, every `$sp`
   offset).
4. Form: the sibling's exact declaration plus `volatile`, no initializer, never referenced; the
   non-volatile form banked as compiling byte-identically.
5. Name: the sibling's identifier (never `pad`, `dummy`, `spill`, ...).
6. A `/* FAKE: ... */` comment citing the siblings, the ledger and this ruling.
7. Honest producers measured inert: the body without the array, the three
   [[phantom-slot-frame-lever]] producers, every real local that could occupy the region.
8. Its own `_SANCTIONED_UNWRITTEN_PADS` row (detector-config only, `engine test` green) and a
   fresh layer-2; a Judge PASS is not enough.

Related: [[dead-vars-local-array]] · [[phantom-slot-frame-lever]]
