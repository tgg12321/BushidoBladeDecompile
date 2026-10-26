# Set-and-observe: slot17 against baseline

- scenario: Group Battle 3 entries, group-battle-hitmap-1 script cut at 10500 (P1 RIGHT + TRIANGLE/CIRCLE/SQUARE every 100 frames from 9600, P2 idle; IPPON at 10371); poked run: every eff_StartRecycledCommandWithOrangeLight start takes slot 17 (command 0x11) instead of slot 16 (command 0x10)
- baseline: `..\baseline\capture.manifest.json` (SHA1 b872469c9b25c7fa56b1576bcbac18be65270568), outcome ok, script ended True, q124_complete True
- poked: `capture.manifest.json` (SHA1 ceb9feb992d22f4995dc4f740029ddcc9549e223), outcome ok, script ended True, q124_complete True
- stubs: none
- pokes: `0x800617F8` (0x800617F8) = 1600001001006424 (was 0b00401000000000) from phase t_bout1 (frame 9350), 1600001001006424 at the end
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
| bout1.10380 (frame 10380) | 0 of 153600 (0.00%) | - |
| bout1.10384 (frame 10384) | 0 of 153600 (0.00%) | - |
| bout1.10388 (frame 10388) | 0 of 153600 (0.00%) | - |
| bout1.10392 (frame 10392) | 0 of 153600 (0.00%) | - |
| bout1.10396 (frame 10396) | 0 of 153600 (0.00%) | - |
| bout1.ippon (frame 10400) | 0 of 153600 (0.00%) | - |
| bout1.10450 (frame 10450) | 0 of 153600 (0.00%) | - |

A poke counts only if the game kept running (both runs reached the script's end). Everything after it went in is a modified game: the differences show what the game does with the poked value, not what an unmodified run reaches.
