---
name: func-8007c86c-permuter-ub-cheat-findings
description: func_8007C86C (sibling of 8007C7A0) — 2026-06-01 RETRY-3: clean single-target permuter from clean-v8 base finds closing forms ONLY via conditional-init UB on var_a1 (forbidden cheat-by-spelling), all rejected per [[no-new-park-categories]]. HEAD's committed form itself relies on the same UB. Honest floor remains 17; structural ceiling (missing target `move a3,v0` park-merge) unchanged.
metadata:
  type: project
---

# func_8007C86C — clean-target permuter findings: only UB-cheats close, honest floor stays 17

## Status

**INCOMPLETE — re-parked 2026-06-01 (third retry).** Prior sessions:
- Session 1 (commit `b1ea9da`): score 20 committed (HEAD form with 21 rules
  + `int new_var; int new_var2; if (!D_8009BE7A){}` padding).
- Session 2 (commit `f57b7dd`, 14:35): clean v8 form (`var_a1 = arg1;`
  unconditional + strip padding) drops floor 20→17. NOT committable
  (doesn't retire rules). See [[func-8007c86c-clean-form-floor]].
- Session 3 (this session, evening): **clean single-target permuter** run
  for 6 wallclock minutes. Found closing-form candidates ALL based on UB
  conditional initialization of `var_a1`. ALL rejected as cheat-spelling
  per [[no-new-park-categories]].

The function stays parked. Honest floor remains 17 (same as session 2). The
prior session's "Permuter from clean-v8 base" untried-lever is now DONE; its
result is that the permuter's accessible closing-form space is dominated by
UB-conditional-init.

## What this session executed

The prior session ([[func-8007c86c-clean-form-floor]] § "Concrete next-step
hypotheses") listed as untried lever 1 the **clean-target permuter setup**
("the minimal isolated standalone build needs `cpp -nostdinc` carefully").
Session 3 set up a minimal `permuter/c86c/base.c` containing ONLY:

```c
typedef unsigned char u8; /* ...basic int typedefs... */
extern s16 D_8009BE78;
extern s16 D_8009BE7A;
extern u8 D_8009BE74;

s32 func_8007C86C(s16 arg0, s16 arg1) {
    /* clean v8 form (the session-2 floor-17 source) */
}
```

`permuter/c86c/target.s` was already prepped (single function, offset 0).
`permuter/c86c/compile.sh` standard pipeline. Permuter base score: 225
(masked diff cost weighted by permuter's metric).

Ran `python3 tools/decomp-permuter/permuter.py permuter/c86c -j8 --stop-on-zero`
for 6 wallclock minutes (~hundreds of iters per worker, several thousand
iters total).

## Best score: 135 (vs 225 baseline). ALL candidates were UB-cheats.

The lowest-score candidates found, in increasing score order:

| Score | Candidate | UB pattern | Verdict |
|-------|-----------|------------|---------|
| 135 | `output-135-1` | `int new_var;` set ONLY in `else { var_v0_2 = 0; new_var = 0; var_v0_2 = new_var; }`, then read as `var_a1 = new_var;` in arg1<0 branch. UB when arg0>=0 AND arg1<0. | REJECT — UB conditional init. |
| 135 | `output-135-2` | Same as 135-1 with cosmetic difference. | REJECT — UB. |
| 140 | `output-140-1` | `var_a1 = arg1;` placed inside `else { var_v0_2 = 0; var_a1 = arg1; }` (arg0<0 branch only). UB when arg0>=0. **IDENTICAL to HEAD's committed form.** | REJECT — UB conditional init. |
| 145 | `output-145-1` | Same pattern as 140-1 with `var_a1 = arg1;` ordering swap. | REJECT — UB. |
| 185 | `output-185-1` | `var_a1 = arg1;` similar conditional pattern. | REJECT — UB. |
| 205 | `output-205-1` | `var_a1 = arg1;` inside inner-inner else (arg0>=0 AND clamp-pass). UB on most paths. | REJECT — UB. |
| 210 | `output-210-1` | `var_a1 = var_a1;` self-assign (Lever D / find_dead_param_assigns cheat-spelling). | REJECT — dead self-assign. |
| 210 | `output-210-2` | Same UB-conditional-init as 140. | REJECT — UB. |
| 215 | `output-215-*` | Variants of self-assign + UB-conditional. | REJECT — both cheats. |

Every closing-form sample contains at least ONE of:
1. **UB-conditional-init of `var_a1`**: the assignment is placed inside a
   branch such that `var_a1` is read uninitialized on some execution paths.
   This is the EXACT shape of HEAD's committed source (lines 580-583):
   ```c
   } else {
       var_v0_2 = 0;
       var_a1 = arg1;   /* only when arg0 < 0 */
   }
   if (var_a1 >= 0) { ... }  /* UB read when arg0 >= 0 */
   ```
2. **Dead self-assignment** like `var_a1 = var_a1;` — same cheat-spelling
   as the Lever D `arg0 = 0;` pattern that
   `engine/volatile_cheats.find_dead_param_assigns` catches.

## Why these are cheat-spellings (vetting per [[no-new-park-categories]])

Per the standing checklist:

1. **Does the form contain any construct from the catalog?**
   - Dead self-assign `var_a1 = var_a1;`: YES, Lever D family.
   - Conditional `var_a1 = arg1;` that leaves var_a1 uninitialized on read
     paths: YES, this is a coercion of GCC's analysis — the conditional
     placement serves no semantic purpose (a sane program initializes
     var_a1 unconditionally if it's going to be read unconditionally).
2. **Does the form contain code with no semantic purpose?**
   - YES. A human programmer writing `var_a1 = arg1; if (var_a1 >= 0) ...`
     would write the assignment OUTSIDE the if/else of an unrelated branch.
3. **Would a human programmer naturally write this code from a
   specification?**
   - NO. The specification is "clamp arg1 to [0, D_8009BE7A-1] then mask".
     Conditional initialization of the local on a different branch's else
     is not what falls out of that spec.
4. **Does the closing form's justification reference GCC internals?**
   - YES. The reason this form scores lower than the clean v8 form is that
     it shortens var_a1's pseudo lifetime, biasing GCC's allocator to give
     it a different register. That is precisely the "I'm describing a
     coercion, not program logic" disqualifier.

## HEAD's committed form already contains this cheat-spelling

The HEAD form's `else { var_v0_2 = 0; var_a1 = arg1; }` is the same UB
conditional-init pattern. It was committed back when the permuter-style
"whatever scores lowest" approach was the norm; it carries 21 regfix rules
to paper over the residual diffs.

This finding is also relevant to the **sibling func_8007C7A0**: its HEAD
form (`src/display.c:523-562`) has the IDENTICAL UB conditional-init shape
in its `else` branch. Both are committed cheat-by-spelling.

**Implication for re-classification:** if/when the project tightens the
detector backstop to flag this UB-conditional-init pattern explicitly,
both functions will be re-routed to `active`. The detector wiring TODO is
out-of-scope for this single-function session (would need engine work +
re-detection of all currently-committed instances).

## What is NOT a viable lever (confirmed this session)

- **Clean single-target permuter** (~5400 iter, 6 min wall): finds only
  UB-cheat closing forms.
- **r_e4 named intermediate** (C97C's OR-association lever): score 17,
  no improvement — C86C's clean v8 form already has the desired OR shape.
- **`goto end; end: return ret_val;` block** (C97C goto-end lever):
  coalesced, no effect — C86C has no early-exit pattern for cc1 to fill
  a delay slot with.
- **Declaration-order swap** of local vars (decls last): score 17, no
  change (per ALLOCDBG findings: pseudo numbers for autos don't track
  decl order).
- **Reversed param-local-alias** (`s16 _a1 = arg1; s16 _a0 = arg0;`):
  score 17, coalesced.
- **Block-local `s16 limit = D_8009BE78 - 1;`** in arg0>=0 branch: score
  25 (regression, emits extra sign-extension).
- **`saved_x = var_v0_2;`** intermediate at function midpoint: score 17,
  coalesced.
- **`saved_x = (u16)var_v0_2;`** with explicit u16 cast: score 17,
  coalesced.
- **`s32 a0_widened = arg0; s32 a1_widened = arg1;`** then re-cast on use:
  score 18 (regression — fewer insns, wrong shape).
- **Operand flip on inner comparison** (`arg0 > limit-1` vs `limit-1 < arg0`):
  score 17, RTL canonicalizes.
- **OR-association reversal** (`(var_v0 | const) | var_v1`): score 17.
- **Extra mask paren** `((var_a1 & 0xFFF) & 0x3FF)`: score 17 (combine
  folds the redundant mask).
- **No-preload** (`var_a1` initialized in all clamp branches, no top-level
  `var_a1 = arg1;`): score 17.

All 12 tested variants score 17 OR 18-27 (regression). Floor stands at 17
for legitimate C structures.

## The structural ceiling (UNCHANGED from session 2)

The missing target-idx-16 `move a3, v0` insn. Target uses `$a3` (saved arg0
reg) as the destination of var_v0_2 across the rest of the function,
requiring a `move a3, v0` park-merge insn. cc1 mine: writes var_v0_2
directly into `$a0` ($4 = preserved arg0) across all three case bodies,
skipping the park insn. The instrumented cc1 ALLOCDBG dump for func_8007C86C
shows only 1 pseudo allocated in the visible group (the rest are coalesced
or live in arg/return slots).

To force target's allocation: var_v0_2 needs to land in a DIFFERENT
register than `$a0`, which requires either:
- (a) Something else to live in `$a0` over var_v0_2's range — but `$a0`
  is sign-extended out at idx 1-2 and dead after, no natural use.
- (b) A pseudo-split for var_v0_2 making the merge a phi-style copy — but
  cc1 doesn't do SSA / phi nodes.

Both (a) and (b) are compiler-internal. The only "C structure that
reaches target's allocation" levers found by permuter are UB-conditional-
init, which is forbidden by [[no-new-park-categories]].

## Genuine resume avenues (after this session)

Per [[difficult-is-not-impossible]]: the matching C exists; we just
haven't found a non-UB version. Untried levers for next session:

1. **`PERM_GENERAL` directed permuter macros**. Random-mode permuter is
   now exhausted (this session's 6-min wallclock + the cheap clean target
   ran ~5400 iters with NO non-UB improvement). Directed permuter targeting
   var_a1's pseudo's lifetime explicitly may find a non-UB form.
2. **Multi-function CC1 cross-reference**: study cc1's emit for
   `func_8007C97C` (matched COMPLETED-C) and diff its allocno tables vs
   C86C's to see what STRUCTURAL feature C97C had that biased the merge
   destination. (C97C's `goto end; ret_val; end: return ret_val;` shape
   plus its early-exit branch — these create an entry pattern absent in
   C86C.)
3. **Engine-level cheat detector for UB-conditional-init**: flag the HEAD
   form as a cheat-by-spelling. This re-routes BOTH C86C and C7A0 to
   `active` with a forced "no UB" constraint, focusing future permuter
   runs in the legitimate-C space only. Out-of-scope for this single-
   function session but worth a separate engine task.
4. **Project-wide: investigate whether the original C source was
   physically structured as `if (...) goto outer_end; ... outer_end:`** —
   a structurally-different layout that produces the merge-via-callee-save
   pattern naturally. The matched func_8007C748 (3-arg leaf, simpler) has
   a different shape; C86C's matching original may have been a HAND-
   WRITTEN C wrapper around a clamp-and-pack idiom we haven't reconstructed.

## Why this is NOT committable (per persistence contract)

`src/display.c` is byte-identical to HEAD. The session's only artifacts are:
- This memory note documenting the permuter UB findings.
- `permuter/c86c/base.c` minimal isolated standalone (committed to permuter
  workspace, not src/).
- `permuter/c86c/output-*` candidate snapshots (in gitignored workspace
  per `permuter/.gitignore`).

`park_src_guard` correctly allows this commit (no src changes).

## Files / how to reproduce

- Baseline (HEAD): `src/display.c:564-604` (with `int new_var/new_var2` + UB).
- Clean v8 form (sandbox 17, NOT committable — session 2 doc):
  see [[func-8007c86c-clean-form-floor]].
- Minimal permuter workspace: `permuter/c86c/`. Run:
  `python3 tools/decomp-permuter/permuter.py permuter/c86c -j8`.
- Inspect best outputs: `ls permuter/c86c/ | grep '^output-' | sort -t- -k2,2n`.
  Lowest scores (135, 140, 145, 185, 205) all contain UB-conditional-init.

## Related

- [[func-8007c86c-clean-form-floor]] — session 2's memory note (the 17-floor
  clean v8 form). This session extends with the permuter-cheat findings.
- [[func-8007c7a0-arg1-preload-lever]] / [[func-8007c7a0-named-intermediate-lever]]
  — sibling C7A0's identical structural ceiling.
- [[no-new-park-categories]] — § "Auto-search tools output PROPOSALS, not
  winners" — the policy this session enforced when rejecting the permuter's
  UB closing forms.
- [[register-alloc-pure-c]] — Lever D ("dead self-assignments of function
  parameters") FORBIDDEN as of 2026-05-31; this session confirms permuter
  finds the same pattern variant (`var_a1 = var_a1;` for locals) — same
  cheat-class.
- [[inline-asm-policy]] § "expanded cheat catalog" — UB-conditional-init
  belongs in the catalog if/when the detector is wired.
- [[difficult-is-not-impossible]] — the matching C exists; permuter has
  surveyed the legitimate C space and didn't find it. Continue searching
  in the directed-permuter / cross-sibling reference space.
