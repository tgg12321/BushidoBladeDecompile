---
name: global-label-drift-sibling-cheat
description: A pure-C retire that changes cc1's global .L label count shifts every later function's label numbers, breaking a sibling's hardcoded-absolute-label regfix rule
paths: ["regfix.txt", "asmfix.txt"]
# broad src/*.c glob removed 2026-06-11: surfaced via codegen-technique-index
---

# A pure-C retire that changes cc1's `.L` label count can break a LATER sibling's hardcoded-global-label rule

## Symptom

You take a queue item, find a genuine pure-C structure that makes `sandbox
--disable all` read 0 **and** is byte-identical to the target in isolation
(real-pipeline, rules-dropped, full-register objdump = 0 diffs for *that
function*). But `retire` rolls back with a non-oracle SHA1, and a whole-file
function-by-function object diff shows the sole change is **1 instruction in a
DIFFERENT function later in the same .c file** — a branch/jump whose target
label moved (e.g. `bnez v0,+0x88` → `+0x48`).

## Cause — GCC `.L` labels are numbered globally across the translation unit

GCC 2.7.2 assigns `.L<N>` numbers from a single per-TU counter (`label_num`),
in RTL-generation order, top of file to bottom. If your pure-C rewrite of an
**earlier** function allocates a different number of labels than the committed
source did, **every function after it shifts** by that delta.

A common trigger: the target uses a **dbr delay-slot-threaded** call/branch
(an instruction duplicated into a jump's delay slot, the jump retargeted past
it). The committed source emits the simpler **full-merge** form (one shared
merge label) and a regfix `insert_after` manufactures the thread post-cc1 using
a **named, non-numbered** label (e.g. `.L_csmd2_b:`) — so the committed cc1
label count is unchanged. To get cc1 to emit the thread *itself* you restructure
(e.g. invert the `if` + add a `goto merge;` label), and the threadable basic-block
layout needs **>=1 extra cc1 `.L` label** than the full-merge form. That +N bumps
the global counter.

The break lands on a sibling whose **regfix/asmfix rule hardcodes an ABSOLUTE
global label number**, e.g.:

```
func_LATER: subst "\.L\d+" ".L280-4" @ 29   # forces the @29 branch to target .L280-4
```

After your +N shift, the label that *was* `.L280` is now `.L(280+N)`, so `.L280`
resolves to a different (earlier) label and the forced branch goes to the wrong
address. The rule is only correct for the exact global numbering the committed
source produced — it is a latent landmine for ANY edit to ANY earlier function.

## Diagnosis (decisive)

1. `sandbox --disable all` = 0 but `retire` rolls back (non-oracle SHA1).
2. Whole-file object diff (`mipsel-linux-gnu-objdump -d` both objects, group by
   `<func>:`, index-aligned, normalize branch-target hex): the only diff is a
   single branch target in a **later** sibling, your worked function is clean.
3. Count cc1 `.L` label defs for your function, committed vs your edit:
   `cpp ... | cc1 ... | grep -coE '^\.L[0-9]+:'` and `grep -oE '\.L[0-9]+' | sort -u`.
   A higher **max** label number = the global shift = the delta the sibling drifted.
4. Grep the sibling's rules for a hardcoded `\.L\d+` literal — that's the victim.

## Resolution — three ways forward

**Preferred (since 2026-06-01): `{lbl#N}` function-local label slots.**
Migrate the sibling's hardcoded-`.L<N>` rule to use the function-local label
slot mechanism in `tools/regfix.py` (commit `[this commit when landed]`).
Replace the literal label reference (e.g. `"j\t.L32"`) with `{lbl#N}` where N
is the 1-indexed position of the target label in cc1's emission order for
THAT function. The substitution happens at regfix-apply time using cc1's
current output, so the rule survives any TU-wide `.L` renumbering — what
matters is the function-local label sequence, which is stable as long as
THAT function's source isn't restructured (which is the same source the rule
is anchored against).

Supported in: `splice` (replacement strings), `subst` (replacement string),
`subst_multi` (each replacement), `insert` (asm text), `insert_after` (asm
text). The `insert_label` directive is for synthesizing NEW stable labels
(different mechanism — when you want to anchor a hand-written branch target
that cc1 wouldn't emit anyway). `{lbl#N}` is for referencing labels cc1
already emits within the function.

To migrate a rule:
1. Probe cc1's current label sequence for the function:
   `bash tmp/probe_func_labels.sh <func_name> [src/<file>.c]`
   (Returns labels in document order, 1-indexed.)
2. Identify which slot # each hardcoded label maps to.
3. Rewrite the rule, e.g. `"j\t.L152"` → `"j\t{lbl#1}"`.
4. `verify-oracle --rebuild` — the bytes don't change (same labels resolved),
   confirming the migration is byte-for-byte correct.
5. The rule is now drift-robust.

**Fallback alternatives (only when {lbl#N} doesn't apply):**

- **Mechanical drift repair:** bump the sibling's hardcoded label by the delta
  (`.L280-4` → `.L(280+N)-4`). Verify by full SHA1 (the oracle is the only proof
  — do NOT trust the number alone; this is exactly the "auto-repair misdiagnoses
  label drift" trap in [[maspsx-noreorder-stripping]]).
- **De-cheat the sibling:** retire the sibling's hardcoded-label rule to pure C
  (removes the landmine entirely), then your function lands clean.

Until then, **park** your function with the full derivation (the pure-C
structure is ready; only the sibling coupling blocks it).

## Confirmed case — cpu_side_move_dir_2 (code6cac_c2.c, 2026-05-28)

Queue top, verdict C, distance 2, 2 regfix `insert_after` rules manufacturing a
dbr-threaded `func_8005FBC8(a0,0x80118800)` call after `if(D_800A38A4==9)a0=8;
else a0=D`. Pure-C trigger `if(D!=9){a0=D;goto call;}a0=8;call:func(...)` → cc1
emits the thread, `sandbox --disable all` 2→0, real-pipeline rules-dropped
objdump 0 diffs full-register vs target. BUT it allocated 2 extra global labels
(committed max `.L73`/13 labels → `.L75`/14). Sibling `func_8003DBE4` (line 1309,
after) has `subst "\.L\d+" ".L280-4" @ 29`; the +2 shift broke it (retire SHA1
`aa40fdb3...`, sole diff func_8003DBE4 idx31 `bnez v0` +0x88→+0x48). Parked
pending user policy decision on the sibling's brittle absolute-label rule.

## FORBIDDEN — dead-goto label-pad + DImode chain (user policy 2026-06-02)

> **Both the dead `goto X; X:` label-pad AND the `unsigned long long new_var2;
> new_var2 = new_var; count = new_var2;` DImode chain are FORBIDDEN as of
> 2026-06-02** ([[no-new-park-categories]] "cheats by any spelling" applied
> to the techniques themselves). Each has zero semantic purpose — they exist
> ONLY to influence cc1's compilation. Same intent as the
> [[lost-codegen-insert-cheat]] / [[inline-asm-injection]] / Lever D
> (dead-param-assign) / dead-conditional-store / empty-if dead-read family,
> just spelled with C goto + label / DImode-typed locals.

### Why they're cheats

The dead-goto pad's mechanism is explicit: bump cc1's `label_num` counter by 1
without changing emitted bytes. The construct is dead by design — jump-
optimization folds it away, leaving zero bytes in the output. Its sole effect
is to make the global counter end at the value the matching rule needs. **A
human programmer would not write `goto X; X:;` with nothing in between.** It
is reverse-engineered coercion against the rule's brittle global-label
references, not source-faithful C.

The DImode chain (`unsigned long long temp; temp = u32_var; count = temp;`)
similarly has no semantic purpose. The `u32 → u64 → s32` round-trip discards
nothing and changes nothing observable; its only effect is to shift cc1's
RTL pseudo numbering enough to change the prologue's scheduling decisions.
Same shape as the volatile-coercion / register-asm-pin / dead-store family.

### What replaces them (the legitimate alternatives are unchanged)

The diagnostic + the policy-resolution sections above (drift detection,
sibling `{lbl#N}` migration per `tools/regfix.py`, sibling de-cheat) remain
valid. The `{lbl#N}` engine mechanism (commit `db690e7`) is the SOTN-clean
way to handle the underlying coupling: it makes the sibling's splice rules
robust to TU-wide `.L` renumbering. With `{lbl#N}` in place, the negative-
delta problem **does not exist** — the sibling rule auto-adapts to whatever
label numbers cc1 currently emits for the target function.

So for any function previously "fixed" via dead-goto label-pad or DImode
chain:
- Migrate the affected sibling's splice rules to `{lbl#N}` (if not already).
- Remove the dead-goto pad / DImode chain from the function.
- Re-derive the function's pure-C body without those constructs.
- If a clean lever exists → committable. If not → re-park; the cheat is no
  longer load-bearing because `{lbl#N}` made it unnecessary.

### Affected COMPLETED-C function (needs re-derivation)

**`tslPolyF4Init` (system.c)** — committed with both constructs as
COMPLETED-C; per the thorough cheat audit
([[thorough-cheat-audit-2026-06-02]]) the commit is a cheat-by-spelling
match. Re-derivation pending Phase 2.

### Historical content (preserved for the lesson)

The pre-2026-06-02 version of this section documented the dead-goto pad as
"the general lever for negative label-drift" and the DImode chain as
"load-bearing for scheduling." Both framings were the rationalization-as-
technique error that the 2026-06-01 cheats-by-any-spelling policy
specifically forbids ("does the form contain code with no semantic
purpose? would a human programmer naturally write this code?"). The full
prior content is preserved in git history at `HEAD~1:.claude/rules/
global-label-drift-sibling-cheat.md` for context — read it only to see
the shape of forbidden coercion, never as a recipe to apply.

## Related
- [[maspsx-noreorder-stripping]] — the other source-change-shifts-a-later-branch
  case; same "fix the label reference, verify by SHA1" discipline, different root
- [[jtbl-rodata-split-infrastructure]] — also "a sibling's index/label-anchored
  rule breaks when output shifts"; there it's the sandbox that can't score
- [[lost-codegen-insert-cheat]] — hardcoded-label/instruction regfix rules are
  the brittle cheats that make this collision possible
