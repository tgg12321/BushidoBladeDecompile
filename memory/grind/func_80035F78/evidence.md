# Evidence bank — func_80035F78

## 2026-09-30 — REVERTED: the match depended on the per-function maspsx COMMON gate (a cheat)
Owner ruling 2026-09-30 (docs/grind/decisions.md): `maspsx_comm_syms.txt` is a cheat; this function's
row (`func_80035F78: D_800A36B8`) was its dependency. The function is INCLUDE_ASM again and back in the queue. The landed
body is banked as `cheated-comm-gate-body.c` (a lead, not landable). Its honest floor with the list
emptied: 6/12 (`sandbox --disable all`, 2026-09-30; `migration_pin.json`). Frontier: reach the target's
`sym+N` addressing without any assembler gate — the declaration and access spelling are the only lever.
