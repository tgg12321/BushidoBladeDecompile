# Hypothesis ledger — func_80020E74

## CURRENT (oct2-a3, 2026-10-02)
- candidate.c is byte-exact with the data model in evidence.md (private full link == oracle).
- FAKEs in the body: `loads[130]` (frame layout), shared `j` (loop-1 character + loop-2 menu index),
  `(&D_800A38C4)[i]` (index past D_800A38C4 into D_800A38C6). The last needs a Q63-style per-function
  admission (item 3 refuses cross-symbol derivation); orchestrator declined 2026-10-02 -> owner
  policy-question (docs/grind/borderline.md).
- If refused: the array model makes this body exact but costs func_80020CDC / func_80020D38 3 insns each;
  the frontier is then a byte-exact spelling of those two under `u16 D_800A38C4[2]` (CSE of the
  forced constant address across seq_Reset; d38/ holds six failing spellings).
- Landing shape (once granted): one landing — header (code6cac.h), code6cac_tu2.c (body + func_80020CDC,
  func_80021210, func_800224E0 respellings); `dm/apply_dm.py --inplace` makes the header/consumer edits.
  undefined_syms rows stay (asm referrers remain).

## Superseded
- 2026-10-01 warm-start plan (u16 loads[130], do-while(0), aggregate handles): frame and handles confirmed;
  the do-while(0) is unnecessary (shared `j`).
