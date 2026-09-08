# Hypothesis ledger — SsUtKeyOffV

## [s1] 2026-09-08 — recon

H1 (data model) — `_snd_ev_flag` as s32 extern (main.c:227) and `D_8010280A` as
  `_svm_cur.voice` (include/sound.h struct_svm) are the correct declarations.
  CONFIRMED — both reproduce their target words; score 0 reached with no declaration edits.

H2 — The `$a0`/`$a1` parm split comes from the range check being folded through
  fold_range_test (`voice < 0 || voice >= 24` or `voice >= 0 && voice < 24`) rather than an
  explicit `(u16)voice` cast. CONFIRMED (13 -> 12, disassembly shows move/andi-a0/sh-a1).

H3 — The un-merged `return -1` tails + success-falls-into-epilogue layout follow from the
  SUCCESS arm being written first (`if (voice >= 0 && voice < 24) { ...; return 0; }
  _snd_ev_flag = 0; return -1;`): jump1's range swap (jump.c:1826) then leaves the error
  block inline ending in `jump Lend`, sched1 hoists `v0=-1` above the flag store, jump2's
  cross-jump suffix match falls below min 2. CONFIRMED (12 -> 0).

H4 (KILLED, instance) — an explicit `else` arm around the success path changes the jump1
  layout. Measured 12, byte-identical to the error-first form; chassis = flat body with
  `voice < 0 || voice >= 24`, no FAKE constructs.

H5 (KILLED, instance) — psyz SsUtKeyOff's literal nesting (`if (_snd_ev_flag != 1) {...}`
  with a single trailing `return -1`) is what produces the target layout. Measured 12,
  byte-identical to H4's output; the nesting is not the lever, arm order is.

Frontier: none — candidate at 0, submitted.
