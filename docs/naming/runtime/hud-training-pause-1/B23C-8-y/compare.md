# Set-and-observe: B23C-8-y against baseline

- scenario: Training Mode, training-ninja script (vs the Ninja) cut at 9400: the Pause Menu with the Ninja settings; poked: D_8009B23C[8].y (0x8009B27E) 94 -> 154 at t_load
- baseline: `..\baseline\capture.manifest.json` (SHA1 38722dd4274968bb734148f4c1deda4b1677e7e5), outcome ok, script ended True, q124_complete True
- poked: `capture.manifest.json` (SHA1 d8863370f73f93282734bf5337e4e872ca5001a3), outcome ok, script ended True, q124_complete True
- stubs: none
- pokes: `0x8009B27E` (0x8009B27E) = 9a00 (was 5e00) from phase t_load (frame 8500), 9a00 at the end
- same inputs and script

| screenshot | changed pixels | bounding box (x0,y0-x1,y1) |
|---|---|---|
| boot_intro.last (frame 5489) | 0 of 76800 (0.00%) | - |
| top_menu.first (frame 6050) | 0 of 153600 (0.00%) | - |
| top_menu.last (frame 6199) | 0 of 153600 (0.00%) | - |
| training_menu.first (frame 6300) | 0 of 153600 (0.00%) | - |
| training_menu.last (frame 6999) | 0 of 153600 (0.00%) | - |
| char_select.first (frame 7050) | 0 of 153600 (0.00%) | - |
| char_select.last (frame 7399) | 0 of 153600 (0.00%) | - |
| weapon_select.first (frame 7450) | 0 of 153600 (0.00%) | - |
| weapon_select.mid (frame 7999) | 0 of 153600 (0.00%) | - |
| weapon_select.enemy (frame 8350) | 0 of 153600 (0.00%) | - |
| weapon_select.last (frame 8499) | 0 of 153600 (0.00%) | - |
| t_load.mid (frame 8650) | 0 of 153600 (0.00%) | - |
| pause_menu.first (frame 9300) | 227 of 153600 (0.15%) | 180,94-191,165 |
| pause_menu.9350 (frame 9350) | 227 of 153600 (0.15%) | 180,94-191,165 |
| pause_menu.last (frame 9399) | 227 of 153600 (0.15%) | 180,94-191,165 |

A poke counts only if the game kept running (both runs reached the script's end). Everything after it went in is a modified game: the differences show what the game does with the poked value, not what an unmodified run reaches.
