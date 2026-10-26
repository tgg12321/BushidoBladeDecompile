# Runtime trace evidence: capture.jsonl

- recorded 2026-10-26 06:25, map `C:\Users\Trenton\Desktop\bb2-worktrees\ln-trace\build\bb2.map`, 94 events in 179.077 s
- scenario: Versus 2P with hidden SUMINAGASHI (unlock poke 0x80106A50 at top_menu, both runs), lib-versus-2p-hidden-suminagashi-1 script cut at 9450 (two pale-red starts, 8949 and 9185); eff_StartRecycledVectorCommandAndJitteredBillboardsWithPaleRedLight 0x80061B08 poked to b 0x80061B80 / addiu v1, s0, 1 at t_load so every start takes the second slot (status 21, command 0x15)
- manifest `capture.manifest.json` (SHA1 be18c33798fa9f59142c4260999fcbe5893b67a7), capture SHA1 ea9d648b42e0517cd270bfa260f89a2ff0e2832c
- canary breakpoint (EXE entry) hit at 0x800836EC in phase boot_intro
- POKED: `0x80106A50` (0x80106A50) set to fff33f05 (was 07700000) when top_menu started (frame 6050): every hit and read after that is from a modified game
- POKED: `0x80061B08` (0x80061B08) set to 1d00001001000326 (was 0b00401000000000) when t_load started (frame 8040): every hit and read after that is from a modified game
- phases covered: boot_intro (frames 0-5490), t_menu (frames 5490-6050), top_menu (frames 6050-6260), t_char (frames 6260-6550), char_select (frames 6550-7380), weapon_select (frames 7380-7680), t_stage (frames 7680-7850), stage_select (frames 7850-8040), t_load (frames 8040-8805), fight (frames 8805-9450); script reached its end

## eff_StartRecycledVectorCommandAndJitteredBillboardsWithPaleRedLight: 2 hits

Hits per phase: fight 2

Call sites (ra-8):
- `func_80032854+0x318` x2

| arg | distinct | values (top 4) |
|---|---|---|
| a0 | 1 | 0x1F8001BC |
| a1 | 1 | 0x800A37E8 `D_800A37E8` |
| a2 | 2 | 0x0000006A, 0x00000074 |
| a3 | 1 | 0x00000000 |

## func_80066FF8: 2 hits

Hits per phase: fight 2

Call sites (ra-8):
- `eff_StartCommand+0xBC` x2

| arg | distinct | values (top 4) |
|---|---|---|
| a0 | 1 | 0x00000015 |
| a1 | 1 | 0x00001000 |
| a2 | 2 | 0x0000006A, 0x00000074 |
| a3 | 1 | 0x00000000 |

## func_8006702C: 2 hits

Hits per phase: fight 2

Call sites (ra-8):
- `eff_StepCommand+0xCC` x2

| arg | distinct | values (top 4) |
|---|---|---|
| a0 | 1 | 0x1F800030 |
| a1 | 1 | 0x8009BB74 `D_8009BB74` |
| a2 | 1 | 0x1F800030 |
| a3 | 1 | 0xFF000000 |

## func_8006777C: 88 hits

Hits per phase: fight 88

Call sites (ra-8):
- `eff_StepCommand+0xCC` x88

| arg | distinct | values (top 4) |
|---|---|---|
| a0 | 1 | 0x1F800030 |
| a1 | 1 | 0x8009BB74 `D_8009BB74` |
| a2 | 1 | 0x1F800030 |
| a3 | 3 | 0x1F8000AC, 0x00008000, 0xFF000000 |

## Armed, never hit in this scenario

- `func_80066F90`
- `func_80066FC4`
- `func_80067740`

