# Runtime trace evidence: capture.jsonl

- recorded 2026-11-02 18:08, map `C:\Users\Trenton\Desktop\bb2-worktrees\ln-trace-a\build\bb2.map`, 12 events in 176.556 s
- scenario: Training Mode, training-ninja script (vs the Ninja) cut at 9400: the Pause Menu with the Ninja settings; poked: D_8009B20C[0].y (0x8009B20E) 165 -> 105 at t_load
- manifest `capture.manifest.json` (SHA1 f25ecdeae38a6c5d0c1752d21a185301bd66f62c), capture SHA1 dbfac3715cd030616b926a2ab094e16e55bef1c1
- canary breakpoint (EXE entry) hit at 0x800836EC in phase boot_intro
- POKED: `0x8009B20E` (0x8009B20E) set to 6900 (was a500) when t_load started (frame 8500): every hit and read after that is from a modified game
- phases covered: boot_intro (frames 0-5490), t_menu (frames 5490-6050), top_menu (frames 6050-6200), t_training (frames 6200-6300), training_menu (frames 6300-7000), t_char (frames 7000-7050), char_select (frames 7050-7400), t_weapon (frames 7400-7450), weapon_select (frames 7450-8500), t_load (frames 8500-9300), pause_menu (frames 9300-9400); script reached its end

## gpu_DrawModeSizedPanelWithSelectedCellOpaque: 12 hits

Hits per phase: t_load 6, pause_menu 6
Disarmed after 12 hits (max_hits) in phase pause_menu: later phases are not covered.

Call sites (ra-8):
- `menu_RunPauseMenu+0x118` x12

| arg | distinct | values (top 4) |
|---|---|---|
| a0 | 1 | 0x00000002 |
| a1 | 1 | 0x00000000 |
| a2 | 2 | 0x801F5600, 0x801EBC00 |
| a3 | 1 | 0x00000000 |

