# Set-and-observe: B7D0-y against baseline

- scenario: Story Mode, story-gengoro-stage-0e script (GENGORO, character id 15, default weapon) cut at 8790: the first fight's stage card (stage 0x02) before the fight; poked: D_8009B7D0.y (0x8009B7D2) 52 -> 112 at t_stage
- baseline: `..\baseline\capture.manifest.json` (SHA1 12fd9cfddd8a3ec511cdc524c3034a147135d2a9), outcome ok, script ended True, q124_complete True
- poked: `capture.manifest.json` (SHA1 beabd451ff08a2b067e6eff3d690e8cc49fa8f58), outcome ok, script ended True, q124_complete True
- stubs: none
- pokes: `0x8009B7D2` (0x8009B7D2) = 7000 (was 3400) from phase t_stage (frame 7900), 7000 at the end
- same inputs and script

| screenshot | changed pixels | bounding box (x0,y0-x1,y1) |
|---|---|---|
| boot_intro.last (frame 5489) | 0 of 76800 (0.00%) | - |
| top_menu.first (frame 6060) | 0 of 153600 (0.00%) | - |
| char_select.first (frame 6950) | 0 of 153600 (0.00%) | - |
| char_select.last (frame 7299) | 0 of 153600 (0.00%) | - |
| weapon_select.first (frame 7360) | 0 of 153600 (0.00%) | - |
| stage_02.card (frame 8360) | 60399 of 153600 (39.32%) | 65,52-422,238 |
| stage_02.8450 (frame 8450) | 60309 of 153600 (39.26%) | 65,52-422,238 |
| stage_02.8600 (frame 8600) | 60309 of 153600 (39.26%) | 65,52-422,238 |
| stage_02.8750 (frame 8750) | 60309 of 153600 (39.26%) | 65,52-422,238 |

A poke counts only if the game kept running (both runs reached the script's end). Everything after it went in is a modified game: the differences show what the game does with the poked value, not what an unmodified run reaches.
