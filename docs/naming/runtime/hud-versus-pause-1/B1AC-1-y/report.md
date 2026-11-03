# Runtime trace evidence: capture.jsonl

- recorded 2026-11-02 19:40, map `C:\Users\Trenton\Desktop\bb2-worktrees\ln-trace-a\build\bb2.map`, 12 events in 181.426 s
- scenario: Vs. CPU Mode, versus-cpu-first-fight script cut at 9700: the Pause Menu after the first fight's KO (lost); poked: D_8009B1AC[1].y (0x8009B1B6) 18 -> 78 at t_pause
- manifest `capture.manifest.json` (SHA1 5dedabf1590b5edb8a2a0b2556df357ff3ccf3a4), capture SHA1 3a7a3f57368cdcc5a0a40fdfc339dc0811f58a50
- canary breakpoint (EXE entry) hit at 0x800836EC in phase boot_intro
- POKED: `0x8009B1B6` (0x8009B1B6) set to 4e00 (was 1200) when t_pause started (frame 9300): every hit and read after that is from a modified game
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

