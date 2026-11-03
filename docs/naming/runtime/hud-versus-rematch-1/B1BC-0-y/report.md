# Runtime trace evidence: capture.jsonl

- recorded 2026-11-02 17:43, map `C:\Users\Trenton\Desktop\bb2-worktrees\ln-trace-a\build\bb2.map`, 12 events in 197.848 s
- scenario: Vs. 2P, versus-2p-bamboo script (KANNUKI vs GENGORO, Bamboo Thicket) cut at 10700: the post-KO Rematch menu; poked: D_8009B1BC[0].y (0x8009B1BE) 4 -> 64 at fight
- manifest `capture.manifest.json` (SHA1 88b54404c31de757cf8ce53bc38b4b46e51ad483), capture SHA1 aeee4c5e8c00648287e7249e134aec1bf03d4ba4
- canary breakpoint (EXE entry) hit at 0x800836EC in phase boot_intro
- POKED: `0x8009B1BE` (0x8009B1BE) set to 4000 (was 0400) when fight started (frame 8350): every hit and read after that is from a modified game
- phases covered: boot_intro (frames 0-5490), t_menu (frames 5490-6050), top_menu (frames 6050-6260), t_char (frames 6260-6550), char_select (frames 6550-6700), weapon_select (frames 6700-7000), t_stage (frames 7000-7175), stage_select (frames 7175-7525), t_load (frames 7525-8350), fight (frames 8350-9900), t_ko (frames 9900-10450), rematch_menu (frames 10450-10700); script reached its end

## gpu_DrawModeSizedPanelWithSelectedCellOpaque: 12 hits

Hits per phase: t_ko 12
Disarmed after 12 hits (max_hits) in phase t_ko: later phases are not covered.

Call sites (ra-8):
- `fight_UpdateFighterFocusViewAndThreeItemMenuWithConfirmDelayedPhaseChange+0x148` x12

| arg | distinct | values (top 4) |
|---|---|---|
| a0 | 1 | 0x00000001 |
| a1 | 1 | 0x00000000 |
| a2 | 11 | 0x801DFF58, 0x801F3384, 0x801F33F8, 0x801DFE24 |
| a3 | 1 | 0x00000000 |

