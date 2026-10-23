# Stub-and-observe: func_8006BB68 against baseline

- scenario: menu-tour script (Option Menu, Key Configuration, Memory Card), stub from the named phase
- baseline: `..\baseline\capture.manifest.json` (SHA1 8b72c65575cad867d6d67a84e552d885e47d3fdd), outcome ok, script ended True, q124_complete True
- stubbed: `capture.manifest.json` (SHA1 5b93b9691406b5f34ad12b60de62b8551135bf11), outcome ok, script ended True, q124_complete True
- stubs: `func_8006BB68` (0x8006BB68) from phase t_card (frame 6960), returns 0, intact at the end, called while stubbed in t_card, memory_card, t_card_exit
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
| option_menu.last (frame 6599) | 0 of 153600 (0.00%) | - |
| t_key.6620 (frame 6620) | 0 of 153600 (0.00%) | - |
| key_config.first (frame 6640) | 0 of 153600 (0.00%) | - |
| key_config.6700 (frame 6700) | 0 of 153600 (0.00%) | - |
| key_config.last (frame 6749) | 0 of 153600 (0.00%) | - |
| option_menu_2.first (frame 6800) | 0 of 153600 (0.00%) | - |
| option_menu_2.6880 (frame 6880) | 0 of 153600 (0.00%) | - |
| option_menu_2.last (frame 6959) | 0 of 153600 (0.00%) | - |
| memory_card.first (frame 7000) | 4341 of 153600 (2.83%) | 232,22-406,169 |
| memory_card.7150 (frame 7150) | 4341 of 153600 (2.83%) | 232,22-406,169 |
| memory_card.last (frame 7299) | 4341 of 153600 (2.83%) | 232,22-406,169 |
| option_toggles.first (frame 7350) | 0 of 153600 (0.00%) | - |
| option_toggles.7500 (frame 7500) | 0 of 153600 (0.00%) | - |
| option_toggles.music (frame 7540) | 0 of 153600 (0.00%) | - |
| option_toggles.difficulty (frame 7690) | 0 of 153600 (0.00%) | - |
| option_toggles.monitor (frame 7840) | 0 of 153600 (0.00%) | - |
| option_toggles.last (frame 8149) | 0 of 153600 (0.00%) | - |
| top_menu_2.first (frame 8350) | 0 of 153600 (0.00%) | - |
| top_menu_2.last (frame 8999) | 0 of 153600 (0.00%) | - |

A stub counts only if the game kept running (both runs reached the script's end) and the lost effect is the function's own output (.claude/rules/naming-runtime-evidence.md). Pixels that differ beyond the function's own drawing point at changed game state. A call already in progress when the stub went in finishes normally (a leak breakpoint catches only code reaching the word after the stub).
