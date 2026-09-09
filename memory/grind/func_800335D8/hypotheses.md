# Hypothesis ledger -- func_800335D8

## s1 (2026-09-08, recon) -- floor 47 (body A, retired form on new chassis) -> 6 (scorer floor; true 0 by oracle SHA1)

CONFIRMED
1. Declaration fix (pair structs for EBFC/EBFD and EA44/EA45, u8[6] for EBF4, TU arrays for E914/EA00/A3750):
   47 -> 27 (bodies A -> C). Mechanism: no `idx = cat*2` pseudo across the call; cat stays in $s1.
2. `buf[i] = 1` (store the constant, not `type`) in the type-1 arm: the constant `1` becomes a 2-use movable and
   loop.c hoists it into $s5 (combine_movables match), 173 -> 176 insns. A -> B1: 47 -> 42.
3. Loop 1 index form: signed `slt` and constant bound D_800A3918+6. 47 -> 46 (B3); carried into D.
4. Loop 2 index form over pointer local `buf` + pointer biv `tbl` + per-iteration `data = tbl + 1` + `0x7B + i`:
   20 (D) -> 16 (F2) -> 14 (F3, data/tbl increment order) -> 14 (G1, counter giv) -> 12 (H1, data giv).
5. Inline ratio expression `((scount - limit) << 10) / (D_8008EA44[adj].b * 30)`: 12 -> 6 (I2/I3); the remaining 6
   are the unresolved `+1` addend pairs (scorer gap), oracle SHA1 matches with the merge applied.

KILLED (all kill_scope=instance; chassis -mel -msoft-float, pair-struct decls applied, no FAKE constructs)
- B4 loop 2 direct `D_800A3750[i]` + `tbl[i*2]`/`tbl[i*2+1]`: 62 (param giv folded to `lbu 1($tbl)`, no $18).
- E `for (i...; i++, p++, data += 2, tbl += 2)` with `*p`: 33 (i not eliminable, stays a counter).
- F1 three index givs (buf[i], tbl[i*2], data[i*2]): 41, 179 insns.
- G3 `tbl[i*2]` + `(tbl+1)[i*2]`: 38 (givs combined, loop dump `giv at 421 combined with giv at 463`).
- H2 `limit` computed before `scount`: 21 (block-wide seat shuffle).
- I1 `diff` computed before the branch: 40, 178 insns.
- A2 parallel u8 arrays `D_8008EBFC[cat*2]` / `D_8008EBFD[cat*2]` (the split-scalar spelling): 22 (`la D_8008EBFD`
  hoisted into $s4); also NOT the object model.
- K1 fully structured if/else dispatch with the default call duplicated into both arms: 19, 173 insns.
- D2 `(s32)` casts on the pointer compares: -1 each but cosmetic; superseded by the index forms.

FRONTIER
1. Land the banked form via the aggregate-merge integration surface (include/code6cac.h, undefined_syms_auto.txt,
   named_syms.txt, TU externs) -- bytes already proven by full-build SHA1; needs the Judge/driver to accept
   verify-oracle as the byte proof because the sandbox cannot reach 0 on this reference (scorer gap).
2. If a scorer-visible 0 is required first: engine/score.py `_symtab()` must also learn dlabel addresses from
   asm/data/7D920.data.s (or build/bb2.map). Not a session surface.
3. Nothing else is open: 176/176 insns, every register and emission order matches target.
