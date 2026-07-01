---
name: no-new-park-categories
paths: ["src/*.c", "engine/queue.py", "engine/cheats.py", ".claude/rules/*.md", "CLAUDE.md", "AGENTS.md"]
description: "User policy 2026-06-01: do NOT propose new auto-park / canonical-infrastructure categories that accept cheat-tolerant outcomes. The two existing carve-outs (jtbl-infra, GTE leaf wrappers) are NO-C-FORM exceptions, not generalizable patterns. Register-rotation walls, cross-jump merges, RA plateaus, prologue scheduling — these are ALL pure-C problems that the agent has not yet solved, not new categories to retire as 'infrastructure'."
metadata:
  type: rules
---

# Standing policy: no new cheat-tolerant park categories

> **User policy, codified 2026-06-01:** *"We won't be sanctioning any kind of
> new policy or methodology that is akin to a cheat. ... We can discuss more
> on what to do with register rotation infrastructure but it will never be
> something worse than the SOTN standard."*

When a worker / orchestrator surveys a cluster of parked functions and
notices a recurring wall pattern (e.g. "20+ regfix rules of register
renames", "cross-jump-store-tail-merge floor", "prologue save-order
4-insn reorder"), the temptation is to propose a **new auto-park category**
that retires those functions en masse with their cheats intact, by analogy
to the existing carve-outs:

- **jtbl-infra** ([[jtbl-rodata-split-infrastructure]]) — rodata-split jump
  tables. The GCC-emitted jtbl can't be at the address the function
  references because rodata link order puts the asm/data block before the
  C file's `.rodata`. There is **literally no pure-C form** that resolves
  this without a project-wide rodata reorder.
- **GTE leaf wrappers** ([[gte-wrapper-misroute-park]]) — pure cop2-op
  leaves (avsz3/avsz4 + mtc2/mfc2). The cop2 ops have **no C analog**.
  There is **literally no pure-C form**.

Both carve-outs share the property: **no C input to the same compiler can
produce the target bytes**, by construction (the GCC emitter or linker
literally cannot reach that state from any C source). They are not
"this function is hard to match", they are "no C exists for this".

## What this policy forbids

**Do NOT** propose, document, suggest, or implement any of the following as
project-accepted auto-park / canonical-infrastructure categories:

- **Register-rotation infrastructure.** Functions plateaued behind N-cycle
  register-allocation tiebreaker ties (cpu_side_move_dir_4 /
  marionation_Exec / saEft00Add and the func_8007B***/8007C*** cluster
  members in the same shape) — these are pure-C-reachable, the lever just
  hasn't been found. See [[difficult-is-not-impossible]] and the
  ALLOCDBG-instrumented sessions in [[register-alloc-pure-c]] for the
  proof-of-concept that the levers DO exist for these allocations; the
  remaining gap is more search, not a new category.
- **Cross-jump merge walls.** Functions plateaued behind GCC's `jump2`
  `find_cross_jump` block-suffix merge ([[cross-jump-store-tail-merge]],
  [[cross-jump-call-merge]]). These have known C levers (arg-count for
  CALL suffixes, mixed exit forms for STORE suffixes); when the levers
  don't close a specific function, the answer is more permuter / more
  derivation, not a new category.
- **Prologue save-order infrastructure.** Functions where cc1's
  `save_restore_insns` (mips.c) emits the prologue saves in the order
  GP_REG_LAST → FIRST while target wants the opposite (the
  `func_8007C2A0`/`C4B8` twin wall). This is a register-allocation tie at
  the prologue surface; same category as the above — a C lever exists or
  the function is canonical-asm-authorizable per [[hand-coded-asm-recognition]]
  + user sign-off, but it is NOT a new infrastructure category.
- **Anything labeled "X infrastructure"** as a justification to retire N
  similar functions en masse with their cheats intact.
- **Speculative system-wide rodata reorders to force a SHA1 match.**
  Globally reordering `bb2.ld` rodata placement without evidence that
  the original source had that layout is a structural cheat — same
  category as regfix offset paperwork (it changes the bytes' attribution
  to make the build work, not because we discovered the original was
  that way). SOTN does NOT do this. Evidence-based source-file
  re-attribution (re-splitting splat-output .c files because evidence
  shows the original had different TU boundaries, with `bb2.ld` updated
  to reflect the discovered layout) IS legitimate and is the SOTN
  workflow — but it requires evidence (function ↔ data adjacency,
  single-owner cross-references, byte-pattern signatures, comparable
  resolved siblings), not the bare desire to retire a parked function.
  See [[jtbl-rodata-split-infrastructure]] for the canonical case +
  the evidence checklist.

## Cheats by any spelling — the standing posture (user policy 2026-06-01)

> *"If it's a cheat, it will not be accepted. Full stop. Ideally I'd
> like agents to not even waste time attempting these kinds of
> approaches in the first place even out of curiosity."*

The detectors that have been wired into `engine/volatile_cheats.py`
catch the LITERAL forms of forbidden patterns: `s32 buf[N];` unused
arrays, `(void)&local;` address-coerced scalars, `arg0 = 0;` dead
self-assignments of parameters, `if (cond) { v = x; } ...; v = x;`
dead conditional stores, register-asm pins, hardcoded-`$N` `__asm__`,
volatile coercion casts, alias renames, macro-hidden `__asm__`,
lost-codegen `insert_after` regfix, manufactured dead-branch
scheduling insertions, `negu/move/addu/lui/nop` general-purpose
opcode inline asm.

**The detectors are a backstop, not the standard.** The standard is
THE INTENT, not the syntactic form. If you find yourself drafting any
code construct whose ONLY purpose is to change GCC's analysis (without
that purpose appearing in the emitted output), you are drafting a
cheat — regardless of whether the existing detector recognises the
specific syntax. The catalog is a partial enumeration of an open class.

Concrete signals that a code construct is a cheat-by-spelling:

- **No semantic purpose.** A human programmer reading the function
  would NOT write the construct because the function's stated behaviour
  doesn't require it. Naming patterns like `pad`, `buf`, `pre_pad`,
  `dummy`, `spill`, `slack`, `_unused`, `_tmp` announce coercion intent.
- **Dead in the emitted output.** GCC's DCE removes it from the
  generated code, but its EXISTENCE in source changed the codegen
  decisions upstream of DCE.
- **The construct is "necessary" only because the permuter found that
  removing it raises the sandbox score.** That's evidence of cheating,
  not evidence of correctness.
- **You can describe what it does without referencing GCC's allocator,
  scheduler, or DCE.** If your justification is "this defeats CSE",
  "this changes the allocno priority", "this makes the SLL emit before
  the OR" — and the construct itself has no observable behaviour —
  you're describing a coercion, not a piece of program logic.

## Auto-search tools (permuter, etc.) — output is PROPOSALS, not winners

The permuter, directed-PERM macros, brute-force structural sweepers,
and any other auto-search tool mutate C source looking for byte
matches. **These tools cannot judge whether a closing form is a
cheat.** They report `sandbox == 0` and `SHA1 == oracle` for any form
that produces target bytes — including forms that match the cheat
catalog above.

**Worker discipline.** When an auto-search tool returns a "closing
form", you MUST vet it against the cheat catalog BEFORE proposing it
to the user. The vetting is your job; do not surface forms that you
can identify as cheats yourself in the hope that the user will sanction
them. Surfacing a cheat-form proposal wastes the user's review
budget and erodes the policy.

Vetting checklist:
1. Does the form contain any construct from the catalog above
   (directly or by analogy)?
2. Does the form contain any code with no semantic purpose (dead
   stores, unused declarations, address-coercions, padding, named-for-
   role variables)?
3. Would a human programmer naturally write this code from a
   specification of the function's behaviour?
4. Does the closing form's justification reference GCC internals
   instead of program logic?

If ANY answer suggests the form is a cheat, reject it yourself.
Document the find in a memory note as "permuter found cheat-form X,
rejected per policy" — that's useful evidence about the closing-form
space. Then keep searching for a legitimate lever.

**Confirmed example (2026-06-01, `func_8007B844`).** Directed permuter
~36k iters returned `u32 *p; if (debug) {...; p = ot;} ...; p = ot;` as
a sandbox-0 closing form retiring 6 rules. The inner `p = ot;` is a
dead conditional store — same intent as the forbidden `arg0 = 0;`
Lever D, just spelled with a local and wrapped in `if`. User policy:
forbidden by any spelling. Detector
(`engine/volatile_cheats.find_dead_conditional_stores`) added to
catch the variant; the agent's correct move was to RECOGNIZE the find
as a cheat AND NOT surface it. (The agent did surface it; this entry
exists so future agents see the correct posture.)

## SOTN-accepted techniques (resolved 2026-06-02 borderline-rule research)

The following techniques were classified BORDERLINE by the 2026-06-02
techniques audit and subsequently resolved as **ALLOWED** based on direct
SOTN master-branch evidence ([[sotn-borderline-research-2026-06-02]]):

- **Variable reuse for codegen control** ([[defeat-licm-hoist-var-reuse]]):
  reusing one C variable for two unrelated values to influence
  loop-invariant detection or RA. SOTN ships `idxSub = idxSub;` and
  `randy = basePoint.x; baseX = randy;` with "FAKE but makes register
  allocation work" comments.
- **Opaque arithmetic variables** ([[loop-rotation-two-shift]]):
  `s32 one = 1;` to prevent compiler bit-test transforms. SOTN's
  official wiki endorses `(Random() & 3) + 1 - 1;` as the canonical
  shape.
- **Sub-word param reads** ([[narrow-stack-param-subword-offset]]):
  `*(u16 *)&local` cast to read a specific half-word. Standard C usage
  in SOTN.
- **Mixed exit forms** ([[cross-jump-store-tail-merge]]): deliberately
  mix `goto endK` with inline `return` to defeat `find_cross_jump`. SOTN
  ships this verbatim in `SsVabOpenHeadWithMode`
  (`src/main/psxsdk/libsnd/vs_vh.c`).
- **Duplicate-read into branch arms** ([[split-read-defeats-hoist]] #1+#2):
  pin offset computations inside their branch via duplication. SOTN
  ships `color_fake = *palette;` repeated rebinds (`src/dra/42398.c`).
- **Named-intermediate declaration order** ([[narrow-byte-args-packed-call]]
  hi/lo sub-trick): declare a sub-expression as a separately-named
  local to bias LUID. SOTN's `randy` chain in `src/weapon/w_037.c` is
  the same mechanism.
- **`do { ... } while (0);` wrap** (empty or non-empty body)
  ([[do-while-zero-exception]] / [[sotn-do-while-zero-research-2026-06-04]]):
  emits NOTE_INSN_LOOP_BEG which sets LABEL_OUTSIDE_LOOP_P on outside-loop
  labels, suppressing reorg.c's `relax_delay_slots` invert-jump peephole
  for NE conditions. **Narrowly sanctioned, last-resort only.** Annotate
  with `/* FAKE: ... */` or `// FAKE`. SOTN evidence: 18+ instances in
  master across `sprintf.c`, `5087C.c`, `c_004.c`, `w_045.c`, etc., with
  two PR-merge messages explicitly accepting it. **The dedicated rule
  [[do-while-zero-exception]] enumerates the strict prerequisites
  (demonstrated lever-exhaustion, applies only to the LABEL_OUTSIDE_LOOP_P
  / reorg.c interaction, NOT a precedent for other wrappers).** User policy
  2026-06-04: this is the ONE no-semantic-purpose wrapper sanctioned in
  BB2 source. Other syntactic equivalents (`for (i=0;i<1;i++)`,
  `while(1) { ...; break; }`, `if (1) { }`) are NOT sanctioned by this
  rule's existence — they have to clear the same SOTN-evidence bar
  independently.

The `cheat-reviewer` agent treats these patterns as ALLOWED — the
"family check" (test #5 of its 6-test checklist) no longer flags them.

**2026-07-01 additions (owner rulings, evidence:
[[sotn-family-research-2026-07-01]] — full-tree SOTN census + oot/
papermario/MGS/esa/VS corroboration).** Each is a LAST-RESORT sanction
with strict prerequisites (documented lever-exhaustion, named GCC-pass
mechanism, mandatory `/* FAKE */` annotation, layer-1+2 review) — read
the rule file BEFORE using:

- **Dead stores / self-assigns to locals+params**
  ([[dead-store-fake-exception]]) — SOTN `dest = val1; // fake`,
  `idxSub = idxSub;`; oot `rtile = rtile; // Fake match?`. Supersedes
  the Lever-D blanket ban (whose "SOTN's bar rejects them" rationale
  the census disproved). Register pins remain forbidden; a dead store
  paired with a pin still fails on the pin. The `func_8007B844`
  confirmed-example below predates this ruling: that closing form's
  dead conditional store would TODAY be reviewable under the carve-out's
  prerequisites rather than auto-rejected.
- **Constant-holder / dead scalar locals**
  ([[named-local-fake-exception]]) — SOTN `s16 three = 3;`, `s32 zero
  = 0; // needed for PSP`, constant-holder named `fake`; `new_var` in
  9 committed files. Arrays / frame coercion remain forbidden.
- **C-level pointer aliases to globals**
  ([[pointer-alias-fake-exception]]) — SOTN `tilemap = &g_Tilemap; //
  n.b.! unused, required for PSP`, `fakeEntity = self; // !FAKE`,
  FakePrim family. `asm("Sym")` alias-RENAMES remain forbidden.
- **Type-level MMIO volatile** ([[mmio-volatile-type-level]]) —
  hardware I/O-register range (0x1F801000-0x1F802FFF) declarations are
  volatile as ordinary hardware semantics, all shapes incl. single-read
  probes (SOTN types register pointers volatile at declaration). NOT
  fake — no annotation; `extern volatile` spellings still tracked via
  the allowlist. Game-state globals keep the
  [[legitimate-volatile-interrupt-touched]] two-prong gate.

What the 2026-07-01 research explicitly does NOT support relaxing
(zero community precedent found): register-asm pins, hardcoded-`$N`
`__asm__` injection, regfix/asmfix-style build-time rewriting,
`asm("sym")` alias renames, redundant width casts (F2 — evidence
insufficient; those findings close by ordinary cleanup).

This resolution affects only these seven specific techniques. Other
forbidden families ([[dead-vars-local-array]], dead-conditional-store,
dead-param-assign, lost-codegen-insert, register-asm pins, scheduling
barriers, etc.) remain forbidden. **The "cheats by any spelling"
operating principle is unchanged for everything else.** Each future
proposed exception must clear its own SOTN-master-branch evidence bar
(mirroring the 2026-06-02 borderline-research methodology, not blanket
sanction). Agents must NOT generalize from any single sanctioned
exception to treat the broader category as relaxed.

## What the SOTN standard accepts

[[community-standard]] is the bar: pure C, or canonical-body asm for code
that was *originally* hand-written assembly (not just code GCC won't
emit in our fork). The two carve-outs are no-C-form exceptions; anything
else is a pure-C problem awaiting a C-source solution.

## What to do when you see the temptation

You are surveying the queue / standing escalations and notice 4 functions
with the same 20-rule register-rotation pattern. You are tempted to
suggest "register-rotation-infrastructure" as a new park category. **Stop.**
Per this policy:

1. **The standing answer for register-rotation walls is more search** —
   directed-PERM permuter, instrumented cc1 dumps (BB2_ALLOC_DEBUG,
   BB2_SCHED_DEBUG, BB2_PRIO_DEBUG; see [[register-alloc-pure-c]]),
   cross-reference matched siblings, m2c-reconstructed structure, novel C
   restructurings.
2. **The fallback for a function with strong hand-coded signals is
   canonical-asm authorization** (per [[hand-coded-asm-recognition]]
   /[[canonical-asm-retirement]], requires user judgment).
3. **The fallback for a function without those signals is to keep it
   parked**, in the existing "INCOMPLETE — search continues" state. The
   queue is a worklist of unfinished work; it can hold parked items
   indefinitely without inventing a new "done" category for them.

When surfacing escalations to the user, do NOT phrase options as "accept
X as a new infrastructure category." Phrase them as "(a) permuter budget,
(b) canonical-asm authorization IF the hand-coded signals support it,
(c) keep parked." Categories (a) and (b) and (c) are exhaustive; there
is no (d) "new infrastructure carve-out."

## Related

- [[community-standard]] — the SOTN bar this policy enforces
- [[completion-standard]] — the three function states (INCOMPLETE /
  COMPLETED-C / COMPLETED-INLINE-ASM-CANONICAL); no fourth state exists
- [[no-compiler-divergence]] — companion HARD RULE; the toolchain is
  frozen, so the variable is the C
- [[difficult-is-not-impossible]] — the cardinal rule this policy enforces
  in spirit: stuck = unfinished work, never proven-impossible
- [[jtbl-rodata-split-infrastructure]] — the jtbl carve-out (legitimate,
  no-C-form)
- [[gte-wrapper-misroute-park]] — the GTE leaf carve-out (legitimate,
  no-C-form)
- [[hand-coded-asm-recognition]] — the legitimate escape for
  hand-written-asm constructs
- [[register-alloc-pure-c]] — the pure-C lever playbook with instrumented-
  cc1 evidence that allocation walls ARE C-reachable
