# Runtime trace evidence: capture.jsonl

- recorded 2026-10-23 09:01, map `C:\Users\Trenton\Desktop\bb2-worktrees\ln-trace\build\bb2.map`, 7 events in 180.295 s
- scenario: Vs. CPU mode boot..first fight..pause menu (versus-cpu-hitmap-1 script), value watches on globals cited in no_name.tsv
- manifest `capture.manifest.json` (SHA1 f374e12954b50a8d10c9d91333e57733416c8c48), capture SHA1 f7af68406dd135645338bc2a438e0a8941a7719a
- canary breakpoint (EXE entry) hit at 0x800836EC in phase boot_intro
- phases covered: boot_intro (frames 0-5490), t_menu (frames 5490-6050), top_menu (frames 6050-6200), t_char (frames 6200-6500), char_select (frames 6500-6800), t_weapon (frames 6800-6900), weapon_select (frames 6900-7400), t_load (frames 7400-8350), fight (frames 8350-8650), t_ko (frames 8650-8750), after_ko (frames 8750-9300), t_pause (frames 9300-9400), pause_menu (frames 9400-10000); script reached its end

## D_800A34F0 (Z2 watch): 2 hits

Hits per phase: boot_intro 2

Accessing instructions:
- `0xBFC0D864` x1
- `__SN_ENTRY_POINT+0x10` x1

| access | width | value stored |
|---|---|---|
| `0x800A34F0` | 4 | 0x0 (x1) |
| `0x800A34F0` | 4 | 0x0 (x1) |

## D_800A38DC (Z2 watch): 3 hits

Hits per phase: boot_intro 2, t_load 1

Accessing instructions:
- `0xBFC0D864` x1
- `__SN_ENTRY_POINT+0x10` x1
- `func_800343F0+0x50` x1

| access | width | value stored |
|---|---|---|
| `D_800A38DC` | 2 | 0x1 (x1) |
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

## value `D_800A38DC`: 13 reads (at phase starts and the end, emulator paused by the script)

| phase | frame | pointer | bytes (hex, memory order) |
|---|---|---|---|
| t_menu (start) | 5490 | - | `00000000` |
| top_menu (start) | 6050 | - | `00000000` |
| t_char (start) | 6200 | - | `00000000` |
| char_select (start) | 6500 | - | `00000000` |
| t_weapon (start) | 6800 | - | `00000000` |
| weapon_select (start) | 6900 | - | `00000000` |
| t_load (start) | 7400 | - | `00000000` |
| fight (start) | 8350 | - | `01000000` |
| t_ko (start) | 8650 | - | `01000000` |
| after_ko (start) | 8750 | - | `01000000` |
| t_pause (start) | 9300 | - | `01000000` |
| pause_menu (start) | 9400 | - | `01000000` |
| pause_menu (end) | 10000 | - | `01000000` |

| field | span (bytes to the next symbol) | behaviour | values |
|---|---|---|---|
| `D_800A38DC` (0x800A38DC) | 2 | varies (2 values) | 0x0000 x7, 0x0001 x6 |
| `D_800A38DE` (0x800A38DE) | 1 | const | 0x00 |
| `D_800A38DF` (0x800A38DF) | 1 | const | 0x00 |

## value `D_800A3910`: 13 reads (at phase starts and the end, emulator paused by the script)

| phase | frame | pointer | bytes (hex, memory order) |
|---|---|---|---|
| t_menu (start) | 5490 | - | `00000000` |
| top_menu (start) | 6050 | - | `00000000` |
| t_char (start) | 6200 | - | `00000000` |
| char_select (start) | 6500 | - | `00000000` |
| t_weapon (start) | 6800 | - | `00000000` |
| weapon_select (start) | 6900 | - | `00000000` |
| t_load (start) | 7400 | - | `00000000` |
| fight (start) | 8350 | - | `00000000` |
| t_ko (start) | 8650 | - | `00000000` |
| after_ko (start) | 8750 | - | `00000000` |
| t_pause (start) | 9300 | - | `00000000` |
| pause_menu (start) | 9400 | - | `00000000` |
| pause_menu (end) | 10000 | - | `00000000` |

| field | span (bytes to the next symbol) | behaviour | values |
|---|---|---|---|
| `D_800A3910` (0x800A3910) | 2 | const | 0x0000 |
| `D_800A3912` (0x800A3912) | 1 | const | 0x00 |
| `D_800A3913` (0x800A3913) | 1 | const | 0x00 |

## value `D_800A34F0`: 13 reads (at phase starts and the end, emulator paused by the script)

| phase | frame | pointer | bytes (hex, memory order) |
|---|---|---|---|
| t_menu (start) | 5490 | - | `00000000` |
| top_menu (start) | 6050 | - | `00000000` |
| t_char (start) | 6200 | - | `00000000` |
| char_select (start) | 6500 | - | `00000000` |
| t_weapon (start) | 6800 | - | `00000000` |
| weapon_select (start) | 6900 | - | `00000000` |
| t_load (start) | 7400 | - | `00000000` |
| fight (start) | 8350 | - | `00000000` |
| t_ko (start) | 8650 | - | `00000000` |
| after_ko (start) | 8750 | - | `00000000` |
| t_pause (start) | 9300 | - | `00000000` |
| pause_menu (start) | 9400 | - | `00000000` |
| pause_menu (end) | 10000 | - | `00000000` |

| field | span (bytes to the next symbol) | behaviour | values |
|---|---|---|---|
| `0x800A34F0` (0x800A34F0) | 4 | const | 0x00000000 |

## value `D_800A3710`: 13 reads (at phase starts and the end, emulator paused by the script)

| phase | frame | pointer | bytes (hex, memory order) |
|---|---|---|---|
| t_menu (start) | 5490 | - | `00000000 00000000 00000000 00000000` |
| top_menu (start) | 6050 | - | `00000000 00000000 00000000 00000000` |
| t_char (start) | 6200 | - | `00000000 00000000 ff00ff00 00000000` |
| char_select (start) | 6500 | - | `00000000 00000000 00000000 00000000` |
| t_weapon (start) | 6800 | - | `00000000 00000000 ff00ff00 00000000` |
| weapon_select (start) | 6900 | - | `00000000 00000000 ff00ff00 00000000` |
| t_load (start) | 7400 | - | `00000000 00000000 ff00ff00 00000000` |
| fight (start) | 8350 | - | `00000000 00000100 00000000 00000000` |
| t_ko (start) | 8650 | - | `00000000 00000100 00000000 00000000` |
| after_ko (start) | 8750 | - | `00000000 00000100 00000000 00000000` |
| t_pause (start) | 9300 | - | `00000000 00000100 00000000 00000000` |
| pause_menu (start) | 9400 | - | `00000000 00000100 00000000 00000000` |
| pause_menu (end) | 10000 | - | `00000000 00000100 00000000 00000000` |

| field | span (bytes to the next symbol) | behaviour | values |
|---|---|---|---|
| `g_camera_distance_offset` (0x800A3710) | 2 | const | 0x0000 |
| `D_800A3712` (0x800A3712) | 1 | const | 0x00 |
| `D_800A3713` (0x800A3713) | 1 | const | 0x00 |
| `g_replay_model_draw_search_cursor` (0x800A3714) | 2 | const | 0x0000 |
| `D_800A3716` (0x800A3716) | 2 | varies (2 values) | 0x0000 x7, 0x0001 x6 |
| `g_cd_atv` (0x800A3718) | 4 | varies (2 values) | 0x00000000 x9, 0x00FF00FF x4 |
| `D_800A371C` (0x800A371C) | 4 | const | 0x00000000 |

## value `D_800A9CF8`: 13 reads (at phase starts and the end, emulator paused by the script)

| phase | frame | pointer | bytes (hex, memory order) |
|---|---|---|---|
| t_menu (start) | 5490 | - | `00000000 00000000 00000000 00000000 00000000` |
| top_menu (start) | 6050 | - | `00000000 00000000 00000000 00000000 00000000` |
| t_char (start) | 6200 | - | `00000000 00000000 00000000 00000000 00000000` |
| char_select (start) | 6500 | - | `00000000 00000000 00000000 00000000 00000000` |
| t_weapon (start) | 6800 | - | `00000000 00000000 00000000 00000000 00000000` |
| weapon_select (start) | 6900 | - | `00000000 00000000 00000000 00000000 00000000` |
| t_load (start) | 7400 | - | `00000000 00000000 00000000 00000000 00000000` |
| fight (start) | 8350 | - | `00000000 00000000 00000000 00000000 00000000` |
| t_ko (start) | 8650 | - | `00000000 00000000 00000000 00000000 00000000` |
| after_ko (start) | 8750 | - | `00000000 00000000 00000000 00000000 00000000` |
| t_pause (start) | 9300 | - | `00000000 00000000 00000000 00000000 00000000` |
| pause_menu (start) | 9400 | - | `00000000 00000000 00000000 00000000 00000000` |
| pause_menu (end) | 10000 | - | `00000000 00000000 00000000 00000000 00000000` |

| field | span (bytes to the next symbol) | behaviour | values |
|---|---|---|---|
| `D_800A9CF8` (0x800A9CF8) | 2 | const | 0x0000 |
| `D_800A9CFA` (0x800A9CFA) | 2 | const | 0x0000 |
| `D_800A9CFC` (0x800A9CFC) | 2 | const | 0x0000 |
| `D_800A9CFE` (0x800A9CFE) | 2 | const | 0x0000 |
| `D_800A9D00` (0x800A9D00) | 4 | const | 0x00000000 |
| `D_800A9D04` (0x800A9D04) | 4 | const | 0x00000000 |
| `D_800A9D08` (0x800A9D08) | 4 | const | 0x00000000 |

## value `D_80101EC8`: 13 reads (at phase starts and the end, emulator paused by the script)

| phase | frame | pointer | bytes (hex, memory order) |
|---|---|---|---|
| t_menu (start) | 5490 | - | `00000000 00000000 00000000 00000000` |
| top_menu (start) | 6050 | - | `00000000 00000000 00000000 00000000` |
| t_char (start) | 6200 | - | `00000000 00000000 00000000 00000000` |
| char_select (start) | 6500 | - | `00000000 00000000 00000000 00000000` |
| t_weapon (start) | 6800 | - | `00000000 00000000 00000000 00000000` |
| weapon_select (start) | 6900 | - | `00000000 00000000 00000000 00000000` |
| t_load (start) | 7400 | - | `00000000 00000000 00000000 00000000` |
| fight (start) | 8350 | - | `14231080 00000000 00100000 03000000` |
| t_ko (start) | 8650 | - | `14231080 00000000 00100000 03000000` |
| after_ko (start) | 8750 | - | `14231080 00000000 00100000 03000000` |
| t_pause (start) | 9300 | - | `14231080 00000000 00100000 03000000` |
| pause_menu (start) | 9400 | - | `14231080 00000000 00100000 03000000` |
| pause_menu (end) | 10000 | - | `14231080 00000000 00100000 03000000` |

| field | span (bytes to the next symbol) | behaviour | values |
|---|---|---|---|
| `D_80101EC8` (0x80101EC8) | 16 | varies (2 values) | 0x00000000 x7, 0x3000010000000000080102314 x6 |

