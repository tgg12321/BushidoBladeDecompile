# Runtime trace evidence: capture.jsonl

- recorded 2026-10-26 05:58, map `C:\Users\Trenton\Desktop\bb2-worktrees\ln-trace\build\bb2.map`, 13 events in 196.651 s
- scenario: Group Battle 3 entries, group-battle-hitmap-1 script cut at 10500 (P1 RIGHT + TRIANGLE/CIRCLE/SQUARE every 100 frames from 9600, P2 idle; IPPON at 10371)
- manifest `capture.manifest.json` (SHA1 b872469c9b25c7fa56b1576bcbac18be65270568), capture SHA1 360492ca38df6316acc297246088d13662b53646
- canary breakpoint (EXE entry) hit at 0x800836EC in phase boot_intro
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

## func_8006517C: 1 hits

Hits per phase: bout1 1

Call sites (ra-8):
- `eff_StartCommand+0xBC` x1

| arg | distinct | values (top 4) |
|---|---|---|
| a0 | 1 | 0x00000010 |
| a1 | 1 | 0x00000C80 |
| a2 | 1 | 0x1F8001C8 |
| a3 | 1 | 0x801FFC60 |

## eff_DrawAndAdvanceTwoAnimatedQuadsUntilSecondCounterExpires: 11 hits

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

- `func_800651F0`
- `eff_DrawAndAdvanceTwoAnimatedQuadsUntilFirstCounterExpires`

