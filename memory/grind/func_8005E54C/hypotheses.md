# Hypothesis ledger — func_8005E54C

- [s2 CONFIRMED 2026-09-29] s1 #3 (explicit `if (rounds)` guard) REFUTED: plain `for` + signed bound gives the
  `beqz` via combine simplify_comparison (GT 0 -> NE 0 on a nonnegative value).
- [s2 CONFIRMED] s1 #2: `s16 vals[2]; s16 wins[2];` declared in that order land at 0x18/0x20 (BLKmode, 8-aligned);
  the `lh 8(a1)` is CSE of &wins[j] off &vals[j]. No spacer needed.
- [s2 CONFIRMED] 0x8009BD24 is one struct (2-byte chr records + flags word at +0x14): with it, evidence [s2]
  items 2+3 close exactly (sbxp.py 44 -> 20 for the chr part; match0 0/799).
- [s2 OPEN] frame +8 at s+0x30: needs an untouched 8-aligned object; only non-honest spellings found
  (trailing descriptor members, or a dead local). Owner question candidate.
- [s2 OPEN] R3 word zero `sw zero,0x18(sp)`: only a 32-bit view reproduces it (pun or union). Owner question
  candidate (same class as borderline.md 2026-09-29 func_80070188 union question).
- [s2 OPEN] R4 `move s4,s0`: only `i = y;` (copy into the reused counter) reproduces it; Ruling 11 (C)(3) refuses
  a bare-copy value. Owner question candidate. Probes without a copy statement: p3a (y held in i, sums counted
  with k) 14, p3b (y used directly) 6, p3c (y as s16) 21.
