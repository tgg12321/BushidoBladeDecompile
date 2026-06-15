# func_8006E10C (text1b.c) — WIP / BLOCKED (register-alloc + scheduling wall)

## TL;DR
HEAD matches ONLY via 5 register-asm pins (`ff0 asm("$17")`, `temp_s3 asm("$19")`,
`a0v asm("$4")`, `a1v asm("$5")`) — cheat-asm. Clean pin-free body (candidate.c)
produces a correct instruction stream but residual full-build distance **13** is a
coupled **s0 <-> s1 register swap + jal-delay-slot fill**. 5 structural orderings
flatline (13) or worsen (21). Register-allocation+scheduling wall (marionation_Exec
class). Endpoint: permuter (unreliable here, 103 insns + many relocations) or
canonical-asm review.

## The gap (103-insn display-init function)
- Target: `base` (=&D_800F7438) in **$s0**, `ff0` (=0xF0/240) in **$s1**, and the
  `li s1,240` materialization is in the **func_80036EA8 jal delay slot**.
- GCC natural: `base` in **$s1**, `ff0` in **$s0**, a **nop** in the jal delay slot,
  `ff0` materialized later (after the first base setup).
The register swap and the delay-slot fill look coupled — the pin on `ff0`($17=s1)
is what both put it in $s1 AND let GCC schedule its `li` into the call delay slot.

## Levers tried (full build, 0 rules)
| form | score |
|---|---|
| clean (pin-free) | 13 |
| ff0/base reorder x3 (base_first, ff0_then_base, base_before_ff0_after) | 13 |
| base computed at top (base_top) | 21 (worse) |

## Endpoint
Permuter from the pin-free base is the remaining pure-C modality, but for a
103-insn function with many jal relocations the isolated permuter score is
unreliable (per register-alloc-pure-c) — any find must be SHA1-verified in the
full build. greg -da diagnosis could locate the $s0 blocker. If it walls,
canonical-asm review. Blocked on the board with this measured reason.
