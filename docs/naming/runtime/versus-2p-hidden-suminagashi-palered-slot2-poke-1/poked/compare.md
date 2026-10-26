# Set-and-observe: poked against baseline

- scenario: Versus 2P with hidden SUMINAGASHI (unlock poke 0x80106A50 at top_menu, both runs), lib-versus-2p-hidden-suminagashi-1 script cut at 9450 (two pale-red starts, 8949 and 9185); poked run: eff_StartRecycledVectorCommandAndJitteredBillboardsWithPaleRedLight 0x80061B08 poked to b 0x80061B80 / addiu v1, s0, 1 at t_load so every start takes the second slot (status 21, command 0x15)
- baseline: `..\baseline\capture.manifest.json` (SHA1 25b7f85738c0da8946356b94d06339b7d8951803), outcome ok, script ended True, q124_complete True
- poked: `capture.manifest.json` (SHA1 be18c33798fa9f59142c4260999fcbe5893b67a7), outcome ok, script ended True, q124_complete True
- stubs: none
- pokes in both runs (baseline's too, not compared): `0x80106A50` (0x80106A50) = fff33f05 (was 07700000) from phase top_menu (frame 6050), fff33f05 at the end
- pokes: `0x80061B08` (0x80061B08) = 1d00001001000326 (was 0b00401000000000) from phase t_load (frame 8040), 1d00001001000326 at the end
- same inputs and script

| screenshot | changed pixels | bounding box (x0,y0-x1,y1) |
|---|---|---|
| boot_intro.last (frame 5489) | 0 of 76800 (0.00%) | - |
| top_menu.first (frame 6050) | 0 of 153600 (0.00%) | - |
| top_menu.last (frame 6259) | 0 of 153600 (0.00%) | - |
| char_select.first (frame 6550) | 0 of 153600 (0.00%) | - |
| char_select.pick (frame 7285) | 0 of 153600 (0.00%) | - |
| char_select.last (frame 7374) | 0 of 153600 (0.00%) | - |
| weapon_select.first (frame 7380) | 0 of 153600 (0.00%) | - |
| weapon_select.7650 (frame 7650) | 0 of 153600 (0.00%) | - |
| weapon_select.last (frame 7679) | 0 of 153600 (0.00%) | - |
| t_stage.mid (frame 7780) | 0 of 153600 (0.00%) | - |
| stage_select.first (frame 7850) | 0 of 153600 (0.00%) | - |
| stage_select.last (frame 8039) | 0 of 153600 (0.00%) | - |
| t_load.mid (frame 8400) | 0 of 153600 (0.00%) | - |
| t_load.last (frame 8799) | 0 of 153600 (0.00%) | - |
| fight.first (frame 8805) | 0 of 153600 (0.00%) | - |
| fight.8948 (frame 8948) | 0 of 153600 (0.00%) | - |
| fight.8950 (frame 8950) | 0 of 153600 (0.00%) | - |
| fight.8952 (frame 8952) | 0 of 153600 (0.00%) | - |
| fight.8958 (frame 8958) | 0 of 153600 (0.00%) | - |
| fight.8970 (frame 8970) | 0 of 153600 (0.00%) | - |
| fight.8985 (frame 8985) | 0 of 153600 (0.00%) | - |
| fight.9000 (frame 9000) | 0 of 153600 (0.00%) | - |
| fight.9020 (frame 9020) | 0 of 153600 (0.00%) | - |
| fight.9040 (frame 9040) | 0 of 153600 (0.00%) | - |
| fight.9050 (frame 9050) | 0 of 153600 (0.00%) | - |
| fight.9070 (frame 9070) | 0 of 153600 (0.00%) | - |
| fight.9100 (frame 9100) | 0 of 153600 (0.00%) | - |
| fight.9184 (frame 9184) | 0 of 153600 (0.00%) | - |
| fight.9188 (frame 9188) | 0 of 153600 (0.00%) | - |
| fight.9194 (frame 9194) | 0 of 153600 (0.00%) | - |
| fight.9206 (frame 9206) | 0 of 153600 (0.00%) | - |
| fight.9221 (frame 9221) | 0 of 153600 (0.00%) | - |
| fight.9240 (frame 9240) | 0 of 153600 (0.00%) | - |
| fight.9260 (frame 9260) | 0 of 153600 (0.00%) | - |
| fight.9280 (frame 9280) | 0 of 153600 (0.00%) | - |
| fight.9290 (frame 9290) | 0 of 153600 (0.00%) | - |
| fight.9400 (frame 9400) | 0 of 153600 (0.00%) | - |

A poke counts only if the game kept running (both runs reached the script's end). Everything after it went in is a modified game: the differences show what the game does with the poked value, not what an unmodified run reaches.
