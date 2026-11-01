# Knockout observations

Liveness uses the script end_frame fallback (no RAM counter established). No names or roles are inferred.

## func_800430E4

Status: **ok**. script reached end_frame 11500 past requested frames 8400,8500,8650,8800,8950,9100,9250,9400,9550,9750 (fallback).

# Stub-and-observe: func_800430E4 against baseline

- scenario: versus-2p-bamboo
- baseline: `docs\naming\runtime\versus-2p-bamboo-node-draw-stub-1\baseline\capture.manifest.json` (SHA1 898039df21bfb1a7962e4a4812d68ed95f4243a8), outcome ok, script ended True, q124_complete True
- stubbed: `docs\naming\runtime\versus-2p-bamboo-node-draw-stub-1\func_800430E4\capture.manifest.json` (SHA1 3e7ea0d3f5803638b689067899e685abed4b554f), outcome ok, script ended True, q124_complete True
- stubs: `func_800430E4` (0x800430E4) from phase fight (frame 8350), returns 0, intact at the end, called while stubbed in fight, t_ko, rematch_menu
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
| stage_select.7299 (frame 7299) | 0 of 153600 (0.00%) | - |
| stage_select.7350 (frame 7350) | 0 of 153600 (0.00%) | - |
| stage_select.7410 (frame 7410) | 0 of 153600 (0.00%) | - |
| stage_select.7470 (frame 7470) | 0 of 153600 (0.00%) | - |
| stage_select.last (frame 7524) | 0 of 153600 (0.00%) | - |
| t_load.mid (frame 7900) | 0 of 153600 (0.00%) | - |
| t_load.last (frame 8349) | 0 of 153600 (0.00%) | - |
| fight.first (frame 8350) | 0 of 153600 (0.00%) | - |
| knockout.8400 (frame 8400) | 6098 of 153600 (3.97%) | 115,72-551,159 |
| knockout.8500 (frame 8500) | 5951 of 153600 (3.87%) | 117,72-550,159 |
| fight.8650 (frame 8650) | 5919 of 153600 (3.85%) | 117,72-550,159 |
| knockout.8650 (frame 8650) | 5919 of 153600 (3.85%) | 117,72-550,159 |
| knockout.8800 (frame 8800) | 5928 of 153600 (3.86%) | 117,72-550,159 |
| fight.8950 (frame 8950) | 5917 of 153600 (3.85%) | 117,72-550,159 |
| knockout.8950 (frame 8950) | 5917 of 153600 (3.85%) | 117,72-550,159 |
| knockout.9100 (frame 9100) | 4765 of 153600 (3.10%) | 126,70-542,163 |
| fight.9250 (frame 9250) | 11492 of 153600 (7.48%) | 165,60-516,172 |
| knockout.9250 (frame 9250) | 11492 of 153600 (7.48%) | 165,60-516,172 |
| knockout.9400 (frame 9400) | 14359 of 153600 (9.35%) | 165,60-509,179 |
| fight.9550 (frame 9550) | 14387 of 153600 (9.37%) | 173,59-490,180 |
| knockout.9550 (frame 9550) | 14387 of 153600 (9.37%) | 173,59-490,180 |
| fight.9750 (frame 9750) | 12424 of 153600 (8.09%) | 211,59-443,179 |
| knockout.9750 (frame 9750) | 12424 of 153600 (8.09%) | 211,59-443,179 |
| fight.9850 (frame 9850) | 132996 of 153600 (86.59%) | 0,0-639,239 |
| fight.last (frame 9899) | 148399 of 153600 (96.61%) | 0,0-639,239 |
| t_ko.9950 (frame 9950) | 135815 of 153600 (88.42%) | 0,0-639,239 |
| t_ko.10050 (frame 10050) | 101934 of 153600 (66.36%) | 0,0-639,239 |
| t_ko.10150 (frame 10150) | 145700 of 153600 (94.86%) | 0,0-639,239 |
| t_ko.10350 (frame 10350) | 102860 of 153600 (66.97%) | 0,0-639,239 |
| t_ko.last (frame 10449) | 133179 of 153600 (86.71%) | 0,0-639,239 |
| rematch_menu.first (frame 10450) | 134075 of 153600 (87.29%) | 0,0-639,239 |
| rematch_menu.10950 (frame 10950) | 130137 of 153600 (84.72%) | 0,0-639,239 |
| rematch_menu.11150 (frame 11150) | 139213 of 153600 (90.63%) | 0,0-639,239 |
| rematch_menu.last (frame 11499) | 136550 of 153600 (88.90%) | 0,0-639,239 |
| knockout.live (frame 11499) | 136550 of 153600 (88.90%) | 0,0-639,239 |

A stub counts only if the game kept running (both runs reached the script's end) and the lost effect is the function's own output (.claude/rules/naming-runtime-evidence.md). Pixels that differ beyond the function's own drawing point at changed game state. A call already in progress when the stub went in finishes normally (a leak breakpoint catches only code reaching the word after the stub).

## func_8004DDB4

Status: **ok**. script reached end_frame 11500 past requested frames 8400,8500,8650,8800,8950,9100,9250,9400,9550,9750 (fallback).

# Stub-and-observe: func_8004DDB4 against baseline

- scenario: versus-2p-bamboo
- baseline: `docs\naming\runtime\versus-2p-bamboo-node-draw-stub-1\baseline\capture.manifest.json` (SHA1 898039df21bfb1a7962e4a4812d68ed95f4243a8), outcome ok, script ended True, q124_complete True
- stubbed: `docs\naming\runtime\versus-2p-bamboo-node-draw-stub-1\func_8004DDB4\capture.manifest.json` (SHA1 3bc7d94333578d5824ca94af16421eed955875fc), outcome ok, script ended True, q124_complete True
- stubs: `func_8004DDB4` (0x8004DDB4) from phase fight (frame 8350), returns 0, intact at the end, called while stubbed in fight, t_ko, rematch_menu
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
| stage_select.7299 (frame 7299) | 0 of 153600 (0.00%) | - |
| stage_select.7350 (frame 7350) | 0 of 153600 (0.00%) | - |
| stage_select.7410 (frame 7410) | 0 of 153600 (0.00%) | - |
| stage_select.7470 (frame 7470) | 0 of 153600 (0.00%) | - |
| stage_select.last (frame 7524) | 0 of 153600 (0.00%) | - |
| t_load.mid (frame 7900) | 0 of 153600 (0.00%) | - |
| t_load.last (frame 8349) | 0 of 153600 (0.00%) | - |
| fight.first (frame 8350) | 0 of 153600 (0.00%) | - |
| knockout.8400 (frame 8400) | 6098 of 153600 (3.97%) | 115,72-551,159 |
| knockout.8500 (frame 8500) | 5951 of 153600 (3.87%) | 117,72-550,159 |
| fight.8650 (frame 8650) | 5919 of 153600 (3.85%) | 117,72-550,159 |
| knockout.8650 (frame 8650) | 5919 of 153600 (3.85%) | 117,72-550,159 |
| knockout.8800 (frame 8800) | 5928 of 153600 (3.86%) | 117,72-550,159 |
| fight.8950 (frame 8950) | 5917 of 153600 (3.85%) | 117,72-550,159 |
| knockout.8950 (frame 8950) | 5917 of 153600 (3.85%) | 117,72-550,159 |
| knockout.9100 (frame 9100) | 4765 of 153600 (3.10%) | 126,70-542,163 |
| fight.9250 (frame 9250) | 11492 of 153600 (7.48%) | 165,60-516,172 |
| knockout.9250 (frame 9250) | 11492 of 153600 (7.48%) | 165,60-516,172 |
| knockout.9400 (frame 9400) | 14359 of 153600 (9.35%) | 165,60-509,179 |
| fight.9550 (frame 9550) | 14387 of 153600 (9.37%) | 173,59-490,180 |
| knockout.9550 (frame 9550) | 14387 of 153600 (9.37%) | 173,59-490,180 |
| fight.9750 (frame 9750) | 12424 of 153600 (8.09%) | 211,59-443,179 |
| knockout.9750 (frame 9750) | 12424 of 153600 (8.09%) | 211,59-443,179 |
| fight.9850 (frame 9850) | 132996 of 153600 (86.59%) | 0,0-639,239 |
| fight.last (frame 9899) | 148399 of 153600 (96.61%) | 0,0-639,239 |
| t_ko.9950 (frame 9950) | 135815 of 153600 (88.42%) | 0,0-639,239 |
| t_ko.10050 (frame 10050) | 101961 of 153600 (66.38%) | 0,0-639,239 |
| t_ko.10150 (frame 10150) | 145700 of 153600 (94.86%) | 0,0-639,239 |
| t_ko.10350 (frame 10350) | 102860 of 153600 (66.97%) | 0,0-639,239 |
| t_ko.last (frame 10449) | 133179 of 153600 (86.71%) | 0,0-639,239 |
| rematch_menu.first (frame 10450) | 134075 of 153600 (87.29%) | 0,0-639,239 |
| rematch_menu.10950 (frame 10950) | 130137 of 153600 (84.72%) | 0,0-639,239 |
| rematch_menu.11150 (frame 11150) | 139213 of 153600 (90.63%) | 0,0-639,239 |
| rematch_menu.last (frame 11499) | 136550 of 153600 (88.90%) | 0,0-639,239 |
| knockout.live (frame 11499) | 136550 of 153600 (88.90%) | 0,0-639,239 |

A stub counts only if the game kept running (both runs reached the script's end) and the lost effect is the function's own output (.claude/rules/naming-runtime-evidence.md). Pixels that differ beyond the function's own drawing point at changed game state. A call already in progress when the stub went in finishes normally (a leak breakpoint catches only code reaching the word after the stub).
