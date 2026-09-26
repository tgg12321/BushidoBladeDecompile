# Hypothesis ledger — func_80034708

## [s2] slotB4 2026-09-26
- KILLED: separate u8 scalars for 0x84..0x87 (v1, 268) — cannot produce the s7-relative
  walker inits or the s5+10/11 phase-A reads (need one symbol base).
- KILLED at -G0: every cursor spelling that is an honest array (cursor[0] is a bare-reg address,
  never folded). Scalars + `(&D_800A3174)[i]` reach 75 but are a cross-object pun.
- KILLED: nested if/else for the up/down cursor wrap (3 phantom frame slots, extra loads).
- OPEN (landing plan): new -G8 TU for func_80034708 (+ -G0 TU for the 4 tail functions moved
  verbatim), header aggregate for 0x7C..0x87 (extend PlayerBytePairs), aggregate at 0x80106A70
  covering the flags byte, jtbls emitted by the new TU after code6cac_b_rodata_pre's lead word.
