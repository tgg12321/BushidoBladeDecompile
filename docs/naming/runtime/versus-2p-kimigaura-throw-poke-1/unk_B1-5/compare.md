# Set-and-observe: unk_B1-5 against baseline

- baseline: `..\baseline\capture.manifest.json` (SHA1 20bbefa7bafaa599f5018377de2e6d7d85f332eb), outcome ok, script ended True, q124_complete True
- poked: `capture.manifest.json` (SHA1 242ff9cc134140c051ebf3e60bac41baf622c4c4), outcome ok, script ended True, q124_complete True
- stubs: none
- pokes: `0x80023134` (0x80023134) = 00000000 (was b10062a2) from phase in_01_r2_cross_3 (frame 8550), 00000000 at the end, `0x80101F79` (0x80101F79) = 05 (was 03) from phase in_01_r2_cross_3 (frame 8550), 05 at the end
- same inputs and script

| screenshot | changed pixels | bounding box (x0,y0-x1,y1) |
|---|---|---|
| boot_intro.last (frame 5489) | 0 of 76800 (0.00%) | - |
| top_menu.first (frame 6050) | 0 of 153600 (0.00%) | - |
| top_menu.last (frame 6259) | 0 of 153600 (0.00%) | - |
| char_select.first (frame 6550) | 0 of 153600 (0.00%) | - |
| char_select.last (frame 6699) | 0 of 153600 (0.00%) | - |
| weapon_select.first (frame 6700) | 0 of 153600 (0.00%) | - |
| weapon_select.last (frame 6999) | 0 of 153600 (0.00%) | - |
| t_stage.mid (frame 7100) | 0 of 153600 (0.00%) | - |
| stage_select.first (frame 7175) | 0 of 153600 (0.00%) | - |
| stage_select.7299 (frame 7299) | 0 of 153600 (0.00%) | - |
| stage_select.7325 (frame 7325) | 0 of 153600 (0.00%) | - |
| stage_select.7355 (frame 7355) | 0 of 153600 (0.00%) | - |
| stage_select.last (frame 7449) | 0 of 153600 (0.00%) | - |
| t_load.mid (frame 7900) | 0 of 153600 (0.00%) | - |
| t_load.last (frame 8229) | 0 of 153600 (0.00%) | - |
| fight.first (frame 8230) | 0 of 153600 (0.00%) | - |
| in_00_r2_circle_3.15 (frame 8445) | 0 of 153600 (0.00%) | - |
| in_00_r2_circle_3.30 (frame 8460) | 0 of 153600 (0.00%) | - |
| in_00_r2_circle_3.60 (frame 8490) | 0 of 153600 (0.00%) | - |
| in_01_r2_cross_3.15 (frame 8565) | 0 of 153600 (0.00%) | - |
| in_01_r2_cross_3.30 (frame 8580) | 0 of 153600 (0.00%) | - |
| in_01_r2_cross_3.60 (frame 8610) | 2067 of 153600 (1.35%) | 235,94-349,135 |
| in_02_r2_circle_1.15 (frame 8685) | 0 of 153600 (0.00%) | - |
| in_02_r2_circle_1.30 (frame 8700) | 0 of 153600 (0.00%) | - |
| in_02_r2_circle_1.60 (frame 8730) | 0 of 153600 (0.00%) | - |
| in_03_r2_cross_5.15 (frame 8805) | 0 of 153600 (0.00%) | - |
| in_03_r2_cross_5.30 (frame 8820) | 0 of 153600 (0.00%) | - |
| in_03_r2_cross_5.60 (frame 8850) | 1661 of 153600 (1.08%) | 230,94-324,133 |
| in_04_r2_circle_7.15 (frame 8925) | 0 of 153600 (0.00%) | - |
| in_04_r2_circle_7.30 (frame 8940) | 0 of 153600 (0.00%) | - |
| in_04_r2_circle_7.60 (frame 8970) | 493 of 153600 (0.32%) | 84,148-132,167 |
| in_05_r2_square_3.15 (frame 9045) | 0 of 153600 (0.00%) | - |
| in_05_r2_square_3.30 (frame 9060) | 0 of 153600 (0.00%) | - |
| in_05_r2_square_3.60 (frame 9090) | 0 of 153600 (0.00%) | - |
| in_06_r2_alone.15 (frame 9165) | 0 of 153600 (0.00%) | - |
| in_06_r2_alone.30 (frame 9180) | 0 of 153600 (0.00%) | - |
| in_06_r2_alone.60 (frame 9210) | 0 of 153600 (0.00%) | - |
| in_07_r2_circle_3b.15 (frame 9285) | 0 of 153600 (0.00%) | - |
| in_07_r2_circle_3b.30 (frame 9300) | 0 of 153600 (0.00%) | - |
| in_07_r2_circle_3b.60 (frame 9330) | 426 of 153600 (0.28%) | 123,153-179,177 |
| in_08_r2_alone_b.15 (frame 9405) | 0 of 153600 (0.00%) | - |
| in_08_r2_alone_b.30 (frame 9420) | 0 of 153600 (0.00%) | - |
| in_08_r2_alone_b.60 (frame 9450) | 0 of 153600 (0.00%) | - |
| in_09_r2_cross_3b.15 (frame 9525) | 0 of 153600 (0.00%) | - |
| in_09_r2_cross_3b.30 (frame 9540) | 0 of 153600 (0.00%) | - |
| in_09_r2_cross_3b.60 (frame 9570) | 2724 of 153600 (1.77%) | 253,94-379,136 |
| last (frame 9629) | 0 of 153600 (0.00%) | - |

A poke counts only if the game kept running (both runs reached the script's end). Everything after it went in is a modified game: the differences show what the game does with the poked value, not what an unmodified run reaches.
