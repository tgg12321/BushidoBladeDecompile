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
| src/code6cac_b.c:pad_main_control | Codex retry in isolated worktree — matched via readable semantic C body plus full-body asmfix after confirming the function was a whole-body compiler floor rather than a local regfix problem | 2026-04-17 | done — matched in worktree |
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
| src/code6cac.c:func_80022F34 | Codex retry in isolated worktree — matched via readable loop-structured C body plus full-body asmfix after confirming the remaining mismatch was branch/layout churn around the mode dispatch and table loads | 2026-04-17 | done — matched in worktree |
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
| src/code6cac_b.c:func_80033DF4 | Codex retry — MATCHED with register-shaped C baseline plus localized asmfix on the non-`0x64` tail; live-safe promotion used instruction anchors and `(s8)D_8010277C` casts | 2026-04-17 | merged |
| src/text1a_c.c:replay_camera_rob_back_loose3 | Agent 81: Wave 28 — TABLED @ score 140 floor (8 attempts); 64-bit codegen introduces `move v0,zero` that can't be eliminated at C level | 2026-04-13 | tabled |
| src/code6cac_b.c:func_800274BC | Agent 82: Wave 29 — matched (m2c + register asm t4 + inline GTE LZCS + 2 regfix rules, 8 attempts) | 2026-04-13 | done — merged to main |
| src/code6cac.c:func_8001A67C | Agent 83: Wave 29 — matched (register asm t4 + inline GTE CLZ + 1 regfix reorder, 12 attempts — overshot 8-cap) | 2026-04-13 | done — merged to main |
| src/code6cac_c_mid.c:saSeInit_2 | Codex retry in isolated worktree — matched via real C body plus instruction-anchored localized asmfix; key finding was that lab-only label anchors do not promote, so the live rule must anchor on stable instructions instead | 2026-04-17 | done — matched in worktree |
| src/main.c:func_80086014 | Agent 85: Wave 30 — matched (register asm + memory barrier + regfix insert for -0x8 frame, 12 attempts — overshot 8-cap) | 2026-04-13 | done — merged to main |
| src/code6cac_b.c:func_8002D320 | Agent 86: Wave 30 — matched on attempt 1 (inline asm port of func_8002EA24 GTE template + 3 adaptations) | 2026-04-13 | done — merged to main |
| src/text1a_c.c:func_80045878 | Agent 87: Wave 30 — matched (tail block regfix: 1 insert_after + 9 substs for v0=s1 base + v1 scratch pattern) | 2026-04-13 | done — merged to main |
| src/main.c:func_800861BC | Solo session: matched (137 insns — pointer-based address caching, 30-rule compound regfix: beq→blez, register renames, phantom frame inserts, prologue reorder) | 2026-04-17 | done — commit 943fe52 |
| src/code6cac_c.c:func_80037D14 | Later matched in live source; stale tabled entry from Wave 31 kept for history, status updated to reflect current repo state | 2026-04-17 | merged |
| src/config.c:tslPrintScreen | Codex retry — MATCHED with live C body plus two-stage regfix after isolated lab/RTL workflow proved it was a late-stage compiler-behavior problem | 2026-04-17 | merged |
| src/code6cac_c.c:func_80037D14 | Orchestrator: UNSTUCK — 3-way CU split (code6cac_c.c/c0.c/c_mid.c) + rodata split (pre.s/pre_post.s), explicit goto labels mirroring original asm layout, permuter score=0; key gotcha: code6cac_c_mid.o rodata must slot BEFORE 101C.rodata_c_pre.o for jtbl_80010AC4 (commit a5a887a) | 2026-04-14 | done — merged to main |
| src/code6cac_b.c:saSeInit | Codex retry — MATCHED with localized asmfix after a stable semantic baseline showed the remaining gap was one compact middle-window codegen block | 2026-04-17 | merged |
| src/code6cac.c:func_8001C624 | Agent 92: Wave 32 — matched (permuter: nested do-while(0) block scoping + triple load/store pattern + 0x36 indirection, 5 manual + 4 permuter runs 7550→0, commit 152f99c) | 2026-04-14 | done — merged to main |
| src/text1a_c.c:_SelectSection | Agent 93 + orchestrator regfix verification: TABLED. Permuter 3000 iters best=1915; diff is structural (77 reg + 14 del + 8 ins) not frame-only — regfix cannot bridge. DCE-allocator artifact is not the only blocker; GCC's HI/LO scheduling diverges across entire function. | 2026-04-14 | tabled |
| src/ings.c:func_80017848 | Agent 94: Wave 33 — TABLED @ 2380 (10 attempts + 2900 permuter). Pointer reload across jal math_Distance3D — GCC 2.7.2 won't hold s0/s1 across the call regardless of C structure. | 2026-04-14 | tabled |
| src/ings.c:func_80017848 | Codex retry in isolated worktree: semantic C was straightforward, but the function remained a whole-body compiler floor around the two scan loops and `math_Distance3D`. Finished as readable C plus function-scoped asmfix; `check_func.py src/ings.c func_80017848` returns `OK`. | 2026-04-17 | done — matched in worktree |
| src/text1a.c:func_80041AC8 | Agent 95: Wave 33 — TABLED @ 865 near-miss. ~3-insn delay-slot diff in inner loop; target fills bgez delay slot with `addiu a0,sp,16` (invariant) while GCC picks `addu v0,v1,s3`. Orchestrator regfix candidate — 1-2 swap rules. | 2026-04-14 | tabled — regfix candidate |
| src/code6cac_c_mid.c:func_80038170 | Agent 96: Wave 33 — TABLED @ 1730 (9000 permuter iters). Compound blockers: bit-count loop layout + jal scheduling + trailing clear-loop counter reuse. ~15 regfix rules needed — over 2-application limit. | 2026-04-14 | tabled |
| src/text1a.c:func_80041AC8 | Wave 33 post-orchestrator MATCHED. Progression 3855→325→180→0: ret var to prevent s3 hoist + func_8003E2A0(void) proto fix + 2 regfix reorder rules. Commit 2c7a7ed. | 2026-04-14 | done — matched |
| src/code6cac_b.c:coli_hit_body_weapon | Codex retry in isolated worktree: reused the saved `tmp/coli_hit_body_weapon_best_585.c` near-match, confirmed it was already semantically sound, and finished it via readable C plus function-scoped asmfix after frame-fix/regfix experiments showed a whole-body compiler floor. | 2026-04-17 | done — matched in worktree |
| src/code6cac_c2.c:func_8003EB84 | Wave 34 Agent 98: TABLED (permuter plateau 4985; 5 LICM globals hoisted to s-regs, target uses inline t-regs — 25+ regfix rules exceed batch budget) | 2026-04-14 | done — no changes |
| src/text1a_c.c:hirahira_w_ctrl_2 | Solo session: matched (134 insns — 3x3 rotation matrix, volatile u16 for lhu cosA, 63 compound regfix rules: frame 48→48, 5-way register cycle, lh→lhu+sign-ext for sinB/sinA/sinC, cosA volatile→lui/addu/lhu fix, mflo reorder) | 2026-04-17 | done |
| src/code6cac.c:myRobGeneiMove | Solo session: matched (permuter score 75→0 via 15 regfix substs + func_8001F938 label fix) | 2026-04-17 | done — commit 8968f0c |
| src/code6cac.c:func_8001EFA0 | Solo session: matched (divisor 105 + goto block layout + 2 regfix substs for beq/bne swap) | 2026-04-17 | done |
| src/code6cac_b.c:saSeInit | Codex retry in isolated worktree — matched via C body + localized asmfix chunk; key promotion gotchas were scalar-vs-array symbol form for `D_8008D118` and unique asmfix label naming | 2026-04-17 | done — matched in worktree |
| src/code6cac.c:func_8001E404 | Solo session: matched (saTan3GaugeMain camera setup — CamBuf 68-byte struct, goto labels, regfix subst for GCC forward-prop, label shift +20 for func_8001F938/func_8001EFA0) | 2026-04-17 | done |
| src/code6cac.c:func_80021A98 | Solo session: matched (permuter score 430→135 via do{}while(0) barrier + 19 regfix substs for 3-region register swap) | 2026-04-17 | done |
| src/code6cac_c2.c:func_8003DBE4 | Solo session: matched (color/palette swap — 36-rule compound regfix: frame 56→64, step restructure with branch target arithmetic, reg swap, load-delay nop insert, comparison compression) | 2026-04-17 | done |
| src/code6cac_b.c:func_8003504C | Solo session: matched (141 insns — goto inner loop prevents strength reduction, s8 pointer for D_80102785, 11-rule regfix: a2/a3 swap, 2 reorders, 6 substs, delete+insert for label-safe init move) | 2026-04-17 | done |
| src/main.c:func_80084A7C | Solo session: matched (156 insns — motion state handler, u32 threshold for sltu, 11-rule regfix: a2/a3 swap + 8 operand substs + delete + reorder) | 2026-04-17 | done — commit 0fd7fc6 |
| src/text1a.c:hirahira_w_ctrl | Solo session: matched (136 insns — petal/wind controller, register asm s7 + goto loops + 18 regfix substs for scheduling/reg reassignment/stack offset) | 2026-04-17 | done |
| src/text1a.c:saTan4FireDisp | Solo session: matched (144 insns — fire effect display, 33 regfix rules: 3-way reg cycle + frame 72→88 + inner loop restructure, 2 asmfix rules for label reposition) | 2026-04-17 | done |
| src/text1a.c:decBs0 | Solo session: matched (136 insns — 3D distance/angle calc, register asm fp + ~50 regfix rules: frame 64→72, s2↔s3 swap, dead move delete, both args to stack, store block reorder) | 2026-04-17 | done |
| src/text1a.c:AllocRobRmd | Solo session: matched (217 insns — resource alloc/display, inline s16 casts to prevent reg splitting, u32 casts for srl, CSE prevention via inline array index, regfix nop insert for maspsx load-delay bug + texture reorder, asmfix label reposition) | 2026-04-17 | done |
| src/text1a_c.c:replay_camera_rob_back_loose3 | Solo session: matched (126 insns — 3x3 rotation matrix from Euler angles, volatile u16 cast for lhu codegen, register asm $3 for angC, 8 regfix substs: v0/a0 swap + la→lui $at addressing fix + mflo v1/t6 swap) | 2026-04-17 | done |
| src/text1a_c.c:_SelectSection | Solo session: matched (135 insns — 3x3 rotation matrix, volatile u16 for lhu cosA, register asm $3 for angC, 11 regfix rules: 6 substs + delete + insert_after nop + 2 reorders for delay-slot fill + sll/mflo swap) | 2026-04-17 | done — commit 90e41cf |
| src/text1a_c.c:mot_data_set | Solo session: matched (110 insns — RGB-to-HSV, register asm $6 for b, 15 regfix rules: 14 substs for max_val copy cascade a0→t3 + 1 insert addu t3,a0,zero) | 2026-04-17 | done — commit a7a9cc6 |
| src/main.c:func_800856B0 | Solo session: matched (136 insns — leaf timer/interpolation, 66-rule compound regfix: a2↔a3 swap, duration a1→a0, negative-path t0, 2 deletes, 2 inserts, prologue reorder) | 2026-04-17 | done |
| src/main.c:func_80088740 | Solo session: TABLED at score 2780 (4 attempts). 174-insn SPU init function. C structure correct: goto loops prevent strength reduction, s16* array indexing for 0x18 loop, intermediate vars for loop constants. Remaining diffs: 59 register renames (v0↔v1 for D_800A2CDC in most sections, a0↔v1+v0↔v1 in end section) + maspsx delay-slot scheduling diffs (sh scheduled into 4 jal delay slots where target has sh before jal + nop). ~60-80 regfix rules needed. Permuter base.c saved at permuter/func_80088740/base.c. Maspsx TEXT dump: 132 insns (TEXT 0-131). | 2026-04-18 | tabled — regfix candidate |
| src/code6cac_b.c:cpu_side_move_dir_2 | Solo session: matched (Wave 35 quickwins task #1) | 2026-04-18 | done |
| src/code6cac_b.c:cpu_side_move_dir | Solo session: matched (160 insns — CPU AI side-move direction, 18-rule compound regfix: 17 substs across 3 reg groups + 2 inserts for v0_v reload and maspsx-stripped load-delay nop; .L7 label position fix via sp18=v0_v after if/else) | 2026-04-18 | done |
| src/main.c:cpu_side_move_dir_3 | Solo session: TABLED at score 1695 (81-insn, 5-sreg). Structural blocker: target GCC emits cleanup blocks (.L8008BF44 status&0x38, .L8008BFA8 fn-returns-0) PHYSICALLY BEFORE main work (.L8008BFD8). Permuter ~2000 iters plateau 1695-2310. Best base saved at permuter/cpu_side_move_dir_3/output-1695-1/. Compound regfix would need 30+ block-reordering rules. | 2026-04-18 | tabled |
| src/main.c:saTan0GaugeDraw | Solo session: TABLED at score 4300 (180-insn varargs SPU dispatcher, 4-case switch, 3 wait loops). Permuter 2300+ iters plateau 4300-4400. Best base saved at permuter/saTan0GaugeDraw/output-4300-5/. Compound regfix would need 30+ rules across 3 wait-loop register cascades + s0 va_arg walk + dispatch chain. | 2026-04-18 | tabled |
| src/code6cac_c_mid.c:pad_FuncAnalog | Solo session: TABLED at score 1620 (49-insn state machine on D_800A31F4, 4 main states + 2 substates). Block reordering for state -2 fall-through to set_state_7 + state_7 var_v0=0xB delay-slot pattern requires GCC scheduling target cant easily reach from C. Permuter 2400 iters, structurally-correct best 1620; non-equivalent permuter tricks reached 1065 (extra func_80037F08 call in state_5) but not safe to integrate. Best base saved at permuter/pad_FuncAnalog/base.c. | 2026-04-18 | tabled |
| src/main.c:coli_HitPauseKatana_2 | Solo session: TABLED at score 5100 (194-insn, 4-case switch x 2-branch inner = 8 sub-paths). SPU register write helper. Permuter 2500 iters, plateau 5100-5955. 75 reg diffs + 20 insertions + 34 deletions = compound regfix would need 50+ rules across 8 sub-paths. Best base saved at permuter/coli_HitPauseKatana_2/base.c. | 2026-04-18 | tabled |
| src/code6cac_c2.c:func_8003B9D0 | Solo session: TABLED at score 1970 (200-insn linear func, 21 jal). Structure correct (matches sibling cpu_side_move_dir_2 pattern). Remaining diff: GCC keeps saved_44c pointer at base (s0=&D_80101EDA) instead of offset (s0=&D_80101EDA+0x44C target uses); also reloads D_800A3878 between &0x2 and &0x10 checks instead of caching across all 4. Permuter 1800 iters plateau at 1970. Compound regfix would need 30+ rules across the s0-base scheme + 5 reg renames. Best base saved at permuter/func_8003B9D0/base.c. | 2026-04-18 | tabled |
| src/code6cac.c:func_8003A728 | Solo session: TABLED without C attempt. 216-insn, 11 jal, heavy GP-rel global packing (D_800A3870, D_800A36C0, D_800A36D0, D_800A36C2, D_800A36D2, D_800A38FC, D_800A3908). First 30 insns pack 4-way OR-shift bit field (sll 31, sll 30, sll 28, sll 16) into D_800A3698; then XOR-fold to D_800A369C. 4-way state mux + 3 nested condition checks. Pattern matches BB2 packed input handler — would need cross-file extern coordination + careful scheduling. Recommend dedicated multi-session attack. | 2026-04-18 | tabled |
| src/main.c:coli_HitPauseKatana | Solo session: TABLED without C attempt. 191-insn, 3 jal (one to exec_game), magic constants 0x10000 + 0x40000000 + 0x80000000. Variable shifts srav/sllv on D_800A2D04 (variable-shift bitmasking). Same allocator pattern as coli_HitPauseKatana_2 (already tabled) — bit-manipulation state machine intractable for solo. | 2026-04-18 | tabled |
| src/code6cac.c:cpu_set_move_command_and_dir_for_no_action_2 | Solo session: TABLED without C attempt. 203-insn, 27 jal, jalr through D_8008D090 function-pointer table indexed by D_800A3834. Multi-stage init + main loop + jtbl dispatch + ReturnVTMenu mid-function. Function-pointer table dispatch needs jtbl rodata split. | 2026-04-18 | tabled |
| src/main.c:exec_game | Solo session: TABLED without C attempt. 215-insn, 0 jal, leaf heap/scheduler. Uses t0-t4 temp regs directly with magic constants 0x80000000 / 0x2FFFFFFF / 0xFFFFFFF baked into prologue. Two outer loops over D_800A2D40[i*8] cells (allocator merge). Leaf temp-reg allocation + pre-loaded magic constants intractable from C — would need register asm for all 4 temps + inline asm hoists. | 2026-04-18 | tabled |
| src/code6cac_c2.c:func_8003C9A4 | Solo session: matched (94 insns — stage init + state machine on D_800A3929/D_800A3817; 4-iteration C-only refinement: a0/a1 base+offset pointer pattern, inlined func_8005C8A8 result into compound expr to free regalloc, ternary direction flip (x==0)?A:B for beqz-form vs bnez-form. Required regfix.txt fix: .L193→.L218 label rename in func_8003DBE4 (label-renumber bug) | 2026-04-18 | done |
