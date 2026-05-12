# Existing C Audit - 2026-05-12

Worktree: `C:/tmp/bb2-codex-audit-existing-c`
Branch: `codex/audit-existing-c`

## Baseline

- Clean build was verified with `make clean-check` after linking the ignored local toolchain/disc assets into this isolated worktree.
- Final binary verifies with `sha1sum -c bb2.sha1`.
- `tools/census.py` reports 924 C definitions and zero `INCLUDE_ASM` stubs, but that overstates true pure-C completion because asmfix bridge placeholders are normal C functions.
- `tools/audit_bridge_signatures.py` reports 188 bridged functions from `asmfix.txt`.
- `tools/audit_named_syms_orphans.py` reports no orphaned `named_syms.txt` assignments.

## Cleanup Completed

Bridge stub signatures in `src/text1b.c` were aligned with existing callers/prototypes for:

- `func_80054604`
- `single_game_SetStatusUpData`
- `func_8005C8A8`
- `func_8005D814`
- `func_8005FC9C`
- `func_8006CFBC`
- `func_8006D808`
- `func_80073728`

Result:

- Latent bridge-signature findings dropped from 11 issue instances to 0.
- Informational-only bridge-signature findings dropped from 34 to 30.
- Final SHA still verifies.

Second pass:

- Continued bridge/declaration cleanup in `src/text1b.c`, including return/arity fixes for additional asmfix bridge placeholders such as `func_8005C6D0`, `func_8005E098`, `func_8005E54C`, `func_80063BD0`, `motion_SetExMotion`, `func_80067200`, `func_800678A8`, `func_80067D14`, `func_80068D88`, `func_80069E18`, `func_8006BB68`, `func_8006BEC4`, `saTan4GaugeMain`, `func_8006CCC8`, `func_8006D3DC`, `func_8006DD94`, `func_8006EC0C`, `func_8006F528`, `func_80077374`, and `func_80077D94`.
- Normalized matching extern declarations in `src/ings.c`, `src/code6cac.c`, `src/code6cac_b.c`, `src/code6cac_c.c`, `src/code6cac_c_ab.c`, `src/code6cac_c_mid.c`, and `src/display.c`.
- `tools/audit_bridge_signatures.py` now reports 188 bridged functions with 1 informational finding and no latent findings.
- The remaining informational finding is intentional: `src/text1b.c` keeps a local `extern s32 motion_SetExMotion(s32)` before `func_80065680`/`func_800656EC`. Changing it to the canonical `u8` return makes GCC 2.7.2 emit extra masking and breaks byte-matching.
- Final SHA still verifies.

## Naming Suspects

`tools/audit_kengo_renames.py` flags 17 score-4 names as likely false positives or too weak to trust without manual review:

- `cpu_check_same_dir_timer`
- `cpu_get_dist`
- `cpu_get_dist_2`
- `cpu_set_move_command_and_dir`
- `katinuki_game_get_katinuki_max_num_80016868`
- `katinuki_game_get_katinuki_max_num_800168D0`
- `katinuki_game_get_katinuki_max_num_80046914`
- `katinuki_game_get_katinuki_max_num_80046934`
- `katinuki_game_get_katinuki_max_num_80046A60`
- `motion_SavePreCalcData_8003A3F0`
- `motion_SavePreCalcData_8003A574`
- `motion_SavePreCalcData_80080014`
- `motion_SavePreCalcData_8008003C`
- `motion_SavePreCalcData_80080064`
- `replay_camera_Init`
- `saTanMainDispGnd_80046020`
- `special_camera_get_rot_dir`

Recommended handling: do not mass-rename these from the Kengo audit alone. Compare callers, data touched, and nearby known functions first. If evidence stays weak, demote to address-bearing names or keep the semantic name only as a comment.

Follow-up triage is recorded in `NAMING_TRIAGE_2026-05-12.md`. Short version:
five CPU/camera names are strong enough to keep, `cpu_get_dist_2` is misleading,
the five `katinuki_game_get_katinuki_max_num_*` aliases should be demoted or
replaced, the five `motion_SavePreCalcData_*` names need local evidence before
promotion, and `saTanMainDispGnd_80046020` should stay address-suffixed.
The reviewed decisions are machine-readable in `kengo_name_decisions.csv`, and
`tools/audit_kengo_renames.py --decided-only` now prints the actionable queue.
The current reviewed queue is also summarized in `KENGO_RENAME_QUEUE_2026-05-12.md`.

## Subsystem Map

The current finished-C navigation map is recorded in `SUBSYSTEM_MAP_2026-05-12.md`.
It includes object-level text ranges from `build/bb2.map` and a line/address
landmark map for the large mixed `src/text1b.c` translation unit.

## Inline ASM Debt

`tools/audit_inline_asm.py` classifies inline asm as:

- 52 accepted GTE/BIOS-style bodies.
- 29 other currently accepted match scaffolds.

The second group is the best future cleanup queue after asmfix retirement, but each should be handled as a normal one-function match-retirement task with `dc.sh verify <func>`.

## Safe Next Passes

1. Continue bridge-stub signature hygiene for informational findings only when callers agree and the final SHA is checked.
2. Convert naming suspects into explicit audit comments before renaming, especially size-only Kengo matches.
3. Add section comments or lightweight indexes for large files such as `src/text1b.c`; avoid moving functions between translation units until rodata/regfix dependencies are understood.
4. Normalize local extern declarations gradually. Many are intentionally loose for old-GCC matching, so use small patches and SHA verification.
