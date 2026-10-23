# Runtime trace evidence: capture.jsonl

- recorded 2026-10-23 07:59, map `C:\Users\Trenton\Desktop\bb2-worktrees\ln-trace\build\bb2.map`, 7 events in 191.646 s
- scenario: story mode boot..first fight..rematch menu (story-hitmap-1 script), value watches on globals cited in no_name.tsv
- manifest `capture.manifest.json` (SHA1 f901280ca8e872aa333259b60183a720eca3fd11), capture SHA1 86b4866e3da8901b1bd2e1f9c0ab181954b9d2b0
- canary breakpoint (EXE entry) hit at 0x800836EC in phase boot_intro
- phases covered: boot_intro (frames 0-5490), t_menu (frames 5490-6060), top_menu (frames 6060-6100), t_story (frames 6100-6810), story_text (frames 6810-6910), t_char (frames 6910-6950), char_select (frames 6950-7300), t_weapon (frames 7300-7360), weapon_select (frames 7360-7900), t_stage (frames 7900-8360), stage_card (frames 8360-8800), t_fight (frames 8800-8900), fight (frames 8900-9200), t_ko (frames 9200-10150), rematch_menu (frames 10150-10700); script reached its end

## D_800A34F0 (Z2 watch): 2 hits

Hits per phase: boot_intro 2

Accessing instructions:
- `0xBFC0D864` x1
- `__SN_ENTRY_POINT+0x10` x1

| access | width | value stored |
|---|---|---|
| `D_800A34F0` | 4 | 0x0 (x1) |
| `D_800A34F0` | 4 | 0x0 (x1) |

## D_800A38DC (Z2 watch): 3 hits

Hits per phase: boot_intro 2, t_stage 1

Accessing instructions:
- `0xBFC0D864` x1
- `__SN_ENTRY_POINT+0x10` x1
- `func_800343F0+0x50` x1

| access | width | value stored |
|---|---|---|
| `D_800A38DC` | 2 | 0x0 (x1) |
| `D_800A38DC` | 4 | 0x0 (x1) |
| `D_800A38DC` | 4 | 0x0 (x1) |

## D_800A3910 (Z2 watch): 2 hits

Hits per phase: boot_intro 2

Accessing instructions:
- `0xBFC0D864` x1
- `__SN_ENTRY_POINT+0x10` x1

| access | width | value stored |
|---|---|---|
| `D_800A3910` | 4 | 0x0 (x1) |
| `D_800A3910` | 4 | 0x0 (x1) |

## Stops not credited to any entry

- unmatched-stop: 1 (see the capture)

## value `D_800A38DC`: 15 reads (at phase starts and the end, emulator paused by the script)

| phase | frame | pointer | bytes (hex, memory order) |
|---|---|---|---|
| t_menu (start) | 5490 | - | `00000000` |
| top_menu (start) | 6060 | - | `00000000` |
| t_story (start) | 6100 | - | `00000000` |
| story_text (start) | 6810 | - | `00000000` |
| t_char (start) | 6910 | - | `00000000` |
| char_select (start) | 6950 | - | `00000000` |
| t_weapon (start) | 7300 | - | `00000000` |
| weapon_select (start) | 7360 | - | `00000000` |
| t_stage (start) | 7900 | - | `00000000` |
| stage_card (start) | 8360 | - | `00000000` |
| t_fight (start) | 8800 | - | `00000000` |
| fight (start) | 8900 | - | `00000000` |
| t_ko (start) | 9200 | - | `00000000` |
| rematch_menu (start) | 10150 | - | `00000000` |
| rematch_menu (end) | 10700 | - | `00000000` |

| field | span (bytes to the next symbol) | behaviour | values |
|---|---|---|---|
| `D_800A38DC` (0x800A38DC) | 2 | const | 0x0000 |
| `D_800A38DE` (0x800A38DE) | 1 | const | 0x00 |
| `D_800A38DF` (0x800A38DF) | 1 | const | 0x00 |

## value `D_800A3910`: 15 reads (at phase starts and the end, emulator paused by the script)

| phase | frame | pointer | bytes (hex, memory order) |
|---|---|---|---|
| t_menu (start) | 5490 | - | `00000000` |
| top_menu (start) | 6060 | - | `00000000` |
| t_story (start) | 6100 | - | `00000000` |
| story_text (start) | 6810 | - | `00000000` |
| t_char (start) | 6910 | - | `00000000` |
| char_select (start) | 6950 | - | `00000000` |
| t_weapon (start) | 7300 | - | `00000000` |
| weapon_select (start) | 7360 | - | `00000000` |
| t_stage (start) | 7900 | - | `00000000` |
| stage_card (start) | 8360 | - | `00000000` |
| t_fight (start) | 8800 | - | `00000000` |
| fight (start) | 8900 | - | `00000000` |
| t_ko (start) | 9200 | - | `00000000` |
| rematch_menu (start) | 10150 | - | `00000000` |
| rematch_menu (end) | 10700 | - | `00000000` |

| field | span (bytes to the next symbol) | behaviour | values |
|---|---|---|---|
| `D_800A3910` (0x800A3910) | 2 | const | 0x0000 |
| `D_800A3912` (0x800A3912) | 1 | const | 0x00 |
| `D_800A3913` (0x800A3913) | 1 | const | 0x00 |

## value `D_800A34F0`: 15 reads (at phase starts and the end, emulator paused by the script)

| phase | frame | pointer | bytes (hex, memory order) |
|---|---|---|---|
| t_menu (start) | 5490 | - | `00000000` |
| top_menu (start) | 6060 | - | `00000000` |
| t_story (start) | 6100 | - | `00000000` |
| story_text (start) | 6810 | - | `00000000` |
| t_char (start) | 6910 | - | `00000000` |
| char_select (start) | 6950 | - | `00000000` |
| t_weapon (start) | 7300 | - | `00000000` |
| weapon_select (start) | 7360 | - | `00000000` |
| t_stage (start) | 7900 | - | `00000000` |
| stage_card (start) | 8360 | - | `00000000` |
| t_fight (start) | 8800 | - | `00000000` |
| fight (start) | 8900 | - | `00000000` |
| t_ko (start) | 9200 | - | `00000000` |
| rematch_menu (start) | 10150 | - | `00000000` |
| rematch_menu (end) | 10700 | - | `00000000` |

| field | span (bytes to the next symbol) | behaviour | values |
|---|---|---|---|
| `D_800A34F0` (0x800A34F0) | 2 | const | 0x0000 |
| `D_800A34F2` (0x800A34F2) | 2 | const | 0x0000 |

## value `D_800A3710`: 15 reads (at phase starts and the end, emulator paused by the script)

| phase | frame | pointer | bytes (hex, memory order) |
|---|---|---|---|
| t_menu (start) | 5490 | - | `00000000 00000000 00000000 00000000` |
| top_menu (start) | 6060 | - | `00000000 00000000 00000000 00000000` |
| t_story (start) | 6100 | - | `00000000 00000000 ff00ff00 00000000` |
| story_text (start) | 6810 | - | `00000000 00000000 00000000 00000000` |
| t_char (start) | 6910 | - | `00000000 00000000 00000000 00000000` |
| char_select (start) | 6950 | - | `00000000 00000000 ff00ff00 00000000` |
| t_weapon (start) | 7300 | - | `00000000 00000000 ff00ff00 00000000` |
| weapon_select (start) | 7360 | - | `00000000 00000000 ff00ff00 00000000` |
| t_stage (start) | 7900 | - | `00000000 00000000 ff00ff00 00000000` |
| stage_card (start) | 8360 | - | `00000000 00000100 00000000 00000000` |
| t_fight (start) | 8800 | - | `00000000 00000100 00000000 00000000` |
| fight (start) | 8900 | - | `00000000 00000100 00000000 00000000` |
| t_ko (start) | 9200 | - | `00000000 00000100 00000000 00000000` |
| rematch_menu (start) | 10150 | - | `00000000 00000100 00000000 00000000` |
| rematch_menu (end) | 10700 | - | `00000000 00000100 00000000 00000000` |

| field | span (bytes to the next symbol) | behaviour | values |
|---|---|---|---|
| `g_camera_distance_offset` (0x800A3710) | 2 | const | 0x0000 |
| `D_800A3712` (0x800A3712) | 1 | const | 0x00 |
| `D_800A3713` (0x800A3713) | 1 | const | 0x00 |
| `g_replay_model_draw_search_cursor` (0x800A3714) | 2 | const | 0x0000 |
| `D_800A3716` (0x800A3716) | 2 | varies (2 values) | 0x0000 x9, 0x0001 x6 |
| `g_cd_atv` (0x800A3718) | 4 | varies (2 values) | 0x00000000 x10, 0x00FF00FF x5 |
| `D_800A371C` (0x800A371C) | 4 | const | 0x00000000 |

## value `D_800A9CF8`: 15 reads (at phase starts and the end, emulator paused by the script)

| phase | frame | pointer | bytes (hex, memory order) |
|---|---|---|---|
| t_menu (start) | 5490 | - | `00000000 00000000 00000000 00000000 00000000` |
| top_menu (start) | 6060 | - | `00000000 00000000 00000000 00000000 00000000` |
| t_story (start) | 6100 | - | `00000000 00000000 00000000 00000000 00000000` |
| story_text (start) | 6810 | - | `00000000 00000000 00000000 00000000 00000000` |
| t_char (start) | 6910 | - | `00000000 00000000 00000000 00000000 00000000` |
| char_select (start) | 6950 | - | `00000000 00000000 00000000 00000000 00000000` |
| t_weapon (start) | 7300 | - | `00000000 00000000 00000000 00000000 00000000` |
| weapon_select (start) | 7360 | - | `00000000 00000000 00000000 00000000 00000000` |
| t_stage (start) | 7900 | - | `00000000 00000000 00000000 00000000 00000000` |
| stage_card (start) | 8360 | - | `00000000 00000000 00000000 00000000 00000000` |
| t_fight (start) | 8800 | - | `00000000 00000000 00000000 00000000 00000000` |
| fight (start) | 8900 | - | `00000000 00000000 00000000 00000000 00000000` |
| t_ko (start) | 9200 | - | `00000000 00000000 00000000 00000000 00000000` |
| rematch_menu (start) | 10150 | - | `00000000 00000000 00000000 00000000 00000000` |
| rematch_menu (end) | 10700 | - | `00000000 00000000 00000000 00000000 00000000` |

| field | span (bytes to the next symbol) | behaviour | values |
|---|---|---|---|
| `D_800A9CF8` (0x800A9CF8) | 2 | const | 0x0000 |
| `D_800A9CFA` (0x800A9CFA) | 2 | const | 0x0000 |
| `D_800A9CFC` (0x800A9CFC) | 2 | const | 0x0000 |
| `D_800A9CFE` (0x800A9CFE) | 2 | const | 0x0000 |
| `D_800A9D00` (0x800A9D00) | 4 | const | 0x00000000 |
| `D_800A9D04` (0x800A9D04) | 4 | const | 0x00000000 |
| `D_800A9D08` (0x800A9D08) | 4 | const | 0x00000000 |

