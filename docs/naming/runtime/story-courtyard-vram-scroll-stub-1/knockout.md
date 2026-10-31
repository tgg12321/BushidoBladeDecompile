# Knockout observations

Liveness uses the script end_frame fallback (no RAM counter established). No names or roles are inferred.

## func_8004775C

Status: **ok**. script reached end_frame 91000 past requested frames 85300,85600,86000,86300,86600,87000,87500,88000,88500,89000 (fallback).

# Stub-and-observe: func_8004775C against baseline

- scenario: story-gengoro-stage-0e
- baseline: `docs\naming\runtime\story-courtyard-vram-scroll-stub-1\baseline\capture.manifest.json` (SHA1 51b96f11628bb42b0ee575976e141ec4bf878ae7), outcome ok, script ended True, q124_complete True
- stubbed: `docs\naming\runtime\story-courtyard-vram-scroll-stub-1\func_8004775C\capture.manifest.json` (SHA1 f43d2b0b91a781b4e4ba32cd6b60348da44a2fa6), outcome ok, script ended True, q124_complete True
- stubs: `func_8004775C` (0x8004775C) from phase stage_0e (frame 85233), returns with v0 unchanged, intact at the end, called while stubbed in stage_0e
- same inputs and script

| screenshot | changed pixels | bounding box (x0,y0-x1,y1) |
|---|---|---|
| boot_intro.last (frame 5489) | 0 of 76800 (0.00%) | - |
| top_menu.first (frame 6060) | 0 of 153600 (0.00%) | - |
| char_select.first (frame 6950) | 0 of 153600 (0.00%) | - |
| char_select.last (frame 7299) | 0 of 153600 (0.00%) | - |
| weapon_select.first (frame 7360) | 0 of 153600 (0.00%) | - |
| stage_02.card (frame 8360) | 0 of 153600 (0.00%) | - |
| stage_02.10000 (frame 10000) | 0 of 153600 (0.00%) | - |
| stage_16.27500 (frame 27500) | 0 of 153600 (0.00%) | - |
| stage_15.35500 (frame 35500) | 0 of 153600 (0.00%) | - |
| stage_11.55000 (frame 55000) | 0 of 153600 (0.00%) | - |
| stage_01.71500 (frame 71500) | 0 of 153600 (0.00%) | - |
| stage_0d.80000 (frame 80000) | 0 of 153600 (0.00%) | - |
| stage_0d.85000 (frame 85000) | 0 of 153600 (0.00%) | - |
| knockout.85300 (frame 85300) | 0 of 153600 (0.00%) | - |
| knockout.85600 (frame 85600) | 0 of 153600 (0.00%) | - |
| stage_0e.86000 (frame 86000) | 22749 of 153600 (14.81%) | 0,112-444,239 |
| knockout.86000 (frame 86000) | 22749 of 153600 (14.81%) | 0,112-444,239 |
| knockout.86300 (frame 86300) | 9579 of 153600 (6.24%) | 0,115-226,239 |
| stage_0e.86500 (frame 86500) | 15532 of 153600 (10.11%) | 0,104-422,239 |
| knockout.86600 (frame 86600) | 11028 of 153600 (7.18%) | 0,107-263,239 |
| knockout.87000 (frame 87000) | 9903 of 153600 (6.45%) | 0,188-491,239 |
| stage_0e.87500 (frame 87500) | 5193 of 153600 (3.38%) | 0,104-639,132 |
| knockout.87500 (frame 87500) | 5193 of 153600 (3.38%) | 0,104-639,132 |
| knockout.88000 (frame 88000) | 1385 of 153600 (0.90%) | 0,91-214,120 |
| stage_0e.88500 (frame 88500) | 3300 of 153600 (2.15%) | 0,91-628,102 |
| knockout.88500 (frame 88500) | 3300 of 153600 (2.15%) | 0,91-628,102 |
| knockout.89000 (frame 89000) | 2035 of 153600 (1.32%) | 195,81-639,112 |
| stage_0e.90000 (frame 90000) | 148483 of 153600 (96.67%) | 0,0-639,239 |
| stage_0e.last (frame 90999) | 143721 of 153600 (93.57%) | 0,0-639,239 |
| knockout.live (frame 90999) | 143721 of 153600 (93.57%) | 0,0-639,239 |

A stub counts only if the game kept running (both runs reached the script's end) and the lost effect is the function's own output (.claude/rules/naming-runtime-evidence.md). Pixels that differ beyond the function's own drawing point at changed game state. A call already in progress when the stub went in finishes normally (a leak breakpoint catches only code reaching the word after the stub).
