# Active Work Claims

Sessions must check this file before starting work and register their claims here.
See CLAUDE.md "Cross-Session Coordination" for the full protocol.

<!-- Format: | file_or_scope | session_description | claimed_at | status | -->

| Scope | Session | Claimed | Status |
|-------|---------|---------|--------|
| src/ings2.c | Agent 1: motion_make_table matched | 2026-04-09 | done — merged to main |
| src/system.c | Agent 2: tslPolyF4Init + func_80080258 matched | 2026-04-09 | done — merged to main |
| src/ings.c | Agent 3: all 3 tabled (blockers) | 2026-04-09 | done — no changes |
| include/m2c_context.h | GameObj field naming (~40 fields named) | 2026-04-09 | done — merged to main |
| src/*.c (not ings/ings2/system) | 188 symbols renamed, 825 replacements | 2026-04-10 | done — merged to main |
| include/*.h, src/*.c | 5 subsystem headers, 2508 externs removed | 2026-04-10 | done — merged to main |
| src/ings2.c:func_800832A0 | Agent 4: matched | 2026-04-10 | done — merged to main |
| src/code6cac_b2.c:func_80036FD4 | Agent 5: matched | 2026-04-10 | done — merged to main |
| src/system.c:saEft01Init | Agent 6: matched | 2026-04-10 | done — merged to main |
| src/system.c:saEft00Add | Agent 7: matched | 2026-04-10 | done — merged to main |
| src/ings2.c:conv_matrix_rotation | Agent 8: never spawned (power flicker) | 2026-04-10 | cancelled |
| src/sound.c:func_800477E8 | Agent 9: matched | 2026-04-10 | done — merged to main |
| src/code6cac.c:se_data_set | Agent 10: matched | 2026-04-10 | done — merged to main |
| src/ings2.c:motion_Open | Agent 11: matched (+ motion_Close) | 2026-04-10 | done — merged to main |
| src/code6cac_c.c:damage_DebugDisp | Agent 12: matched | 2026-04-10 | done — merged to main |
| src/*.c, include/*.h, asm/ | Item 4: 29 function renames | 2026-04-10 | done — merged to main |
| include/*.h, src/*.c | Item 5: magic number constants | 2026-04-10 | done — merged to main |
| src/*.c, asm/ | Item 6: 76 function renames (gpu/display/main) | 2026-04-10 | done — merged to main |
| src/*.c, asm/, regfix.txt | Item 6b: 30 config.c/system.c renames + regfix | 2026-04-10 | done |
| src/text1a.c:func_80044B30 | Agent 13: matched | 2026-04-10 | done — merged to main |
| src/code6cac_c.c:pad_ClearAppliBuffer | Agent 14: matched (regfix 23 ops) | 2026-04-10 | done — merged to main |
| src/main.c:md_game_check_change_main_mode_katinuki | Agent 15: matched | 2026-04-10 | done — merged to main |
| src/text1a.c:func_80040304 | Agent 16: matched (1 attempt, rodata split) | 2026-04-10 | done — merged to main |
| src/main.c:saTan4GaugeInit | Agent 17: matched (4 attempts, regfix 15 ops) | 2026-04-10 | done — merged to main |
| src/code6cac_c2.c:tslLineG5Init | Agent 18: matched (4 attempts, regfix) | 2026-04-10 | done — merged to main |
| src/code6cac_b.c:func_80034F88 | Agent 19: matched (30 regfix rules) | 2026-04-10 | done — merged to main |
| src/main.c:func_80087CAC | Agent 20: matched (+ func_80087E30, func_80087F30) | 2026-04-10 | done — merged to main |
| src/text1a.c:func_80044170 | Agent 21: matched | 2026-04-10 | done — merged to main |
| src/code6cac_b.c:func_800278C0 | Agent 22: matched (7 attempts, regfix) | 2026-04-10 | done — merged to main |
| src/text1a.c:calc_fc_frame | Agent 23: matched (permuter + regfix 10 rules) | 2026-04-10 | done — merged to main |
| src/code6cac_c.c:func_8003A450 | Agent 24: matched (7 attempts, goto + regfix) | 2026-04-10 | done — merged to main |
| src/text1a.c:hirahira_w_frie | Agent 25: matched (6 attempts, permuter + regfix) | 2026-04-11 | done — merged to main |
| src/main.c:title_mv_exec2 | Agent 26: matched (5 attempts, regfix 18 rules + regfix.py fix) | 2026-04-11 | done — merged to main |
| src/code6cac_b.c:func_80030208 | Agent 27: matched (4 attempts, goto + regfix) | 2026-04-11 | done — merged to main |
| src/text1a.c:func_800453E0 | Agent 28 (Sonnet): matched (regfix) | 2026-04-11 | done — merged to main |
| src/main.c:saEft03Start2 | Agent 29 (Sonnet): inline asm (not real decomp) | 2026-04-11 | rejected — needs C decomp |
| src/code6cac_b.c:func_8002C0DC | Agent 30 (Sonnet): matched (register asm + regfix) | 2026-04-11 | done — merged to main |
| src/main.c:saEft03Start2 | Agent 31: Wave 12 — matched (goto + regfix, split func_80089E30) | 2026-04-11 | done — merged to main |
| src/code6cac_b.c:cpu_check_tubazeri_2 | Agent 32: Wave 12 — matched (goto + 9 regfix subst) | 2026-04-11 | done — merged to main |
| src/text1a.c:videoDecCreate | Agent 33: Wave 12 — matched (goto + regfix) | 2026-04-11 | done — merged to main |
| src/main.c:func_80089E30 | Agent 34: Wave 13 — matched (goto + regfix, 85K tokens) | 2026-04-11 | done — merged to main |
| src/text1a_b.c:func_800466C0 | Agent 35: Wave 13 — matched (switch + rodata split) | 2026-04-11 | done — merged to main |
| src/code6cac_b.c:cpu_check_tubazeri | Agent 36: Wave 13 — matched (inline asm + GTE) | 2026-04-11 | done — merged to main |
| src/code6cac.c:func_8001979C | Agent 37: Wave 14 — matched (killed mid-commit, salvaged by orchestrator) | 2026-04-11 | done — merged to main |
| src/code6cac_b.c:func_80030BA8 | Agent 38: Wave 14 — matched (goto + permuter, 7 attempts) | 2026-04-11 | done — merged to main |
| src/text1a.c:func_80042FA0 | Agent 39: Wave 14 — matched (GTE + permuter, 2 attempts) | 2026-04-11 | done — merged to main |
| src/code6cac.c:func_8001DA8C | Agent 40: Wave 15 — matched (switch + 800.rodata split, 3 attempts) | 2026-04-11 | done — merged to main |
| src/code6cac_b.c:myRobGeneiDraw2 | Agent 41: Wave 15 — matched (permuter + regfix, 6 attempts) | 2026-04-11 | done — merged to main |
| src/code6cac_c2.c:md_game_check_change_sub_mode | Agent 42: Wave 15 — matched (switch + rodata split, 2 attempts) | 2026-04-11 | done — merged to main |
| src/code6cac.c:func_800203B4 | Agent 43: Wave 16 — matched (GTE .word directives, 7 attempts) | 2026-04-11 | done — merged to main |
| src/text1a.c:rob_life_ctrl | Agent 44: Wave 16 — matched (table lookups + regfix, 7 attempts) | 2026-04-11 | done — merged to main |
| src/sound.c:func_80047BE0 | Agent 45: Wave 16 — matched (goto loops + regfix s1/s2 swap) | 2026-04-11 | done — merged to main |
| src/code6cac_c.c:func_8003AE5C | Agent 46: Wave 17 — tabled (needs rodata split scaffolding) | 2026-04-11 | done — tabled |
| src/code6cac.c:func_80019310 | Agent 47: Wave 17 — matched (GTE + regfix + insert_after, 6 attempts) | 2026-04-11 | done — merged to main |
| src/text1a.c:saTan0Init | Agent 48: Wave 17 — matched (permuter + regfix delete/insert, 6 attempts) | 2026-04-11 | done — merged to main |
