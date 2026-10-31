# Knockout observations

Liveness uses the script end_frame fallback (no RAM counter established). No names or roles are inferred.

## eff_DrawAndAdvanceTwoAnimatedQuadsUntilSecondCounterExpires

Status: **ok**. script reached end_frame 16000 past requested frames 12308,12312,12314,12316,12318,12320,12322,12324,12326,12328,12332,12336,12345,14586,14590,14594,14598,14602,14606,14610,14614,14625 (fallback).

# Stub-and-observe: eff_DrawAndAdvanceTwoAnimatedQuadsUntilSecondCounterExpires against baseline

- scenario: training-practice-ko
- baseline: `docs\naming\runtime\training-practice-ko-orange-stub-1\baseline\capture.manifest.json` (SHA1 62cb4903fb8744ff2af1ce395c3f97f910e7169b), outcome ok, script ended True, q124_complete True
- stubbed: `docs\naming\runtime\training-practice-ko-orange-stub-1\eff_DrawAndAdvanceTwoAnimatedQuadsUntilSecondCounterExpires\capture.manifest.json` (SHA1 4de6fbf35acf1bacd101e42abf8bbe572e0ad2c2), outcome ok, script ended True, q124_complete True
- stubs: `eff_DrawAndAdvanceTwoAnimatedQuadsUntilSecondCounterExpires` (0x80065680) from phase fight (frame 9600), returns 0, intact at the end, called while stubbed in f12000, f14000
- same inputs and script

| screenshot | changed pixels | bounding box (x0,y0-x1,y1) |
|---|---|---|
| boot_intro.last (frame 5489) | 0 of 76800 (0.00%) | - |
| top_menu.first (frame 6050) | 0 of 153600 (0.00%) | - |
| training_menu.first (frame 6300) | 0 of 153600 (0.00%) | - |
| char_select.first (frame 7050) | 0 of 153600 (0.00%) | - |
| weapon_select.first (frame 7450) | 0 of 153600 (0.00%) | - |
| weapon_select.last (frame 8499) | 0 of 153600 (0.00%) | - |
| pause_menu.first (frame 9300) | 0 of 153600 (0.00%) | - |
| pause_menu.on (frame 9450) | 0 of 153600 (0.00%) | - |
| pause_menu.last (frame 9555) | 0 of 153600 (0.00%) | - |
| fight.first (frame 9600) | 0 of 153600 (0.00%) | - |
| fight.10000 (frame 10000) | 0 of 153600 (0.00%) | - |
| fight.10400 (frame 10400) | 0 of 153600 (0.00%) | - |
| fight.10800 (frame 10800) | 0 of 153600 (0.00%) | - |
| f11000.11200 (frame 11200) | 0 of 153600 (0.00%) | - |
| f11000.11600 (frame 11600) | 0 of 153600 (0.00%) | - |
| f12000.first (frame 12000) | 0 of 153600 (0.00%) | - |
| knockout.12308 (frame 12308) | 0 of 153600 (0.00%) | - |
| knockout.12312 (frame 12312) | 0 of 153600 (0.00%) | - |
| knockout.12314 (frame 12314) | 0 of 153600 (0.00%) | - |
| knockout.12316 (frame 12316) | 0 of 153600 (0.00%) | - |
| knockout.12318 (frame 12318) | 0 of 153600 (0.00%) | - |
| knockout.12320 (frame 12320) | 108 of 153600 (0.07%) | 223,101-240,108 |
| knockout.12322 (frame 12322) | 505 of 153600 (0.33%) | 217,96-254,113 |
| knockout.12324 (frame 12324) | 1125 of 153600 (0.73%) | 211,91-266,118 |
| knockout.12326 (frame 12326) | 1868 of 153600 (1.22%) | 204,87-277,122 |
| knockout.12328 (frame 12328) | 2966 of 153600 (1.93%) | 198,83-287,126 |
| knockout.12332 (frame 12332) | 4702 of 153600 (3.06%) | 191,76-302,131 |
| knockout.12336 (frame 12336) | 1574 of 153600 (1.02%) | 191,74-311,133 |
| knockout.12345 (frame 12345) | 0 of 153600 (0.00%) | - |
| f12000.12500 (frame 12500) | 0 of 153600 (0.00%) | - |
| f13000.first (frame 13000) | 0 of 153600 (0.00%) | - |
| f13000.13500 (frame 13500) | 0 of 153600 (0.00%) | - |
| f14000.first (frame 14000) | 0 of 153600 (0.00%) | - |
| f14000.14500 (frame 14500) | 0 of 153600 (0.00%) | - |
| knockout.14586 (frame 14586) | 0 of 153600 (0.00%) | - |
| knockout.14590 (frame 14590) | 0 of 153600 (0.00%) | - |
| knockout.14594 (frame 14594) | 0 of 153600 (0.00%) | - |
| knockout.14598 (frame 14598) | 107 of 153600 (0.07%) | 224,101-241,108 |
| knockout.14602 (frame 14602) | 1131 of 153600 (0.74%) | 212,91-267,118 |
| knockout.14606 (frame 14606) | 2954 of 153600 (1.92%) | 200,83-289,126 |
| knockout.14610 (frame 14610) | 4726 of 153600 (3.08%) | 192,76-303,131 |
| knockout.14614 (frame 14614) | 1665 of 153600 (1.08%) | 191,73-315,134 |
| knockout.14625 (frame 14625) | 0 of 153600 (0.00%) | - |
| f15000.first (frame 15000) | 0 of 153600 (0.00%) | - |
| f15000.last (frame 15999) | 0 of 153600 (0.00%) | - |
| knockout.live (frame 15999) | 0 of 153600 (0.00%) | - |

A stub counts only if the game kept running (both runs reached the script's end) and the lost effect is the function's own output (.claude/rules/naming-runtime-evidence.md). Pixels that differ beyond the function's own drawing point at changed game state. A call already in progress when the stub went in finishes normally (a leak breakpoint catches only code reaching the word after the stub).
