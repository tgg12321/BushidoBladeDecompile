# Knockout observations

Liveness uses the script end_frame fallback (no RAM counter established). No names or roles are inferred.

## func_8003F1E4

Status: **ok**. script reached end_frame 9851 past requested frames 8500,8540,8600,8800,9100,9500,9790 (fallback).

# Stub-and-observe: func_8003F1E4 against baseline

- scenario: versus-2p-kimigaura-far-view
- baseline: `..\bb2-worktrees\ln-trace-a\docs\naming\runtime\versus-2p-kimigaura-far-view-stub-1\baseline\capture.manifest.json` (SHA1 eeb64a80f4f6ba23e9b5ba9e865d8aead98fcc79), outcome ok, script ended True, q124_complete True
- stubbed: `..\bb2-worktrees\ln-trace-a\docs\naming\runtime\versus-2p-kimigaura-far-view-stub-1\func_8003F1E4\capture.manifest.json` (SHA1 d16ad37eba5d9e9d98c63a465a7dd92b0f971526), outcome ok, script ended True, q124_complete True
- stubs: `func_8003F1E4` (0x8003F1E4) from phase fight (frame 8230), returns with v0 unchanged, intact at the end, called while stubbed in fight
- same inputs and script

| screenshot | changed pixels | bounding box (x0,y0-x1,y1) |
|---|---|---|
| boot_intro.last (frame 5489) | 0 of 76800 (0.00%) | - |
| top_menu.first (frame 6050) | 0 of 153600 (0.00%) | - |
| top_menu.last (frame 6259) | 0 of 153600 (0.00%) | - |
| char_select.first (frame 6550) | 0 of 153600 (0.00%) | - |
| char_select.last (frame 6699) | 0 of 153600 (0.00%) | - |
| weapon_select.first (frame 6700) | 0 of 153600 (0.00%) | - |
| weapon_select.last (frame 6999) | 0 of 153600 (0.00%) | - |
| t_stage.mid (frame 7100) | 0 of 153600 (0.00%) | - |
| stage_select.first (frame 7175) | 0 of 153600 (0.00%) | - |
| t_load.last (frame 8229) | 0 of 153600 (0.00%) | - |
| fight.first (frame 8230) | 0 of 153600 (0.00%) | - |
| fight.8300 (frame 8300) | 0 of 153600 (0.00%) | - |
| fight.8500 (frame 8500) | 0 of 153600 (0.00%) | - |
| knockout.8500 (frame 8500) | 0 of 153600 (0.00%) | - |
| knockout.8540 (frame 8540) | 122543 of 153600 (79.78%) | 0,1-639,239 |
| knockout.8600 (frame 8600) | 126887 of 153600 (82.61%) | 0,11-639,239 |
| fight.8700 (frame 8700) | 129311 of 153600 (84.19%) | 0,23-639,239 |
| knockout.8800 (frame 8800) | 128113 of 153600 (83.41%) | 0,27-639,239 |
| fight.8900 (frame 8900) | 127014 of 153600 (82.69%) | 0,31-639,239 |
| fight.9100 (frame 9100) | 125142 of 153600 (81.47%) | 0,37-639,239 |
| knockout.9100 (frame 9100) | 125142 of 153600 (81.47%) | 0,37-639,239 |
| fight.9300 (frame 9300) | 122398 of 153600 (79.69%) | 0,42-639,239 |
| fight.9500 (frame 9500) | 122860 of 153600 (79.99%) | 0,38-639,239 |
| knockout.9500 (frame 9500) | 122860 of 153600 (79.99%) | 0,38-639,239 |
| fight.9700 (frame 9700) | 117883 of 153600 (76.75%) | 0,0-639,239 |
| knockout.9790 (frame 9790) | 145721 of 153600 (94.87%) | 0,0-639,239 |
| knockout.live (frame 9850) | 145304 of 153600 (94.60%) | 0,0-639,239 |

A stub counts only if the game kept running (both runs reached the script's end) and the lost effect is the function's own output (.claude/rules/naming-runtime-evidence.md). Pixels that differ beyond the function's own drawing point at changed game state. A call already in progress when the stub went in finishes normally (a leak breakpoint catches only code reaching the word after the stub).
