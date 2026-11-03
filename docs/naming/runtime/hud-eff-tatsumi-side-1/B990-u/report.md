# Runtime trace evidence: capture.jsonl

- recorded 2026-11-03 11:22, map `C:\Users\Trenton\Desktop\bb2-worktrees\ln-trace-a\build\bb2.map`, 1880 events in 294.526 s
- scenario: Vs. CPU, versus-cpu-tatsumi script with RIGHT at 6070 on the top menu's Story row (Vs. CPU row left on Side View; P1 TATSUMI in side view: the fight shots 8350-12400 show 0 KILLS against one opponent, the run ends on the Rematch menu at 12999; opp1 / opp2 / opp3 are phase labels, not opponents), screenshots every 2 frames over the eff_DrawAnimatedQuads effects; breakpoint eff_DrawAnimatedQuads, read watches on 0x8009B8C8..0x8009B8E7, 0x8009B978..0x8009B997, 0x8009B9D8..0x8009B9F7; poked: D_8009B990.u (0x8009B994) 0x40 -> 0x80 at opp1
- manifest `capture.manifest.json` (SHA1 e0e8e962d5995e4d3bd10843daf13ad5dfc9c18e), capture SHA1 14b0630c6a06057e3714fee9b9e83220db7985da
- canary breakpoint (EXE entry) hit at 0x800836EC in phase boot_intro
- POKED: `0x8009B994` (0x8009B994) set to 80 (was 40) when opp1 started (frame 8350): every hit and read after that is from a modified game
- phases covered: boot_intro (frames 0-5490), t_menu (frames 5490-6050), top_menu (frames 6050-6200), t_char (frames 6200-6500), char_select (frames 6500-6800), t_weapon (frames 6800-6900), weapon_select (frames 6900-7400), t_load (frames 7400-8350), opp1 (frames 8350-8900), opp2 (frames 8900-10560), opp3 (frames 10560-13000); script reached its end

## eff_DrawAnimatedQuads: 248 hits

Hits per phase: opp1 6, opp2 29, opp3 213

Call sites (ra-8):
- `eff_DrawAndAdvanceRateSelectedCosineScaledQuad+0x14` x144
- `eff_DrawAndAdvanceCosineScaledQuad+0x8` x23
- `eff_DrawAndAdvanceSpinningQuadWithDepthSelectedTextureFirstSlot+0x8` x22
- `eff_DrawAndAdvancePreseededGrowingQuadFirstSlot+0x8` x18
- `eff_DrawAndAdvanceQuadPairNotMirroredByViewSetting+0x8` x18
- `eff_DrawAndAdvanceSpinningQuadWithDepthSelectedTextureSecondSlot+0x8` x11
- `eff_DrawAndAdvanceQuadPairWithFirstMirroredByViewSettingOutsideReplay+0x8` x6
- `eff_DrawAndAdvanceQuadMirroredByViewSettingOutsideReplay+0x8` x6

| arg | distinct | values (top 4) |
|---|---|---|
| a0 | 8 | 0x00000005, 0x00000000, 0x00000001, 0x00000003 |
| a1 | 1 | 0x8009BB74 `g_eff_rotation_matrix_scale` |
| a2 | 1 | 0x1F800030 |
| a3 | 2 | 0x1F8000AC, 0xFF000000 |

## 0x8009B978 (Z3 watch): 432 hits

Hits per phase: opp1 36, opp2 180, opp3 216

Accessing instructions:
- `eff_DrawAnimatedQuads+0x718` x72
- `eff_DrawAnimatedQuads+0x71C` x72
- `eff_DrawAnimatedQuads+0x744` x72
- `eff_DrawAnimatedQuads+0x758` x72
- `eff_DrawAnimatedQuads+0x76C` x72
- `eff_DrawAnimatedQuads+0x790` x72

| access | width | value stored |
|---|---|---|
| `g_eff_growing_quad_and_quad_pair_second_late_tex_rec` | 2 | - (x14) |
| `g_eff_growing_quad_and_quad_pair_second_late_tex_rec+0x2` | 2 | - (x14) |
| `g_eff_growing_quad_and_quad_pair_second_late_tex_rec+0x4` | 2 | - (x28) |
| `g_eff_growing_quad_and_quad_pair_second_late_tex_rec+0x6` | 2 | - (x28) |
| `g_eff_growing_quad_and_quad_pair_second_early_tex_rec` | 2 | - (x28) |
| `g_eff_growing_quad_and_quad_pair_second_early_tex_rec+0x2` | 2 | - (x28) |
| `g_eff_growing_quad_and_quad_pair_second_early_tex_rec+0x4` | 2 | - (x56) |
| `g_eff_growing_quad_and_quad_pair_second_early_tex_rec+0x6` | 2 | - (x56) |
| `D_8009B988` | 2 | - (x12) |
| `D_8009B988+0x2` | 2 | - (x12) |
| `D_8009B988+0x4` | 2 | - (x24) |
| `D_8009B988+0x6` | 2 | - (x24) |
| `D_8009B990` | 2 | - (x18) |
| `D_8009B990+0x2` | 2 | - (x18) |
| `D_8009B990+0x4` | 2 | - (x36) |
| `D_8009B990+0x6` | 2 | - (x36) |

## 0x8009B8C8 (Z3 watch): 1200 hits

Hits per phase: opp2 66, opp3 1134

Accessing instructions:
- `eff_DrawAnimatedQuads+0x718` x200
- `eff_DrawAnimatedQuads+0x71C` x200
- `eff_DrawAnimatedQuads+0x744` x200
- `eff_DrawAnimatedQuads+0x758` x200
- `eff_DrawAnimatedQuads+0x76C` x200
- `eff_DrawAnimatedQuads+0x790` x200

| access | width | value stored |
|---|---|---|
| `D_8009B8C8` | 2 | - (x167) |
| `D_8009B8C8+0x2` | 2 | - (x167) |
| `D_8009B8C8+0x4` | 2 | - (x334) |
| `D_8009B8C8+0x6` | 2 | - (x334) |
| `g_eff_spinning_quad_far_tex_rec` | 2 | - (x33) |
| `g_eff_spinning_quad_far_tex_rec+0x2` | 2 | - (x33) |
| `g_eff_spinning_quad_far_tex_rec+0x4` | 2 | - (x66) |
| `g_eff_spinning_quad_far_tex_rec+0x6` | 2 | - (x66) |

## Armed, never hit in this scenario

- `0x8009B9D8 (Z3 watch)`

