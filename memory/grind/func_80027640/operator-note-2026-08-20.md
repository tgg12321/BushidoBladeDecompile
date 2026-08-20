# Operator research note — 2026-08-20 (precedent pass)

- The floor 35 in the migration pin is NOT HONEST: it was measured with 8
  `volatile s32` locals in place, 4 of them never referenced — frame-gap
  fillers that `find_volatile_unused_locals` will refuse at `queue done`
  regardless of SHA1. Re-measure with all eight removed FIRST; expect the
  floor to rise before it falls.
- Target round-trips four stack slots at an 8-byte stride with no address
  taken; the honest producer hypothesis is a memory-resident AGGREGATE
  local (GCC 2.7.2 keeps struct/array locals in memory unconditionally) —
  see split-scalars-hide-aggregate. The entity's own 8-apart field pairs
  supply the object-model evidence the aggregate route requires.
- 2 of its 19 rules are forbidden-family injections
  (lost-codegen-insert-cheat).
- The four unused volatiles map to the phantom-frame-slot volatile pad
  family (owner rulings 2026-08-18/20) IF frame forensics confirm the
  slots untouched — sibling integration precedent: func_80047EE8 /
  func_80047FBC (decisions.md OWNER RULING 2026-08-20).
