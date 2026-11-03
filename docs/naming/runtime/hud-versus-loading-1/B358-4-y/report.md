# Runtime trace evidence: capture.jsonl

- recorded 2026-11-02 23:26, map `C:\Users\Trenton\Desktop\bb2-worktrees\ln-trace-a\build\bb2.map`, 12 events in 156.28 s
- scenario: Vs. 2P, versus-2p-bamboo script (KANNUKI vs GENGORO, Bamboo Thicket) cut at 8240: the Vs. loading screen before the fight; poked: D_8009B358[4].y (0x8009B37A) 198 -> 138 at t_load
- manifest `capture.manifest.json` (SHA1 7d0799224366896ba00830c971a1fb1d885c8d45), capture SHA1 a93745763be9f50d2807fefea8b6bd1b52611536
- canary breakpoint (EXE entry) hit at 0x800836EC in phase boot_intro
- POKED: `0x8009B37A` (0x8009B37A) set to 8a00 (was c600) when t_load started (frame 7525): every hit and read after that is from a modified game
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

