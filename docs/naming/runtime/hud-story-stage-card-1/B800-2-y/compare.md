# Set-and-observe: B800-2-y against baseline

- scenario: Story Mode, story-concede script (KANNUKI, default weapon) cut at 8790: the first fight's stage card before the fight; poked: D_8009B800[2].y (0x8009B812) 116 -> 176 at t_stage
- baseline: `..\baseline\capture.manifest.json` (SHA1 09df7774e478c58bf51ca60b0839acb55e8494d8), outcome ok, script ended True, q124_complete True
- poked: `capture.manifest.json` (SHA1 872c4773321b931658beaf17f6e4155e81dc7320), outcome ok, script ended True, q124_complete True
- stubs: none
- pokes: `0x8009B812` (0x8009B812) = b000 (was 7400) from phase t_stage (frame 7900), b000 at the end
- same inputs and script

| screenshot | changed pixels | bounding box (x0,y0-x1,y1) |
|---|---|---|
| boot_intro.last (frame 5489) | 0 of 76800 (0.00%) | - |
| top_menu.first (frame 6060) | 0 of 153600 (0.00%) | - |
| char_select.first (frame 6950) | 0 of 153600 (0.00%) | - |
| stage_card.first (frame 8360) | 7872 of 153600 (5.12%) | 447,116-510,238 |
| stage_card.8450 (frame 8450) | 7872 of 153600 (5.12%) | 447,116-510,238 |
| stage_card.8600 (frame 8600) | 7872 of 153600 (5.12%) | 447,116-510,238 |
| stage_card.8750 (frame 8750) | 7872 of 153600 (5.12%) | 447,116-510,238 |

A poke counts only if the game kept running (both runs reached the script's end). Everything after it went in is a modified game: the differences show what the game does with the poked value, not what an unmodified run reaches.
