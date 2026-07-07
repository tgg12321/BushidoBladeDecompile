---
name: do-while-zero-exception
paths: [".claude/rules/do-while-zero-exception.md"]
# on-demand only: surfaced via codegen-technique-index (auto-loads on src/*.c)
description: "NARROW SANCTIONED EXCEPTION (user policy 2026-06-04): `do { ... } while (0);` is the ONE no-semantic-purpose wrapper construct allowed in BB2 source. Allowed ONLY because SOTN ships it openly with `// FAKE` annotation. Strictly last-resort: requires demonstrated lever-exhaustion. Does NOT relax the 'cheats by any spelling' policy for ANY other construct."
metadata:
  type: rule
---

# The do-while-zero narrow exception

**This rule documents the ONE exception to the "cheats by any spelling are
forbidden" policy ([[no-new-park-categories]]). It is deliberately scoped to
exactly one construct, in exactly one set of circumstances, and exists ONLY
because SOTN ships it openly. It is NOT a precedent for any other
codegen-coercion device.**

## What is sanctioned

A `do { ... } while (0);` wrap around real code, when:

1. The body has real semantic content (one or more real statements). The
   surrounding `do { } while (0);` itself has no observable behavior — it
   runs exactly once unconditionally — but the body it wraps is real code.
2. The wrap is annotated with `/* FAKE: <one-line reason> */` or `// FAKE`
   so future agents reading the source can identify it as a matching
   device, not an intent-bearing construct.
3. The wrap is the close-out lever for a function that has been driven to
   genuine lever-exhaustion through the standard playbook
   ([[register-alloc-pure-c]], [[difficult-is-not-impossible]],
   permuter, instrumented cc1 diagnostics, the SOTN-accepted technique
   list, etc.).

Both empty (`do { } while (0);`) and non-empty bodies are sanctioned — SOTN
ships both. The 2026-06-04 research memo
([[sotn-do-while-zero-research-2026-06-04]]) cites 18+ instances across SOTN
master + two PR-merge messages explicitly accepting the construct.

## Why this specific exception (and only this)

Three properties make do-while-zero qualitatively different from other
no-semantic-purpose constructs:

1. **SOTN ships it openly.** Not hidden, not renamed, not disguised — they
   literally write `do { } while (0); // FAKE` in their master branch and
   merge it through code review with comments like *"Not sure about the
   `do {...} while (0)` parts, but I saw that it is in other parts of the
   repo as well."* The project benchmarks against SOTN, and this is
   genuinely the SOTN bar for unmatchable-otherwise functions.
2. **It is transparent.** The construct is recognizable on sight, easy to
   grep for, and conventionally `// FAKE`-annotated. A future reader knows
   immediately "this is a matching device, not program logic." Contrast
   with cheats that disguise themselves as program logic (variable named
   `pad`, dead store written to look intentional, etc.).
3. **No legitimate alternative exists in our toolchain.** When a function
   triggers GCC's reorg.c branch-inversion peephole on a goto-loop and the
   matching close requires the loop-note that only a `do`/`while`/`for`
   construct provides, real loop constructs trigger LICM hoists of literal
   constants into callee-saves (+4 insn regression). The degenerate
   `do { } while (0);` creates the loop note WITHOUT triggering LICM —
   GCC folds the trivial loop before LICM runs. This was empirically
   verified for `cpu_check_same_dir_timer` against 23+ structural variants
   + cc1psx parity test + 10k+ permuter iterations.

## What is NOT sanctioned

This exception covers `do { } while (0);` and nothing else. It does NOT
sanction:

- Other no-semantic-purpose wrappers (`for (i=0;i<1;i++) { }`, `while(1)
  { ...; break; }`, `if (1) { }`, etc.) — these would be syntactic
  workarounds for the same exception, and per [[no-new-park-categories]]
  the catalog is open: new spellings of forbidden intent are also
  forbidden. SOTN does NOT ship these forms; only `do { } while (0);`.
- The `do { } while (0);` wrap when a real lever exists. If you find a
  pure-C structural change that matches the function, that's the right
  answer. The wrap is the last-resort tool.
- The `do { } while (0);` wrap to defeat OTHER codegen passes. The
  sanctioned use case is specifically for the LABEL_OUTSIDE_LOOP_P /
  reorg.c interaction described above. Using it to bend other GCC passes
  reopens the slippery-slope this rule exists to close.
- Implicit relaxation of the "cheats by any spelling" policy.
  [[no-new-park-categories]]'s overall posture is unchanged. The next
  borderline construct that someone proposes does NOT get the SOTN-ships-
  it benefit of the doubt by default — it gets the FAIL default like
  everything else, unless and until specific SOTN-master-branch evidence
  surfaces for that specific construct (mirroring the
  2026-06-02 borderline-research methodology, not blanket sanction).

## How to use the exception when it applies

If you genuinely believe the do-while-zero exception applies to a function
you're working on:

1. **Demonstrate lever exhaustion first.** Document in the WIP entry
   what alternative levers you tried (real loops, structural rewrites,
   permuter from candidate.c, instrumented cc1 dumps, etc.) and why each
   failed. The cheat-reviewer is empowered to refuse the exception if
   the worker hasn't shown the exhaustion.
2. **Annotate with `/* FAKE: ... */` or `// FAKE`** in the source.
   Include a one-line reason explaining what GCC-internal mechanism the
   wrap is defeating (e.g., "FAKE: do { } while (0); — emits
   NOTE_INSN_LOOP_BEG which suppresses reorg.c's invert-jump peephole on
   NE conditions, preserving target's branch sense" — see
   `src/code6cac_b.c::cpu_check_same_dir_timer` for the canonical
   example).
3. **Invoke the cheat-reviewer.** The reviewer's family check
   ([[review-discipline-before-commit]]) allows do-while-zero specifically
   under this exception, but ONLY when the worker has demonstrated the
   prerequisites above. The reviewer will FAIL the form if it's the
   first lever tried, OR if any alternative pure-C lever would have
   worked, OR if the wrap is being used to bend a non-LABEL_OUTSIDE_LOOP_P
   codegen pass.
4. **Reference this rule in the commit message** so the policy basis is
   linked from the git history.

## Confirmed application

**`cpu_check_same_dir_timer`** (commit `cf3e6ce7`, 2026-06-04) — the
canonical use of this exception. The function is a byte-stream
interpreter; the natural `do { ... } while (op != 0);` form triggers LICM
hoists of `0xFF` and `1` into callee-save registers, regressing by 4
instructions. The `do { ... } while (0);` wrap preserves the loop note
needed for `reorg.c`'s LABEL_OUTSIDE_LOOP_P check WITHOUT triggering
LICM (degenerate loop is folded as trivial before LICM runs). Source
includes the `/* FAKE: ... */` annotation pointing at the research memo.
This commit is the reference for what a sanctioned use looks like.

## Declined extension — do-while(0) as register-allocation weighting (2026-07-06)

**Owner ruling 2026-07-06: NOT SANCTIONED.** A marionation_Exec lineage
(sessions 8-9, work/marion) reached masked 4 by scattering do-while(0) wraps
at 4 sites to shift local-alloc/global allocno priorities (flow's loop-depth
ref weighting) — using the sanctioned construct to bend REGISTER ALLOCATION
rather than the reorg.c label-note interaction sanctioned above. A fresh
layer-1 reviewer (rev-vt31) FAILed it as register-web gaming by a new
spelling (same forbidden intent as the 2026-07-05 cross-symbol-FAKE
DO-NOT-SANCTION ruling), and the owner confirmed: do not sanction.
Session-9's "SOTN uses do-while(0) mechanism-agnostically" research was
considered and declined — the mechanism scoping in this rule stands.
Quantified: removing the wraps moved marionation_Exec from masked 4 to
masked 30 (the wraps carried the entire delta). The FAILed form is banked at
`memory/wip/marionation_Exec/rejected/do-while0-ra-weighting-masked4.c`.
This also applies to the twin `cpu_side_move_dir_4` and any other function:
wraps whose measured effect is allocno-priority movement are FAIL by
default, whatever the annotation says.

## Adding future exceptions (if any)

This rule is the SECOND time the project has added a narrow exception to
"cheats by any spelling" based on SOTN-master-branch evidence (the first
was the 2026-06-02 borderline-research audit that added 6 specific
techniques). The methodology is the same and the bar should be at least
as high:

- **Direct SOTN master-branch citation** — multiple instances of the exact
  construct shipping in their tree, with `// FAKE` annotation or
  acknowledging comments where present.
- **Code-review evidence** — at least one merged commit message
  acknowledging the construct.
- **Independent justification** — explaining why this specific construct
  cannot be substituted with a legitimate lever in our toolchain.
- **User policy decision** — the exception is added by user policy, not
  agent inference. Agents may surface candidates with research; only the
  user grants the exception.

The fact that this rule exists does NOT lower the bar for the next
proposed exception. Each must be researched and sanctioned on its own
evidence.

## Related

- [[no-new-park-categories]] — the "cheats by any spelling" policy this
  rule narrowly amends.
- [[sotn-do-while-zero-research-2026-06-04]] — the SOTN master-branch
  research that grounds this exception.
- [[review-discipline-before-commit]] — the cheat-reviewer architecture;
  its family check honors this exception under the conditions above.
- [[difficult-is-not-impossible]] — keep grinding before reaching for the
  exception; this is the LAST tool, not the first.
- [[inline-asm-policy]] — the broader policy on what is and is not
  cheat-asm; this exception is scoped narrowly so it does not erode that
  policy.
