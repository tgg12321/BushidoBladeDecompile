# Knockout observations

Liveness uses the script end_frame fallback (no RAM counter established). No names or roles are inferred.

## func_80063AF0

Status: **ok**. script reached end_frame 12730 past requested frames 8800,9167,9171,9827,9831,9887,9891,10500,11146,11150,11806,11810,11866,11870,12600 (fallback).

# Stub-and-observe: func_80063AF0 against baseline

- scenario: versus-2p-kimigaura-effects
- baseline: `..\bb2-worktrees\ln-trace-a\docs\naming\runtime\versus-2p-kimigaura-eff-stub-1\baseline\capture.manifest.json` (SHA1 95a1023452b3ff88a39af9350a90b76428371a2f), outcome ok, script ended True, q124_complete True
- stubbed: `..\bb2-worktrees\ln-trace-a\docs\naming\runtime\versus-2p-kimigaura-eff-stub-1\func_80063AF0\capture.manifest.json` (SHA1 9bca3caa2a816bd810dee285edb2f360abc01b3c), outcome ok, script ended True, q124_complete True
- stubs: `func_80063AF0` (0x80063AF0) from phase fight (frame 8230), returns 0, intact at the end, called while stubbed in fight
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
| fight.8730 (frame 8730) | 48 of 153600 (0.03%) | 154,160-268,165 |
| knockout.8800 (frame 8800) | 156 of 153600 (0.10%) | 0,166-275,178 |
| fight.8830 (frame 8830) | 138 of 153600 (0.09%) | 15,167-232,179 |
| fight.8930 (frame 8930) | 41 of 153600 (0.03%) | 76,169-99,171 |
| fight.9030 (frame 9030) | 50 of 153600 (0.03%) | 48,172-71,175 |
| fight.9130 (frame 9130) | 46 of 153600 (0.03%) | 29,176-56,178 |
| knockout.9167 (frame 9167) | 63 of 153600 (0.04%) | 30,178-58,181 |
| knockout.9171 (frame 9171) | 53 of 153600 (0.03%) | 28,178-55,181 |
| fight.9230 (frame 9230) | 71 of 153600 (0.05%) | 0,168-211,178 |
| fight.9330 (frame 9330) | 81 of 153600 (0.05%) | 32,170-118,175 |
| fight.9430 (frame 9430) | 0 of 153600 (0.00%) | - |
| fight.9530 (frame 9530) | 81 of 153600 (0.05%) | 21,164-224,174 |
| fight.9630 (frame 9630) | 108 of 153600 (0.07%) | 0,159-212,171 |
| fight.9730 (frame 9730) | 46 of 153600 (0.03%) | 48,167-72,170 |
| knockout.9827 (frame 9827) | 41 of 153600 (0.03%) | 12,171-39,173 |
| fight.9830 (frame 9830) | 49 of 153600 (0.03%) | 9,170-36,173 |
| knockout.9831 (frame 9831) | 49 of 153600 (0.03%) | 9,170-36,173 |
| knockout.9887 (frame 9887) | 40 of 153600 (0.03%) | 15,166-40,168 |
| knockout.9891 (frame 9891) | 44 of 153600 (0.03%) | 17,166-40,169 |
| fight.9930 (frame 9930) | 95 of 153600 (0.06%) | 80,161-250,175 |
| fight.10030 (frame 10030) | 78 of 153600 (0.05%) | 102,153-216,164 |
| fight.10130 (frame 10130) | 192 of 153600 (0.12%) | 14,183-141,211 |
| fight.10230 (frame 10230) | 222 of 153600 (0.14%) | 4,198-86,210 |
| fight.10330 (frame 10330) | 165 of 153600 (0.11%) | 0,195-55,206 |
| fight.10430 (frame 10430) | 483 of 153600 (0.31%) | 3,172-178,239 |
| knockout.10500 (frame 10500) | 400 of 153600 (0.26%) | 0,210-147,239 |
| fight.10530 (frame 10530) | 370 of 153600 (0.24%) | 0,209-141,238 |
| fight.10630 (frame 10630) | 246 of 153600 (0.16%) | 74,226-123,234 |
| fight.10730 (frame 10730) | 132 of 153600 (0.09%) | 62,221-101,229 |
| fight.10830 (frame 10830) | 174 of 153600 (0.11%) | 31,216-80,223 |
| fight.10930 (frame 10930) | 238 of 153600 (0.15%) | 0,161-232,178 |
| fight.11030 (frame 11030) | 71120 of 153600 (46.30%) | 0,0-639,239 |
| fight.11130 (frame 11130) | 90990 of 153600 (59.24%) | 0,0-639,239 |
| knockout.11146 (frame 11146) | 103771 of 153600 (67.56%) | 0,0-639,239 |
| knockout.11150 (frame 11150) | 108693 of 153600 (70.76%) | 0,0-639,239 |
| fight.11230 (frame 11230) | 100004 of 153600 (65.11%) | 0,0-639,239 |
| fight.11330 (frame 11330) | 99150 of 153600 (64.55%) | 0,0-639,239 |
| fight.11430 (frame 11430) | 99558 of 153600 (64.82%) | 0,0-639,239 |
| fight.11530 (frame 11530) | 106571 of 153600 (69.38%) | 0,0-639,239 |
| fight.11630 (frame 11630) | 106448 of 153600 (69.30%) | 0,0-639,239 |
| fight.11730 (frame 11730) | 101120 of 153600 (65.83%) | 0,0-639,239 |
| knockout.11806 (frame 11806) | 105944 of 153600 (68.97%) | 0,0-639,239 |
| knockout.11810 (frame 11810) | 103796 of 153600 (67.58%) | 0,0-639,239 |
| fight.11830 (frame 11830) | 104755 of 153600 (68.20%) | 0,0-639,239 |
| knockout.11866 (frame 11866) | 100954 of 153600 (65.73%) | 0,0-639,239 |
| knockout.11870 (frame 11870) | 107463 of 153600 (69.96%) | 0,0-639,239 |
| fight.11930 (frame 11930) | 108702 of 153600 (70.77%) | 0,0-639,239 |
| fight.12030 (frame 12030) | 127378 of 153600 (82.93%) | 0,0-639,239 |
| fight.12130 (frame 12130) | 132742 of 153600 (86.42%) | 0,0-639,239 |
| fight.12230 (frame 12230) | 132523 of 153600 (86.28%) | 0,0-639,239 |
| fight.12330 (frame 12330) | 149443 of 153600 (97.29%) | 0,0-639,239 |
| fight.12430 (frame 12430) | 150112 of 153600 (97.73%) | 0,0-639,239 |
| fight.12530 (frame 12530) | 146757 of 153600 (95.54%) | 0,0-639,239 |
| knockout.12600 (frame 12600) | 143495 of 153600 (93.42%) | 0,0-639,239 |
| fight.12630 (frame 12630) | 141680 of 153600 (92.24%) | 0,0-639,239 |
| fight.last (frame 12729) | 138533 of 153600 (90.19%) | 0,0-639,239 |
| knockout.live (frame 12729) | 138533 of 153600 (90.19%) | 0,0-639,239 |

A stub counts only if the game kept running (both runs reached the script's end) and the lost effect is the function's own output (.claude/rules/naming-runtime-evidence.md). Pixels that differ beyond the function's own drawing point at changed game state. A call already in progress when the stub went in finishes normally (a leak breakpoint catches only code reaching the word after the stub).

## func_80063B34

Status: **ok**. script reached end_frame 12730 past requested frames 8800,9167,9171,9827,9831,9887,9891,10500,11146,11150,11806,11810,11866,11870,12600 (fallback).

# Stub-and-observe: func_80063B34 against baseline

- scenario: versus-2p-kimigaura-effects
- baseline: `..\bb2-worktrees\ln-trace-a\docs\naming\runtime\versus-2p-kimigaura-eff-stub-1\baseline\capture.manifest.json` (SHA1 95a1023452b3ff88a39af9350a90b76428371a2f), outcome ok, script ended True, q124_complete True
- stubbed: `..\bb2-worktrees\ln-trace-a\docs\naming\runtime\versus-2p-kimigaura-eff-stub-1\func_80063B34\capture.manifest.json` (SHA1 da093e64c9ca9d2f0145d0c3e28fe88a23625d0e), outcome ok, script ended True, q124_complete True
- stubs: `func_80063B34` (0x80063B34) from phase fight (frame 8230), returns 0, intact at the end, called while stubbed in fight
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
| knockout.8800 (frame 8800) | 28 of 153600 (0.02%) | 455,176-467,179 |
| fight.8830 (frame 8830) | 125 of 153600 (0.08%) | 415,176-450,181 |
| fight.8930 (frame 8930) | 221 of 153600 (0.14%) | 255,163-363,172 |
| fight.9030 (frame 9030) | 253 of 153600 (0.16%) | 239,166-354,176 |
| fight.9130 (frame 9130) | 256 of 153600 (0.17%) | 234,168-355,180 |
| knockout.9167 (frame 9167) | 213 of 153600 (0.14%) | 242,169-369,181 |
| knockout.9171 (frame 9171) | 293 of 153600 (0.19%) | 241,170-368,182 |
| fight.9230 (frame 9230) | 236 of 153600 (0.15%) | 185,169-306,180 |
| fight.9330 (frame 9330) | 302 of 153600 (0.20%) | 89,169-212,180 |
| fight.9430 (frame 9430) | 100 of 153600 (0.07%) | 0,185-25,191 |
| fight.9530 (frame 9530) | 92 of 153600 (0.06%) | 186,171-222,175 |
| fight.9630 (frame 9630) | 168 of 153600 (0.11%) | 0,163-301,175 |
| fight.9730 (frame 9730) | 131 of 153600 (0.09%) | 91,161-162,173 |
| knockout.9827 (frame 9827) | 156 of 153600 (0.10%) | 58,163-137,176 |
| fight.9830 (frame 9830) | 153 of 153600 (0.10%) | 55,163-132,176 |
| knockout.9831 (frame 9831) | 153 of 153600 (0.10%) | 55,163-132,176 |
| knockout.9887 (frame 9887) | 123 of 153600 (0.08%) | 63,160-131,172 |
| knockout.9891 (frame 9891) | 132 of 153600 (0.09%) | 65,160-131,172 |
| fight.9930 (frame 9930) | 41 of 153600 (0.03%) | 123,158-172,168 |
| fight.10030 (frame 10030) | 75 of 153600 (0.05%) | 142,149-195,158 |
| fight.10130 (frame 10130) | 112 of 153600 (0.07%) | 0,174-64,191 |
| fight.10230 (frame 10230) | 73 of 153600 (0.05%) | 0,173-54,192 |
| fight.10330 (frame 10330) | 48 of 153600 (0.03%) | 2,183-30,189 |
| fight.10430 (frame 10430) | 0 of 153600 (0.00%) | - |
| knockout.10500 (frame 10500) | 0 of 153600 (0.00%) | - |
| fight.10530 (frame 10530) | 0 of 153600 (0.00%) | - |
| fight.10630 (frame 10630) | 0 of 153600 (0.00%) | - |
| fight.10730 (frame 10730) | 0 of 153600 (0.00%) | - |
| fight.10830 (frame 10830) | 0 of 153600 (0.00%) | - |
| fight.10930 (frame 10930) | 0 of 153600 (0.00%) | - |
| fight.11030 (frame 11030) | 71242 of 153600 (46.38%) | 0,0-639,239 |
| fight.11130 (frame 11130) | 90995 of 153600 (59.24%) | 0,0-639,239 |
| knockout.11146 (frame 11146) | 103796 of 153600 (67.58%) | 0,0-639,239 |
| knockout.11150 (frame 11150) | 108748 of 153600 (70.80%) | 0,0-639,239 |
| fight.11230 (frame 11230) | 100043 of 153600 (65.13%) | 0,0-639,239 |
| fight.11330 (frame 11330) | 99232 of 153600 (64.60%) | 0,0-639,239 |
| fight.11430 (frame 11430) | 99615 of 153600 (64.85%) | 0,0-639,239 |
| fight.11530 (frame 11530) | 106578 of 153600 (69.39%) | 0,0-639,239 |
| fight.11630 (frame 11630) | 106476 of 153600 (69.32%) | 0,0-639,239 |
| fight.11730 (frame 11730) | 100958 of 153600 (65.73%) | 0,0-639,239 |
| knockout.11806 (frame 11806) | 105741 of 153600 (68.84%) | 0,0-639,239 |
| knockout.11810 (frame 11810) | 103618 of 153600 (67.46%) | 0,0-639,239 |
| fight.11830 (frame 11830) | 104606 of 153600 (68.10%) | 0,0-639,239 |
| knockout.11866 (frame 11866) | 100837 of 153600 (65.65%) | 0,0-639,239 |
| knockout.11870 (frame 11870) | 107321 of 153600 (69.87%) | 0,0-639,239 |
| fight.11930 (frame 11930) | 108647 of 153600 (70.73%) | 0,0-639,239 |
| fight.12030 (frame 12030) | 127381 of 153600 (82.93%) | 0,0-639,239 |
| fight.12130 (frame 12130) | 132747 of 153600 (86.42%) | 0,0-639,239 |
| fight.12230 (frame 12230) | 132516 of 153600 (86.27%) | 0,0-639,239 |
| fight.12330 (frame 12330) | 149443 of 153600 (97.29%) | 0,0-639,239 |
| fight.12430 (frame 12430) | 150112 of 153600 (97.73%) | 0,0-639,239 |
| fight.12530 (frame 12530) | 146757 of 153600 (95.54%) | 0,0-639,239 |
| knockout.12600 (frame 12600) | 143495 of 153600 (93.42%) | 0,0-639,239 |
| fight.12630 (frame 12630) | 141680 of 153600 (92.24%) | 0,0-639,239 |
| fight.last (frame 12729) | 138533 of 153600 (90.19%) | 0,0-639,239 |
| knockout.live (frame 12729) | 138533 of 153600 (90.19%) | 0,0-639,239 |

A stub counts only if the game kept running (both runs reached the script's end) and the lost effect is the function's own output (.claude/rules/naming-runtime-evidence.md). Pixels that differ beyond the function's own drawing point at changed game state. A call already in progress when the stub went in finishes normally (a leak breakpoint catches only code reaching the word after the stub).

## func_80064ED8

Status: **ok**. script reached end_frame 12730 past requested frames 8800,9167,9171,9827,9831,9887,9891,10500,11146,11150,11806,11810,11866,11870,12600 (fallback).

# Stub-and-observe: func_80064ED8 against baseline

- scenario: versus-2p-kimigaura-effects
- baseline: `..\bb2-worktrees\ln-trace-a\docs\naming\runtime\versus-2p-kimigaura-eff-stub-1\baseline\capture.manifest.json` (SHA1 95a1023452b3ff88a39af9350a90b76428371a2f), outcome ok, script ended True, q124_complete True
- stubbed: `..\bb2-worktrees\ln-trace-a\docs\naming\runtime\versus-2p-kimigaura-eff-stub-1\func_80064ED8\capture.manifest.json` (SHA1 3b21027d83b3347322a5ffdecf1bb7075b0d1c85), outcome ok, script ended True, q124_complete True
- stubs: `func_80064ED8` (0x80064ED8) from phase fight (frame 8230), returns with v0 unchanged, intact at the end, called while stubbed in fight
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
| knockout.8800 (frame 8800) | 0 of 153600 (0.00%) | - |
| fight.8830 (frame 8830) | 0 of 153600 (0.00%) | - |
| fight.8930 (frame 8930) | 0 of 153600 (0.00%) | - |
| fight.9030 (frame 9030) | 0 of 153600 (0.00%) | - |
| fight.9130 (frame 9130) | 0 of 153600 (0.00%) | - |
| knockout.9167 (frame 9167) | 0 of 153600 (0.00%) | - |
| knockout.9171 (frame 9171) | 0 of 153600 (0.00%) | - |
| fight.9230 (frame 9230) | 0 of 153600 (0.00%) | - |
| fight.9330 (frame 9330) | 0 of 153600 (0.00%) | - |
| fight.9430 (frame 9430) | 0 of 153600 (0.00%) | - |
| fight.9530 (frame 9530) | 0 of 153600 (0.00%) | - |
| fight.9630 (frame 9630) | 0 of 153600 (0.00%) | - |
| fight.9730 (frame 9730) | 0 of 153600 (0.00%) | - |
| knockout.9827 (frame 9827) | 0 of 153600 (0.00%) | - |
| fight.9830 (frame 9830) | 0 of 153600 (0.00%) | - |
| knockout.9831 (frame 9831) | 0 of 153600 (0.00%) | - |
| knockout.9887 (frame 9887) | 0 of 153600 (0.00%) | - |
| knockout.9891 (frame 9891) | 0 of 153600 (0.00%) | - |
| fight.9930 (frame 9930) | 0 of 153600 (0.00%) | - |
| fight.10030 (frame 10030) | 0 of 153600 (0.00%) | - |
| fight.10130 (frame 10130) | 0 of 153600 (0.00%) | - |
| fight.10230 (frame 10230) | 0 of 153600 (0.00%) | - |
| fight.10330 (frame 10330) | 0 of 153600 (0.00%) | - |
| fight.10430 (frame 10430) | 0 of 153600 (0.00%) | - |
| knockout.10500 (frame 10500) | 0 of 153600 (0.00%) | - |
| fight.10530 (frame 10530) | 0 of 153600 (0.00%) | - |
| fight.10630 (frame 10630) | 0 of 153600 (0.00%) | - |
| fight.10730 (frame 10730) | 0 of 153600 (0.00%) | - |
| fight.10830 (frame 10830) | 0 of 153600 (0.00%) | - |
| fight.10930 (frame 10930) | 0 of 153600 (0.00%) | - |
| fight.11030 (frame 11030) | 0 of 153600 (0.00%) | - |
| fight.11130 (frame 11130) | 0 of 153600 (0.00%) | - |
| knockout.11146 (frame 11146) | 0 of 153600 (0.00%) | - |
| knockout.11150 (frame 11150) | 0 of 153600 (0.00%) | - |
| fight.11230 (frame 11230) | 0 of 153600 (0.00%) | - |
| fight.11330 (frame 11330) | 0 of 153600 (0.00%) | - |
| fight.11430 (frame 11430) | 0 of 153600 (0.00%) | - |
| fight.11530 (frame 11530) | 0 of 153600 (0.00%) | - |
| fight.11630 (frame 11630) | 0 of 153600 (0.00%) | - |
| fight.11730 (frame 11730) | 0 of 153600 (0.00%) | - |
| knockout.11806 (frame 11806) | 0 of 153600 (0.00%) | - |
| knockout.11810 (frame 11810) | 0 of 153600 (0.00%) | - |
| fight.11830 (frame 11830) | 0 of 153600 (0.00%) | - |
| knockout.11866 (frame 11866) | 0 of 153600 (0.00%) | - |
| knockout.11870 (frame 11870) | 0 of 153600 (0.00%) | - |
| fight.11930 (frame 11930) | 0 of 153600 (0.00%) | - |
| fight.12030 (frame 12030) | 0 of 153600 (0.00%) | - |
| fight.12130 (frame 12130) | 0 of 153600 (0.00%) | - |
| fight.12230 (frame 12230) | 0 of 153600 (0.00%) | - |
| fight.12330 (frame 12330) | 0 of 153600 (0.00%) | - |
| fight.12430 (frame 12430) | 0 of 153600 (0.00%) | - |
| fight.12530 (frame 12530) | 0 of 153600 (0.00%) | - |
| knockout.12600 (frame 12600) | 0 of 153600 (0.00%) | - |
| fight.12630 (frame 12630) | 0 of 153600 (0.00%) | - |
| fight.last (frame 12729) | 0 of 153600 (0.00%) | - |
| knockout.live (frame 12729) | 0 of 153600 (0.00%) | - |

A stub counts only if the game kept running (both runs reached the script's end) and the lost effect is the function's own output (.claude/rules/naming-runtime-evidence.md). Pixels that differ beyond the function's own drawing point at changed game state. A call already in progress when the stub went in finishes normally (a leak breakpoint catches only code reaching the word after the stub).

## func_80064F68

Status: **ok**. script reached end_frame 12730 past requested frames 8800,9167,9171,9827,9831,9887,9891,10500,11146,11150,11806,11810,11866,11870,12600 (fallback).

# Stub-and-observe: func_80064F68 against baseline

- scenario: versus-2p-kimigaura-effects
- baseline: `..\bb2-worktrees\ln-trace-a\docs\naming\runtime\versus-2p-kimigaura-eff-stub-1\baseline\capture.manifest.json` (SHA1 95a1023452b3ff88a39af9350a90b76428371a2f), outcome ok, script ended True, q124_complete True
- stubbed: `..\bb2-worktrees\ln-trace-a\docs\naming\runtime\versus-2p-kimigaura-eff-stub-1\func_80064F68\capture.manifest.json` (SHA1 42be9cf69035094e71726ade3051149974a68dfc), outcome ok, script ended True, q124_complete True
- stubs: `func_80064F68` (0x80064F68) from phase fight (frame 8230), returns 0, intact at the end, called while stubbed in fight
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
| knockout.8800 (frame 8800) | 0 of 153600 (0.00%) | - |
| fight.8830 (frame 8830) | 0 of 153600 (0.00%) | - |
| fight.8930 (frame 8930) | 0 of 153600 (0.00%) | - |
| fight.9030 (frame 9030) | 0 of 153600 (0.00%) | - |
| fight.9130 (frame 9130) | 0 of 153600 (0.00%) | - |
| knockout.9167 (frame 9167) | 0 of 153600 (0.00%) | - |
| knockout.9171 (frame 9171) | 0 of 153600 (0.00%) | - |
| fight.9230 (frame 9230) | 0 of 153600 (0.00%) | - |
| fight.9330 (frame 9330) | 0 of 153600 (0.00%) | - |
| fight.9430 (frame 9430) | 0 of 153600 (0.00%) | - |
| fight.9530 (frame 9530) | 0 of 153600 (0.00%) | - |
| fight.9630 (frame 9630) | 0 of 153600 (0.00%) | - |
| fight.9730 (frame 9730) | 0 of 153600 (0.00%) | - |
| knockout.9827 (frame 9827) | 0 of 153600 (0.00%) | - |
| fight.9830 (frame 9830) | 0 of 153600 (0.00%) | - |
| knockout.9831 (frame 9831) | 0 of 153600 (0.00%) | - |
| knockout.9887 (frame 9887) | 0 of 153600 (0.00%) | - |
| knockout.9891 (frame 9891) | 0 of 153600 (0.00%) | - |
| fight.9930 (frame 9930) | 0 of 153600 (0.00%) | - |
| fight.10030 (frame 10030) | 0 of 153600 (0.00%) | - |
| fight.10130 (frame 10130) | 0 of 153600 (0.00%) | - |
| fight.10230 (frame 10230) | 0 of 153600 (0.00%) | - |
| fight.10330 (frame 10330) | 0 of 153600 (0.00%) | - |
| fight.10430 (frame 10430) | 0 of 153600 (0.00%) | - |
| knockout.10500 (frame 10500) | 0 of 153600 (0.00%) | - |
| fight.10530 (frame 10530) | 0 of 153600 (0.00%) | - |
| fight.10630 (frame 10630) | 0 of 153600 (0.00%) | - |
| fight.10730 (frame 10730) | 0 of 153600 (0.00%) | - |
| fight.10830 (frame 10830) | 0 of 153600 (0.00%) | - |
| fight.10930 (frame 10930) | 0 of 153600 (0.00%) | - |
| fight.11030 (frame 11030) | 0 of 153600 (0.00%) | - |
| fight.11130 (frame 11130) | 0 of 153600 (0.00%) | - |
| knockout.11146 (frame 11146) | 0 of 153600 (0.00%) | - |
| knockout.11150 (frame 11150) | 0 of 153600 (0.00%) | - |
| fight.11230 (frame 11230) | 0 of 153600 (0.00%) | - |
| fight.11330 (frame 11330) | 0 of 153600 (0.00%) | - |
| fight.11430 (frame 11430) | 0 of 153600 (0.00%) | - |
| fight.11530 (frame 11530) | 0 of 153600 (0.00%) | - |
| fight.11630 (frame 11630) | 0 of 153600 (0.00%) | - |
| fight.11730 (frame 11730) | 0 of 153600 (0.00%) | - |
| knockout.11806 (frame 11806) | 0 of 153600 (0.00%) | - |
| knockout.11810 (frame 11810) | 0 of 153600 (0.00%) | - |
| fight.11830 (frame 11830) | 0 of 153600 (0.00%) | - |
| knockout.11866 (frame 11866) | 0 of 153600 (0.00%) | - |
| knockout.11870 (frame 11870) | 0 of 153600 (0.00%) | - |
| fight.11930 (frame 11930) | 0 of 153600 (0.00%) | - |
| fight.12030 (frame 12030) | 0 of 153600 (0.00%) | - |
| fight.12130 (frame 12130) | 0 of 153600 (0.00%) | - |
| fight.12230 (frame 12230) | 0 of 153600 (0.00%) | - |
| fight.12330 (frame 12330) | 0 of 153600 (0.00%) | - |
| fight.12430 (frame 12430) | 0 of 153600 (0.00%) | - |
| fight.12530 (frame 12530) | 0 of 153600 (0.00%) | - |
| knockout.12600 (frame 12600) | 0 of 153600 (0.00%) | - |
| fight.12630 (frame 12630) | 0 of 153600 (0.00%) | - |
| fight.last (frame 12729) | 0 of 153600 (0.00%) | - |
| knockout.live (frame 12729) | 0 of 153600 (0.00%) | - |

A stub counts only if the game kept running (both runs reached the script's end) and the lost effect is the function's own output (.claude/rules/naming-runtime-evidence.md). Pixels that differ beyond the function's own drawing point at changed game state. A call already in progress when the stub went in finishes normally (a leak breakpoint catches only code reaching the word after the stub).
