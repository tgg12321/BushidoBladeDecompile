# Active Work Claims

Sessions must check this file before starting work and register their claims here.
See CLAUDE.md "Cross-Session Coordination" for the full protocol.

## Current Status (2026-04-22)

**99 stubs remain** out of 1,410 functions (~93.0% resolved).

<!-- Format: | file_or_scope | session_description | claimed_at | status | -->

## Active Claims

| Scope | Session | Claimed | Status |
|-------|---------|---------|--------|
| src/code6cac.c:single_game_VoiceContorol | Codex on main — investigating Kengo-sized voice-control routine with exact-size analogue; aiming for semantic C baseline first | 2026-04-22 America/Chicago | in progress |

## Tabled Functions (7 — still INCLUDE_ASM stubs with prior attempt notes)

| Scope | Notes | Last Attempt | Status |
|-------|-------|--------------|--------|
| src/code6cac_b.c:PutRobShadow | Score 1825 (from 14820, 88% reduction). 15+ permuter rounds. Plateau: 146 reg diffs + 10 reorderings + 4 insertions + 1 deletion. Register allocation rotation dominant. Archived at archive/tabled_attempts/PutRobShadow_score1825.c | 2026-04-21 | tabled |
| src/main.c:saTan0Main | Score 1800 (from 4445, 60% reduction). 4 permuter rounds. Plateau: 48 regs + 6 reorder + 5 ins + 7 del. Target shares single jalr via common dispatch block (L80085030); mine generates per-case jalrs. Structural goto convergence needed. Archived at archive/tabled_attempts/saTan0Main_score1800.c | 2026-04-21 | tabled |
| src/code6cac_c2.c:func_8003EB84 | Score 1370 (from 3200, 57% reduction). 3 sessions. 5 LICM globals hoisted to s-regs vs target's inline t-regs. Stack -128 vs target -136. ~20+ regfix rules needed for LICM swap + t0↔t1 cascade. Archived at archive/tabled_attempts/func_8003EB84_score1370.c | 2026-04-20 | tabled |
| src/code6cac_c2.c:md_game_check_mode | Score 1945 (from 8915, 78% reduction). 2 sessions + 120+ permuter iters at floor. 234-insn target vs 235-insn mine. Frame -0x40 vs -0x48. ~60 register-level diffs (v0↔v1 swap across outer loop, t0/t1/a2 base pointer ping-pong). ~50+ regfix rules needed. Archived at archive/tabled_attempts/md_game_check_mode_score1945.c | 2026-04-20 | tabled |
| src/main.c:coli_HitPauseKatana | No C attempt. 191-insn, 3 jal. Variable shifts srav/sllv on D_800A2D04. Bit-manipulation state machine — same intractable pattern as coli_HitPauseKatana_2 (which was later matched via 43-rule compound regfix). | 2026-04-18 | tabled |
| src/code6cac.c:cpu_set_move_command_and_dir_for_no_action_2 | No C attempt. 203-insn, 27 jal, jalr through D_8008D090 function-pointer table indexed by D_800A3834. Needs jtbl rodata split for function-pointer table dispatch. | 2026-04-18 | tabled |
| src/main.c:exec_game | No C attempt. 215-insn leaf heap/scheduler. t0-t4 temps with magic constants 0x80000000/0x2FFFFFFF/0xFFFFFFF baked into prologue. Leaf temp-reg allocation intractable from C without register asm for all 4 temps. | 2026-04-18 | tabled |

## Reverted Matches (1 — matched but caused build issues)

| Scope | Notes | Last Attempt | Status |
|-------|-------|--------------|--------|
| src/code6cac_b.c:coli_hit_body_weapon | Matched via asmfix in worktree (commit b13dd90) but reverted in 44cbbed — C structure triggers GCC 2.7.2 segfault, silently truncating all subsequent functions in code6cac_b.o. Prior permuter work reached score 880 (from ~5000). Saved at tmp/coli_hit_body_weapon_best_585.c. Needs a GCC-safe C structure that avoids the crash. | 2026-04-17 | reverted |

## Completed History

All 128 completed entries removed — see `git log` for authoritative match history.
Notable milestones: Waves 1-34 (parallel agents), solo sessions 2026-04-14 onward, Codex sessions 2026-04-15..17.
