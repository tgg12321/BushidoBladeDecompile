# Hypothesis ledger — calc_loc_mat_fw

## WARM-START PLAN (queue review 2026-10-01; read-only review, no engine runs - scores are from this ledger, [I] = inference, unmeasured; re-baseline before trusting. Any 'owner ruling/question' step = a borderline.md entry per judge-sole-gate, never a wait state: keep working the function)
- STATE: INCLUDE_ASM (`src/code6cac_b_tu2.c:1539`). No candidate ever written: 1110 = an unwritten ~1112-insn body. Canonical-asm rejected (hand-coded tier LOW). Ledger is one 2026-07-07 recon session; its m2c output `tmp/blitz/calc_loc_mat_fw_m2c.c` is GONE and its callee names (calc_loc_mat_fw, special_camera_Init, single_game_getEnemyCharId) are stale. Real callees per asm: func_8002A458 x2, func_8002CD58, func_8002CA8C, func_800274BC x2, func_80027AD8, func_80032854 x5, ratan2 x2.
- CONSTRAINTS: ordinary-C only (no GTE, no jtbl, no division). Records via `g_practice_menu_table` (Q73: the `D_80101FBC` alias row stays only until this is C). No `(u8*)&D_80101EC8` puns.
- BLOCKER: volume - there is no draft.
- PLAN:
  1. Regenerate m2c (`tools/m2c/`, `--valid-syntax`, `m2c_macros.h`).
  2. Draft in TU conventions: self/opponent `PracticeMenuRec *` = `&g_practice_menu_table[s6]`; workspace 0x1F8002B8 as `u8 *scr` with displaced casts (A458/C22C style); 0x1F800000 blocks via `SPAD`; `&D_800F5F68[id*0x1B8]` (A458); `&g_practice_menu_table[k].unk_F4` where asm uses D_80101FBC; the +0x210/+0x234 members from func_8002C22C step 1; copy the clash tail from func_800288C8.
  3. Keep both base-select forms the asm shows: arithmetic `-(s6==0)&0x24` and an if/else pointer select (evidence.md [s1]).
  4. `sandbox --diff`, fix region by region; state.json hypotheses cover the cross-jump ladder and the midpoint `/2` expressions.
- DEPENDS: after func_8002C22C (header members), func_800288C8 (field spellings), func_8002CD58 (prototype). D_80101EC8 alias row shared with func_80023F08 and func_80055B60.
- ODDS/LANE: 3-6 sessions, ~45% to 0 [I]. Grinder-suited (pure C, long ladder) but Grinder restart needs owner approval; otherwise manual.
