# myRobGeneiMove — `val` $a0/$s0 coalescing wall (code6cac.c)

## TL;DR (2026-06-14)
Honest distance 14; both builds 134 insns (structure correct). 13 of the 15
regfix rules are `subst $4 -> $16`: the `val` variable should live in $s0
(callee-saved) but GCC keeps val's PRE-CALL portions in $a0 ($4). The
cross-call portion DOES land in $s0 (greg pseudo 113 in reg 16) — so this is a
COALESCING problem: target merges all of val's reassignments into ONE $s0
pseudo; GCC fragments them and gives the no-call branches $a0. far-direct and
declaration-order levers did NOT move it. No floor improvement; blocked.

## Resume steps
1. `sandbox myRobGeneiMove --disable all` on HEAD = 14 (15 rules stripped).
2. Diff is consistently `s0` (target) vs `a0`/`a1` (mine) on val and its
   neighbour. `val` is assigned in many branches (val=expr; =0x200;
   =*(obj+0x184); +=v>>12; =(result*...)>>10) then read at the `diff = val -
   old` join AFTER the single_game_getEnemyCharId call.

## Live hypotheses
- COALESCING lever: compute each branch's val into a block-local, then ONE
  `val = blocklocal;` at the join — may let global.c merge into a single $s0
  pseudo instead of per-branch $a0 pseudos.
- Instrumented cc1 ALLOCDBG: is val one pseudo the allocator split, or
  multiple the coalescer declined to merge?
- register-alloc-pure-c confirmed-limits class -> canonical-asm review.

## Ruled out (do not re-derive)
- far = (expr) >= 0x391 directly (drop throwaway early val) -> 14.
- declare val/far/a2 before obj/s2 -> 14.

## Pointers
- `.claude/rules/register-alloc-pure-c.md` — Lever A (live-range) is the
  closest; this is a coalescing variant of it.
- Contrast: mot_data_set (text1a_c, CLOSED this session) was a single clean
  swap fixed by one reorder; this is a multi-def coalescing decision that
  resists reordering. Siblings on the wall: func_80072CD4, D_80083418,
  func_800200DC.
