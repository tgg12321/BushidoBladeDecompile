---
name: global-label-drift-sibling-cheat
description: A pure-C retire that changes cc1's global .L label count shifts every later function's label numbers, breaking a sibling's hardcoded-absolute-label regfix rule
paths: ["src/*.c", "regfix.txt", "asmfix.txt"]
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

## Confirmed case — tslPolyF4Init (system.c, 2026-05-28 parked → 2026-05-29 RESOLVED) — a NEGATIVE delta closed via dead-goto label pad

Queue top, verdict C, distance 2, 4 regfix rules papering the done dispatch (idx
50 `li v0,-1`→`move v0,zero`; idx 56 `beq s0,v0`→`bnez v0`). Target reuses the
loop's `-1` sentinel (left in `v0` by `count != -1`) as the "exhausted" flag at
the merge and the 3rd-call success sets `v0=0` in the beqz delay slot — i.e. a
`bnez v0` flag test, not a `count == -1` re-test. Pure-C fix: thread a **separate**
`s32 result` (NOT the call-result var, or GCC threads the success edge straight to
the call and collapses the merge → distance 6): success `result = 0; goto done;`,
exhaust `result = -1;`, dispatch `if (result != 0) return 0;`. `sandbox --disable
all` 2→0; full-register objdump of the function vs canonical = **0/81 diffs**.

The parked form consumed **one FEWER** global `.L` label than HEAD (cc1 max `.L`
232 vs 233), drifting every later system.c sibling -1 and breaking three
hardcoded-absolute-`.L` cheats (`marionation_Exec`, `saEft00Add`,
`cpu_side_move_dir_4`).

### The fix — a dead `goto X; X:` pad to restore the +1 `.L` allocation

The drift was closed by inserting an unconditional `goto done_label_pad;
done_label_pad:` between the `result = -1;` line and the `done:` label:

```c
    g_cd_callback_a = saved;
    result = -1;
    goto done_label_pad;     /* restores the +1 .L allocation that HEAD's
                              * `count == -1` test ate; emits zero bytes. */
done_label_pad:
done:
```

Mechanism: cc1's `label_num` counter is bumped at *every* RTL label allocation,
including labels that don't survive optimisation. The dead `goto X; X:` is folded
away by jump-optimisation (zero bytes emitted, region label set is unchanged),
but `label_num` was already incremented during the front-end's RTL-gen pass — so
the global counter ends at HEAD's value. Confirmed: file-wide max `.L` 232 → 233
(matches HEAD); siblings' label numbers unshifted; `verify-oracle --rebuild` SHA1
== oracle; 4 regfix rules retired. 100% pure C, all siblings keep their cheats
(they're not pure-C-matchable per their own park rationales — see
[[cross-jump-store-tail-merge]] saEft00Add and [[register-alloc-pure-c]]
cpu_side_move_dir_4 / marionation_Exec).

### Hybrid was load-bearing — keep HEAD's local-var declarations

The local-var declarations matter for scheduling. HEAD's body had:
```c
s32 count;
unsigned long long new_var2;
s32 idx;
s32 saved;
int new_var;
s32 *elem;
...
new_var = 3;
new_var2 = new_var;
count = new_var2;
```
Replacing this with a plain `s32 count; ... count = 3;` shifts the
`elem = &g_cd_sector_buf[idx]` scheduling so `system.o` no longer matches oracle
(different register/scheduling at the prologue's `sll/lui/addiu`). The final
matching form **adds** `s32 result;` to HEAD's declaration block (keeping
`new_var`/`new_var2`/the `unsigned long long` chain) and **restructures only the
done dispatch** — *not* the early initialisation. So when reaching for this rule,
keep the existing source's odd declarations + initialisation chain intact; only
the dispatch surface is the work surface.

### The general lever
For a *negative* label-drift (your matching C allocates FEWER `.L`s than the
committed form), add a dead `goto X; X:` to bump `label_num` by exactly the delta
needed without changing emitted bytes. Doesn't apply to the positive-delta case
(cpu_side_move_dir_2's +2) — you can't easily *reduce* `label_num` from C source.

## Related
- [[maspsx-noreorder-stripping]] — the other source-change-shifts-a-later-branch
  case; same "fix the label reference, verify by SHA1" discipline, different root
- [[jtbl-rodata-split-infrastructure]] — also "a sibling's index/label-anchored
  rule breaks when output shifts"; there it's the sandbox that can't score
- [[lost-codegen-insert-cheat]] — hardcoded-label/instruction regfix rules are
  the brittle cheats that make this collision possible
