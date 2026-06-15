# func_8007B844 — WIP resume notes (current-state; history in git)

## TL;DR

- **Floor 6** (HEAD floor 7), build_insns 38 == target. Lever: named-intermediate
  `u32 mask;` (Lever B from `register-alloc-pure-c.md`). STABLE across 15 sessions,
  55+ structural variants, ~50k permuter iters — re-confirmed 2026-06-14.
- **Mechanism (instrumented, decisive):** target needs the return-staging
  `move $v0, $s0` (ot→v0) scheduled at idx 23 **BEFORE** the mask/addr chain
  (idx 24-26). cc1 `sched.c` gives return-staging `INSN_PRIORITY = 1` (chain depth
  to `jr ra` is 1); the mask/addr chain (`lui→addiu→and→sw`) has priority 4. To flip
  the schedule, the return-staging pseudo's chain depth must reach ≥4. The only
  pure-C way is for a later insn (the store) to **consume the return-value register
  `$v0`** — which needs GCC to keep `ret_val` a SEPARATE pseudo from `ot` after
  copy-prop. **No pure-C SOTN-allowed construct prevents that copy-prop:** every
  `u32 *p = ot; *p = ...; return p;` form folds to `ot`.
- **Sibling evidence (key):** `gpu_ClearOTag` (byte-matched, display.c:177) has the
  IDENTICAL final-statement C tail and ALSO emits return-staging-AFTER-store at the
  score-6 emission order. So **score-6 IS a legitimate matching emission shape** for
  this C tail; the discriminator is the function's preceding BODY shape (single
  vtable call here vs ClearOTag's loop), not the tail's C structure.

## Resume

1. Apply `candidate.c` to `src/display.c` (replace HEAD's func_8007B844 body).
2. Confirm floor: `& tools/eng.ps1 sandbox func_8007B844 --disable all` → `"score": 6`.
3. Do NOT re-derive the `rejected_forms` (meta.json) — 23 forms already ruled out.

## What's left (the 6-diff residual)

6 register-rotation + scheduling diffs in the final mask/addr/return-staging block:
- cc1 picks `$v0` for the mask; target uses `$a0`.
- return-staging emits AFTER the store; target emits it BEFORE (the priority flip above).
These cascade from the single sched.c priority decision; they are NOT independent.

## FORBIDDEN closing forms (do NOT reach for — preserved in rejected/)

- `rejected/conditional_dead_store.c` — Lever D family (conditional dead-store to
  manipulate $v0 liveness). FORBIDDEN per 2026-06-02 cheat catalog.
- `rejected/fnptr_return_type_lie.c` — return-type-lie. FORBIDDEN.

## Untried resume avenues (for the next session — change MODALITY)

1. **Instrumented `BB2_SCHED_DEBUG` on the score-6 form** to read the return-staging
   pseudo's `REG_DEP_TRUE` predecessor list directly — the closing lever must extend
   that chain WITHOUT emitting bytes (combine-foldable) AND without coercing RA into
   the no-new-park-categories family.
2. **Fresh SOTN borderline research** for a community pattern that affects sched.c
   return-staging priority specifically (none found in 15 sessions so far).
3. **Sibling cross-ref of other thin GPU vtable-dispatch wrappers** in display.c:
   does any match pure-C while emitting return-staging BEFORE the store? If so, what
   natural body structure raised its return-staging chain depth?

## Status

INCOMPLETE in queue. Honest floor 6 stable; no pure-C path found across 15 sessions.
Not a GTE/canonical-asm candidate (ordinary C; the gap is a cc1 sched.c priority tie).
No new park category requested (per [[no-new-park-categories]]).
