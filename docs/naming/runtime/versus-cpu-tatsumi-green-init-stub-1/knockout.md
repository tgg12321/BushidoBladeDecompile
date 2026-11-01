# Knockout observations

Liveness uses the script end_frame fallback (no RAM counter established). No names or roles are inferred.

## func_800650EC

Status: **ok**. script reached end_frame 13000 past requested frames 9876,9880,9883,9886,9889,9892,9895,9900,9971,9975,9978,9981,9983,9985,9987,9992,12305,12308,12310,12312,12314,12316,12318,12322,12900 (fallback).

# Stub-and-observe: func_800650EC against baseline

- scenario: versus-cpu-tatsumi
- baseline: `docs\naming\runtime\versus-cpu-tatsumi-green-init-stub-1\baseline\capture.manifest.json` (SHA1 dc84a3fbc20f424fc8a0c669a9bba757b44ee2b5), outcome ok, script ended True, q124_complete True
- stubbed: `docs\naming\runtime\versus-cpu-tatsumi-green-init-stub-1\func_800650EC\capture.manifest.json` (SHA1 795af24e5365984de797e257f282fa11efadbcc0), outcome ok, script ended True, q124_complete True
- stubs: `func_800650EC` (0x800650EC) from phase opp1 (frame 8350), returns with v0 unchanged, intact at the end, called while stubbed in opp2, opp3
- same inputs and script

| screenshot | changed pixels | bounding box (x0,y0-x1,y1) |
|---|---|---|
| boot_intro.last (frame 5489) | 0 of 76800 (0.00%) | - |
| top_menu.first (frame 6050) | 0 of 153600 (0.00%) | - |
| char_select.pick (frame 6599) | 0 of 153600 (0.00%) | - |
| weapon_select.first (frame 6900) | 0 of 153600 (0.00%) | - |
| t_load.mid (frame 8000) | 0 of 153600 (0.00%) | - |
| opp1.first (frame 8350) | 0 of 153600 (0.00%) | - |
| opp1.8600 (frame 8600) | 0 of 153600 (0.00%) | - |
| opp1.last (frame 8850) | 0 of 153600 (0.00%) | - |
| opp2.9200 (frame 9200) | 0 of 153600 (0.00%) | - |
| knockout.9876 (frame 9876) | 0 of 153600 (0.00%) | - |
| knockout.9880 (frame 9880) | 0 of 153600 (0.00%) | - |
| knockout.9883 (frame 9883) | 0 of 153600 (0.00%) | - |
| knockout.9886 (frame 9886) | 0 of 153600 (0.00%) | - |
| knockout.9889 (frame 9889) | 215 of 153600 (0.14%) | 196,103-207,150 |
| knockout.9892 (frame 9892) | 319 of 153600 (0.21%) | 195,104-212,154 |
| knockout.9895 (frame 9895) | 422 of 153600 (0.27%) | 197,105-220,153 |
| knockout.9900 (frame 9900) | 189 of 153600 (0.12%) | 230,111-262,145 |
| knockout.9971 (frame 9971) | 0 of 153600 (0.00%) | - |
| knockout.9975 (frame 9975) | 0 of 153600 (0.00%) | - |
| knockout.9978 (frame 9978) | 0 of 153600 (0.00%) | - |
| knockout.9981 (frame 9981) | 0 of 153600 (0.00%) | - |
| knockout.9983 (frame 9983) | 0 of 153600 (0.00%) | - |
| knockout.9985 (frame 9985) | 0 of 153600 (0.00%) | - |
| knockout.9987 (frame 9987) | 0 of 153600 (0.00%) | - |
| knockout.9992 (frame 9992) | 0 of 153600 (0.00%) | - |
| opp2.10000 (frame 10000) | 0 of 153600 (0.00%) | - |
| opp2.last (frame 10540) | 148378 of 153600 (96.60%) | 0,0-639,239 |
| opp3.10800 (frame 10800) | 149139 of 153600 (97.10%) | 0,0-639,239 |
| opp3.11200 (frame 11200) | 152288 of 153600 (99.15%) | 0,0-639,239 |
| opp3.11600 (frame 11600) | 152386 of 153600 (99.21%) | 0,0-639,239 |
| opp3.12000 (frame 12000) | 151237 of 153600 (98.46%) | 0,0-639,239 |
| knockout.12305 (frame 12305) | 150009 of 153600 (97.66%) | 0,0-639,239 |
| knockout.12308 (frame 12308) | 150051 of 153600 (97.69%) | 0,0-639,239 |
| knockout.12310 (frame 12310) | 150271 of 153600 (97.83%) | 0,0-639,239 |
| knockout.12312 (frame 12312) | 150112 of 153600 (97.73%) | 0,0-639,239 |
| knockout.12314 (frame 12314) | 150270 of 153600 (97.83%) | 0,0-639,239 |
| knockout.12316 (frame 12316) | 149975 of 153600 (97.64%) | 0,0-639,239 |
| knockout.12318 (frame 12318) | 149765 of 153600 (97.50%) | 0,0-639,239 |
| knockout.12322 (frame 12322) | 149402 of 153600 (97.27%) | 0,0-639,239 |
| opp3.12400 (frame 12400) | 146995 of 153600 (95.70%) | 0,0-639,239 |
| knockout.12900 (frame 12900) | 146471 of 153600 (95.36%) | 0,0-639,239 |
| opp3.last (frame 12999) | 146445 of 153600 (95.34%) | 0,0-639,239 |
| knockout.live (frame 12999) | 146445 of 153600 (95.34%) | 0,0-639,239 |

A stub counts only if the game kept running (both runs reached the script's end) and the lost effect is the function's own output (.claude/rules/naming-runtime-evidence.md). Pixels that differ beyond the function's own drawing point at changed game state. A call already in progress when the stub went in finishes normally (a leak breakpoint catches only code reaching the word after the stub).

## func_80065134

Status: **ok**. script reached end_frame 13000 past requested frames 9876,9880,9883,9886,9889,9892,9895,9900,9971,9975,9978,9981,9983,9985,9987,9992,12305,12308,12310,12312,12314,12316,12318,12322,12900 (fallback).

# Stub-and-observe: func_80065134 against baseline

- scenario: versus-cpu-tatsumi
- baseline: `docs\naming\runtime\versus-cpu-tatsumi-green-init-stub-1\baseline\capture.manifest.json` (SHA1 dc84a3fbc20f424fc8a0c669a9bba757b44ee2b5), outcome ok, script ended True, q124_complete True
- stubbed: `docs\naming\runtime\versus-cpu-tatsumi-green-init-stub-1\func_80065134\capture.manifest.json` (SHA1 2bece0ead1a28e40b47511ed0b51907988b8de3f), outcome ok, script ended True, q124_complete True
- stubs: `func_80065134` (0x80065134) from phase opp1 (frame 8350), returns with v0 unchanged, intact at the end, called while stubbed in opp2, opp3
- same inputs and script

| screenshot | changed pixels | bounding box (x0,y0-x1,y1) |
|---|---|---|
| boot_intro.last (frame 5489) | 0 of 76800 (0.00%) | - |
| top_menu.first (frame 6050) | 0 of 153600 (0.00%) | - |
| char_select.pick (frame 6599) | 0 of 153600 (0.00%) | - |
| weapon_select.first (frame 6900) | 0 of 153600 (0.00%) | - |
| t_load.mid (frame 8000) | 0 of 153600 (0.00%) | - |
| opp1.first (frame 8350) | 0 of 153600 (0.00%) | - |
| opp1.8600 (frame 8600) | 0 of 153600 (0.00%) | - |
| opp1.last (frame 8850) | 0 of 153600 (0.00%) | - |
| opp2.9200 (frame 9200) | 0 of 153600 (0.00%) | - |
| knockout.9876 (frame 9876) | 0 of 153600 (0.00%) | - |
| knockout.9880 (frame 9880) | 0 of 153600 (0.00%) | - |
| knockout.9883 (frame 9883) | 0 of 153600 (0.00%) | - |
| knockout.9886 (frame 9886) | 0 of 153600 (0.00%) | - |
| knockout.9889 (frame 9889) | 0 of 153600 (0.00%) | - |
| knockout.9892 (frame 9892) | 0 of 153600 (0.00%) | - |
| knockout.9895 (frame 9895) | 0 of 153600 (0.00%) | - |
| knockout.9900 (frame 9900) | 0 of 153600 (0.00%) | - |
| knockout.9971 (frame 9971) | 0 of 153600 (0.00%) | - |
| knockout.9975 (frame 9975) | 0 of 153600 (0.00%) | - |
| knockout.9978 (frame 9978) | 0 of 153600 (0.00%) | - |
| knockout.9981 (frame 9981) | 0 of 153600 (0.00%) | - |
| knockout.9983 (frame 9983) | 272 of 153600 (0.18%) | 388,76-401,132 |
| knockout.9985 (frame 9985) | 406 of 153600 (0.26%) | 383,76-402,132 |
| knockout.9987 (frame 9987) | 527 of 153600 (0.34%) | 374,77-401,131 |
| knockout.9992 (frame 9992) | 638 of 153600 (0.42%) | 375,81-411,127 |
| opp2.10000 (frame 10000) | 0 of 153600 (0.00%) | - |
| opp2.last (frame 10540) | 148378 of 153600 (96.60%) | 0,0-639,239 |
| opp3.10800 (frame 10800) | 149139 of 153600 (97.10%) | 0,0-639,239 |
| opp3.11200 (frame 11200) | 152288 of 153600 (99.15%) | 0,0-639,239 |
| opp3.11600 (frame 11600) | 152386 of 153600 (99.21%) | 0,0-639,239 |
| opp3.12000 (frame 12000) | 151237 of 153600 (98.46%) | 0,0-639,239 |
| knockout.12305 (frame 12305) | 150009 of 153600 (97.66%) | 0,0-639,239 |
| knockout.12308 (frame 12308) | 150051 of 153600 (97.69%) | 0,0-639,239 |
| knockout.12310 (frame 12310) | 150271 of 153600 (97.83%) | 0,0-639,239 |
| knockout.12312 (frame 12312) | 150112 of 153600 (97.73%) | 0,0-639,239 |
| knockout.12314 (frame 12314) | 150270 of 153600 (97.83%) | 0,0-639,239 |
| knockout.12316 (frame 12316) | 149975 of 153600 (97.64%) | 0,0-639,239 |
| knockout.12318 (frame 12318) | 149765 of 153600 (97.50%) | 0,0-639,239 |
| knockout.12322 (frame 12322) | 149402 of 153600 (97.27%) | 0,0-639,239 |
| opp3.12400 (frame 12400) | 146995 of 153600 (95.70%) | 0,0-639,239 |
| knockout.12900 (frame 12900) | 146471 of 153600 (95.36%) | 0,0-639,239 |
| opp3.last (frame 12999) | 146445 of 153600 (95.34%) | 0,0-639,239 |
| knockout.live (frame 12999) | 146445 of 153600 (95.34%) | 0,0-639,239 |

A stub counts only if the game kept running (both runs reached the script's end) and the lost effect is the function's own output (.claude/rules/naming-runtime-evidence.md). Pixels that differ beyond the function's own drawing point at changed game state. A call already in progress when the stub went in finishes normally (a leak breakpoint catches only code reaching the word after the stub).
