# Knockout observations

Liveness uses the script end_frame fallback (no RAM counter established). No names or roles are inferred.

## eff_LinkParticleQuads

Status: **ok**. script reached end_frame 12180 past requested frames 10584,10588,10592,10600,10610,10620,10635,10650,10680,10700,10840,10860,10900 (fallback).

# Stub-and-observe: eff_LinkParticleQuads against baseline

- scenario: versus-2p-kimigaura
- baseline: `docs\naming\runtime\versus-2p-kimigaura-blood-link-stub-1\baseline\capture.manifest.json` (SHA1 6d1c18cf1df6a4463cf443073c2bdebe0d184015), outcome ok, script ended True, q124_complete True
- stubbed: `docs\naming\runtime\versus-2p-kimigaura-blood-link-stub-1\eff_LinkParticleQuads\capture.manifest.json` (SHA1 1c0e70d0b771b9b9af9f9f2baddeb21f15ed19d1), outcome ok, script ended True, q124_complete True
- stubs: `eff_LinkParticleQuads` (0x80068D88) from phase fight (frame 8230), returns 1, intact at the end, called while stubbed in fight, t_ko
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
| fight.8700 (frame 8700) | 0 of 153600 (0.00%) | - |
| fight.9200 (frame 9200) | 0 of 153600 (0.00%) | - |
| fight.9600 (frame 9600) | 0 of 153600 (0.00%) | - |
| fight.9850 (frame 9850) | 0 of 153600 (0.00%) | - |
| fight.10050 (frame 10050) | 0 of 153600 (0.00%) | - |
| fight.10300 (frame 10300) | 0 of 153600 (0.00%) | - |
| knockout.10584 (frame 10584) | 0 of 153600 (0.00%) | - |
| knockout.10588 (frame 10588) | 0 of 153600 (0.00%) | - |
| knockout.10592 (frame 10592) | 88841 of 153600 (57.84%) | 0,0-639,239 |
| knockout.10600 (frame 10600) | 81899 of 153600 (53.32%) | 0,0-639,239 |
| knockout.10610 (frame 10610) | 101391 of 153600 (66.01%) | 0,0-639,239 |
| knockout.10620 (frame 10620) | 107445 of 153600 (69.95%) | 0,0-639,239 |
| knockout.10635 (frame 10635) | 93953 of 153600 (61.17%) | 0,0-639,239 |
| fight.10650 (frame 10650) | 104822 of 153600 (68.24%) | 0,0-639,239 |
| knockout.10650 (frame 10650) | 104822 of 153600 (68.24%) | 0,0-639,239 |
| knockout.10680 (frame 10680) | 121887 of 153600 (79.35%) | 0,0-639,239 |
| knockout.10700 (frame 10700) | 126771 of 153600 (82.53%) | 0,0-639,239 |
| fight.last (frame 10719) | 152680 of 153600 (99.40%) | 0,0-639,239 |
| t_ko.10800 (frame 10800) | 136494 of 153600 (88.86%) | 0,0-639,239 |
| knockout.10840 (frame 10840) | 65515 of 153600 (42.65%) | 0,0-639,239 |
| knockout.10860 (frame 10860) | 70804 of 153600 (46.10%) | 0,0-639,239 |
| t_ko.10900 (frame 10900) | 51247 of 153600 (33.36%) | 0,1-639,239 |
| knockout.10900 (frame 10900) | 51247 of 153600 (33.36%) | 0,1-639,239 |
| t_ko.11000 (frame 11000) | 84101 of 153600 (54.75%) | 0,0-639,239 |
| t_ko.11100 (frame 11100) | 111884 of 153600 (72.84%) | 0,0-639,239 |
| t_ko.last (frame 11179) | 97619 of 153600 (63.55%) | 0,0-639,239 |
| rematch_menu.first (frame 11180) | 99946 of 153600 (65.07%) | 0,0-639,239 |
| rematch_menu.11500 (frame 11500) | 96564 of 153600 (62.87%) | 0,0-639,239 |
| rematch_menu.11800 (frame 11800) | 98392 of 153600 (64.06%) | 0,0-639,239 |
| rematch_menu.last (frame 12179) | 94535 of 153600 (61.55%) | 0,0-639,239 |
| knockout.live (frame 12179) | 94535 of 153600 (61.55%) | 0,0-639,239 |

A stub counts only if the game kept running (both runs reached the script's end) and the lost effect is the function's own output (.claude/rules/naming-runtime-evidence.md). Pixels that differ beyond the function's own drawing point at changed game state. A call already in progress when the stub went in finishes normally (a leak breakpoint catches only code reaching the word after the stub).
