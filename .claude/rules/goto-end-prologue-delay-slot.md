---
name: goto-end-prologue-delay-slot
paths: [".claude/rules/goto-end-prologue-delay-slot.md"]
# on-demand only: surfaced via codegen-technique-index (auto-loads on src/*.c)
description: "ARCHIVED FORBIDDEN — the `s32 ret_val; if (arg==NULL) {ret_val=0; goto end;} ... end: return ret_val;` accumulator+shared-label has zero semantic purpose; exists solely to make reorg.c fill the bnez/j delay slots with the prologue and zero-assignment. Same intent as the dead-goto label-pad / DImode chain / inline-asm-injection family."
metadata:
  type: archived
  status: forbidden
---

# ARCHIVED — goto-end-prologue-delay-slot is forbidden

This file used to document a "lever" for the 4-insn entry pattern
(`bnez/sp-adjust-delay/j/zero-delay`) target asm sometimes has. The
technique: restructure `if (x == NULL) return 0;` as

```c
s32 ret_val;
if (x == NULL) {
    ret_val = 0;
    goto end;
}
/* ... body that computes the real result ... */
ret_val = real_result;
end:
return ret_val;
```

with the explicit instruction that the shared `end:` label was
"load-bearing" so the body's final return goes through it.

## Why this is a cheat (user policy 2026-06-02)

The function semantically has **one return value** and **one true
return path**. The early-exit returns 0; the body computes the real
result. `return 0;` and `return ret_val;` (where `ret_val = 0`)
produce **identical observable behavior**. The accumulator + shared
label encode **no new program behavior** — they exist solely to
give `reorg.c` the dependency graph it needs to fill the `bnez`'s
delay slot with the prologue's `addiu sp,-N` and the `j`'s delay
slot with the `addu $v0, $zero, $zero` zero-assignment.

The rule's mechanism section was the textbook cheat signal: it
described "the delayed-branch pass", "GCC's jump.c may try to thread
the goto end", "INSN_PRIORITY", "the goto-end form's RTL emission"
— pure GCC-internals reasoning. The rule explicitly named the
construct a "lever". A human programmer writing the function from
its specification would write `if (x == NULL) return 0;` and let the
compiler emit whatever it emits — they would not invent a
return-value accumulator and a shared-label control-flow rerouting
just to bend `reorg.c`.

Per [[no-new-park-categories]] "cheats by any spelling":
- Does the construct have semantic purpose? **No** — `return 0;`
  vs `return ret_val;` (= 0) is identical behavior.
- Would a human naturally write this code? **No** — it's
  reverse-engineered RTL control.
- Does the justification reference GCC internals? **Yes, exclusively**.

Same family as the now-forbidden dead-goto label-pad, DImode chain,
register-asm-pin, dead-conditional-store, and unused-local-array.

## Legitimate exception (not what this rule was teaching)

`goto end; end: cleanup; return ret_val;` with REAL shared cleanup
work (closing file handles, freeing memory, releasing locks, etc.)
at `end:` is a legitimate C idiom for managed-resource exit paths.
This rule is NOT about that pattern. The rule specifically teaches
the bare `end: return ret_val;` zero-shared-work form — the
accumulator + label exist only for codegen, no cleanup.

## Affected COMPLETED-C function (needs re-derivation)

**`func_8007C97C`** — committed bc346de (2026-06-01) using this
technique. The commit retired 9 rules; per the 2026-06-02 techniques
audit ([[techniques-audit-2026-06-02]]), the closing form is a
cheat-by-spelling. Commit reverted; function re-parked pending
search for a clean lever. The original 9 regfix substs return to
regfix.txt.

## Historical content

The original rule body — symptom description, full 4-lever mechanism
write-up, the func_8007C97C reference match — is preserved in git
history at `HEAD~1:.claude/rules/goto-end-prologue-delay-slot.md`
for the lesson. Read it only to recognize the shape of forbidden
coercion, never as a recipe.

## Related

- [[no-new-park-categories]] — the policy this technique fails
- [[global-label-drift-sibling-cheat]] — sibling forbidden technique
  (dead-goto label-pad) with same intent
- [[register-alloc-pure-c]] Lever D — sibling forbidden technique
  (dead-param-assign) with same intent
- [[techniques-audit-2026-06-02]] — the audit that flagged this rule
