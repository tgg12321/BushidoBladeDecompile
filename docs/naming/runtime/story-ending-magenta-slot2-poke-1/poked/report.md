# Runtime trace evidence: capture.jsonl

- recorded 2026-11-02 07:02, map `C:\Users\Trenton\Desktop\bb2-worktrees\ln-trace-a\build\bb2.map`, 26 events in 236.667 s
- scenario: Story Mode, story-ending-poke script (fighter 0x1C poked in as the opponent, P1 KANNUKI CROSS every 20 from 9800) cut at 12800 (two magenta-light starts, 12650 and 12745); eff_StartRecycledCommandWithMagentaLight 0x80061EF0 poked to b 0x80061F4C / addiu a0, v1, 1 at intro so every start takes the second slot (status 19, command 0x13)
- manifest `capture.manifest.json` (SHA1 026eee767791e711f2ad6c12e3399539beb137f6), capture SHA1 94c6d9bc9958432c48803842865c8504ab7798c3
- canary breakpoint (EXE entry) hit at 0x800836EC in phase boot_intro
- POKED: `0x8008F222` (0x8008F222) set to 111c0000120301ff131624008300 (was 100710033324121300190a01010f) when top_menu started (frame 6060): every hit and read after that is from a modified game
- POKED: `0x80061EF0` (0x80061EF0) set to 1600001001006424 (was 0b00401000000000) when intro started (frame 8600): every hit and read after that is from a modified game
- phases covered: boot_intro (frames 0-5490), t_menu (frames 5490-6060), top_menu (frames 6060-6100), t_story (frames 6100-6810), story_text (frames 6810-6910), t_char (frames 6910-6950), char_select (frames 6950-7300), t_weapon (frames 7300-7360), weapon_select (frames 7360-7900), t_stage (frames 7900-8100), stage_card (frames 8100-8600), intro (frames 8600-9800), fight (frames 9800-12800); script reached its end

## eff_StartRecycledCommandWithMagentaLight: 2 hits

Hits per phase: fight 2

Call sites (ra-8):
- `fight_RecordReplayEventAndDispatchSoundOrEffect+0x330` x2

| arg | distinct | values (top 4) |
|---|---|---|
| a0 | 1 | 0x1F800228 |
| a1 | 1 | 0x00000011 |
| a2 | 1 | 0x1F800228 |
| a3 | 1 | 0x00000000 |

## eff_InitSpinningQuadWithFixedTextureSecondSlot: 2 hits

Hits per phase: fight 2

Call sites (ra-8):
- `eff_StartCommand+0xBC` x2

| arg | distinct | values (top 4) |
|---|---|---|
| a0 | 1 | 0x00000013 |
| a1 | 1 | 0x00000C80 |
| a2 | 1 | 0x1F800228 |
| a3 | 1 | 0x00000000 |

## eff_DrawAndAdvanceSpinningQuadWithFixedTextureSecondSlot: 22 hits

Hits per phase: fight 22

Call sites (ra-8):
- `eff_StepCommand+0xCC` x22

| arg | distinct | values (top 4) |
|---|---|---|
| a0 | 1 | 0x1F800030 |
| a1 | 1 | 0x8009BB74 `g_eff_rotation_matrix_scale` |
| a2 | 1 | 0x1F800030 |
| a3 | 1 | 0x1F8000AC |

## Armed, never hit in this scenario

- `eff_InitSpinningQuadWithFixedTextureFirstSlot`
- `eff_DrawAndAdvanceSpinningQuadWithFixedTextureFirstSlot`

