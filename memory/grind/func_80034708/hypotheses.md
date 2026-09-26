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
- KILLED: aggregate based at 0x8010277C (F7) — rows 2/3 stay base-relative.
- KILLED: 4-/8-byte struct for the flags byte under -G8 (small data; 46).
- DECISION POINT for review: the only honest closing form needs the TU compiled -G8
  (compiler-flags-canonical screening rule; text1a precedent 6e11d4da7). If layer-2 rules a
  new -G8 unit needs an owner ruling, file the policy-question in docs/grind/borderline.md.

## [s3] slotB4
- KILLED (both compilers): aggregate based at 0x7C with 0x78 separate — rows 2/3 stay base-relative
  (12). s8 fields, 2-D pairs, block pointer: 12/12/14.
- OPEN (policy): PracticeParams 0x78..0x87 meets (a1)-(a3) of the 2026-09-26 compiler-necessity
  amendment with cc1psx confirmation, fails only the drafting agent's (a4) one-record-shape
  narrowing (the owner's words: "merged object limited to the labels actually used").
- OPEN (search): independent evidence tying 0x78/0x7A to 0x7C.. — none in any linked asm
  (only func_8001C444 direct stores, func_80022F34's u16 walker + separate 0x82[i] index).
