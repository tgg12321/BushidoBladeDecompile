# `replay_camera_rob_back_loose2` Codex Lab

This lab is isolated from the live decomp workflow.

## Metadata

- Source file: `C:/Users/Trenton/Desktop/Bushido Blade 2 Decompile/src/code6cac_b2.c`
- Split asm: `C:/Users/Trenton/Desktop/Bushido Blade 2 Decompile/asm/funcs/replay_camera_rob_back_loose2.s`
- Address: `0x80035618`
- Size: `0x210`
- Source state: `matched live C`

## Lane

- Classification: `source-shape with split-rodata/label post-pass`
- Why:
  - the main blocker was block layout and tail sharing, not missing semantics
  - the function also needed an asm-text post-pass so the switch jump table could reference the shared external `jtbl_800108CC`

## Hot Regions

1. `case 3` when `D_800A31DA != 0`
2. switch jump-table export / label remap for the split rodata object

## Workflow

1. Confirm the cleanest semantic baseline.
2. Compare isolated and live pre-regfix asm before deep iteration.
3. Use `asmfix` only for label/export fixes after the source shape is stable.

## Notes

- The first big improvement was fixing the real case mapping from target asm.
- A broad goto refactor made the layout worse; small local control-flow changes worked better.
- The winning source change was keeping the tight baseline and adding the extra `func_800355E8()` call in `case 3`'s nonzero branch before the shared tail call.
- `python tools/check_func.py src/code6cac_b2.c replay_camera_rob_back_loose2` is the final gate and returns `OK`.
