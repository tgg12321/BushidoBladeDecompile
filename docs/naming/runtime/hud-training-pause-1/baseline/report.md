# Runtime trace evidence: capture.jsonl

- recorded 2026-11-02 18:08, map `C:\Users\Trenton\Desktop\bb2-worktrees\ln-trace-a\build\bb2.map`, 12 events in 176.501 s
- scenario: Training Mode, training-ninja script (vs the Ninja) cut at 9400: the Pause Menu with the Ninja settings; baseline: no instrument poke
- manifest `capture.manifest.json` (SHA1 38722dd4274968bb734148f4c1deda4b1677e7e5), capture SHA1 3f503571fc7f029a8342951dfcec84ccc3a804ba
- canary breakpoint (EXE entry) hit at 0x800836EC in phase boot_intro
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

