---
name: no-new-regfix-rules
paths: ["regfix.txt", "asmfix.txt"]
description: "Standing policy 2026-06-08: net additions to regfix.txt/asmfix.txt are forbidden. SOTN — the PS1 decomp community standard — has NO regfix-equivalent; their bar is pure C OR INCLUDE_ASM (= our COMPLETED-INLINE-ASM-CANONICAL). Every BB2 regfix/asmfix rule is debt against that bar. Enforced by tools/hooks/no_new_regfix_guard.py."
metadata:
  type: rules
---

# Standing policy: no new regfix/asmfix rules

> **User policy, codified 2026-06-08:** Net additions to `regfix.txt` and
> `asmfix.txt` are **forbidden by default**. The only legitimate completion
> states are pure C (zero rules, zero cheat-asm) or canonical-asm
> authorization. There is no middle layer.

## The SOTN comparison

The PS1 decompilation community standard — established by Symphony of the Night
(SOTN), Vagrant Story, ESA, CTR, and other long-running matching projects — has
**no equivalent of `regfix.txt` / `asmfix.txt`**. Their model is binary:

- **Pure C** — the function compiles to byte-identical target output with no
  post-processing of the assembly stream. This is the default goal.
- **`INCLUDE_ASM`** — the function is canonical hand-coded asm (no C form
  exists). The whole-body `__asm__("glabel ...")` block is the accepted finished
  form. This is our **COMPLETED-INLINE-ASM-CANONICAL** state.

That's it. No "and also a small list of post-cc1 rules that paper over a
register diff." No "scheduling reorders applied via an external pipeline stage."
No "label substitutions that adapt the assembly to a layout the C didn't quite
produce." Every such rule in `regfix.txt` / `asmfix.txt` is **BB2-specific
debt** that the SOTN-bar projects do not carry.

## Why this is a policy, not a guideline

The history this rule responds to:

- **Label-shift cascades.** Every `auth:` commit that removes a function's
  C body shifts cc1's global `.L<N>` counter — and every hardcoded `.L<N>`
  reference in a sibling rule has to be hand-updated. See
  [[global-label-drift-sibling-cheat]] (and the `{lbl#N}` migration, which
  exists *because* hardcoded label rules are brittle). The
  cumulative friction of maintaining cross-function-coupled rules grew large
  enough that Phase 2 of this initiative is a project-wide migration to
  function-local label slots.
- **Brittle anchors.** Rules anchor on instruction-stream indices, opcode
  text, regex literals. Any sibling function's source change can shift those
  anchors. Phase 1 of this initiative cataloged the failure modes; the friction
  is documented across multiple `.claude/rules/` entries
  ([[lost-codegen-insert-cheat]], [[inline-asm-injection]],
  [[sandbox-zero-retire-fails]], [[switch-vs-ifchain-branch-sense]], etc.).
- **Agents producing cheat-class rules.** When a worker hits a residual
  diff, the path of least resistance is to add a one-line `subst` or
  `insert_after` rule. The audit ([[completion-standard]]) catches the
  obvious classes (`addu $r,$0,$zero` injection, `subst "srl" "sra"`-class
  opcode flips), but new patterns surface faster than the catalog can keep
  up. **Default-deny on net additions** closes the door at the source.

## The rule

`tools/hooks/no_new_regfix_guard.py` (a commit-msg hook chained off
`.git/hooks/commit-msg`) blocks any commit whose staged diff against
`regfix.txt` or `asmfix.txt` introduces a **positive net count** of rule
lines.

- A "rule line" is one matching `^[a-zA-Z_][a-zA-Z_0-9]+:` — the
  `<func_name>:` prefix used by both files. Comments, blank lines, and
  continuation lines (e.g. extended `splice` payloads) don't count.
- The math is `added_rule_lines - removed_rule_lines`. Modifications-in-place
  (Phase 2 migration shape: `s/old anchor/new anchor/` on existing rules)
  net to zero and pass. Pure removals net negative and pass. Only the
  positive-net case is blocked.

## The escape hatch (narrow, closed-list)

Genuinely-new infrastructure rules — not register-rotation paperwork — get
through via a commit-body tag:

```
[infra-rule: <category>]
<one-line justification>
```

The category MUST be one of the sanctioned values below. Anything else
(or no tag at all) blocks.

| Category | What it covers | Example |
|---|---|---|
| `jtbl-infra` | Rodata-split jump-table renames / jlabel marks / delete_between for an `asm/data` jtbl ([[jtbl-rodata-split-infrastructure]]). | `replay_camera_rob_back_loose2: rename ".L28" "jtbl_800108CC"` |
| `prologue-coordination` | Paired `tools/prologue_config.json` + regfix `reorder` for prologue layout. Use ONLY when the config-stage genuinely needs the reorder ([[prologue-fix-redundant-reorder]] documents when it doesn't — most cases). |
| `maspsx-label-nop` | Load-delay-after-label gate; coordinates with `maspsx_label_nop_funcs.txt` ([[maspsx-label-nop-gate]]). |
| `lwl-fix` | `lwl`/`lwr` handling per `FIX_LWL_FILES` in the Makefile. |
| `multu-pad` | Canonical `multu`/`mflo` scheduling pad (PSX hardware requirement; not a codegen workaround). |
| `reviewer-fail-revert` | Restoring rule lines **byte-identical** to ones deleted by a commit now being reverted after a cheat-reviewer FAIL verdict (retroactive audit or pre-commit). The commit body must name the reverted commit and the verdict. Not a license for new rules. User decision 2026-06-10 (saFidLoad retroactive-audit revert). |

The categories are deliberately **narrow** — they correspond to documented
infrastructure mechanisms with no pure-C equivalent. They are NOT general
buckets for "I need to add a rule and have a justification." When in doubt,
the answer is one of the three options at the top of this rule (find the
pure-C lever, authorize as canonical-asm, or park).

If you genuinely need a new infrastructure category, that's a **user policy
decision** ([[no-new-park-categories]]) — surface it; do not invent a new
escape-hatch category in the hook.

## What if you really, really need a new rule?

Read this section last. The default-deny works because the alternatives are
real:

1. **Find the pure-C lever.** The catalog in `.claude/rules/` documents
   ~50 techniques that have closed similar walls. Start with the one whose
   symptom matches yours. Then read the GCC source
   ([[no-compiler-divergence]]) for what shape the C must take. Then permute
   ([[difficult-is-not-impossible]]). The matching C exists; finding it is
   the work.
2. **Authorize as COMPLETED-INLINE-ASM-CANONICAL.** If the construct
   genuinely has no C form (GTE ops, BIOS trampolines, hand-coded signals
   per [[hand-coded-asm-recognition]]), the canonical end-state is a
   whole-body `__asm__("glabel ...")` block ([[canonical-asm-retirement]],
   [[canonical-asm-authorization-recipe]]). This requires user sign-off.
3. **Park with documented lever-exhaustion.** A genuine endpoint has the
   form "I tried levers A, B, C; here's the exact measurement showing each
   didn't close it; I cannot derive any further un-tried lever from the
   compiler source / RTL dumps / matched siblings / permuter evidence."
   The function stays INCOMPLETE in `engine/queue.json` until a future
   session finds a lever or it gets re-classified.

## Related

- [[no-new-park-categories]] — sibling user policy: do not invent new
  cheat-tolerant park categories either. The two policies together draw the
  same line: no new cheat-tolerant outcomes.
- [[completion-standard]] — the COMPLETED-C bar this rule enforces.
- [[community-standard]] — what SOTN / VS / ESA / CTR actually accept
  (the reference frame for "SOTN bar").
- [[lost-codegen-insert-cheat]] — the audit-detector-caught family this
  rule pre-empts. Phase 3 (this guard) closes the gap the catalog can't
  keep up with.
- [[inline-asm-injection]] — sibling cheat family: rules expressed as
  hardcoded-`$N` `__asm__` blocks instead of regfix entries. Both are
  forbidden.
- [[global-label-drift-sibling-cheat]] — the Phase 2 migration target
  (`{lbl#N}` function-local label slots) that this Phase 3 guard sits
  above.
