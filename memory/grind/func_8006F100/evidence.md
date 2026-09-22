# Evidence — func_8006F100 (manual lane, 2026-09-22)

Fresh function (no prior ledger/C). Floor ladder, sandbox --disable all, 266 target insns:
266 -> 120 (first transcription) -> 103 -> 77/70 -> 64 -> 60 -> 35 -> 27 -> 0.

## Levers that moved the floor (each measured)
- `(dx * sx >> 8) / 2` — target is `sra 8; srl 31; addu; sra 1` (signed /2 of a >>8), NOT `/256 >> 1`.
- Statement order `t0; t1 = t0; <idx/dx/dy>; t2; if (i) t1 = -t0;`.
- `s32 idx = s.hdr->count - 1; s.ent[idx]` keeps `(n-1)<<3 + ent` (a direct `&p[n-1]` is
  distributed by c-typeck pointer_int_sum to `n*8 - 8`).
- Loop bound `1 + D_800A35B0 + D_800A3558` (same as sibling func_8006ECF4) -> lh 3558 before lw 35B0.
- `if (flag & 2) sel = 1; else sel = 0;` -> andi+sltu (the `!= 0` form becomes srl/andi via do_store_flag).
- D_800A3550 must be s16 (target `lh` + signed compare); func_8006F038's v1..v3 then s16 to keep its `lhu`.
- s16 locals t1/t2/dx/dy: the target sign-extends at each use and keeps `move t1,t0` distinct.
- cx/cy constant-holder locals (block scope): fold-const.c associate/split_tree reassociates the literal
  `tbl + (t1 + 0x140)`; target keeps the grouping. const local = 44 (decl_constant_value), inline helper = 43,
  function-scope holder = 6 (loop.c hoists to $s8), dropping cy = 12.
- Struct size of the func_80073C78 param block: no tail padding needed (0x2C bytes) once D_800A3550 is s16.

## Harness
tmp/f100/sbx.py scored candidates against a textually patched copy of src/text1b.c (for the
declaration experiments) and also scored func_8006F038/func_8006ECF4 to catch neighbour regressions.
