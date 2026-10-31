# Knockout observations

Liveness uses the script end_frame fallback (no RAM counter established). No names or roles are inferred.

## func_800474D0

Status: **ok**. script reached end_frame 25000 past requested frames 19300,19500,19700,19900,20100,20300,20500,20700 (fallback).

# Stub-and-observe: func_800474D0 against baseline

- scenario: versus-cpu-clear-retail
- baseline: `docs\naming\runtime\versus-cpu-pier-vram-scroll-stub-1\baseline\capture.manifest.json` (SHA1 22e4ea24f411083fc9b28429d69e025951ff00ef), outcome ok, script ended True, q124_complete True
- stubbed: `docs\naming\runtime\versus-cpu-pier-vram-scroll-stub-1\func_800474D0\capture.manifest.json` (SHA1 34ef830bd6685b1204e3ee99fdc29ca98c495bbf), outcome ok, script ended True, q124_complete True
- stubs: `func_800474D0` (0x800474D0) from phase opp4 (frame 19220), returns 0, intact at the end, called while stubbed in opp4
- same inputs and script

| screenshot | changed pixels | bounding box (x0,y0-x1,y1) |
|---|---|---|
| boot_intro.last (frame 5489) | 0 of 76800 (0.00%) | - |
| top_menu.first (frame 6050) | 0 of 153600 (0.00%) | - |
| char_select.first (frame 6500) | 0 of 153600 (0.00%) | - |
| weapon_select.first (frame 6900) | 0 of 153600 (0.00%) | - |
| t_load.mid (frame 8000) | 0 of 153600 (0.00%) | - |
| opp1.8600 (frame 8600) | 0 of 153600 (0.00%) | - |
| opp1.last (frame 9250) | 0 of 153600 (0.00%) | - |
| opp2.11000 (frame 11000) | 0 of 153600 (0.00%) | - |
| opp2.12000 (frame 12000) | 0 of 153600 (0.00%) | - |
| opp2.13000 (frame 13000) | 0 of 153600 (0.00%) | - |
| opp2.last (frame 14400) | 0 of 153600 (0.00%) | - |
| opp3.16000 (frame 16000) | 0 of 153600 (0.00%) | - |
| opp3.17500 (frame 17500) | 0 of 153600 (0.00%) | - |
| opp3.last (frame 19150) | 0 of 153600 (0.00%) | - |
| knockout.19300 (frame 19300) | 0 of 153600 (0.00%) | - |
| knockout.19500 (frame 19500) | 0 of 153600 (0.00%) | - |
| opp4.19700 (frame 19700) | 0 of 153600 (0.00%) | - |
| knockout.19700 (frame 19700) | 0 of 153600 (0.00%) | - |
| knockout.19900 (frame 19900) | 0 of 153600 (0.00%) | - |
| knockout.20100 (frame 20100) | 0 of 153600 (0.00%) | - |
| knockout.20300 (frame 20300) | 0 of 153600 (0.00%) | - |
| opp4.20400 (frame 20400) | 0 of 153600 (0.00%) | - |
| knockout.20500 (frame 20500) | 0 of 153600 (0.00%) | - |
| knockout.20700 (frame 20700) | 0 of 153600 (0.00%) | - |
| opp4.last (frame 20850) | 0 of 153600 (0.00%) | - |
| opp5.21600 (frame 21600) | 0 of 153600 (0.00%) | - |
| opp5.22500 (frame 22500) | 0 of 153600 (0.00%) | - |
| opp5.23100 (frame 23100) | 0 of 153600 (0.00%) | - |
| opp5.last (frame 23340) | 0 of 153600 (0.00%) | - |
| clear.first (frame 23400) | 0 of 153600 (0.00%) | - |
| clear.mid (frame 23500) | 0 of 153600 (0.00%) | - |
| clear.last (frame 23640) | 0 of 153600 (0.00%) | - |
| t_top.mid (frame 23700) | 0 of 153600 (0.00%) | - |
| top_menu_again.first (frame 23900) | 0 of 153600 (0.00%) | - |
| top_menu_again.last (frame 24999) | 0 of 153600 (0.00%) | - |
| knockout.live (frame 24999) | 0 of 153600 (0.00%) | - |

A stub counts only if the game kept running (both runs reached the script's end) and the lost effect is the function's own output (.claude/rules/naming-runtime-evidence.md). Pixels that differ beyond the function's own drawing point at changed game state. A call already in progress when the stub went in finishes normally (a leak breakpoint catches only code reaching the word after the stub).
