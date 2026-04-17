# `saSeInit` Codex Lab

This lab is isolated from the live decomp workflow.

## Metadata

- Source file: `C:/Users/Trenton/Desktop/Bushido Blade 2 Decompile - codex-tabled/src/code6cac_b.c`
- Split asm: `C:/Users/Trenton/Desktop/Bushido Blade 2 Decompile - codex-tabled/asm/funcs/saSeInit.s`
- Address: `0x8002E838`
- Size: `0x1ec`
- Source state: `stub only (no live C body yet)`

## Lane

- Classification: `source-shape -> localized asmfix`
- Why:
  - the semantic C body was straightforward and got close quickly
  - the remaining gap was a compact middle block with persistent register/live-range drift
  - isolated matching succeeded once that middle chunk was normalized post-compile

## Hot Regions

1. First `func_8007FD5C` call setup through the CLZ/log-table block
2. Promotion from isolated labels (`.L2/.L3/.L4`) to live-TU-safe unique labels

## Workflow

1. Confirm the cleanest semantic baseline.
2. Compare isolated and live pre-regfix asm before deep iteration.
3. Escalate to `regfix` only after the source shape is stable.

## Notes

- First handwritten baseline: `108` differing words, `-20` bytes.
- Better source-shape pass: `35` differing words. This was the real semantic baseline.
- Several extra C nudges regressed badly; this function did not want more source churn.
- Lab match succeeded with a localized `asmfix` replacement of the stubborn middle block.
- Promotion needed two practical fixes:
  - use `*(&D_8008D118 + idx)` instead of array syntax, because live source declares `D_8008D118` as a scalar extern
  - use unique inserted labels (`.LsaSeInit_*`) instead of isolated `.L2/.L3/.L4`
- Live worktree verification:
  - `wsl python3 tools/check_func.py codex_lab/saSeInit/permuter/base.c saSeInit` -> `OK`
  - `wsl make clean && wsl make check` -> `OK: bb2 matches!`
