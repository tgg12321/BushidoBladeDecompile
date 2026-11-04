# Set-and-observe: slot17-B9E8-u against slot17

- scenario: Group Battle 3 entries, group-battle-orange-slot2-poke script (group-battle-3-0 cut at 10500; P1 RIGHT + TRIANGLE/CIRCLE/SQUARE every 100 frames from 9600, P2 idle; IPPON at 10371): bout 1's IPPON orange effect with group-battle-orange-slot2-poke-1's slot17 poke (0x800617F8 -> b 0x80061854 / addiu a0, v1, 1 at t_bout1: every start takes the second slot, command 0x11); breakpoint eff_DrawAnimatedQuads, read watches on the TexRec records 0x8009B8C8..0x8009B8E7, 0x8009B978..0x8009B997, 0x8009B9D8..0x8009B9F7; poked: D_8009B9E8.u (0x8009B9EC) -> 0x80 at t_bout1
- baseline: `..\slot17\capture.manifest.json` (SHA1 cc9e8bb1f0a46d067914edaad6e479d3be66b77f), outcome ok, script ended True, q124_complete True
- poked: `capture.manifest.json` (SHA1 83701cf126875c9dbbaa5071890f2934524ea9ff), outcome ok, script ended True, q124_complete True
- stubs: none
- pokes in both runs (baseline's too, not compared): `0x800617F8` (0x800617F8) = 1600001001006424 (was 0b00401000000000) from phase t_bout1 (frame 9350), 1600001001006424 at the end
- pokes: `0x8009B9EC` (0x8009B9EC) = 80 (was 40) from phase t_bout1 (frame 9350), 80 at the end
- same inputs and script

| screenshot | changed pixels | bounding box (x0,y0-x1,y1) |
|---|---|---|
| boot_intro.last (frame 5489) | 0 of 76800 (0.00%) | - |
| top_menu.last (frame 6300) | 0 of 153600 (0.00%) | - |
| group_settings.first (frame 6600) | 0 of 153600 (0.00%) | - |
| school_select.first (frame 6700) | 0 of 153600 (0.00%) | - |
| school_select.mid (frame 6800) | 0 of 153600 (0.00%) | - |
| char_select.first (frame 7000) | 0 of 153600 (0.00%) | - |
| char_select.pick1 (frame 7300) | 0 of 153600 (0.00%) | - |
| char_select.pick2 (frame 7500) | 0 of 153600 (0.00%) | - |
| weapon_select.first (frame 7700) | 0 of 153600 (0.00%) | - |
| weapon_select.8000 (frame 8000) | 0 of 153600 (0.00%) | - |
| weapon_select.8300 (frame 8300) | 0 of 153600 (0.00%) | - |
| weapon_select.last (frame 8500) | 0 of 153600 (0.00%) | - |
| roster.first (frame 8700) | 0 of 153600 (0.00%) | - |
| roster.mid (frame 9000) | 0 of 153600 (0.00%) | - |
| bout1.first (frame 9400) | 0 of 153600 (0.00%) | - |
| bout1.10368 (frame 10368) | 0 of 153600 (0.00%) | - |
| bout1.10370 (frame 10370) | 0 of 153600 (0.00%) | - |
| bout1.10372 (frame 10372) | 0 of 153600 (0.00%) | - |
| bout1.10374 (frame 10374) | 0 of 153600 (0.00%) | - |
| bout1.10376 (frame 10376) | 0 of 153600 (0.00%) | - |
| bout1.10378 (frame 10378) | 0 of 153600 (0.00%) | - |
| bout1.10380 (frame 10380) | 91 of 153600 (0.06%) | 341,115-357,121 |
| bout1.10384 (frame 10384) | 827 of 153600 (0.54%) | 328,106-382,131 |
| bout1.10388 (frame 10388) | 0 of 153600 (0.00%) | - |
| bout1.10392 (frame 10392) | 0 of 153600 (0.00%) | - |
| bout1.10396 (frame 10396) | 0 of 153600 (0.00%) | - |
| bout1.ippon (frame 10400) | 0 of 153600 (0.00%) | - |
| bout1.10450 (frame 10450) | 0 of 153600 (0.00%) | - |

A poke counts only if the game kept running (both runs reached the script's end). Everything after it went in is a modified game: the differences show what the game does with the poked value, not what an unmodified run reaches.
