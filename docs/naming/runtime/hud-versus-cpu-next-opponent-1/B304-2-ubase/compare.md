# Set-and-observe: B304-2-ubase against baseline

- scenario: Vs. CPU, versus-cpu-tatsumi script (TATSUMI) cut at 9200: the screen after P1 beats the first opponent (draw_mode 2 from 8862); breakpoint gpu_DrawSheetPairSelectedByDrawMode (max_hits 300), read watch on 0x8009B304..0x8009B33F; poked: g_gpu_draw_mode_sheet_header_rows[1][2].ubase (0x8009b324) 0x90 -> 0xa0 at opp1 (8350)
- baseline: `..\baseline\capture.manifest.json` (SHA1 cde305fcb5ba481714ec3fc1d0022fe5fd8c88d0), outcome ok, script ended True, q124_complete True
- poked: `capture.manifest.json` (SHA1 997dda364a94700a7496d61d2765e78422f4742e), outcome ok, script ended True, q124_complete True
- stubs: none
- pokes: `0x8009B324` (0x8009B324) = a000 (was 9000) from phase opp1 (frame 8350), a000 at the end
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
| opp1.8880 (frame 8880) | 0 of 153600 (0.00%) | - |
| opp2.8920 (frame 8920) | 675 of 153600 (0.44%) | 567,78-637,105 |
| opp2.8960 (frame 8960) | 2577 of 153600 (1.68%) | 274,71-637,195 |
| opp2.9000 (frame 9000) | 4557 of 153600 (2.97%) | 86,71-637,235 |
| opp2.9100 (frame 9100) | 8443 of 153600 (5.50%) | 43,11-637,235 |
| opp2.last (frame 9199) | 10690 of 153600 (6.96%) | 0,6-637,235 |

A poke counts only if the game kept running (both runs reached the script's end). Everything after it went in is a modified game: the differences show what the game does with the poked value, not what an unmodified run reaches.
