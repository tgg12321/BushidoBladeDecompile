# D_80083418 — branch reproducible in C, register-rotation is the wall (ings2.c)

## TL;DR (2026-06-14)
Honest distance 13 (12 regfix rules stripped); build_insns 96 vs target 97.
Two separable problems:
1. **SOLVED in pure C** — the missing instruction is a 2nd `beqz $s1,$0`
   (the reload check) that GCC folds on first loop entry. Moving `i=0;
   tbl=base;` ahead of the `outer:` bits==0 check makes GCC emit it
   (96 -> 97 insns). This retires the lost-codegen `insert beq $17,$0` cheat.
2. **WALL** — coupled register rotation the 11 `subst` rules encode:
   - target `one`=1 -> $s4, `mask_const`=0xFFFFFF -> $s3, `base` -> $s5;
     mine swaps $s4/$s5.
   - done-block: target `lw v0; lui v1; and v0,v0,v1` / `addu v1,v1,v0`;
     mine has $v0<->$v1 swapped.
   Declaration-order swap did NOT move the RA. Same plateau class as
   func_80072CD4 this session.

Net: no floor improvement (the branch-correct base scores 14 because the RA
swap, no longer hidden behind the merged branch, costs +1). Blocked; the
branch finding + clean base are preserved for the next agent.

## Resume steps
1. Apply `rejected/goto_outer_recheck_97insn.c` to src/ings2.c — it is
   branch-correct (97 insns) and clean pure C (reviewer PASS). Confirm
   `sandbox D_80083418 --disable all` ~ 14.
2. Attack ONLY the RA swap from there (the lost-codegen insert is then dead).

## Live hypotheses
- Lever A on `one` vs `mask_const`: try `mask_const` referenced before `one`
  in the loop expression, or hoist one of them into a block-local, to flip
  $s4/$s5.
- done-block: read `*ctrl` into a fresh local instead of reusing `val` to flip
  which value lands in $v0 (the $2/$3 swap).
- Instrumented cc1 ALLOCDBG ([[register-alloc-deep-dive]]) for the $s4/$s5 tie.

## Ruled out (do not re-derive)
- swap decl/init order of one & mask_const -> 13 (no change).
- while-loop restructure / goto-outer-recheck -> 14 (branch fixed, RA +1).

## Pointers
- `.claude/rules/register-alloc-pure-c.md` — Step-0 + Levers; the
  "confirmed limits" coupled-rotation class is the precedent.
- Sibling on the same wall this session: `memory/wip/func_80072CD4/`.
