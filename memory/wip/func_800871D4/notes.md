# func_800871D4 — BLOCKED (likely cc1-vs-cc1psx andi-fold divergence + coupled RA/sched)

## TL;DR
main.c bit-flag setter (reads D_8010280A=u16, builds two shift masks, clears a
table slot, RMWs four flag globals). HEAD "matches" via cheat-asm: two
`__asm__("andi %0,%1,0xffff" ...)` blocks (general-purpose `andi` opcode =
forbidden inline-asm cheat, [[inline-asm-policy]]) + an empty
`__asm__("" : : "r"(var_a1))` scheduling barrier. INCOMPLETE.

## The core blocker: a redundant-andi fold our cc1 does and the target didn't
`temp_a0 = D_8010280A` (u16) makes GCC PROVE temp_a0 <= 0xFFFF, so
`var_v1 = temp_a0 & 0xFFFF` is folded away (candidate.c v1: the andi vanishes,
19 diffs). Writing it as two stmts (`var_v1 = temp_a0; var_v1 &= 0xFFFF`)
preserves the andi but emits an EXTRA `move` (v2: `move a0,v1; andi v1,v1` vs
target's single `andi v1,a0`). There is NO pure-C form that yields a single
`andi v1,a0` from a u16-bounded source: one-expr folds it, two-stmt adds a move.
The target's standalone `andi v1,a0` (from a u16 lhu) is the signature of a
**cc1-vs-cc1psx fold divergence** -- the `__asm__ andi` cheat papers over it.

## Other coupled diffs (v2, 18)
- load register: ours v1, target a0.
- if/else branch scheduling differs (sllv/j placement, branch sense).
- var_a1<->var_a2 swap on the final OR-RMW pair.
The empty `__asm__("" :: "r"(var_a1))` barrier was holding var_a1 live to fix
the a1/a2 ordering -- another over-optimization defeat.

## Avenues for next session
- **cc1psx calibration** on candidate.c: if PsyQ cc1psx emits the standalone
  `andi v1,a0` (no move, no fold) from the same u16 read, this is a confirmed
  compiler divergence -> escalate (not pure-C-closable without the cheat).
- if cc1psx ALSO folds, hunt a pure-C form for the andi + the a1/a2 ordering.

## Floor
- HEAD distance 15 (2 andi cheat-asm + 1 empty-asm barrier).
- candidate.c (v1, no cheats): 19. v2 (split mask): 18. Floor NOT lowered.
