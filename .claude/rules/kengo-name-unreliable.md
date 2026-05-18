---
name: kengo-name-unreliable
paths: ["named_syms.txt", "docs/naming/**", "kengo_matches.csv"]
description: "Kengo `name-unique` confidence in kengo_matches.csv and apply_kengo_names.py is name+size only, NOT semantic. Verify body shape before trusting; multi-address landings are especially unsafe (27% misnomer rate confirmed across 150-function audit)."
metadata:
  type: rule
---

# Kengo "name-unique" match ≠ same function

`kengo_matches.csv` matches BB2 functions to Kengo PS2 functions by NAME + SIZE (instruction count). A `name-unique` confidence tag means the matcher found exactly one Kengo function with this BB2 name + the same instruction count. **It does NOT mean the function bodies are semantically equivalent.** Size collision on common function shapes (1-line wrappers, syscall trampolines, event emitters) is frequent enough that the bare `name-unique` tag is a weak signal.

## How to apply

1. **Single-address Kengo names in established subsystems are reliable.** Passes 7-8 audited 84 single-address names in `cpu_*`, `motion_*`, `pad_*`, `mario_*`, `replay_camera_*`, `efc_*`, `gnd_*`, `bios_*` — 0 misnomers (84/84 MATCH or ASMFIX). Trust these.
2. **Multi-address families (3+ BB2 addrs share the same base Kengo name) are SUSPECT by default.** Combined passes 5-8 final stats: 150 audited, 41 misnomers (27% overall) — **all 41 in multi-address families.**
3. Before using Kengo as a decomp reference (rather than just a naming hint): run `dc.sh kengo-ref <func>` and visually compare structure (stack frame, jal count, constants, control flow shape) before assuming the PS2 body is a usable C-structure reference.

## Concrete failures

### Single-function rename failure: `exec_game` (2026-05-17)

`kengo_matches.csv` reports a perfect-looking match:
```
exec_game, main.c, 194, exec_game, 194, 0, md_game, src/common/md_game.c, name-unique
```
Size diff: 0. Name: exact. Confidence: name-unique.

But the actual PS2 body is structurally unrelated:
- BB2 `exec_game`: leaf, no calls, 5-phase in-place SPU voice list compaction with `lui+lw` globals
- Kengo `exec_game`: stack frame (-128 sp), saves `$s0-$s6 + $ra`, 7+ jal targets (`motion_GameCalcMotion`, `util_CalcBukiMatrix`, `judge_Exec`, `motion_SeControl`, `coli_MakeKatanaVec`, `action_UkenagashiCheck`, ...), uses `lwc1` (FPU) and `lbu`, GP-relative addressing.

The constants unique to BB2's `exec_game` (`0x2FFFFFFF`, `0x0FFFFFFF`) don't appear ANYWHERE in Kengo's disassembly (verified by grep).

### Multi-address mass-rename failures

| Pass | Family | Outcome |
|------|--------|---------|
| 4 (2026-05-17) | `katinuki_game_get_katinuki_max_num_*` | 5 addrs, 3 wrong |
| 4 | `tslSmdSendVu1Code_*` | 2 addrs, 2 wrong |
| 5 (2026-05-18) | `Vu0SetLightColMatrix_*` | 4/4 wrong (VU0 is PS2-only hardware; none of these BB2 funcs touch GTE color matrix) |
| 5 | `camera_SetMatrix_*` | 3/4 wrong (only `0x8001DA8C` is real) |
| 5 | `motion_SavePreCalcData_*` | 5/5 wrong (3 are duplicated `bios_DeliverEvent` wrappers) |
| 5 | `motion_LoadPreCalcData_*` | 3/3 wrong |
| 5 | `mario_getMarioVoiceData_*` | 4/4 wrong (heterogeneous bodies, only size collision) |

## Why this happens

Between BB2 (PS1, 1998) and Kengo (PS2, 2000), Lightweight:
- Renamed/refactored many functions
- Same function names sometimes got applied to evolved implementations
- PS1 SPU-specific code (e.g. `exec_game`'s domain) was rewritten for PS2 SPU2 with different data layouts
- PS2 has hardware (VU0, FPU) that PS1 lacks — any PS2 function naming VU0/FPU primitives won't have a real BB2 equivalent

## Tooling

### Detect suspect families in existing named_syms.txt

```python
import re, pathlib, collections
names = [m.group(1) for line in pathlib.Path("named_syms.txt").read_text().splitlines()
         for m in [re.match(r"\s*([A-Za-z_]\w*)\s*=", line)] if m]
def base(n): return re.sub(r"_[0-9A-Fa-f]{8}$", "", n)
for b, c in collections.Counter(base(n) for n in names).most_common(40):
    if c >= 3 and not b.startswith(("g_", "D_", "jtbl_")):
        print(f"  {c}x  {b}")
```

### Validation checklist before using Kengo as decomp reference

For any function where you're tempted to use Kengo as a C-structure reference:

1. `dc.sh kengo-ref <func>` — get the actual PS2 asm
2. Compare:
   - Stack frame presence (leaf vs framed)
   - Call count (BB2 leaf vs Kengo with jal)
   - Constants used (do the same `0x...` magic values appear?)
   - Control flow shape (linear vs nested loops vs early-exit)
3. If structure differs → Kengo is naming-only, not semantic reference; decompile from BB2's asm alone

### MISNOMER cleanup procedure

For each suspect family that audit confirms as misnamed:

1. Dispatch a body-audit (read each function's `src/*.c` or `asm/funcs/*.s` body, classify MATCH / PARTIAL / MISNOMER / UNKNOWN).
2. Demote misnomers with `/* MISNAMED: <body summary> */` comments — don't delete (preserves `src/*.c` compatibility).
3. Add replacement aliases in a `=== MISNOMERS pass N replacement aliases ===` section at the end of `named_syms.txt`.
4. Replacement alias names preserve the address suffix (e.g., `cpu_side_move_dir_4_wrapper_800801E8`) so the uncertainty marker carries forward.
5. SHA1-verify after each batch (alias-only adds are safe).

See `docs/naming/MISNOMERS.md` for the documented audit format. Remaining suspect families per `tmp/misnomer_audit_pass5.py` output: `ang_hosei`, `calc_loc_mat_fw`, `InitHiraRmd`, `calc_fc_frame`, `md_option_reset`, `saTanMainDispGnd`, `saTan2InfoInit`.

## Reliable Kengo use cases

The CSV IS good for:
- Function naming HINTS when a single-address landing in an established subsystem
- Identifying functions that genuinely share structure — look for `kengo:HIGH` comments in source (those have been manually verified)
- Callee cascading via Kengo named `jal` targets at structurally matching positions

The CSV is NOT reliable for:
- Multi-address Kengo families (default-suspect; audit before trusting)
- Assuming the PS2 function body is a usable reference for C structure
- BB2-specific systems (SPU voice management, scratchpad coord) that were rewritten for PS2
