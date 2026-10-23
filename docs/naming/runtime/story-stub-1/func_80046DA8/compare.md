# Stub-and-observe: func_80046DA8 against baseline

- baseline: `..\baseline\capture.manifest.json` (SHA1 7d6cb9203d30ed1e4a26065ef04c79cc27fd19a9), outcome ok, script ended True, q124_complete True
- stubbed: `capture.manifest.json` (SHA1 fd87591493c542be1787f7231eaae618d3b60b22), outcome ok, script ended True, q124_complete True
- stubs: `func_80046DA8` (0x80046DA8) from phase t_fight (frame 8800), returns with v0 unchanged, intact at the end, called while stubbed in t_fight, fight, t_ko
- same inputs and script

| screenshot | changed pixels | bounding box (x0,y0-x1,y1) |
|---|---|---|
| boot_intro.last (frame 5489) | 0 of 76800 (0.00%) | - |
| top_menu.first (frame 6060) | 0 of 153600 (0.00%) | - |
| top_menu.last (frame 6099) | 0 of 153600 (0.00%) | - |
| story_text.first (frame 6810) | 0 of 153600 (0.00%) | - |
| story_text.last (frame 6909) | 0 of 153600 (0.00%) | - |
| char_select.first (frame 6950) | 0 of 153600 (0.00%) | - |
| char_select.last (frame 7299) | 0 of 153600 (0.00%) | - |
| weapon_select.first (frame 7360) | 0 of 153600 (0.00%) | - |
| weapon_select.last (frame 7899) | 0 of 153600 (0.00%) | - |
| stage_card.first (frame 8360) | 0 of 153600 (0.00%) | - |
| stage_card.last (frame 8799) | 0 of 153600 (0.00%) | - |
| fight.8900 (frame 8900) | 123930 of 153600 (80.68%) | 0,0-639,239 |
| fight.8960 (frame 8960) | 122314 of 153600 (79.63%) | 0,0-639,239 |
| fight.9020 (frame 9020) | 119283 of 153600 (77.66%) | 0,0-639,239 |
| fight.9080 (frame 9080) | 118828 of 153600 (77.36%) | 0,0-639,239 |
| fight.9140 (frame 9140) | 119753 of 153600 (77.96%) | 0,0-639,239 |
| fight.last (frame 9199) | 121535 of 153600 (79.12%) | 0,0-639,239 |
| t_ko.f9299 (frame 9299) | 121459 of 153600 (79.07%) | 0,0-639,239 |

A stub counts only if the game kept running (both runs reached the script's end) and the lost effect is the function's own output (.claude/rules/naming-runtime-evidence.md). Pixels that differ beyond the function's own drawing point at changed game state. A call already in progress when the stub went in finishes normally (a leak breakpoint catches only code reaching the word after the stub).
