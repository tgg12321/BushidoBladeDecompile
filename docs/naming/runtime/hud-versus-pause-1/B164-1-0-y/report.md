# Runtime trace evidence: capture.jsonl

- recorded 2026-11-02 19:40, map `C:\Users\Trenton\Desktop\bb2-worktrees\ln-trace-a\build\bb2.map`, 12 events in 181.441 s
- scenario: Vs. CPU Mode, versus-cpu-first-fight script cut at 9700: the Pause Menu after the first fight's KO (lost); poked: D_8009B164[1][0].y (0x8009B176) -6 -> 54 at t_pause
- manifest `capture.manifest.json` (SHA1 8aad086a2e4d65dcb131c3e10296071ac40f96ea), capture SHA1 4e71271e7eb27dc26116ad6be3a37d4126363954
- canary breakpoint (EXE entry) hit at 0x800836EC in phase boot_intro
- POKED: `0x8009B176` (0x8009B176) set to 3600 (was faff) when t_pause started (frame 9300): every hit and read after that is from a modified game
- phases covered: boot_intro (frames 0-5490), t_menu (frames 5490-6050), top_menu (frames 6050-6200), t_char (frames 6200-6500), char_select (frames 6500-6800), t_weapon (frames 6800-6900), weapon_select (frames 6900-7400), t_load (frames 7400-8350), fight (frames 8350-8650), t_ko (frames 8650-8750), after_ko (frames 8750-9300), t_pause (frames 9300-9400), pause_menu (frames 9400-9700); script reached its end

## gpu_DrawModeSizedPanelWithSelectedCellOpaque: 12 hits

Hits per phase: t_pause 12
Disarmed after 12 hits (max_hits) in phase t_pause: later phases are not covered.

Call sites (ra-8):
- `menu_RunPauseMenu+0x118` x12

| arg | distinct | values (top 4) |
|---|---|---|
| a0 | 1 | 0x00000000 |
| a1 | 1 | 0x00000000 |
| a2 | 2 | 0x801D8800, 0x801E2200 |
| a3 | 1 | 0x00000000 |

