# Runtime trace evidence: capture.jsonl

- recorded 2026-11-03 02:28, map `C:\Users\Trenton\Desktop\bb2-worktrees\ln-trace-a\build\bb2.map`, 25 events in 167.431 s
- scenario: Story Mode, story-concede script (KANNUKI, default weapon) cut at 8790: the first fight's stage card before the fight; read watch on D_8009B7A0 (0x8009B7A0, 12 bytes); unpoked
- manifest `capture.manifest.json` (SHA1 3e6b64279c565c61e21b6c71013851c3fde6a20b), capture SHA1 51313dfe6a2bd1264086640965dbb0b3256fb423
- canary breakpoint (EXE entry) hit at 0x800836EC in phase boot_intro
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

## 0x8009B7A0 (Z3 watch): 22 hits

Hits per phase: t_stage 22

Accessing instructions:
- `gpu_AddSpriteCells+0xD8` x8
- `gpu_AddSpriteCells+0xEC` x8
- `gpu_AddSpriteCells+0x38` x2
- `gpu_AddSpriteCells+0x3C` x2
- `gpu_AddSpriteCells+0x4C` x2

| access | width | value stored |
|---|---|---|
| `D_8009B7A0+0x2` | 1 | - (x2) |
| `D_8009B7A0+0x4` | 2 | - (x2) |
| `D_8009B7A0+0x6` | 2 | - (x2) |
| `D_8009B7A0+0x8` | 1 | - (x8) |
| `D_8009B7A0+0xA` | 1 | - (x8) |

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

