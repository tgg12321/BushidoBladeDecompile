# Set-and-observe: B7A0-ubase against baseline

- scenario: Story Mode, story-concede script (KANNUKI, default weapon) cut at 8790: the first fight's stage card before the fight; read watch on D_8009B7A0 (0x8009B7A0, 12 bytes); poked: D_8009B7A0.ubase (0x8009B7A8) 0 -> 8 at t_stage
- baseline: `..\baseline\capture.manifest.json` (SHA1 3e6b64279c565c61e21b6c71013851c3fde6a20b), outcome ok, script ended True, q124_complete True
- poked: `capture.manifest.json` (SHA1 aee7f5406b54e3c67a8a84edbdae55f9d6d4b7f4), outcome ok, script ended True, q124_complete True
- stubs: none
- pokes: `0x8009B7A8` (0x8009B7A8) = 0800 (was 0000) from phase t_stage (frame 7900), 0800 at the end
- same inputs and script

| screenshot | changed pixels | bounding box (x0,y0-x1,y1) |
|---|---|---|
| boot_intro.last (frame 5489) | 0 of 76800 (0.00%) | - |
| top_menu.first (frame 6060) | 0 of 153600 (0.00%) | - |
| char_select.first (frame 6950) | 0 of 153600 (0.00%) | - |
| stage_card.first (frame 8360) | 1772 of 153600 (1.15%) | 319,199-574,210 |
| stage_card.8450 (frame 8450) | 1772 of 153600 (1.15%) | 319,199-574,210 |
| stage_card.8600 (frame 8600) | 1772 of 153600 (1.15%) | 319,199-574,210 |
| stage_card.8750 (frame 8750) | 1772 of 153600 (1.15%) | 319,199-574,210 |

A poke counts only if the game kept running (both runs reached the script's end). Everything after it went in is a modified game: the differences show what the game does with the poked value, not what an unmodified run reaches.
