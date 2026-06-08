---
name: hoist-flag-load-defeat-add-combine
description: Hoist the if-test's pointer-deref into a named local BEFORE the gated `p += K` so combine sees a use of p between two unconditional adds and doesn't merge them.
paths: ["src/*.c"]
---

# Hoist a flag-word load BEFORE its `+=` to split a combine-folded address add

## Symptom

A pointer-walking function has two unconditional `pN += K` adds back-to-back
that target keeps as two separate `addiu $sN,$sN,K1; addiu $sN,$sN,K2`
instructions, but GCC's combine pass merges into `addiu $sN,$sN,(K1+K2)`. The
sandbox `--disable all` distance is small (2) and the lone diff is the
combined add (build_insns target-1; the missing instruction is the second
addiu). Cluster of regfix `subst` rules manufactures the split (typically
also providing the `addiu $sN,$sp,K` materialization the missing split
forces).

## Cause

```c
if ((*(s32 *)arg0 & 8) == 0) return;   /* discarded-result load */
arg0 += 4;                              /* [A] */
arg0 += 8;                              /* [B] */
```

The discarded-result load `*(s32 *)arg0 & 8` is evaluated at the if-test and
its result is dead afterward — GCC's RTL has no surviving use of arg0
between [A] and [B], so combine merges them into `arg0 += 12`.

## The lever — hoist the load into a named local BEFORE [A]

```c
flags = *(s32 *)arg0;                   /* hoist; uses arg0 BEFORE [A] */
arg0 += 4;                              /* [A] */
if ((flags & 8) == 0) return;           /* test the saved value */
arg0 += 8;                              /* [B] — now combine sees the [A] use */
```

Reading the flag word into a named local BEFORE the `+= 4` creates a real
RTL use of arg0 at the load. Combine then sees:

```
(set flags (mem arg0))            ; arg0 USE
(set arg0 (plus arg0 4))          ; def [A]
... if test on flags ...
(set arg0 (plus arg0 8))          ; def [B]
```

The intervening if-test isn't what blocks the merge (jump-threading would
collapse it), it is the load's USE of arg0 between the two adds — combine
needs a clear path from [A]'s def to [B]'s use to substitute, and the
hoisted load's use of arg0 disrupts that. Result: target's exact split
emits naturally, the discarded-result `lw $vN, 0($sN)` at the test position
matches target's bytes (target also emits this load — it's the if-test's
load), zero extra instructions.

## Why this is not a "cheats by any spelling" violation

`flags` carries the semantic content of "the header word at offset 4
holds bit flags". A real programmer reading the function from its
specification would naturally write `flags = ...; arg0 += sizeof(header);
if (!(flags & FLAG_X)) return; ...` — the named flag-word is idiomatic
when its bits get tested and the post-test code continues advancing the
pointer. The lever IS to align with how the original C was written; the
combine-fold defeat is a downstream consequence, not the construct's
purpose.

This is structurally identical to the matched sibling `efc_buki_draw_zanzou`
(src/text1b.c, the function above the affected one) — the sibling reads
`head = arg0[0]; arg0 += 4; if (head != 0x10) return;` AND
`flags = *(s32 *)arg0 & 8; arg0 += 4; if (flags != 0) { ... } arg0 += 8;`
with the same hoist-then-advance idiom, and matches in pure C without
cheats. This rule is just naming the lever the sibling already uses.

## When this lever applies

Look for the cluster of preconditions together:

1. A pointer-walking function with two unconditional `p += K1; p += K2;`
   advances and a regfix that manufactures the split (a `subst` cluster on
   the merged add's bytes, often paired with `addiu $sp,K`/store
   manufactures the missing split forces a stack address materialization).
2. The advance is gated by an if-test that reads through the pointer
   (`if ((*(T *)p & mask) == 0) return;`).
3. Sandbox `--disable all` distance is small (2), build_insns target-1.
4. The function has a matched sibling using the hoist idiom (read the
   sibling first — its existence IS evidence the hoist is the
   project-standard form).

Hoist the if-test's load into a named local BEFORE the first advance,
test the local instead of the inline expression. Verify by sandbox 0 +
SHA1 == oracle.

## When this does NOT apply

- The if-test's load is needed in its original position for scheduling
  reasons elsewhere in the function (e.g., a later code path also reads
  from that address — the hoist might over-extend `flags`'s live range).
- The two adds are NOT both unconditional — sibling-style conditional
  reassignment between them already defeats the merge via the
  conditional def. Use the sibling-pattern two-call form instead
  ([[cross-jump-call-merge]] companion).
- No matched sibling exists in the same file with the hoist idiom — the
  lever may still apply, but vet against the cheats-by-any-spelling
  checklist explicitly (the sibling is the strongest evidence-of-intent
  there is for this pattern).

## Confirmed case — func_800484A0 (text1b.c, 2026-06-07)

Queue top, verdict C, distance 5, 5 regfix rules. Two WIP sessions
(floor 5 → 4 → 2) explored the residual: the sandbox-2 candidate had
`arg0 += 4; arg0 += 8;` back-to-back, combine-folded to `arg0 += 12`.
7 forms rejected (p_buf alias, double-assign, interleaved stores,
goto-skip, single-statement interleave, if-wrap, offset-dim-load).

Session 3: hoisting `flags = *(s32 *)arg0;` to BEFORE the `arg0 += 4`
(mirroring the sibling at src/text1b.c:267) → `sandbox --disable all`
2 → 0; `retire` dropped all 5 regfix rules; SHA1 == oracle; 100% pure
C, build_insns 36 = target 36.

## Related

- [[defeat-combine-symbol-fold]] — sibling combine-defeat lever for a
  different combine pattern (displaced-symbol address fold). Both rules
  share the "give combine a reason to bail" intent; the levers differ
  by what the intervening use looks like.
- [[cross-jump-call-merge]] — the companion sibling-pattern two-call
  form (the session-2 lever); this rule closes the residual the two-call
  form leaves.
- [[register-alloc-pure-c]] — the parent RA playbook; the hoist also
  affects RA (`flags` lands in `$v0`, naturally matching target).
- [[no-new-park-categories]] — the cheats-by-any-spelling policy this
  lever was vetted against (the sibling's existence is the strongest
  human-programmer-would-write-this evidence).
