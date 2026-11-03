# Set-and-observe: B3BC-tp1 against baseline

- scenario: Story Mode, story-concede script (KANNUKI, default weapon) cut at 8790: the first fight's stage card before the fight; read watch on g_gpu_digit_label_and_line_segment_sheet_headers[3] (0x8009B3BC, 12 bytes); poked: g_gpu_digit_label_and_line_segment_sheet_headers[3].tp1 (0x8009B3BD) 0 -> 1 at t_stage
- baseline: `..\baseline\capture.manifest.json` (SHA1 5e9271f00b1c2c67b218ff9c5f91d0f203034617), outcome ok, script ended True, q124_complete True
- poked: `capture.manifest.json` (SHA1 07cc2d74fd2b522d100d0c284db40301358bab42), outcome ok, script ended True, q124_complete True
- stubs: none
- pokes: `0x8009B3BD` (0x8009B3BD) = 01 (was 00) from phase t_stage (frame 7900), 01 at the end
- same inputs and script

| screenshot | changed pixels | bounding box (x0,y0-x1,y1) |
|---|---|---|
| boot_intro.last (frame 5489) | 0 of 76800 (0.00%) | - |
| top_menu.first (frame 6060) | 0 of 153600 (0.00%) | - |
| char_select.first (frame 6950) | 0 of 153600 (0.00%) | - |
| stage_card.first (frame 8360) | 5523 of 153600 (3.60%) | 319,193-638,214 |
| stage_card.8450 (frame 8450) | 5523 of 153600 (3.60%) | 319,193-638,214 |
| stage_card.8600 (frame 8600) | 5523 of 153600 (3.60%) | 319,193-638,214 |
| stage_card.8750 (frame 8750) | 5523 of 153600 (3.60%) | 319,193-638,214 |

A poke counts only if the game kept running (both runs reached the script's end). Everything after it went in is a modified game: the differences show what the game does with the poked value, not what an unmodified run reaches.
