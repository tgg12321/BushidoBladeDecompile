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
| src/code6cac_b.c:func_800300B4 | Agent 49: Wave 18 — matched (GTE inline asm, 6 attempts) | 2026-04-11 | done — merged to main |
| src/text1a.c:func_80041988 | Agent 50: Wave 18 — reworked on main (regfix reindex + D_80094D40 extern) | 2026-04-11 | done — merged to main |
| src/main.c:func_80085FB8 | Agent 51: Wave 18 — reworked: 3/4 sub-funcs decompiled, func_80086014 kept as asm (leaf frame intractable) | 2026-04-11 | done — merged to main |
| src/text1a.c:rob_calc_2d_position | Agent 52: Wave 19 — matched (regfix global swap + insert_after + 8 substs) | 2026-04-11 | done — merged to main |
| src/code6cac_b.c:pad_main_control | Agent 53: Wave 19 — tabled at score 405 (v0 reg-asm trick caused -2 insn mismatch, 15 attempts) | 2026-04-11 | done — tabled |
| src/text1a.c:func_800430E4 | Agent 54: Wave 19 — matched (scratchpad + struct copy + regfix, stale Wave 5 base resolved by orchestrator) | 2026-04-11 | done — merged to main |
| src/main.c:saTan1SyuryoDraw | Agent 55: Wave 20 — tabled (3 leaf funcs glued; near-miss -4 bytes, reg alloc on funcs A/B) | 2026-04-11 | done — tabled |
| src/main.c:saTan1SyuryoDraw | Orchestrator manual finish (3 funcs: saTan1SyuryoDraw + func_80089178 + func_800891B4; goto structure + unsigned div + 3 regfix substs) | 2026-04-11 | done — merged to main |
| src/text1a.c:func_80041688 | Agent 56: Wave 20 — matched (goto loops + volatile frame pad + regfix substs, 8 attempts at cap) | 2026-04-11 | done — merged to main |
| src/code6cac_b.c:coli_check_circle_hit_line | Agent 57: Wave 20 — matched (leaf + inline asm GTE + scratchpad, 1 attempt) | 2026-04-11 | done — merged to main |
| src/main.c:SetBloodSpot | Agent 58: Wave 21 — C-level match (score=0, 4 attempts) | 2026-04-12 | done — merged to main |
| main.c rodata split | Orchestrator: split 101C.rodata_text1a_b.s around jtbl_800163C0 → new 101C.rodata_main_post.s; main.o(.rodata) now sandwiched; unblocked SetBloodSpot | 2026-04-12 | done — merged to main |
| src/text1a.c:rob_life_ctrl_2 | Agent 59: Wave 21 — near-match score=10 (1-reg diff), blocked on jtbl_80010DD4 rodata split (text1a_a section); staged C body saved as comment | 2026-04-12 | superseded by orchestrator finish |
| src/text1a_c.c:rob_life_ctrl_2 | Orchestrator: three-way split text1a.c/text1a_c.c + 101C.rodata_text1a_DB8.s; Agent 59 C body + 2-subst regfix for mflo/sra $t0→$a1 | 2026-04-12 | done — merged to main |
| src/code6cac_b.c:func_800274BC | Agent 60: Wave 21 — rejected (full __asm__ block, not real decomp; same pattern as rejected Agent 29). Function is GCC-emitted signed divs + GTE inline; needs C body with u32 div cast + __asm__ GTE block only | 2026-04-12 | rejected |
| src/code6cac_b.c:mottest_disp | Agent 61: Wave 22 — matched (4 attempts, inline load between store pair) | 2026-04-12 | done — merged to main |
| src/code6cac_c2.c:SetCurrentCursor | Agent 62: Wave 22 — TABLED (10 attempts + 2 permuter runs, best score 430-500); /1800 signed-div magic constant (0x91A2B3C5) hoisted loop-invariant by GCC CSE, target recomputes in $v0 each iter; cannot force via volatile/asm/structural variants | 2026-04-12 | done — tabled |
| src/code6cac.c:func_8001F938 | Agent 63: Wave 22 — TABLED at score 455 (10 attempts); got from 6430 via u8* struct cast + if/else chain + register asm a1/a2 + unsigned state_raw; remaining diffs are delay-slot fills of `li v0,0x1000` across 3x sltiu state range checks + tail-merge of default `sh 0x44` store. Candidate for overnight permuter. GameObj offsets documented in audit log. | 2026-04-12 | done — tabled |
| src/code6cac_c2.c:SetCurrentCursor | Orchestrator: investigated LICM hoist root cause; confirmed intractable across 3 more variants (volatile, clobber, structural); documented in feedback_licm_hoist_intractability.md | 2026-04-12 | done — tabled (final) |
| src/main.c:AddTbpOfst | Agent 64: Wave 23 — matched (4 attempts + 3 permuter runs + 3-rule regfix subst, int v1 + hoisted shift + operand order) | 2026-04-12 | done — merged to main |
| src/code6cac.c:func_80022F34 | Agent 65: Wave 23 — TABLED score=740 (10 attempts); GCC LICM hoists &D_80101EC8 + const 3 to s-regs, target re-materializes inline (same pattern as feedback_licm_hoist_intractability.md) | 2026-04-12 | done — tabled |
| src/system.c:func_800826CC | Agent 66: Wave 23 — TABLED at -16 bytes (9 attempts); 5 compounding blockers: la pattern unreproducible (lui+sw vs lui+addiu+sw 0(reg)), jal delay slot scheduling, volatile vs non-volatile reordering, maspsx mangles inline asm | 2026-04-12 | done — tabled |
| src/code6cac_c2.c:SetCurrentCursor | Orchestrator: LICM unhoist breakthrough — regfix 7-rule recipe (delete preheader lui+ori, insert inline lui+ori with insert_after for delay-slot fill, $t2→$t1 cascade rename, prologue arg reorder); matched first try, recipe documented in feedback_licm_unhoist_recipe.md | 2026-04-12 | done — merged to main (22c1cc0) |
| src/code6cac_c.c:func_8003AB44 | Agent 67: Wave 24 — TABLED at score 340 (10 attempts + 400 permuter iters). 8-state switch, structurally correct with goto-based tail merge. Remaining 340pts = case 2 bnez direct vs target beqz+j (200), case 4 v1 vs v0 reg (20), 2 reorderings (120). Build also blocked on rodata split for jtbl_80010CA4 + missing sym D_800A37D8. Worktree had file-targeting bug (dc.sh replace hit main repo) — added defensive print to dc.sh in 64eeddf+ | 2026-04-12 | done — tabled |
| src/code6cac_c.c:func_8003AB44 | Orchestrator: MATCHED by splitting code6cac_c.c into code6cac_c_ab.c (func_8003AB44 onward) so jtbl_80010CA4 emits from a separate .o interposed between new asm/data/101C.rodata_c_pre.s (jtbl_80010AF4..C5C, lifted from rodata_post.s) and 101C.rodata_post.s. Goto/tail-merge C body handles 8-state switch; 5 regfix rules resolve case 2 branch polarity + case 5/6 $v1→$v0 swap | 2026-04-12 | done — matched |
| src/code6cac_b.c:single_game_SetWazaData | Agent 68: Wave 24 — TABLED at score 1705 (12k+ permuter iters). Math/loop layout correct; target spills loaded X to stack via sw/lw across 3 div-by-constant computations and GCC won't reproduce the spill from any C variant tried. TABLED comment merged from worktree (a14bb72) | 2026-04-12 | done — tabled, merged |
| src/main.c:func_8008AAD4 | Agent 69 TABLED → Orchestrator MATCHED via 26 substs + 15 deletes + 1 multi-line insert (commit 64eeddf). Demonstrated post-delete index gotcha — earlier deletes (16, 23, 28) shift later indices, so insert needed @ 58 not @ 61 | 2026-04-12 | done — merged |
| src/code6cac_b.c:single_game_SetWazaData | Orchestrator: UNSTUCK — layered regfix (13 rules, 3 substs, 1 reorder) — LICM unhoist + 3-transposition 4-cycle reg rotation + dst base bias conv + temp freeing + op swap + preheader reorder (00f3a27) | 2026-04-12 | done — matched |
| src/main.c:func_80085270 | Agent 70: Wave 25 — matched + 2 sibling wrappers (func_800853F4, func_8008541C), multifunction stub, m2c + 2 regfix reorders, 4 attempts | 2026-04-13 | done — merged to main |
| src/text1a_c.c:mot_data_set | Agent 71: Wave 25 — TABLED at 150 (10 attempts, 5 permuter rounds); RGB→HSV, 4 signed divisions, pure regalloc plateau with 30 cascading reg diffs | 2026-04-13 | done — tabled |
| src/code6cac.c:func_80022224 | Agent 72: Wave 25 — matched as tanren_calc_rob_pos (4-point distance + random middle-picker), C + 5 regfix rules (LICM un-hoist + 2 reorders), 8 attempts | 2026-04-13 | done — merged to main |
| src/code6cac_b.c:func_8002FF20 | Agent 73: Wave 26 — matched (4 attempts, natural regalloc + inline GTE asm, sp10[8] frame padding) | 2026-04-13 | done — merged to main |
| src/text1a.c:gnd_land_hit_char_die_main | Agent 74: Wave 26 — matched (10 attempts, goto layout + base-ptr deref technique for first color stores) | 2026-04-13 | done — merged to main |
| src/code6cac.c:single_game_SetAbilityData | Agent 75: Wave 26 — matched (11 attempts, inline asm GTE .word + v1 regasm + nop delay slot + long temp_s1) | 2026-04-13 | done — merged to main |
| src/code6cac_c2.c:func_8003C560 | Agent 78: Wave 27 — matched (state dispatcher) | 2026-04-13 | done — merged to main |
| src/code6cac_b.c:func_8002EA24 | Agent 77: Wave 27 — matched (inline asm GTE rotation + distance check) | 2026-04-13 | done — merged to main |
| src/code6cac_b.c:Pad_Prs | Agent 76 → Orchestrator: Wave 27 — agent crashed (API 500) after scoring 95 with 13 regfix rules; orchestrator salvaged by fixing regfix replacement escaping bug ($ -> $ in replacement side) — matched first rebuild | 2026-04-13 | done — merged to main |
| src/code6cac.c:func_8001BAE4 | Agent 79: Wave 28 — matched (Judge cast + regfix srl→sra @ 29, 5 attempts) | 2026-04-13 | done — merged to main |
| src/code6cac_b.c:func_80033DF4 | Agent 80: Wave 28 — TABLED @ score 1330 (8 attempts); regfix candidate — 3 unhoisted base pointers, ~12 rules | 2026-04-13 | tabled |
| src/text1a_c.c:replay_camera_rob_back_loose3 | Agent 81: Wave 28 — TABLED @ score 140 floor (8 attempts); 64-bit codegen introduces `move v0,zero` that can't be eliminated at C level | 2026-04-13 | tabled |
| src/code6cac_b.c:func_800274BC | Agent 82: Wave 29 — matched (m2c + register asm t4 + inline GTE LZCS + 2 regfix rules, 8 attempts) | 2026-04-13 | done — merged to main |
| src/code6cac.c:func_8001A67C | Agent 83: Wave 29 — matched (register asm t4 + inline GTE CLZ + 1 regfix reorder, 12 attempts — overshot 8-cap) | 2026-04-13 | done — merged to main |
| src/code6cac_c.c:saSeInit_2 | Agent 84: Wave 29 — TABLED @ score 600 (8 attempts); final report hallucinated, audit log shows no commit | 2026-04-13 | tabled |
| src/main.c:func_80086014 | Agent 85: Wave 30 — matched (register asm + memory barrier + regfix insert for -0x8 frame, 12 attempts — overshot 8-cap) | 2026-04-13 | done — merged to main |
| src/code6cac_b.c:func_8002D320 | Agent 86: Wave 30 — matched on attempt 1 (inline asm port of func_8002EA24 GTE template + 3 adaptations) | 2026-04-13 | done — merged to main |
| src/text1a_c.c:func_80045878 | Agent 87: Wave 30 — matched (tail block regfix: 1 insert_after + 9 substs for v0=s1 base + v1 scratch pattern) | 2026-04-13 | done — merged to main |
| src/main.c:func_800861BC | Agent 88: Wave 31 — TABLED @ score 2825 after 2300 permuter iters; compounding diffs (missing frame + 4 hoists + reorder + second-pointer alias) not regfix-bridgeable | 2026-04-13 | tabled |
| src/code6cac_c.c:func_80037D14 | Agent 89: Wave 31 — TABLED (no attempts); jtbl_800109D8 lives inside 101C.rodata_pre.s — requires rodata split scaffolding (Phase A orchestrator work) | 2026-04-13 | tabled |
| src/config.c:tslPrintScreen | Agent 90: Wave 31 — TABLED @ score 1170 (11 attempts + 5 permuter runs); persistent ptr in s6 vs target s0 + 4 jal scheduling diffs — too many compound reassignments | 2026-04-13 | tabled |
| src/code6cac_c.c:func_80037D14 | Orchestrator: UNSTUCK — 3-way CU split (code6cac_c.c/c0.c/c_mid.c) + rodata split (pre.s/pre_post.s), explicit goto labels mirroring original asm layout, permuter score=0; key gotcha: code6cac_c_mid.o rodata must slot BEFORE 101C.rodata_c_pre.o for jtbl_80010AC4 (commit a5a887a) | 2026-04-14 | done — merged to main |
| src/code6cac_b.c:saSeInit | Agent 91: Wave 32 — TABLED @ 430 (5 attempts); regalloc plateau + 2 hard deletions (GCC DCE of redundant move t4,v0) + inverse arg-load order; regfix candidate not attempted | 2026-04-14 | tabled |
| src/code6cac.c:func_8001C624 | Agent 92: Wave 32 — matched (permuter: nested do-while(0) block scoping + triple load/store pattern + 0x36 indirection, 5 manual + 4 permuter runs 7550→0, commit 152f99c) | 2026-04-14 | done — merged to main |
| src/text1a_c.c:_SelectSection | Agent 93 + orchestrator regfix verification: TABLED. Permuter 3000 iters best=1915; diff is structural (77 reg + 14 del + 8 ins) not frame-only — regfix cannot bridge. DCE-allocator artifact is not the only blocker; GCC's HI/LO scheduling diverges across entire function. | 2026-04-14 | tabled |
| src/ings.c:func_80017848 | Agent 94: Wave 33 — TABLED @ 2380 (10 attempts + 2900 permuter). Pointer reload across jal math_Distance3D — GCC 2.7.2 won't hold s0/s1 across the call regardless of C structure. | 2026-04-14 | tabled |
| src/text1a.c:func_80041AC8 | Agent 95: Wave 33 — TABLED @ 865 near-miss. ~3-insn delay-slot diff in inner loop; target fills bgez delay slot with `addiu a0,sp,16` (invariant) while GCC picks `addu v0,v1,s3`. Orchestrator regfix candidate — 1-2 swap rules. | 2026-04-14 | tabled — regfix candidate |
| src/code6cac_c_mid.c:func_80038170 | Agent 96: Wave 33 — TABLED @ 1730 (9000 permuter iters). Compound blockers: bit-count loop layout + jal scheduling + trailing clear-loop counter reuse. ~15 regfix rules needed — over 2-application limit. | 2026-04-14 | tabled |
| src/text1a.c:func_80041AC8 | Wave 33 post-orchestrator MATCHED. Progression 3855→325→180→0: ret var to prevent s3 hoist + func_8003E2A0(void) proto fix + 2 regfix reorder rules. Commit 2c7a7ed. | 2026-04-14 | done — matched |
| src/code6cac_b.c:coli_hit_body_weapon | Wave 34 Agent 97: TABLED (permuter plateau 585; needs frame_fix_funcs.txt + regfix for LICM of `addiu v1,a3,0xa`) — best base saved at tmp/coli_hit_body_weapon_best_585.c | 2026-04-14 | done — no changes |
| src/code6cac_c2.c:func_8003EB84 | Wave 34 Agent 98: TABLED (permuter plateau 4985; 5 LICM globals hoisted to s-regs, target uses inline t-regs — 25+ regfix rules exceed batch budget) | 2026-04-14 | done — no changes |
| src/text1a_c.c:hirahira_w_ctrl_2 | Wave 34 Agent 99: TABLED (permuter plateau 2895; 132-insn rotation transform, GCC schedules rot[0] load ahead of target, lh vs lhu+sll/sra divergence — 20+ regfix rules intractable) | 2026-04-14 | done — no changes |
| src/code6cac.c:myRobGeneiMove | Solo session: matched (permuter score 75→0 via 15 regfix substs + func_8001F938 label fix) | 2026-04-17 | done — commit 8968f0c |
| src/code6cac.c:func_8001EFA0 | Solo session: matched (divisor 105 + goto block layout + 2 regfix substs for beq/bne swap) | 2026-04-17 | done |
| src/code6cac_b.c:saSeInit | Codex retry in isolated worktree — matched via C body + localized asmfix chunk; key promotion gotchas were scalar-vs-array symbol form for `D_8008D118` and unique asmfix label naming | 2026-04-17 | done — matched in worktree |
| src/code6cac.c:func_8001E404 | Solo session: matched (saTan3GaugeMain camera setup — CamBuf 68-byte struct, goto labels, regfix subst for GCC forward-prop, label shift +20 for func_8001F938/func_8001EFA0) | 2026-04-17 | done |
