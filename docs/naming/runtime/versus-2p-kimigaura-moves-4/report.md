# Runtime trace evidence: capture.jsonl

- recorded 2026-10-24 08:29, map `C:\Users\Trenton\Desktop\bb2-worktrees\ln-trace\build\bb2.map`, 96 events in 193.666 s
- scenario: 2P Vs. Kimigaura Coast, P2 idle, P1 RIGHT+button and direction+SELECT/L1/R1 combos, 40-frame holds every 90 frames from 8430 (one phase per input); write watches on move_kind (+0x6A) of D_80101EC8[0] and [1]
- manifest `capture.manifest.json` (SHA1 7e21714e570c90dd42a2130bb5d3889687aee474), capture SHA1 8f5e5ad346c715028081da14e4df68f80d7433b1
- canary breakpoint (EXE entry) hit at 0x800836EC in phase boot_intro
- phases covered: boot_intro (frames 0-5490), t_menu (frames 5490-6050), top_menu (frames 6050-6260), t_char (frames 6260-6550), char_select (frames 6550-6700), weapon_select (frames 6700-7000), t_stage (frames 7000-7175), stage_select (frames 7175-7450), t_load (frames 7450-8230), fight (frames 8230-8430), in_00_right_r2 (frames 8430-8520), in_01_right_l2 (frames 8520-8610), in_02_right_l1 (frames 8610-8700), in_03_right_r1 (frames 8700-8790), in_04_right_select (frames 8790-8880), in_05_down_select (frames 8880-8970), in_06_up_select (frames 8970-9060), in_07_left_select (frames 9060-9150), in_08_up_r1 (frames 9150-9240), in_09_down_l1 (frames 9240-9330), in_10_left_l1 (frames 9330-9420), in_11_left_r1 (frames 9420-9510), in_12_hold_r2 (frames 9510-9600), in_13_hold_l2 (frames 9600-9690), in_14_hold_l1 (frames 9690-9780), in_15_hold_select (frames 9780-9870), in_16_right_cross (frames 9870-9960), in_17_right_square (frames 9960-10050), in_18_right_triangle (frames 10050-10140), in_19_right_circle (frames 10140-10230); script reached its end

## 0x80101F32 (Z2 watch): 61 hits

Hits per phase: boot_intro 2, t_load 1, fight 3, in_00_right_r2 2, in_01_right_l2 4, in_02_right_l1 5, in_03_right_r1 2, in_04_right_select 4, in_05_down_select 4, in_06_up_select 4, in_07_left_select 4, in_08_up_r1 4, in_09_down_l1 5, in_10_left_l1 5, in_11_left_r1 4, in_12_hold_r2 2, in_13_hold_l2 2, in_14_hold_l1 2, in_15_hold_select 1, in_17_right_square 1

Accessing instructions:
- `func_80021A98+0x118` x59
- `0xBFC0D864` x1
- `__SN_ENTRY_POINT+0x10` x1

| access | width | value stored |
|---|---|---|
| `D_80101F12+0x1E` | 4 | 0x0 (x1) |
| `D_80101F32` | 2 | 0x2 (x1) |
| `D_80101F32` | 2 | 0x5 (x1) |
| `D_80101F32` | 2 | 0x13 (x3) |
| `D_80101F32` | 2 | 0x15 (x36) |
| `D_80101F32` | 2 | 0x19 (x9) |
| `D_80101F32` | 2 | 0x2A (x2) |
| `D_80101F32` | 2 | 0x2D (x1) |
| `D_80101F32` | 2 | 0x30 (x3) |
| `D_80101F32` | 2 | 0x31 (x3) |
| `D_80101F12+0x1E` | 4 | 0x0 (x1) |

## 0x8010237E (Z2 watch): 33 hits

Hits per phase: boot_intro 2, t_load 1, fight 3, in_00_right_r2 2, in_01_right_l2 1, in_02_right_l1 2, in_03_right_r1 1, in_04_right_select 2, in_05_down_select 1, in_06_up_select 2, in_07_left_select 2, in_08_up_r1 1, in_09_down_l1 2, in_10_left_l1 1, in_11_left_r1 2, in_12_hold_r2 1, in_13_hold_l2 2, in_14_hold_l1 1, in_15_hold_select 2, in_16_right_cross 2

Accessing instructions:
- `func_80021A98+0x118` x31
- `0xBFC0D864` x1
- `__SN_ENTRY_POINT+0x10` x1

| access | width | value stored |
|---|---|---|
| `D_8010231A+0x62` | 4 | 0x0 (x1) |
| `D_8010237E` | 2 | 0x15 (x31) |
| `D_8010231A+0x62` | 4 | 0x0 (x1) |

## object_PickUpNearby: 2 hits

Hits per phase: in_00_right_r2 1, in_12_hold_r2 1

Call sites (ra-8):
- `fighter_Update+0x2CB4` x2

| arg | distinct | values (top 4) |
|---|---|---|
| a0 | 1 | 0x80101EC8 `D_80101EC8` |
| a1 | 1 | 0x1F800380 |
| a2 | 1 | 0x00000002 |
| a3 | 1 | 0x00000001 |

## Armed, never hit in this scenario

- `func_80032854`
- `func_80032064`
- `object_ThrowQueuedKind`
- `object_SpawnFromFighter`

