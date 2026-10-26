# Set-and-observe: poked against baseline

- scenario: Versus 2P on Kimigaura Coast, lib-versus-2p-kimigaura-effects-1 script cut at 11300 (two code-3 rose-light starts, 9885 and 11144); poked run: eff_StartRecycledCommandWithRoseLight 0x80061280 poked to b 0x800612DC / addiu a0, v1, 1 at t_load so every start takes the second slot (status 10, command 0xA)
- baseline: `..\baseline\capture.manifest.json` (SHA1 eaa5764ed33f8f422c2aae51af31601716289239), outcome ok, script ended True, q124_complete True
- poked: `capture.manifest.json` (SHA1 1ce484fa2baf71f36168666981b1e9c021a2b7d2), outcome ok, script ended True, q124_complete True
- stubs: none
- pokes: `0x80061280` (0x80061280) = 1600001001006424 (was 0b00401000000000) from phase t_load (frame 7450), 1600001001006424 at the end
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
| fight.9230 (frame 9230) | 0 of 153600 (0.00%) | - |
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
| fight.9892 (frame 9892) | 0 of 153600 (0.00%) | - |
| fight.9894 (frame 9894) | 0 of 153600 (0.00%) | - |
| fight.9898 (frame 9898) | 0 of 153600 (0.00%) | - |
| fight.9904 (frame 9904) | 0 of 153600 (0.00%) | - |
| fight.9930 (frame 9930) | 0 of 153600 (0.00%) | - |
| fight.10030 (frame 10030) | 0 of 153600 (0.00%) | - |
| fight.10130 (frame 10130) | 0 of 153600 (0.00%) | - |
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
| fight.11151 (frame 11151) | 0 of 153600 (0.00%) | - |
| fight.11153 (frame 11153) | 0 of 153600 (0.00%) | - |
| fight.11157 (frame 11157) | 0 of 153600 (0.00%) | - |
| fight.11163 (frame 11163) | 0 of 153600 (0.00%) | - |
| fight.11230 (frame 11230) | 0 of 153600 (0.00%) | - |

A poke counts only if the game kept running (both runs reached the script's end). Everything after it went in is a modified game: the differences show what the game does with the poked value, not what an unmodified run reaches.
