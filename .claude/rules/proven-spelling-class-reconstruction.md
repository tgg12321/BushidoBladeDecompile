---
name: proven-spelling-class-reconstruction
description: Narrow same-bytes respelling exception — sanctioned only with mechanism-level proof the original source used a different spelling class, plain-C form, most natural variant, last lever (user policy 2026-06-10)
paths: ["src/*.c", "regfix.txt", "asmfix.txt"]
---

# Proven-spelling-class reconstruction — the narrow same-bytes respelling exception

**User policy 2026-06-10 (InitHiraRmd_80041AC8 escalation).** A same-bytes
respelling — a C form that emits byte-identical instructions to the current
form and differs only in its effect on GCC's internal analysis — is
sanctioned ONLY when ALL of the following hold:

1. **Mechanical proof of the original spelling class.** The target bytes are
   only producible from a spelling class OTHER than the current one, proven
   at the compiler-mechanism level (RTL dumps + GCC source reading, e.g. the
   MEM_IN_STRUCT_P `/s` flag pair controlling a sched.c `true_dependence`
   escape clause). "This spelling scores better" is NOT proof; the proof is
   "the original source provably did not write the current form."
2. **The committed form is plain, natural C** — something a 1998 programmer
   would write (a pointer local, a cast read, a differently-typed access).
   No dead code, no unused declarations, no asm, no rules.
3. **The most human-plausible representative is chosen.** The match cannot
   distinguish between members of the proven class; pick the variant that
   reads most naturally and annotate it with a comment citing this rule
   (so future agents don't "simplify" it back to the broken spelling).
4. **The respelling is exhaustively the last lever.** All conventional
   levers tried and measured negative; the cheat-reviewer adjudicated the
   candidate forms; the policy question was surfaced to the user (or this
   rule's criteria demonstrably apply 1:1).

This is the THIRD SOTN-evidence-shaped carve-out (after
[[do-while-zero-exception]] and [[legitimate-volatile-interrupt-touched]]),
and like them it does NOT lower the bar for the next proposed exception.
The key distinction from the forbidden coercion families: those ADD
semantically-empty constructs (dead stores, unused arrays, barriers,
alias renames); this rule covers CHOOSING AMONG equally-natural spellings
of a line the function genuinely needs, with proof the original used one
of them. SOTN ships far more contrived pure-C respellings (FAKE-annotated
variable reuse, mixed exit forms, duplicate reads) — this is within that
bar, with stronger evidence than SOTN typically has.

## What this does NOT sanction

- Same-bytes respellings WITHOUT the mechanism-level proof of the original
  spelling class (= ordinary forbidden coercion; the cheat-reviewer's
  default FAIL stands).
- Forms that are not natural C (the `s16[2]` array re-type with a never-used
  element FAILED this test even though it was in the proven class — a type
  change to a global that nothing else justifies is less faithful than a
  local spelling change).
- Permuter-found respellings vetted by nobody — the reviewer + (for novel
  classes) the user remain the gate. Reviewer verdict NEEDS_USER is the
  expected outcome for the first instance of a new proof shape.

## Confirmed case — InitHiraRmd_80041AC8 (text1a.c, 2026-06-10)

Floor-2 residual: target stores `sh a1,D_800A9A20` BEFORE reloading
`lh v0,8(a0)`; our `arg0[4]` reload was hoisted above the store by sched1.
Mechanism (verified in .combine/.sched dumps + sched.c~816 + expr.c~4570):
pointer-indexing (`INDIRECT_REF` of `PLUS_EXPR`) gets `MEM_IN_STRUCT_P=1`,
firing `true_dependence`'s in-struct escape clause → no dependence edge →
sched1 hoists the lh. Target's order requires `/s=0` on the reload ⇒ the
original 1998 source did not write `arg0[4]` for this read. Three sandbox-0
forms existed (load-side ptr-local, load-side byte-cast, store-side array
re-type); reviewer FAILed all three as same-bytes coercion; user sanctioned
the most natural (pointer-local `id_ptr = &arg0[4]; ... *id_ptr`).
Both regfix reorder rules retired; SHA1 == oracle; COMPLETED-C.

## Related

- [[no-new-park-categories]] — the cheats-by-any-spelling parent policy this
  rule narrowly amends; everything outside the 4-point criterion unchanged.
- [[do-while-zero-exception]] / [[legitimate-volatile-interrupt-touched]] —
  sibling narrow carve-outs; same structure (evidence bar + reviewer
  enforcement + user grant).
- [[review-discipline-before-commit]] — the reviewer remains the gate; for
  this family the expected verdict on a NEW instance is NEEDS_USER unless
  the 4-point criterion maps exactly.
- [[switch-break-shared-return-sched-hoist]] — the sibling sched1-hoist
  mechanism rule (different fix: there a structural form change closed it
  with no respelling needed — always check for that first).
