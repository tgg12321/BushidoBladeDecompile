# Set-and-observe: B990-u against baseline

- scenario: Vs. CPU, versus-cpu-tatsumi script with RIGHT at 6070 on the top menu's Story row (Vs. CPU row left on Side View; P1 TATSUMI in side view: the fight shots 8350-12400 show 0 KILLS against one opponent, the run ends on the Rematch menu at 12999; opp1 / opp2 / opp3 are phase labels, not opponents), screenshots every 2 frames over the eff_DrawAnimatedQuads effects; breakpoint eff_DrawAnimatedQuads, read watches on 0x8009B8C8..0x8009B8E7, 0x8009B978..0x8009B997, 0x8009B9D8..0x8009B9F7; poked: D_8009B990.u (0x8009B994) 0x40 -> 0x80 at opp1
- baseline: `..\baseline\capture.manifest.json` (SHA1 53bd18b6770a7edea8da9b493ba835c3c64c2ba8), outcome ok, script ended True, q124_complete True
- poked: `capture.manifest.json` (SHA1 e0e8e962d5995e4d3bd10843daf13ad5dfc9c18e), outcome ok, script ended True, q124_complete True
- stubs: none
- pokes: `0x8009B994` (0x8009B994) = 80 (was 40) from phase opp1 (frame 8350), 80 at the end
- same inputs and script

| screenshot | changed pixels | bounding box (x0,y0-x1,y1) |
|---|---|---|
| boot_intro.last (frame 5489) | 0 of 76800 (0.00%) | - |
| top_menu.first (frame 6050) | 0 of 153600 (0.00%) | - |
| top_menu.story_pov (frame 6079) | 0 of 153600 (0.00%) | - |
| top_menu.vs_cpu_side (frame 6120) | 0 of 153600 (0.00%) | - |
| char_select.pick (frame 6599) | 0 of 153600 (0.00%) | - |
| weapon_select.first (frame 6900) | 0 of 153600 (0.00%) | - |
| t_load.mid (frame 8000) | 0 of 153600 (0.00%) | - |
| opp1.first (frame 8350) | 0 of 153600 (0.00%) | - |
| opp1.8600 (frame 8600) | 0 of 153600 (0.00%) | - |
| opp1.last (frame 8850) | 0 of 153600 (0.00%) | - |
| opp2.9128 (frame 9128) | 0 of 153600 (0.00%) | - |
| opp2.9130 (frame 9130) | 0 of 153600 (0.00%) | - |
| opp2.9132 (frame 9132) | 0 of 153600 (0.00%) | - |
| opp2.9134 (frame 9134) | 0 of 153600 (0.00%) | - |
| opp2.9136 (frame 9136) | 0 of 153600 (0.00%) | - |
| opp2.9138 (frame 9138) | 0 of 153600 (0.00%) | - |
| opp2.9140 (frame 9140) | 1303 of 153600 (0.85%) | 381,71-416,127 |
| opp2.9200 (frame 9200) | 0 of 153600 (0.00%) | - |
| opp2.9208 (frame 9208) | 0 of 153600 (0.00%) | - |
| opp2.9210 (frame 9210) | 0 of 153600 (0.00%) | - |
| opp2.9212 (frame 9212) | 0 of 153600 (0.00%) | - |
| opp2.9214 (frame 9214) | 0 of 153600 (0.00%) | - |
| opp2.9216 (frame 9216) | 0 of 153600 (0.00%) | - |
| opp2.9218 (frame 9218) | 0 of 153600 (0.00%) | - |
| opp2.9220 (frame 9220) | 1301 of 153600 (0.85%) | 398,71-433,127 |
| opp2.10000 (frame 10000) | 0 of 153600 (0.00%) | - |
| opp2.last (frame 10540) | 0 of 153600 (0.00%) | - |
| opp3.10586 (frame 10586) | 0 of 153600 (0.00%) | - |
| opp3.10588 (frame 10588) | 0 of 153600 (0.00%) | - |
| opp3.10590 (frame 10590) | 0 of 153600 (0.00%) | - |
| opp3.10592 (frame 10592) | 0 of 153600 (0.00%) | - |
| opp3.10594 (frame 10594) | 0 of 153600 (0.00%) | - |
| opp3.10596 (frame 10596) | 0 of 153600 (0.00%) | - |
| opp3.10598 (frame 10598) | 1220 of 153600 (0.79%) | 230,89-265,143 |
| opp3.10622 (frame 10622) | 0 of 153600 (0.00%) | - |
| opp3.10624 (frame 10624) | 0 of 153600 (0.00%) | - |
| opp3.10626 (frame 10626) | 0 of 153600 (0.00%) | - |
| opp3.10628 (frame 10628) | 539 of 153600 (0.35%) | 195,70-208,130 |
| opp3.10630 (frame 10630) | 858 of 153600 (0.56%) | 198,70-219,130 |
| opp3.10632 (frame 10632) | 1139 of 153600 (0.74%) | 202,71-231,129 |
| opp3.10634 (frame 10634) | 1229 of 153600 (0.80%) | 225,73-260,127 |
| opp3.10800 (frame 10800) | 0 of 153600 (0.00%) | - |
| opp3.11200 (frame 11200) | 0 of 153600 (0.00%) | - |
| opp3.11600 (frame 11600) | 0 of 153600 (0.00%) | - |
| opp3.12000 (frame 12000) | 0 of 153600 (0.00%) | - |
| opp3.12208 (frame 12208) | 0 of 153600 (0.00%) | - |
| opp3.12210 (frame 12210) | 0 of 153600 (0.00%) | - |
| opp3.12212 (frame 12212) | 0 of 153600 (0.00%) | - |
| opp3.12214 (frame 12214) | 0 of 153600 (0.00%) | - |
| opp3.12216 (frame 12216) | 0 of 153600 (0.00%) | - |
| opp3.12218 (frame 12218) | 0 of 153600 (0.00%) | - |
| opp3.12220 (frame 12220) | 1305 of 153600 (0.85%) | 381,71-416,127 |
| opp3.12400 (frame 12400) | 0 of 153600 (0.00%) | - |
| opp3.last (frame 12999) | 0 of 153600 (0.00%) | - |

A poke counts only if the game kept running (both runs reached the script's end). Everything after it went in is a modified game: the differences show what the game does with the poked value, not what an unmodified run reaches.
