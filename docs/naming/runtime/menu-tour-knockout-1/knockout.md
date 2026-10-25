# Knockout observations

Liveness uses the script end_frame fallback (no RAM counter established). No names or roles are inferred.

## menu_DrawToggleChoices

Status: **ok**. script reached end_frame 9000 past requested frames 6580,6700,6880,7150,7500 (fallback).

# Stub-and-observe: menu_DrawToggleChoices against baseline

- scenario: menu-tour-stub
- baseline: `docs\naming\runtime\menu-tour-knockout-1\baseline\capture.manifest.json` (SHA1 300fd8c5cd131ce7c110447893c615f32c92f8f4), outcome ok, script ended True, q124_complete True
- stubbed: `docs\naming\runtime\menu-tour-knockout-1\menu_DrawToggleChoices\capture.manifest.json` (SHA1 e0b8c63cf55577a79f4db5aa203020c1605f3ba8), outcome ok, script ended True, q124_complete True
- stubs: `menu_DrawToggleChoices` (0x8006B120) from phase t_option (frame 6460), returns with v0 unchanged, intact at the end, called while stubbed in t_option, option_menu, t_key, t_key_exit, option_menu_2, t_card, t_card_exit, option_toggles, t_top
- same inputs and script

| screenshot | changed pixels | bounding box (x0,y0-x1,y1) |
|---|---|---|
| boot_intro.last (frame 5489) | 0 of 76800 (0.00%) | - |
| top_menu.first (frame 6050) | 0 of 153600 (0.00%) | - |
| top_menu.entry1 (frame 6070) | 0 of 153600 (0.00%) | - |
| top_menu.entry2 (frame 6130) | 0 of 153600 (0.00%) | - |
| top_menu.entry3 (frame 6190) | 0 of 153600 (0.00%) | - |
| top_menu.entry4 (frame 6250) | 0 of 153600 (0.00%) | - |
| top_menu.entry5 (frame 6310) | 0 of 153600 (0.00%) | - |
| top_menu.entry6 (frame 6370) | 0 of 153600 (0.00%) | - |
| top_menu.entry7 (frame 6430) | 0 of 153600 (0.00%) | - |
| top_menu.last (frame 6459) | 0 of 153600 (0.00%) | - |
| option_menu.first (frame 6560) | 9519 of 153600 (6.20%) | 107,22-489,212 |
| knockout.6580 (frame 6580) | 9519 of 153600 (6.20%) | 107,22-489,212 |
| option_menu.last (frame 6599) | 9519 of 153600 (6.20%) | 107,22-489,212 |
| t_key.6620 (frame 6620) | 0 of 153600 (0.00%) | - |
| key_config.first (frame 6640) | 0 of 153600 (0.00%) | - |
| key_config.6700 (frame 6700) | 0 of 153600 (0.00%) | - |
| knockout.6700 (frame 6700) | 0 of 153600 (0.00%) | - |
| key_config.last (frame 6749) | 0 of 153600 (0.00%) | - |
| option_menu_2.first (frame 6800) | 9519 of 153600 (6.20%) | 107,22-489,212 |
| option_menu_2.6880 (frame 6880) | 9519 of 153600 (6.20%) | 107,22-489,212 |
| knockout.6880 (frame 6880) | 9519 of 153600 (6.20%) | 107,22-489,212 |
| option_menu_2.last (frame 6959) | 9519 of 153600 (6.20%) | 107,22-489,212 |
| memory_card.first (frame 7000) | 0 of 153600 (0.00%) | - |
| memory_card.7150 (frame 7150) | 0 of 153600 (0.00%) | - |
| knockout.7150 (frame 7150) | 0 of 153600 (0.00%) | - |
| memory_card.last (frame 7299) | 0 of 153600 (0.00%) | - |
| option_toggles.first (frame 7350) | 9519 of 153600 (6.20%) | 107,22-489,212 |
| option_toggles.7500 (frame 7500) | 9519 of 153600 (6.20%) | 107,22-489,212 |
| knockout.7500 (frame 7500) | 9519 of 153600 (6.20%) | 107,22-489,212 |
| option_toggles.music (frame 7540) | 9519 of 153600 (6.20%) | 107,22-489,212 |
| option_toggles.difficulty (frame 7690) | 9519 of 153600 (6.20%) | 107,22-489,212 |
| option_toggles.monitor (frame 7840) | 9360 of 153600 (6.09%) | 107,22-489,212 |
| option_toggles.last (frame 8149) | 9346 of 153600 (6.08%) | 107,22-489,212 |
| top_menu_2.first (frame 8350) | 0 of 153600 (0.00%) | - |
| top_menu_2.last (frame 8999) | 0 of 153600 (0.00%) | - |
| knockout.live (frame 8999) | 0 of 153600 (0.00%) | - |

A stub counts only if the game kept running (both runs reached the script's end) and the lost effect is the function's own output (.claude/rules/naming-runtime-evidence.md). Pixels that differ beyond the function's own drawing point at changed game state. A call already in progress when the stub went in finishes normally (a leak breakpoint catches only code reaching the word after the stub).

## menu_HandleToggleChoices

Status: **ok**. script reached end_frame 9000 past requested frames 6580,6700,6880,7150,7500 (fallback).

# Stub-and-observe: menu_HandleToggleChoices against baseline

- scenario: menu-tour-stub
- baseline: `docs\naming\runtime\menu-tour-knockout-1\baseline\capture.manifest.json` (SHA1 300fd8c5cd131ce7c110447893c615f32c92f8f4), outcome ok, script ended True, q124_complete True
- stubbed: `docs\naming\runtime\menu-tour-knockout-1\menu_HandleToggleChoices\capture.manifest.json` (SHA1 cdce2d9a8eb2019d5e414d6436055eed3427529c), outcome ok, script ended True, q124_complete True
- stubs: `menu_HandleToggleChoices` (0x8006B578) from phase t_option (frame 6460), returns 0, intact at the end, called while stubbed in t_option, option_menu, t_key, key_config, t_key_exit, option_menu_2, t_card, memory_card, t_card_exit, option_toggles, t_top, top_menu_2
- same inputs and script

| screenshot | changed pixels | bounding box (x0,y0-x1,y1) |
|---|---|---|
| boot_intro.last (frame 5489) | 0 of 76800 (0.00%) | - |
| top_menu.first (frame 6050) | 0 of 153600 (0.00%) | - |
| top_menu.entry1 (frame 6070) | 0 of 153600 (0.00%) | - |
| top_menu.entry2 (frame 6130) | 0 of 153600 (0.00%) | - |
| top_menu.entry3 (frame 6190) | 0 of 153600 (0.00%) | - |
| top_menu.entry4 (frame 6250) | 0 of 153600 (0.00%) | - |
| top_menu.entry5 (frame 6310) | 0 of 153600 (0.00%) | - |
| top_menu.entry6 (frame 6370) | 0 of 153600 (0.00%) | - |
| top_menu.entry7 (frame 6430) | 0 of 153600 (0.00%) | - |
| top_menu.last (frame 6459) | 0 of 153600 (0.00%) | - |
| option_menu.first (frame 6560) | 0 of 153600 (0.00%) | - |
| knockout.6580 (frame 6580) | 0 of 153600 (0.00%) | - |
| option_menu.last (frame 6599) | 0 of 153600 (0.00%) | - |
| t_key.6620 (frame 6620) | 72029 of 153600 (46.89%) | 63,22-576,223 |
| key_config.first (frame 6640) | 72029 of 153600 (46.89%) | 63,22-576,223 |
| key_config.6700 (frame 6700) | 72029 of 153600 (46.89%) | 63,22-576,223 |
| knockout.6700 (frame 6700) | 72029 of 153600 (46.89%) | 63,22-576,223 |
| key_config.last (frame 6749) | 72029 of 153600 (46.89%) | 63,22-576,223 |
| option_menu_2.first (frame 6800) | 0 of 153600 (0.00%) | - |
| option_menu_2.6880 (frame 6880) | 0 of 153600 (0.00%) | - |
| knockout.6880 (frame 6880) | 0 of 153600 (0.00%) | - |
| option_menu_2.last (frame 6959) | 2586 of 153600 (1.68%) | 107,143-313,180 |
| memory_card.first (frame 7000) | 11317 of 153600 (7.37%) | 107,22-489,212 |
| memory_card.7150 (frame 7150) | 11317 of 153600 (7.37%) | 107,22-489,212 |
| knockout.7150 (frame 7150) | 11317 of 153600 (7.37%) | 107,22-489,212 |
| memory_card.last (frame 7299) | 11317 of 153600 (7.37%) | 107,22-489,212 |
| option_toggles.first (frame 7350) | 2586 of 153600 (1.68%) | 107,143-313,180 |
| option_toggles.7500 (frame 7500) | 2350 of 153600 (1.53%) | 107,119-478,180 |
| knockout.7500 (frame 7500) | 2350 of 153600 (1.53%) | 107,119-478,180 |
| option_toggles.music (frame 7540) | 2350 of 153600 (1.53%) | 107,119-478,180 |
| option_toggles.difficulty (frame 7690) | 3198 of 153600 (2.08%) | 107,95-489,180 |
| option_toggles.monitor (frame 7840) | 119838 of 153600 (78.02%) | 0,0-639,239 |
| option_toggles.last (frame 8149) | 119838 of 153600 (78.02%) | 0,0-639,239 |
| top_menu_2.first (frame 8350) | 125994 of 153600 (82.03%) | 0,0-639,239 |
| top_menu_2.last (frame 8999) | 125994 of 153600 (82.03%) | 0,0-639,239 |
| knockout.live (frame 8999) | 125994 of 153600 (82.03%) | 0,0-639,239 |

A stub counts only if the game kept running (both runs reached the script's end) and the lost effect is the function's own output (.claude/rules/naming-runtime-evidence.md). Pixels that differ beyond the function's own drawing point at changed game state. A call already in progress when the stub went in finishes normally (a leak breakpoint catches only code reaching the word after the stub).

## menu_UpdateToggleChoices

Status: **not_alive**. outcome not ok; script not ended; stub not applied whole and intact; run exit 1; no later liveness screenshot/end_frame; NOT COMPARABLE (see knockout.md).

# Stub-and-observe: menu_UpdateToggleChoices against baseline

- scenario: menu-tour-stub
- baseline: `docs\naming\runtime\menu-tour-knockout-1\baseline\capture.manifest.json` (SHA1 300fd8c5cd131ce7c110447893c615f32c92f8f4), outcome ok, script ended True, q124_complete True
- stubbed: `docs\naming\runtime\menu-tour-knockout-1\menu_UpdateToggleChoices\capture.manifest.json` (SHA1 b143d572976113869232f07584bd353efea48af6), outcome error: ConnectionResetError: [WinError 10054] An existing connection was forcibly closed by the remote host, script ended False, q124_complete False
- stubs: `menu_UpdateToggleChoices` (0x8006B898) from phase t_option (frame 6460), returns 0, not read: [WinError 10054] An existing connection was forcibly closed by the remote host at the end, called while stubbed in t_option, option_menu
- NOT COMPARABLE: stubbed is not q124_complete: ['a run that reached its end', 'stub menu_UpdateToggleChoices applied whole and still in place at the end', 'a script that ran as written']


Diff withheld as evidence.

## menu_HandleReturningChoices

Status: **ok**. script reached end_frame 9000 past requested frames 6580,6700,6880,7150,7500 (fallback).

# Stub-and-observe: menu_HandleReturningChoices against baseline

- scenario: menu-tour-stub
- baseline: `docs\naming\runtime\menu-tour-knockout-1\baseline\capture.manifest.json` (SHA1 300fd8c5cd131ce7c110447893c615f32c92f8f4), outcome ok, script ended True, q124_complete True
- stubbed: `docs\naming\runtime\menu-tour-knockout-1\menu_HandleReturningChoices\capture.manifest.json` (SHA1 29491836188619230a1c4b1155e18ddedfc10fe5), outcome ok, script ended True, q124_complete True
- stubs: `menu_HandleReturningChoices` (0x8006B92C) from phase t_option (frame 6460), returns 0, intact at the end, called while stubbed in t_card, memory_card, t_card_exit, option_toggles, t_top, top_menu_2
- same inputs and script

| screenshot | changed pixels | bounding box (x0,y0-x1,y1) |
|---|---|---|
| boot_intro.last (frame 5489) | 0 of 76800 (0.00%) | - |
| top_menu.first (frame 6050) | 0 of 153600 (0.00%) | - |
| top_menu.entry1 (frame 6070) | 0 of 153600 (0.00%) | - |
| top_menu.entry2 (frame 6130) | 0 of 153600 (0.00%) | - |
| top_menu.entry3 (frame 6190) | 0 of 153600 (0.00%) | - |
| top_menu.entry4 (frame 6250) | 0 of 153600 (0.00%) | - |
| top_menu.entry5 (frame 6310) | 0 of 153600 (0.00%) | - |
| top_menu.entry6 (frame 6370) | 0 of 153600 (0.00%) | - |
| top_menu.entry7 (frame 6430) | 0 of 153600 (0.00%) | - |
| top_menu.last (frame 6459) | 0 of 153600 (0.00%) | - |
| option_menu.first (frame 6560) | 0 of 153600 (0.00%) | - |
| knockout.6580 (frame 6580) | 0 of 153600 (0.00%) | - |
| option_menu.last (frame 6599) | 0 of 153600 (0.00%) | - |
| t_key.6620 (frame 6620) | 0 of 153600 (0.00%) | - |
| key_config.first (frame 6640) | 0 of 153600 (0.00%) | - |
| key_config.6700 (frame 6700) | 0 of 153600 (0.00%) | - |
| knockout.6700 (frame 6700) | 0 of 153600 (0.00%) | - |
| key_config.last (frame 6749) | 0 of 153600 (0.00%) | - |
| option_menu_2.first (frame 6800) | 0 of 153600 (0.00%) | - |
| option_menu_2.6880 (frame 6880) | 0 of 153600 (0.00%) | - |
| knockout.6880 (frame 6880) | 0 of 153600 (0.00%) | - |
| option_menu_2.last (frame 6959) | 0 of 153600 (0.00%) | - |
| memory_card.first (frame 7000) | 0 of 153600 (0.00%) | - |
| memory_card.7150 (frame 7150) | 0 of 153600 (0.00%) | - |
| knockout.7150 (frame 7150) | 0 of 153600 (0.00%) | - |
| memory_card.last (frame 7299) | 0 of 153600 (0.00%) | - |
| option_toggles.first (frame 7350) | 11319 of 153600 (7.37%) | 107,22-489,212 |
| option_toggles.7500 (frame 7500) | 11319 of 153600 (7.37%) | 107,22-489,212 |
| knockout.7500 (frame 7500) | 11319 of 153600 (7.37%) | 107,22-489,212 |
| option_toggles.music (frame 7540) | 11319 of 153600 (7.37%) | 107,22-489,212 |
| option_toggles.difficulty (frame 7690) | 11318 of 153600 (7.37%) | 107,22-489,212 |
| option_toggles.monitor (frame 7840) | 120955 of 153600 (78.75%) | 0,0-639,239 |
| option_toggles.last (frame 8149) | 120955 of 153600 (78.75%) | 0,0-639,239 |
| top_menu_2.first (frame 8350) | 126004 of 153600 (82.03%) | 0,0-639,239 |
| top_menu_2.last (frame 8999) | 126004 of 153600 (82.03%) | 0,0-639,239 |
| knockout.live (frame 8999) | 126004 of 153600 (82.03%) | 0,0-639,239 |

A stub counts only if the game kept running (both runs reached the script's end) and the lost effect is the function's own output (.claude/rules/naming-runtime-evidence.md). Pixels that differ beyond the function's own drawing point at changed game state. A call already in progress when the stub went in finishes normally (a leak breakpoint catches only code reaching the word after the stub).

## menu_DrawReturningChoices

Status: **ok**. script reached end_frame 9000 past requested frames 6580,6700,6880,7150,7500 (fallback).

# Stub-and-observe: menu_DrawReturningChoices against baseline

- scenario: menu-tour-stub
- baseline: `docs\naming\runtime\menu-tour-knockout-1\baseline\capture.manifest.json` (SHA1 300fd8c5cd131ce7c110447893c615f32c92f8f4), outcome ok, script ended True, q124_complete True
- stubbed: `docs\naming\runtime\menu-tour-knockout-1\menu_DrawReturningChoices\capture.manifest.json` (SHA1 602e9e0a220e702317b51354ddbbc8a2babe7bf5), outcome ok, script ended True, q124_complete True
- stubs: `menu_DrawReturningChoices` (0x8006BB68) from phase t_option (frame 6460), returns with v0 unchanged, intact at the end, called while stubbed in t_card, memory_card, t_card_exit
- same inputs and script

| screenshot | changed pixels | bounding box (x0,y0-x1,y1) |
|---|---|---|
| boot_intro.last (frame 5489) | 0 of 76800 (0.00%) | - |
| top_menu.first (frame 6050) | 0 of 153600 (0.00%) | - |
| top_menu.entry1 (frame 6070) | 0 of 153600 (0.00%) | - |
| top_menu.entry2 (frame 6130) | 0 of 153600 (0.00%) | - |
| top_menu.entry3 (frame 6190) | 0 of 153600 (0.00%) | - |
| top_menu.entry4 (frame 6250) | 0 of 153600 (0.00%) | - |
| top_menu.entry5 (frame 6310) | 0 of 153600 (0.00%) | - |
| top_menu.entry6 (frame 6370) | 0 of 153600 (0.00%) | - |
| top_menu.entry7 (frame 6430) | 0 of 153600 (0.00%) | - |
| top_menu.last (frame 6459) | 0 of 153600 (0.00%) | - |
| option_menu.first (frame 6560) | 0 of 153600 (0.00%) | - |
| knockout.6580 (frame 6580) | 0 of 153600 (0.00%) | - |
| option_menu.last (frame 6599) | 0 of 153600 (0.00%) | - |
| t_key.6620 (frame 6620) | 0 of 153600 (0.00%) | - |
| key_config.first (frame 6640) | 0 of 153600 (0.00%) | - |
| key_config.6700 (frame 6700) | 0 of 153600 (0.00%) | - |
| knockout.6700 (frame 6700) | 0 of 153600 (0.00%) | - |
| key_config.last (frame 6749) | 0 of 153600 (0.00%) | - |
| option_menu_2.first (frame 6800) | 0 of 153600 (0.00%) | - |
| option_menu_2.6880 (frame 6880) | 0 of 153600 (0.00%) | - |
| knockout.6880 (frame 6880) | 0 of 153600 (0.00%) | - |
| option_menu_2.last (frame 6959) | 0 of 153600 (0.00%) | - |
| memory_card.first (frame 7000) | 4341 of 153600 (2.83%) | 232,22-406,169 |
| memory_card.7150 (frame 7150) | 4341 of 153600 (2.83%) | 232,22-406,169 |
| knockout.7150 (frame 7150) | 4341 of 153600 (2.83%) | 232,22-406,169 |
| memory_card.last (frame 7299) | 4341 of 153600 (2.83%) | 232,22-406,169 |
| option_toggles.first (frame 7350) | 0 of 153600 (0.00%) | - |
| option_toggles.7500 (frame 7500) | 0 of 153600 (0.00%) | - |
| knockout.7500 (frame 7500) | 0 of 153600 (0.00%) | - |
| option_toggles.music (frame 7540) | 0 of 153600 (0.00%) | - |
| option_toggles.difficulty (frame 7690) | 0 of 153600 (0.00%) | - |
| option_toggles.monitor (frame 7840) | 0 of 153600 (0.00%) | - |
| option_toggles.last (frame 8149) | 0 of 153600 (0.00%) | - |
| top_menu_2.first (frame 8350) | 0 of 153600 (0.00%) | - |
| top_menu_2.last (frame 8999) | 0 of 153600 (0.00%) | - |
| knockout.live (frame 8999) | 0 of 153600 (0.00%) | - |

A stub counts only if the game kept running (both runs reached the script's end) and the lost effect is the function's own output (.claude/rules/naming-runtime-evidence.md). Pixels that differ beyond the function's own drawing point at changed game state. A call already in progress when the stub went in finishes normally (a leak breakpoint catches only code reaching the word after the stub).

## menu_UpdateReturningChoices

Status: **ok**. script reached end_frame 9000 past requested frames 6580,6700,6880,7150,7500 (fallback).

# Stub-and-observe: menu_UpdateReturningChoices against baseline

- scenario: menu-tour-stub
- baseline: `docs\naming\runtime\menu-tour-knockout-1\baseline\capture.manifest.json` (SHA1 300fd8c5cd131ce7c110447893c615f32c92f8f4), outcome ok, script ended True, q124_complete True
- stubbed: `docs\naming\runtime\menu-tour-knockout-1\menu_UpdateReturningChoices\capture.manifest.json` (SHA1 47f68772c78b75fe33ad74f0a3d8088a6f7a0a8d), outcome ok, script ended True, q124_complete True
- stubs: `menu_UpdateReturningChoices` (0x8006C168) from phase t_option (frame 6460), returns 0, intact at the end, called while stubbed in t_card, memory_card, t_card_exit, option_toggles, t_top, top_menu_2
- same inputs and script

| screenshot | changed pixels | bounding box (x0,y0-x1,y1) |
|---|---|---|
| boot_intro.last (frame 5489) | 0 of 76800 (0.00%) | - |
| top_menu.first (frame 6050) | 0 of 153600 (0.00%) | - |
| top_menu.entry1 (frame 6070) | 0 of 153600 (0.00%) | - |
| top_menu.entry2 (frame 6130) | 0 of 153600 (0.00%) | - |
| top_menu.entry3 (frame 6190) | 0 of 153600 (0.00%) | - |
| top_menu.entry4 (frame 6250) | 0 of 153600 (0.00%) | - |
| top_menu.entry5 (frame 6310) | 0 of 153600 (0.00%) | - |
| top_menu.entry6 (frame 6370) | 0 of 153600 (0.00%) | - |
| top_menu.entry7 (frame 6430) | 0 of 153600 (0.00%) | - |
| top_menu.last (frame 6459) | 0 of 153600 (0.00%) | - |
| option_menu.first (frame 6560) | 0 of 153600 (0.00%) | - |
| knockout.6580 (frame 6580) | 0 of 153600 (0.00%) | - |
| option_menu.last (frame 6599) | 0 of 153600 (0.00%) | - |
| t_key.6620 (frame 6620) | 0 of 153600 (0.00%) | - |
| key_config.first (frame 6640) | 0 of 153600 (0.00%) | - |
| key_config.6700 (frame 6700) | 0 of 153600 (0.00%) | - |
| knockout.6700 (frame 6700) | 0 of 153600 (0.00%) | - |
| key_config.last (frame 6749) | 0 of 153600 (0.00%) | - |
| option_menu_2.first (frame 6800) | 0 of 153600 (0.00%) | - |
| option_menu_2.6880 (frame 6880) | 0 of 153600 (0.00%) | - |
| knockout.6880 (frame 6880) | 0 of 153600 (0.00%) | - |
| option_menu_2.last (frame 6959) | 0 of 153600 (0.00%) | - |
| memory_card.first (frame 7000) | 121285 of 153600 (78.96%) | 0,0-639,239 |
| memory_card.7150 (frame 7150) | 121285 of 153600 (78.96%) | 0,0-639,239 |
| knockout.7150 (frame 7150) | 121285 of 153600 (78.96%) | 0,0-639,239 |
| memory_card.last (frame 7299) | 121285 of 153600 (78.96%) | 0,0-639,239 |
| option_toggles.first (frame 7350) | 121102 of 153600 (78.84%) | 0,0-639,239 |
| option_toggles.7500 (frame 7500) | 121303 of 153600 (78.97%) | 0,0-639,239 |
| knockout.7500 (frame 7500) | 121303 of 153600 (78.97%) | 0,0-639,239 |
| option_toggles.music (frame 7540) | 121321 of 153600 (78.99%) | 0,0-639,239 |
| option_toggles.difficulty (frame 7690) | 121202 of 153600 (78.91%) | 0,0-639,239 |
| option_toggles.monitor (frame 7840) | 118434 of 153600 (77.11%) | 0,0-639,239 |
| option_toggles.last (frame 8149) | 118159 of 153600 (76.93%) | 0,0-639,239 |
| top_menu_2.first (frame 8350) | 102408 of 153600 (66.67%) | 0,0-639,239 |
| top_menu_2.last (frame 8999) | 102451 of 153600 (66.70%) | 0,0-639,239 |
| knockout.live (frame 8999) | 102451 of 153600 (66.70%) | 0,0-639,239 |

A stub counts only if the game kept running (both runs reached the script's end) and the lost effect is the function's own output (.claude/rules/naming-runtime-evidence.md). Pixels that differ beyond the function's own drawing point at changed game state. A call already in progress when the stub went in finishes normally (a leak breakpoint catches only code reaching the word after the stub).

## func_8006C1FC

Status: **ok**. script reached end_frame 9000 past requested frames 6580,6700,6880,7150,7500 (fallback).

# Stub-and-observe: func_8006C1FC against baseline

- scenario: menu-tour-stub
- baseline: `docs\naming\runtime\menu-tour-knockout-1\baseline\capture.manifest.json` (SHA1 300fd8c5cd131ce7c110447893c615f32c92f8f4), outcome ok, script ended True, q124_complete True
- stubbed: `docs\naming\runtime\menu-tour-knockout-1\func_8006C1FC\capture.manifest.json` (SHA1 949be90e91ef36d9864dd0f83970e10cd3de9ed8), outcome ok, script ended True, q124_complete True
- stubs: `func_8006C1FC` (0x8006C1FC) from phase t_option (frame 6460), returns 0, intact at the end, called while stubbed in t_card, memory_card, t_card_exit, option_toggles, t_top, top_menu_2
- same inputs and script

| screenshot | changed pixels | bounding box (x0,y0-x1,y1) |
|---|---|---|
| boot_intro.last (frame 5489) | 0 of 76800 (0.00%) | - |
| top_menu.first (frame 6050) | 0 of 153600 (0.00%) | - |
| top_menu.entry1 (frame 6070) | 0 of 153600 (0.00%) | - |
| top_menu.entry2 (frame 6130) | 0 of 153600 (0.00%) | - |
| top_menu.entry3 (frame 6190) | 0 of 153600 (0.00%) | - |
| top_menu.entry4 (frame 6250) | 0 of 153600 (0.00%) | - |
| top_menu.entry5 (frame 6310) | 0 of 153600 (0.00%) | - |
| top_menu.entry6 (frame 6370) | 0 of 153600 (0.00%) | - |
| top_menu.entry7 (frame 6430) | 0 of 153600 (0.00%) | - |
| top_menu.last (frame 6459) | 0 of 153600 (0.00%) | - |
| option_menu.first (frame 6560) | 0 of 153600 (0.00%) | - |
| knockout.6580 (frame 6580) | 0 of 153600 (0.00%) | - |
| option_menu.last (frame 6599) | 0 of 153600 (0.00%) | - |
| t_key.6620 (frame 6620) | 0 of 153600 (0.00%) | - |
| key_config.first (frame 6640) | 0 of 153600 (0.00%) | - |
| key_config.6700 (frame 6700) | 0 of 153600 (0.00%) | - |
| knockout.6700 (frame 6700) | 0 of 153600 (0.00%) | - |
| key_config.last (frame 6749) | 0 of 153600 (0.00%) | - |
| option_menu_2.first (frame 6800) | 0 of 153600 (0.00%) | - |
| option_menu_2.6880 (frame 6880) | 0 of 153600 (0.00%) | - |
| knockout.6880 (frame 6880) | 0 of 153600 (0.00%) | - |
| option_menu_2.last (frame 6959) | 0 of 153600 (0.00%) | - |
| memory_card.first (frame 7000) | 121285 of 153600 (78.96%) | 0,0-639,239 |
| memory_card.7150 (frame 7150) | 121285 of 153600 (78.96%) | 0,0-639,239 |
| knockout.7150 (frame 7150) | 121285 of 153600 (78.96%) | 0,0-639,239 |
| memory_card.last (frame 7299) | 121285 of 153600 (78.96%) | 0,0-639,239 |
| option_toggles.first (frame 7350) | 121102 of 153600 (78.84%) | 0,0-639,239 |
| option_toggles.7500 (frame 7500) | 121303 of 153600 (78.97%) | 0,0-639,239 |
| knockout.7500 (frame 7500) | 121303 of 153600 (78.97%) | 0,0-639,239 |
| option_toggles.music (frame 7540) | 121321 of 153600 (78.99%) | 0,0-639,239 |
| option_toggles.difficulty (frame 7690) | 121202 of 153600 (78.91%) | 0,0-639,239 |
| option_toggles.monitor (frame 7840) | 118434 of 153600 (77.11%) | 0,0-639,239 |
| option_toggles.last (frame 8149) | 118159 of 153600 (76.93%) | 0,0-639,239 |
| top_menu_2.first (frame 8350) | 102408 of 153600 (66.67%) | 0,0-639,239 |
| top_menu_2.last (frame 8999) | 102451 of 153600 (66.70%) | 0,0-639,239 |
| knockout.live (frame 8999) | 102451 of 153600 (66.70%) | 0,0-639,239 |

A stub counts only if the game kept running (both runs reached the script's end) and the lost effect is the function's own output (.claude/rules/naming-runtime-evidence.md). Pixels that differ beyond the function's own drawing point at changed game state. A call already in progress when the stub went in finishes normally (a leak breakpoint catches only code reaching the word after the stub).

## menu_DrawRowBitSheetsTilesAndGradientQuads

Status: **ok**. script reached end_frame 9000 past requested frames 6580,6700,6880,7150,7500 (fallback).

# Stub-and-observe: menu_DrawRowBitSheetsTilesAndGradientQuads against baseline

- scenario: menu-tour-stub
- baseline: `docs\naming\runtime\menu-tour-knockout-1\baseline\capture.manifest.json` (SHA1 300fd8c5cd131ce7c110447893c615f32c92f8f4), outcome ok, script ended True, q124_complete True
- stubbed: `docs\naming\runtime\menu-tour-knockout-1\menu_DrawRowBitSheetsTilesAndGradientQuads\capture.manifest.json` (SHA1 531d2b359afaf85c5062883ef58b2c806980ac71), outcome ok, script ended True, q124_complete True
- stubs: `menu_DrawRowBitSheetsTilesAndGradientQuads` (0x8006C21C) from phase t_option (frame 6460), returns with v0 unchanged, intact at the end, called while stubbed in t_key, key_config, t_key_exit
- same inputs and script

| screenshot | changed pixels | bounding box (x0,y0-x1,y1) |
|---|---|---|
| boot_intro.last (frame 5489) | 0 of 76800 (0.00%) | - |
| top_menu.first (frame 6050) | 0 of 153600 (0.00%) | - |
| top_menu.entry1 (frame 6070) | 0 of 153600 (0.00%) | - |
| top_menu.entry2 (frame 6130) | 0 of 153600 (0.00%) | - |
| top_menu.entry3 (frame 6190) | 0 of 153600 (0.00%) | - |
| top_menu.entry4 (frame 6250) | 0 of 153600 (0.00%) | - |
| top_menu.entry5 (frame 6310) | 0 of 153600 (0.00%) | - |
| top_menu.entry6 (frame 6370) | 0 of 153600 (0.00%) | - |
| top_menu.entry7 (frame 6430) | 0 of 153600 (0.00%) | - |
| top_menu.last (frame 6459) | 0 of 153600 (0.00%) | - |
| option_menu.first (frame 6560) | 0 of 153600 (0.00%) | - |
| knockout.6580 (frame 6580) | 0 of 153600 (0.00%) | - |
| option_menu.last (frame 6599) | 0 of 153600 (0.00%) | - |
| t_key.6620 (frame 6620) | 48674 of 153600 (31.69%) | 88,22-551,218 |
| key_config.first (frame 6640) | 48674 of 153600 (31.69%) | 88,22-551,218 |
| key_config.6700 (frame 6700) | 48674 of 153600 (31.69%) | 88,22-551,218 |
| knockout.6700 (frame 6700) | 48674 of 153600 (31.69%) | 88,22-551,218 |
| key_config.last (frame 6749) | 48674 of 153600 (31.69%) | 88,22-551,218 |
| option_menu_2.first (frame 6800) | 0 of 153600 (0.00%) | - |
| option_menu_2.6880 (frame 6880) | 0 of 153600 (0.00%) | - |
| knockout.6880 (frame 6880) | 0 of 153600 (0.00%) | - |
| option_menu_2.last (frame 6959) | 0 of 153600 (0.00%) | - |
| memory_card.first (frame 7000) | 0 of 153600 (0.00%) | - |
| memory_card.7150 (frame 7150) | 0 of 153600 (0.00%) | - |
| knockout.7150 (frame 7150) | 0 of 153600 (0.00%) | - |
| memory_card.last (frame 7299) | 0 of 153600 (0.00%) | - |
| option_toggles.first (frame 7350) | 0 of 153600 (0.00%) | - |
| option_toggles.7500 (frame 7500) | 0 of 153600 (0.00%) | - |
| knockout.7500 (frame 7500) | 0 of 153600 (0.00%) | - |
| option_toggles.music (frame 7540) | 0 of 153600 (0.00%) | - |
| option_toggles.difficulty (frame 7690) | 0 of 153600 (0.00%) | - |
| option_toggles.monitor (frame 7840) | 0 of 153600 (0.00%) | - |
| option_toggles.last (frame 8149) | 0 of 153600 (0.00%) | - |
| top_menu_2.first (frame 8350) | 0 of 153600 (0.00%) | - |
| top_menu_2.last (frame 8999) | 0 of 153600 (0.00%) | - |
| knockout.live (frame 8999) | 0 of 153600 (0.00%) | - |

A stub counts only if the game kept running (both runs reached the script's end) and the lost effect is the function's own output (.claude/rules/naming-runtime-evidence.md). Pixels that differ beyond the function's own drawing point at changed game state. A call already in progress when the stub went in finishes normally (a leak breakpoint catches only code reaching the word after the stub).

## menu_DrawRowBitsGetEmptyRowSides

Status: **ok**. script reached end_frame 9000 past requested frames 6580,6700,6880,7150,7500 (fallback).

# Stub-and-observe: menu_DrawRowBitsGetEmptyRowSides against baseline

- scenario: menu-tour-stub
- baseline: `docs\naming\runtime\menu-tour-knockout-1\baseline\capture.manifest.json` (SHA1 300fd8c5cd131ce7c110447893c615f32c92f8f4), outcome ok, script ended True, q124_complete True
- stubbed: `docs\naming\runtime\menu-tour-knockout-1\menu_DrawRowBitsGetEmptyRowSides\capture.manifest.json` (SHA1 7586cbdb9546f5387b387391483e16ca80ac96cd), outcome ok, script ended True, q124_complete True
- stubs: `menu_DrawRowBitsGetEmptyRowSides` (0x8006CFBC) from phase t_option (frame 6460), returns 0, intact at the end, called while stubbed in t_key, key_config, t_key_exit
- same inputs and script

| screenshot | changed pixels | bounding box (x0,y0-x1,y1) |
|---|---|---|
| boot_intro.last (frame 5489) | 0 of 76800 (0.00%) | - |
| top_menu.first (frame 6050) | 0 of 153600 (0.00%) | - |
| top_menu.entry1 (frame 6070) | 0 of 153600 (0.00%) | - |
| top_menu.entry2 (frame 6130) | 0 of 153600 (0.00%) | - |
| top_menu.entry3 (frame 6190) | 0 of 153600 (0.00%) | - |
| top_menu.entry4 (frame 6250) | 0 of 153600 (0.00%) | - |
| top_menu.entry5 (frame 6310) | 0 of 153600 (0.00%) | - |
| top_menu.entry6 (frame 6370) | 0 of 153600 (0.00%) | - |
| top_menu.entry7 (frame 6430) | 0 of 153600 (0.00%) | - |
| top_menu.last (frame 6459) | 0 of 153600 (0.00%) | - |
| option_menu.first (frame 6560) | 0 of 153600 (0.00%) | - |
| knockout.6580 (frame 6580) | 0 of 153600 (0.00%) | - |
| option_menu.last (frame 6599) | 0 of 153600 (0.00%) | - |
| t_key.6620 (frame 6620) | 6055 of 153600 (3.94%) | 90,35-549,217 |
| key_config.first (frame 6640) | 6063 of 153600 (3.95%) | 90,35-549,217 |
| key_config.6700 (frame 6700) | 6063 of 153600 (3.95%) | 90,35-549,217 |
| knockout.6700 (frame 6700) | 6063 of 153600 (3.95%) | 90,35-549,217 |
| key_config.last (frame 6749) | 6055 of 153600 (3.94%) | 90,35-549,217 |
| option_menu_2.first (frame 6800) | 0 of 153600 (0.00%) | - |
| option_menu_2.6880 (frame 6880) | 0 of 153600 (0.00%) | - |
| knockout.6880 (frame 6880) | 0 of 153600 (0.00%) | - |
| option_menu_2.last (frame 6959) | 0 of 153600 (0.00%) | - |
| memory_card.first (frame 7000) | 0 of 153600 (0.00%) | - |
| memory_card.7150 (frame 7150) | 0 of 153600 (0.00%) | - |
| knockout.7150 (frame 7150) | 0 of 153600 (0.00%) | - |
| memory_card.last (frame 7299) | 0 of 153600 (0.00%) | - |
| option_toggles.first (frame 7350) | 0 of 153600 (0.00%) | - |
| option_toggles.7500 (frame 7500) | 0 of 153600 (0.00%) | - |
| knockout.7500 (frame 7500) | 0 of 153600 (0.00%) | - |
| option_toggles.music (frame 7540) | 0 of 153600 (0.00%) | - |
| option_toggles.difficulty (frame 7690) | 0 of 153600 (0.00%) | - |
| option_toggles.monitor (frame 7840) | 0 of 153600 (0.00%) | - |
| option_toggles.last (frame 8149) | 0 of 153600 (0.00%) | - |
| top_menu_2.first (frame 8350) | 0 of 153600 (0.00%) | - |
| top_menu_2.last (frame 8999) | 0 of 153600 (0.00%) | - |
| knockout.live (frame 8999) | 0 of 153600 (0.00%) | - |

A stub counts only if the game kept running (both runs reached the script's end) and the lost effect is the function's own output (.claude/rules/naming-runtime-evidence.md). Pixels that differ beyond the function's own drawing point at changed game state. A call already in progress when the stub went in finishes normally (a leak breakpoint catches only code reaching the word after the stub).

## menu_RunFlowStateWithRowBitEdit

Status: **ok**. script reached end_frame 9000 past requested frames 6580,6700,6880,7150,7500 (fallback).

# Stub-and-observe: menu_RunFlowStateWithRowBitEdit against baseline

- scenario: menu-tour-stub
- baseline: `docs\naming\runtime\menu-tour-knockout-1\baseline\capture.manifest.json` (SHA1 300fd8c5cd131ce7c110447893c615f32c92f8f4), outcome ok, script ended True, q124_complete True
- stubbed: `docs\naming\runtime\menu-tour-knockout-1\menu_RunFlowStateWithRowBitEdit\capture.manifest.json` (SHA1 443accf6cb7592698af0ac7aadec3ca5cb1bf104), outcome ok, script ended True, q124_complete True
- stubs: `menu_RunFlowStateWithRowBitEdit` (0x80077B30) from phase t_option (frame 6460), returns 0, intact at the end, called while stubbed in t_option, option_menu, t_key, key_config, t_key_exit, option_menu_2, t_card, memory_card, t_card_exit, option_toggles, t_top, top_menu_2
- same inputs and script

| screenshot | changed pixels | bounding box (x0,y0-x1,y1) |
|---|---|---|
| boot_intro.last (frame 5489) | 0 of 76800 (0.00%) | - |
| top_menu.first (frame 6050) | 0 of 153600 (0.00%) | - |
| top_menu.entry1 (frame 6070) | 0 of 153600 (0.00%) | - |
| top_menu.entry2 (frame 6130) | 0 of 153600 (0.00%) | - |
| top_menu.entry3 (frame 6190) | 0 of 153600 (0.00%) | - |
| top_menu.entry4 (frame 6250) | 0 of 153600 (0.00%) | - |
| top_menu.entry5 (frame 6310) | 0 of 153600 (0.00%) | - |
| top_menu.entry6 (frame 6370) | 0 of 153600 (0.00%) | - |
| top_menu.entry7 (frame 6430) | 0 of 153600 (0.00%) | - |
| top_menu.last (frame 6459) | 0 of 153600 (0.00%) | - |
| option_menu.first (frame 6560) | 120960 of 153600 (78.75%) | 0,0-639,239 |
| knockout.6580 (frame 6580) | 121002 of 153600 (78.78%) | 0,0-639,239 |
| option_menu.last (frame 6599) | 120960 of 153600 (78.75%) | 0,0-639,239 |
| t_key.6620 (frame 6620) | 116964 of 153600 (76.15%) | 0,0-639,239 |
| key_config.first (frame 6640) | 116972 of 153600 (76.15%) | 0,0-639,239 |
| key_config.6700 (frame 6700) | 116972 of 153600 (76.15%) | 0,0-639,239 |
| knockout.6700 (frame 6700) | 116972 of 153600 (76.15%) | 0,0-639,239 |
| key_config.last (frame 6749) | 116964 of 153600 (76.15%) | 0,0-639,239 |
| option_menu_2.first (frame 6800) | 121002 of 153600 (78.78%) | 0,0-639,239 |
| option_menu_2.6880 (frame 6880) | 120960 of 153600 (78.75%) | 0,0-639,239 |
| knockout.6880 (frame 6880) | 120960 of 153600 (78.75%) | 0,0-639,239 |
| option_menu_2.last (frame 6959) | 121102 of 153600 (78.84%) | 0,0-639,239 |
| memory_card.first (frame 7000) | 121285 of 153600 (78.96%) | 0,0-639,239 |
| memory_card.7150 (frame 7150) | 121285 of 153600 (78.96%) | 0,0-639,239 |
| knockout.7150 (frame 7150) | 121285 of 153600 (78.96%) | 0,0-639,239 |
| memory_card.last (frame 7299) | 121285 of 153600 (78.96%) | 0,0-639,239 |
| option_toggles.first (frame 7350) | 121102 of 153600 (78.84%) | 0,0-639,239 |
| option_toggles.7500 (frame 7500) | 121303 of 153600 (78.97%) | 0,0-639,239 |
| knockout.7500 (frame 7500) | 121303 of 153600 (78.97%) | 0,0-639,239 |
| option_toggles.music (frame 7540) | 121321 of 153600 (78.99%) | 0,0-639,239 |
| option_toggles.difficulty (frame 7690) | 121202 of 153600 (78.91%) | 0,0-639,239 |
| option_toggles.monitor (frame 7840) | 118434 of 153600 (77.11%) | 0,0-639,239 |
| option_toggles.last (frame 8149) | 118159 of 153600 (76.93%) | 0,0-639,239 |
| top_menu_2.first (frame 8350) | 102408 of 153600 (66.67%) | 0,0-639,239 |
| top_menu_2.last (frame 8999) | 102451 of 153600 (66.70%) | 0,0-639,239 |
| knockout.live (frame 8999) | 102451 of 153600 (66.70%) | 0,0-639,239 |

A stub counts only if the game kept running (both runs reached the script's end) and the lost effect is the function's own output (.claude/rules/naming-runtime-evidence.md). Pixels that differ beyond the function's own drawing point at changed game state. A call already in progress when the stub went in finishes normally (a leak breakpoint catches only code reaching the word after the stub).
