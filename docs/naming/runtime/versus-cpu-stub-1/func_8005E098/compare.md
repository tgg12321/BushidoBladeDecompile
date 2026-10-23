# Stub-and-observe: func_8005E098 against baseline

- scenario: Vs. CPU mode, stub from phase fight
- baseline: `..\baseline\capture.manifest.json` (SHA1 ccbd7c9b56f25dc888c156a3ce19c155704f459d), outcome ok, script ended True, q124_complete True
- stubbed: `capture.manifest.json` (SHA1 568700292871e1f13cd536dc6c04ed39f48975f8), outcome ok, script ended True, q124_complete True
- stubs: `func_8005E098` (0x8005E098) from phase fight (frame 8350), returns 0, intact at the end, called while stubbed in fight, t_ko, after_ko
- same inputs and script

| screenshot | changed pixels | bounding box (x0,y0-x1,y1) |
|---|---|---|
| boot_intro.last (frame 5489) | 0 of 76800 (0.00%) | - |
| top_menu.first (frame 6050) | 0 of 153600 (0.00%) | - |
| top_menu.last (frame 6199) | 0 of 153600 (0.00%) | - |
| char_select.first (frame 6500) | 0 of 153600 (0.00%) | - |
| char_select.last (frame 6799) | 0 of 153600 (0.00%) | - |
| weapon_select.first (frame 6900) | 0 of 153600 (0.00%) | - |
| weapon_select.last (frame 7399) | 0 of 153600 (0.00%) | - |
| t_load.mid (frame 8000) | 0 of 153600 (0.00%) | - |
| fight.8350 (frame 8350) | 0 of 153600 (0.00%) | - |
| fight.8410 (frame 8410) | 741 of 153600 (0.48%) | 521,22-631,36 |
| fight.8470 (frame 8470) | 95574 of 153600 (62.22%) | 0,0-639,239 |
| fight.8530 (frame 8530) | 52353 of 153600 (34.08%) | 0,0-639,239 |
| fight.8590 (frame 8590) | 79071 of 153600 (51.48%) | 0,0-639,239 |
| fight.last (frame 8649) | 109760 of 153600 (71.46%) | 0,0-639,239 |
| after_ko.first (frame 8750) | 143524 of 153600 (93.44%) | 0,0-639,239 |
| after_ko.last (frame 9299) | 150467 of 153600 (97.96%) | 0,0-639,239 |

A stub counts only if the game kept running (both runs reached the script's end) and the lost effect is the function's own output (.claude/rules/naming-runtime-evidence.md). Pixels that differ beyond the function's own drawing point at changed game state. A call already in progress when the stub went in finishes normally (a leak breakpoint catches only code reaching the word after the stub).
