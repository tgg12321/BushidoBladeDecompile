# Knockout observations

Liveness uses the script end_frame fallback (no RAM counter established). No names or roles are inferred.

## func_80067830

Status: **ok**. script reached end_frame 10330 past requested frames 9150,9152,9154,9155,9156,9157,9158,9160,9162,9165 (fallback).

# Stub-and-observe: func_80067830 against baseline

- scenario: versus-2p-kimigaura-kick-2p
- baseline: `docs\naming\runtime\versus-2p-kimigaura-kick-particle-stub-2\baseline\capture.manifest.json` (SHA1 924cc770b7e6f019f645f49aa88161904bde81f8), outcome ok, script ended True, q124_complete True
- stubbed: `docs\naming\runtime\versus-2p-kimigaura-kick-particle-stub-2\func_80067830\capture.manifest.json` (SHA1 eaa48d39c071e733b23598f69b0e0428802b9abe), outcome ok, script ended True, q124_complete True
- stubs: `func_80067830` (0x80067830) from phase fight (frame 8230), returns 0, intact at the end, called while stubbed in kick2p_3, kick_4, kick_5, kick_6, kick_7
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
| kick2p_0.45 (frame 8555) | 0 of 153600 (0.00%) | - |
| kick2p_1.45 (frame 8755) | 0 of 153600 (0.00%) | - |
| kick2p_2.45 (frame 8955) | 0 of 153600 (0.00%) | - |
| knockout.9150 (frame 9150) | 0 of 153600 (0.00%) | - |
| knockout.9152 (frame 9152) | 0 of 153600 (0.00%) | - |
| knockout.9154 (frame 9154) | 0 of 153600 (0.00%) | - |
| kick2p_3.45 (frame 9155) | 0 of 153600 (0.00%) | - |
| knockout.9155 (frame 9155) | 0 of 153600 (0.00%) | - |
| knockout.9156 (frame 9156) | 0 of 153600 (0.00%) | - |
| knockout.9157 (frame 9157) | 3 of 153600 (0.00%) | 469,149-470,150 |
| knockout.9158 (frame 9158) | 3 of 153600 (0.00%) | 469,149-470,150 |
| knockout.9160 (frame 9160) | 87 of 153600 (0.06%) | 449,138-471,151 |
| knockout.9162 (frame 9162) | 50025 of 153600 (32.57%) | 0,0-639,239 |
| knockout.9165 (frame 9165) | 43298 of 153600 (28.19%) | 0,0-639,239 |
| kick_4.a45 (frame 9355) | 4732 of 153600 (3.08%) | 0,133-639,239 |
| kick_4.b45 (frame 9455) | 3381 of 153600 (2.20%) | 0,81-639,239 |
| kick_5.a45 (frame 9615) | 2607 of 153600 (1.70%) | 0,43-531,239 |
| kick_5.b45 (frame 9715) | 4737 of 153600 (3.08%) | 0,134-639,239 |
| kick_6.a45 (frame 9875) | 4849 of 153600 (3.16%) | 0,131-639,239 |
| kick_6.b45 (frame 9975) | 3142 of 153600 (2.05%) | 0,43-558,239 |
| kick_7.a45 (frame 10135) | 58122 of 153600 (37.84%) | 0,0-639,239 |
| kick_7.b45 (frame 10235) | 53735 of 153600 (34.98%) | 0,0-639,239 |
| last (frame 10329) | 45810 of 153600 (29.82%) | 0,0-639,239 |
| knockout.live (frame 10329) | 45810 of 153600 (29.82%) | 0,0-639,239 |

A stub counts only if the game kept running (both runs reached the script's end) and the lost effect is the function's own output (.claude/rules/naming-runtime-evidence.md). Pixels that differ beyond the function's own drawing point at changed game state. A call already in progress when the stub went in finishes normally (a leak breakpoint catches only code reaching the word after the stub).

## func_8006786C

Status: **ok**. script reached end_frame 10330 past requested frames 9150,9152,9154,9155,9156,9157,9158,9160,9162,9165 (fallback).

# Stub-and-observe: func_8006786C against baseline

- scenario: versus-2p-kimigaura-kick-2p
- baseline: `docs\naming\runtime\versus-2p-kimigaura-kick-particle-stub-2\baseline\capture.manifest.json` (SHA1 924cc770b7e6f019f645f49aa88161904bde81f8), outcome ok, script ended True, q124_complete True
- stubbed: `docs\naming\runtime\versus-2p-kimigaura-kick-particle-stub-2\func_8006786C\capture.manifest.json` (SHA1 3b0ef2fc22793ad804564163f6593100a9886286), outcome ok, script ended True, q124_complete True
- stubs: `func_8006786C` (0x8006786C) from phase fight (frame 8230), returns 0, intact at the end, called while stubbed in kick2p_3
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
| kick2p_0.45 (frame 8555) | 0 of 153600 (0.00%) | - |
| kick2p_1.45 (frame 8755) | 0 of 153600 (0.00%) | - |
| kick2p_2.45 (frame 8955) | 0 of 153600 (0.00%) | - |
| knockout.9150 (frame 9150) | 0 of 153600 (0.00%) | - |
| knockout.9152 (frame 9152) | 0 of 153600 (0.00%) | - |
| knockout.9154 (frame 9154) | 0 of 153600 (0.00%) | - |
| kick2p_3.45 (frame 9155) | 0 of 153600 (0.00%) | - |
| knockout.9155 (frame 9155) | 0 of 153600 (0.00%) | - |
| knockout.9156 (frame 9156) | 0 of 153600 (0.00%) | - |
| knockout.9157 (frame 9157) | 0 of 153600 (0.00%) | - |
| knockout.9158 (frame 9158) | 0 of 153600 (0.00%) | - |
| knockout.9160 (frame 9160) | 0 of 153600 (0.00%) | - |
| knockout.9162 (frame 9162) | 50113 of 153600 (32.63%) | 0,0-639,239 |
| knockout.9165 (frame 9165) | 43347 of 153600 (28.22%) | 0,0-639,239 |
| kick_4.a45 (frame 9355) | 4732 of 153600 (3.08%) | 0,133-639,239 |
| kick_4.b45 (frame 9455) | 3381 of 153600 (2.20%) | 0,81-639,239 |
| kick_5.a45 (frame 9615) | 2607 of 153600 (1.70%) | 0,43-531,239 |
| kick_5.b45 (frame 9715) | 27134 of 153600 (17.67%) | 0,0-639,239 |
| kick_6.a45 (frame 9875) | 4893 of 153600 (3.19%) | 0,131-639,239 |
| kick_6.b45 (frame 9975) | 3201 of 153600 (2.08%) | 0,43-558,239 |
| kick_7.a45 (frame 10135) | 58104 of 153600 (37.83%) | 0,0-639,239 |
| kick_7.b45 (frame 10235) | 53704 of 153600 (34.96%) | 0,0-639,239 |
| last (frame 10329) | 45792 of 153600 (29.81%) | 0,0-639,239 |
| knockout.live (frame 10329) | 45792 of 153600 (29.81%) | 0,0-639,239 |

A stub counts only if the game kept running (both runs reached the script's end) and the lost effect is the function's own output (.claude/rules/naming-runtime-evidence.md). Pixels that differ beyond the function's own drawing point at changed game state. A call already in progress when the stub went in finishes normally (a leak breakpoint catches only code reaching the word after the stub).
