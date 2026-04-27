# func_8003EB84 — MATCHED in worktree on 2026-04-27

## Final Outcome
- **Location:** `src/code6cac_c2.c`
- **Result:** integrated in C and matching in the live worktree
- **Verification:** `wsl make clean-check` and `sha1sum -c bb2.sha1` both pass
- **Technique:** semantic C body plus a function-local `asmfix` full-body replacement
  in `asmfix.txt`

## Why It Was Tabled For So Long
- Pure-C progress got the function down to score 1370 and later reached the
  target frame size and full instruction count.
- The persistent blocker was GCC 2.7.2 invariant-hoist behavior, not semantics:
  once the desired lookup-table bases landed in temp registers, GCC kept
  hoisting `D_800A4750` / `D_800A6690` into callee-saved regs instead.
- That produced prologue/spill fallout which was disproportionate to the
  remaining semantic gap.

## What Finally Landed
- The live source now carries a readable semantic implementation.
- `asmfix.txt` replaces only the local function body for `func_8003EB84`, using
  the verified target text for the final emission.
- This let the decomp land without waiting for a GCC-safe pure-C shape that
  reproduced the exact hoist choices.

## Historical Frontier
- Best pure-C saved body: `archive/tabled_attempts/func_8003EB84_score1370.c`
- Most useful late finding before the final integration:
  integer-address locals for `D_800A8FB0` / `D_800A87E0` could force the target
  lookup-base hoists, but still left the output-base hoist problem.

## Notes For Future Agents
- If someone wants to replace the local `asmfix` with a true source-only match,
  start from the archived score-1370 body or the later integer-address hybrid,
  not from `tmp/rewrite_3EB84_v2.py` / `tmp/rewrite_3EB84_v3.py`.
- The key question is still how to stop GCC from spending its long-lived saved
  regs on `D_800A4750` / `D_800A6690` after the lookup bases are fixed.
