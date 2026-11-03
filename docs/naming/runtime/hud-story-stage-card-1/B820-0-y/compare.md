# Set-and-observe: B820-0-y against baseline

- scenario: Story Mode, story-concede script (KANNUKI, default weapon) cut at 8790: the first fight's stage card before the fight; poked: D_8009B820[0].y (0x8009B822) 193 -> 133 at t_stage
- baseline: `..\baseline\capture.manifest.json` (SHA1 09df7774e478c58bf51ca60b0839acb55e8494d8), outcome ok, script ended True, q124_complete True
- poked: `capture.manifest.json` (SHA1 a37dfd8055138fc52a5e1966444b4c342bf2c2ef), outcome ok, script ended True, q124_complete True
- stubs: none
- pokes: `0x8009B822` (0x8009B822) = 8500 (was c100) from phase t_stage (frame 7900), 8500 at the end
- same inputs and script

| screenshot | changed pixels | bounding box (x0,y0-x1,y1) |
|---|---|---|
| boot_intro.last (frame 5489) | 0 of 76800 (0.00%) | - |
| top_menu.first (frame 6060) | 0 of 153600 (0.00%) | - |
| char_select.first (frame 6950) | 0 of 153600 (0.00%) | - |
| stage_card.first (frame 8360) | 386 of 153600 (0.25%) | 319,199-382,214 |
| stage_card.8450 (frame 8450) | 386 of 153600 (0.25%) | 319,199-382,214 |
| stage_card.8600 (frame 8600) | 386 of 153600 (0.25%) | 319,199-382,214 |
| stage_card.8750 (frame 8750) | 386 of 153600 (0.25%) | 319,199-382,214 |

A poke counts only if the game kept running (both runs reached the script's end). Everything after it went in is a modified game: the differences show what the game does with the poked value, not what an unmodified run reaches.
