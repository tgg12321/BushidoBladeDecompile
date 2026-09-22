# Hypotheses — func_8006F100

- KILLED: literal-offset spellings x1..x5 (27-44), temp forms z1..z4 / w1..w3 (37-46): fold or scheduling
  always lost the (t1 + 0x140)-first grouping or the placement of the add.
- KILLED: `const s32 cx` (44) — C front end substitutes the initializer.
- KILLED: static inline helper (43).
- CONFIRMED: block-scope `s32 cx = 0x140; s32 cy = 0x9D;` -> 0 (full build == oracle).
- NOTE: an assignment-expression probe `tbl.x + (xo = t1 + 0x140)` also hits 0 but its xo/yo are write-only
  (dead-store shape); not proposed.
