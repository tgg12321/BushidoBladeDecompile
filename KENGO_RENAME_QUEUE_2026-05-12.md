# Kengo Rename Queue - 2026-05-12

Generated from reviewed decisions in `kengo_name_decisions.csv`.

Command:

```sh
python3 tools/audit_kengo_renames.py --decided-only
```

## Reviewed Action Queue

| Symbol | Address | Decision | Why |
| --- | --- | --- | --- |
| `cpu_get_dist_2` | `0x800324D0` | demote/review | Body decodes command-pattern bytes from the actor command stream; it is not a distance helper. |
| `katinuki_game_get_katinuki_max_num_80016868` | `0x80016868` | demote/replace | One-call wrapper to `gpu_SetMode(1)`; already corresponds to `gpu_EnableDisplay`. |
| `katinuki_game_get_katinuki_max_num_800168D0` | `0x800168D0` | demote/replace | One-call wrapper to `gpu_SetDispMask(1)`; already corresponds to `gpu_DisableDisplay`. |
| `katinuki_game_get_katinuki_max_num_80046914` | `0x80046914` | demote | One-call wrapper to `func_800453E0(8)`; not a katinuki max-count query. |
| `katinuki_game_get_katinuki_max_num_80046934` | `0x80046934` | demote | One-call wrapper to `func_800455AC(9)`; not a katinuki max-count query. |
| `katinuki_game_get_katinuki_max_num_80046A60` | `0x80046A60` | demote | One-call wrapper to `func_800453E0(0xA)`; not a katinuki max-count query. |
| `motion_SavePreCalcData_8003A3F0` | `0x8003A3F0` | demote/review | Calls `func_8003A39C()` and sets `D_800A3928`; grouped sequence match only. |
| `motion_SavePreCalcData_8003A574` | `0x8003A574` | demote/review | Calls `func_800789F8(D_800A3734, &D_800A3688, 8)` and appears in CD/init retry flow. |
| `motion_SavePreCalcData_80080014` | `0x80080014` | demote/review | Callback wrapper for `func_8008008C(0xF0000003, 0x20)`. |
| `motion_SavePreCalcData_8008003C` | `0x8008003C` | demote/review | Callback wrapper for `func_8008008C(0xF0000003, 0x40)`. |
| `motion_SavePreCalcData_80080064` | `0x80080064` | demote/review | VSync-mode callback wrapper for `func_8008008C(0xF0000003, 0x40)`. |

## Suggested Order

1. Start with the two display aliases at `0x80016868` and `0x800168D0`;
   they already have better names in current C and `undefined_syms_auto.txt`.
2. Next demote the three `katinuki_game_get_katinuki_max_num_800469*`
   wrappers to address names unless the `func_800453E0`/`func_800455AC`
   channel subsystem is named first.
3. Leave `cpu_get_dist_2` and the five `motion_SavePreCalcData_*` names for
   caller-review batches. They need better local semantic names, not just
   address-name demotion.
