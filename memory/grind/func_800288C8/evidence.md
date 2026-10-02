# func_800288C8 — evidence

Session history (s1 2026-09-25: real-pipeline proof, layer-2 FAIL on the
then-unscoreable islands + `tbl` copy, rotation) is in git
(`git log -- memory/grind/func_800288C8/`).

## What the function does
Body-overlap push between the two character records (g_practice_menu_table[0]
/ [1]). Returns if either unk_6A is 0x28. Builds two points per character in
SPAD->unk78 (joint 1; midpoint of joints 15 and 19 of SPAD->unkA8), tests each
character-0 point against each character-1 point with radii D_800A3144[0..1]:
box reject, then d^2 <= (r_i + r_j)^2; hits++; dist = LUT sqrt
(g_sqrt_table_u8 + GTE LZC); pen = min(r - dist, 0x80); dist = max(dist,1);
bias = (unk_20[1] - unk_20[0])/4; four x/z push accumulators. After a hit:
flag1/flag2 from unk_6A in {0x13,0x1B,0x30} with unk_14E > 0x9C4; facing =
|unk_1C8.vy[0] - unk_1C8.vy[1]| folded to 0..0x800; branch to unk_286 writes,
func_80027A58, func_80032854(id, 0x21/0x2D, &unk_F4, 0), D_800A38A8/D_800A3876,
or hits = 0. Finally the pushes are added to unk_134.vx/.vz of each record.

## s2 (2026-10-02, lane oct2-a5) — re-baseline on today's data model
Re-modelled: PracticeMenuRec fields via g_practice_menu_table[k], SPAD with
ScrPad's `u8 unk78[0x30]` pad declared as `LeafPos unk78[2][2]` (no other C
reads those bytes), g_sqrt_table_u8, header-exact inline_o.h gte_Lzc island
(now scored by engine/gtemacro.py PINNED). First measurement: sandbox
--disable all **0/465** (all 31 hunks not-scored relocation addends).

Ablations (sandbox --disable all, 2026-10-02; files in rejected/):
| construct | ablated spelling | score |
|---|---|---|
| `rad = D_800A3144` local pointer | `D_800A3144[i] + D_800A3144[j]` | 0 -> dropped |
| `hits` declared before `i` | reversed | 0 -> order free |
| `lz = ~1; lz &= lzc_out;` (s1) | `shift = lzc_out; shift = 0x16 - (shift & ~1);` | 0 (one local fewer; code6cac.c LZC sites' spelling) |
| shift staged via `shift = lzc_out;` | `shift = 0x16 - (lzc_out & ~1);` (rejected/s2-shift-one-expression-4.c) | 4 |
| also tried: `lz = lzc_out & ~1`, `~1 & lzc_out`, `lz = lzc_out; lz &= ~1`, `& 0xFFFFFFFE`, `>>1<<1`, `- (x&1)`, `shift = ~1; shift &= x` | | 4-5 |
| `tbl = dist;` LZC-input copy | no copy, island reads dist (rejected/s2-no-lzc-copy-13.c) | 13 |
| | fresh copy local `lzc_in` (rejected/s2-fresh-lzc-copy-12.c) | 12 |
| `dist` = squared, then root | separate `dist_sq` (rejected/s2-dist-sq-separate-7.c) | 7 |
| separate loop-1 counter `c` | shared `i` | 18 (kept; ordinary) |

Landing body = candidate.c: three FAKE-annotated locals (dist, tbl, shift),
nothing else codegen-only. Same three shapes as the landed func_8002A458
(temp / temp2 / LZC staging) in this file.
