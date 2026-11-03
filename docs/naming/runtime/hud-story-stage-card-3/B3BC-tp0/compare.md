# Set-and-observe: B3BC-tp0 against baseline

- scenario: Story Mode, story-concede script (KANNUKI, default weapon) cut at 8790: the first fight's stage card before the fight; read watch on g_gpu_digit_label_and_line_segment_sheet_headers[3] (0x8009B3BC, 12 bytes); poked: g_gpu_digit_label_and_line_segment_sheet_headers[3].tp0 (0x8009B3BC) 0x1F -> 0x1E at t_stage
- baseline: `..\baseline\capture.manifest.json` (SHA1 5e9271f00b1c2c67b218ff9c5f91d0f203034617), outcome ok, script ended True, q124_complete True
- poked: `capture.manifest.json` (SHA1 cc7d2b5736db6b90d82677739238f46cc5c2f5fd), outcome ok, script ended True, q124_complete True
- stubs: none
- pokes: `0x8009B3BC` (0x8009B3BC) = 1e (was 1f) from phase t_stage (frame 7900), 1e at the end
- same inputs and script

| screenshot | changed pixels | bounding box (x0,y0-x1,y1) |
|---|---|---|
| boot_intro.last (frame 5489) | 0 of 76800 (0.00%) | - |
| top_menu.first (frame 6060) | 0 of 153600 (0.00%) | - |
| char_select.first (frame 6950) | 0 of 153600 (0.00%) | - |
| stage_card.first (frame 8360) | 5591 of 153600 (3.64%) | 319,193-636,214 |
| stage_card.8450 (frame 8450) | 5591 of 153600 (3.64%) | 319,193-636,214 |
| stage_card.8600 (frame 8600) | 5591 of 153600 (3.64%) | 319,193-636,214 |
| stage_card.8750 (frame 8750) | 5591 of 153600 (3.64%) | 319,193-636,214 |

A poke counts only if the game kept running (both runs reached the script's end). Everything after it went in is a modified game: the differences show what the game does with the poked value, not what an unmodified run reaches.
