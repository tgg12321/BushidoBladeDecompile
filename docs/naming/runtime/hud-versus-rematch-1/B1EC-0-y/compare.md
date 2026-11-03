# Set-and-observe: B1EC-0-y against baseline

- scenario: Vs. 2P, versus-2p-bamboo script (KANNUKI vs GENGORO, Bamboo Thicket) cut at 10700: the post-KO Rematch menu; poked: D_8009B1EC[0].y (0x8009B1EE) 34 -> 94 at fight
- baseline: `..\baseline\capture.manifest.json` (SHA1 6e197f2bb024d2f7181b9a3da454f5366d68b1af), outcome ok, script ended True, q124_complete True
- poked: `capture.manifest.json` (SHA1 10000d9e06732e803c2a5415ac10fe768722bd3f), outcome ok, script ended True, q124_complete True
- stubs: none
- pokes: `0x8009B1EE` (0x8009B1EE) = 5e00 (was 2200) from phase fight (frame 8350), 5e00 at the end
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
| fight.8650 (frame 8650) | 0 of 153600 (0.00%) | - |
| fight.8950 (frame 8950) | 0 of 153600 (0.00%) | - |
| fight.9250 (frame 9250) | 0 of 153600 (0.00%) | - |
| fight.9550 (frame 9550) | 0 of 153600 (0.00%) | - |
| fight.9750 (frame 9750) | 0 of 153600 (0.00%) | - |
| fight.9850 (frame 9850) | 0 of 153600 (0.00%) | - |
| fight.last (frame 9899) | 0 of 153600 (0.00%) | - |
| t_ko.9950 (frame 9950) | 0 of 153600 (0.00%) | - |
| t_ko.10050 (frame 10050) | 0 of 153600 (0.00%) | - |
| t_ko.10150 (frame 10150) | 0 of 153600 (0.00%) | - |
| t_ko.10350 (frame 10350) | 0 of 153600 (0.00%) | - |
| t_ko.last (frame 10449) | 989 of 153600 (0.64%) | 203,128-266,199 |
| rematch_menu.first (frame 10450) | 989 of 153600 (0.64%) | 203,128-266,199 |
| rematch_menu.10550 (frame 10550) | 991 of 153600 (0.65%) | 203,128-266,199 |
| rematch_menu.10699 (frame 10699) | 987 of 153600 (0.64%) | 203,128-266,199 |

A poke counts only if the game kept running (both runs reached the script's end). Everything after it went in is a modified game: the differences show what the game does with the poked value, not what an unmodified run reaches.
