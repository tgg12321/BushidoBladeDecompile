# Set-and-observe: B304-1-ubase against baseline

- scenario: Vs. CPU, versus-cpu-tatsumi script (TATSUMI) cut at 9200: the screen after P1 beats the first opponent (draw_mode 2 from 8862); breakpoint gpu_DrawSheetPairSelectedByDrawMode (max_hits 300), read watch on 0x8009B304..0x8009B33F; poked: g_gpu_draw_mode_sheet_header_rows[1][1].ubase (0x8009b318) 0x0 -> 0x10 at opp1 (8350)
- baseline: `..\baseline\capture.manifest.json` (SHA1 cde305fcb5ba481714ec3fc1d0022fe5fd8c88d0), outcome ok, script ended True, q124_complete True
- poked: `capture.manifest.json` (SHA1 f21c7bd5b3bf85ace34b5c5a1ac7fba39729c258), outcome ok, script ended True, q124_complete True
- stubs: none
- pokes: `0x8009B318` (0x8009B318) = 1000 (was 0000) from phase opp1 (frame 8350), 1000 at the end
- same inputs and script

| screenshot | changed pixels | bounding box (x0,y0-x1,y1) |
|---|---|---|
| boot_intro.last (frame 5489) | 0 of 76800 (0.00%) | - |
| top_menu.first (frame 6050) | 0 of 153600 (0.00%) | - |
| char_select.pick (frame 6599) | 0 of 153600 (0.00%) | - |
| weapon_select.first (frame 6900) | 0 of 153600 (0.00%) | - |
| t_load.mid (frame 8000) | 0 of 153600 (0.00%) | - |
| opp1.first (frame 8350) | 0 of 153600 (0.00%) | - |
| opp1.8600 (frame 8600) | 0 of 153600 (0.00%) | - |
| opp1.last (frame 8850) | 0 of 153600 (0.00%) | - |
| opp1.8860 (frame 8860) | 0 of 153600 (0.00%) | - |
| opp1.8880 (frame 8880) | 6385 of 153600 (4.16%) | 476,134-619,221 |
| opp2.8920 (frame 8920) | 6385 of 153600 (4.16%) | 476,134-619,221 |
| opp2.8960 (frame 8960) | 6385 of 153600 (4.16%) | 476,134-619,221 |
| opp2.9000 (frame 9000) | 6385 of 153600 (4.16%) | 476,134-619,221 |
| opp2.9100 (frame 9100) | 6385 of 153600 (4.16%) | 476,134-619,221 |
| opp2.last (frame 9199) | 6385 of 153600 (4.16%) | 476,134-619,221 |

A poke counts only if the game kept running (both runs reached the script's end). Everything after it went in is a modified game: the differences show what the game does with the poked value, not what an unmodified run reaches.
