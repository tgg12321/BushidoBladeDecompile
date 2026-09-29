# compute vein (sweep 2026-09-29) - examined and dropped

Scope: the 127 AUTO targets in targets.csv. Triage (triage.py -> triage.txt) decodes the RAW EXE words of each body: gp-relative accesses, lui-rooted absolute addresses (0x8xxx / 0x1F80), cop2, jump tables, and cross-references every docs/naming/sweep-*/rejected|keep|held list. Every body without a game-global/game-table access, and every body not already in a prior rejected list, was hand-read (dis.py). Proposed at HIGH: 0x80042ED8, 0x80052CD4. Kept MEDIUM: 0x8002E6B0 (see candidates.csv).

Note: targets.csv describes every row as callee-free, but 0x8005B9C4, 0x8005B868, 0x8005BA6C, 0x8005BF3C, 0x8005B9FC, 0x80044B30 (jalr), 0x8005B5AC and 0x8005B72C make calls.

| addr | insns | reason |
|---|---|---|
| 0x80047EC8 | 2 | body `return 0xD00` - a constant; no computation to restate (also 2026-09-25 sys_snd_io/snd_followups: coined size name rejected). No new evidence. |
| 0x8007E08C | 2 | rejected 2026-09-24 compute.md (reason there); re-checked from raw words: arg-struct/record layout or data-as-code, as recorded. No new evidence. |
| 0x8003E2A0 | 3 | rejected 2026-09-24 compute.md (reason there); re-checked from raw words: gp_rel x1. No new evidence. |
| 0x8003F1C8 | 3 | accessor: `return D_800A336C` (gp_rel) - game-global read, no computation. Pair-private clause gives nothing: a name could only restate "load word X", which names the global (2026-09-25 misc precedent for get_ptr_D_*). |
| 0x8003F268 | 3 | accessor: `return D_800A322C` (gp_rel) - game-global read, no computation (same as 0x8003F1C8). |
| 0x80046780 | 3 | `return D_800A33B0` (gp_rel) - accessor (2026-09-25 misc keep list). No computation. |
| 0x8004678C | 3 | accessor: `return D_800A33B4` (gp_rel; a relocated record-array pointer per 2026-09-25b) - no computation. |
| 0x800477DC | 3 | setter: `D_800A33D0 = a0` (gp_rel) - game-global store, no computation. |
| 0x80054434 | 3 | accessor: `return (s16)D_800A33F8` (gp_rel) - no computation. |
| 0x800392B8 | 4 | returns &D_800F33D8 - address getter of a game buffer, no computation. |
| 0x80042864 | 4 | rejected 2026-09-24 compute.md (reason there); re-checked from raw words: lui [800F]. No new evidence. |
| 0x80046E44 | 4 | setter: `(s16)D_800F6654 = 0` - game-global store. |
| 0x80046E7C | 4 | accessor: `return (s16)D_800F6654` - game-global read. |
| 0x800545F4 | 4 | rejected 2026-09-24 compute.md (reason there); re-checked from raw words: lui [4880 4C80]. No new evidence. |
| 0x80077B20 | 4 | rejected 2026-09-24 compute.md (reason there); re-checked from raw words: gp_rel x1. No new evidence. |
| 0x80077D00 | 4 | rejected 2026-09-24 compute.md (reason there); re-checked from raw words: lui [800A]. No new evidence. |
| 0x800457E8 | 5 | returns 0x45000 - D_800A33A8 (gp_rel game global) - reads game state; a "remaining bytes" name would be game-semantic. |
| 0x80046E8C | 5 | setter: `D_800A3790 = 0x23` - game-global store of a constant. |
| 0x80052C10 | 5 | rejected 2026-09-24 compute.md (reason there); re-checked from raw words: lui [1F80]. No new evidence. |
| 0x8006D324 | 5 | rejected 2026-09-24 compute.md (reason there); re-checked from raw words: gp_rel x1. No new evidence. |
| 0x8001BE08 | 6 | rejected 2026-09-24 compute.md (reason there); re-checked from raw words: arg-struct/record layout or data-as-code, as recorded. No new evidence. |
| 0x80022568 | 6 | rejected 2026-09-24 compute.md (reason there); re-checked from raw words: arg-struct/record layout or data-as-code, as recorded. No new evidence. |
| 0x80036F28 | 6 | `return g_cd_file_table[a0].+4` - bare table read of a game global (2026-09-25 sys_snd_io rejected cdrom_GetFileSize: no admitted class for table reads). No new evidence. |
| 0x80047ED0 | 6 | `D_800A33D0 += a0` (gp_rel) - game-global accumulator (D_800A33D0 is also written by 0x800477DC and used elsewhere) - not pure, not pair-private. |
| 0x80048B8C | 6 | rejected 2026-09-24 compute.md (reason there); re-checked from raw words: gp_rel x2. No new evidence. |
| 0x8005441C | 6 | `D_800A33F0 += a0` (gp_rel) - game-global accumulator; a pair-private name could only restate "add to global X". |
| 0x8003DDF8 | 7 | rejected 2026-09-24 compute.md (reason there); re-checked from raw words: lui [00FF 800A]. No new evidence. |
| 0x8003E2AC | 7 | `D_800F6656 &= ~2` - game-global bit clear. |
| 0x8003FFC4 | 7 | rejected 2026-09-24 compute.md (reason there); re-checked from raw words: arg-struct/record layout or data-as-code, as recorded. No new evidence. |
| 0x8006E480 | 7 | (p[0] & 0x1F) + p[1]*128 + a1 over a byte pair - rejected 2026-09-24 compute + 2026-09-25 misc (no neutral name; game data packing). No new evidence. |
| 0x8003877C | 8 | rejected 2026-09-24 compute.md (reason there); re-checked from raw words: gp_rel x4. No new evidence. |
| 0x8005BA6C | 8 | NOT a leaf: snd_VabFakeOpen(a0, 9) wrapper - api vein; 2026-09-25 sys_snd_io rejected (hard-coded VAB id). No new evidence. |
| 0x80052C28 | 9 | pure leaf (9 insns, no memory): v=a0>>(3-a1) logical; e=v>>11; m=(v&0x7FF)-0x1000; return (m>>e)&0xFFF. Clear computation, but it can only be described as a formula (no standard operation / neutral noun) - dropped per brief. |
| 0x80038148 | 10 | rejected 2026-09-24 compute.md (reason there); re-checked from raw words: lui [800F]. No new evidence. |
| 0x80046E54 | 10 | `(s16)D_800F6654 = (a0 != 0)` - game-global flag store. |
| 0x8004019C | 12 | if (p->+0x24) { p->+0x24 += a1; p->+0x28 += a1; ((s16*)p->+0x24)[3] = 1 } - arg-struct pointer relocation with a game-struct layout; no neutral name. |
| 0x8003F1E4 | 13 | writes game globals (lui 800F) under an a0 flag - game state. |
| 0x80043244 | 13 | pure leaf: returns 3 if a0>0x16A09, 2 if >0xB500, 1 if >0x5A00, else 0 (0 references in the EXE - dead). A threshold classifier; the thresholds are not a clean series (0x16A09=floor(2^16*sqrt2), but 0xB500/0x5A00 are not 2^15*sqrt2/2^14*sqrt2), so any name would be an interpretation. Same call as calc_DistanceCategory / 0x8006E480. |
| 0x80060E04 | 13 | rejected 2026-09-24 compute.md (reason there); re-checked from raw words: gp_rel x5. No new evidence. |
| 0x800404A0 | 14 | walks 0x68-byte records at a0 until s16 +2 == -1, storing a1 at +0x58 - game record layout; no neutral name. |
| 0x800404D8 | 14 | rejected 2026-09-24 compute.md (reason there); re-checked from raw words: lui [8009 800B]. No new evidence. |
| 0x8005B9C4 | 14 | NOT a leaf: func_800858D0 (owner-held PROBABLE SsUtAllKeyOff) + SsVabClose(9), zeroes g_vab_* slots - api vein; 2026-09-25 followups rejected snd_* restatements of this family. |
| 0x8003D2F4 | 15 | rejected 2026-09-24 compute.md (reason there); re-checked from raw words: gp_rel x8, lui [00F0]. No new evidence. |
| 0x8005BF3C | 15 | NOT a leaf: func_800858D0 + SsUtReverbOff/SetReverbType/SetReverbDepth - api vein; rejected 2026-09-25 (PROBABLE SsUtAllKeyOff callee). No new evidence. |
| 0x8003043C | 16 | rejected 2026-09-24 compute.md (reason there); re-checked from raw words: lui [8010]. No new evidence. |
| 0x8006E440 | 16 | pure (arg memory only): in place `for (q=a0; *q != -1; q++) *q += (u32)a0`. 2026-09-25 misc rejected a coined reloc_* name ("not worth the false-positive risk"); no new evidence, not re-proposed. |
| 0x80030D04 | 17 | rejected 2026-09-24 compute.md (reason there); re-checked from raw words: lui [8010]. No new evidence. |
| 0x80040068 | 18 | rejected 2026-09-24 compute.md (reason there); re-checked from raw words: arg-struct/record layout or data-as-code, as recorded. No new evidence. |
| 0x800400B0 | 18 | rejected 2026-09-24 compute.md (reason there); re-checked from raw words: arg-struct/record layout or data-as-code, as recorded. No new evidence. |
| 0x8004939C | 18 | rejected 2026-09-24 compute.md (reason there); re-checked from raw words: gp_rel x3, lui [800F]. No new evidence. |
| 0x80064E90 | 18 | rejected 2026-09-24 compute.md (reason there); re-checked from raw words: gp_rel x1, lui [800F]. No new evidence. |
| 0x8006505C | 18 | rejected 2026-09-24 compute.md (reason there); re-checked from raw words: gp_rel x1, lui [800F]. No new evidence. |
| 0x800650A4 | 18 | rejected 2026-09-24 compute.md (reason there); re-checked from raw words: gp_rel x1, lui [800F]. No new evidence. |
| 0x800650EC | 18 | rejected 2026-09-24 compute.md (reason there); re-checked from raw words: gp_rel x1, lui [800F]. No new evidence. |
| 0x800652AC | 18 | rejected 2026-09-24 compute.md (reason there); re-checked from raw words: gp_rel x1, lui [800F]. No new evidence. |
| 0x8004001C | 19 | rejected 2026-09-24 compute.md (reason there); re-checked from raw words: arg-struct/record layout or data-as-code, as recorded. No new evidence. |
| 0x8005509C | 19 | rejected 2026-09-24 compute.md (reason there); re-checked from raw words: lui [8010]. No new evidence. |
| 0x8003F218 | 20 | game globals (gp_rel x3, lui 800F) - game state. |
| 0x8005B868 | 20 | NOT a leaf: func_800858D0 + SsVabClose(8)/(4), zeroes g_vab_* slots - api vein; rejected 2026-09-25 followups (hard-coded VAB ids). |
| 0x8004574C | 21 | searches game table D_800EED10 (count D_800A33AC, stride 0x10) for s16 key a0 - game table walk (twin of rejected 0x80045694/0x80045510). |
| 0x80027334 | 22 | rejected 2026-09-24 compute.md (reason there); re-checked from raw words: arg-struct/record layout or data-as-code, as recorded. No new evidence. |
| 0x80060C60 | 22 | rejected 2026-09-24 compute.md (reason there); re-checked from raw words: gp_rel x9, lui [800F]. No new evidence. |
| 0x80030524 | 23 | rejected 2026-09-24 compute.md (reason there); re-checked from raw words: lui [8010]. No new evidence. |
| 0x80037AA4 | 23 | sums DIRENTRY sizes over the memcard list global - 2026-09-25 rejected a RENAME (reads globals: neither api- nor computation-restatement). No new evidence. |
| 0x80045694 | 23 | rejected 2026-09-24 compute.md (reason there); re-checked from raw words: gp_rel x1, lui [800F]. No new evidence. |
| 0x80045230 | 25 | rejected 2026-09-24 compute.md (reason there); re-checked from raw words: gp_rel x3, lui [0004 800B]. No new evidence. |
| 0x80044098 | 26 | rejected 2026-09-24 compute.md (reason there); re-checked from raw words: lui [8010]. No new evidence. |
| 0x8003D330 | 27 | rejected 2026-09-24 compute.md (reason there); re-checked from raw words: gp_rel x1, lui [00FF 800A E100 FF00]. No new evidence. |
| 0x80040400 | 27 | rejected 2026-09-24 compute.md (reason there); re-checked from raw words: arg-struct/record layout or data-as-code, as recorded. No new evidence. |
| 0x80047E5C | 27 | lerp between game table entries D_800EF800[k]/[k+1] by game globals D_800A33D4/D_800A33D8 - game state. |
| 0x8005B9FC | 28 | NOT a leaf: func_8005B9C4, func_80036EA8 [INFERRED], game_FrameLoop [INFERRED], cdrom_StartRead [CORROBORATED], func_80036F28, func_8005C2A8 - api vein, non-verified callees. |
| 0x8006517C | 29 | rejected 2026-09-24 compute.md (reason there); re-checked from raw words: gp_rel x1, lui [800F]. No new evidence. |
| 0x800651F0 | 29 | rejected 2026-09-24 compute.md (reason there); re-checked from raw words: gp_rel x1, lui [800F]. No new evidence. |
| 0x80033498 | 30 | rejected 2026-09-24 compute.md (reason there); re-checked from raw words: lui [8001 800A]. No new evidence. |
| 0x800213A0 | 33 | rejected 2026-09-24 compute.md (reason there); re-checked from raw words: lui [8000 800A]. No new evidence. |
| 0x80027438 | 33 | rejected 2026-09-24 compute.md (reason there); re-checked from raw words: lui [8001 800A]. No new evidence. |
| 0x8003FE40 | 35 | rejected 2026-09-24 compute.md (reason there); re-checked from raw words: arg-struct/record layout or data-as-code, as recorded. No new evidence. |
| 0x80086130 | 35 | rejected 2026-09-24 compute.md (reason there); re-checked from raw words: lui [800F 8010]. No new evidence. |
| 0x80037B00 | 36 | filename compare over the memcard list global - 2026-09-25 rejected (pure data walk over a global). No new evidence. |
| 0x80040CB8 | 36 | rejected 2026-09-24 compute.md (reason there); re-checked from raw words: lui [8009]. No new evidence. |
| 0x80030B10 | 38 | rejected 2026-09-24 compute.md (reason there); re-checked from raw words: arg-struct/record layout or data-as-code, as recorded. No new evidence. |
| 0x80041398 | 38 | rejected 2026-09-24 compute.md (reason there); re-checked from raw words: lui [8001 8005 FFFF]. No new evidence. |
| 0x8005B5AC | 38 | func_800858D0-based 24-slot init of game table D_800EFB78 - 2026-09-25 followups rejected. Not a leaf. |
| 0x8005B72C | 38 | SsVabClose hard-coded-id family - 2026-09-25 followups rejected. Not a leaf. |
| 0x80062FEC | 38 | rejected 2026-09-24 compute.md (reason there); re-checked from raw words: gp_rel x3, lui [800F]. No new evidence. |
| 0x80045510 | 39 | rejected 2026-09-24 compute.md (reason there); re-checked from raw words: gp_rel x2, lui [800F]. No new evidence. |
| 0x80034200 | 40 | rejected 2026-09-24 compute.md (reason there); re-checked from raw words: lui [800A 800F]. No new evidence. |
| 0x80054FDC | 40 | adds a0 to game globals D_800EFB14..D_800EFB28 (last four only if non-zero) - game-global pointer relocation; no neutral name. |
| 0x8002738C | 43 | rejected 2026-09-24 compute.md (reason there); re-checked from raw words: lui [8001 800A]. No new evidence. |
| 0x80056FE8 | 43 | rejected 2026-09-24 compute.md (reason there); re-checked from raw words: lui [800A]. No new evidence. |
| 0x80033D38 | 47 | rejected 2026-09-24 compute.md (reason there); re-checked from raw words: lui [800A 8010]. No new evidence. |
| 0x80047D94 | 50 | magic-divides a0 (0x51EB851F), stores quotient/parity to game globals D_800A33D4/D_800A33D8, lerps game table D_800EF7BC - game state. |
| 0x80037F40 | 51 | rejected 2026-09-24 compute.md (reason there); re-checked from raw words: lui [8010]. No new evidence. |
| 0x8006288C | 52 | rejected 2026-09-24 compute.md (reason there); re-checked from raw words: gp_rel x4, lui [800F]. No new evidence. |
| 0x8003D39C | 55 | rejected 2026-09-24 compute.md (reason there); re-checked from raw words: gp_rel x3, lui [00FF 7400 800A FF00]. No new evidence. |
| 0x8003FECC | 55 | rejected 2026-09-24 compute.md (reason there); re-checked from raw words: arg-struct/record layout or data-as-code, as recorded. No new evidence. |
| 0x80017FA0 | 61 | rejected 2026-09-24 compute.md (reason there); re-checked from raw words: lui [1F80]. No new evidence. |
| 0x8006CBD4 | 61 | rejected 2026-09-24 compute.md (reason there); re-checked from raw words: gp_rel x3. No new evidence. |
| 0x8001CD68 | 62 | rejected 2026-09-24 compute.md (reason there); re-checked from raw words: lui [0002 800A 8888 91A2]. No new evidence. |
| 0x800692C0 | 67 | rejected 2026-09-24 compute.md (reason there); re-checked from raw words: lui [800A]. No new evidence. |
| 0x8003032C | 68 | writes game-struct fields a0+0x44..0x4C - 2026-09-25 motion_ai rejected (not pure). No new evidence. |
| 0x800324D0 | 68 | rejected 2026-09-24 compute.md (reason there); re-checked from raw words: lui [8001]. No new evidence. |
| 0x80021280 | 72 | rejected 2026-09-24 compute.md (reason there); re-checked from raw words: lui [800A 8010]. No new evidence. |
| 0x8003800C | 79 | rejected 2026-09-24 compute.md (reason there); re-checked from raw words: gp_rel x1, lui [001F 8000 8010]. No new evidence. |
| 0x80044B30 | 80 | game globals D_800A9CF8..D_800A9D08 + jalr through table D_800F66A0 - not a leaf, game state. |
| 0x80068D88 | 81 | rejected 2026-09-24 compute.md (reason there); re-checked from raw words: gp_rel x8, lui [00FF 800A FF00]. No new evidence. |
| 0x80047A90 | 84 | rejected 2026-09-24 compute.md (reason there); re-checked from raw words: lui [6666 8009 800A 800F]. No new evidence. |
| 0x8001B138 | 87 | clears 7 game globals and steps D_800A3710 by +-0x4CC with clamps, gated by D_800A38BA/D_800A3834 - game state. |
| 0x800340A0 | 88 | rejected 2026-09-24 compute.md (reason there); re-checked from raw words: lui [800A 800F]. No new evidence. |
| 0x8007526C | 91 | rejected 2026-09-24 compute.md (reason there); re-checked from raw words: gp_rel x1. No new evidence. |
| 0x8002FDB0 | 92 | rejected 2026-09-24 compute.md (reason there); re-checked from raw words: lui [1F80]. No new evidence. |
| 0x80040B44 | 93 | rejected 2026-09-24 compute.md (reason there); re-checked from raw words: arg-struct/record layout or data-as-code, as recorded. No new evidence. |
| 0x80033BC0 | 94 | rejected 2026-09-24 compute.md (reason there); re-checked from raw words: lui [8009 800A 8010]. No new evidence. |
| 0x8002EA24 | 110 | rejected 2026-09-24 compute.md (reason there); re-checked from raw words: lui [8009]. No new evidence. |
| 0x80032314 | 111 | rejected 2026-09-24 compute.md (reason there); re-checked from raw words: lui [8009 8010]. No new evidence. |
| 0x800393C8 | 123 | rejected 2026-09-24 compute.md (reason there); re-checked from raw words: gp_rel x5, lui [800F]. No new evidence. |
| 0x8003EB84 | 143 | rejected 2026-09-24 compute.md (reason there); re-checked from raw words: gp_rel x1, lui [800A 800B]. No new evidence. |
| 0x80063BD0 | 144 | rejected 2026-09-24 compute.md (reason there); re-checked from raw words: gp_rel x4, lui [800A 800F CCCC]. No new evidence. |
| 0x80030580 | 148 | rejected 2026-09-24 compute.md (reason there); re-checked from raw words: lui [8009 8010]. No new evidence. |
| 0x8002D518 | 154 | rejected 2026-09-24 compute.md (reason there); re-checked from raw words: lui [8000 8009]. No new evidence. |
| 0x800520B8 | 202 | rejected 2026-09-24 compute.md (reason there); re-checked from raw words: lui [8005]. No new evidence. |
| 0x8003EDC0 | 234 | rejected 2026-09-24 compute.md (reason there); re-checked from raw words: gp_rel x3, lui [800A 800B 800F]. No new evidence. |
| 0x8002C22C | 252 | rejected 2026-09-24 compute.md (reason there); re-checked from raw words: lui [1F80 800A 8010]. No new evidence. |
| 0x80018300 | 317 | rejected 2026-09-24 compute.md (reason there); re-checked from raw words: lui [1F80 8000 8009]. No new evidence. |
