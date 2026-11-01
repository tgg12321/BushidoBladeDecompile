# Knockout observations

Liveness uses the script end_frame fallback (no RAM counter established). No names or roles are inferred.

## func_8003F218

Status: **ok**. script reached end_frame 14200 past requested frames 8900,9200,9500,9800,10100,11000,12200,12399,12500,12700,13200,13500,14100 (fallback).

# Stub-and-observe: func_8003F218 against baseline

- scenario: story-pov-rematch
- baseline: `docs\naming\runtime\story-pov-rematch-view-flag-stub-1\baseline\capture.manifest.json` (SHA1 394c979933d24043cd5871002c043e8db2303f39), outcome ok, script ended True, q124_complete True
- stubbed: `docs\naming\runtime\story-pov-rematch-view-flag-stub-1\func_8003F218\capture.manifest.json` (SHA1 9cddbfff427046f357461792221eafb2c957a89f), outcome ok, script ended True, q124_complete True
- stubs: `func_8003F218` (0x8003F218) from phase stage_card (frame 8360), returns with v0 unchanged, intact at the end, called while stubbed in stage_card, fight, t_ko, rematch_menu, t_rematch, fight_2
- same inputs and script

| screenshot | changed pixels | bounding box (x0,y0-x1,y1) |
|---|---|---|
| boot_intro.last (frame 5489) | 0 of 76800 (0.00%) | - |
| top_menu.first (frame 6060) | 0 of 153600 (0.00%) | - |
| top_menu.side_view (frame 6069) | 0 of 153600 (0.00%) | - |
| top_menu.pov (frame 6095) | 0 of 153600 (0.00%) | - |
| top_menu.last (frame 6099) | 0 of 153600 (0.00%) | - |
| t_story.first (frame 6100) | 0 of 153600 (0.00%) | - |
| t_story.mid (frame 6455) | 0 of 153600 (0.00%) | - |
| t_story.last (frame 6809) | 0 of 153600 (0.00%) | - |
| story_text.first (frame 6810) | 0 of 153600 (0.00%) | - |
| story_text.last (frame 6909) | 0 of 153600 (0.00%) | - |
| t_char.first (frame 6910) | 0 of 153600 (0.00%) | - |
| t_char.last (frame 6949) | 0 of 153600 (0.00%) | - |
| char_select.first (frame 6950) | 0 of 153600 (0.00%) | - |
| char_select.mid (frame 7125) | 0 of 153600 (0.00%) | - |
| char_select.last (frame 7299) | 0 of 153600 (0.00%) | - |
| t_weapon.first (frame 7300) | 0 of 153600 (0.00%) | - |
| t_weapon.last (frame 7359) | 0 of 153600 (0.00%) | - |
| weapon_select.first (frame 7360) | 0 of 153600 (0.00%) | - |
| weapon_select.mid (frame 7630) | 0 of 153600 (0.00%) | - |
| weapon_select.last (frame 7899) | 0 of 153600 (0.00%) | - |
| t_stage.first (frame 7900) | 0 of 153600 (0.00%) | - |
| t_stage.mid (frame 8130) | 0 of 153600 (0.00%) | - |
| t_stage.last (frame 8359) | 0 of 153600 (0.00%) | - |
| stage_card.first (frame 8360) | 0 of 153600 (0.00%) | - |
| stage_card.mid (frame 8580) | 0 of 153600 (0.00%) | - |
| stage_card.last (frame 8799) | 0 of 153600 (0.00%) | - |
| t_fight.first (frame 8800) | 0 of 153600 (0.00%) | - |
| t_fight.last (frame 8899) | 0 of 153600 (0.00%) | - |
| fight.first (frame 8900) | 0 of 153600 (0.00%) | - |
| knockout.8900 (frame 8900) | 0 of 153600 (0.00%) | - |
| fight.9200 (frame 9200) | 7477 of 153600 (4.87%) | 0,0-639,239 |
| knockout.9200 (frame 9200) | 7477 of 153600 (4.87%) | 0,0-639,239 |
| fight.9500 (frame 9500) | 88799 of 153600 (57.81%) | 0,0-639,239 |
| knockout.9500 (frame 9500) | 88799 of 153600 (57.81%) | 0,0-639,239 |
| fight.9800 (frame 9800) | 95372 of 153600 (62.09%) | 0,0-639,239 |
| knockout.9800 (frame 9800) | 95372 of 153600 (62.09%) | 0,0-639,239 |
| fight.10100 (frame 10100) | 88592 of 153600 (57.68%) | 0,0-639,239 |
| knockout.10100 (frame 10100) | 88592 of 153600 (57.68%) | 0,0-639,239 |
| fight.10400 (frame 10400) | 89707 of 153600 (58.40%) | 0,0-639,239 |
| fight.10700 (frame 10700) | 97136 of 153600 (63.24%) | 0,0-639,239 |
| fight.11000 (frame 11000) | 89003 of 153600 (57.94%) | 0,0-639,239 |
| knockout.11000 (frame 11000) | 89003 of 153600 (57.94%) | 0,0-639,239 |
| fight.11300 (frame 11300) | 94605 of 153600 (61.59%) | 0,0-639,239 |
| fight.11600 (frame 11600) | 89288 of 153600 (58.13%) | 0,0-639,239 |
| fight.11900 (frame 11900) | 91159 of 153600 (59.35%) | 0,0-639,239 |
| fight.12200 (frame 12200) | 88233 of 153600 (57.44%) | 0,0-639,239 |
| knockout.12200 (frame 12200) | 88233 of 153600 (57.44%) | 0,0-639,239 |
| fight.last (frame 12399) | 112021 of 153600 (72.93%) | 0,0-639,239 |
| knockout.12399 (frame 12399) | 112021 of 153600 (72.93%) | 0,0-639,239 |
| t_ko.first (frame 12400) | 111993 of 153600 (72.91%) | 0,0-639,239 |
| t_ko.12500 (frame 12500) | 86022 of 153600 (56.00%) | 0,0-639,239 |
| knockout.12500 (frame 12500) | 86022 of 153600 (56.00%) | 0,0-639,239 |
| t_ko.12600 (frame 12600) | 88557 of 153600 (57.65%) | 0,53-639,239 |
| t_ko.mid (frame 12625) | 92522 of 153600 (60.24%) | 0,52-639,239 |
| t_ko.12700 (frame 12700) | 114376 of 153600 (74.46%) | 0,0-639,239 |
| knockout.12700 (frame 12700) | 114376 of 153600 (74.46%) | 0,0-639,239 |
| t_ko.last (frame 12849) | 148714 of 153600 (96.82%) | 0,0-639,239 |
| rematch_menu.first (frame 12850) | 148714 of 153600 (96.82%) | 0,0-639,239 |
| rematch_menu.last (frame 12999) | 143970 of 153600 (93.73%) | 0,0-639,239 |
| t_rematch.first (frame 13000) | 143932 of 153600 (93.71%) | 0,0-639,239 |
| t_rematch.last (frame 13199) | 100743 of 153600 (65.59%) | 0,0-639,239 |
| fight_2.first (frame 13200) | 102450 of 153600 (66.70%) | 0,0-639,239 |
| knockout.13200 (frame 13200) | 102450 of 153600 (66.70%) | 0,0-639,239 |
| fight_2.13500 (frame 13500) | 84168 of 153600 (54.80%) | 0,0-639,239 |
| knockout.13500 (frame 13500) | 84168 of 153600 (54.80%) | 0,0-639,239 |
| fight_2.13800 (frame 13800) | 84141 of 153600 (54.78%) | 0,0-639,239 |
| fight_2.14100 (frame 14100) | 83936 of 153600 (54.65%) | 0,0-639,239 |
| knockout.14100 (frame 14100) | 83936 of 153600 (54.65%) | 0,0-639,239 |
| fight_2.last (frame 14199) | 84091 of 153600 (54.75%) | 0,0-639,239 |
| knockout.live (frame 14199) | 84091 of 153600 (54.75%) | 0,0-639,239 |

A stub counts only if the game kept running (both runs reached the script's end) and the lost effect is the function's own output (.claude/rules/naming-runtime-evidence.md). Pixels that differ beyond the function's own drawing point at changed game state. A call already in progress when the stub went in finishes normally (a leak breakpoint catches only code reaching the word after the stub).

## func_8003F268

Status: **ok**. script reached end_frame 14200 past requested frames 8900,9200,9500,9800,10100,11000,12200,12399,12500,12700,13200,13500,14100 (fallback).

# Stub-and-observe: func_8003F268 against baseline

- scenario: story-pov-rematch
- baseline: `docs\naming\runtime\story-pov-rematch-view-flag-stub-1\baseline\capture.manifest.json` (SHA1 394c979933d24043cd5871002c043e8db2303f39), outcome ok, script ended True, q124_complete True
- stubbed: `docs\naming\runtime\story-pov-rematch-view-flag-stub-1\func_8003F268\capture.manifest.json` (SHA1 776d806824c758f59b0f5f416f5e1881aa0df9eb), outcome ok, script ended True, q124_complete True
- stubs: `func_8003F268` (0x8003F268) from phase stage_card (frame 8360), returns 0, intact at the end, called while stubbed in fight, t_ko, rematch_menu, t_rematch, fight_2
- same inputs and script

| screenshot | changed pixels | bounding box (x0,y0-x1,y1) |
|---|---|---|
| boot_intro.last (frame 5489) | 0 of 76800 (0.00%) | - |
| top_menu.first (frame 6060) | 0 of 153600 (0.00%) | - |
| top_menu.side_view (frame 6069) | 0 of 153600 (0.00%) | - |
| top_menu.pov (frame 6095) | 0 of 153600 (0.00%) | - |
| top_menu.last (frame 6099) | 0 of 153600 (0.00%) | - |
| t_story.first (frame 6100) | 0 of 153600 (0.00%) | - |
| t_story.mid (frame 6455) | 0 of 153600 (0.00%) | - |
| t_story.last (frame 6809) | 0 of 153600 (0.00%) | - |
| story_text.first (frame 6810) | 0 of 153600 (0.00%) | - |
| story_text.last (frame 6909) | 0 of 153600 (0.00%) | - |
| t_char.first (frame 6910) | 0 of 153600 (0.00%) | - |
| t_char.last (frame 6949) | 0 of 153600 (0.00%) | - |
| char_select.first (frame 6950) | 0 of 153600 (0.00%) | - |
| char_select.mid (frame 7125) | 0 of 153600 (0.00%) | - |
| char_select.last (frame 7299) | 0 of 153600 (0.00%) | - |
| t_weapon.first (frame 7300) | 0 of 153600 (0.00%) | - |
| t_weapon.last (frame 7359) | 0 of 153600 (0.00%) | - |
| weapon_select.first (frame 7360) | 0 of 153600 (0.00%) | - |
| weapon_select.mid (frame 7630) | 0 of 153600 (0.00%) | - |
| weapon_select.last (frame 7899) | 0 of 153600 (0.00%) | - |
| t_stage.first (frame 7900) | 0 of 153600 (0.00%) | - |
| t_stage.mid (frame 8130) | 0 of 153600 (0.00%) | - |
| t_stage.last (frame 8359) | 0 of 153600 (0.00%) | - |
| stage_card.first (frame 8360) | 0 of 153600 (0.00%) | - |
| stage_card.mid (frame 8580) | 0 of 153600 (0.00%) | - |
| stage_card.last (frame 8799) | 0 of 153600 (0.00%) | - |
| t_fight.first (frame 8800) | 0 of 153600 (0.00%) | - |
| t_fight.last (frame 8899) | 0 of 153600 (0.00%) | - |
| fight.first (frame 8900) | 0 of 153600 (0.00%) | - |
| knockout.8900 (frame 8900) | 0 of 153600 (0.00%) | - |
| fight.9200 (frame 9200) | 0 of 153600 (0.00%) | - |
| knockout.9200 (frame 9200) | 0 of 153600 (0.00%) | - |
| fight.9500 (frame 9500) | 93649 of 153600 (60.97%) | 0,0-639,239 |
| knockout.9500 (frame 9500) | 93649 of 153600 (60.97%) | 0,0-639,239 |
| fight.9800 (frame 9800) | 99609 of 153600 (64.85%) | 0,0-639,239 |
| knockout.9800 (frame 9800) | 99609 of 153600 (64.85%) | 0,0-639,239 |
| fight.10100 (frame 10100) | 95883 of 153600 (62.42%) | 0,0-639,239 |
| knockout.10100 (frame 10100) | 95883 of 153600 (62.42%) | 0,0-639,239 |
| fight.10400 (frame 10400) | 94520 of 153600 (61.54%) | 0,0-639,239 |
| fight.10700 (frame 10700) | 100840 of 153600 (65.65%) | 0,0-639,239 |
| fight.11000 (frame 11000) | 116745 of 153600 (76.01%) | 0,0-639,239 |
| knockout.11000 (frame 11000) | 116745 of 153600 (76.01%) | 0,0-639,239 |
| fight.11300 (frame 11300) | 150891 of 153600 (98.24%) | 0,0-639,239 |
| fight.11600 (frame 11600) | 150310 of 153600 (97.86%) | 0,0-639,239 |
| fight.11900 (frame 11900) | 151816 of 153600 (98.84%) | 0,0-639,239 |
| fight.12200 (frame 12200) | 150955 of 153600 (98.28%) | 0,0-639,239 |
| knockout.12200 (frame 12200) | 150955 of 153600 (98.28%) | 0,0-639,239 |
| fight.last (frame 12399) | 151283 of 153600 (98.49%) | 0,0-639,239 |
| knockout.12399 (frame 12399) | 151283 of 153600 (98.49%) | 0,0-639,239 |
| t_ko.first (frame 12400) | 151449 of 153600 (98.60%) | 0,0-639,239 |
| t_ko.12500 (frame 12500) | 150120 of 153600 (97.73%) | 0,0-639,239 |
| knockout.12500 (frame 12500) | 150120 of 153600 (97.73%) | 0,0-639,239 |
| t_ko.12600 (frame 12600) | 147427 of 153600 (95.98%) | 0,0-639,239 |
| t_ko.mid (frame 12625) | 148233 of 153600 (96.51%) | 0,0-639,239 |
| t_ko.12700 (frame 12700) | 149169 of 153600 (97.12%) | 0,0-639,239 |
| knockout.12700 (frame 12700) | 149169 of 153600 (97.12%) | 0,0-639,239 |
| t_ko.last (frame 12849) | 137875 of 153600 (89.76%) | 0,0-639,239 |
| rematch_menu.first (frame 12850) | 137765 of 153600 (89.69%) | 0,0-639,239 |
| rematch_menu.last (frame 12999) | 144006 of 153600 (93.75%) | 0,0-639,239 |
| t_rematch.first (frame 13000) | 143798 of 153600 (93.62%) | 0,0-639,239 |
| t_rematch.last (frame 13199) | 99716 of 153600 (64.92%) | 0,0-639,239 |
| fight_2.first (frame 13200) | 101509 of 153600 (66.09%) | 0,0-639,239 |
| knockout.13200 (frame 13200) | 101509 of 153600 (66.09%) | 0,0-639,239 |
| fight_2.13500 (frame 13500) | 59023 of 153600 (38.43%) | 0,0-639,239 |
| knockout.13500 (frame 13500) | 59023 of 153600 (38.43%) | 0,0-639,239 |
| fight_2.13800 (frame 13800) | 64335 of 153600 (41.88%) | 0,0-639,239 |
| fight_2.14100 (frame 14100) | 93971 of 153600 (61.18%) | 0,0-639,239 |
| knockout.14100 (frame 14100) | 93971 of 153600 (61.18%) | 0,0-639,239 |
| fight_2.last (frame 14199) | 92989 of 153600 (60.54%) | 0,0-639,239 |
| knockout.live (frame 14199) | 92989 of 153600 (60.54%) | 0,0-639,239 |

A stub counts only if the game kept running (both runs reached the script's end) and the lost effect is the function's own output (.claude/rules/naming-runtime-evidence.md). Pixels that differ beyond the function's own drawing point at changed game state. A call already in progress when the stub went in finishes normally (a leak breakpoint catches only code reaching the word after the stub).
