# Knockout observations

Liveness uses the script end_frame fallback (no RAM counter established). No names or roles are inferred.

## func_80067740

Status: **ok**. script reached end_frame 9300 past requested frames 8950,8952,8954,8956,8958,8960,8964,8968,8972,8980,8990,9000,9031,9034,9038,9042,9046,9050,9060,9070,9080 (fallback).

# Stub-and-observe: func_80067740 against baseline

- scenario: versus-2p-hidden-suminagashi-double-throw
- baseline: `docs\naming\runtime\versus-2p-hidden-suminagashi-pale-red-stub-1\baseline\capture.manifest.json` (SHA1 9dab29ffea0022326e029fa159f7e3dc23e54a69), outcome ok, script ended True, q124_complete True
- stubbed: `docs\naming\runtime\versus-2p-hidden-suminagashi-pale-red-stub-1\func_80067740\capture.manifest.json` (SHA1 4927c535442d871747387ae4dd6a3386edea3a72), outcome ok, script ended True, q124_complete True
- stubs: `func_80067740` (0x80067740) from phase fight (frame 8805), returns 0, intact at the end, called while stubbed in fight, second_impact
- pokes in both runs (baseline's too, not compared): `0x80106A50` (0x80106A50) = fff33f05 (was 07700000) from phase top_menu (frame 6050), fff33f05 at the end
- same inputs and script

| screenshot | changed pixels | bounding box (x0,y0-x1,y1) |
|---|---|---|
| boot_intro.last (frame 5489) | 0 of 76800 (0.00%) | - |
| top_menu.first (frame 6050) | 0 of 153600 (0.00%) | - |
| top_menu.last (frame 6259) | 0 of 153600 (0.00%) | - |
| char_select.first (frame 6550) | 0 of 153600 (0.00%) | - |
| char_select.pick (frame 7285) | 0 of 153600 (0.00%) | - |
| char_select.last (frame 7374) | 0 of 153600 (0.00%) | - |
| weapon_select.first (frame 7380) | 0 of 153600 (0.00%) | - |
| weapon_select.7650 (frame 7650) | 0 of 153600 (0.00%) | - |
| weapon_select.last (frame 7679) | 0 of 153600 (0.00%) | - |
| t_stage.mid (frame 7780) | 0 of 153600 (0.00%) | - |
| stage_select.first (frame 7850) | 0 of 153600 (0.00%) | - |
| stage_select.last (frame 8039) | 0 of 153600 (0.00%) | - |
| t_load.mid (frame 8400) | 0 of 153600 (0.00%) | - |
| t_load.last (frame 8799) | 0 of 153600 (0.00%) | - |
| fight.first (frame 8805) | 0 of 153600 (0.00%) | - |
| fight.8950 (frame 8950) | 0 of 153600 (0.00%) | - |
| knockout.8950 (frame 8950) | 0 of 153600 (0.00%) | - |
| knockout.8952 (frame 8952) | 0 of 153600 (0.00%) | - |
| knockout.8954 (frame 8954) | 0 of 153600 (0.00%) | - |
| knockout.8956 (frame 8956) | 0 of 153600 (0.00%) | - |
| knockout.8958 (frame 8958) | 57881 of 153600 (37.68%) | 0,0-639,239 |
| knockout.8960 (frame 8960) | 58269 of 153600 (37.94%) | 0,0-639,239 |
| knockout.8964 (frame 8964) | 66913 of 153600 (43.56%) | 0,0-639,239 |
| knockout.8968 (frame 8968) | 65089 of 153600 (42.38%) | 0,0-639,239 |
| knockout.8972 (frame 8972) | 68061 of 153600 (44.31%) | 0,0-639,239 |
| knockout.8980 (frame 8980) | 65588 of 153600 (42.70%) | 0,0-639,239 |
| knockout.8990 (frame 8990) | 67726 of 153600 (44.09%) | 0,0-639,239 |
| knockout.9000 (frame 9000) | 58795 of 153600 (38.28%) | 0,0-639,239 |
| second_impact.9030 (frame 9030) | 63935 of 153600 (41.62%) | 0,0-639,239 |
| knockout.9031 (frame 9031) | 63935 of 153600 (41.62%) | 0,0-639,239 |
| knockout.9034 (frame 9034) | 67245 of 153600 (43.78%) | 0,0-639,239 |
| knockout.9038 (frame 9038) | 70844 of 153600 (46.12%) | 0,0-639,239 |
| second_impact.9040 (frame 9040) | 71341 of 153600 (46.45%) | 0,0-639,239 |
| knockout.9042 (frame 9042) | 71063 of 153600 (46.26%) | 0,0-639,239 |
| knockout.9046 (frame 9046) | 72144 of 153600 (46.97%) | 0,0-639,239 |
| knockout.9050 (frame 9050) | 73844 of 153600 (48.08%) | 0,0-639,239 |
| knockout.9060 (frame 9060) | 71013 of 153600 (46.23%) | 0,0-639,239 |
| second_impact.9070 (frame 9070) | 66511 of 153600 (43.30%) | 0,0-639,239 |
| knockout.9070 (frame 9070) | 66511 of 153600 (43.30%) | 0,0-639,239 |
| knockout.9080 (frame 9080) | 56585 of 153600 (36.84%) | 0,0-639,239 |
| second_impact.9100 (frame 9100) | 17902 of 153600 (11.65%) | 0,0-639,239 |
| second_impact.last (frame 9299) | 1422 of 153600 (0.93%) | 127,43-444,179 |
| knockout.live (frame 9299) | 1422 of 153600 (0.93%) | 127,43-444,179 |

A stub counts only if the game kept running (both runs reached the script's end) and the lost effect is the function's own output (.claude/rules/naming-runtime-evidence.md). Pixels that differ beyond the function's own drawing point at changed game state. A call already in progress when the stub went in finishes normally (a leak breakpoint catches only code reaching the word after the stub).

## func_8006777C

Status: **ok**. script reached end_frame 9300 past requested frames 8950,8952,8954,8956,8958,8960,8964,8968,8972,8980,8990,9000,9031,9034,9038,9042,9046,9050,9060,9070,9080 (fallback).

# Stub-and-observe: func_8006777C against baseline

- scenario: versus-2p-hidden-suminagashi-double-throw
- baseline: `docs\naming\runtime\versus-2p-hidden-suminagashi-pale-red-stub-1\baseline\capture.manifest.json` (SHA1 9dab29ffea0022326e029fa159f7e3dc23e54a69), outcome ok, script ended True, q124_complete True
- stubbed: `docs\naming\runtime\versus-2p-hidden-suminagashi-pale-red-stub-1\func_8006777C\capture.manifest.json` (SHA1 0cf682feaeace044715bd5ecc20db377fb09d75d), outcome ok, script ended True, q124_complete True
- stubs: `func_8006777C` (0x8006777C) from phase fight (frame 8805), returns 0, intact at the end, called while stubbed in second_impact
- pokes in both runs (baseline's too, not compared): `0x80106A50` (0x80106A50) = fff33f05 (was 07700000) from phase top_menu (frame 6050), fff33f05 at the end
- same inputs and script

| screenshot | changed pixels | bounding box (x0,y0-x1,y1) |
|---|---|---|
| boot_intro.last (frame 5489) | 0 of 76800 (0.00%) | - |
| top_menu.first (frame 6050) | 0 of 153600 (0.00%) | - |
| top_menu.last (frame 6259) | 0 of 153600 (0.00%) | - |
| char_select.first (frame 6550) | 0 of 153600 (0.00%) | - |
| char_select.pick (frame 7285) | 0 of 153600 (0.00%) | - |
| char_select.last (frame 7374) | 0 of 153600 (0.00%) | - |
| weapon_select.first (frame 7380) | 0 of 153600 (0.00%) | - |
| weapon_select.7650 (frame 7650) | 0 of 153600 (0.00%) | - |
| weapon_select.last (frame 7679) | 0 of 153600 (0.00%) | - |
| t_stage.mid (frame 7780) | 0 of 153600 (0.00%) | - |
| stage_select.first (frame 7850) | 0 of 153600 (0.00%) | - |
| stage_select.last (frame 8039) | 0 of 153600 (0.00%) | - |
| t_load.mid (frame 8400) | 0 of 153600 (0.00%) | - |
| t_load.last (frame 8799) | 0 of 153600 (0.00%) | - |
| fight.first (frame 8805) | 0 of 153600 (0.00%) | - |
| fight.8950 (frame 8950) | 0 of 153600 (0.00%) | - |
| knockout.8950 (frame 8950) | 0 of 153600 (0.00%) | - |
| knockout.8952 (frame 8952) | 0 of 153600 (0.00%) | - |
| knockout.8954 (frame 8954) | 0 of 153600 (0.00%) | - |
| knockout.8956 (frame 8956) | 0 of 153600 (0.00%) | - |
| knockout.8958 (frame 8958) | 0 of 153600 (0.00%) | - |
| knockout.8960 (frame 8960) | 0 of 153600 (0.00%) | - |
| knockout.8964 (frame 8964) | 0 of 153600 (0.00%) | - |
| knockout.8968 (frame 8968) | 0 of 153600 (0.00%) | - |
| knockout.8972 (frame 8972) | 0 of 153600 (0.00%) | - |
| knockout.8980 (frame 8980) | 0 of 153600 (0.00%) | - |
| knockout.8990 (frame 8990) | 0 of 153600 (0.00%) | - |
| knockout.9000 (frame 9000) | 0 of 153600 (0.00%) | - |
| second_impact.9030 (frame 9030) | 0 of 153600 (0.00%) | - |
| knockout.9031 (frame 9031) | 0 of 153600 (0.00%) | - |
| knockout.9034 (frame 9034) | 0 of 153600 (0.00%) | - |
| knockout.9038 (frame 9038) | 54060 of 153600 (35.20%) | 0,0-639,239 |
| second_impact.9040 (frame 9040) | 56475 of 153600 (36.77%) | 0,0-639,239 |
| knockout.9042 (frame 9042) | 58160 of 153600 (37.86%) | 0,0-639,239 |
| knockout.9046 (frame 9046) | 67340 of 153600 (43.84%) | 0,0-639,239 |
| knockout.9050 (frame 9050) | 69005 of 153600 (44.93%) | 0,0-639,239 |
| knockout.9060 (frame 9060) | 66315 of 153600 (43.17%) | 0,0-639,239 |
| second_impact.9070 (frame 9070) | 62246 of 153600 (40.52%) | 0,0-639,239 |
| knockout.9070 (frame 9070) | 62246 of 153600 (40.52%) | 0,0-639,239 |
| knockout.9080 (frame 9080) | 51276 of 153600 (33.38%) | 0,0-639,239 |
| second_impact.9100 (frame 9100) | 14148 of 153600 (9.21%) | 0,0-639,239 |
| second_impact.last (frame 9299) | 1267 of 153600 (0.82%) | 127,43-444,179 |
| knockout.live (frame 9299) | 1267 of 153600 (0.82%) | 127,43-444,179 |

A stub counts only if the game kept running (both runs reached the script's end) and the lost effect is the function's own output (.claude/rules/naming-runtime-evidence.md). Pixels that differ beyond the function's own drawing point at changed game state. A call already in progress when the stub went in finishes normally (a leak breakpoint catches only code reaching the word after the stub).
