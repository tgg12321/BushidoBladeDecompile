# Set-and-observe: B7C0-ubase against baseline

- scenario: Story Mode, story-final-fight-retail script (KANNUKI, default weapon) cut at 26400: the second stage card (Terraced Fields); breakpoint gpu_DrawSpriteAtPackedTablePosition, read watch on 0x8009B7AC..0x8009B7CF; poked: g_gpu_stage_card_map_marker_sheet_headers[1] ubase (0x8009B7C0) 0xC8 -> 0xB4 at card2 (25900)
- baseline: `..\baseline\capture.manifest.json` (SHA1 b3777aa47aeedc2e192ee54497e389e9d804c427), outcome ok, script ended True, q124_complete True
- poked: `capture.manifest.json` (SHA1 b00e1e7778cb1e2f4c80fab9d243d5354bf8ca89), outcome ok, script ended True, q124_complete True
- stubs: none
- pokes: `0x8009B7C0` (0x8009B7C0) = b4 (was c8) from phase card2 (frame 25900), b4 at the end
- same inputs and script

| screenshot | changed pixels | bounding box (x0,y0-x1,y1) |
|---|---|---|
| s.5489 (frame 5489) | 0 of 76800 (0.00%) | - |
| s.8899 (frame 8899) | 0 of 153600 (0.00%) | - |
| card2.before (frame 25899) | 0 of 153600 (0.00%) | - |
| card2.26000 (frame 26000) | 0 of 153600 (0.00%) | - |
| card2.26100 (frame 26100) | 160 of 153600 (0.10%) | 381,130-400,142 |
| card2.26200 (frame 26200) | 160 of 153600 (0.10%) | 381,130-400,142 |
| card2.26320 (frame 26320) | 160 of 153600 (0.10%) | 381,130-400,142 |
| card2.last (frame 26399) | 160 of 153600 (0.10%) | 381,130-400,142 |

A poke counts only if the game kept running (both runs reached the script's end). Everything after it went in is a modified game: the differences show what the game does with the poked value, not what an unmodified run reaches.
