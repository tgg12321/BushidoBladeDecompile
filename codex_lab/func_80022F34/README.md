# `func_80022F34` Codex Lab

This lab is isolated from the live decomp workflow.

## Metadata

- Source file: `C:/Users/Trenton/Desktop/Bushido Blade 2 Decompile - codex-1f938/src/code6cac.c`
- Split asm: `C:/Users/Trenton/Desktop/Bushido Blade 2 Decompile - codex-1f938/asm/funcs/func_80022F34.s`
- Address: `0x80022F34`
- Size: `0x118`
- Source state: `matched in worktree`

## Lane

- Classification: `post-compile`
- Why:
  - the loop structure and semantics are straightforward and recover cleanly
  - the remaining mismatch was concentrated in GCC's branch/layout choices around the mode dispatch and table loads
  - a localized finish was possible, but a full-body `asmfix` was the lowest-cost reliable promotion

## Hot Regions

1. Mode dispatch in the middle of the loop: `mode < 3` / `mode == 0` / `mode == 3` routing changes branch lengths and labels.
2. The paired `D_801027BC` table loads want exact address-materialization and ordering around the `func_80055138` call.

## Workflow

1. Recover the semantic loop first: two entries, 0x44C stride, mode-dependent store to `entry + 8`, then two 5x-indexed table lookups into `func_80055138`.
2. Use a register-shaped baseline to confirm the loop header and induction variables.
3. Once the remaining gap is mostly dispatch/table-order churn, stop and finish with `asmfix`.

## Notes

- First readable baseline: `56` differing words.
- Register-shaped rewrite with `s0/s1/s2` induction variables: `43` differing words.
- Matching path: readable C body in `src/code6cac.c` plus full-body `asmfix`.
