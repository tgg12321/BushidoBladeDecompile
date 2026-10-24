# Runtime trace evidence: capture.jsonl

- recorded 2026-10-24 08:08, map `C:\Users\Trenton\Desktop\bb2-worktrees\ln-trace\build\bb2.map`, 124 events in 194.503 s
- scenario: 2P Vs. Kimigaura Coast, P2 idle, P1 direction+button combos every 90 frames from 8430 (one phase per input); write watches on move_kind (+0x6A) of D_80101EC8[0] and [1]
- manifest `capture.manifest.json` (SHA1 f053744f91fb735ecebb5ce17e49b53b62c0866f), capture SHA1 2ad22c3470b480d6cfc91e7c25009e967b58438e
- canary breakpoint (EXE entry) hit at 0x800836EC in phase boot_intro
- phases covered: boot_intro (frames 0-5490), t_menu (frames 5490-6050), top_menu (frames 6050-6260), t_char (frames 6260-6550), char_select (frames 6550-6700), weapon_select (frames 6700-7000), t_stage (frames 7000-7175), stage_select (frames 7175-7450), t_load (frames 7450-8230), fight (frames 8230-8430), in_00_up_cross (frames 8430-8520), in_01_down_cross (frames 8520-8610), in_02_left_cross (frames 8610-8700), in_03_up_square (frames 8700-8790), in_04_up_triangle (frames 8790-8880), in_05_up_circle (frames 8880-8970), in_06_down_square (frames 8970-9060), in_07_down_triangle (frames 9060-9150), in_08_down_circle (frames 9150-9240), in_09_left_square (frames 9240-9330), in_10_left_triangle (frames 9330-9420), in_11_left_circle (frames 9420-9510), in_12_up_r2 (frames 9510-9600), in_13_down_r2 (frames 9600-9690), in_14_left_r2 (frames 9690-9780), in_15_up_l2 (frames 9780-9870), in_16_down_l2 (frames 9870-9960), in_17_left_l2 (frames 9960-10050), in_18_up_l1 (frames 10050-10140), in_19_down_r1 (frames 10140-10230); script reached its end

## 0x80101F32 (Z2 watch): 87 hits

Hits per phase: boot_intro 2, t_load 1, fight 3, in_00_up_cross 4, in_01_down_cross 4, in_02_left_cross 4, in_03_up_square 4, in_04_up_triangle 4, in_05_up_circle 4, in_06_down_square 4, in_07_down_triangle 4, in_08_down_circle 4, in_09_left_square 4, in_10_left_triangle 4, in_11_left_circle 4, in_12_up_r2 4, in_13_down_r2 4, in_14_left_r2 4, in_15_up_l2 4, in_16_down_l2 4, in_17_left_l2 4, in_18_up_l1 5, in_19_down_r1 4

Accessing instructions:
- `func_80021A98+0x118` x85
- `0xBFC0D864` x1
- `__SN_ENTRY_POINT+0x10` x1

| access | width | value stored |
|---|---|---|
| `D_80101F12+0x1E` | 4 | 0x0 (x1) |
| `D_80101F32` | 2 | 0x13 (x1) |
| `D_80101F32` | 2 | 0x15 (x62) |
| `D_80101F32` | 2 | 0x19 (x20) |
| `D_80101F32` | 2 | 0x30 (x1) |
| `D_80101F32` | 2 | 0x31 (x1) |
| `D_80101F12+0x1E` | 4 | 0x0 (x1) |

## 0x8010237E (Z2 watch): 37 hits

Hits per phase: boot_intro 2, t_load 1, fight 3, in_00_up_cross 2, in_01_down_cross 1, in_02_left_cross 2, in_03_up_square 1, in_04_up_triangle 2, in_05_up_circle 1, in_06_down_square 2, in_07_down_triangle 2, in_08_down_circle 1, in_09_left_square 2, in_10_left_triangle 1, in_11_left_circle 2, in_12_up_r2 1, in_13_down_r2 2, in_14_left_r2 1, in_15_up_l2 2, in_16_down_l2 2, in_17_left_l2 1, in_18_up_l1 2, in_19_down_r1 1

Accessing instructions:
- `func_80021A98+0x118` x35
- `0xBFC0D864` x1
- `__SN_ENTRY_POINT+0x10` x1

| access | width | value stored |
|---|---|---|
| `D_8010231A+0x62` | 4 | 0x0 (x1) |
| `D_8010237E` | 2 | 0x15 (x35) |
| `D_8010231A+0x62` | 4 | 0x0 (x1) |

## Armed, never hit in this scenario

- `func_80032854`
- `func_80032064`
- `object_ThrowQueuedKind`
- `object_PickUpNearby`
- `object_SpawnFromFighter`

