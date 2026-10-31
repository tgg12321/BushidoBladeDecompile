# Knockout observations

Liveness uses the script end_frame fallback (no RAM counter established). No names or roles are inferred.

## func_800677B8

Status: **ok**. script reached end_frame 10330 past requested frames 8552,8554,8556,8558,8560,8563,8566,8570,8575 (fallback).

# Stub-and-observe: func_800677B8 against baseline

- scenario: versus-2p-kimigaura-kick-2p
- baseline: `docs\naming\runtime\versus-2p-kimigaura-kick-sand-stub-1\baseline\capture.manifest.json` (SHA1 b2152943603c368400e30cbf5d657d0c12b75723), outcome ok, script ended True, q124_complete True
- stubbed: `docs\naming\runtime\versus-2p-kimigaura-kick-sand-stub-1\func_800677B8\capture.manifest.json` (SHA1 0caecb02686054dc8e52f93d2b44bfe87aab1b12), outcome ok, script ended True, q124_complete True
- stubs: `func_800677B8` (0x800677B8) from phase fight (frame 8230), returns 0, intact at the end, called while stubbed in kick2p_0, kick2p_1, kick2p_2
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
| knockout.8552 (frame 8552) | 0 of 153600 (0.00%) | - |
| knockout.8554 (frame 8554) | 0 of 153600 (0.00%) | - |
| kick2p_0.45 (frame 8555) | 0 of 153600 (0.00%) | - |
| knockout.8556 (frame 8556) | 2 of 153600 (0.00%) | 469,150-469,151 |
| knockout.8558 (frame 8558) | 106 of 153600 (0.07%) | 448,140-466,147 |
| knockout.8560 (frame 8560) | 263 of 153600 (0.17%) | 426,130-462,142 |
| knockout.8563 (frame 8563) | 459 of 153600 (0.30%) | 403,121-456,139 |
| knockout.8566 (frame 8566) | 931 of 153600 (0.61%) | 359,105-437,133 |
| knockout.8570 (frame 8570) | 1674 of 153600 (1.09%) | 316,91-422,129 |
| knockout.8575 (frame 8575) | 3360 of 153600 (2.19%) | 262,80-422,125 |
| kick2p_1.45 (frame 8755) | 143 of 153600 (0.09%) | 132,132-190,157 |
| kick2p_2.45 (frame 8955) | 730 of 153600 (0.48%) | 134,132-639,239 |
| kick2p_3.45 (frame 9155) | 1317 of 153600 (0.86%) | 0,132-633,239 |
| kick_4.a45 (frame 9355) | 4314 of 153600 (2.81%) | 0,133-639,239 |
| kick_4.b45 (frame 9455) | 4509 of 153600 (2.94%) | 0,40-639,239 |
| kick_5.a45 (frame 9615) | 68587 of 153600 (44.65%) | 0,0-639,239 |
| kick_5.b45 (frame 9715) | 64021 of 153600 (41.68%) | 0,0-639,239 |
| kick_6.a45 (frame 9875) | 57516 of 153600 (37.45%) | 0,0-639,239 |
| kick_6.b45 (frame 9975) | 68211 of 153600 (44.41%) | 0,0-639,239 |
| kick_7.a45 (frame 10135) | 16800 of 153600 (10.94%) | 0,24-639,239 |
| kick_7.b45 (frame 10235) | 8002 of 153600 (5.21%) | 0,0-639,239 |
| last (frame 10329) | 8706 of 153600 (5.67%) | 0,0-639,239 |
| knockout.live (frame 10329) | 8706 of 153600 (5.67%) | 0,0-639,239 |

A stub counts only if the game kept running (both runs reached the script's end) and the lost effect is the function's own output (.claude/rules/naming-runtime-evidence.md). Pixels that differ beyond the function's own drawing point at changed game state. A call already in progress when the stub went in finishes normally (a leak breakpoint catches only code reaching the word after the stub).

## func_800677F4

Status: **ok**. script reached end_frame 10330 past requested frames 8552,8554,8556,8558,8560,8563,8566,8570,8575 (fallback).

# Stub-and-observe: func_800677F4 against baseline

- scenario: versus-2p-kimigaura-kick-2p
- baseline: `docs\naming\runtime\versus-2p-kimigaura-kick-sand-stub-1\baseline\capture.manifest.json` (SHA1 b2152943603c368400e30cbf5d657d0c12b75723), outcome ok, script ended True, q124_complete True
- stubbed: `docs\naming\runtime\versus-2p-kimigaura-kick-sand-stub-1\func_800677F4\capture.manifest.json` (SHA1 0cbec66a78a63cf3f9b6e1a73c074da4204eae2d), outcome ok, script ended True, q124_complete True
- stubs: `func_800677F4` (0x800677F4) from phase fight (frame 8230), returns 0, intact at the end, called while stubbed in kick2p_0, kick2p_1, kick2p_2
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
| knockout.8552 (frame 8552) | 0 of 153600 (0.00%) | - |
| knockout.8554 (frame 8554) | 0 of 153600 (0.00%) | - |
| kick2p_0.45 (frame 8555) | 0 of 153600 (0.00%) | - |
| knockout.8556 (frame 8556) | 0 of 153600 (0.00%) | - |
| knockout.8558 (frame 8558) | 0 of 153600 (0.00%) | - |
| knockout.8560 (frame 8560) | 16 of 153600 (0.01%) | 186,141-195,148 |
| knockout.8563 (frame 8563) | 256 of 153600 (0.17%) | 184,132-218,144 |
| knockout.8566 (frame 8566) | 634 of 153600 (0.41%) | 200,116-264,138 |
| knockout.8570 (frame 8570) | 1231 of 153600 (0.80%) | 222,102-312,134 |
| knockout.8575 (frame 8575) | 2508 of 153600 (1.63%) | 230,92-354,133 |
| kick2p_1.45 (frame 8755) | 143 of 153600 (0.09%) | 132,132-190,157 |
| kick2p_2.45 (frame 8955) | 730 of 153600 (0.48%) | 134,132-639,239 |
| kick2p_3.45 (frame 9155) | 1317 of 153600 (0.86%) | 0,132-633,239 |
| kick_4.a45 (frame 9355) | 4314 of 153600 (2.81%) | 0,133-639,239 |
| kick_4.b45 (frame 9455) | 4509 of 153600 (2.94%) | 0,40-639,239 |
| kick_5.a45 (frame 9615) | 68587 of 153600 (44.65%) | 0,0-639,239 |
| kick_5.b45 (frame 9715) | 64021 of 153600 (41.68%) | 0,0-639,239 |
| kick_6.a45 (frame 9875) | 57516 of 153600 (37.45%) | 0,0-639,239 |
| kick_6.b45 (frame 9975) | 62814 of 153600 (40.89%) | 0,0-639,239 |
| kick_7.a45 (frame 10135) | 36987 of 153600 (24.08%) | 0,0-639,239 |
| kick_7.b45 (frame 10235) | 26426 of 153600 (17.20%) | 0,0-639,239 |
| last (frame 10329) | 6137 of 153600 (4.00%) | 0,76-639,239 |
| knockout.live (frame 10329) | 6137 of 153600 (4.00%) | 0,76-639,239 |

A stub counts only if the game kept running (both runs reached the script's end) and the lost effect is the function's own output (.claude/rules/naming-runtime-evidence.md). Pixels that differ beyond the function's own drawing point at changed game state. A call already in progress when the stub went in finishes normally (a leak breakpoint catches only code reaching the word after the stub).
