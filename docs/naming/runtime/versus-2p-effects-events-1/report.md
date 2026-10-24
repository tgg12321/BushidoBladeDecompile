# Runtime trace evidence: capture.jsonl

- recorded 2026-10-24 06:47, map `C:\Users\Trenton\Desktop\bb2-worktrees\ln-trace\build\bb2.map`, 835 events in 276.411 s
- manifest `capture.manifest.json` (SHA1 b00f1af21e228dbf0d7e2c3fc884afc40d3e2d29), capture SHA1 2fa05558de81441698d8cd462bd6faa31278cf78
- canary breakpoint (EXE entry) hit at 0x800836EC in phase boot_intro
- phases covered: boot_intro (frames 0-5490), t_menu (frames 5490-6050), top_menu (frames 6050-6260), t_char (frames 6260-6550), char_select (frames 6550-6700), weapon_select (frames 6700-7000), t_stage (frames 7000-7175), stage_select (frames 7175-7525), t_load (frames 7525-8300), fight (frames 8300-12513), t_ko (frames 12513-12970), rematch_menu (frames 12970-13970); script reached its end

## D_800A3910 (Z2 watch): 708 hits

Hits per phase: boot_intro 2, fight 706

Accessing instructions:
- `func_80026DA4+0x534` x620
- `func_80032854+0x24C` x45
- `func_80032854+0x27C` x24
- `func_80032854+0x21C` x9
- `func_80032854+0x1EC` x6
- `func_8002C61C+0x1E4` x2
- `0xBFC0D864` x1
- `__SN_ENTRY_POINT+0x10` x1

| access | width | value stored |
|---|---|---|
| `D_800A3910` | 2 | 0x0 (x84) |
| `D_800A3910` | 2 | 0x1 (x82) |
| `D_800A3910` | 2 | 0x2 (x76) |
| `D_800A3910` | 2 | 0x3 (x76) |
| `D_800A3910` | 2 | 0x4 (x76) |
| `D_800A3910` | 2 | 0x5 (x78) |
| `D_800A3910` | 2 | 0x6 (x54) |
| `D_800A3910` | 2 | 0x7 (x54) |
| `D_800A3910` | 2 | 0x8 (x54) |
| `D_800A3910` | 2 | 0x9 (x9) |
| `D_800A3910` | 2 | 0xA (x9) |
| `D_800A3910` | 2 | 0xB (x9) |
| `D_800A3910` | 2 | 0xC (x9) |
| `D_800A3910` | 2 | 0xD (x9) |
| `D_800A3910` | 2 | 0xE (x9) |
| `D_800A3910` | 2 | 0xF (x9) |

## func_80032854: 127 hits

Hits per phase: fight 121, t_ko 6

Call sites (ra-8):
- `func_80026DA4+0x51C` x84
- `func_80026DA4+0x2C0` x12
- `replay_PlayFrame+0x4E0` x6
- `fighter_ResolveWeaponContact+0x258` x3
- `fighter_ResolveWeaponContact+0x26C` x3
- `fighter_ResolveWeaponContact+0x15C` x3
- `fighter_ResolveWeaponContact+0x170` x3
- `func_800278C0+0xB4` x3
- `object_UpdatePool+0x824` x3
- `fighter_ResolveBoneHit+0x2CC` x1

| arg | distinct | values (top 4) |
|---|---|---|
| a0 | 2 | 0x00000000, 0x00000001 |
| a1 | 16 | 0x00000007, 0x00000008, 0x0000002D, 0x00000006 |
| a2 | 10 | 0x801FFE78, 0x1F8003F4, 0x80102408 `D_80102408`, 0x801FFD18 |
| a3 | 3 | 0x00000000, 0x801FFD28, 0x801FFC60 |

