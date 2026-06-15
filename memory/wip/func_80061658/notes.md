# func_80061658 — WIP (no floor improvement; register-rotation plateau)

## TL;DR (2026-06-14)
HEAD carries two register pins (`register s32 t asm("$2")`,
`register s32 mask asm("$3")`) and 0 regfix rules. Stripping the pins, the
honest masked floor is **9** and NO pure-C form found this session beats it.
This is a register-ROTATION + scheduling plateau (same family as the parked
`func_800618B4`, "register-rotation pin cluster floor 4"). Blocked for a
deeper permuter campaign or canonical-asm/plateau decision.

## The gap (why pins are load-bearing)
Tail does: `t=arg0[0..2]` (loads) interleaved with `mask=0x10FFFF`.
Target:  loads use **v0**, mask uses **v1**, and `lui v1,0x10` is SCHEDULED
between load1's store and load2 (mask kept live in v1 across the loads).
Unpinned GCC: loads land in **v1**, mask in **v0**, mask emitted as its own
block (not interleaved). Masking normalizes register NAMES but NOT the
load/mask interleaving+scheduling, so the diff stays 9 — it is a genuine
register-cycle + schedule coupling, not a relabel.

## Ruled out this session (all >= floor 9, none a cheat)
- split `t` into t0/t1/t2 single-def temps — floor 11
- move `mask=` assignment after the 3rd load — floor 9
- compute `mask=` early (right after the call, before loads) — floor 9
- inline the loads directly into the global stores (no `t`) — floor 11

## Next hypotheses
- Directed permuter from a CLEAN single-function target (caveat: text1b.c is
  the highest-cascade file; register-rotation campaigns elsewhere plateaued at
  large residuals — see memory/project/register-alloc-deep-dive.md).
- `-da` RTL dump to see the v0/v1 cycle and what forces the swap.
- If confirmed immutable: family-level canonical decision (whole sibling set
  func_80061658/710/617C8/618B4/611A4/6133C share the pin pattern).

## Why blocked, not matched
COMPLETED-C requires 0 pins; removing the pins raises the honest distance to 9
and no explored pure-C structure reproduces the target's v0/v1 rotation +
interleaved mask schedule. Genuine register-rotation plateau.
