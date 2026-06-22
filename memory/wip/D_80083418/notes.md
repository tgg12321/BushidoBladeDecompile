# D_80083418 — floor 13→7 via three pure-C levers (ings2.c)

## TL;DR (2026-06-22)
Honest distance HEAD 13 → candidate 7 (97 vs target 97 insns). Three structural
levers, all pure C, cheat-reviewer PASS:

1. **Init order `mask_const, base, one`** — flips global-allocator priority so
   `one`→$s4 and `base`→$s5 (matches target). Retires the 4 `subst $20<->$21`
   register-rotation regfix rules. Prior session tried decl-order; this swaps
   *assignment* order in the function body (not declaration order). The
   `(log2(n_refs)*n_refs)/live_length` priority math is in `global.c:611`.
2. **Outer structure `i = 0; if (bits == 0) goto reload; tbl = base;`** —
   placing `i = 0;` BEFORE the branch keeps GCC from folding the redundant
   bits-check, restores the missing 2nd `beqz $s1` (96→97 insns), and puts
   `move $s0, $zero` (i=0) in the branch delay slot exactly as target.
   Retires `insert beq $17,$0,{lbl#4} @ 19`.
3. **Error-loop `p[i << 2]`** — replacing `*(s32 *)((u8 *)p + (i << 4))`
   with the idiomatic s32-array index (mathematically identical) flips the
   addu destination from $v0 to $v1 in the debug_printf loop. Retires the
   `subst addu $2,$2,$3 → addu $3,$3,$2 @ 71` regfix.

Oracle SHA1 still matches with these src changes (existing 12 rules cover the
residual 7-position diff). Floor measurably lowered; src/ reverted per WIP
protocol so the next agent applies `candidate.c` and resumes from floor 7.

## Resume steps
1. Apply `candidate.c` to src/ings2.c. Confirm `sandbox D_80083418 --disable all`
   reports score 7 (97 build_insns vs 97 target).
2. Attack the residual 7. Two clusters (see meta.json remaining_gap):
   - **Init prologue scheduling order** (~2 ops): RA is correct, but
     scheduler emits `mask, base, one` while target emits `one, mask, base`.
     Sched1 picks the lui+ori/lui+addiu chains over `addiu $s4, 0, 1`.
   - **Done-block $v0/$v1 swap on val** (~5 ops): pseudo 94 (val, /v global)
     conflicts with hardreg 2 because pseudo 96 (const 0xFF000000) gets
     local-allocated $v0 first. Split-val variants drop to 95 insns
     (scheduler fuses delay-slot nops).

## Live hypotheses
- Instrumented cc1 with `BB2_ALLOC_DEBUG` / `BB2_PRIO_DEBUG` to read the
  exact local-alloc priority for pseudo 96 vs the global pseudo 94 — see
  [[register-alloc-deep-dive]] for the recipe. Goal: find a structural
  lever that makes val win $v0 over const.
- Split val into single-write locals BUT add a control-flow barrier that
  prevents scheduler from fusing the load-delay slots. Hard without a cheat
  barrier; needs creativity.
- For prologue order: try to make `one`'s critical path APPEAR longer to
  sched1 so it issues first. The `sllv $v0, $s4, $v0` consumer is in inner,
  far away. Maybe an early dummy reference to `one`? (Borderline.)

## Ruled out (do NOT re-derive — see meta.json rejected_forms)
- Decl-order swaps (8 variants, all floor 9 once init+outer lever applied).
- `i += one` and `bits >>= one` — float to 10 but introduce `addu` vs `addiu`
  (cheat-by-spelling: using `one` solely to influence RA in non-mask contexts).
- `mask_const = (one << 24) - 1` (mask via one) — breaks lui+ori emission.
- Inline base re-read (drop `base` var) — 94 insns, breaks LICM hoist.
- Split val into separate locals — drops to 95 insns (scheduler fuses nops).
- compound_andne / via_xor / via_subtract / complement done-block rewrites.
- register hints on val / ctrl / one — no RA effect.

## Pointers
- `.claude/rules/register-alloc-pure-c.md` — Step-0 + Levers (the candidate
  is essentially Lever A applied to assignment ORDER, not just declaration).
- `.claude/rules/no-new-park-categories.md` — the cheat-by-spelling lens
  that ruled out `i += one` / `bits >>= one`.
- Prior WIP base (`rejected/goto_outer_recheck_97insn.c`) — branch-correct
  base from 2026-06-14 that the (1)+(2) levers improved on.
