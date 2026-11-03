# Set-and-observe: B9E0-u against baseline

- scenario: Vs. CPU, versus-cpu-tatsumi script with the Vs. CPU row's view on POV (RIGHT at 6140; P1 TATSUMI: 0 KILLS against a first opponent to 11600, a title splash at 12000, 1 KILL against a second opponent from 12400; opp1 / opp2 / opp3 are phase labels, not opponents), screenshots every 2-4 frames over the eff_DrawAnimatedQuads effects; breakpoint eff_DrawAnimatedQuads, read watches on 0x8009B8C8..0x8009B8E7, 0x8009B978..0x8009B997, 0x8009B9D8..0x8009B9F7; poked: D_8009B9E0.u (0x8009B9E4) 0x40 -> 0x80 at opp1
- baseline: `..\baseline\capture.manifest.json` (SHA1 edd7453e64c1f7bb3af2a841ef022ac6918997cc), outcome ok, script ended True, q124_complete True
- poked: `capture.manifest.json` (SHA1 161ec8f794e59b40e67e90c2240b6820744afc1f), outcome ok, script ended True, q124_complete True
- stubs: none
- pokes: `0x8009B9E4` (0x8009B9E4) = 80 (was 40) from phase opp1 (frame 8350), 80 at the end
- same inputs and script

| screenshot | changed pixels | bounding box (x0,y0-x1,y1) |
|---|---|---|
| boot_intro.last (frame 5489) | 0 of 76800 (0.00%) | - |
| top_menu.first (frame 6050) | 0 of 153600 (0.00%) | - |
| top_menu.pov (frame 6170) | 0 of 153600 (0.00%) | - |
| char_select.pick (frame 6599) | 0 of 153600 (0.00%) | - |
| weapon_select.first (frame 6900) | 0 of 153600 (0.00%) | - |
| t_load.mid (frame 8000) | 0 of 153600 (0.00%) | - |
| opp1.first (frame 8350) | 0 of 153600 (0.00%) | - |
| opp1.8600 (frame 8600) | 0 of 153600 (0.00%) | - |
| opp1.8820 (frame 8820) | 0 of 153600 (0.00%) | - |
| opp1.8822 (frame 8822) | 0 of 153600 (0.00%) | - |
| opp1.8824 (frame 8824) | 0 of 153600 (0.00%) | - |
| opp1.8826 (frame 8826) | 0 of 153600 (0.00%) | - |
| opp1.8828 (frame 8828) | 0 of 153600 (0.00%) | - |
| opp1.8830 (frame 8830) | 0 of 153600 (0.00%) | - |
| opp1.8832 (frame 8832) | 0 of 153600 (0.00%) | - |
| opp1.8834 (frame 8834) | 0 of 153600 (0.00%) | - |
| opp1.last (frame 8850) | 0 of 153600 (0.00%) | - |
| opp2.9010 (frame 9010) | 0 of 153600 (0.00%) | - |
| opp2.9012 (frame 9012) | 0 of 153600 (0.00%) | - |
| opp2.9014 (frame 9014) | 0 of 153600 (0.00%) | - |
| opp2.9016 (frame 9016) | 0 of 153600 (0.00%) | - |
| opp2.9018 (frame 9018) | 0 of 153600 (0.00%) | - |
| opp2.9020 (frame 9020) | 0 of 153600 (0.00%) | - |
| opp2.9022 (frame 9022) | 0 of 153600 (0.00%) | - |
| opp2.9200 (frame 9200) | 0 of 153600 (0.00%) | - |
| opp2.9569 (frame 9569) | 0 of 153600 (0.00%) | - |
| opp2.9571 (frame 9571) | 0 of 153600 (0.00%) | - |
| opp2.9573 (frame 9573) | 0 of 153600 (0.00%) | - |
| opp2.9575 (frame 9575) | 0 of 153600 (0.00%) | - |
| opp2.9577 (frame 9577) | 0 of 153600 (0.00%) | - |
| opp2.9579 (frame 9579) | 0 of 153600 (0.00%) | - |
| opp2.9581 (frame 9581) | 0 of 153600 (0.00%) | - |
| opp2.9583 (frame 9583) | 0 of 153600 (0.00%) | - |
| opp2.9584 (frame 9584) | 0 of 153600 (0.00%) | - |
| opp2.10000 (frame 10000) | 0 of 153600 (0.00%) | - |
| opp2.last (frame 10540) | 0 of 153600 (0.00%) | - |
| opp3.10800 (frame 10800) | 0 of 153600 (0.00%) | - |
| opp3.11200 (frame 11200) | 0 of 153600 (0.00%) | - |
| opp3.11351 (frame 11351) | 0 of 153600 (0.00%) | - |
| opp3.11353 (frame 11353) | 0 of 153600 (0.00%) | - |
| opp3.11355 (frame 11355) | 0 of 153600 (0.00%) | - |
| opp3.11357 (frame 11357) | 0 of 153600 (0.00%) | - |
| opp3.11359 (frame 11359) | 0 of 153600 (0.00%) | - |
| opp3.11361 (frame 11361) | 0 of 153600 (0.00%) | - |
| opp3.11363 (frame 11363) | 12161 of 153600 (7.92%) | 365,148-608,239 |
| opp3.11364 (frame 11364) | 12161 of 153600 (7.92%) | 365,148-608,239 |
| opp3.11600 (frame 11600) | 0 of 153600 (0.00%) | - |
| opp3.12000 (frame 12000) | 0 of 153600 (0.00%) | - |
| opp3.12400 (frame 12400) | 0 of 153600 (0.00%) | - |
| opp3.12405 (frame 12405) | 0 of 153600 (0.00%) | - |
| opp3.12409 (frame 12409) | 0 of 153600 (0.00%) | - |
| opp3.12413 (frame 12413) | 0 of 153600 (0.00%) | - |
| opp3.12417 (frame 12417) | 0 of 153600 (0.00%) | - |
| opp3.12421 (frame 12421) | 0 of 153600 (0.00%) | - |
| opp3.12425 (frame 12425) | 0 of 153600 (0.00%) | - |
| opp3.last (frame 12999) | 0 of 153600 (0.00%) | - |

A poke counts only if the game kept running (both runs reached the script's end). Everything after it went in is a modified game: the differences show what the game does with the poked value, not what an unmodified run reaches.
