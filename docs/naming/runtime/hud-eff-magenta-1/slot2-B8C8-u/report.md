# Runtime trace evidence: capture.jsonl

- recorded 2026-11-03 20:10, map `C:\Users\Trenton\Desktop\bb2-worktrees\ln-trace-a\build\bb2.map`, 3234 events in 331.416 s
- scenario: Story Mode, story-ending-magenta-slot2-poke script (story-ending-poke: fighter 0x1C poked in as the opponent, P1 KANNUKI CROSS every 20 from 9800, cut at 12800; two magenta-light starts, 12650 and 12745), screenshots over both starts; breakpoint eff_DrawAnimatedQuads, read watches on 0x8009B8C8..0x8009B8E7, 0x8009B978..0x8009B997, 0x8009B9D8..0x8009B9F7; eff_StartRecycledCommandWithMagentaLight 0x80061EF0 poked at intro so every start takes the second slot (command 0x13, mode 17); poked: D_8009B8C8.u (0x8009B8CC) 0x00 -> 0x80 at fight
- manifest `capture.manifest.json` (SHA1 7436569725e23d9d7c50878ced0a8a27768562ad), capture SHA1 9a0d6b1b256a504cb328b2825e1d68cfbf8107f7
- canary breakpoint (EXE entry) hit at 0x800836EC in phase boot_intro
- POKED: `0x8008F222` (0x8008F222) set to 111c0000120301ff131624008300 (was 100710033324121300190a01010f) when top_menu started (frame 6060): every hit and read after that is from a modified game
- POKED: `0x80061EF0` (0x80061EF0) set to 1600001001006424 (was 0b00401000000000) when intro started (frame 8600): every hit and read after that is from a modified game
- POKED: `0x8009B8CC` (0x8009B8CC) set to 80 (was 00) when fight started (frame 9800): every hit and read after that is from a modified game
- phases covered: boot_intro (frames 0-5490), t_menu (frames 5490-6060), top_menu (frames 6060-6100), t_story (frames 6100-6810), story_text (frames 6810-6910), t_char (frames 6910-6950), char_select (frames 6950-7300), t_weapon (frames 7300-7360), weapon_select (frames 7360-7900), t_stage (frames 7900-8100), stage_card (frames 8100-8600), intro (frames 8600-9800), fight (frames 9800-12800); script reached its end

## eff_DrawAnimatedQuads: 462 hits

Hits per phase: fight 462

Call sites (ra-8):
- `eff_DrawAndAdvanceSpinningQuadWithDepthSelectedTextureFirstSlot+0x8` x440
- `eff_DrawAndAdvanceSpinningQuadWithFixedTextureSecondSlot+0x8` x22

| arg | distinct | values (top 4) |
|---|---|---|
| a0 | 2 | 0x00000001, 0x00000011 |
| a1 | 1 | 0x8009BB74 `g_eff_rotation_matrix_scale` |
| a2 | 1 | 0x1F800030 |
| a3 | 1 | 0x1F8000AC |

## 0x8009B8C8 (Z3 watch): 2772 hits

Hits per phase: fight 2772

Accessing instructions:
- `eff_DrawAnimatedQuads+0x718` x462
- `eff_DrawAnimatedQuads+0x71C` x462
- `eff_DrawAnimatedQuads+0x744` x462
- `eff_DrawAnimatedQuads+0x758` x462
- `eff_DrawAnimatedQuads+0x76C` x462
- `eff_DrawAnimatedQuads+0x790` x462

| access | width | value stored |
|---|---|---|
| `D_8009B8C8` | 2 | - (x22) |
| `D_8009B8C8+0x2` | 2 | - (x22) |
| `D_8009B8C8+0x4` | 2 | - (x44) |
| `D_8009B8C8+0x6` | 2 | - (x44) |
| `D_8009B8D0` | 2 | - (x440) |
| `D_8009B8D0+0x2` | 2 | - (x440) |
| `D_8009B8D0+0x4` | 2 | - (x880) |
| `D_8009B8D0+0x6` | 2 | - (x880) |

## Armed, never hit in this scenario

- `0x8009B978 (Z3 watch)`
- `0x8009B9D8 (Z3 watch)`

