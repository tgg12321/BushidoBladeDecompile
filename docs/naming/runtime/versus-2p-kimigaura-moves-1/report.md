# Runtime trace evidence: capture.jsonl

- recorded 2026-10-24 08:15, map `C:\Users\Trenton\Desktop\bb2-worktrees\ln-trace\build\bb2.map`, 91 events in 193.218 s
- scenario: 2P Vs. Kimigaura Coast, P2 idle, P1 one face/shoulder button or two-button combo every 90 frames from 8430 (one phase per input); write watches on move_kind (+0x6A) of D_80101EC8[0] and [1]
- manifest `capture.manifest.json` (SHA1 9419d9e717067dafabf3a9857780ca85882cdf5a), capture SHA1 49037fc14f76647f7b7f19d8b8bef2452c3eb379
- canary breakpoint (EXE entry) hit at 0x800836EC in phase boot_intro
- phases covered: boot_intro (frames 0-5490), t_menu (frames 5490-6050), top_menu (frames 6050-6260), t_char (frames 6260-6550), char_select (frames 6550-6700), weapon_select (frames 6700-7000), t_stage (frames 7000-7175), stage_select (frames 7175-7450), t_load (frames 7450-8230), fight (frames 8230-8430), in_00_square (frames 8430-8520), in_01_triangle (frames 8520-8610), in_02_circle (frames 8610-8700), in_03_cross (frames 8700-8790), in_04_l1 (frames 8790-8880), in_05_l2 (frames 8880-8970), in_06_r1 (frames 8970-9060), in_07_r2 (frames 9060-9150), in_08_r2_square (frames 9150-9240), in_09_r2_triangle (frames 9240-9330), in_10_r2_circle (frames 9330-9420), in_11_r2_cross (frames 9420-9510), in_12_l2_square (frames 9510-9600), in_13_l2_triangle (frames 9600-9690), in_14_l2_circle (frames 9690-9780), in_15_l2_cross (frames 9780-9870), in_16_l1_square (frames 9870-9960), in_17_l1_cross (frames 9960-10050), in_18_r1_square (frames 10050-10140), in_19_r1_cross (frames 10140-10230); script reached its end

## 0x80101F32 (Z2 watch): 46 hits

Hits per phase: boot_intro 2, t_load 1, fight 3, in_00_square 3, in_01_triangle 1, in_02_circle 2, in_03_cross 2, in_04_l1 2, in_05_l2 2, in_06_r1 2, in_07_r2 2, in_08_r2_square 2, in_09_r2_triangle 2, in_10_r2_circle 2, in_11_r2_cross 2, in_12_l2_square 3, in_13_l2_triangle 1, in_14_l2_circle 2, in_15_l2_cross 2, in_16_l1_square 2, in_17_l1_cross 2, in_18_r1_square 2, in_19_r1_cross 2

Accessing instructions:
- `func_80021A98+0x118` x44
- `0xBFC0D864` x1
- `__SN_ENTRY_POINT+0x10` x1

| access | width | value stored |
|---|---|---|
| `D_80101F12+0x1E` | 4 | 0x0 (x1) |
| `D_80101F32` | 2 | 0x2 (x7) |
| `D_80101F32` | 2 | 0x15 (x30) |
| `D_80101F32` | 2 | 0x25 (x2) |
| `D_80101F32` | 2 | 0x2A (x5) |
| `D_80101F12+0x1E` | 4 | 0x0 (x1) |

## 0x8010237E (Z2 watch): 38 hits

Hits per phase: boot_intro 2, t_load 1, fight 3, in_00_square 2, in_01_triangle 1, in_02_circle 2, in_03_cross 1, in_04_l1 2, in_05_l2 1, in_06_r1 2, in_07_r2 2, in_08_r2_square 1, in_09_r2_triangle 2, in_10_r2_circle 1, in_11_r2_cross 2, in_12_l2_square 1, in_13_l2_triangle 2, in_14_l2_circle 2, in_15_l2_cross 2, in_16_l1_square 1, in_17_l1_cross 2, in_18_r1_square 1, in_19_r1_cross 2

Accessing instructions:
- `func_80021A98+0x118` x36
- `0xBFC0D864` x1
- `__SN_ENTRY_POINT+0x10` x1

| access | width | value stored |
|---|---|---|
| `D_8010231A+0x62` | 4 | 0x0 (x1) |
| `D_8010237E` | 2 | 0x3 (x1) |
| `D_8010237E` | 2 | 0x15 (x35) |
| `D_8010231A+0x62` | 4 | 0x0 (x1) |

## object_PickUpNearby: 5 hits

Hits per phase: in_07_r2 1, in_08_r2_square 1, in_09_r2_triangle 1, in_10_r2_circle 1, in_11_r2_cross 1

Call sites (ra-8):
- `fighter_Update+0x2CB4` x5

| arg | distinct | values (top 4) |
|---|---|---|
| a0 | 1 | 0x80101EC8 `D_80101EC8` |
| a1 | 1 | 0x1F800380 |
| a2 | 1 | 0x00000002 |
| a3 | 1 | 0x00000001 |

## func_80032854: 2 hits

Hits per phase: in_14_l2_circle 2

Call sites (ra-8):
- `func_800278C0+0xB4` x1
- `fighter_ResolveBoneHit+0x860` x1

| arg | distinct | values (top 4) |
|---|---|---|
| a0 | 2 | 0x00000000, 0x00000001 |
| a1 | 2 | 0x00000022, 0x00000003 |
| a2 | 1 | 0x1F8001B0 |
| a3 | 1 | 0x00000000 |

## Armed, never hit in this scenario

- `func_80032064`
- `object_ThrowQueuedKind`
- `object_SpawnFromFighter`

