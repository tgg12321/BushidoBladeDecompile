# Evidence bank — myRobGeneiMove

- s1 (2026-07-28): target asm proves `angle` lives in $s0 and is live across the
  single_game_getEnemyCharId call (subu $v1,$v1,$s0 AFTER the jal at BDC4); `neg_angle`
  is also $s0 (negu $s0 @ BE08, sh $s0 @ BE68). The "coalescing wall" was a variable-
  identity fact: original C reused ONE variable (val) for angle + neg_angle. Merging
  them closes the floor to 0 — no dead stores, no pins, pure live variable reuse.
- s1: sandbox --disable all = 0 with edits in src (134/134). Baseline re-measured 14
  at current HEAD before the edit.

- WIP rejected_form: {'form': 'far = (expr) >= 0x391 directly (drop the throwaway early `val =`)', 'score': 14, 'reason': 'No change — the early throwaway val was not the blocker; the no-call val portions still take $a0.'}

- WIP rejected_form: {'form': 'declare val/far/a2 before obj/s2 (pseudo-number / coalescing nudge)', 'score': 14, 'reason': 'No change — allocation of val to $a0 vs $s0 is robust to declaration order.'}

- == imported from memory/wip notes.md ==
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

