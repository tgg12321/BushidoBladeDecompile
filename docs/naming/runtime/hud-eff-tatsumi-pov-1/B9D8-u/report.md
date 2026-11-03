# Runtime trace evidence: capture.jsonl

- recorded 2026-11-03 11:10, map `C:\Users\Trenton\Desktop\bb2-worktrees\ln-trace-a\build\bb2.map`, 792 events in 261.657 s
- scenario: Vs. CPU, versus-cpu-tatsumi script with the Vs. CPU row's view on POV (RIGHT at 6140; P1 TATSUMI: 0 KILLS against a first opponent to 11600, a title splash at 12000, 1 KILL against a second opponent from 12400; opp1 / opp2 / opp3 are phase labels, not opponents), screenshots every 2-4 frames over the eff_DrawAnimatedQuads effects; breakpoint eff_DrawAnimatedQuads, read watches on 0x8009B8C8..0x8009B8E7, 0x8009B978..0x8009B997, 0x8009B9D8..0x8009B9F7; poked: D_8009B9D8.u (0x8009B9DC) 0x00 -> 0x80 at opp1
- manifest `capture.manifest.json` (SHA1 8323e66e5bf864db242ab8a6f2b9fbbc07a92b15), capture SHA1 513570c4589fb35ba6b598067ede379dea11a2a5
- canary breakpoint (EXE entry) hit at 0x800836EC in phase boot_intro
- POKED: `0x8009B9DC` (0x8009B9DC) set to 80 (was 00) when opp1 started (frame 8350): every hit and read after that is from a modified game
- phases covered: boot_intro (frames 0-5490), t_menu (frames 5490-6050), top_menu (frames 6050-6200), t_char (frames 6200-6500), char_select (frames 6500-6800), t_weapon (frames 6800-6900), weapon_select (frames 6900-7400), t_load (frames 7400-8350), opp1 (frames 8350-8900), opp2 (frames 8900-10560), opp3 (frames 10560-13000); script reached its end

## eff_DrawAnimatedQuads: 108 hits

Hits per phase: opp1 6, opp2 12, opp3 90

Call sites (ra-8):
- `eff_DrawAndAdvanceSpinningQuadWithDepthSelectedTextureFirstSlot+0x8` x33
- `eff_DrawAndAdvanceCosineScaledQuad+0x8` x23
- `eff_DrawAndAdvanceRateSelectedCosineScaledQuad+0x14` x17
- `eff_DrawAndAdvanceQuadNotMirroredByViewSetting+0x8` x12
- `eff_DrawAndAdvanceSpinningQuadWithDepthSelectedTextureSecondSlot+0x8` x11
- `eff_DrawAndAdvancePreseededGrowingQuadFirstSlot+0x8` x6
- `eff_DrawAndAdvanceQuadPairWithFirstMirroredByViewSettingOutsideReplay+0x8` x6

| arg | distinct | values (top 4) |
|---|---|---|
| a0 | 7 | 0x00000001, 0x00000000, 0x00000005, 0x0000000B |
| a1 | 1 | 0x8009BB74 `g_eff_rotation_matrix_scale` |
| a2 | 1 | 0x1F800030 |
| a3 | 2 | 0x1F8000AC, 0xFF000000 |

## 0x8009B9D8 (Z3 watch): 108 hits

Hits per phase: opp1 36, opp2 36, opp3 36

Accessing instructions:
- `eff_DrawAnimatedQuads+0x718` x18
- `eff_DrawAnimatedQuads+0x71C` x18
- `eff_DrawAnimatedQuads+0x744` x18
- `eff_DrawAnimatedQuads+0x758` x18
- `eff_DrawAnimatedQuads+0x76C` x18
- `eff_DrawAnimatedQuads+0x790` x18

| access | width | value stored |
|---|---|---|
| `D_8009B9D8` | 2 | - (x15) |
| `D_8009B9D8+0x2` | 2 | - (x15) |
| `D_8009B9D8+0x4` | 2 | - (x30) |
| `D_8009B9D8+0x6` | 2 | - (x30) |
| `D_8009B9E0` | 2 | - (x3) |
| `D_8009B9E0+0x2` | 2 | - (x3) |
| `D_8009B9E0+0x4` | 2 | - (x6) |
| `D_8009B9E0+0x6` | 2 | - (x6) |

## 0x8009B978 (Z3 watch): 72 hits

Hits per phase: opp2 36, opp3 36

Accessing instructions:
- `eff_DrawAnimatedQuads+0x718` x12
- `eff_DrawAnimatedQuads+0x71C` x12
- `eff_DrawAnimatedQuads+0x744` x12
- `eff_DrawAnimatedQuads+0x758` x12
- `eff_DrawAnimatedQuads+0x76C` x12
- `eff_DrawAnimatedQuads+0x790` x12

| access | width | value stored |
|---|---|---|
| `g_eff_growing_quad_and_quad_pair_second_late_tex_rec` | 2 | - (x4) |
| `g_eff_growing_quad_and_quad_pair_second_late_tex_rec+0x2` | 2 | - (x4) |
| `g_eff_growing_quad_and_quad_pair_second_late_tex_rec+0x4` | 2 | - (x8) |
| `g_eff_growing_quad_and_quad_pair_second_late_tex_rec+0x6` | 2 | - (x8) |
| `g_eff_growing_quad_and_quad_pair_second_early_tex_rec` | 2 | - (x8) |
| `g_eff_growing_quad_and_quad_pair_second_early_tex_rec+0x2` | 2 | - (x8) |
| `g_eff_growing_quad_and_quad_pair_second_early_tex_rec+0x4` | 2 | - (x16) |
| `g_eff_growing_quad_and_quad_pair_second_early_tex_rec+0x6` | 2 | - (x16) |

## 0x8009B8C8 (Z3 watch): 504 hits

Hits per phase: opp3 504

Accessing instructions:
- `eff_DrawAnimatedQuads+0x718` x84
- `eff_DrawAnimatedQuads+0x71C` x84
- `eff_DrawAnimatedQuads+0x744` x84
- `eff_DrawAnimatedQuads+0x758` x84
- `eff_DrawAnimatedQuads+0x76C` x84
- `eff_DrawAnimatedQuads+0x790` x84

| access | width | value stored |
|---|---|---|
| `D_8009B8C8` | 2 | - (x40) |
| `D_8009B8C8+0x2` | 2 | - (x40) |
| `D_8009B8C8+0x4` | 2 | - (x80) |
| `D_8009B8C8+0x6` | 2 | - (x80) |
| `g_eff_spinning_quad_near_tex_rec` | 2 | - (x44) |
| `g_eff_spinning_quad_near_tex_rec+0x2` | 2 | - (x44) |
| `g_eff_spinning_quad_near_tex_rec+0x4` | 2 | - (x88) |
| `g_eff_spinning_quad_near_tex_rec+0x6` | 2 | - (x88) |

