# cpu_check_run_attack — RESOLVED (historical integration note)

## Resolution Update (2026-04-27)
This note is now historical. `cpu_check_run_attack` has been integrated into
`src/code6cac.c`, the stale `regfix`/`asmfix` fallout was repaired, and the
repo returned to a full matching build. Keep this file as background for the
integration path and the translation-unit cascade pattern it exposed.

## Status
Function was fully decompiled and matching at the binary level before
integration. The original integration attempt into `code6cac.c` caused GCC
label renumbering cascade that changed codegen for other C-compiled functions
in the same file.

## Patches saved
- `tmp/cpu_check_run_attack_c_code.patch` — C code replacing INCLUDE_ASM
- `tmp/cpu_check_run_attack_regfix.patch` — 91 regfix rules for cpu_check_run_attack
- `tmp/cpu_check_run_attack_asmfix.patch` — 3 asmfix rules (insert_after + replace_first)
- `tmp/cpu_check_run_attack_stage2.patch` — label renumber fixes for single_game_VoiceContorol

## Cascade impact (9 functions)
| Function | Delta | Cause |
|----------|-------|-------|
| func_80017FA0 | +4 | Scratchpad lui/sw→lui/ori/sw |
| marionation_camera_Exec | +20 | Multiple scratchpad + reg realloc |
| func_80019310 | -8 | Scheduling/reg changes |
| func_8001979C | -8 | Scheduling/reg changes |
| DispPracticeMenuTex_A | -4 | Instruction reorder |
| func_8001E404 | -4 | Constant fold change |
| func_80022F34 | -4 | Frame size change |
| func_8002304C | -8 | LICM un-hoist |
| camera_set_zoom | +8 | Delay slot/reg changes |
| **Total** | **-4** | |

## Historical integration strategy at the time
The original tabled conclusion was to either:
- wait until more `code6cac.c` functions were C-compiled, reducing cascade scope
- or add a large block of compound `regfix` rules for the affected functions

That is no longer the current state. The function is already integrated and the
translation-unit fallout was repaired directly.

## Historical verification snapshot
- `cpu_check_run_attack` binary matched target before integration
- the saved `regfix` / `asmfix` patches validated at the time of this note
