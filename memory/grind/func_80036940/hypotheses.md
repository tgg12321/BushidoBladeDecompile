# func_80036940 — hypotheses (ruled out / open)

## Ruled out (with how)
- **Current split headers** (E78..E98 as scalars) — 41: the `++E98` / `++E8C` member forms and the
  E8C-0x20 / E98-0x2C cse of &pair cannot form across separate symbols.
- **Record extension without the E58 outer record** — 12: the two `++E5C` sites (cases 0xB/0xD)
  stay direct `lw/sw %lo(E5C)` while the target uses `la; lw 0(v1); sw 0(v1)`.
- **No case at 0** — 16 (GCC biases the table by the minimum case, `addiu -2`).
- **Function-scope `s32 ret`** — same bytes as block-scoped, but it is a multi-write local
  (Ruling 5 1(c) fails: each write read twice). Block-scoped per-case `ret` used instead.

## Open
- The joint -G8 landing with func_80036140 (owner Q10, 2026-09-26): func_80036940 is score 2
  (jtbl operand only) in the -G8 model too; for ruling (ii) cc1psx -G8/-G0 calibration of
  func_80036940's `lbu %gp_rel(g_cd_result)` (80036C24, 80036C74) is not banked yet.
- The zero word at 0x80010974 (element [15] of the hand-transcribed jtbl_80010938): origin unknown
  (ASPSX did not 8-align tables; 0x978 is 8-aligned). Carried verbatim; retires/relocates when
  func_80036140 lands.
