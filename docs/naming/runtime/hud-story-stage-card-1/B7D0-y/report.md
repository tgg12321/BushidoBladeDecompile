# Runtime trace evidence: capture.jsonl

- recorded 2026-11-02 23:54, map `C:\Users\Trenton\Desktop\bb2-worktrees\ln-trace-a\build\bb2.map`, 3 events in 166.163 s
- scenario: Story Mode, story-concede script (KANNUKI, default weapon) cut at 8790: the first fight's stage card before the fight; poked: D_8009B7D0.y (0x8009B7D2) 52 -> 112 at t_stage
- manifest `capture.manifest.json` (SHA1 c3fb4cc7612d009770c54e24d53140131f63edaa), capture SHA1 600a6e5a18444d25ba0a07ef8912d75a9495c5cf
- canary breakpoint (EXE entry) hit at 0x800836EC in phase boot_intro
- POKED: `0x8009B7D2` (0x8009B7D2) set to 7000 (was 3400) when t_stage started (frame 7900): every hit and read after that is from a modified game
- phases covered: boot_intro (frames 0-5490), t_menu (frames 5490-6060), top_menu (frames 6060-6100), t_story (frames 6100-6810), story_text (frames 6810-6910), t_char (frames 6910-6950), char_select (frames 6950-7300), t_weapon (frames 7300-7360), weapon_select (frames 7360-7900), t_stage (frames 7900-8360), stage_card (frames 8360-8790); script reached its end

## gpu_DrawFixedSpriteCellsStretchedQuadAndRedTintedCells: 2 hits

Hits per phase: t_stage 2

Call sites (ra-8):
- `fight_UpdateCommandStreamImageLoadAndResume+0x30` x2

| arg | distinct | values (top 4) |
|---|---|---|
| a0 | 2 | 0x801D8800, 0x801EBC00 |
| a1 | 1 | 0x00000001 |
| a2 | 1 | 0x00000018 |
| a3 | 1 | 0x00000001 |

## gpu_DrawSpriteAtPackedTablePosition: 1 hits

Hits per phase: stage_card 1

Call sites (ra-8):
- `gpu_DrawDisplayModeOnLocalOtAndSync+0x1E8` x1

| arg | distinct | values (top 4) |
|---|---|---|
| a0 | 1 | 0x00008008 |
| a1 | 1 | 0x800F33D8 `g_shared_buffer` |
| a2 | 1 | 0x00000000 |
| a3 | 1 | 0x8009BED4 `g_gpu_ctx+0x60` |

