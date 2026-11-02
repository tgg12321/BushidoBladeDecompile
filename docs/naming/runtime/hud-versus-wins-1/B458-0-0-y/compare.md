# Set-and-observe: B458-0-0-y against baseline

- scenario: Vs. 2P, versus-2p-bamboo script (KANNUKI vs GENGORO, Bamboo Thicket) cut at 8700: the fight HUD before any fight input; poked: D_8009B458[0][0].y (0x8009B45A) 23 -> 83 at t_load
- baseline: `..\baseline\capture.manifest.json` (SHA1 d192feb06d57533bade4d87a90c25207627e00b4), outcome ok, script ended True, q124_complete True
- poked: `capture.manifest.json` (SHA1 6f928c3815c6e575a57d3cc8e5510ce7df63a5a6), outcome ok, script ended True, q124_complete True
- stubs: none
- pokes: `0x8009B45A` (0x8009B45A) = 5300 (was 1700) from phase t_load (frame 7525), 5300 at the end
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
| t_load.last (frame 8349) | 942 of 153600 (0.61%) | 565,24-624,95 |
| fight.first (frame 8350) | 952 of 153600 (0.62%) | 565,24-624,95 |
| fight.8500 (frame 8500) | 941 of 153600 (0.61%) | 565,24-624,95 |
| fight.8650 (frame 8650) | 941 of 153600 (0.61%) | 565,24-624,95 |
| fight.8699 (frame 8699) | 941 of 153600 (0.61%) | 565,24-624,95 |

A poke counts only if the game kept running (both runs reached the script's end). Everything after it went in is a modified game: the differences show what the game does with the poked value, not what an unmodified run reaches.
