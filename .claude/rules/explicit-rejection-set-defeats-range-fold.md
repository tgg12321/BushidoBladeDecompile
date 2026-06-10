---
name: explicit-rejection-set-defeats-range-fold
description: Replace a 2-element subtract-range exclusion `(u32)((s32)a - K) >= 2` with explicit `a != K && a != K+1` — extends the variable's && chain liveness, biases RA to keep it in the load-destination register, and materializes target's `andi $vN, $a, 0xFFFF` that combine folded away from the subtract form.
paths: ["src/*.c"]
---

# Replace a 2-element subtract-range exclusion with explicit `!= K1 && != K2`

## Symptom

A u16-loaded global / field / parameter is range-excluded against a small
adjacent constant set in an && chain, written as the optimized
"subtract-then-unsigned-compare" form:

```c
u16 a1 = *(u16 *)(entry + 0x6A);
if (a1 != 0xA && *(s16 *)(entry + 0x72) == 0 &&
    (u32)((s32)a1 - 0x17) >= 2 && *(s16 *)(entry + 0x96) == 0) {
    ...
}
```

The cheat-free `sandbox --disable all` distance is small (≤3). The diff
cluster sits at the comparison block:
- `lhu` lands in `$v1` (build); target uses `$a1`.
- Missing `andi $v1, $a1, 0xFFFF` — combine folded away the redundant mask.
- `addiu` in delay slot uses `$v1`; target uses `$a1`.

Regfix carries swaps like `$3 <-> $5 @ N` and `insert "andi $3,$5,0xFFFF" @ M`
that re-materialize what GCC's combine pass elided.

## The lever — write the rejection set explicitly

When the subtract form excludes a 2-element set `{K, K+1}`, the literal
equivalent is two explicit `!=` tests:

```c
if (a1 != 0xA && *(s16 *)(entry + 0x72) == 0 &&
    a1 != 0x17 && a1 != 0x18 && *(s16 *)(entry + 0x96) == 0) {
    ...
}
```

GCC's combine pass **recomposes** `a1 != K && a1 != K+1` BACK into the same
`addiu $vN, $a1, -K; sltiu $vN, $vN, 0x2; bnez $vN` byte sequence the
subtract form produces — so the range check's emitted bytes are unchanged.
What's different is the FIRST comparison's RTL context: the && chain now
references `a1` at three distinct compare points instead of one explicit
`!= K` plus one subtract expression. The extended liveness of `a1` biases
GCC's register allocator to keep `a1` in its load-destination register
(`$a1` for a 5th-arg / lhu landing), and the first `a1 != 0xA` compare
then needs `andi $v1, $a1, 0xFFFF; beq $v1, $v0` — emitting the andi
combine was folding away in the subtract form.

Net: lhu/andi/addiu register pattern flips to match target; rules retire.

## Why this is not a "cheats by any spelling" violation

Per [[no-new-park-categories]] / [[review-discipline-before-commit]]:

1. **Semantic purpose** — YES. `a1 != 0x17 && a1 != 0x18` literally tests
   the rejection set {0x17, 0x18} as two explicit equality exclusions.
   Identical predicate to the subtract form for any u16-bounded `a1`.
2. **Human-programmer test** — YES. Two-element exclusion as `!= K1 && != K2`
   is the more idiomatic / readable form a programmer would write from
   spec. The subtract trick `(u32)(x - K) >= N` is the optimization-aware
   form; the explicit `!= K && != K+1` is the natural form.
3. **GCC-internals justification** — the RA biasing IS a consequence, but
   the construct has independent semantic justification (the explicit
   rejection set). Same family as the sanctioned RA levers in
   [[register-alloc-pure-c]] (block-local var split, narrow integer type) —
   the source change has semantic content and the RA outcome falls out.
4. **Permuter-derived** — NO. Derived from inverting the subtract trick.
5. **Family check** — NOT in the forbidden narrow-type-cast family. There
   is no type-coercion RTL node injected; this is a boolean-expression
   restructure with semantic content.
6. **Naming-announces-intent** — N/A, no new variables.

## When this lever applies

Look for the cluster of preconditions together:
- A `u16`/`s16` local cached from a load (typically `u16 a1 = *(u16 *)P;`).
- An && chain with one `a1 != K` test PLUS one subtract-range-exclusion
  `(u32)((s32)a1 - L) >= N` (or `(u32)(a1 - L) >= N`).
- The range exclusion's `N` is small (2 in the confirmed case; the lever
  scales for N=2 ⇒ 2 explicit tests, N=3 ⇒ 3, etc., but pay-off shrinks
  fast as `N` grows because GCC may NOT recompose 3+ explicit tests into
  the range form — verify with objdump after substitution).
- Target's comparison block shows `lhu → andi → beq` (the andi cc1 elided
  in the subtract form, present in target) AND the range check's
  `addiu+sltiu+bnez` cluster.

Substitute `(u32)((s32)a1 - K) >= 2` with `a1 != K && a1 != K+1`. Verify
that GCC's combine still recomposes the explicit `!=`s into the
`addiu+sltiu` range check (sandbox `--disable all` should produce
build_insns equal to target). If combine doesn't recompose (build_insns
goes up), the lever doesn't apply — fall back to other levers.

## When this does NOT apply

- The exclusion set is non-adjacent (e.g. `a != 0x10 && a != 0x17`). GCC
  cannot recompose to a range check; the lever produces extra `xori+beq`
  instructions instead.
- The exclusion set has 3+ elements. The explicit form is N branches in
  the && chain; combine may not recompose all of them; instruction count
  diverges from target.
- The first compare in the && chain isn't `a != K0` against the same
  variable — the lever's RA-biasing effect relies on `a`'s extended
  liveness across MORE compares than baseline.
- A non-u16 typed variable. The lever's andi-materialization mechanism
  relies on the lhu/andi/$a1 register flow specific to halfword loads;
  s16/u8/s32 variables don't trigger the same pattern.

## Confirmed case — func_8001EEB4 (code6cac.c, 2026-06-10)

Queue top, verdict C, 11 sessions of WIP work (HEAD floor 6, candidate
floor 3 via [[hoist-call-arg-local-flips-jal-delay]] idx2 hoist), 30
forms rejected including (s16), (unsigned short), (char), and (u32)
casts and various && reorderings. Sessions 2-10 all hit the 3-diff
cluster: `lhu → $v1` (target $a1), missing andi, `addiu → $v1`.

Session 11 substituted `(u32)((s32)a1 - 0x17) >= 2` →
`a1 != 0x17 && a1 != 0x18`: sandbox 3 → 0, build_insns 58 → 59 = target,
all 4 rules retired (`$3 <-> $5 @ 14`, `insert "andi $3,$5,0xFFFF" @ 16`,
`$3 <-> $5 @ 21`, `reorder 36,38,40,39,37,41 @ 36-41`). SHA1 == oracle.

Objdump of the sandbox output at the comparison block produces
byte-identical bytes to target (lhu/andi/beq/lh/bnez/addiu/sltiu/bnez
sequence at offsets matching target's exact pattern). GCC's combine
recomposes the two explicit != tests into the addiu+sltiu range check,
while the first `a1 != 0xA` retains its andi materialization due to
`a1`'s extended liveness across the chain.

Cheat-reviewer PASS verdict 2026-06-10 (6-test checklist, no failure).

## Related

- [[register-alloc-pure-c]] — the parent RA-via-C-structure playbook;
  this lever is in the "extend a variable's liveness via more compare
  points" family.
- [[hoist-call-arg-local-flips-jal-delay]] — sibling lever applied in
  the same function (idx2 hoist for jal delay-slot fill); this rule's
  lever closed the residual after that one was applied.
- [[no-new-park-categories]] — the cheats-by-any-spelling policy this
  lever was vetted against; the explicit-rejection-set form passes
  because it has independent semantic content.
- [[review-discipline-before-commit]] — the cheat-reviewer architecture
  that PASS-vetted this lever after demanding objdump evidence of the
  actual mechanism.
- [[inline-asm-injection]] — sibling REJECTED family; the forbidden
  narrow-type casts ((s16), (unsigned short), (char)) defeat the andi
  fold via RTL coercion node injection. This lever achieves the same
  effect via boolean restructure with semantic content — categorically
  different family.
