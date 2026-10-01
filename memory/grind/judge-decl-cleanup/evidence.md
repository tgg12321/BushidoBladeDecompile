# Judge declaration cleanup (retro-audit follow-up, laneH 2026-09-30)

Source: docs/audits/RETRO-AUDIT-2026-09-29.md § Follow-ups — "36 scalar `&Judge` uses in
code6cac_b / code6cac_b_tu2 / code6cac_tu2". These TUs declared `extern s16 Judge;` (a scalar)
and indexed it as `(&Judge)[i]`, `*((&Judge) + i)`, `*(&Judge + i)`, and once as a byte offset,
`*(s16 *)((u8 *)&Judge + ...)`.

## The object (target evidence)
- asm/data/7D920.data.s: `dlabel Judge` .. `enddlabel Judge`, 0x800973FC..0x800993FB, which is
  0x2000 bytes = 0x1000 `.short`. It is ONE splat object; the next dlabel is D_800993FC.
- Contents (tmp/laneH/verify_tables.py): Judge[i] == round(4096 * sin(2*pi*i/4096)) within 1 for
  all 4096 entries; Judge[0x400] = 4096. It is a plain s16 sine table, not a record table.
  Every reader masks the index to 12 bits; cos is read as Judge[(a + 0x400) & 0xFFF].
- Other TUs already declare `extern s16 Judge[];`: ings.c, sound.c, text1a_c.c, and text1b.c
  (17e01239b). No splat alias rows exist inside 0x800973FC..0x800993FB (undefined_syms_auto.txt,
  named_syms.txt and symbol_addrs.txt have no rows there).

## Change
- include/code6cac.h: `extern s16 Judge[0x1000];`, with an evidence comment. Every consumer TU
  includes this header.
- The scalar `extern s16 Judge;` is removed from code6cac.c, code6cac_b.c, code6cac_b_tu2.c and
  code6cac_tu2.c. In code6cac.c and code6cac_b.c it was declared but never used.
- Element access in 10 functions:
  - code6cac_b_tu2.c: func_800283D0, func_8003032C, func_80030580, func_80030D7C, func_80032064,
    func_800325E0.
  - code6cac_tu2.c: func_8001BAE4, func_80021DB0, func_800233AC, func_80023648.
- func_8001BAE4: the byte offset `*(s16 *)((u8 *)&Judge + ((var_v1 >> 1) & 0x1FFE))` becomes
  `Judge[(var_v1 >> 2) & 0xFFF]`, which is the same value. combine folds sra 2 / andi 0xFFF /
  sll 1 into the target's `sra 1; andi 0x1FFE`. Object-identical.
- func_800233AC: the PRE-EXISTING, unannotated second handle `s16 *judge_ptr` is REMOVED
  (orchestrator's decision 2026-09-30: it was a pointer alias with no FAKE paperwork). It was
  `judge_ptr = &Judge;` with three `judge_ptr[angleN]` reads, which are now `Judge[angleN]`.
  Object-identical: probe_judge_noalias, then the final `judge` stage that includes the removal.

## Measurement
tmp/laneH/harness.py builds all 33 TUs that include code6cac.h through the engine's exact C
pipeline, at one scratch path, before and after the change, and byte-compares the .o files.
- `judge`: 33 TUs, 0 differing.
- Negative control (probe_negative, `* 4000` -> `* 4001`): 1 differing. The harness does detect
  real differences.
- probe_judge_unsized (`Judge[]` instead of `[0x1000]`): 0 differing.
