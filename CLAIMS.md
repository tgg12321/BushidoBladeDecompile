# Active Work Claims

Sessions must check this file before starting work and register their claims here.
See CLAUDE.md "Cross-Session Coordination" for the full protocol.

## Current Status (2026-04-27)

**43 stubs remain** out of 1,410 functions (~97.0% resolved).

Repository status:
- `wsl make clean-check` passes (`build/bb2.exe: OK`)
- `cpu_check_run_attack` is integrated and committed
- `func_8003EB84` is integrated and committed
- a 20-stub autonomous helper-swap run is fully committed
- the prior `single_game_VoiceContorol` active claim is resolved

Remaining stub distribution:
- `src/code6cac_b.c`: 16
- `src/code6cac.c`: 15
- `src/main.c`: 11
- `src/code6cac_c_mid.c`: 1

<!-- Format: | file_or_scope | session_description | claimed_at | status | -->

## Active Claims

None currently registered.

## Tabled Functions (5 — still INCLUDE_ASM stubs with prior attempt notes)

| Scope | Notes | Last Attempt | Status |
|-------|-------|--------------|--------|
| src/code6cac_b.c:PutRobShadow | Score 1825 (from 14820, 88% reduction). 15+ permuter rounds. Plateau: 146 reg diffs + 10 reorderings + 4 insertions + 1 deletion. Register allocation rotation dominant. Archived at archive/tabled_attempts/PutRobShadow_score1825.c | 2026-04-21 | tabled |
| src/main.c:saTan0Main | Score 1800 (from 4445, 60% reduction). 4 permuter rounds. Plateau: 48 regs + 6 reorder + 5 ins + 7 del. Target shares single jalr via common dispatch block (L80085030); mine generates per-case jalrs. Structural goto convergence needed. Archived at archive/tabled_attempts/saTan0Main_score1800.c | 2026-04-21 | tabled |
| src/main.c:coli_HitPauseKatana | No C attempt. 191-insn, 3 jal. Variable shifts srav/sllv on D_800A2D04. Bit-manipulation state machine — same intractable pattern as coli_HitPauseKatana_2 (which was later matched via 43-rule compound regfix). | 2026-04-18 | tabled |
| src/code6cac.c:cpu_set_move_command_and_dir_for_no_action_2 | No C attempt. 203-insn, 27 jal, jalr through D_8008D090 function-pointer table indexed by D_800A3834. Needs jtbl rodata split for function-pointer table dispatch. | 2026-04-18 | tabled |
| src/main.c:exec_game | No C attempt. 215-insn leaf heap/scheduler. t0-t4 temps with magic constants 0x80000000/0x2FFFFFFF/0xFFFFFFF baked into prologue. Leaf temp-reg allocation intractable from C without register asm for all 4 temps. | 2026-04-18 | tabled |

## Reverted Matches (1 — matched but caused build issues)

| Scope | Notes | Last Attempt | Status |
|-------|-------|--------------|--------|
| src/code6cac_b.c:coli_hit_body_weapon | Matched via asmfix in worktree (commit b13dd90) but reverted in 44cbbed — C structure triggers GCC 2.7.2 segfault, silently truncating all subsequent functions in code6cac_b.o. Prior permuter work reached score 880 (from ~5000). Saved at tmp/coli_hit_body_weapon_best_585.c. Needs a GCC-safe C structure that avoids the crash. | 2026-04-17 | reverted |

## Completed History

All completed entries removed — see `git log` for authoritative match history.
Recent milestones:
- `cpu_check_run_attack` matched and committed on 2026-04-27.
- `func_8003EB84` matched and committed on 2026-04-27.
- 20 additional stubs matched and committed on 2026-04-27:
  `func_8002CA8C`, `func_8008AF9C`, `func_80089F3C`, `func_80017A44`,
  `func_80082D34`, `func_8003FA24`, `func_80035828`, `func_80080828`,
  `tslTm2LoadImage_2`, `tslTm2LoadImage`, `func_80043454`, `func_80045B68`,
  `special_camera_set_win_cam`, `special_camera_Exec`, `saTan1MainJump`,
  `md_game_check_mode`, `ang_hosei`, `DispHira`,
  `replay_camera_get_attack_number`, `func_80034708`.
