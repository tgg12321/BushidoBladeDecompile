---
name: no-compiler-divergence
paths: ["src/*.c", "tools/gcc-2.7.2/**", "tools/maspsx/**"]
description: "HARD RULE: do NOT patch cc1, do NOT switch to cc1psx, do NOT consider 'maybe the compiler is the variable'. The compiler is FROZEN. Every unmatched function closes in pure C source structure, full stop."
metadata:
  type: rule
---

# The compiler is frozen — pure-C source structure is the ONLY lever

> **Historical framing note (2026-08-30):** this rule predates the removal of the
> regfix/asmfix rule system (retired at zero rules; machinery deleted). Where the
> symptom text says a function "carries a rule", read it as "the honest build shows
> this diff shape vs target". The technique itself is unchanged.

User policy, made explicit 2026-05-30 after a worker spiraled toward
"the residual gap requires a compiler-level fix" on cpu_side_move_dir_4:

> Compiler patches and considering compiler divergence is not allowed.
> We aren't going to be forking our compiler or going down that rabbit hole.
> Keep pushing for new and novel solutions in C.

This is a HARD RULE. Not a heuristic, not a tier, not a last-resort gate.
There is NO compiler-modification path on this project.

## Owner ruling 2026-09-26 (ninth batch, Q17) — a compiler patch is a cheat

Context (record: docs/grind/owner-rulings-2026-09-26.md, batch 9): the
orchestrator proposed a scratch-only study of a narrow cc1 fidelity patch for
func_8001A820, citing the adopted PLUS->IOR patch (9bc64b751) as precedent. The
owner asked "SOTN doesn't do compiler patches do they?". The orchestrator
answered that, to its knowledge, SOTN-style PS1 decomps use a rebuild of the
original compiler version patched only to build on modern hosts, and switch
compiler versions rather than patch output, and recommended withdrawing the
study. Owner (Trenton), verbatim: **"Withdraw that option, consider a compiler
patch a cheat. Update documentation, mark relevant funcs as cheated and add
them back to the queue"**.

**Rule text** (the author's narrowing, not the owner's words):
- **A compiler patch is a cheat.** Any modification to the build compiler
  (`tools/gcc-2.7.2/build/cc1`, built from the pinned upstream
  decompals/mips-gcc-2.7.2 @43d1cdb6) that changes the compiler's output is a
  cheat. A function whose match depends on such a modification is NOT
  COMPLETED: it is reverted to `INCLUDE_ASM` and returns to the queue.
- **One named host-build fix is kept, as the author's reading.** The
  2026-08-24 reorg.c `negate_rtx` declaration crash fix
  (`tools/cc1-reorg-negate-rtx-decl.patch`, owner ruling 262930f1b) stays in
  the build compiler. It is a host-build fix: without it six TUs crash on this
  host, and every TU that survives compiles identically with and without it
  (census 2026-09-26, `tmp/patchcensus/`); this matches the practice the
  orchestrator described to the owner, a rebuild of the original compiler with
  host fixes only. Keeping it is the author's reading of Q17, not an owner
  ruling, and stays open for the owner to overrule. The carve-out covers that
  one fix only. There is no general test by which an agent may admit a
  change to the compiler: any NEW host-build fix needs its own owner ruling.
- **Compile flags are not patches.** The canonical `CC_FLAGS` (including
  `-mel` and `-msoft-float`, [[compiler-flags-canonical]]) configure the
  unmodified compiler and are outside this ruling.
- **Diagnostic compilers are not the build compiler.** The instrumented
  diagnostic cc1 (`tools/gcc-2.7.2/cc1`, `BB2_*_DEBUG` hooks) and cc1psx stay
  diagnostic-only; no function lands on their output, as before.
- **Superseded.** The PLUS->IOR rulings below (owner, 2026-09-25, study and
  second-batch adoption, recorded in bcdc1648e) and the adoption itself
  (9bc64b751, `tools/cc1-plus-to-ior-narrow.patch`, which replaced
  `tools/cc1-no-plus-to-ior.patch`) are SUPERSEDED by this ruling. They stay
  below as history. The census (`tmp/patchcensus/`, 2026-09-26) found one
  output-changing patch in the build compiler, the narrow PLUS->IOR patch in
  `combine.c`, and one function depending on it: func_800174F4
  (`src/ings.c`), whose `addiu s1,v0,4` becomes `ori s1,v0,0x4` without it.
  func_80073C78 and `main` match under the unpatched compiler. Returning the
  build compiler to the pinned upstream plus the one kept host fix, and
  reverting
  func_800174F4, are separate changes after the commit that records this
  ruling.
- **Scope: the compiler only, by the orchestrator's stated scoping.** The
  owner's words address the compiler. After the ruling, the orchestrator told
  the owner, verbatim: "Today's assembler-shim changes (the `($12)` parser fix
  and the "declared, no value" list) are changes to maspsx, not the compiler.
  SOTN uses maspsx too. I'm treating those as outside this ruling." The owner
  has not replied to that point. So the maspsx gates ([[maspsx-gate-lists]])
  and the maspsx parser fix are outside Q17 as the orchestrator's stated
  scoping, NOT an owner ruling. It stays open for the owner to overrule, and
  those changes keep their own rulings meanwhile.
Record: docs/grind/decisions.md 2026-09-26 OWNER RULING — a compiler patch is a
cheat (Q17).

## What is forbidden

1. **Patching `tools/gcc-2.7.2/`** (cc1, the linker, anything in the toolchain).
   Even surgical patches to `reorg.c` / `global.c` / `sched.c` / `combine.c` /
   `flow.c` are off the table. Read the GCC source to UNDERSTAND what shape the
   C must have — never to change what GCC does.
2. **Patching `tools/maspsx/`** beyond bug-fix scope. The remaining
   per-function gates (`maspsx_prefill_label_funcs.txt`, `expand_lb_funcs.txt`,
   `expand_dest_funcs.txt`) are the established mechanism for assembler-fidelity
   gaps; new GLOBAL behaviour changes require owner policy sign-off. (The
   owner signed off on one new per-function gate on 2026-09-26, fourth batch:
   `maspsx_comm_syms.txt`, admitted only under its prongs in
   [[maspsx-gate-lists]].) The same batch authorized a bug-fix-scope
   parser fix: maspsx's load/store parser accepts an empty-offset `($REG)`
   operand, as Sony's ASPSX 2.34 does (calibration check, commit 4ef521cdd,
   `memory/grind/func_8002DE20/aspsx-paren-check/`), provided every object
   builds byte-identical with and without it (inline-asm-policy.md § "Per-function grant:
   func_8002DE20"). Bug-fix
   scope IS permitted and has been exercised: on 2026-09-14 the owner signed off
   on globalizing the `.L`-label load-delay arm after a missing `$at`/`$gp`
   expansion guard was found in it, which retired `maspsx_label_nop_funcs.txt`
   entirely — see [[maspsx-label-nop-gate]].
3. **Switching the build to cc1psx**, or per-function cc1psx opt-in. cc1psx is
   diagnostic-only ([[cc1psx-calibration-only]] / [[cc1psx-calibration]]).
4. **Forking** anything in the toolchain (cc1, ld, as, maspsx) into a new
   variant, even "just for this one function."
5. **Asserting "the toolchain is the variable"** as a reason to escalate, park,
   or stop. The toolchain is fixed; therefore the variable is the C.

> **SUPERSEDED by the owner ruling of 2026-09-26 (Q17) above; kept as
> history.**
>
> **Owner ruling 2026-09-25 — study only.** A scratch-only STUDY of narrowed
> `combine.c` PLUS->IOR variants is authorized. Its question is which condition
> separates the sites where the original kept `addu` from those where it
> emitted `ori`. Variants are built outside the repository and never
> installed. Nothing in `tools/gcc-2.7.2/`, the Makefile or the build inputs
> changes, and no function lands on a variant's output. ADOPTING any variant
> is NOT authorized and returns to the owner with evidence.
> `tools/cc1-no-plus-to-ior.patch` stays the build compiler, and the
> `|`-for-`+` spelling stays refused. Full terms: docs/ORACLE-COMPILER.md
> § "Owner ruling 2026-09-25". Items 1-5 above stand for everything outside
> that study.

> **SUPERSEDED by the owner ruling of 2026-09-26 (Q17) above; kept as
> history.**
>
> **Owner ruling 2026-09-25 (second batch) — adoption authorized, conditionally.**
> The study's narrowed PLUS->IOR condition may REPLACE
> `tools/cc1-no-plus-to-ior.patch`. First, a register-plus-register scan of the
> target binary must be run. If a compile-proven discriminating site selects
> `exprop`, `exprop` is adopted instead. If no site discriminates, narrow is
> adopted and `exprop` stays a live alternative, and a later discriminating
> site reopens the choice for the owner. If discriminating sites disagree, or a site matches neither candidate, nothing is adopted and the question returns to the owner. Adoption requires every step in the full terms, including:
> - re-record the manifest, including the 2026-08-24 crash fix;
> - fix the `--stock` self-check;
> - commit the patch under `tools/`;
> - rebuild the compiler from the recipe;
> - pass the oracle SHA1, `engine test` and `fixtures-verify`;
> - let the toolchain-fingerprint auto-return run.
>
> The adopted patch becomes the rule's only PLUS->IOR amendment. Items 1-5
> above stand for everything else, and the `|`-for-`+` spelling stays
> refused. Full terms: docs/ORACLE-COMPILER.md § "Owner ruling 2026-09-25
> (second batch)".

**Executed 2026-09-25 (SUPERSEDED 2026-09-26 by Q17 above; history).** No
scanned site discriminated, so narrow was adopted:
`tools/cc1-plus-to-ior-narrow.patch` replaced `tools/cc1-no-plus-to-ior.patch`,
and the crash fix is pinned as `tools/cc1-reorg-negate-rtx-decl.patch`.
`exprop` stays a live alternative. Record: docs/ORACLE-COMPILER.md § "Adoption
record (2026-09-25)".

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
   sign-off for canonical-body hand-coded asm for a construct with no C form. NOT
   for "GCC won't produce this." For genuinely no-C-form constructs only.
2. **Project-wide architecture decisions** — e.g. rodata reorder
   ([[jtbl-rodata-split-infrastructure]]), per-file flag list extension
   ([[compiler-flags-canonical]]). These are user policy. (One standing
   route exists: a per-file `-G8` TU by proof, owner ruling 2026-09-26,
   compiler-flags-canonical.md § "Per-file -G8 by proof".)
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
