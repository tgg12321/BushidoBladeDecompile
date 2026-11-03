# Runtime trace evidence: capture.jsonl

- recorded 2026-11-03 08:06, map `C:\Users\Trenton\Desktop\bb2-worktrees\ln-trace-a\build\bb2.map`, 3207 events in 330.371 s
- scenario: Versus 2P, versus-2p-kimigaura-effects script (KANNUKI vs GENGORO, Kimigaura Coast, both pads acting every 60 frames from 8430, no KO by 12730), screenshots every 2-4 frames over the eff_DrawAnimatedQuads effects; breakpoint eff_DrawAnimatedQuads, read watches on 0x8009B8C8..0x8009B8E7, 0x8009B978..0x8009B997, 0x8009B9D8..0x8009B9F7; poked: D_8009B8C8.u (0x8009B8CC) 0x00 -> 0x80 at fight
- manifest `capture.manifest.json` (SHA1 b88cf060ea16ca690d703ad7bb7c2e2749bccab9), capture SHA1 5d3782ad649368aaf3230a5caed0ac067574103b
- canary breakpoint (EXE entry) hit at 0x800836EC in phase boot_intro
- POKED: `0x8009B8CC` (0x8009B8CC) set to 80 (was 00) when fight started (frame 8230): every hit and read after that is from a modified game
- phases covered: boot_intro (frames 0-5490), t_menu (frames 5490-6050), top_menu (frames 6050-6260), t_char (frames 6260-6550), char_select (frames 6550-6700), weapon_select (frames 6700-7000), t_stage (frames 7000-7175), stage_select (frames 7175-7450), t_load (frames 7450-8230), fight (frames 8230-12730); script reached its end

## eff_DrawAnimatedQuads: 453 hits

Hits per phase: fight 453

Call sites (ra-8):
- `eff_DrawAndAdvanceRateSelectedCosineScaledQuad+0x14` x378
- `eff_DrawAndAdvanceSpinningQuadWithDepthSelectedTextureFirstSlot+0x8` x44
- `eff_DrawAndAdvanceCosineScaledQuad+0x8` x13
- `eff_DrawAndAdvancePreseededGrowingQuadFirstSlot+0x8` x12
- `eff_DrawAndAdvanceQuadPairNotMirroredByViewSetting+0x8` x6

| arg | distinct | values (top 4) |
|---|---|---|
| a0 | 5 | 0x00000005, 0x00000001, 0x00000000, 0x00000003 |
| a1 | 1 | 0x8009BB74 `g_eff_rotation_matrix_scale` |
| a2 | 1 | 0x1F800030 |
| a3 | 13 | 0x8010EA80, 0x801130D0, 0x1F800082, 0x80113170 |

## 0x8009B8C8 (Z3 watch): 2610 hits

Hits per phase: fight 2610

Accessing instructions:
- `eff_DrawAnimatedQuads+0x718` x435
- `eff_DrawAnimatedQuads+0x71C` x435
- `eff_DrawAnimatedQuads+0x744` x435
- `eff_DrawAnimatedQuads+0x758` x435
- `eff_DrawAnimatedQuads+0x76C` x435
- `eff_DrawAnimatedQuads+0x790` x435

| access | width | value stored |
|---|---|---|
| `D_8009B8C8` | 2 | - (x391) |
| `D_8009B8C8+0x2` | 2 | - (x391) |
| `D_8009B8C8+0x4` | 2 | - (x782) |
| `D_8009B8C8+0x6` | 2 | - (x782) |
| `g_eff_spinning_quad_far_tex_rec` | 2 | - (x44) |
| `g_eff_spinning_quad_far_tex_rec+0x2` | 2 | - (x44) |
| `g_eff_spinning_quad_far_tex_rec+0x4` | 2 | - (x88) |
| `g_eff_spinning_quad_far_tex_rec+0x6` | 2 | - (x88) |

## 0x8009B978 (Z3 watch): 144 hits

Hits per phase: fight 144

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

