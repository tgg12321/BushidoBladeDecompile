# Knockout observations

Liveness uses the script end_frame fallback (no RAM counter established). No names or roles are inferred.

## func_80065760

Status: **ok**. script reached end_frame 158000 past requested frames 154556,154560,154562,154564,154566,154568,154570,154572,154574,154576,154578,154580,154584,154590,154688,154694,154700,154706,154900,154996,155002,155008,155014,155020 (fallback).

# Stub-and-observe: func_80065760 against baseline

- scenario: story-final-fight-retail
- baseline: `docs\naming\runtime\story-final-fight-magenta-stub-1\baseline\capture.manifest.json` (SHA1 3441bd6b6bac76c9dea5b11c5ebd43e3857c24e7), outcome ok, script ended True, q124_complete True
- stubbed: `docs\naming\runtime\story-final-fight-magenta-stub-1\func_80065760\capture.manifest.json` (SHA1 b62b20c39cc3128dee2c04d95081f0012f9c051d), outcome ok, script ended True, q124_complete True
- stubs: `func_80065760` (0x80065760) from phase final_fight (frame 153700), returns 0, intact at the end, called while stubbed in final_fight
- same inputs and script

| screenshot | changed pixels | bounding box (x0,y0-x1,y1) |
|---|---|---|
| s.5489 (frame 5489) | 0 of 76800 (0.00%) | - |
| s.8899 (frame 8899) | 0 of 153600 (0.00%) | - |
| s.126599 (frame 126599) | 0 of 153600 (0.00%) | - |
| s.127900 (frame 127900) | 0 of 153600 (0.00%) | - |
| s.128000 (frame 128000) | 0 of 153600 (0.00%) | - |
| s.128700 (frame 128700) | 0 of 153600 (0.00%) | - |
| s.128800 (frame 128800) | 0 of 153600 (0.00%) | - |
| s.128900 (frame 128900) | 0 of 153600 (0.00%) | - |
| s.129000 (frame 129000) | 0 of 153600 (0.00%) | - |
| s.129100 (frame 129100) | 0 of 153600 (0.00%) | - |
| s.129200 (frame 129200) | 0 of 153600 (0.00%) | - |
| s.129600 (frame 129600) | 0 of 153600 (0.00%) | - |
| s.141700 (frame 141700) | 0 of 153600 (0.00%) | - |
| s.151400 (frame 151400) | 0 of 153600 (0.00%) | - |
| s.153900 (frame 153900) | 0 of 153600 (0.00%) | - |
| s.154000 (frame 154000) | 0 of 153600 (0.00%) | - |
| s.154100 (frame 154100) | 0 of 153600 (0.00%) | - |
| s.154200 (frame 154200) | 0 of 153600 (0.00%) | - |
| s.154300 (frame 154300) | 0 of 153600 (0.00%) | - |
| s.154400 (frame 154400) | 0 of 153600 (0.00%) | - |
| s.154500 (frame 154500) | 0 of 153600 (0.00%) | - |
| knockout.154556 (frame 154556) | 0 of 153600 (0.00%) | - |
| knockout.154560 (frame 154560) | 0 of 153600 (0.00%) | - |
| knockout.154562 (frame 154562) | 0 of 153600 (0.00%) | - |
| knockout.154564 (frame 154564) | 0 of 153600 (0.00%) | - |
| knockout.154566 (frame 154566) | 0 of 153600 (0.00%) | - |
| knockout.154568 (frame 154568) | 15 of 153600 (0.01%) | 392,86-406,94 |
| knockout.154570 (frame 154570) | 161 of 153600 (0.10%) | 358,86-425,94 |
| knockout.154572 (frame 154572) | 518 of 153600 (0.34%) | 342,79-423,101 |
| knockout.154574 (frame 154574) | 1038 of 153600 (0.68%) | 344,65-407,117 |
| knockout.154576 (frame 154576) | 1610 of 153600 (1.05%) | 322,63-415,118 |
| knockout.154578 (frame 154578) | 2018 of 153600 (1.31%) | 313,63-412,118 |
| knockout.154580 (frame 154580) | 2095 of 153600 (1.36%) | 311,52-406,129 |
| knockout.154584 (frame 154584) | 944 of 153600 (0.61%) | 258,82-449,99 |
| knockout.154590 (frame 154590) | 0 of 153600 (0.00%) | - |
| s.154600 (frame 154600) | 0 of 153600 (0.00%) | - |
| knockout.154688 (frame 154688) | 0 of 153600 (0.00%) | - |
| knockout.154694 (frame 154694) | 15 of 153600 (0.01%) | 392,86-406,94 |
| s.154700 (frame 154700) | 1038 of 153600 (0.68%) | 343,64-406,116 |
| knockout.154700 (frame 154700) | 1038 of 153600 (0.68%) | 343,64-406,116 |
| knockout.154706 (frame 154706) | 2095 of 153600 (1.36%) | 311,52-406,129 |
| knockout.154900 (frame 154900) | 0 of 153600 (0.00%) | - |
| knockout.154996 (frame 154996) | 0 of 153600 (0.00%) | - |
| s.155000 (frame 155000) | 0 of 153600 (0.00%) | - |
| knockout.155002 (frame 155002) | 0 of 153600 (0.00%) | - |
| knockout.155008 (frame 155008) | 518 of 153600 (0.34%) | 263,73-344,95 |
| knockout.155014 (frame 155014) | 2018 of 153600 (1.31%) | 220,56-319,111 |
| knockout.155020 (frame 155020) | 831 of 153600 (0.54%) | 163,77-342,93 |
| s.156400 (frame 156400) | 0 of 153600 (0.00%) | - |
| s.157650 (frame 157650) | 0 of 153600 (0.00%) | - |
| s.157700 (frame 157700) | 0 of 153600 (0.00%) | - |
| s.157950 (frame 157950) | 0 of 153600 (0.00%) | - |
| knockout.live (frame 157999) | 0 of 153600 (0.00%) | - |

A stub counts only if the game kept running (both runs reached the script's end) and the lost effect is the function's own output (.claude/rules/naming-runtime-evidence.md). Pixels that differ beyond the function's own drawing point at changed game state. A call already in progress when the stub went in finishes normally (a leak breakpoint catches only code reaching the word after the stub).
