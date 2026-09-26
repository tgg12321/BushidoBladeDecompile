# Hypothesis ledger — func_8006F97C

## s2 (manual lane slotC3, 2026-09-26) — measured on the v10/v11 chassis (sandbox 0/515)

Every number is `sandbox --disable all` against the named variant in tmp/func_8006F97C/.

### `cells` (Ruling 9 receipts, prong (i))
- KILLED per-site locals `cells0..cells3` (function scope): 98/510 (ps1.c). Frame 0x80, the
  row counter gets $s8 instead of the target's sp+0x58 spill.
- KILLED no local at the grid/tail sites (`s.table = s.header + 0xC;`), block-1 local kept:
  98/510 (ps2.c).
- KILLED block-1 local separate, grid+tail sharing one `cells`: 96/510 (ps3.c; the v8 byte-
  pointer twin 96/510).
- Allocation dump (instrumented cc1 -da, tmp/func_8006F97C/dump, v10): `cells` is ONE pseudo
  (74), global.c seats it in $s1 at all four sites (insns 108/1028/1101/1174), exactly the
  target's `addiu s1,v0,36` / `addiu s1,v0,12` x2 / `addiu s1,v0,12`. It is live across loop 1's
  rsin/rcos calls, which is what makes the grid/tail +0xC sums callee-saved; per-site, the
  +0xC pseudos are single-block and local-alloc gives them $v1 (ps1/ps3).
- Permuter from the carrier-free chassis (ps1.c, tmp/perm_f97c, 2 workers, --stack-diffs): see
  evidence.md "permuter" for the harvest; every find below base reuses cells0/cells2/a
  new_var as a carrier of an unrelated value (`col`, `row`, `arg0`, the constant 1), i.e. the
  banned multi-role form; none is a legal per-site spelling.

### `rec` (named intermediate) exhaustion
- KILLED inline `D_800A3560[i * 3]`: 31/515 (x1.c). KILLED `*(D_800A3560 + i * 3)`: 31 (x5.c).
  KILLED `D_800A3560[i + i * 2]`: 31 (x6.c). KILLED ternary `shift[i] = (...) ? 7 : 9`: 97/506 (v4 chassis, r2.c).
  Mechanism: loop.c movable threshold on the symbol pseudo's lifetime (evidence.md s2).

### Grid-arm draw tail duplication
- KILLED one shared tail after the if/else (with or without a `geom` local): 7/515 (d-series
  before d4, v6 chassis). Target shape needs cross-jumped per-arm tails (evidence.md s2).

### Other spellings measured
- Bound: `1 + D_800A35B0 + D_800A3554` 95 vs 105/129/134 for other orders (v1 chassis).
- Mode test: `(u32)x < 2 || x == 2 || x == 3` and `x == 0 || x == 1 || x == 2 || x == 3` both 0;
  `(u32)x < 4` 6/511 (folds to one sltiu). v11 uses the enumeration (no cast).
- `shift[2]` and `shift[4]` both 0 (8-aligned BLKmode slot at sp+0x48 either way); v11 uses [2]
  (D_800A3588/D_800A358C hold two players).
- x formula: `row * 116 + (row >> 1) * 20` 0; `((row>>1)*5 + row*29)*4` 7-11.
