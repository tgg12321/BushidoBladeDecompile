# Runtime trace evidence: capture.jsonl

- recorded 2026-10-26 05:54, map `C:\Users\Trenton\Desktop\bb2-worktrees\ln-trace\build\bb2.map`, 13 events in 196.421 s
- scenario: Group Battle 3 entries, group-battle-hitmap-1 script cut at 10500 (P1 RIGHT + TRIANGLE/CIRCLE/SQUARE every 100 frames from 9600, P2 idle; IPPON at 10371); eff_StartRecycledCommandWithOrangeLight 0x800617F8 poked to b 0x80061854 / addiu a0, v1, 1 at t_bout1 so the start takes the second slot (status 17, command 0x11)
- manifest `capture.manifest.json` (SHA1 ceb9feb992d22f4995dc4f740029ddcc9549e223), capture SHA1 d6cbd5fffee88d4d1e447154cefc9ed50e367aa3
- canary breakpoint (EXE entry) hit at 0x800836EC in phase boot_intro
- POKED: `0x800617F8` (0x800617F8) set to 1600001001006424 (was 0b00401000000000) when t_bout1 started (frame 9350): every hit and read after that is from a modified game
- phases covered: boot_intro (frames 0-5490), t_menu (frames 5490-6050), top_menu (frames 6050-6320), t_group (frames 6320-6550), group_settings (frames 6550-6620), school_select (frames 6620-6950), t_char (frames 6950-7000), char_select (frames 7000-7650), t_weapon (frames 7650-7700), weapon_select (frames 7700-8550), t_roster (frames 8550-8700), roster (frames 8700-9350), t_bout1 (frames 9350-9400), bout1 (frames 9400-10500); script reached its end

## eff_StartRecycledCommandWithOrangeLight: 1 hits

Hits per phase: bout1 1

Call sites (ra-8):
- `func_80032854+0x160` x1

| arg | distinct | values (top 4) |
|---|---|---|
| a0 | 1 | 0x1F8001C8 |
| a1 | 1 | 0x00000003 |
| a2 | 1 | 0x1F8001C8 |
| a3 | 1 | 0x801FFC60 |

## func_800651F0: 1 hits

Hits per phase: bout1 1

Call sites (ra-8):
- `eff_StartCommand+0xBC` x1

| arg | distinct | values (top 4) |
|---|---|---|
| a0 | 1 | 0x00000011 |
| a1 | 1 | 0x00000C80 |
| a2 | 1 | 0x1F8001C8 |
| a3 | 1 | 0x801FFC60 |

## eff_DrawAndAdvanceTwoAnimatedQuadsUntilFirstCounterExpires: 11 hits

Hits per phase: bout1 11

Call sites (ra-8):
- `eff_StepCommand+0xCC` x11

| arg | distinct | values (top 4) |
|---|---|---|
| a0 | 1 | 0x1F800030 |
| a1 | 1 | 0x8009BB74 `D_8009BB74` |
| a2 | 1 | 0x1F800030 |
| a3 | 1 | 0x1F8000AC |

## Armed, never hit in this scenario

- `func_8006517C`
- `eff_DrawAndAdvanceTwoAnimatedQuadsUntilSecondCounterExpires`

