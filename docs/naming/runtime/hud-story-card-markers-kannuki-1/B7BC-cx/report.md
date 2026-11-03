# Runtime trace evidence: capture.jsonl

- recorded 2026-11-03 14:46, map `C:\Users\Trenton\Desktop\bb2-worktrees\ln-trace-a\build\bb2.map`, 24 events in 465.309 s
- scenario: Story Mode, story-final-fight-retail script (KANNUKI, default weapon) cut at 26400: the second stage card (Terraced Fields); breakpoint gpu_DrawSpriteAtPackedTablePosition, read watch on 0x8009B7AC..0x8009B7CF; poked: g_gpu_stage_card_map_marker_sheet_headers[1] CLUT x (0x8009B7BC) 0x3B0 -> 0x3A0 (low byte) at card2 (25900)
- manifest `capture.manifest.json` (SHA1 8f0719f27240fff02eb3fd197aee4e839b41ec12), capture SHA1 2cd8da87f2ee30013e12d5c1dad5a113647c21a1
- canary breakpoint (EXE entry) hit at 0x800836EC in phase boot_intro
- POKED: `0x8009B7BC` (0x8009B7BC) set to a0 (was b0) when card2 started (frame 25900): every hit and read after that is from a modified game
- phases covered: boot_intro (frames 0-8900), story (frames 8900-25900), card2 (frames 25900-26400); script reached its end

## gpu_DrawSpriteAtPackedTablePosition: 3 hits

Hits per phase: boot_intro 1, card2 2

Call sites (ra-8):
- `gpu_DrawDisplayModeOnLocalOtAndSync+0x1E8` x3

| arg | distinct | values (top 4) |
|---|---|---|
| a0 | 3 | 0x00008008, 0x00000008, 0x0000800F |
| a1 | 1 | 0x800F33D8 `g_shared_buffer` |
| a2 | 1 | 0x00000000 |
| a3 | 1 | 0x8009BED4 `g_gpu_ctx+0x60` |

## 0x8009B7AC (Z3 watch): 21 hits

Hits per phase: boot_intro 7, card2 14

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
| `D_8009B7AC+0xC` | 1 | - (x1) |
| `D_8009B7AC+0xD` | 1 | - (x1) |
| `D_8009B7AC+0xE` | 1 | - (x1) |
| `D_8009B7AC+0x10` | 2 | - (x1) |
| `D_8009B7AC+0x12` | 2 | - (x1) |
| `D_8009B7AC+0x14` | 1 | - (x1) |
| `D_8009B7AC+0x16` | 1 | - (x1) |

