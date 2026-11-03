# Set-and-observe: B7CC-ubase against baseline

- scenario: Story Mode, story-gengoro-stage-0e script (GENGORO, character id 15) cut at 26900: the second stage card (Ferry Dock); breakpoint gpu_DrawSpriteAtPackedTablePosition, read watch on 0x8009B7AC..0x8009B7CF; poked: g_gpu_stage_card_map_marker_sheet_headers[2] ubase (0x8009B7CC) 0xDC -> 0xB4 at stage_16 (26474)
- baseline: `..\baseline\capture.manifest.json` (SHA1 51b94b31f3aff7515dd55426b80df2b69766fe2f), outcome ok, script ended True, q124_complete True
- poked: `capture.manifest.json` (SHA1 f7e2a9a544048ce9598d32ff7b266c2ed0ca7100), outcome ok, script ended True, q124_complete True
- stubs: none
- pokes: `0x8009B7CC` (0x8009B7CC) = b4 (was dc) from phase stage_16 (frame 26474), b4 at the end
- same inputs and script

| screenshot | changed pixels | bounding box (x0,y0-x1,y1) |
|---|---|---|
| boot_intro.last (frame 5489) | 0 of 76800 (0.00%) | - |
| top_menu.first (frame 6060) | 0 of 153600 (0.00%) | - |
| char_select.first (frame 6950) | 0 of 153600 (0.00%) | - |
| char_select.last (frame 7299) | 0 of 153600 (0.00%) | - |
| weapon_select.first (frame 7360) | 0 of 153600 (0.00%) | - |
| stage_02.card (frame 8360) | 0 of 153600 (0.00%) | - |
| stage_02.10000 (frame 10000) | 0 of 153600 (0.00%) | - |
| stage_02.last (frame 26473) | 0 of 153600 (0.00%) | - |
| stage_16.26500 (frame 26500) | 0 of 153600 (0.00%) | - |
| stage_16.26600 (frame 26600) | 158 of 153600 (0.10%) | 89,99-108,111 |
| stage_16.26700 (frame 26700) | 158 of 153600 (0.10%) | 89,99-108,111 |
| stage_16.26820 (frame 26820) | 158 of 153600 (0.10%) | 89,99-108,111 |
| stage_16.last (frame 26899) | 158 of 153600 (0.10%) | 89,99-108,111 |

A poke counts only if the game kept running (both runs reached the script's end). Everything after it went in is a modified game: the differences show what the game does with the poked value, not what an unmodified run reaches.
