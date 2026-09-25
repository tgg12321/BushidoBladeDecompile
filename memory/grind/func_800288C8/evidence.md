# func_800288C8 — evidence (manual session 1, 2026-09-25)

Queue: distance 465, ASM-PARTIAL (one gte_Lzc LZCS/LZCR site, census row
:68 of the 2026-08-17 cop2 cluster, `.claude/rules/cop2-addressing-preamble-cluster.md`).
No prior ledger. Called from func_8002C61C (code6cac_b.c).

## Result (2026-09-25)
candidate.c = the landing body (header-exact gte_Lzc islands). It needs the
file's `typedef struct { s32 x, y, z; } Vec3i;` moved up from above
func_8002C61C (the body carries the moved typedef line). Full real pipeline
(cpp | cc1 | prologue_fix | maspsx | as, NO cheat stripping) on the spliced
TU: func_800288C8 is identical to build/src/code6cac_b.o except relocation
addends (the body spells every character-record field as `&D_80101EC8 + off`;
the target's relocs name splat's per-field symbols; same linked address).
Tool: tmp/f288/fullobj.sh + tmp/f288/fo.sh.

`sandbox --disable all` CANNOT score this body: the engine strips every
`__asm__` statement with no cop2 instruction (engine/inlineasm.py
_block_category), i.e. the header's own `move $12,%0` and `nop` statements,
so the sandbox compiles a different program. See "Why header-exact" below.

## What the function does
Body-overlap push between the two character records (D_80101EC8, +0x44C).
Returns if either +0x6A state is 0x28. Builds two points per character at
scratchpad 0x1F800078 (joint 1; midpoint of joints 15 and 19; joints at
0x1F8000A8 stride 0x108), tests each character-1 point against each
character-2 point with radii D_800A3144[0..1]: box reject, then
d^2 <= (r_i + r_j)^2; hits++; dist = LUT sqrt (D_8008D118 + GTE LZC);
pen = min(r - dist, 0x80); dist = max(dist,1); bias = (p2+0x20 - p1+0x20)/4;
four x/z push accumulators. After a hit: flag1/flag2 from +0x6A in
{0x13,0x1B,0x30} with +0x14E > 0x9C4; facing = |p1+0x1CA - p2+0x1CA| folded
to 0..0x800; branch to state writes (+0x286), func_80027A58,
func_80032854(id, 0x21/0x2D, rec+0xF4, 0), D_800A38A8/D_800A3876, or
hits = 0. Finally the pushes are added to +0x134/+0x13C of each record.

## Floor progression (sandbox --disable all, combined 2-statement islands)
81 (first transcription) -> 79 (Vec3 arrays) -> 54 (scratchpad struct based
at 0x1F800000 so every access has a non-zero field offset, + header clobbers
$12-$15,memory on the islands) -> 37 (all record fields as &D_80101EC8+off:
cse related-value addressing of the mode/arg pointers) -> 33 (`lz = ~1;
lz &= sp` Ruling-4 split) -> 17 (squared distance in-place in `dist`) -> 15
(LZC copy staged in `tbl`) -> 8 (`rad` local pointer). Floor 8 = the hits/i
seat swap ($s3/$s4), all operand-only.

## Why header-exact (the allocation fact that decides it)
global.c priority = floor_log2(refs)*refs/live_len (int, x10000); ties go to
the lower pseudo number (= declaration order for locals).
Combined islands: i12 7/105 = 1333 (s1), radp 7/106 = 1320 (s2),
i 7/108 = 1296 (s3), hits 10/236 = 1271 (s4) -> target wants hits in s3.
Header-exact gte_Lzc is 6 asm statements instead of 2 = 4 more RTL insns
inside the loop nest: i12 7/109 = 1284, radp 7/110 = 1272, i 7/112 = 1250,
hits 10/240 = 1250 -> exact tie, and `hits` declared before `i` wins it.
That reproduces the target with zero extra constructs, and A458 found the
same (+8 insns from its two header-exact sites fixed its rec/scr seats).
So the original was compiled from the header's six statements. The combined
form is 4 RTL insns short and needs a compensating FAKE construct the
original did not have; 35 single do-while(0) wraps and 91 pairs were
screened (tmp/f288/gen18.py / gen23.py) and none gives s1..s4 =
i12/radp/hits/i (wraps inside the inner loop lift hits past radp or lift
the rad_i copy past i).

## Constructs in the landing body (each ablated, real pipeline, header-exact)
- `rad = D_800A3144` local pointer (pointer-alias FAKE): without it 7 words
  differ (i12/radp seats swap: loop.c/cse2 initialize the hoisted j-row base
  from the i-row pointer when both use the symbol -> radp 8 refs).
- squared distance staged in `dist` (staged-value): separate once-set
  dist_sq -> 20 words differ.
- LZC input staged in `tbl` (staged-value): fresh copy -> 2 words ($v1).
- `lz = ~1; lz &= lzc_out;` (Ruling 4): one-expression form -> 6 words.
- separate loop-1 counter `c`: shared `i` -> 20 words (counter in $s1).
- `hits` declared before `i`: reversed -> 19 words (tie goes to i).
