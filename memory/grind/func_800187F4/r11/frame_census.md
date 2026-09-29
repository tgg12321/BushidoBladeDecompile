# func_800187F4 — phantom-slot producer census for lz[6] (2026-09-28)

Question (dead-vars-local-array.md OVERSIZED-LOCALS prerequisites 1 and 4; phantom-slot-frame-lever.md):
can an ordinary-C spelling of the fully-written form (lz[2], the two LZC outputs only) reserve the 16
extra frame bytes of the target (0x78) at zero instruction cost? The lz[2] form already carries the
count spill slot and the four 8-byte phantom slots of the combine orphan-USE loop-guard pseudos
(evidence.md [s2] item 7); its frame is 0x68. Bodies: variants_v2/fr_*.c (r11/tools/mkframe.py), each c6
with lz[2] (fr_scalars: two scalars lz0, lz1 instead). "insns" = the engine sandbox count (target 644).

| spelling | frame | lines | sandbox | insns |
|---|---|---|---|---|
| lz[2] (base) | 0x68 | 24 | 24 | 644 |
| `(u8)bits` force indexes | 0x68 | 24 | 24 | 644 |
| `s16 count` | 0x70 | 80 | 32 | 645 |
| `depth >= 0x3201` | 0x68 | 24 | 24 | 644 |
| force loop 1 rotated behind its own guard | 0x68 | 24 | 24 | 644 |
| node loop behind `if (i < count)` | 0x68 | 24 | 24 | 644 |
| ellipsoid loop behind `if (idx < SCR->nsph)` | 0x68 | 24 | 24 | 644 |
| `s16 i` | 0x60 | 156 | 101 | 643 |
| `s16 idx` | 0x50 | 235 | 193 | 680 |
| `s16 nforce` | 0x68 | 88 | 47 | 646 |
| `nsph = SCR->nsph` named local | 0x68 | 130 | 106 | 642 |
| the same, s16 | 0x68 | 141 | 94 | 646 |
| two scalars lz0, lz1 | 0x68 | 24 | 24 | 644 |
| node[6] named `s16 state` | 0x68 | 185 | 126 | 634 |
| node[6] named `s32 state` | 0x68 | 185 | 126 | 634 |

Result: no spelling reaches 0x78. The zero-instruction-cost ones keep 0x68 (their 24 lines are the
frame shift through the save block only); the ones that move the frame cost instructions (`s16 count`:
0x70, 80 lines). lz[5] and lz[6] (17-24 bytes) give 0x78 at 0 lines; lz[7]/lz[8] 0x80 (v1 measurements).
