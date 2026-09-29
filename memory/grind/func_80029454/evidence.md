# Evidence bank — func_80029454

- [s1] Queue entry: distance=1024, verdict=ASM-STRUCTURAL, rules=1, status=parked (2026-06-09 audit REJECTED auto-auth: standard GCC-compiled C, distance>500 heuristic routing). Single rule: asmfix.txt:98 replace_with_asmfile; src stub at src/code6cac_b.c:748 `s32 func_80029454(void) { return 0; }` — 1024 = unwritten body.

- [s1] NO Kengo lead for this function: the `kengo:LOW su_menu_tuto/_DispPracticeMenuTex` comment at code6cac_b.c:747 belongs to the DispPracticeMenuTex_C stub ABOVE it (line 744), itself a misapplied PS2 UI name (LOW, size coincidence). func_80029454 has no kengo annotation.

- [s1] Structure (tmp/blitz/func_80029454_map.txt): 1025 insns @0x80029454-0x8002A454, frame 0x120 (288 — includes a 0xC0-byte local vec3[16] save buffer at sp+0x10), saves s0-s7+fp+ra. Single jr $ra, 4 m2c return points funneled to one epilogue (returns: -1 twice, two encoded pair indices).

- [s1] Signature: s32 func_80029454(void), no args. Single callsite: `D_800A3824 = func_80029454();` at src/code6cac_b.c:1033. Return encodes clash pair `((i>>1)*2) | (j>>1)` or -1 for no clash.

- [s1] Entry guard: `if (D_80101F04 < 4) return -1;` then `if (D_80102350 < 4) return -1;` (m2c renders as nested ifs; asm = two early bnez to epilogue at 0x80029498/0x800294B0) — these globals are per-player state (likely phase counters at D_80101EC8+0x3C and +0x44C+0x3C region... they are absolute globals inside the player structs: 0x80101F04 = D_80101EC8+0x3C, 0x80102350 = D_80101EC8+0x44C+0x3C — i.e. both players' field 0x3C >= 4).

- [s1] 17 backward branches but NO irregular control flow: 4x identical 16-iter vec3 save/restore loops (sp10[16] <-> scratchpad player blocks +0xA8/+0xAC/+0xB0 stride 0xC vs 0x24-ish — copy loop at entry saves, 3 restore loops before each return path); 2-player setup loop (s3<2, cursors s6+=0x18, fp+=0x24, s7+=0x44C); second 2-player workspace-build loop; 2x4 halving loop (all 24 s32 fields of 0x1F8000A8 blocks >>= 1, inner goto-style 4-iter); 2 nested clash-scan loop pairs (outer over spD0/spD4 counts, inner over the other player's count) each containing a 3-iter pointer-walk min/max box loop ((s32)p < 0x1F8002B8+0xC).

- [s1] spD0/spD4 (m2c's unassigned locals) are REAL stack fields sp+0xD0/sp+0xD4 written via `M2C_FIELD(var_a3_2, s32*, 0xC0) = 2` (var_a3_2=&sp10, advanced +4 per player) in the second player loop: per-player segment count = 2, +2 more if player field 0x8C != 0 (second weapon/broken blade) => bounds 2 or 4 for the clash scans.

- [s1] Zero mult/div, zero GTE/cop2, zero jump tables (both m2c 'irregular switch' constructs are if/else ladders over small ints — same 4-case endpoint-pointer-triple selection idiom as calc_loc_mat_fw's, storing into 0x1F8002B8+0x60/0x64/0x68 and +0x6C/0x70/0x74). Dominant mnemonics lw(220)/sw(193)/addiu(144) — this is a block-copy + compare function.

- [s1] Workspace layout at 0x1F8000A8: two 0x60-byte per-player blocks of 4 vec3 pairs (weapon segment endpoints: current +0x210..0x230 and previous frame from D_1F800024/D_1F800018 scratch mirrors and +0x234..0x248 second-weapon variants); conditional overwrite y-components with 0x186A0 (100000) when player is dead/inactive (fields 0x96!=0 || 0x92==0 || 0xC==0x1F); then ALL fields halved (>>1, arithmetic sra on s32).

- [s1] Clash core (repeated twice, pass A then pass B with role swap): for i in 0..countA: select segment-triple pointers by i; saTan0KiWareMoveB(0x1F8002B8) gate (pass A: ==0 sets skip-bit `s5 |= 1<<i` and continues; pass B: !=0 proceeds); compute min/max AABB into +0x78..0x8C; for j in 0..countB (pass B also requires s5 bit j set): select other triple by j, AABB into +0x90..0xA4, 6-way overlap test, then func_8002DE20(ws, p6C, p70, p74) precise segment test — nonzero => restore saved vec3[16] to scratchpad and return encoded (i,j).

- [s1] The pass-A/pass-B asymmetry: pass A tests player0 segments vs player1 (indices from 0x1F8000A8+0x0..0x5C selected in switch 1, +0x60..0xB4 in the inner ladder), pass B swaps (outer over +0x60.. block via switch 2, inner over +0x0.. block gated by the s5 skip-mask from pass A). Case bodies differ ONLY in which triple constants they load — write all 8 arms explicitly and let cross-jump merge the shared `sw ->0x68/0x74` tails (block_52/block_127/block_82/block_157 in m2c).

- [s1] Callees (12 calls, 4 unique): DispPracticeMenuTex_C x2 (misnamed box/hit test: (which, isPhase45, ws)), func_80032854 x6 (spark spawn ids 1, 0x26, 0x2D twice — same trio both players), saTan0KiWareMoveB x2 (segment-validity gate), func_8002DE20 x2 (precise segment-pair intersection). First player loop also sets player 0x286 = 0xB or 0x19 (stagger id) and 0xAD = 0.

- [s1] m2c decompiles CLEAN standalone (no jtbl input needed): tmp/blitz/func_80029454_m2c.c (603 lines, --valid-syntax). One m2c artifact to ignore: 'Duplicate return node #188' comment — the two -1 returns share the epilogue.

- [s1] Move-id gate set identical to calc_loc_mat_fw's: {2, 0x1B, 0x28, 0x26} on u16 field 0x6A, phase field 0xE tested via (x-4)<2U and (x-6)>=2U unsigned-range idioms, frame window 0x40 in [0xA1..0xA3] / [0xA2..0xA4] u8 bounds — shared struct vocabulary across the code6cac_b contact cluster; drafting this and calc_loc_mat_fw together amortizes the struct work.

- [s1] Ledger existed as empty skeleton (session_count=0, prior aborted blitz); no WIP checkpoint; regfix.txt has zero rules for this function.

## s2 — manual laneC, 2026-09-29: from-scratch body, 1024 -> byte-identical

- [s2] Callee names in the s1 entries are stale (pre-rename): the calls are func_800290B8 (grid
  hit test, `(idx, flag, LeafPos *tbl)`), func_8002DAD0 (x2) and func_8002DE20 (x2); the m2c
  output was regenerated at probes/m2c.c.
- [s2] Stack frame: `saved[16]` (LeafPos) at sp+0x10, `count[2]` at sp+0xD0 (the target's
  `sw v0,0xC0($a3)` with $a3 walking from sp+0x10 is GCC's combined giv for count[i]; separate
  arrays reproduce it).
- [s2] Scratchpad layout used (independent of this function's bytes): 0x1F800000 three points per
  character and 0x1F800048 two per character (func_8002C61C copies them to the records' +0x210 /
  +0x234 in its two copy loops); 0x1F8000A8 22 points per character at stride 0x108
  (func_8002A458's `pos` computation). Declared as the local struct view `ScrPad` / `SPAD`.
- [s2] Progress (harness structdiff, probes/q.py): first draft 339 (official sandbox 332) ->
  struct view of 0x1F800000 260 -> 0x286 ternary + halving byte offset 239 -> bounds re-reads
  24 -> overlap operand order 12 -> `u32 mask` 5 -> loop-counter sharing: raw 5.
  Each lever and its ablation: hypotheses.md s2 H1-H9, probes/ablations/results.txt.
- [s2] FINAL BODY (candidate.c): harness 1025/1025 insns, 5 differing lines with registers
  compared (probes/final_harness_diff.txt). Official `sandbox --disable all --diff` on
  candidate.c (probes/sandbox_candidate.txt): score 5, 1025/1025, 0 source-level hunks,
  5 operand-only hunks — all 5 are the splat pseudo-relocation artifact below.
- [s2] SCORER ARTIFACT (5): asm/funcs/func_80029454.s writes the loop-pointer inits and steps
  as `lui $s4, %hi(D_1F800024)` (0x80029500), `addiu $s4, $s4, %lo(D_1F800024)` (0x80029854),
  `lui $t1, %hi(D_1F800018)` (0x8002987C), `addiu $t1, $t1, %lo(D_1F800018)` (0x800299C4),
  `addiu $t0, $t0, %lo(D_1F800024)` (0x800299CC): splat read `lui 0x1F80` + a later `addiu 0x24`
  / `0x18` increment as symbol halves. Linked they are `lui 0x1f80` / `addiu 36` / `lui 0x1f80` /
  `addiu 24` / `addiu 36`, exactly our literal immediates; engine/score.py:265-270 rewrites the
  reference's named HI16/LO16 pair to `@hi(0x1f80)` / `@lo(0x0024)` tokens while our literal
  immediates stay literal, so the lines compare unequal. The full-build SHA1 is the check.
- [s2] Whole-TU object check (probes/tu_objdiff.py, candidate spliced into a tmp copy of
  src/code6cac_b.c, Makefile recipe): 83 functions in both objects, the other 82 identical;
  func_80029454's `objdump -dr` differs from build/src/code6cac_b.o ONLY in those five words
  (probes/tu_objdiff_func.txt: reference `3c140000`+HI16 D_1F800024 vs ours `3c141f80`, etc.).
  undefined_syms_auto.txt:1/:3 define D_1F800018 = 0x1F800018 and D_1F800024 = 0x1F800024, so
  the linked words are equal; no out-of-line `box_overlap` is emitted. The two symbol rows stay:
  asm/funcs/func_8001A820.s and asm/6CAC.s still reference them.
- [s2] No FAKE construct, no reused-local ruling (Rulings 5-12) is invoked; no asm. Constructs a
  reviewer should look at: the `ScrPad`/`SPAD` struct view (H1), the `box_overlap` inline helper
  used by both passes (H4), `u32 mask` (H5), the parenthesised halving offset (H2), loop counters
  shared across loops (H7), the K&R `extern s32 func_8002DE20();` (its Unk8002DE20Obj parameter
  type is declared later in the file).
