# Runtime trace evidence: capture.jsonl

- recorded 2026-11-03 14:47, map `C:\Users\Trenton\Desktop\bb2-worktrees\ln-trace-a\build\bb2.map`, 24 events in 474.091 s
- scenario: Story Mode, story-gengoro-stage-0e script (GENGORO, character id 15) cut at 26900: the second stage card (Ferry Dock); breakpoint gpu_DrawSpriteAtPackedTablePosition, read watch on 0x8009B7AC..0x8009B7CF
- manifest `capture.manifest.json` (SHA1 51b94b31f3aff7515dd55426b80df2b69766fe2f), capture SHA1 541680924d87c59af962bef7e9f8b734dc79bad1
- canary breakpoint (EXE entry) hit at 0x800836EC in phase boot_intro
- phases covered: boot_intro (frames 0-5490), t_menu (frames 5490-6060), top_menu (frames 6060-6100), t_story (frames 6100-6810), story_text (frames 6810-6910), t_char (frames 6910-6950), char_select (frames 6950-7300), t_weapon (frames 7300-7360), weapon_select (frames 7360-7900), t_stage (frames 7900-8320), stage_02 (frames 8320-26474), stage_16 (frames 26474-26900); script reached its end

## gpu_DrawSpriteAtPackedTablePosition: 3 hits

Hits per phase: stage_02 1, stage_16 2

Call sites (ra-8):
- `gpu_DrawDisplayModeOnLocalOtAndSync+0x1E8` x3

| arg | distinct | values (top 4) |
|---|---|---|
| a0 | 3 | 0x00008002, 0x00000002, 0x00008016 |
| a1 | 1 | 0x800F33D8 `g_shared_buffer` |
| a2 | 1 | 0x00000000 |
| a3 | 1 | 0x8009BED4 `g_gpu_ctx+0x60` |

## 0x8009B7AC (Z3 watch): 21 hits

Hits per phase: stage_02 7, stage_16 14

Accessing instructions:
- `gpu_AddSpriteCells+0x38` x3
- `gpu_AddSpriteCells+0x3C` x3
- `gpu_AddSpriteCells+0x4C` x3
- `gpu_AddSpriteCells+0xD8` x3
- `gpu_AddSpriteCells+0xEC` x3
- `gpu_GetSheetTPage` x3
- `gpu_GetSheetTPage+0x4` x3

| access | width | value stored |
|---|---|---|
| `D_8009B7AC` | 1 | - (x2) |
| `D_8009B7AC+0x1` | 1 | - (x2) |
| `D_8009B7AC+0x2` | 1 | - (x2) |
| `D_8009B7AC+0x4` | 2 | - (x2) |
| `D_8009B7AC+0x6` | 2 | - (x2) |
| `D_8009B7AC+0x8` | 1 | - (x2) |
| `D_8009B7AC+0xA` | 1 | - (x2) |
| `D_8009B7AC+0x18` | 1 | - (x1) |
| `D_8009B7AC+0x19` | 1 | - (x1) |
| `D_8009B7AC+0x1A` | 1 | - (x1) |
| `D_8009B7AC+0x1C` | 2 | - (x1) |
| `D_8009B7AC+0x1E` | 2 | - (x1) |
| `D_8009B7AC+0x20` | 1 | - (x1) |
| `D_8009B7AC+0x22` | 1 | - (x1) |

