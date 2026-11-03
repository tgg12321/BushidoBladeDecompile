# Set-and-observe: B1EC-3-y against baseline

- scenario: Vs. CPU Mode, versus-cpu-first-fight script cut at 9700: the Pause Menu after the first fight's KO (lost); poked: D_8009B1EC[3].y (0x8009B206) 34 -> 94 at t_pause
- baseline: `..\baseline\capture.manifest.json` (SHA1 426f637f52ea934b5254d5d5728e6a02b5e936da), outcome ok, script ended True, q124_complete True
- poked: `capture.manifest.json` (SHA1 23b596fbd4315396a95261bd13f3d0b222ece373), outcome ok, script ended True, q124_complete True
- stubs: none
- pokes: `0x8009B206` (0x8009B206) = 5e00 (was 2200) from phase t_pause (frame 9300), 5e00 at the end
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
| pause_menu.first (frame 9400) | 576 of 153600 (0.38%) | 395,137-436,206 |
| pause_menu.9600 (frame 9600) | 576 of 153600 (0.38%) | 395,137-436,206 |
| pause_menu.9699 (frame 9699) | 576 of 153600 (0.38%) | 395,137-436,206 |

A poke counts only if the game kept running (both runs reached the script's end). Everything after it went in is a modified game: the differences show what the game does with the poked value, not what an unmodified run reaches.
