# Runtime trace evidence: capture.jsonl

- recorded 2026-10-24 08:36, map `C:\Users\Trenton\Desktop\bb2-worktrees\ln-trace\build\bb2.map`, 51 events in 192.027 s
- scenario: 2P Vs. Kimigaura Coast, P2 idle, P1 two-press sequences 10 frames apart (double taps, SELECT then a button, held shoulder then SQUARE) every 90 frames from 8430 (one phase per input); write watches on move_kind (+0x6A) of D_80101EC8[0] and [1]
- manifest `capture.manifest.json` (SHA1 a2f7ffaed78c17982556e96e2f19dab9ec548013), capture SHA1 722c58f454881344055162b5d92fc01b7ce6f853
- canary breakpoint (EXE entry) hit at 0x800836EC in phase boot_intro
- phases covered: boot_intro (frames 0-5490), t_menu (frames 5490-6050), top_menu (frames 6050-6260), t_char (frames 6260-6550), char_select (frames 6550-6700), weapon_select (frames 6700-7000), t_stage (frames 7000-7175), stage_select (frames 7175-7450), t_load (frames 7450-8230), fight (frames 8230-8430), in_00_r2_r2 (frames 8430-8520), in_01_l2_l2 (frames 8520-8610), in_02_select_cross (frames 8610-8700), in_03_r2_then_l2 (frames 8700-8790), in_04_l1_l1 (frames 8790-8880), in_05_left_left (frames 8880-8970), in_06_down_down (frames 8970-9060), in_07_up_up (frames 9060-9150), in_08_down_left_cross (frames 9150-9240), in_09_select_square (frames 9240-9330), in_10_select_triangle (frames 9330-9420), in_11_select_circle (frames 9420-9510), in_12_select_r2 (frames 9510-9600), in_13_l2_then_cross (frames 9600-9690), in_14_l2_then_square (frames 9690-9780), in_15_hold_l2_square (frames 9780-9870), in_16_hold_r1_square (frames 9870-9960), in_17_hold_l1_square (frames 9960-10050), in_18_right_right (frames 10050-10140), in_19_right_right_cross (frames 10140-10230); script reached its end

## 0x80101F32 (Z2 watch): 28 hits

Hits per phase: boot_intro 2, t_load 1, fight 3, in_00_r2_r2 2, in_01_l2_l2 2, in_02_select_cross 1, in_04_l1_l1 1, in_14_l2_then_square 1, in_15_hold_l2_square 2, in_16_hold_r1_square 2, in_17_hold_l1_square 3, in_18_right_right 4, in_19_right_right_cross 4

Accessing instructions:
- `func_80021A98+0x118` x26
- `0xBFC0D864` x1
- `__SN_ENTRY_POINT+0x10` x1

| access | width | value stored |
|---|---|---|
| `D_80101F12+0x1E` | 4 | 0x0 (x1) |
| `D_80101F32` | 2 | 0x2 (x1) |
| `D_80101F32` | 2 | 0x5 (x1) |
| `D_80101F32` | 2 | 0x15 (x16) |
| `D_80101F32` | 2 | 0x19 (x2) |
| `D_80101F32` | 2 | 0x1A (x2) |
| `D_80101F32` | 2 | 0x25 (x2) |
| `D_80101F32` | 2 | 0x2A (x1) |
| `D_80101F32` | 2 | 0x2D (x1) |
| `D_80101F12+0x1E` | 4 | 0x0 (x1) |

## 0x8010237E (Z2 watch): 22 hits

Hits per phase: boot_intro 2, t_load 1, fight 3, in_00_r2_r2 2, in_01_l2_l2 1, in_02_select_cross 2, in_03_r2_then_l2 1, in_04_l1_l1 1, in_14_l2_then_square 1, in_15_hold_l2_square 2, in_16_hold_r1_square 1, in_17_hold_l1_square 2, in_18_right_right 1, in_19_right_right_cross 2

Accessing instructions:
- `func_80021A98+0x118` x20
- `0xBFC0D864` x1
- `__SN_ENTRY_POINT+0x10` x1

| access | width | value stored |
|---|---|---|
| `D_8010231A+0x62` | 4 | 0x0 (x1) |
| `D_8010237E` | 2 | 0x15 (x20) |
| `D_8010231A+0x62` | 4 | 0x0 (x1) |

## object_PickUpNearby: 1 hits

Hits per phase: in_00_r2_r2 1

Call sites (ra-8):
- `fighter_Update+0x2CB4` x1

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

