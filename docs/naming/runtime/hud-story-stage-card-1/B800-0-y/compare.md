# Set-and-observe: B800-0-y against baseline

- scenario: Story Mode, story-concede script (KANNUKI, default weapon) cut at 8790: the first fight's stage card before the fight; poked: D_8009B800[0].y (0x8009B802) 52 -> 112 at t_stage
- baseline: `..\baseline\capture.manifest.json` (SHA1 09df7774e478c58bf51ca60b0839acb55e8494d8), outcome ok, script ended True, q124_complete True
- poked: `capture.manifest.json` (SHA1 cf7dafe5e52f90381803a71757de3901c3c1b0a4), outcome ok, script ended True, q124_complete True
- stubs: none
- pokes: `0x8009B802` (0x8009B802) = 7000 (was 3400) from phase t_stage (frame 7900), 7000 at the end
- same inputs and script

| screenshot | changed pixels | bounding box (x0,y0-x1,y1) |
|---|---|---|
| boot_intro.last (frame 5489) | 0 of 76800 (0.00%) | - |
| top_menu.first (frame 6060) | 0 of 153600 (0.00%) | - |
| char_select.first (frame 6950) | 0 of 153600 (0.00%) | - |
| stage_card.first (frame 8360) | 6662 of 153600 (4.34%) | 447,52-510,175 |
| stage_card.8450 (frame 8450) | 6662 of 153600 (4.34%) | 447,52-510,175 |
| stage_card.8600 (frame 8600) | 6662 of 153600 (4.34%) | 447,52-510,175 |
| stage_card.8750 (frame 8750) | 6662 of 153600 (4.34%) | 447,52-510,175 |

A poke counts only if the game kept running (both runs reached the script's end). Everything after it went in is a modified game: the differences show what the game does with the poked value, not what an unmodified run reaches.
