# func_80078A68 — WIP (blocked-lane triage 2026-06-14)

## TL;DR
Item-slot init (text1b_b.c:0x80078a68). Matches ONLY with 1 regfix rule
(reorder 12,11). Single-insn **cc1-internal delay-slot fill choice**: target
eager-fills the outer `beqz` delay slot with `andi v0,a2,0x10` (then-block's
first insn); cc1 instead moves `sh a1,8(v1)` down into the slot. NOT closed
this pass — directed permuter is the named untried avenue.

## The exact gap (cc1 reorg.c)
```
80078a94: sh   zero,4(v1)
80078a98: sh   a1,8(v1)        <- target keeps this BEFORE the branch
80078a9c: beqz v0,...          (if ((u32)t0 < 2))
80078aa0: andi v0,a2,0x10      <- target DELAY SLOT (then-block first insn, eager-fill)
```
cc1 moves `sh a1,8(v1)` into the slot instead. Confirmed cc1-internal
(fill_simple_delay_slots) — raw cc1 output before maspsx already mis-fills.

## Resume steps (next session)
1. DIRECTED PERMUTER (PERM_*) on this function from the rule-removed base.
   FIRST: check what lever closed the SIBLING func_80078B04 (same file, same
   class, 7-session wall, closed 2026-06-12 commit 03c07363) — it may transfer.
   Mirror funcs func_80078B3C / func_80078B70 share this fill choice.
2. Make `sh a1,8(v1)` unmovable into the slot (data-dep that ties it to a
   post-branch value) so cc1 must eager-fill from the then-block.
3. Read cc1 reorg.c fill_simple/eager_delay_slots tiebreak; craft a C shape
   making the then-block insn the only legal fill candidate.

## Ruled out (do NOT re-derive)
- HEAD body, rule removed -> DIFF (rule load-bearing).
- `v0 = arg2 & 0x10` named temp before inner if -> DIFF, == HEAD-no-rules.
- duplicate `sh a1,8` into both arms -> DIFF (store no longer single-before-branch).
- flip outer cond to `>= 2U` (swap then/else) -> DIFF (different fill).
- (prior card) store-order swap; ((s16*)base)[2]/[4] array index; narrow s16 arg1.

## Pointers
- `.claude/rules/switch-vs-ifchain-branch-sense.md`, `.claude/rules/goto-end-prologue-delay-slot.md`
- Sibling: func_80078B04 close-out (commit 03c07363) — check the transferable lever.
