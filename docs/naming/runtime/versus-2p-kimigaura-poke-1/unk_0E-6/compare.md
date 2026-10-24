# Stub-and-observe: unk_0E-6 against baseline

- scenario: versus-2p-kimigaura-weapon-nodachi-katana script; P1 unk_0E (D_80101EC8+0xE) poked 2 -> 6 when fight starts
- baseline: `..\baseline\capture.manifest.json` (SHA1 d04553d4bd7ff999795ba04212519ac8089c48b3), outcome ok, script ended True, q124_complete True
- stubbed: `capture.manifest.json` (SHA1 ccf1d8488feca406eeec35b0d7d512790a042d0d), outcome ok, script ended True, q124_complete True
- stubs: none
- pokes: `D_80101EC8+0xE` (0x80101ED6) = 0600 (was 0200) from phase fight (frame 8230), 0600 at the end
- same inputs and script

| screenshot | changed pixels | bounding box (x0,y0-x1,y1) |
|---|---|---|
| boot_intro.last (frame 5489) | 0 of 76800 (0.00%) | - |
| top_menu.first (frame 6050) | 0 of 153600 (0.00%) | - |
| top_menu.last (frame 6259) | 0 of 153600 (0.00%) | - |
| char_select.first (frame 6550) | 0 of 153600 (0.00%) | - |
| char_select.last (frame 6699) | 0 of 153600 (0.00%) | - |
| weapon_select.first (frame 6700) | 0 of 153600 (0.00%) | - |
| weapon_select.pick (frame 6880) | 0 of 153600 (0.00%) | - |
| weapon_select.last (frame 6999) | 0 of 153600 (0.00%) | - |
| t_stage.mid (frame 7100) | 0 of 153600 (0.00%) | - |
| stage_select.first (frame 7175) | 0 of 153600 (0.00%) | - |
| stage_select.7299 (frame 7299) | 0 of 153600 (0.00%) | - |
| stage_select.7325 (frame 7325) | 0 of 153600 (0.00%) | - |
| stage_select.7355 (frame 7355) | 0 of 153600 (0.00%) | - |
| stage_select.last (frame 7449) | 0 of 153600 (0.00%) | - |
| t_load.mid (frame 7900) | 0 of 153600 (0.00%) | - |
| t_load.last (frame 8229) | 0 of 153600 (0.00%) | - |
| fight.first (frame 8230) | 0 of 153600 (0.00%) | - |
| fight.8330 (frame 8330) | 0 of 153600 (0.00%) | - |
| fight.8430 (frame 8430) | 0 of 153600 (0.00%) | - |
| fight.8530 (frame 8530) | 1175 of 153600 (0.76%) | 72,60-321,148 |
| fight.8630 (frame 8630) | 137437 of 153600 (89.48%) | 0,0-639,239 |
| fight.8730 (frame 8730) | 138184 of 153600 (89.96%) | 0,0-639,239 |
| fight.8830 (frame 8830) | 139339 of 153600 (90.72%) | 0,0-639,239 |
| fight.8930 (frame 8930) | 146739 of 153600 (95.53%) | 0,0-639,239 |
| fight.9030 (frame 9030) | 145931 of 153600 (95.01%) | 0,0-639,239 |
| fight.9130 (frame 9130) | 147955 of 153600 (96.32%) | 0,0-639,239 |
| fight.9230 (frame 9230) | 146980 of 153600 (95.69%) | 0,0-639,239 |
| fight.9330 (frame 9330) | 151357 of 153600 (98.54%) | 0,0-639,239 |
| fight.9430 (frame 9430) | 151756 of 153600 (98.80%) | 0,0-639,239 |
| fight.9530 (frame 9530) | 153524 of 153600 (99.95%) | 0,0-639,239 |
| fight.9630 (frame 9630) | 150787 of 153600 (98.17%) | 0,0-639,239 |
| fight.9730 (frame 9730) | 152956 of 153600 (99.58%) | 0,0-639,239 |
| fight.9830 (frame 9830) | 150814 of 153600 (98.19%) | 0,0-639,239 |
| fight.9930 (frame 9930) | 152599 of 153600 (99.35%) | 0,0-639,239 |
| fight.10030 (frame 10030) | 153316 of 153600 (99.82%) | 0,0-639,239 |
| fight.10130 (frame 10130) | 152957 of 153600 (99.58%) | 0,0-639,239 |
| fight.10230 (frame 10230) | 153180 of 153600 (99.73%) | 0,0-639,239 |
| fight.10330 (frame 10330) | 153373 of 153600 (99.85%) | 0,0-639,239 |
| fight.10430 (frame 10430) | 153112 of 153600 (99.68%) | 0,0-639,239 |
| fight.10530 (frame 10530) | 152834 of 153600 (99.50%) | 0,0-639,239 |

A stub counts only if the game kept running (both runs reached the script's end) and the lost effect is the function's own output (.claude/rules/naming-runtime-evidence.md). Pixels that differ beyond the function's own drawing point at changed game state. A call already in progress when the stub went in finishes normally (a leak breakpoint catches only code reaching the word after the stub).
