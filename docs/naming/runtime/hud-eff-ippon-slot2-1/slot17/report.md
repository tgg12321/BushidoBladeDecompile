# Runtime trace evidence: capture.jsonl

- recorded 2026-11-05 22:10, map `C:\Users\Trenton\Desktop\bb2-worktrees\ln-trace-a\build\bb2.map`, 154 events in 200.869 s
- scenario: Group Battle 3 entries, group-battle-orange-slot2-poke script (group-battle-3-0 cut at 10500; P1 RIGHT + TRIANGLE/CIRCLE/SQUARE every 100 frames from 9600, P2 idle; IPPON at 10371): bout 1's IPPON orange effect with group-battle-orange-slot2-poke-1's slot17 poke (0x800617F8 -> b 0x80061854 / addiu a0, v1, 1 at t_bout1: every start takes the second slot, command 0x11); breakpoint eff_DrawAnimatedQuads, read watches on the TexRec records 0x8009B8C8..0x8009B8E7, 0x8009B978..0x8009B997, 0x8009B9D8..0x8009B9F7
- manifest `capture.manifest.json` (SHA1 cc9e8bb1f0a46d067914edaad6e479d3be66b77f), capture SHA1 d049187f1e7af9a03c4152e29de3d274871d7bb2
- canary breakpoint (EXE entry) hit at 0x800836EC in phase boot_intro
- POKED: `0x800617F8` (0x800617F8) set to 1600001001006424 (was 0b00401000000000) when t_bout1 started (frame 9350): every hit and read after that is from a modified game
- phases covered: boot_intro (frames 0-5490), t_menu (frames 5490-6050), top_menu (frames 6050-6320), t_group (frames 6320-6550), group_settings (frames 6550-6620), school_select (frames 6620-6950), t_char (frames 6950-7000), char_select (frames 7000-7650), t_weapon (frames 7650-7700), weapon_select (frames 7700-8550), t_roster (frames 8550-8700), roster (frames 8700-9350), t_bout1 (frames 9350-9400), bout1 (frames 9400-10500); script reached its end

## eff_DrawAnimatedQuads: 22 hits

Hits per phase: bout1 22

Call sites (ra-8):
- `eff_DrawAndAdvanceTwoAnimatedQuadsUntilFirstCounterExpires+0xC` x11
- `eff_DrawAndAdvanceTwoAnimatedQuadsUntilFirstCounterExpires+0x28` x11

| arg | distinct | values (top 4) |
|---|---|---|
| a0 | 2 | 0x0000000D, 0x0000000F |
| a1 | 3 | 0x8009BB74 `g_eff_rotation_matrix_scale`, 0x00112E50, 0x0010E800 |
| a2 | 2 | 0x1F800030, 0x00FFFFFF |
| a3 | 2 | 0x1F8000AC, 0xFF000000 |

## 0x8009B9D8 (Z3 watch): 96 hits

Hits per phase: bout1 96

Accessing instructions:
- `eff_DrawAnimatedQuads+0x718` x16
- `eff_DrawAnimatedQuads+0x71C` x16
- `eff_DrawAnimatedQuads+0x744` x16
- `eff_DrawAnimatedQuads+0x758` x16
- `eff_DrawAnimatedQuads+0x76C` x16
- `eff_DrawAnimatedQuads+0x790` x16

| access | width | value stored |
|---|---|---|
| `g_eff_two_animated_quads_early_tex_rec` | 2 | - (x10) |
| `g_eff_two_animated_quads_early_tex_rec+0x2` | 2 | - (x10) |
| `g_eff_two_animated_quads_early_tex_rec+0x4` | 2 | - (x20) |
| `g_eff_two_animated_quads_early_tex_rec+0x6` | 2 | - (x20) |
| `g_eff_two_animated_quads_middle_tex_rec` | 2 | - (x6) |
| `g_eff_two_animated_quads_middle_tex_rec+0x2` | 2 | - (x6) |
| `g_eff_two_animated_quads_middle_tex_rec+0x4` | 2 | - (x12) |
| `g_eff_two_animated_quads_middle_tex_rec+0x6` | 2 | - (x12) |

## 0x8009B8C8 (Z3 watch): 36 hits

Hits per phase: bout1 36

Accessing instructions:
- `eff_DrawAnimatedQuads+0x718` x6
- `eff_DrawAnimatedQuads+0x71C` x6
- `eff_DrawAnimatedQuads+0x744` x6
- `eff_DrawAnimatedQuads+0x758` x6
- `eff_DrawAnimatedQuads+0x76C` x6
- `eff_DrawAnimatedQuads+0x790` x6

| access | width | value stored |
|---|---|---|
| `D_8009B8C8` | 2 | - (x6) |
| `D_8009B8C8+0x2` | 2 | - (x6) |
| `D_8009B8C8+0x4` | 2 | - (x12) |
| `D_8009B8C8+0x6` | 2 | - (x12) |

## Armed, never hit in this scenario

- `0x8009B978 (Z3 watch)`

