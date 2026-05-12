# Kengo Rename Queue - 2026-05-12

Generated from reviewed decisions in `kengo_name_decisions.csv`.

Command:

```sh
python3 tools/audit_kengo_renames.py --decided-only
```

## Resolved In Rename Batch 1

These stale Kengo aliases were removed from `named_syms.txt`, and their split
`asm/funcs` labels/files were renamed to the real symbols already present in C:

| Old symbol | New symbol | Address |
| --- | --- | --- |
| `katinuki_game_get_katinuki_max_num_80016868` | `gpu_EnableDisplay` | `0x80016868` |
| `katinuki_game_get_katinuki_max_num_800168D0` | `gpu_DisableDisplay` | `0x800168D0` |
| `katinuki_game_get_katinuki_max_num_80046914` | `snd_StopBgm` | `0x80046914` |
| `katinuki_game_get_katinuki_max_num_80046934` | `snd_AllocSe` | `0x80046934` |
| `katinuki_game_get_katinuki_max_num_80046A60` | `snd_StopSelection` | `0x80046A60` |

## Resolved In Rename Batch 2

This misleading CPU helper name was replaced with a local semantic name after
reviewing its caller and the actor fields it fills:

| Old symbol | New symbol | Address |
| --- | --- | --- |
| `cpu_get_dist_2` | `cpu_decode_move_pattern_params` | `0x800324D0` |

## Resolved In Rename Batch 3

These grouped Kengo `motion_SavePreCalcData` matches were false positives.
They now use local names based on their BB2 callers:

| Old symbol | New symbol | Address |
| --- | --- | --- |
| `motion_SavePreCalcData_8003A3F0` | `pad_ResetSioOnError_8003A3F0` | `0x8003A3F0` |
| `motion_SavePreCalcData_8003A574` | `pad_ReadSioPacket_8003A574` | `0x8003A574` |
| `motion_SavePreCalcData_80080014` | `cdrom_CallbackA_80080014` | `0x80080014` |
| `motion_SavePreCalcData_8008003C` | `cdrom_CallbackB_8008003C` | `0x8008003C` |
| `motion_SavePreCalcData_80080064` | `cdrom_VsyncCallback_80080064` | `0x80080064` |

## Remaining Reviewed Action Queue

No reviewed Kengo rename actions remain from this audit pass.

## Suggested Order

1. Start the next naming pass from a fresh subsystem review rather than the
   resolved Kengo suspicion list.
