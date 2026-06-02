---
name: param-local-alias-prologue-pair-flip
paths: ["src/*.c"]
description: "ARCHIVED FORBIDDEN — `Rect *_r = r; s32 *_out = out;` literal-rename aliases declared in target's pair order have zero semantic purpose; exist solely to manipulate cc1's expand_function_start LUID assignment so save_restore_insns emits target's prologue save+def pair order."
metadata:
  type: archived
  status: forbidden
---

# ARCHIVED — param-local-alias-prologue-pair-flip is forbidden

This file used to document a technique for flipping cc1's HIGH-to-LOW
prologue save+def pair order to target's LOW-to-HIGH order by capturing
parameters into local aliases declared in target's pair order:

```c
void func_8007C2A0(s32 *out, Rect *r)
{
    Rect *_r = r;       /* arg1 captured FIRST  — gets low LUID */
    s32 *_out = out;    /* arg0 captured SECOND — gets high LUID */
    /* ... body uses _r and _out instead of r and out ... */
}
```

The rule explicitly described the mechanism as "the C declaration
order of the aliases controls the LUID of their `move` RTL insns".

## Why this is a cheat (user policy 2026-06-02)

`_r` and `_out` are **literal renames** of `r` and `out` — identical
lvalues, identical types, no extra information conveyed. The
function's behavior is byte-identical with or without the aliases.

The declaration order exists **solely** to manipulate cc1's
`expand_function_start` LUID assignment so that `save_restore_insns`
emits the prologue save+def pair sequence matching target. The
rule's justification cites RTL emission order, pseudo creation
order, `expand_prologue` pairing semantics — pure GCC-internals
reasoning.

A human programmer writing this function from its specification
would write:

```c
void func_8007C2A0(s32 *out, Rect *r) {
    /* use out, r directly */
}
```

They would not introduce same-typed locals with leading-underscore
names to capture parameters in a specific order solely to bend
prologue scheduling.

Per [[no-new-park-categories]] "cheats by any spelling":
- Does the construct have semantic purpose? **No** — `_r = r;` and
  `_out = out;` are literal renames.
- Would a human naturally write this code? **No** — it's
  reverse-engineered RTL pseudo-numbering manipulation.
- Does the justification reference GCC internals? **Yes** — LUID,
  `expand_function_start`, `save_restore_insns`, `expand_prologue`.

Same family as the now-forbidden dead-goto label-pad, DImode chain,
goto-end-prologue-delay-slot, register-asm-pin, dead-conditional-
store, and unused-local-array.

## Legitimate vs forbidden

Local variables that carry NEW semantic information are legitimate
(`int total = sum_of_things(); use(total);` — the local holds a
computed value). Local variables that are literal renames of params
with leading underscores, declared in a specific order to bend
prologue scheduling, are not.

## Affected COMPLETED-C functions (need re-derivation)

**`func_8007C2A0`** and **`func_8007C4B8`** (twin) — both committed
af10dc8 (2026-06-01) using this technique. The commit retired 1
regfix rule per function (a 4-insn prologue reorder). Per the
2026-06-02 techniques audit ([[techniques-audit-2026-06-02]]), the
closing form is a cheat-by-spelling. Commit reverted; both functions
re-parked pending search for a clean lever. The original regfix
`reorder 3,4,1,2 @ 1-4` rules return for each.

## Historical content

The original rule body — symptom, mechanism, confirmed-case write-up,
applicability section — is preserved in git history at
`HEAD~1:.claude/rules/param-local-alias-prologue-pair-flip.md` for
the lesson. Read it only to recognize the shape of forbidden
coercion, never as a recipe.

## Related

- [[no-new-park-categories]] — the policy this technique fails
- [[global-label-drift-sibling-cheat]] — sibling forbidden technique
  (dead-goto label-pad)
- [[goto-end-prologue-delay-slot]] — sibling forbidden technique
  (return-value accumulator + shared label)
- [[register-alloc-pure-c]] Lever D — sibling forbidden technique
  (dead self-assign)
- [[techniques-audit-2026-06-02]] — the audit that flagged this rule
