# `saSeInit_2` Codex Lab

This lab is isolated from the live decomp workflow.

## Metadata

- Source file: `/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile - codex-saseinit2/src/code6cac_c_mid.c`
- Split asm: `/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile - codex-saseinit2/asm/funcs/saSeInit_2.s`
- Address: `0x800393C8`
- Size: `0x1ec`
- Source state: `matched in worktree`

## Lane

- Classification: `source-shape -> localized asmfix`
- Why:
  - the handwritten C body got close quickly once the slot layout and search path were recovered
  - the remaining gap was a stubborn control-flow/register-layout block spanning the match search and free-slot scan
  - the live promotion required instruction-anchored asmfix, not isolated-label asmfix

## Hot Regions

1. Match-search loop from the first slot walk through the found-entry fast return
2. Free-slot scan and `D_800A3714` update path before the new-entry store

## Workflow

1. Confirm the cleanest semantic baseline.
2. Compare isolated and live pre-regfix asm before deep iteration.
3. Escalate to localized `asmfix` when branch layout and delay slots stay stubborn.

## Notes

- First low-level baseline: `112` differing words, `-16` bytes.
- Pointer-shape and register-pinned rewrite: best semantic baseline `55` differing words.
- A direct lab asmfix keyed on isolated `.L1` matched in the lab but failed to promote to the live TU.
- The fix was to anchor the asmfix on real instructions (`lh $8,0($4)` through `addu $sp,$sp,8`) and give the inserted end label a unique name (`.LsaSeInit2_end`).
- Live verification:
  - `python3 tools/check_func.py codex_lab/saSeInit_2/permuter/base.c saSeInit_2` -> `OK`
  - `make clean && make check` -> `build/bb2.exe: OK` and `OK: bb2 matches!`
