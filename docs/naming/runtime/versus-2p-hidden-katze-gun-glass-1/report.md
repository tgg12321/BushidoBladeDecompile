# Runtime trace evidence: capture.jsonl

- recorded 2026-10-25 19:37, map `C:\Users\Trenton\Desktop\bb2-worktrees\ln-trace\build\bb2.map`, 22 events in 217.993 s
- manifest `capture.manifest.json` (SHA1 7a871dd64adb20532c94a7ec00a5cef0e80bcfd5), capture SHA1 606a297effe4de27ba3f5059c30f93c2e294095a
- canary breakpoint (EXE entry) hit at 0x800836EC in phase boot_intro
- POKED: `0x80106A50` (0x80106A50) set to fff33f05 (was 07700000) when top_menu started (frame 6050): every hit and read after that is from a modified game
- phases covered: boot_intro (frames 0-5490), t_menu (frames 5490-6050), top_menu (frames 6050-6260), t_char (frames 6260-6550), char_select (frames 6550-7380), weapon_select (frames 7380-7680), t_stage (frames 7680-7850), stage_select (frames 7850-8040), t_load (frames 8040-8805), rounds (frames 8805-11800); script reached its end

## fighter_CastWeaponSegment: 7 hits

Hits per phase: rounds 7

Call sites (ra-8):
- `fighter_ProcessWeaponBodyHits+0x220` x7

| arg | distinct | values (top 4) |
|---|---|---|
| a0 | 1 | 0x80102314 `D_801021E2+0x132` |
| a1 | 1 | 0x801FFE10 |
| a2 | 1 | 0x801FFE14 |
| a3 | 1 | 0x00000000 |

## func_80032854: 15 hits

Hits per phase: rounds 15

Call sites (ra-8):
- `fighter_CastWeaponSegment+0x1E4` x7
- `fighter_Update+0x2D98` x7
- `fighter_CastWeaponSegment+0x640` x1

| arg | distinct | values (top 4) |
|---|---|---|
| a0 | 1 | 0x00000000 |
| a1 | 3 | 0x0000000B, 0x00000031, 0x0000000A |
| a2 | 3 | 0x1F800380, 0x80101FBC `D_80101FBC`, 0x1F8003B8 |
| a3 | 2 | 0x00000000, 0x1F8003B0 |

