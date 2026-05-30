---
name: no-compiler-divergence
paths: ["src/*.c", "tools/gcc-2.7.2/**", "tools/maspsx/**", "regfix.txt", "asmfix.txt"]
description: "HARD RULE: do NOT patch cc1, do NOT switch to cc1psx, do NOT consider 'maybe the compiler is the variable'. The compiler is FROZEN. Every unmatched function closes in pure C source structure, full stop."
metadata:
  type: rule
---

# The compiler is frozen — pure-C source structure is the ONLY lever

User policy, made explicit 2026-05-30 after a worker spiraled toward
"the residual gap requires a compiler-level fix" on cpu_side_move_dir_4:

> Compiler patches and considering compiler divergence is not allowed.
> We aren't going to be forking our compiler or going down that rabbit hole.
> Keep pushing for new and novel solutions in C.

This is a HARD RULE. Not a heuristic, not a tier, not a last-resort gate.
There is NO compiler-modification path on this project.

## What is forbidden

1. **Patching `tools/gcc-2.7.2/`** (cc1, the linker, anything in the toolchain).
   Even surgical patches to `reorg.c` / `global.c` / `sched.c` / `combine.c` /
   `flow.c` are off the table. Read the GCC source to UNDERSTAND what shape the
   C must have — never to change what GCC does.
2. **Patching `tools/maspsx/`** beyond bug-fix scope (the per-function gates
   in `maspsx_label_nop_funcs.txt` are the established mechanism; new global
   behaviour changes require user policy sign-off — see
   [[maspsx-label-nop-gate]]).
3. **Switching the build to cc1psx**, or per-function cc1psx opt-in. cc1psx is
   diagnostic-only ([[cc1psx-calibration-only]] / [[cc1psx-calibration]]).
4. **Forking** anything in the toolchain (cc1, ld, as, maspsx) into a new
   variant, even "just for this one function."
5. **Asserting "the toolchain is the variable"** as a reason to escalate, park,
   or stop. The toolchain is fixed; therefore the variable is the C.

## What this means for stuck functions

When a function plateaus and the evidence chain ends at "GCC's
`reorg.c`/`combine.c`/`global.c` does X / does not do Y" — that is
**informational about the C structure you must find**, NOT a license to call
the function unmatched. The matching C exists ([[difficult-is-not-impossible]]);
the compiler's behaviour is just the shape it must fit. Keep grinding C
structures.

Specifically:
- Instrumented cc1 dumps (`tmp/gccdbg/cc1` with `BB2_ALLOC_DEBUG` etc.) are
  for **understanding** what shape GCC wants. Once you understand the shape,
  the work is finding C that produces it. **The dump is the map, not the
  destination.**
- "I traced the dataflow and the eager fill is rejected because the CALL_INSN
  has `(use a0)`" is a great diagnosis. The follow-up is "what C structure
  makes a0 dead at that point?" — NOT "GCC needs a patch."
- Same for combine fold suppression, allocno priority tiebreakers,
  cross-jump merge defeat, USE-INSN-wrapper pollution. All are **GCC's
  internal state machine that the C source feeds.** The fix is always upstream
  of GCC.

## How to surface a genuine wall

If — after exhaustively searching C structures with permuter + every lever
in `[[register-alloc-pure-c]]` / `[[cross-jump-store-tail-merge]]` /
`[[shared-end-label]]` / `[[defeat-licm-hoist-var-reuse]]` / etc. — a
function genuinely won't close, the only valid escalations are:

1. **Canonical-asm authorization** ([[canonical-asm-retirement]]) — user
   sign-off for tier-1 hand-coded asm for a construct with no C form. NOT
   for "GCC won't produce this." For genuinely no-C-form constructs only.
2. **Project-wide architecture decisions** — e.g. rodata reorder
   ([[jtbl-rodata-split-infrastructure]]), per-file flag list extension
   ([[compiler-flags-canonical]]). These are user policy.
3. **Out-of-budget** — token cap reached; surface what was tried and the
   exact partial state for the next session to resume from.

NOT valid escalations:
- "compiler-level fix needed" / "reorg.c patch would solve it"
- "cc1psx might produce this" (it's been measured; it doesn't —
  [[cc1psx-calibration-only]])
- "maybe a future toolchain improvement" (we ship on this toolchain forever)

## The corollary — research, try novel things, build tools

User directive same session: "Research, try new things, build new tools if
you think they will help, but keep grinding through."

So the affirmative side of this rule:
- **Read more of the GCC source.** `tools/gcc-2.7.2/{combine,jump,sched,reorg,global,flow,cse,loop}.c` is the spec for what C produces. Time spent
  reading them is well-spent — it surfaces lever shapes you haven't tried.
- **Read the m2c output.** It reconstructs the original C from the asm. The
  shapes it produces are evidence about the original source structure.
- **Read the matched siblings.** Functions in the same file that DID match
  show GCC's actual behaviour on adjacent-style C. Diff their `.greg`
  dumps against the unmatched one.
- **Write new tools.** If you need a way to compute the symbolic-fold
  reachability of an expression, write `tmp/fold_probe.py`. If you need
  to sweep a structural variant across 50 forms, write `tmp/sweep.py`.
  Reusable tools earn their keep across many functions.
- **Permuter in directed mode.** `PERM_*` macros are the un-explored permuter
  surface (random mode is exhausted on these). See
  `tools/decomp-permuter/README.md`.
- **Cross-reference the Kengo source-file naming** (the PS2 successor, partial
  source available) for hints about how the original C structured these
  functions. Function names alone often reveal the intent.

## Status of the long-parked cluster (saEft00Add / cpu_side_move_dir_4 / marionation_Exec)

The previous worker's "structural ceiling" claims for these three (~14 sessions
of evidence) are evidence that the explored C-source space has been exhausted —
NOT evidence that they need compiler patches. The matching C exists; it is
just outside what's been searched. The remaining work is finding it.

Per the user directive: "Keep pushing for new and novel solutions in C. No one
said this would be easy."

## Related
- [[difficult-is-not-impossible]] — the cardinal rule this enforces
- [[compiler-flags-canonical]] — flags are also frozen (same principle)
- [[compiler-patch-low-roi]] — measured 0/16 ROI; this rule supersedes it as
  POLICY (the ROI data is moot when patches are forbidden)
- [[cc1psx-calibration-only]] — cc1psx is diagnostic, never a path forward
- [[canonical-asm-retirement]] — the only valid "no-C-form" escape, requires
  user sign-off
