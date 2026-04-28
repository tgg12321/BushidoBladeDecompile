# func_8003993C — TABLED

## Overview
- **Size:** 526 instructions, 564 lines asm
- **Location:** `src/code6cac_c_mid.c` line 1496
- **Best score:** 8450 (permuter), 8595 (manual)
- **Saved regs:** 10 (ra, fp, s7-s0)
- **Purpose:** Stage/replay frame processing — 3 loops + complex tail dispatch

## Structure
1. **Division by 120** — modulo+divide for both replay and normal mode
2. **Pre-loop setup** — func_8001BAE4, func_8001BBD8, md_game_restart_init
3. **Main loop** (2 iterations) — character entry processing, 8+ function calls per iteration
4. **Loop 2** (32 iterations) — D_80101BF0 table scan, func_80032854
5. **Loop 3** (180 iterations) — D_800F68E0 range-check, func_80049718
6. **Post-loop** — D_800A37D0 increment, D_800A38DC 3-way dispatch with complex branching

## Attempts (6 total)
1. **base.c v1** — score 12890. Initial C translation from manual asm reading
2. **goto fix** — score 10575. Fixed D_80101F5E block ordering with `goto case0_f5e_nz`
3. **register asm("s7")** for s7val — score 9030. Pinned s7val to match target $s7
4. **s32 s7val** — score 8595. Changed from u8 to s32 to avoid andi zero-extension
5. **register asm s4/fp** — score 9190 (WORSE). Pinning player_idx/fp_off backfired
6. **permuter** — score 8450 after 300+ iterations, only marginal improvement

## Intractable Blockers

### 1. LICM of s7val*56
GCC hoists `s7val * 56` (a loop-invariant 3-instruction sequence: sll/subu/sll) out of the main loop. Target recomputes it inside the loop each iteration. Even `register s32 s7val asm("s7")` doesn't prevent the hoist — GCC computes the result once and stores it, keeping the source register but not the computation in-loop. No known C-level technique prevents GCC 2.7.2 LICM for multi-instruction constant multiplications.

### 2. Register Allocation Cascade (s0/s1 swap)
Target: s0=entry, s1=chdata. My code: s0=chdata, s1=entry. This swap cascades through ~40 instructions in the loop body (every access to entry or chdata). Pinning more registers (attempt 5) makes things worse due to cascading scheduling side effects with 10 callee-saved registers.

### 3. Loop Body Scheduling
Different interleaving of loads/stores for sp1A8-sp1BC fields and function arguments. Target interleaves D_800A36EC load with s7val*56 computation and loads a1val before computing chdata. My code computes chdata first. These scheduling differences manifest as insertion/deletion diffs (not register diffs), which are much harder to fix.

### 4. Post-loop Tail Complexity
The D_800A38DC dispatch has 3 cases with nested branching. Case 0 has a D_80101F5E check that splits into two large blocks (each with subroutine calls and further branching). Fixed the block ordering with goto, but remaining diffs include the shared .L8003A0D8 convergence point (GCC cross-jump) and register allocation for `result` (v0 vs a0).

## Penalty Breakdown (at 8595)
- Register Differences: 139 (5 each) = 695
- Reorderings: 15 (60 each) = 900
- Insertions: 22 (100 each) = 2200
- Deletions: 48 (100 each) = 4800
- **Total: 8595**

70 insertion+deletion diffs = 7000/8595 of the score. These are structural, not fixable by register renaming alone.

## Regfix Feasibility
Would need ~100+ rules: ~70 for insert/delete structural diffs + ~30 for register/reorder diffs. Feasible in principle but extremely fragile — one wrong index cascades through the entire function. Comparable to the most complex regfix attempted (marionation_camera_Exec at 74 rules), but with worse structural alignment.

## Files
- `permuter/func_8003993C/base.c` — best manual C (score 8595)
- `permuter/func_8003993C/output-8450-1/source.c` — best permuter variant (score 8450)
- `tmp/write_func_8003993C_v2.py` — script to write base.c
- `tmp/write_func_8003993C.py` — original v1 script
