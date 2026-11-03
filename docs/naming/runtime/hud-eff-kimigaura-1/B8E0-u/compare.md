# Set-and-observe: B8E0-u against baseline

- scenario: Versus 2P, versus-2p-kimigaura-effects script (KANNUKI vs GENGORO, Kimigaura Coast, both pads acting every 60 frames from 8430, no KO by 12730), screenshots every 2-4 frames over the eff_DrawAnimatedQuads effects; breakpoint eff_DrawAnimatedQuads, read watches on 0x8009B8C8..0x8009B8E7, 0x8009B978..0x8009B997, 0x8009B9D8..0x8009B9F7; poked: D_8009B8E0.u (0x8009B8E4) 0x40 -> 0x80 at fight
- baseline: `..\baseline\capture.manifest.json` (SHA1 f4d5b0f58a30b73adc4a3b8429f48b66f7236675), outcome ok, script ended True, q124_complete True
- poked: `capture.manifest.json` (SHA1 3f43c19622f838de2b16e407770fafa81352e445), outcome ok, script ended True, q124_complete True
- stubs: none
- pokes: `0x8009B8E4` (0x8009B8E4) = 80 (was 40) from phase fight (frame 8230), 80 at the end
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
| fight.9219 (frame 9219) | 0 of 153600 (0.00%) | - |
| fight.9221 (frame 9221) | 0 of 153600 (0.00%) | - |
| fight.9223 (frame 9223) | 0 of 153600 (0.00%) | - |
| fight.9225 (frame 9225) | 0 of 153600 (0.00%) | - |
| fight.9227 (frame 9227) | 0 of 153600 (0.00%) | - |
| fight.9229 (frame 9229) | 0 of 153600 (0.00%) | - |
| fight.9230 (frame 9230) | 0 of 153600 (0.00%) | - |
| fight.9330 (frame 9330) | 0 of 153600 (0.00%) | - |
| fight.9430 (frame 9430) | 0 of 153600 (0.00%) | - |
| fight.9530 (frame 9530) | 0 of 153600 (0.00%) | - |
| fight.9630 (frame 9630) | 0 of 153600 (0.00%) | - |
| fight.9730 (frame 9730) | 0 of 153600 (0.00%) | - |
| fight.9830 (frame 9830) | 0 of 153600 (0.00%) | - |
| fight.9887 (frame 9887) | 0 of 153600 (0.00%) | - |
| fight.9889 (frame 9889) | 0 of 153600 (0.00%) | - |
| fight.9891 (frame 9891) | 0 of 153600 (0.00%) | - |
| fight.9893 (frame 9893) | 0 of 153600 (0.00%) | - |
| fight.9895 (frame 9895) | 0 of 153600 (0.00%) | - |
| fight.9897 (frame 9897) | 0 of 153600 (0.00%) | - |
| fight.9930 (frame 9930) | 0 of 153600 (0.00%) | - |
| fight.10030 (frame 10030) | 0 of 153600 (0.00%) | - |
| fight.10033 (frame 10033) | 0 of 153600 (0.00%) | - |
| fight.10037 (frame 10037) | 0 of 153600 (0.00%) | - |
| fight.10041 (frame 10041) | 0 of 153600 (0.00%) | - |
| fight.10045 (frame 10045) | 0 of 153600 (0.00%) | - |
| fight.10049 (frame 10049) | 0 of 153600 (0.00%) | - |
| fight.10053 (frame 10053) | 0 of 153600 (0.00%) | - |
| fight.10057 (frame 10057) | 0 of 153600 (0.00%) | - |
| fight.10070 (frame 10070) | 0 of 153600 (0.00%) | - |
| fight.10090 (frame 10090) | 0 of 153600 (0.00%) | - |
| fight.10110 (frame 10110) | 0 of 153600 (0.00%) | - |
| fight.10130 (frame 10130) | 0 of 153600 (0.00%) | - |
| fight.10150 (frame 10150) | 0 of 153600 (0.00%) | - |
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
| fight.11146 (frame 11146) | 0 of 153600 (0.00%) | - |
| fight.11148 (frame 11148) | 0 of 153600 (0.00%) | - |
| fight.11150 (frame 11150) | 0 of 153600 (0.00%) | - |
| fight.11152 (frame 11152) | 0 of 153600 (0.00%) | - |
| fight.11154 (frame 11154) | 0 of 153600 (0.00%) | - |
| fight.11156 (frame 11156) | 0 of 153600 (0.00%) | - |
| fight.11230 (frame 11230) | 0 of 153600 (0.00%) | - |
| fight.11330 (frame 11330) | 0 of 153600 (0.00%) | - |
| fight.11430 (frame 11430) | 0 of 153600 (0.00%) | - |
| fight.11530 (frame 11530) | 0 of 153600 (0.00%) | - |
| fight.11630 (frame 11630) | 0 of 153600 (0.00%) | - |
| fight.11730 (frame 11730) | 0 of 153600 (0.00%) | - |
| fight.11830 (frame 11830) | 0 of 153600 (0.00%) | - |
| fight.11930 (frame 11930) | 0 of 153600 (0.00%) | - |
| fight.12030 (frame 12030) | 0 of 153600 (0.00%) | - |
| fight.12130 (frame 12130) | 0 of 153600 (0.00%) | - |
| fight.12230 (frame 12230) | 0 of 153600 (0.00%) | - |
| fight.12330 (frame 12330) | 0 of 153600 (0.00%) | - |
| fight.12430 (frame 12430) | 0 of 153600 (0.00%) | - |
| fight.12530 (frame 12530) | 0 of 153600 (0.00%) | - |
| fight.12630 (frame 12630) | 0 of 153600 (0.00%) | - |
| fight.last (frame 12729) | 0 of 153600 (0.00%) | - |

A poke counts only if the game kept running (both runs reached the script's end). Everything after it went in is a modified game: the differences show what the game does with the poked value, not what an unmodified run reaches.
