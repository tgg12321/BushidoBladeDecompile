# g_sqrt_table_u8 declaration cleanup (retro-audit follow-up, laneH 2026-09-30)

`extern u8 g_sqrt_table_u8;` (include/code6cac.h:11, plus file-scope copies in code6cac.c,
code6cac_b_tu2.c (x2) and code6cac_tu2.c) declared a scalar, but every consumer indexed it.
The forms were `(&g_sqrt_table_u8)[i]`, `*(&g_sqrt_table_u8 + i)`, `*((&g_sqrt_table_u8) + i)`
and `*(((u8 *)&g_sqrt_table_u8) + i)`.

## The object (target evidence)
- 0x8008D118..0x8008D517 = 0x400 bytes, stored in two pieces:
  - The first 8 bytes are the two words after DelDrv in src/main_post.c (`g_sqrt_table_u8:`
    label, `.word 0x0D0B0800`, `.word 0x15131110`).
  - The other 0x3F8 bytes are asm/data/7D920.data.s dlabel D_8008D120 (ends at enddlabel before
    D_8008D518).
- Contents (tmp/laneH/verify_tables.py): byte i == floor(8 * sqrt(i)) for all i in 0..0x3FF, with
  no mismatches. The formula breaks exactly at the D_8008D518 boundary, which is a different
  object.
- Readers use it as an integer square root: `table[x] >> 3` on the `x < 0x400` path, and a
  normalized `table[x >> shift]` on the long path.
- No C consumer of D_8008D120 exists, and no alias rows lie inside the range in
  undefined_syms_auto.txt, named_syms.txt or symbol_addrs.txt. (D_8008D118 / g_isqrt_lut /
  g_module_type_tbl were already retired 2026-09-29.)
- The data-asm dlabel D_8008D120 is the splat label of the .data piece. It is not a C
  declaration and not an alias row, so it is left as it is.

## Change
- include/code6cac.h: `extern u8 g_sqrt_table_u8[0x400];`, with an evidence comment.
- The four file-scope scalar re-declarations are removed.
- Element access `g_sqrt_table_u8[i]` in 21 functions:
  - code6cac.c: func_80018094, func_80018300, func_800187F4.
  - code6cac_b_tu2.c: func_800274BC, func_8002A458, func_8002BC68, func_8002BEA0, func_8002D320,
    func_8002D518, func_8002D780, func_8002DAD0, func_8002E838, func_8002EA24, func_8002EBDC,
    func_8002F2D0, func_8002F770, func_80032314, func_800325E0.
  - code6cac_tu2.c: func_8001A67C, func_8001A820, func_8001F2E4.
- func_800274BC: the PRE-EXISTING, unannotated second handle `u8 *new_var` is REMOVED
  (orchestrator's decision 2026-09-30). `*((new_var = &g_sqrt_table_u8) + idx)` becomes
  `g_sqrt_table_u8[idx]`. Object-identical: probe_sqrt_noalias, then the final `sqrt` stage that
  includes the removal.

## Measurement
tmp/laneH/harness.py (see memory/grind/judge-decl-cleanup/evidence.md), measured on top of the
Judge stage:
- `@judge sqrt`: 33 TUs, 0 differing.
- `@judge @sqrt probe_sqrt_noalias`: 0 differing.
