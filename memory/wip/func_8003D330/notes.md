# func_8003D330 (code6cac_c2.c) — WIP / BLOCKED (scheduler-driven register-allocation tie)

## TL;DR
GPU primitive OT-link (addPrim) — 27 insns, no args. HEAD matches via 4
register-asm pins (code $a1, mask_lo $a2, mask_hi $a3, p $v1) + a bogus
`ori %0,%0,0xFFFF :: "r"(base)` coercion asm — all cheats. The clean pin-free
named-const body (candidate.c) reaches full-build distance **13**, a
register-allocation tie. Best clean floor 13 across 11 decl/structure orderings.

## The tie — root cause confirmed by RTL
- **Target** cc1psx: code→$a1, mask_lo→$a2, mask_hi→$a3, p→$v1, ot→$a1(reused), tag→$a0.
  Scheduler put the ot-load sequence (`lui $a1; lw $a1`) BETWEEN the code-store and the
  tag-load → code/tag do NOT interfere → tag gets $a0 (code's freed register).
- **Our cc1**: scheduler HOISTS `lw tag, 0(p)` BEFORE `sw code, 4(p)` to hide the 1-cycle
  MIPS load latency. This creates code/tag live-range overlap. Pseudo-72 (tag) "dies in
  2 places" → deferred to global allocator. Global alloc: pseudo 72 conflicts with all
  regs 2-7 → only $t0 available. RTL dumps (lreg/greg) confirm this mechanically.
- **Root cause**: cc1psx vs decompals/mips-gcc-2.7.2 scheduler quality difference. Not a
  C-structure problem — no pure-C construct creates the needed load-store dependency that
  -O2 would honor without being a cheat.

## Levers tried — all score 13 (11 variants total)
| form | score |
|---|---|
| named-const ordered (candidate.c) | 13 (best) |
| mask_hi before mask_lo | 13 |
| ot/tag first in declarations | 13 |
| code-inline (no named code var) | 13 |
| ot-before-code (ot assigned before sw-code) | 13 |
| split tag1/tag2 (separate variables) | 13 |
| u32*-p (p as u32* throughout) | 13 |
| ot-init-at-decl | 13 |
| all-inline (no named masks) | 13 |
| p_first / code_first_use | 16 |
| clean inline-mask | 19 |

## Endpoint
**Permuter** (if available): 27 insns + few relocations → isolated score should be faithful.
This is the best known hope. If unavailable, escalate: the a1/a2/a3 arg-register layout in
a no-arg function is a hand-asm hallmark (S1 signal) → canonical-asm review by Adjudicator.
