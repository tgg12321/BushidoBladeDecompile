# Runtime trace evidence: capture.jsonl

- recorded 2026-11-03 06:33, map `C:\Users\Trenton\Desktop\bb2-worktrees\ln-trace-a\build\bb2.map`, 154 events in 200.332 s
- scenario: Group Battle 3 entries, group-battle-orange-slot2-poke script (group-battle-3-0 cut at 10500; P1 RIGHT + TRIANGLE/CIRCLE/SQUARE every 100 frames from 9600, P2 idle; IPPON at 10371): bout 1's IPPON orange effect; breakpoint eff_DrawAnimatedQuads, read watch on the TexRec records D_8009B8C8 .. D_8009B9F7 (0x130 bytes); poked: D_8009B8C8.u (0x8009B8CC) 0x00 -> 0x80 at t_bout1
- manifest `capture.manifest.json` (SHA1 dfc6a915cbe47f184b5e97ddb5d266ae1ca4fa29), capture SHA1 abb6b1809649e2c836613793b1df009665d5f825
- canary breakpoint (EXE entry) hit at 0x800836EC in phase boot_intro
- POKED: `0x8009B8CC` (0x8009B8CC) set to 80 (was 00) when t_bout1 started (frame 9350): every hit and read after that is from a modified game
- phases covered: boot_intro (frames 0-5490), t_menu (frames 5490-6050), top_menu (frames 6050-6320), t_group (frames 6320-6550), group_settings (frames 6550-6620), school_select (frames 6620-6950), t_char (frames 6950-7000), char_select (frames 7000-7650), t_weapon (frames 7650-7700), weapon_select (frames 7700-8550), t_roster (frames 8550-8700), roster (frames 8700-9350), t_bout1 (frames 9350-9400), bout1 (frames 9400-10500); script reached its end

## eff_DrawAnimatedQuads: 22 hits

Hits per phase: bout1 22

Call sites (ra-8):
- `eff_DrawAndAdvanceTwoAnimatedQuadsUntilSecondCounterExpires+0x8` x11
- `eff_DrawAndAdvanceTwoAnimatedQuadsUntilSecondCounterExpires+0x24` x11

| arg | distinct | values (top 4) |
|---|---|---|
| a0 | 2 | 0x0000000C, 0x0000000E |
| a1 | 3 | 0x8009BB74 `g_eff_rotation_matrix_scale`, 0x00112E50, 0x0010E800 |
| a2 | 2 | 0x1F800030, 0x00FFFFFF |
| a3 | 2 | 0x1F8000AC, 0xFF000000 |

## 0x8009B8C8 (Z3 watch): 132 hits

Hits per phase: bout1 132

Accessing instructions:
- `eff_DrawAnimatedQuads+0x718` x22
- `eff_DrawAnimatedQuads+0x71C` x22
- `eff_DrawAnimatedQuads+0x744` x22
- `eff_DrawAnimatedQuads+0x758` x22
- `eff_DrawAnimatedQuads+0x76C` x22
- `eff_DrawAnimatedQuads+0x790` x22

| access | width | value stored |
|---|---|---|
| `D_8009B8C8` | 2 | - (x6) |
| `D_8009B8C8+0x2` | 2 | - (x6) |
| `D_8009B8C8+0x4` | 2 | - (x12) |
| `D_8009B8C8+0x6` | 2 | - (x12) |
| `D_8009B9E8` | 2 | - (x10) |
| `D_8009B9E8+0x2` | 2 | - (x10) |
| `D_8009B9E8+0x4` | 2 | - (x20) |
| `D_8009B9E8+0x6` | 2 | - (x20) |
| `D_8009B9F0` | 2 | - (x6) |
| `D_8009B9F0+0x2` | 2 | - (x6) |
| `D_8009B9F0+0x4` | 2 | - (x12) |
| `D_8009B9F0+0x6` | 2 | - (x12) |

