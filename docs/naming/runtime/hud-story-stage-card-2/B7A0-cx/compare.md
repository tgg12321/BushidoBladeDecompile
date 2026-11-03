# Set-and-observe: B7A0-cx against baseline

- scenario: Story Mode, story-concede script (KANNUKI, default weapon) cut at 8790: the first fight's stage card before the fight; read watch on D_8009B7A0 (0x8009B7A0, 12 bytes); poked: D_8009B7A0.cx (0x8009B7A4) 0x3C0 -> 0x3D0 at t_stage
- baseline: `..\baseline\capture.manifest.json` (SHA1 3e6b64279c565c61e21b6c71013851c3fde6a20b), outcome ok, script ended True, q124_complete True
- poked: `capture.manifest.json` (SHA1 4df5a8b718e790c7a64239519623c39de055ec66), outcome ok, script ended True, q124_complete True
- stubs: none
- pokes: `0x8009B7A4` (0x8009B7A4) = d003 (was c003) from phase t_stage (frame 7900), d003 at the end
- same inputs and script

| screenshot | changed pixels | bounding box (x0,y0-x1,y1) |
|---|---|---|
| boot_intro.last (frame 5489) | 0 of 76800 (0.00%) | - |
| top_menu.first (frame 6060) | 0 of 153600 (0.00%) | - |
| char_select.first (frame 6950) | 0 of 153600 (0.00%) | - |
| stage_card.first (frame 8360) | 1512 of 153600 (0.98%) | 319,199-574,214 |
| stage_card.8450 (frame 8450) | 1512 of 153600 (0.98%) | 319,199-574,214 |
| stage_card.8600 (frame 8600) | 1512 of 153600 (0.98%) | 319,199-574,214 |
| stage_card.8750 (frame 8750) | 1512 of 153600 (0.98%) | 319,199-574,214 |

A poke counts only if the game kept running (both runs reached the script's end). Everything after it went in is a modified game: the differences show what the game does with the poked value, not what an unmodified run reaches.
