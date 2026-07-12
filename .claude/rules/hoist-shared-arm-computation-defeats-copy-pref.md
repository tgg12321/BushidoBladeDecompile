---
name: hoist-shared-arm-computation-defeats-copy-pref
description: When two branches duplicate the same `sum = X + Y` expression and residual is register-choice ($v1 vs $a0) on the sum, hoist the shared computation OUT of both arms. GCC's jump2 duplicates it back into arms during codegen, but the single-pseudo RA now picks the right register.
paths: [".claude/rules/hoist-shared-arm-computation-defeats-copy-pref.md"]
---

# Hoist a shared computation out of two branches — single-pseudo RA breaks copy-preference propagation

## Symptom

An `if/else` pair computes the SAME expression in each branch, feeding a common
downstream compare:

```c
if (cond) {
    x = func(y);     /* y flows in as an arg -> gets $a0 copy-pref */
    if (x == -1) { return -1; }
    z = x + y;       /* branch-1: z inherits y's $a0-pref via expand_preferences */
} else {
    x = k;
    z = x + y;       /* branch-2: same z assignment */
}
if (z > BOUND) { ... }
```

The cheat-free `sandbox --disable all` distance is small (3-5 words). The
target computes the sum expression per-arm at physically separate addresses
(e.g. `addu $v1, $a3, $s0` in the taken-arm's delay slot + a second copy at
the else-arm's join label). Yours matches structurally BUT the residual is
register-choice: target has sum in `$v1`, yours has it in `$a0`, with matching
delay-slot fill and compare-source diffs.

## Mechanism (measured 2026-07-12, saTan2Main = _SsVabOpenHeadWithMode)

Two separate `z = x + y` expressions in the source generate TWO sum pseudos
in RTL, both carrying `REG_DEAD y` at their add insn. GCC 2.7.2's
`global.c:expand_preferences` merges preferences: `y`'s `$a0`-pref (from an
earlier arg-copy call, e.g. `func(y)` in the same branch) propagates to sum
because they don't conflict (y dies at the add). `find_reg`'s preference pass
then picks `$a0` for sum. MIPS has no `REG_ALLOC_ORDER` to break the tie.

The forbidden variants (verified negative, session 8): split-init
(`sum = X; sum += Y;`) combine-refolds; declaration-position (inert);
`do {...} while (0)` wrap (worse, blocks cross-jump merge).

## The fix — hoist the shared computation

```c
if (cond) {
    x = func(y);
    if (x == -1) { return -1; }
} else {
    x = k;
}
z = x + y;                 /* SINGLE expression, single pseudo */
if (z > BOUND) { ... }
```

With one sum pseudo, RA determines its register from its ONE use (the compare).
The a0-pref chain no longer propagates from a branch-local y-use into sum.
GCC picks `$v1`.

**Bytes at codegen time still match target's per-arm structure** because
GCC's `jump2` pass duplicates the simple assignment back into both branches
when doing so eliminates a jump. The residual `addu $v1, $a3, $s0` insns
appear per-arm in the emitted code — exactly like target.

## Why this is not a "cheats by any spelling" violation

The C form is textbook DRY refactoring. Applied to unrelated code:

```c
/* Before: */
if (cond) { x = f(y); z = x + y; }
else      { x = k;    z = x + y; }

/* After: */
if (cond) x = f(y); else x = k;
z = x + y;
```

Any programmer reading a code review would extract the duplicate. The
construct has:

1. **Real semantic purpose.** The sum IS the merged-flow value; it doesn't
   diverge between arms, so one expression models the flow correctly.
2. **Human-programmer plausibility.** SOTN's psxsdk source ships the
   equivalent shape (though with the sum inlined into the compare rather
   than a named temporary). Either is idiomatic.
3. **A justification that doesn't reference GCC internals.** "Compute once,
   compare once" is the argument; the RA side-effect is what makes it match
   target bytes, but the C shape stands on its own merits.
4. **No dead code, no unused declarations, no naming-announces-intent, no
   permuter-only find.** Derived by direct reasoning from the banked
   mechanism.

Contrast with forbidden families:
- **`duplicated-statement-into-arms`** (SANCTIONED 2026-07-01) is the
  INVERSE — deliberately duplicating a real statement into arms with
  `/* FAKE */` annotation and byte-neutrality verification. This rule is
  the honest merge direction; no FAKE annotation needed.
- **`dead-store-fake-exception`** — bearing a dead store to bias RA. This
  rule involves no dead stores.
- **`split-read-defeats-hoist`** — duplicating a real load into branch
  arms to control materialization. Sanctioned; this rule is its structural
  cousin in the other direction.

## When this applies

- The residual is REGISTER-CHOICE ($v1 vs $a0 or similar) on a value fed
  by an argument-passing chain into a downstream compare.
- The offending expression is DUPLICATED in the source's if/else arms.
- The variable feeding the sum has a copy-preference from an earlier call
  (`greg` dump shows `expand_preferences` propagating).
- The hoist is byte-neutral for everything else in the function (only the
  offending register-choice cluster changes).

## When this does NOT apply

- The two branches compute DIFFERENT expressions (not truly shared) — the
  duplication reflects real semantic divergence.
- The shared expression has SIDE EFFECTS that must fire per-arm (a call
  with mutation, a volatile access). Hoisting changes semantics.
- Combine already unified the branches (measure: sandbox score doesn't
  drop when you hoist — that means the expressions weren't distinct at
  RTL time). Look elsewhere for the lever.
- The target's per-arm bytes come from a source that TRULY duplicated the
  expression (verify via other tells — e.g. presence of dead per-arm work,
  arm-specific naming, a semantic asymmetry). If Sony's source really did
  duplicate, session 8's per-arm form is the right shape; this rule
  doesn't apply.

## Diagnosis recipe

1. `greg` dump the current form (`cc1 -da base.i`), read
   `;; Register dispositions:` — confirm sum's pseudo (call it N) is
   allocated to `$a0` and `y`'s pseudo carries an `$a0` copy-pref.
2. Check `;; N conflicts:` — if N doesn't conflict with `y`'s pseudo, the
   `expand_preferences` merge fires; the lever will help.
3. Apply the hoist; re-measure. Score should drop.
4. If oracle-clean via `retire`: the jump2 duplicate-into-arms fires at
   codegen — no source-level indication needed of the byte-level per-arm
   structure.

## Confirmed case — saTan2Main (main.c, 2026-07-12)

Queue item `saTan2Main` = PsyQ 4.0 LIBSND vs_vh `SsVabOpenHeadWithMode`,
dist 245 at session start. Session 8 (2026-07-10) applied the SOTN psxsdk
transcription to floor 5 (245/247 insns) with the per-arm sum form.
Residual: 4 real words at malloc-fail/overflow join (sum in `$a0` not
`$v1`). Session 8 named hypothesis #1: "find an honest spelling that makes
sum conflict var_s0".

Session 13 (2026-07-12) applied the hoist: single `sum = spuAllocMem +
var_s0;` after the if/else. Sandbox 5 → 0 first try. `retire` dropped the
whole-body asmfix splice (asmfix:124). SHA1 == oracle. Layer-1
cheat-reviewer FAILed on "novel unsanctioned shape" grounds; user
adjudicated 2026-07-12: the C is textbook DRY; jump2's arm-duplication is
standard codegen; the RA-mechanism is post-hoc explanation of why the
natural form happens to match, not the primary justification. SANCTIONED
as a project pure-C lever.

## Related

- [[register-alloc-pure-c]] — the parent RA-lever playbook; this is a
  new leaf under Lever B/C (structural respelling to alter conflict sets).
- [[duplicated-statement-into-arms]] — INVERSE lever (sanctioned 2026-07-01);
  deliberately duplicate a real statement across arms to influence RA
  priority. This rule is the honest-merge direction.
- [[split-read-defeats-hoist]] — sibling structural-cousin lever
  (duplicate-into-arms for a load); both operate on the interaction
  between source-level duplication and RTL-level RA.
- [[compare-operand-order-register]] — sibling register-choice lever
  (RTL operand-order steering); this rule is source-structure-level, that
  one is expression-level.
