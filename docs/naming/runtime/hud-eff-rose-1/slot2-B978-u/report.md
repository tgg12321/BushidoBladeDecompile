# Runtime trace evidence: capture.jsonl

- recorded 2026-11-03 20:10, map `C:\Users\Trenton\Desktop\bb2-worktrees\ln-trace-a\build\bb2.map`, 3053 events in 300.403 s
- scenario: Versus 2P on Kimigaura Coast, versus-2p-kimigaura-rose-slot2-poke script (versus-2p-kimigaura-effects cut at 11300; two rose-light starts, 9885 and 11144; phase mark mid at 9990), screenshots over both starts and the mode-5 effect (10070-10150); eff_StartRecycledCommandWithRoseLight 0x80061280 poked at t_load so every start takes the second slot (command 0xA, mode 4); breakpoint eff_DrawAnimatedQuads, read watches on 0x8009B8C8..0x8009B8E7, 0x8009B978..0x8009B997, 0x8009B9D8..0x8009B9F7; poked: g_eff_growing_quad_and_quad_pair_second_late_tex_rec (D_8009B978).u (0x8009B97C) 0x00 -> 0x80 at fight
- manifest `capture.manifest.json` (SHA1 5194fc3bb28bd59f63b2cef76897068f83a9457e), capture SHA1 9bad3d69ed93f7df4d8ac44628de6c1c9ee0cceb
- canary breakpoint (EXE entry) hit at 0x800836EC in phase boot_intro
- POKED: `0x80061280` (0x80061280) set to 1600001001006424 (was 0b00401000000000) when t_load started (frame 7450): every hit and read after that is from a modified game
- POKED: `0x8009B97C` (0x8009B97C) set to 80 (was 00) when fight started (frame 8230): every hit and read after that is from a modified game
- phases covered: boot_intro (frames 0-5490), t_menu (frames 5490-6050), top_menu (frames 6050-6260), t_char (frames 6260-6550), char_select (frames 6550-6700), weapon_select (frames 6700-7000), t_stage (frames 7000-7175), stage_select (frames 7175-7450), t_load (frames 7450-8230), fight (frames 8230-9990), mid (frames 9990-11300); script reached its end

## eff_DrawAnimatedQuads: 431 hits

Hits per phase: fight 34, mid 397

Call sites (ra-8):
- `eff_DrawAndAdvanceRateSelectedCosineScaledQuad+0x14` x378
- `eff_DrawAndAdvanceSpinningQuadWithDepthSelectedTextureFirstSlot+0x8` x22
- `eff_DrawAndAdvanceCosineScaledQuad+0x8` x13
- `eff_DrawAndAdvancePreseededGrowingQuadSecondSlot+0x8` x12
- `eff_DrawAndAdvanceQuadPairNotMirroredByViewSetting+0x8` x6

| arg | distinct | values (top 4) |
|---|---|---|
| a0 | 5 | 0x00000005, 0x00000001, 0x00000000, 0x00000004 |
| a1 | 1 | 0x8009BB74 `g_eff_rotation_matrix_scale` |
| a2 | 1 | 0x1F800030 |
| a3 | 11 | 0x8010EA80, 0x801130D0, 0x1F800082, 0x8010E918 |

## 0x8009B8C8 (Z3 watch): 2478 hits

Hits per phase: fight 132, mid 2346

Accessing instructions:
- `eff_DrawAnimatedQuads+0x718` x413
- `eff_DrawAnimatedQuads+0x71C` x413
- `eff_DrawAnimatedQuads+0x744` x413
- `eff_DrawAnimatedQuads+0x758` x413
- `eff_DrawAnimatedQuads+0x76C` x413
- `eff_DrawAnimatedQuads+0x790` x413

| access | width | value stored |
|---|---|---|
| `D_8009B8C8` | 2 | - (x391) |
| `D_8009B8C8+0x2` | 2 | - (x391) |
| `D_8009B8C8+0x4` | 2 | - (x782) |
| `D_8009B8C8+0x6` | 2 | - (x782) |
| `D_8009B8D0` | 2 | - (x22) |
| `D_8009B8D0+0x2` | 2 | - (x22) |
| `D_8009B8D0+0x4` | 2 | - (x44) |
| `D_8009B8D0+0x6` | 2 | - (x44) |

## 0x8009B978 (Z3 watch): 144 hits

Hits per phase: fight 108, mid 36

Accessing instructions:
- `eff_DrawAnimatedQuads+0x718` x24
- `eff_DrawAnimatedQuads+0x71C` x24
- `eff_DrawAnimatedQuads+0x744` x24
- `eff_DrawAnimatedQuads+0x758` x24
- `eff_DrawAnimatedQuads+0x76C` x24
- `eff_DrawAnimatedQuads+0x790` x24

| access | width | value stored |
|---|---|---|
| `D_8009B978` | 2 | - (x6) |
| `D_8009B978+0x2` | 2 | - (x6) |
| `D_8009B978+0x4` | 2 | - (x12) |
| `D_8009B978+0x6` | 2 | - (x12) |
| `D_8009B980` | 2 | - (x12) |
| `D_8009B980+0x2` | 2 | - (x12) |
| `D_8009B980+0x4` | 2 | - (x24) |
| `D_8009B980+0x6` | 2 | - (x24) |
| `D_8009B988` | 2 | - (x3) |
| `D_8009B988+0x2` | 2 | - (x3) |
| `D_8009B988+0x4` | 2 | - (x6) |
| `D_8009B988+0x6` | 2 | - (x6) |
| `D_8009B990` | 2 | - (x3) |
| `D_8009B990+0x2` | 2 | - (x3) |
| `D_8009B990+0x4` | 2 | - (x6) |
| `D_8009B990+0x6` | 2 | - (x6) |

## Armed, never hit in this scenario

- `0x8009B9D8 (Z3 watch)`

