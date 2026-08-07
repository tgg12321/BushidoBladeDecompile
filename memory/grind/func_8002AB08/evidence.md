# Evidence bank — calc_loc_mat_fw

- [s1] Queue entry: distance=1110, verdict=ASM-STRUCTURAL, rules=1, status=parked (2026-06-09 canonical-asm audit REJECTED: 'reads as standard GCC-compiled C', routed by distance>500 heuristic only). The single rule is asmfix.txt:96 replace_with_asmfile; src/code6cac_b.c:754 is an empty stub `void calc_loc_mat_fw(s32 a0) { (void)a0; }` — the 1110 IS the unwritten body, not a plateau.

- [s1] TWIN DISPELLED (inherited from memory/grind/calc_loc_mat_fw_80055B60 s1 evidence, 2026-07-07): normalized insn-stream similarity vs calc_loc_mat_fw_80055B60 is 0.0117 — splat first-pass naming tag only. One solution will NOT close both; do not re-run the comparison. The twin is a CPU input synthesizer in text1b; THIS function is collision/contact geometry in code6cac_b.

- [s1] KENGO LEAD (stub comment src/code6cac_b.c:757): `kengo:MED | se_fc/calc_loc_mat_fw | 1074i | -38 3.5% no-affinity fallback` — the PS2 successor has a SAME-NAMED function in se_fc; `python3 tools/kengo_ref.py calc_loc_mat_fw --bb2` dumps its MIPS disassembly side-by-side as a structure reference (Kengo/ is disassembly + symbol names, not C source).

- [s1] Structure (tmp/blitz/calc_loc_mat_fw_map.txt): 1112 insns @0x8002AB08-0x8002BC64, frame 0xC0, saves s0-s7+fp+ra (10 regs). Single jr $ra; the mid-body `return` (arg==1 fast path, m2c line 449-453) reaches the epilogue via `j 0x8002BC34`. void return.

- [s1] 5 loops: (1) outer player loop s6=0..1 spanning the whole body (bnez 0x8002BC2C->0x8002AB60), with scratchpad block cursor sp88 += 0x108/iter; (2) contact-pass loop s7=0..t1 (t1 = 0..3 computed from state tests); (3) inner sample loop s5=0..s4 where s4 = (segLen^2 > 0x5F5E0F) ? 4 : 2; (4) 3-iter pointer-walk min/max bounding-box loop (var_a3 += 4 while < base+0xC, loop_78); (5) 22-iter bit-scan min-squared-distance loop (var_s4_2 < 0x16, cursor strides 0xC into scratchpad 0xA8 table and 0x14 into D_800F5F68+s6*0x1B8, threshold u16 at +0xE).

- [s1] No jump tables (jr $ra only), zero cop2/GTE, zero div; 19 mult (all squared-distance / scaled-angle products). m2c decompiles CLEAN with no extra input: tmp/blitz/calc_loc_mat_fw_m2c.c (628 lines, --valid-syntax).

- [s1] Scratchpad-heavy: workspace struct at 0x1F8002B8 (fields +0..0x2C endpoint vectors, +0x30/0x3C midpoints, +0x60/0x64/0x68 ptr triple, +0x78..0x8C min/max box, +0xB4/+0xC4 result masks, +0xC8..0xD0 ref pos), player blocks at 0x1F800000/0x24 and 0x1F800048/0x60 (selected by s6), 0x1F8002C4/0x1F8002D0 stashed in sp locals sp70/sp68 at entry. Completed-C precedent for spelling: plain casts/pointer locals (src/code6cac_b.c:925 `s32 *t0 = (s32 *)0x1F8002B8;`, code6cac.c:3122, text1b.c:11716) — scratchpad is fast RAM, not MMIO; no volatile.

- [s1] Base-select idioms appear in TWO distinct codegen forms: (a) arithmetic `-(s6==0) & 0x24` (m2c lines 190/216/468/484/509 — sltu/negu/andi from a conditional-constant expression) and (b) explicit two-arm pointer select `p = 0x1F800000; if (s6==0) p = 0x1F800024;` (lines 254-257 etc.). The original source mixed a ?:/arith form and an if-else form — preserve which regions use which when drafting.

- [s1] Player struct: var_s3 = OPPONENT = &D_80101EC8 + (s6==0 ? 0x44C : 0); sp30 = SELF = &D_80101EC8 + s6*0x44C (same 0x44C marionation player stride as the text1b twin). Key offsets: 0x6A move-id (compare sets {2,0x1B,0x28,0x26}, {0x13,0x1B,0x30}, {3,7,0xD,0x2C}, 0x15, 0x21/0x2D spark ids), 0xC state (0x1D/0xE/0x1F), 0xE phase (ranges 4-5, 6-7), 0x40 frame vs u8 windows 0xA1/0xA2/0xA3, 0x8C, 0x92, 0x96, 0x3C, 0x26C, 0x34A countdown u8, 0x114/0x11C velocity, 0x134/0x13C position-nudge accumulators, 0x1CA/0x1D8 angles, 0x210/0x214/0x218 + 0x234-0x23C world segment endpoints, 0x286.

- [s1] Callees (14 calls, 7 unique): func_80032854 x5 (spark/hit-effect spawn: (playerIdxBool, effectId, vecPtr, matPtr)); func_8002A458 x2 (&sp20,&sp24 out-params = contact mask pair); func_800274BC x2 (copies +0x114 velocity block to D_800A37E8); single_game_getEnemyCharId x2 (the atan2 misnomer, angle from dx/dz); special_camera_Init(0x1F8002B8, z) result feeds func_8002CA8C(sp30, ., sp60) directly (nested call); calc_teasi_loc_fw(0, sp30, s5, angDiff, t0, fp, s1, 0) 8-arg tail-region call.

- [s1] Distinctive shapes for the draft: midpoint stores use (a+b + ((a+b)>>31)) >> 1 (signed /2, six in a row at +0x30..0x44 with the LAST one m2c'd as literal `/ 2` — same expression, register-pressure variant); the s5-indexed endpoint-pointer-triple selection (m2c lines 331-376) is a goto-shared if/else ladder over (s4==2, s5 in 0..3) with tails block_73/block_76 shared — expect cross-jump merges; the angle gates are `(x & 0xFFF); v = x < 0x400; if (x >= 0x800) v = (0x1000 - x) < 0x400;` folded-circle tests.

- [s1] Both-players-clash special case (m2c 570-589): opp move in {0x13,0x1B,0x30} && self move == 0x15 -> if either phase in 6..7: self->0x286=5 + two func_80032854 spark calls (0x21, 0x2D) targeting D_80101FBC(+0x44C); else D_800A38A8=1, D_800A3876=s6 (global clash latch consumed elsewhere).

- [s1] Entry gate: `if (arg0 == 1 && D_800A38AE == s6) continue;` (skip that player); arg0==1 also takes the early-return fast path after the min-distance scan (func_800274BC + func_80032854(s6,4,minSlotVec,&D_800A37E8); return). Callsites: calc_loc_mat_fw(1) at code6cac_b.c:898, calc_loc_mat_fw(0) at :1045.

- [s1] Ledger did not exist (this init); no WIP checkpoint; regfix.txt has zero rules for this function.
