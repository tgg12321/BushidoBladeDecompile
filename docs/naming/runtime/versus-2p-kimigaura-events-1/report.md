# Runtime trace evidence: capture.jsonl

- recorded 2026-10-24 07:27, map `C:\Users\Trenton\Desktop\bb2-worktrees\ln-trace\build\bb2.map`, 189 events in 236.985 s
- scenario: 2P Vs. Mode, Kimigaura Coast ([Map 1], stage screen LEFT, CROSS 7450), both pads attacking every 60 frames from 8430 (the versus-2p-effects-1 fight inputs 70 frames earlier, the same 200 frames after the fight start), no KO by the end at 12730; Z0 on func_80032854 (event code a1), func_800619A4 and func_80061A3C; value D_800A36A4 at every phase start
- manifest `capture.manifest.json` (SHA1 e3b6d444a5145f3e13197f827d151002851fffb6), capture SHA1 97c1903fa67ddcde1138d2c8f26eb0e98a254127
- canary breakpoint (EXE entry) hit at 0x800836EC in phase boot_intro
- phases covered: boot_intro (frames 0-5490), t_menu (frames 5490-6050), top_menu (frames 6050-6260), t_char (frames 6260-6550), char_select (frames 6550-6700), weapon_select (frames 6700-7000), t_stage (frames 7000-7175), stage_select (frames 7175-7450), t_load (frames 7450-8230), fight (frames 8230-12730); script reached its end

## func_80061A3C: 49 hits

Hits per phase: fight 49

Call sites (ra-8):
- `fighter_DispatchFrameEvent+0x348` x29
- `fighter_DispatchFrameEvent+0x23C` x20

| arg | distinct | values (top 4) |
|---|---|---|
| a0 | 1 | 0x801FFC48 |
| a1 | 48 | 0x000007D6, 0x00000EE5, 0x0000014A, 0x00000F16 |
| a2 | 2 | 0x00040000, 0x00020000 |
| a3 | 2 | 0x00000000, 0x00000001 |

## func_80032854: 107 hits

Hits per phase: fight 107

Call sites (ra-8):
- `func_80026DA4+0x51C` x45
- `fighter_DispatchFrameEvent+0x408` x33
- `func_80026DA4+0x2C0` x8
- `fighter_ResolveWeaponContact+0x15C` x4
- `fighter_ResolveWeaponContact+0x170` x4
- `fighter_ResolveWeaponContact+0x258` x3
- `fighter_ResolveWeaponContact+0x26C` x3
- `func_800278C0+0xB4` x2
- `fighter_ResolveBoneHit+0x2CC` x1
- `fighter_ResolveBoneHit+0x310` x1

| arg | distinct | values (top 4) |
|---|---|---|
| a0 | 2 | 0x00000000, 0x00000001 |
| a1 | 12 | 0x00000009, 0x00000007, 0x00000008, 0x0000002D |
| a2 | 8 | 0x801FFE78, 0x801FFC48, 0x1F8003F4, 0x80102408 `D_80102408` |
| a3 | 1 | 0x00000000 |

## func_800619A4: 33 hits

Hits per phase: fight 33

Call sites (ra-8):
- `func_80032854+0x288` x33

| arg | distinct | values (top 4) |
|---|---|---|
| a0 | 1 | 0x801FFC48 |
| a1 | 1 | 0x00000008 |
| a2 | 1 | 0x801FFC48 |
| a3 | 1 | 0x00000000 |

## value `D_800A36A4`: 10 reads (at phase starts and the end, emulator paused by the script)

| phase | frame | pointer | bytes (hex, memory order) |
|---|---|---|---|
| t_menu (start) | 5490 | - | `00000000` |
| top_menu (start) | 6050 | - | `00000000` |
| t_char (start) | 6260 | - | `00000000` |
| char_select (start) | 6550 | - | `00000000` |
| weapon_select (start) | 6700 | - | `00000000` |
| t_stage (start) | 7000 | - | `00000000` |
| stage_select (start) | 7175 | - | `00000000` |
| t_load (start) | 7450 | - | `00000000` |
| fight (start) | 8230 | - | `0b000000` |
| fight (end) | 12730 | - | `0b000000` |

| field | span (bytes to the next symbol) | behaviour | values |
|---|---|---|---|
| `D_800A36A4` (0x800A36A4) | 4 | varies (2 values) | 0x00000000 x8, 0x0000000B x2 |

