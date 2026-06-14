# func_800611A4 (text1b.c) — WIP / BLOCKED on RA wall

## TL;DR
Honest pure-C floor = **9**, and all 9 diffs are a **single v0<->v1 register-name
swap** on the post-call cluster. The build is otherwise byte-identical to target
(same instructions, same order, same load/store interleave). HEAD only "matches"
via two `register asm("$2")`/`asm("$3")` pins (cheat-asm). The swap is **NOT
reachable from correct pure C** — confirmed against BOTH our GCC 2.7.2 port AND
the original PsyQ cc1psx (both put load-temp in $v1, mask in $v0; target is the
reverse). BLOCKED pending canonical-asm authorization or a family-wide RA lever.

## The exact gap
Post-call body does, for i in {0,1,2}: `t = arg0[i]; D_800F114{0,4,8} = t;`
plus `D_800A3464 = 0xFFFFEF;` interleaved.

- **Target:** load-temp `t` -> `$v0`, mask `0xFFFFEF` -> `$v1`.
- **Ours (pin-free):** load-temp `t` -> `$v1`, mask -> `$v0`.

GCC's allocno priority hands the lower register (`$v0`) to the short-live-range,
CSE'd/REG_EQUIV'd constant mask, displacing the 3x-reused (fragmented, 3-web)
load temp to `$v1`. Target wants the opposite.

## Why it's a wall (not just "untried")
- ~12 pure-C levers tried (see meta.json rejected_forms + next_hypotheses):
  temp split, block-local split, const split, mask live-across-call, ptr deref,
  reorder. All score 9 or worse. None flip the swap.
- **cc1psx calibration (decisive):** the original PsyQ compiler produces the SAME
  allocation as our port (load-temp `$3`, mask `$2`). So target's swap is not what
  EITHER compiler emits from the semantically-correct C — the original source
  must have had a structurally different form, or this is a genuine RA wall.
- Only ways to force the swap are FORBIDDEN cheats: the `$2`/`$3` register-asm
  pins (current HEAD) or a regfix `subst $2 $3`.

## Resume guidance
1. Apply `candidate.c` (pin-free) and confirm `sandbox --disable all` == 9.
   Do NOT re-derive the rejected_forms.
2. Treat as a FAMILY: func_800618B4 (already PARKED, same "register-rotation pin
   cluster"), func_80061250, func_80061658 share the identical $2/$3 pin +
   post-call mask/load shape. A real lever here likely retires all of them.
3. Promising untried avenues (meta.json next_hypotheses): permuter from the
   floor-9 base; check if mask 0xFFFFEF is legitimately a non-constant in the
   original (would change allocno priority); instrumented-cc1 PRIO dump to find
   the minimal C change that inverts the two pseudos' priorities.

## Status
Source left at HEAD (oracle green). Card BLOCKED with reason pointing here.
This needs USER canonical-asm authorization (if the $2/$3 pins are accepted as
the canonical finished form for the family) OR a future family-wide RA lever.
