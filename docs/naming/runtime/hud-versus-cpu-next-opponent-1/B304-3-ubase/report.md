# Runtime trace evidence: capture.jsonl

- recorded 2026-11-05 23:50, map `C:\Users\Trenton\Desktop\bb2-worktrees\ln-trace-a\build\bb2.map`, 700 events in 192.336 s
- scenario: Vs. CPU, versus-cpu-tatsumi script (TATSUMI) cut at 9200: the screen after P1 beats the first opponent (draw_mode 2 from 8862); breakpoint gpu_DrawSheetPairSelectedByDrawMode (max_hits 300), read watch on 0x8009B304..0x8009B33F; poked: g_gpu_draw_mode_sheet_header_rows[1][3].ubase (0x8009b330) 0x90 -> 0xa0 at opp1 (8350)
- manifest `capture.manifest.json` (SHA1 381288cf37c8f9da43223f26a701faab1b8e5e95), capture SHA1 7363c4111403c97e5aa60c451addb047acc54b90
- canary breakpoint (EXE entry) hit at 0x800836EC in phase boot_intro
- POKED: `0x8009B330` (0x8009B330) set to a000 (was 9000) when opp1 started (frame 8350): every hit and read after that is from a modified game
- phases covered: boot_intro (frames 0-5490), t_menu (frames 5490-6050), top_menu (frames 6050-6200), t_char (frames 6200-6500), char_select (frames 6500-6800), t_weapon (frames 6800-6900), weapon_select (frames 6900-7400), t_load (frames 7400-8350), opp1 (frames 8350-8900), opp2 (frames 8900-9200); script reached its end

## gpu_DrawSheetPairSelectedByDrawMode: 300 hits

Hits per phase: t_load 264, opp1 13, opp2 23
Disarmed after 300 hits (max_hits) in phase opp2: later phases are not covered.

Call sites (ra-8):
- `gpu_DrawDisplayModeOnLocalOtAndSync+0xAC` x300

| arg | distinct | values (top 4) |
|---|---|---|
| a0 | 1 | 0x800F33D8 `g_shared_buffer` |
| a1 | 2 | 0x00000001, 0x00000002 |
| a2 | 7 | 0x801FFD58, 0x801FFE08, 0x801FFC90, 0x801FFDF8 |
| a3 | 1 | 0x8009BED4 `g_gpu_ctx+0x60` |

## 0x8009B304 (Z3 watch): 400 hits

Hits per phase: opp1 299, opp2 101
Disarmed after 400 hits (max_hits) in phase opp2: later phases are not covered.

Accessing instructions:
- `gpu_AddScaledSpriteQuads+0x500` x155
- `gpu_AddScaledSpriteQuads+0xBC` x35
- `gpu_AddScaledSpriteQuads+0xC0` x35
- `gpu_AddScaledSpriteQuads+0xD4` x35
- `gpu_AddScaledSpriteQuads+0xD8` x35
- `gpu_AddScaledSpriteQuads+0xEC` x35
- `gpu_AddScaledSpriteQuads+0xF8` x35
- `gpu_AddScaledSpriteQuads+0x154` x35

| access | width | value stored |
|---|---|---|
| `g_gpu_draw_mode_sheet_header_rows+0x3C` | 1 | - (x18) |
| `g_gpu_draw_mode_sheet_header_rows+0x3D` | 1 | - (x18) |
| `g_gpu_draw_mode_sheet_header_rows+0x3E` | 1 | - (x71) |
| `g_gpu_draw_mode_sheet_header_rows+0x40` | 2 | - (x18) |
| `g_gpu_draw_mode_sheet_header_rows+0x42` | 2 | - (x18) |
| `g_gpu_draw_mode_sheet_header_rows+0x44` | 2 | - (x18) |
| `g_gpu_draw_mode_sheet_header_rows+0x46` | 2 | - (x18) |
| `g_gpu_draw_mode_sheet_header_rows+0x48` | 1 | - (x17) |
| `g_gpu_draw_mode_sheet_header_rows+0x49` | 1 | - (x17) |
| `g_gpu_draw_mode_sheet_header_rows+0x4A` | 1 | - (x119) |
| `g_gpu_draw_mode_sheet_header_rows+0x4C` | 2 | - (x17) |
| `g_gpu_draw_mode_sheet_header_rows+0x4E` | 2 | - (x17) |
| `g_gpu_draw_mode_sheet_header_rows+0x50` | 2 | - (x17) |
| `g_gpu_draw_mode_sheet_header_rows+0x52` | 2 | - (x17) |

