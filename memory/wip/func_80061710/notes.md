# func_80061710 - WIP (register-rotation v0/v1 plateau; sibling of func_80061658)

## TL;DR (2026-06-22)
Identical plateau to `func_80061658`. HEAD pins `register s32 t asm("$2"); register
s32 mask asm("$3");` (0 regfix rules). Stripping both pins, honest masked floor =
**9** and 8 explored pure-C forms in this lane all land at >=9 (several worse,
11). The 9 diffs are ALL v0/v1 register-name swaps in the post-jal tail: target
uses v0 for the three arg0[] loads and v1 for the mask (lui/ori 0x10FF10);
sandbox uses v1 for the loads and v0 for the mask. Schedule is byte-identical;
this is purely an allocator preference flip the candidate could not influence.
`candidate.c` saved (PASS cheat-review) for the next agent to resume from.

## What target looks like (post-jal tail)
```
lw  v0, 0(s0)                      # t = arg0[0]
lui at, %hi(D_800F1140)
sw  v0, %lo(D_800F1140)(at)
lw  v0, 4(s0)                      # t = arg0[1]
lui v1, hi(0x10FF10)               # mask = ... (interleaved)
lui at, %hi(D_800F1144)
sw  v0, %lo(D_800F1144)(at)
lw  v0, 8(s0)                      # t = arg0[2]
ori v1, v1, lo(0x10FF10)
sw  v1, %gp_rel(D_800A3464)($gp)   # mask store before D_800F1148
lui at, %hi(D_800F1148)
sw  v0, %lo(D_800F1148)(at)
```
Sandbox (no pins) emits the exact same 12 insns with v0<->v1 swapped on every
load and on the mask. Masked Levenshtein = 9.

## Why pure-C levers don't flip it
GCC 2.7.2 local-alloc gives the LONGER-lived pseudo the lower-numbered reg.
The mask pseudo spans from `lui` (between the 2nd lw and the 2nd sw) to its
final store (between the 3rd lw and the 3rd sw). The t pseudo has 3 short
disjoint live ranges. mask is "longer," so mask -> $v0, t -> $v1. Target's
pinned form forces the opposite. We need a pure-C structure that either
shortens mask's effective live range or extends t's so the allocator preference
flips.

## Forms ruled out this session (all >= 9, none beat HEAD)
- decl order swap (`mask` before `t`) -> 9
- block-local `{ s32 t; s32 mask; ... }` scope after the call -> 9
- `mask = 0x10FF10;` placed right after `func_80060A68()` -> 9
- split mask: `mask = 0x100000; ... mask |= 0xFF10;` -> 9
- inline mask literal `D_800A3464 = 0x10FF10` (no `mask` var) -> 9
- inline t loads `D_800F1140 = arg0[0]` (no `t` var) -> floor 11
- `s32 mask = 0x10FF10;` initialiser at top -> 11 (+2 spill insns, needs callee-save)
- `*arg0 / *(arg0+N)` deref form vs `arg0[N]` -> identical asm

(Sibling 658's prior session also ruled out: split-t into t0/t1/t2 -> 11; mask after 3rd
load -> 9; inline loads (no t) -> 11. None of those repeat here.)

## Next hypotheses (genuinely untried)
- **Standalone cc1 `-da` dump on a minimal isolation TU** to see the actual
  `.greg` priority queue and which structural change in C flips mask's vs t's
  spill-cost ranking. Mechanical answer, not guesswork.
- **Directed permuter** seeded from `candidate.c` with a hint biasing the v0/v1
  allocation between the two pseudos.
- **Family-level resolution** covering func_80061658 / 710 / 617C8 / 618B4 /
  611A4 / 6133C - if all 6 plateau on the same mechanism, one lever (or one
  canonical/plateau decision) covers all siblings, see [[register-alloc-deep-dive]].

## Why blocked, not matched
COMPLETED-C requires 0 pins. Removing both pins raises the honest distance to
9 and no pure-C form explored in this lane reproduces target's v0/v1 rotation.
This is a genuine register-allocator preference plateau, not laziness.
