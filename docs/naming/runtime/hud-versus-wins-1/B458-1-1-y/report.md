# Runtime trace evidence: capture.jsonl

- recorded 2026-11-02 17:34, map `C:\Users\Trenton\Desktop\bb2-worktrees\ln-trace-a\build\bb2.map`, 6 events in 164.636 s
- scenario: Vs. 2P, versus-2p-bamboo script (KANNUKI vs GENGORO, Bamboo Thicket) cut at 8700: the fight HUD before any fight input; poked: D_8009B458[1][1].y (0x8009B472) 0 -> 60 at t_load
- manifest `capture.manifest.json` (SHA1 b81be94f043f814cbcaee5e369bff497241a5dde), capture SHA1 0b6abd6c1ddcf15f2151fab6f917f7a442486fb8
- canary breakpoint (EXE entry) hit at 0x800836EC in phase boot_intro
- POKED: `0x8009B472` (0x8009B472) set to 3c00 (was 0000) when t_load started (frame 7525): every hit and read after that is from a modified game
- phases covered: boot_intro (frames 0-5490), t_menu (frames 5490-6050), top_menu (frames 6050-6260), t_char (frames 6260-6550), char_select (frames 6550-6700), weapon_select (frames 6700-7000), t_stage (frames 7000-7175), stage_select (frames 7175-7525), t_load (frames 7525-8350), fight (frames 8350-8700); script reached its end

## gpu_DrawTwoDigitCounterWithOptionalSecondSide: 6 hits

Hits per phase: t_load 6
Disarmed after 6 hits (max_hits) in phase t_load: later phases are not covered.

Call sites (ra-8):
- `fight_UpdateModeDisplaysAndMatchProgress+0x11C` x6

| arg | distinct | values (top 4) |
|---|---|---|
| a0 | 1 | 0x00000000 |
| a1 | 1 | 0x00000000 |
| a2 | 6 | 0x801F8088, 0x801E4C70, 0x801F866C, 0x801E5584 |
| a3 | 1 | 0x00000001 |

