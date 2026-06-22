# func_80061ACC — WIP (register-rotation v0/v1 plateau; sibling of func_80061658/710)

## TL;DR (2026-06-22)
Identical plateau to siblings `func_80061658` and `func_80061710`. HEAD pins
`register s32 t asm("$2"); register s32 mask asm("$3");` (0 regfix rules). Pins
stripped → honest masked floor = **9**. This lane swept 8 pure-C forms specific
to this function's body (which differs from 658/710: D_800F1164[0/1] dispatch
with goto instead of switch(arg1)); all land at ≥9 (two worse: 11 and 33).
The 9 diffs are ALL v0/v1 register-name swaps on the post-jal tail (3 arg0[]
loads + the lui/ori 0xFF8080 mask). Schedule is byte-identical; purely an
allocator preference flip the candidate cannot influence in pure C. `candidate.c`
saved (PASS cheat-review) so the next agent resumes from the clean no-pin form.

## What target looks like (post-jal tail)
```
jal func_80060A68
nop
lw  $v0, 0x0($s1)                    # t = arg0[0]
lui $at, %hi(D_800F1140)
sw  $v0, %lo(D_800F1140)($at)
lw  $v0, 0x4($s1)                    # t = arg0[1]
lui $v1, 0xFF                        # mask high (interleaved)
lui $at, %hi(D_800F1144)
sw  $v0, %lo(D_800F1144)($at)
lw  $v0, 0x8($s1)                    # t = arg0[2]
ori $v1, $v1, 0x8080                 # mask low (0xFF8080)
sw  $v1, %gp_rel(D_800A3464)($gp)
lui $at, %hi(D_800F1148)
sw  $v0, %lo(D_800F1148)($at)
```
Sandbox (no pins) emits the EXACT SAME 12 insns with v0<->v1 swapped on every
load and on the mask. Masked Levenshtein = 9.

## Why pure-C levers don't flip it (per [[register-alloc-deep-dive]])
GCC 2.7.2 local-alloc gives the LONGER-LIVED pseudo the lower-numbered reg.
The `mask` pseudo spans from `lui` (between 2nd lw and 2nd sw) through to
its final `sw` (between 3rd lw and 3rd sw). The `t` pseudo has 3 short
disjoint live ranges. mask wins on length → mask→$v0, t→$v1. Target's
pinned form forces the opposite. Need a pure-C structure that either
shortens mask's live range or extends t's so the allocator preference
flips — and across 80061658, 80061710, and this function (20 forms
total) none has been found.

## 8 forms ruled out this session (this function specifically)
- v1 baseline no-pins (s32 t; s32 mask;) → 9
- v2 block-local `{ s32 t; s32 mask; ... }` after the call → 9
- v3 decl order swap (mask before t) → 9
- v4 mask split `mask=0xFF0000; ... mask|=0x8080;` between loads → 9
- v5 `mask = 0xFF8080;` right after `func_80060A68()` → 9
- v6 `s32 mask = 0xFF8080;` initialiser at top of fn → 9 (+1 build insn)
- v7 inline arg0[i] no t var (`D_800F1140 = arg0[0]` etc) → 11
- v8 `u8 *p = D_800F1164;` alias in body → 33 (worse)

(Sibling 80061710 also ruled out: identical v1/v2/v3/v5/v6/v7/v8 plus
`*arg0 / *(arg0+N)` deref form — identical asm to `arg0[N]`. Sibling 80061658
also ruled out: split-t into t0/t1/t2 → 11; mask after 3rd load → 9; inline-no-t → 11.)

## Next hypotheses (genuinely untried; require infra beyond this lane)
- **Standalone cc1 `-da` dump on a minimal isolation TU** to expose the .greg
  priority queue and which structural change in C flips mask's vs t's
  spill-cost ranking. Mechanical answer, not guesswork.
- **Directed permuter** seeded from `candidate.c` with PERM_GENERAL biasing
  the v0/v1 allocation between the two pseudos.
- **Family-level resolution** covering func_80061658 / 710 / 617C8 / 618B4 /
  611A4 / 6133C / ACC — if all 7 plateau on the same mechanism, one lever
  (or one canonical decision) covers them all.

## Why blocked, not matched
COMPLETED-C requires 0 pins; pins-stripped honest floor is 9 and no explored
pure-C structure reproduces the target's v0/v1 allocator preference. Genuine
register-rotation plateau, same shape as the 5 documented siblings.
