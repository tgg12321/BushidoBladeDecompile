# func_8002F2D0 — evidence (manual session 2026-09-22)

## What the function is
Matrix -> Euler-angle decomposition. Copies the caller's MATRIX (32-byte struct
copy) to scratch 0x1F800390, computes the first column of its inverse
(cofactors c0..c2 over det) plus a second cofactor triple r0..r2, then:
- ang_z = -ratan2(i1, i0)
- dist  = LUT/GTE-LZC integer sqrt of i0*i0 + i1*i1 (same island as the matched
  func_8002EBDC; template character-identical)
- ang_y = ratan2(i2, dist)
- identity at scr+0xD8 (== 0x1F800390, the SAME scratch address as the input
  copy), RotMatrixZ(ang_z), RotMatrixY(ang_y), gte_SetRotMatrix, rotate
  {r0,r1,r2} (gte_ldlv0 / mvmva / gte_stlvnl at scr+0xA8)
- out[0] = ratan2(v[2], v[1]); out[1] = -ang_y; out[2] = -ang_z  (s16 stores)
Caller: func_800300B4 (`func_8002F2D0(mtx, dir)`, s32 mtx[8], s32 dir[2]).

## Floors (sandbox --disable all, honest)
- first full body: 106
- det accumulated in the det var (`det = m00*(c0>>12); det += ...`): 26
- dist reuses the det variable (target keeps both in t2): required, -34
- c0/c1 reused as i0/i1 (`c0 = c0 / det`): matches target s1/s0 reuse
- F1 (no FAKE): **19**; separate `d0` var placed right after c0,
  `det = (d0 + m10*(c1>>12) + m20*(c2>>12)) >> 12`. First half byte-exact.
- M1 (two sanctioned devices): **6** — only the sum's register differs.
- **FINAL 0/270 (N1d = candidate.c)**: `sum` reused for the LUT byte in the
  else arm (`sum = lut[(u32)sum >> shift]; det = (u32)(sum << 16) >> ...`),
  do-while(0) around `i2 = c2 / det`, do-while(0) around the
  gte_SetRotMatrix island, det reused for the sqrt result. verify-oracle
  --rebuild green with it applied. Counterfactuals: fresh `tbl` 6, no i2
  wrap 5, no island wrap 8, separate dist var 35.
- How the sum lever was found: func_8002F770 inlines this whole body (same
  squares, sum also in $a0), which suggested an inline sqrt helper; the helper
  version (H7, 0) worked because its parameter was reused for the table byte —
  the reuse, not the helper, is the lever (N1 reproduces it without a helper,
  keeping the LZC island inside the function body for the region grant).
  Measured prefs: with reuse sum's find_reg full prefs = {4,16,17} (the `<< 16`
  result is a block-local in $a0 and set_preference records it on the dying
  sum), -> $a0; fresh tbl -> {16,17} -> $s0.
- cc1psx (PsyQ original) gives the same $s0 for the fresh-tbl spelling: the
  residual was a spelling, not a compiler gap.

## Allocation mechanics (instrumented cc1, BB2_FINDREG/QTY/SUGG_DEBUG)
Recipe: tmp/f2d0/dbg.sh <spliced.c> <pseudo> ; tmp/f2d0/cen.sh <cand.c>.
- i2 vs ang_z (s4/s5 swap in F1): global.c priority. a1-param 720, ang_z 517
  (3 refs / 58), i2 465 (2 refs / 43). Target needs i2 in (517, 720) -> one
  more ref. `do { i2 = c2 / det; } while (0);` (loop-depth ref weighting) fixes
  it exactly (L1).
- mat vs ang_y (s0/s1 swap in F1): local-alloc block 5 qty priority. ang_y qty
  refs 5 (ang_y + tied neg result) len 36 = .278 beats mat 4/34 = .235. A named
  intermediate `new_var = scr + 0xD8; mat = new_var;` (permuter find, P55)
  lifts mat's qty to 6 refs -> fixed. A do-while around the mat def ALSO moves
  it but perturbs sched (L2 17).
- sum in s0 instead of a0 (the last 5 insns): the sum pseudo's
  hard_reg_full_preferences = {16,17}, inherited in expand_preferences from the
  two product pseudos (i0*i0, i1*i1), whose own prefs come from set_preference
  on `(mult i0 i0)` with i0/i1 LOCAL-allocated to s1/s0. find_reg pass 0 picks
  a0 as best_reg, then the same-class preference loop switches it to s0.
  Products are never local qtys (LO_REG is likely-spilled), so they are always
  allocnos. For a0 the target needs the sum to carry no usable 16/17 pref:
  either i0/i1 are NOT local (global/unallocated when prefs are recorded), s0/s1
  conflict with the sum, or the sum gets a copy-preference (copy prefs win
  before full prefs). Making i0 global via reuse (c0 reused as ang_y, n4) DOES
  put the sum in a0, but then c0-var (pri 5416) loses s1 to scr (12363).
