# Set-and-observe: B164-0-1-y against baseline

- scenario: Vs. CPU Mode, versus-cpu-first-fight script cut at 9700: the Pause Menu after the first fight's KO (lost); poked: D_8009B164[0][1].y (0x8009B16E) 0 -> 60 at t_pause
- baseline: `..\baseline\capture.manifest.json` (SHA1 426f637f52ea934b5254d5d5728e6a02b5e936da), outcome ok, script ended True, q124_complete True
- poked: `capture.manifest.json` (SHA1 d9203eb1afd6f5b8072dbef494afee6c11dc73e2), outcome ok, script ended True, q124_complete True
- stubs: none
- pokes: `0x8009B16E` (0x8009B16E) = 3c00 (was 0000) from phase t_pause (frame 9300), 3c00 at the end
- same inputs and script

| screenshot | changed pixels | bounding box (x0,y0-x1,y1) |
|---|---|---|
| boot_intro.last (frame 5489) | 0 of 76800 (0.00%) | - |
| top_menu.first (frame 6050) | 0 of 153600 (0.00%) | - |
| top_menu.last (frame 6199) | 0 of 153600 (0.00%) | - |
| char_select.first (frame 6500) | 0 of 153600 (0.00%) | - |
| char_select.last (frame 6799) | 0 of 153600 (0.00%) | - |
| weapon_select.first (frame 6900) | 0 of 153600 (0.00%) | - |
| weapon_select.last (frame 7399) | 0 of 153600 (0.00%) | - |
| t_load.mid (frame 8000) | 0 of 153600 (0.00%) | - |
| fight.first (frame 8350) | 0 of 153600 (0.00%) | - |
| fight.last (frame 8649) | 0 of 153600 (0.00%) | - |
| after_ko.first (frame 8750) | 0 of 153600 (0.00%) | - |
| after_ko.last (frame 9299) | 0 of 153600 (0.00%) | - |
| pause_menu.first (frame 9400) | 88 of 153600 (0.06%) | 437,87-452,152 |
| pause_menu.9600 (frame 9600) | 88 of 153600 (0.06%) | 437,87-452,152 |
| pause_menu.9699 (frame 9699) | 88 of 153600 (0.06%) | 437,87-452,152 |

A poke counts only if the game kept running (both runs reached the script's end). Everything after it went in is a modified game: the differences show what the game does with the poked value, not what an unmodified run reaches.
