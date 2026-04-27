# Active Work Claims

Sessions must check this file before starting work and register their claims
here. See `CLAUDE.md` "Cross-Session Coordination" for the full protocol.

## Current Status (2026-04-27)

**0 stubs remain** out of 1,410 functions (**100% resolved**).

Repository status:
- `wsl make clean-check` passes
- `build/bb2.exe: OK`
- `OK: bb2 matches!`
- all former `INCLUDE_ASM("asm/funcs", ...)` entries in `src/*.c` have been
  replaced with real C symbols
- exact binary matching is preserved through the existing `regfix` /
  `regfix_stage2` / `asmfix` pipeline

Important note:
- the project now relies heavily on `asmfix.txt` `replace_with_asmfile` helper
  swaps for the former stub set
- future semantic-C cleanups should expect translation-unit-sensitive fallout,
  especially in `main.c`, `code6cac.c`, and `single_game_VoiceContorol`

## Active Claims

None currently registered.

<!-- Format: | file_or_scope | session_description | claimed_at | status | -->

## Open Risks / Cleanup Targets

These are not stubs anymore, but they remain the most obvious places where
future cleanup or semantic-C replacement work may be fragile:

| Scope | Notes | Status |
|-------|-------|--------|
| `single_game_VoiceContorol` | Stage-2 label retargeting is source-coupled (`regfix_stage2.txt`) and now expects the current `.L71-8` / `.L103+24` layout. | fragile helper-swap support |
| `camera_set_zoom` | `regfix.txt` now uses a generalized local-label rewrite for the first rounding block (`.Lcam_rnd1`). | fragile helper-swap support |
| `main.c` helper-swaps | `saTan4GaugeInit`, `saTan0GaugeDraw`, `coli_HitPauseKatana`, `coli_HitPauseKatana_2`, `func_8008BC60`, and `SetPacketData` are pinned in `asmfix.txt` to absorb downstream label churn. | stable but coupled |
| `code6cac.c` helper-swaps | `DispPracticeMenuTex_A`, `func_8001EFA0`, `func_8001F938`, and `single_game_VoiceContorol` support rules are part of the late-stage stabilization story. | stable but coupled |

## Completed History

All stub work is complete. Use `git log --oneline` for the authoritative match
history.

Recent endgame milestones on 2026-04-27:
- `cpu_check_run_attack` matched and committed.
- `func_8003EB84` matched and committed.
- 20-stub autonomous helper-swap run committed.
- final zero-stub sweep committed, including:
  `camera_set_target_zoom`, `func_80026DA4`, `func_8001C8DC`,
  `func_80021DB0`, `func_800207C8`,
  `single_game_CheckStatusUpDataTotalOver`, `md_game_rob_data_init`,
  `mario_test_Exec`, `DispPracticeMenuTex_B`, `func_8001BE20`,
  `func_8001A820`, `func_800198D0`, `single_game_setModeRequest`,
  `func_800872A4`, `func_80087770`, `func_80086CF8`, `md_game_end`,
  `action_CheckHitZangeki`, `DispUpdateStatusMessage`, `DispStuff`,
  `AllocBukiRmd`, `saTan2Main`, `exec_game`, `func_8008C464`.
