# Knockout observations

Liveness uses the script end_frame fallback (no RAM counter established). No names or roles are inferred.

## func_8004779C

Status: **ok**. script reached end_frame 98351 past requested frames 97700,97712,97724,97740,97760,97800,97850,97900,97956,97970,97990,98030,98100,98200,98290 (fallback).

# Stub-and-observe: func_8004779C against baseline

- scenario: story-lumberyard-cut
- baseline: `docs\naming\runtime\story-lumberyard-node-stub-1\baseline\capture.manifest.json` (SHA1 bc9b8da03e4be394ed72cc180fd4745e9747b416), outcome ok, script ended True, q124_complete True
- stubbed: `docs\naming\runtime\story-lumberyard-node-stub-1\func_8004779C\capture.manifest.json` (SHA1 bde9c4132a6db8a25eb01da79cfa893040bc3121), outcome ok, script ended True, q124_complete True
- stubs: `func_8004779C` (0x8004779C) from phase lumber (frame 97000), returns with v0 unchanged, intact at the end, called while stubbed in lumber
- same inputs and script

| screenshot | changed pixels | bounding box (x0,y0-x1,y1) |
|---|---|---|
| s.5489 (frame 5489) | 0 of 76800 (0.00%) | - |
| s.8899 (frame 8899) | 0 of 153600 (0.00%) | - |
| story.93500 (frame 93500) | 0 of 153600 (0.00%) | - |
| lumber.97700 (frame 97700) | 0 of 153600 (0.00%) | - |
| knockout.97700 (frame 97700) | 0 of 153600 (0.00%) | - |
| lumber.97712 (frame 97712) | 0 of 153600 (0.00%) | - |
| knockout.97712 (frame 97712) | 0 of 153600 (0.00%) | - |
| lumber.97724 (frame 97724) | 2613 of 153600 (1.70%) | 283,0-314,149 |
| knockout.97724 (frame 97724) | 2613 of 153600 (1.70%) | 283,0-314,149 |
| lumber.97740 (frame 97740) | 3131 of 153600 (2.04%) | 287,0-317,152 |
| knockout.97740 (frame 97740) | 3131 of 153600 (2.04%) | 287,0-317,152 |
| lumber.97760 (frame 97760) | 3401 of 153600 (2.21%) | 273,0-327,154 |
| knockout.97760 (frame 97760) | 3401 of 153600 (2.21%) | 273,0-327,154 |
| lumber.97800 (frame 97800) | 2318 of 153600 (1.51%) | 139,0-247,151 |
| knockout.97800 (frame 97800) | 2318 of 153600 (1.51%) | 139,0-247,151 |
| lumber.97850 (frame 97850) | 3041 of 153600 (1.98%) | 0,38-232,153 |
| knockout.97850 (frame 97850) | 3041 of 153600 (1.98%) | 0,38-232,153 |
| lumber.97900 (frame 97900) | 1976 of 153600 (1.29%) | 0,169-232,210 |
| knockout.97900 (frame 97900) | 1976 of 153600 (1.29%) | 0,169-232,210 |
| lumber.97956 (frame 97956) | 2248 of 153600 (1.46%) | 0,142-233,185 |
| knockout.97956 (frame 97956) | 2248 of 153600 (1.46%) | 0,142-233,185 |
| lumber.97970 (frame 97970) | 4138 of 153600 (2.69%) | 0,0-434,185 |
| knockout.97970 (frame 97970) | 4138 of 153600 (2.69%) | 0,0-434,185 |
| lumber.97990 (frame 97990) | 4634 of 153600 (3.02%) | 0,0-387,186 |
| knockout.97990 (frame 97990) | 4634 of 153600 (3.02%) | 0,0-387,186 |
| lumber.98030 (frame 98030) | 4988 of 153600 (3.25%) | 0,0-341,186 |
| knockout.98030 (frame 98030) | 4988 of 153600 (3.25%) | 0,0-341,186 |
| lumber.98100 (frame 98100) | 16382 of 153600 (10.67%) | 235,0-639,201 |
| knockout.98100 (frame 98100) | 16382 of 153600 (10.67%) | 235,0-639,201 |
| lumber.98200 (frame 98200) | 11821 of 153600 (7.70%) | 146,62-639,239 |
| knockout.98200 (frame 98200) | 11821 of 153600 (7.70%) | 146,62-639,239 |
| lumber.98290 (frame 98290) | 118437 of 153600 (77.11%) | 0,0-639,239 |
| knockout.98290 (frame 98290) | 118437 of 153600 (77.11%) | 0,0-639,239 |
| knockout.live (frame 98350) | 133613 of 153600 (86.99%) | 0,0-639,239 |

A stub counts only if the game kept running (both runs reached the script's end) and the lost effect is the function's own output (.claude/rules/naming-runtime-evidence.md). Pixels that differ beyond the function's own drawing point at changed game state. A call already in progress when the stub went in finishes normally (a leak breakpoint catches only code reaching the word after the stub).
