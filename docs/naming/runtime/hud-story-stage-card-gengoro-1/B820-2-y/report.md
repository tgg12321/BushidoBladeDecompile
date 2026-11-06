# Runtime trace evidence: capture.jsonl

- recorded 2026-11-05 22:32, map `C:\Users\Trenton\Desktop\bb2-worktrees\ln-trace-a\build\bb2.map`, 3 events in 166.216 s
- scenario: Story Mode, story-gengoro-stage-0e script (GENGORO, character id 15, default weapon) cut at 8790: the first fight's stage card (stage 0x02) before the fight; poked: D_8009B820[2].y (0x8009B832) 193 -> 133 at t_stage
- manifest `capture.manifest.json` (SHA1 722ee3ca16a45ef9c93442c6ac0346fd8661d0d3), capture SHA1 8b0c256112eded6ec2875b3c8c39c976471132fe
- canary breakpoint (EXE entry) hit at 0x800836EC in phase boot_intro
- POKED: `0x8009B832` (0x8009B832) set to 8500 (was c100) when t_stage started (frame 7900): every hit and read after that is from a modified game
- phases covered: boot_intro (frames 0-5490), t_menu (frames 5490-6060), top_menu (frames 6060-6100), t_story (frames 6100-6810), story_text (frames 6810-6910), t_char (frames 6910-6950), char_select (frames 6950-7300), t_weapon (frames 7300-7360), weapon_select (frames 7360-7900), t_stage (frames 7900-8320), stage_02 (frames 8320-8790); script reached its end

## gpu_DrawFixedSpriteCellsStretchedQuadAndRedTintedCells: 2 hits

Hits per phase: stage_02 2

Call sites (ra-8):
- `fight_UpdateCommandStreamImageLoadAndResume+0x30` x2

| arg | distinct | values (top 4) |
|---|---|---|
| a0 | 2 | 0x801D8800, 0x801EBC00 |
| a1 | 1 | 0x00000001 |
| a2 | 1 | 0x00000018 |
| a3 | 1 | 0x00000001 |

## gpu_DrawSpriteAtPackedTablePosition: 1 hits

Hits per phase: stage_02 1

Call sites (ra-8):
- `gpu_DrawDisplayModeOnLocalOtAndSync+0x1E8` x1

| arg | distinct | values (top 4) |
|---|---|---|
| a0 | 1 | 0x00008002 |
| a1 | 1 | 0x800F33D8 `g_shared_buffer` |
| a2 | 1 | 0x00000000 |
| a3 | 1 | 0x8009BED4 `g_gpu_ctx+0x60` |

