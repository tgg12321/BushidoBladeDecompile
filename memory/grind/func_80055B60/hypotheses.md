# Hypothesis ledger — calc_loc_mat_fw_80055B60

## WARM-START PLAN (queue review 2026-10-01; read-only review, no engine runs - scores are from this ledger, [I] = inference, unmeasured; re-baseline before trusting. Any 'owner ruling/question' step = a borderline.md entry per judge-sole-gate, never a wait state: keep working the function)
- STATE: active; INCLUDE_ASM (`src/text1b.c:2446`), distance 1108 is real - no C body ever written; only s1 recon. m2c reference (tmp/blitz/...) is GONE. `state.json` has the old name `"func": "calc_loc_mat_fw_80055B60"` - fix before any Grinder session.
- WHAT IT IS: ~1210 insns, frame 0x88, saves s0-s7/fp/ra. `void func_80055B60(s32 arg0, PadState *arg1)`; only caller func_8001BE20 (`src/code6cac_tu2.c:1139`). CPU-player pad synthesizer: works on `rec = &g_practice_menu_table[arg0]` (asm uses alias %hi(D_80101EC8)), writes a PadState-shaped 0x18 block to rec+0x3D0 and *arg1 ([I] the sp+0x10 block is exactly PadState, `include/code6cac.h:181`). 13 calls: rand x4, ratan2 x3, SquareRoot0, file_GetFlag1, func_80056CB8, func_80056FE8, func_80055948, func_80058580. Data: D_80099D88 (`StatusFlagRec[]`, code6cac.h:305), D_80106A78 (0x64-byte objs), D_800A387C, D_8009A088, D_80102790, D_800A38DC, D_800A3258 (4 bytes copied with lwl/lwr). evidence.md's R1-R13 region map is accurate.
- CONSTRAINTS: honest C; PracticeMenuRec bar (`memory/grind/func_80021424/HANDOFF.md` "Goal": no `(u8*)g_practice_menu_table + off`, no `*(T*)(rec+off)`) - siblings func_80055138 / func_80055948 (`text1b.c:2124-2360`) still carry that raw-cast debt; don't copy their spelling. Multi-write locals need Rulings 5-12 / Q51 (`.claude/rules/ordinary-c-judge-decidable.md:37`).
- PLAN:
  1. Regenerate m2c (`tools/m2c` on `asm/funcs/func_80055B60.s`, `include/m2c_context.h`); fix state.json "func".
  2. Extend PracticeMenuRec's pad region (0x352-0x44C) with members at 0x3B4-0x443: func_80055138 is the initializer (0x414 pairs, 0x428, 0x3F5, 0x3E8, 0x430, 0x438); func_80055948 covers 0x3B4/0x3B8/0x3BC/0x3C8/0x3CC; 0x3D0 is a PadState. Measure on a file-local copy first.
  3. Draft region by region per evidence.md: R3 = separate `bN = c << K` statements + one or-reduction at the end (~9 live flags, one spilled to sp+0x50); R7 = arithmetic exactly as the ledger writes it; R13 = PadState local + 4-byte unaligned struct copy for D_800A3258.
  4. `sandbox --disable all --diff`, bisect residual per region.
  5. On landing retire undefined_syms_auto.txt rows 54, 56, 587, 685 (587 shared with func_80023F08 / func_8002AB08).
- DEPENDS: PracticeMenuRec L1/L2 handle series edits the same header (coordinate). func_80058580 shares the D_80099D8F alias. No callee blocks it.
- ODDS/LANE: 3-6 sessions; small residual very likely, 0 moderate (R3 register-allocation core is the risk). Grinder (after the state.json fix, restart needs owner approval) or manual. Modality: recon/draft.

## s2 (2026-10-01, laneA) — full draft, sandbox 0
candidate.c = byte-exact pure-C body (sandbox --disable all 0, 0 source-level / 0 operand-only) when
include/code6cac.h carries the PracticeMenuRec members written by mkhdr.py (run from the repo root; it writes
tmp/b60/inc/include/code6cac.h, which the scoring harness puts ahead of include/). Floor trail: 1108 -> 417 (m2c-led
draft) -> 366 (`(cond) << K` flag values) -> 332 (the flag word assigned once) -> 296 (one expression incl. the
`? 0x1000 : 0` terms; if-chain for 0x3F5) -> 250 (generic reused locals: temp/temp2) -> 105 (slot branch first:
inverted condition) -> 74 (bit-1 gate as one condition) -> 54 -> 28 -> 19 (R7 order) -> 0 (far reuse, store order,
pad statement order, loop-counter init order).
OPEN (policy, before landing): multi-write generic locals `temp` (6 values), `temp2` (3), `far` (3), `diff`
(value then its sign) need Ruling 11 packages or split spellings; `(u16)rec->unk_6A` casts vs retyping unk_6A to
u16 (L1 plans the same); D_80106A78 typed as a 12 x 0x64 record array in text1b.c only.

## s3 (2026-10-01, laneA) — landing body (sandbox 0) with its Ruling 11 package; see evidence.md s3
Simplified to one flag-word expression (no b3..b11/flags locals), no tgt local, natural `!= 1 && != 2` tests,
D_80106A78 byte walk. Reused locals temp/temp2/temp3/work under Ruling 11 (record in evidence.md s3, probes/r11/),
i under Q51 (SOTN AddToInventory). land.py applies the src/header/undefined_syms edits (landing lock only).

## s5 (2026-10-01, laneC) — re-baselined on main; both landings prepared (evidence.md s5)
candidate.c = 0 on main's typed PracticeMenuRec + four members (probes/d6a2/hdr.py). D_80106A78 cluster redone
on main (probes/d6a2/: f/*.c bodies, mkfinal.py builds the base / A / AB file sets) — all TUs 0. Ruling 11 /
Q51 numbers identical to s3 on the new body (probes/r11b/). Next: land A (cheat-cleanup) + B (Match) under the
landing lock, rebuild == oracle, layer-2.
