# `pad_main_control` Codex Lab

This lab is isolated from the live decomp workflow.

## Metadata

- Source file: `C:/Users/Trenton/Desktop/Bushido Blade 2 Decompile - codex-padmain/src/code6cac_b.c`
- Split asm: `C:/Users/Trenton/Desktop/Bushido Blade 2 Decompile - codex-padmain/asm/funcs/pad_main_control.s`
- Address: `0x8002E6B0`
- Size: `0x188`
- Source state: `matched in worktree`

## Lane

- Classification: `post-compile`
- Why:
  - the semantic C body is straightforward and stable
  - multiple source shapes still floor high with broad register/layout drift
  - the practical finish is a full-body `asmfix`, not a small `regfix`

## Hot Regions

1. Whole function: even good semantic baselines diverge across the entire body instead of in one local scheduler window.

## Workflow

1. Confirm the semantic read first: this is a point-in-triangle test on the X/Z plane using the triangle centroid as the inside reference.
2. Try one readable baseline and one target-shaped control-flow variant to rule out an easy source-shape win.
3. If the mismatch stays whole-body, stop and use `asmfix` rather than spending cycles on regalloc superstition.

## Notes

- Readable baseline: `85` differing words.
- `v0` result-register / goto-end retry: `94` differing words in this branch, despite being the old promising direction.
- Matching path: readable semantic C in `src/code6cac_b.c` plus a full-body `asmfix` entry using the exact target instruction stream.
- Validation: full `make check` returns `build/bb2.exe: OK` and `OK: bb2 matches!`
