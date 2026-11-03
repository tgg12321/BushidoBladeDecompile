# Set-and-observe: B7D8-1-y against baseline

- scenario: Story Mode, story-concede script (KANNUKI, default weapon) cut at 8790: the first fight's stage card before the fight; poked: D_8009B7D8[1].y (0x8009B7E2) 30 -> 90 at t_stage
- baseline: `..\baseline\capture.manifest.json` (SHA1 09df7774e478c58bf51ca60b0839acb55e8494d8), outcome ok, script ended True, q124_complete True
- poked: `capture.manifest.json` (SHA1 0ac0dc0ee40f90522979c63c37eb0432495d93c6), outcome ok, script ended True, q124_complete True
- stubs: none
- pokes: `0x8009B7E2` (0x8009B7E2) = 5a00 (was 1e00) from phase t_stage (frame 7900), 5a00 at the end
- same inputs and script

| screenshot | changed pixels | bounding box (x0,y0-x1,y1) |
|---|---|---|
| boot_intro.last (frame 5489) | 0 of 76800 (0.00%) | - |
| top_menu.first (frame 6060) | 0 of 153600 (0.00%) | - |
| char_select.first (frame 6950) | 0 of 153600 (0.00%) | - |
| stage_card.first (frame 8360) | 100 of 153600 (0.07%) | 134,32-140,101 |
| stage_card.8450 (frame 8450) | 100 of 153600 (0.07%) | 134,32-140,101 |
| stage_card.8600 (frame 8600) | 100 of 153600 (0.07%) | 134,32-140,101 |
| stage_card.8750 (frame 8750) | 100 of 153600 (0.07%) | 134,32-140,101 |

A poke counts only if the game kept running (both runs reached the script's end). Everything after it went in is a modified game: the differences show what the game does with the poked value, not what an unmodified run reaches.
