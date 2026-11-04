# Set-and-observe: slot2-B980-u against slot2

- scenario: Versus 2P on Kimigaura Coast, versus-2p-kimigaura-rose-slot2-poke script (versus-2p-kimigaura-effects cut at 11300; two rose-light starts, 9885 and 11144; phase mark mid at 9990), screenshots over both starts and the mode-5 effect (10070-10150); eff_StartRecycledCommandWithRoseLight 0x80061280 poked at t_load so every start takes the second slot (command 0xA, mode 4); breakpoint eff_DrawAnimatedQuads, read watches on 0x8009B8C8..0x8009B8E7, 0x8009B978..0x8009B997, 0x8009B9D8..0x8009B9F7; poked: g_eff_growing_quad_and_quad_pair_second_early_tex_rec (D_8009B980).u (0x8009B984) 0x00 -> 0x80 at fight
- baseline: `..\slot2\capture.manifest.json` (SHA1 a686151b45abfb6f194f90402ed935f406c371dd), outcome ok, script ended True, q124_complete True
- poked: `capture.manifest.json` (SHA1 7be6a8dc1a13d65302422fea581f13f24ced6dae), outcome ok, script ended True, q124_complete True
- stubs: none
- pokes in both runs (baseline's too, not compared): `0x80061280` (0x80061280) = 1600001001006424 (was 0b00401000000000) from phase t_load (frame 7450), 1600001001006424 at the end
- pokes: `0x8009B984` (0x8009B984) = 80 (was 00) from phase fight (frame 8230), 80 at the end
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
| fight.8330 (frame 8330) | 0 of 153600 (0.00%) | - |
| fight.8430 (frame 8430) | 0 of 153600 (0.00%) | - |
| fight.8530 (frame 8530) | 0 of 153600 (0.00%) | - |
| fight.8630 (frame 8630) | 0 of 153600 (0.00%) | - |
| fight.8730 (frame 8730) | 0 of 153600 (0.00%) | - |
| fight.8830 (frame 8830) | 0 of 153600 (0.00%) | - |
| fight.8930 (frame 8930) | 0 of 153600 (0.00%) | - |
| fight.9030 (frame 9030) | 0 of 153600 (0.00%) | - |
| fight.9130 (frame 9130) | 0 of 153600 (0.00%) | - |
| fight.9230 (frame 9230) | 1198 of 153600 (0.78%) | 346,62-401,89 |
| fight.9330 (frame 9330) | 0 of 153600 (0.00%) | - |
| fight.9430 (frame 9430) | 0 of 153600 (0.00%) | - |
| fight.9530 (frame 9530) | 0 of 153600 (0.00%) | - |
| fight.9630 (frame 9630) | 0 of 153600 (0.00%) | - |
| fight.9730 (frame 9730) | 0 of 153600 (0.00%) | - |
| fight.9830 (frame 9830) | 0 of 153600 (0.00%) | - |
| fight.9882 (frame 9882) | 0 of 153600 (0.00%) | - |
| fight.9884 (frame 9884) | 0 of 153600 (0.00%) | - |
| fight.9886 (frame 9886) | 0 of 153600 (0.00%) | - |
| fight.9888 (frame 9888) | 0 of 153600 (0.00%) | - |
| fight.9890 (frame 9890) | 0 of 153600 (0.00%) | - |
| fight.9892 (frame 9892) | 482 of 153600 (0.31%) | 243,101-278,118 |
| fight.9894 (frame 9894) | 884 of 153600 (0.58%) | 240,98-289,121 |
| fight.9898 (frame 9898) | 2442 of 153600 (1.59%) | 236,90-315,129 |
| fight.9904 (frame 9904) | 0 of 153600 (0.00%) | - |
| fight.9930 (frame 9930) | 0 of 153600 (0.00%) | - |
| fight.10030 (frame 10030) | 0 of 153600 (0.00%) | - |
| mid.10070 (frame 10070) | 0 of 153600 (0.00%) | - |
| mid.10090 (frame 10090) | 0 of 153600 (0.00%) | - |
| mid.10110 (frame 10110) | 0 of 153600 (0.00%) | - |
| fight.10130 (frame 10130) | 0 of 153600 (0.00%) | - |
| mid.10150 (frame 10150) | 0 of 153600 (0.00%) | - |
| fight.10230 (frame 10230) | 0 of 153600 (0.00%) | - |
| fight.10330 (frame 10330) | 0 of 153600 (0.00%) | - |
| fight.10430 (frame 10430) | 0 of 153600 (0.00%) | - |
| fight.10530 (frame 10530) | 0 of 153600 (0.00%) | - |
| fight.10630 (frame 10630) | 0 of 153600 (0.00%) | - |
| fight.10730 (frame 10730) | 0 of 153600 (0.00%) | - |
| fight.10830 (frame 10830) | 0 of 153600 (0.00%) | - |
| fight.10930 (frame 10930) | 0 of 153600 (0.00%) | - |
| fight.11030 (frame 11030) | 0 of 153600 (0.00%) | - |
| fight.11130 (frame 11130) | 0 of 153600 (0.00%) | - |
| fight.11141 (frame 11141) | 0 of 153600 (0.00%) | - |
| fight.11143 (frame 11143) | 0 of 153600 (0.00%) | - |
| fight.11145 (frame 11145) | 0 of 153600 (0.00%) | - |
| fight.11147 (frame 11147) | 0 of 153600 (0.00%) | - |
| fight.11149 (frame 11149) | 0 of 153600 (0.00%) | - |
| fight.11151 (frame 11151) | 487 of 153600 (0.32%) | 252,94-287,111 |
| fight.11153 (frame 11153) | 914 of 153600 (0.60%) | 250,91-299,114 |
| fight.11157 (frame 11157) | 2457 of 153600 (1.60%) | 258,83-337,122 |
| fight.11163 (frame 11163) | 0 of 153600 (0.00%) | - |
| fight.11230 (frame 11230) | 0 of 153600 (0.00%) | - |

A poke counts only if the game kept running (both runs reached the script's end). Everything after it went in is a modified game: the differences show what the game does with the poked value, not what an unmodified run reaches.
