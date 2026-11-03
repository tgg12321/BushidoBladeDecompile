# Runtime trace evidence: capture.jsonl

- recorded 2026-11-02 23:27, map `C:\Users\Trenton\Desktop\bb2-worktrees\ln-trace-a\build\bb2.map`, 12 events in 156.171 s
- scenario: Vs. 2P, versus-2p-bamboo script (KANNUKI vs GENGORO, Bamboo Thicket) cut at 8240: the Vs. loading screen before the fight
- manifest `capture.manifest.json` (SHA1 aa300ba1c38e5a1dcb41c17382a44f2c6fc0a062), capture SHA1 1abd77ca5154167d5e8342e5fc29e8843449f64b
- canary breakpoint (EXE entry) hit at 0x800836EC in phase boot_intro
- phases covered: boot_intro (frames 0-5490), t_menu (frames 5490-6050), top_menu (frames 6050-6260), t_char (frames 6260-6550), char_select (frames 6550-6700), weapon_select (frames 6700-7000), t_stage (frames 7000-7175), stage_select (frames 7175-7525), t_load (frames 7525-8240); script reached its end

## gpu_DrawSheetPairSelectedByDrawMode: 6 hits

Hits per phase: t_load 6
Disarmed after 6 hits (max_hits) in phase t_load: later phases are not covered.

Call sites (ra-8):
- `gpu_DrawDisplayModeOnLocalOtAndSync+0xAC` x6

| arg | distinct | values (top 4) |
|---|---|---|
| a0 | 1 | 0x800F33D8 `g_shared_buffer` |
| a1 | 1 | 0x00000001 |
| a2 | 2 | 0x801FFE08, 0x801FFEB0 |
| a3 | 1 | 0x8009BED4 `g_gpu_ctx+0x60` |

## gpu_DrawJitteredQuadPairsAndAdvanceRepeatCycle: 6 hits

Hits per phase: t_load 6
Disarmed after 6 hits (max_hits) in phase t_load: later phases are not covered.

Call sites (ra-8):
- `gpu_DrawDisplayModeOnLocalOtAndSync+0x11C` x6

| arg | distinct | values (top 4) |
|---|---|---|
| a0 | 1 | 0x800F3540 `D_800F34D8+0x68` |
| a1 | 1 | 0x00000001 |
| a2 | 1 | 0x00FFFFFF |
| a3 | 1 | 0x00000005 |

