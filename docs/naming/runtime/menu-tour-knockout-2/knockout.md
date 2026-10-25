# Knockout observations

Liveness uses the script end_frame fallback (no RAM counter established). No names or roles are inferred.

## menu_UpdateToggleChoices

Status: **ok**. script reached end_frame 9000 past requested frames 6580,6700,6880,7150,7500 (fallback).

# Stub-and-observe: menu_UpdateToggleChoices against baseline

- scenario: menu-tour-stub
- baseline: `docs\naming\runtime\menu-tour-knockout-2\baseline\capture.manifest.json` (SHA1 b88382c00747a69871c8ac53060f2667be9a8a89), outcome ok, script ended True, q124_complete True
- stubbed: `docs\naming\runtime\menu-tour-knockout-2\menu_UpdateToggleChoices\capture.manifest.json` (SHA1 ab328afa562648ae1c84b8385a6146f6500dc544), outcome ok, script ended True, q124_complete True
- stubs: `menu_UpdateToggleChoices` (0x8006B898) from phase t_option (frame 6460), returns 0, intact at the end, called while stubbed in t_option, option_menu, t_key, key_config, t_key_exit, option_menu_2, t_card, memory_card, t_card_exit, option_toggles, t_top, top_menu_2
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
