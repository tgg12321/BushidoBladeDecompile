# Hypothesis ledger — single_game_setModeRequest

## [s2] slotQ 2026-09-26/27
- OPEN (the last 2 insns): LZC-1 input copy. Target keeps a copy of the squared
  length in $a0 for the asm input only (compare/small arm/srav use $a1); LZC-2
  has none. Same idiom in func_800288C8 / func_8002A458 (their `tbl`/`lzc_in`
  bare-copy staging FAILED layer-2; Ruling 11 (C)(3) names them). Need an
  honest source for a pre-branch copy that cse keeps non-canonical.
- KILLED: copy via `dist1 = sum` (coalesces), `lut = sum` staged (cse makes lut
  canonical, asm operand canonicalized — cse DOES rewrite output-less asm
  operands), repeated sum expression as the asm arg (combine folds), static
  inline helper for the sqrt (no copy; hoists &lz).
- POLICY items to settle before landing: shared counter j (3 loops), in-place
  dist, shift split, lz[6] oversize, n/bits rewrites; GTE islands need PINNED
  entries for ldlvl/stlvl/lddp/sqr0/gpf0/gpl12/rtv0tr + DMPSX words for
  rtv0tr/sqr0/gpf0/gpl12 (not in DMPSX_WORDS) — the class route admits no
  placeholder substitution, so these four command islands need a per-function
  owner row (func_8002DE20 Q11 precedent) or the inline_c.h route.
