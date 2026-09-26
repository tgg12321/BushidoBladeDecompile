# func_800620B8 — hypotheses (manual lane, slotC, 2026-09-26)

Ruled out (measured, see evidence.md table for the final-chassis receipts):
- Duplicated switch tails in any case order (3021/0312/0123/3012/0321): 52-117 on the e5 chassis.
- Split splat symbols for the sprite table: 47-50 regardless of switch form.
- `SetTransMatrix(base)` / `((MATRIX *)base)->t` inline in the loop: 65-137.
- Struct-member tag link (`prim->tag`): 22.
- One product local reused for width and height: not pursued (multi-write; Ruling 5 1(a) fails).
- Preamble orders: prim first 3; D_800A3474 read at the call 3; all three scratch pointers read up
  front 6; `SetRotMatrix(rot)` 71.

Open: none for bytes (0/501). Review risks listed in evidence.md "Open".
