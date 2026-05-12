# Naming Triage - 2026-05-12

Worktree: `C:/tmp/bb2-codex-audit-existing-c`

Scope: the 17 score-4 names reported by `tools/audit_kengo_renames.py`.
This began as an evidence pass; the byte-matched rename batches it enabled are
recorded below.

## Audit Caveat

`tools/audit_kengo_renames.py` was written to prefer
`tmp/kengo_matches_pre.csv` when that snapshot exists. This worktree does not
have that file, so the script falls back to the live `kengo_matches.csv`.
For already-renamed asm labels, that can map the current renamed symbol back
to a stale `func_XXXXXXXX` CSV row at the same address instead of the row that
originally justified the rename. The result is useful as a suspicion list, but
not as final proof.

## Decisions

| Symbol | Location | Decision | Evidence | Follow-up |
| --- | --- | --- | --- | --- |
| `cpu_check_same_dir_timer` | `src/code6cac_b.c:2310` | Keep | Live CSV row is `name-unique`; nearby source carries `kengo:HIGH nm_cpu/cpu_check_same_dir_timer`. Body walks a CPU command/direction stream and checks direction bits before dispatching. | Keep name. The audit score is a fallback-CSV artifact. |
| `cpu_get_dist` | `src/code6cac_b.c:1527` | Keep, but semantics are narrower than the name | Live CSV row is `name-unique`. Body rotates/scales actor vector fields using `single_game_getEnemyCharId` and the sine table. | Keep Kengo name for now; do not infer a return value or pure distance calculation from it. |
| `cpu_get_dist_2` | `src/code6cac_b.c:2095` | Resolved | Live CSV row is `size-only-ambiguous` against Kengo `cpu_get_dist`. Body does not compute distance; it decodes a command stream from `a0[0x58]` into actor bytes `0xA1..0xAC`. | Renamed to `cpu_decode_move_pattern_params` after caller review. |
| `cpu_set_move_command_and_dir` | `src/code6cac_b.c:1780` | Keep | Live CSV row is `name-unique`. Body allocates/populates a weapon/body collision command via `coli_hit_body_weapon`, randomizes motion components, and stores the actor direction. | Keep. Audit score is a fallback-CSV artifact. |
| `replay_camera_Init` | `src/code6cac_b2.c:417` | Keep | Live CSV row is `name-unique`; nearby source carries `kengo:HIGH nm_replay_cam/replay_camera_Init`. Body initializes replay/special camera globals from `SpecialCam`. | Keep. Audit score is a fallback-CSV artifact. |
| `special_camera_get_rot_dir` | `src/code6cac_b2.c:612` | Keep provisional | Live CSV row is `name-unique`; nearby source carries `kengo:MED nm_special_cam/special_camera_get_rot_dir`. Body reads `SpecialCam` camera data, copies rotation/control blocks, and advances BCD frame state. | Keep, but signature/layout should be revisited when `SpecialCam` struct is better understood. |
| `katinuki_game_get_katinuki_max_num_80016868` | `asm/funcs`, `named_syms.txt` | Resolved | Assembly is a one-call wrapper to `gpu_SetMode(1)`. `undefined_syms_auto.txt` already identifies `0x80016868` as `gpu_EnableDisplay`, and `src/ings.c` defines `gpu_EnableDisplay` at that address. | Renamed split asm metadata to `gpu_EnableDisplay`; removed stale alias from `named_syms.txt`. |
| `katinuki_game_get_katinuki_max_num_800168D0` | `asm/funcs`, `named_syms.txt` | Resolved | Assembly calls `gpu_SetDispMask(1)`. `undefined_syms_auto.txt` identifies `0x800168D0` as `gpu_DisableDisplay`, and `src/ings.c` defines that function. | Renamed split asm metadata to `gpu_DisableDisplay`; removed stale alias from `named_syms.txt`. |
| `katinuki_game_get_katinuki_max_num_80046914` | `asm/funcs`, `named_syms.txt` | Resolved | Assembly calls `func_800453E0(8)`. The built map and `src/sound.c` identify this address as `snd_StopBgm`. | Renamed split asm metadata to `snd_StopBgm`; removed stale alias from `named_syms.txt`. |
| `katinuki_game_get_katinuki_max_num_80046934` | `asm/funcs`, `named_syms.txt` | Resolved | Assembly calls `func_800455AC(9)`. The built map and `src/sound.c` identify this address as `snd_AllocSe`. | Renamed split asm metadata to `snd_AllocSe`; removed stale alias from `named_syms.txt`. |
| `katinuki_game_get_katinuki_max_num_80046A60` | `asm/funcs`, `named_syms.txt` | Resolved | Assembly calls `func_800453E0(0xA)`. The built map and `src/sound.c` identify this address as `snd_StopSelection`. | Renamed split asm metadata to `snd_StopSelection`; removed stale alias from `named_syms.txt`. |
| `motion_SavePreCalcData_8003A3F0` | `src/code6cac_c_mid.c:1560` | Resolved | Body calls `func_8003A39C()` then sets `D_800A3928 = 1`. The Kengo evidence is sequence-similarity only in a group of five same-name claims. | Renamed to `pad_ResetSioOnError_8003A3F0`; caller reaches it after SIO pad packet failure. |
| `motion_SavePreCalcData_8003A574` | `src/code6cac_c_mid.c:1618` | Resolved | Body calls `func_800789F8(D_800A3734, &D_800A3688, 8)` and is used in the `pad_ClearAppliBuffer` retry flow. | Renamed to `pad_ReadSioPacket_8003A574`; it reads 8 bytes from the `sio:` handle into `D_800A3688`. |
| `motion_SavePreCalcData_80080014` | `src/display.c:2280` | Resolved | Body calls `func_8008008C(0xF0000003, 0x20)` and is installed as CD-ROM callback A. | Renamed to `cdrom_CallbackA_80080014`. |
| `motion_SavePreCalcData_8008003C` | `src/display.c:2284` | Resolved | Body calls `func_8008008C(0xF0000003, 0x40)` and is installed as CD-ROM callback B. | Renamed to `cdrom_CallbackB_8008003C`. |
| `motion_SavePreCalcData_80080064` | `src/display.c:2288` | Resolved | Body calls `func_8008008C(0xF0000003, 0x40)` and is installed through `sys_SetVsyncMode`. | Renamed to `cdrom_VsyncCallback_80080064`. |
| `saTanMainDispGnd_80046020` | `src/text1a_c.c:1919` | Keep provisional | Body calls `func_800453E0(6)` and `func_8005B6AC()`. It is a tiny sequence-similarity match in a group of three, but the address suffix already signals uncertainty. | Keep address-suffixed name for now; do not promote to a bare semantic name. |

## Batchable Follow-ups

Rename batch 1 resolved the five `katinuki_game_get_katinuki_max_num_*`
aliases by replacing them with `gpu_EnableDisplay`, `gpu_DisableDisplay`,
`snd_StopBgm`, `snd_AllocSe`, and `snd_StopSelection` in split asm metadata
and removing the stale `named_syms.txt` aliases.

Rename batch 2 resolved `cpu_get_dist_2` as
`cpu_decode_move_pattern_params`, a local name for the actor move-pattern
stream decoder.

Rename batch 3 resolved the five false `motion_SavePreCalcData_*` matches
as SIO pad helpers and CD-ROM callback wrappers.

1. `kengo_name_decisions.csv` records the reviewed decisions from this file.
   `tools/audit_kengo_renames.py` now loads it by default and suppresses
   `keep` / `byte-sensitive` rows unless `--include-suppressed` is used.
2. Use `python3 tools/audit_kengo_renames.py --decided-only` for the reviewed
   action queue. Use `--csv --decided-only` when a spreadsheet-style rename
   batch list is needed.
3. No reviewed Kengo rename actions remain from this audit pass.
