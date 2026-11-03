# Runtime trace evidence: capture.jsonl

- recorded 2026-11-03 04:23, map `C:\Users\Trenton\Desktop\bb2-worktrees\ln-trace-a\build\bb2.map`, 57 events in 167.284 s
- scenario: Story Mode, story-concede script (KANNUKI, default weapon) cut at 8790: the first fight's stage card before the fight; read watch on g_gpu_digit_label_and_line_segment_sheet_headers[3] (0x8009B3BC, 12 bytes); poked: g_gpu_digit_label_and_line_segment_sheet_headers[3].tp0 (0x8009B3BC) 0x1F -> 0x1E at t_stage
- manifest `capture.manifest.json` (SHA1 cc7d2b5736db6b90d82677739238f46cc5c2f5fd), capture SHA1 ec776f00d2bfee96e2865bad25fb6c022ed00d09
- canary breakpoint (EXE entry) hit at 0x800836EC in phase boot_intro
- POKED: `0x8009B3BC` (0x8009B3BC) set to 1e (was 1f) when t_stage started (frame 7900): every hit and read after that is from a modified game
- phases covered: boot_intro (frames 0-5490), t_menu (frames 5490-6060), top_menu (frames 6060-6100), t_story (frames 6100-6810), story_text (frames 6810-6910), t_char (frames 6910-6950), char_select (frames 6950-7300), t_weapon (frames 7300-7360), weapon_select (frames 7360-7900), t_stage (frames 7900-8360), stage_card (frames 8360-8790); script reached its end

## gpu_GetSheetTPage: 40 hits

Hits per phase: t_menu 40
Disarmed after 40 hits (max_hits) in phase t_menu: later phases are not covered.

Call sites (ra-8):
- `gpu_DrawIndexedSheetPair+0x118` x40

| arg | distinct | values (top 4) |
|---|---|---|
| a0 | 1 | 0x80119D4C |
| a1 | 1 | 0x00000000 |
| a2 | 1 | 0x00FFFFFF |
| a3 | 1 | 0xFF000000 |

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

## 0x8009B3BC (Z3 watch): 14 hits

Hits per phase: t_stage 14

Accessing instructions:
- `gpu_AddSpriteCells+0x38` x2
- `gpu_AddSpriteCells+0x3C` x2
- `gpu_AddSpriteCells+0x4C` x2
- `gpu_AddSpriteCells+0xD8` x2
- `gpu_AddSpriteCells+0xEC` x2
- `gpu_GetSheetTPage` x2
- `gpu_GetSheetTPage+0x4` x2

| access | width | value stored |
|---|---|---|
| `g_gpu_digit_label_and_line_segment_sheet_headers+0x24` | 1 | - (x2) |
| `g_gpu_digit_label_and_line_segment_sheet_headers+0x25` | 1 | - (x2) |
| `g_gpu_digit_label_and_line_segment_sheet_headers+0x26` | 1 | - (x2) |
| `g_gpu_digit_label_and_line_segment_sheet_headers+0x28` | 2 | - (x2) |
| `g_gpu_digit_label_and_line_segment_sheet_headers+0x2A` | 2 | - (x2) |
| `g_gpu_digit_label_and_line_segment_sheet_headers+0x2C` | 1 | - (x2) |
| `g_gpu_digit_label_and_line_segment_sheet_headers+0x2E` | 1 | - (x2) |

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

