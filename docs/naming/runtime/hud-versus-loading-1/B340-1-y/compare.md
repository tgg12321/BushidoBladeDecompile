# Set-and-observe: B340-1-y against baseline

- scenario: Vs. 2P, versus-2p-bamboo script (KANNUKI vs GENGORO, Bamboo Thicket) cut at 8240: the Vs. loading screen before the fight; poked: D_8009B340[1].y (0x8009B34A) 170 -> 110 at t_load
- baseline: `..\baseline\capture.manifest.json` (SHA1 aa300ba1c38e5a1dcb41c17382a44f2c6fc0a062), outcome ok, script ended True, q124_complete True
- poked: `capture.manifest.json` (SHA1 1d625cc1cc392d5614d794ebb32f1a2110783e4f), outcome ok, script ended True, q124_complete True
- stubs: none
- pokes: `0x8009B34A` (0x8009B34A) = 6e00 (was aa00) from phase t_load (frame 7525), 6e00 at the end
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
| t_load.7600 (frame 7600) | 3414 of 153600 (2.22%) | 393,110-456,212 |
| t_load.7800 (frame 7800) | 3414 of 153600 (2.22%) | 393,110-456,212 |
| t_load.mid (frame 7900) | 3414 of 153600 (2.22%) | 393,110-456,212 |
| t_load.8000 (frame 8000) | 3414 of 153600 (2.22%) | 393,110-456,212 |
| t_load.8200 (frame 8200) | 3414 of 153600 (2.22%) | 393,110-456,212 |

A poke counts only if the game kept running (both runs reached the script's end). Everything after it went in is a modified game: the differences show what the game does with the poked value, not what an unmodified run reaches.
