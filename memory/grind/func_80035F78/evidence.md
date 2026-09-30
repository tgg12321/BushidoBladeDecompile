# Evidence bank — func_80035F78

## 2026-09-30 — REVERTED: the match depended on the per-function maspsx COMMON gate (a cheat)
Owner ruling 2026-09-30 (docs/grind/decisions.md): `maspsx_comm_syms.txt` is a cheat; this function's
row (`func_80035F78: D_800A36B8`) was its dependency. The function is INCLUDE_ASM again and back in the queue. The landed
body is banked as `cheated-comm-gate-body.c` (a lead, not landable). Its honest floor with the list
emptied: 6/12 (`sandbox --disable all`, 2026-09-30; `migration_pin.json`). Frontier: reach the target's
`sym+N` addressing without any assembler gate — the declaration and access spelling are the only lever.

## 2026-09-30 — laneB: same closing form as cdrom_SetMix (substrate ruling needed)
Same residual (D_800A36B8+1..+3 gp vs target lui/%lo). With `CdlATV D_800A36B8;` as a tentative
definition, the 1-line maspsx `.comm x,size,align` parse fix and upstream `--use-comm-section`,
the banked body scores 0/12 in scratch. Full evidence: memory/grind/cdrom_SetMix/evidence.md
2026-09-30 and memory/grind/cdrom_SetMix/probes-0930/.

## 2026-09-30 -- landing with cdrom_SetMix under owner Q62: see memory/grind/cdrom_SetMix/evidence.md, the 2026-09-30 landing section (D_800A36B8 bytes 00 00 00 00 at file offset 0x93EB8; sandbox 0/12).
