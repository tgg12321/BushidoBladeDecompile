# func_8003D330 (code6cac_c2.c) — WIP / BLOCKED (register-allocation tie)

## TL;DR
GPU primitive OT-link (addPrim) — 27 insns, no args. HEAD matches via 4
register-asm pins (code $a1, mask_lo $a2, mask_hi $a3, p $v1) + a bogus
`ori %0,%0,0xFFFF :: "r"(base)` coercion asm — all cheats. The clean pin-free
named-const body (candidate.c) reaches full-build distance **13**, a
register-allocation tie. Best clean floor 13 across 6 decl/structure orderings.

## The tie
- Target: the three constants in **$a1**(code=0xE100001F) / **$a2**(mask_lo=
  0x00FFFFFF) / **$a3**(mask_hi=0xFF000000), with **$a0** kept free for the
  `*(u32*)p` temp. p in $v1.
- GCC natural: code -> $a0, masks swapped ($a2<->$a3), and the two `*ot` reads
  in the final RMW pair reordered. Since this is a NO-ARG function the three
  constants are independent and GCC's ascending-register tiebreaker has full
  freedom — it doesn't reproduce the target's arg-register (a1/a2/a3) layout.

## Levers tried (full build, 0 rules)
| form | score |
|---|---|
| temps / ordered / inline_const (named consts) | 13 (best) |
| p_first / code_first_use | 16 / 19 |
| clean inline-mask | 19 |

## Endpoint
Most promising of my blocked set for the **permuter**: only 27 insns and few
relocations, so the isolated permuter score should be more faithful than the
larger walls. greg -da could find why $a0 isn't skipped. If it walls,
canonical-asm review (an a1/a2/a3 arg-register layout in a no-arg function is a
hand-asm hallmark). Blocked on the board with this measured reason.
