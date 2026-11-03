# Set-and-observe: B7BC-cx against baseline

- scenario: Story Mode, story-final-fight-retail script (KANNUKI, default weapon) cut at 26400: the second stage card (Terraced Fields); breakpoint gpu_DrawSpriteAtPackedTablePosition, read watch on 0x8009B7AC..0x8009B7CF; poked: g_gpu_stage_card_map_marker_sheet_headers[1] CLUT x (0x8009B7BC) 0x3B0 -> 0x3A0 (low byte) at card2 (25900)
- baseline: `..\baseline\capture.manifest.json` (SHA1 b3777aa47aeedc2e192ee54497e389e9d804c427), outcome ok, script ended True, q124_complete True
- poked: `capture.manifest.json` (SHA1 8f0719f27240fff02eb3fd197aee4e839b41ec12), outcome ok, script ended True, q124_complete True
- stubs: none
- pokes: `0x8009B7BC` (0x8009B7BC) = a0 (was b0) from phase card2 (frame 25900), a0 at the end
- same inputs and script

| screenshot | changed pixels | bounding box (x0,y0-x1,y1) |
|---|---|---|
| s.5489 (frame 5489) | 0 of 76800 (0.00%) | - |
| s.8899 (frame 8899) | 0 of 153600 (0.00%) | - |
| card2.before (frame 25899) | 0 of 153600 (0.00%) | - |
| card2.26000 (frame 26000) | 0 of 153600 (0.00%) | - |
| card2.26100 (frame 26100) | 140 of 153600 (0.09%) | 388,130-399,142 |
| card2.26200 (frame 26200) | 140 of 153600 (0.09%) | 388,130-399,142 |
| card2.26320 (frame 26320) | 140 of 153600 (0.09%) | 388,130-399,142 |
| card2.last (frame 26399) | 140 of 153600 (0.09%) | 388,130-399,142 |

A poke counts only if the game kept running (both runs reached the script's end). Everything after it went in is a modified game: the differences show what the game does with the poked value, not what an unmodified run reaches.
