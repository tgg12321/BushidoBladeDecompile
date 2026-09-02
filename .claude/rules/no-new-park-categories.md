---
name: no-new-park-categories
paths: ["src/*.c", "engine/queue.py", "engine/cheats.py", ".claude/rules/*.md", "CLAUDE.md", "AGENTS.md"]
description: "User policy 2026-06-01: do NOT propose new auto-park / canonical-infrastructure categories that accept cheat-tolerant outcomes. The two existing carve-outs (jtbl-infra, GTE leaf wrappers) are NO-C-FORM exceptions, not generalizable patterns. Register-rotation walls, cross-jump merges, RA plateaus, prologue scheduling — these are ALL pure-C problems that the agent has not yet solved, not new categories to retire as 'infrastructure'."
metadata:
  type: rules
---

# Standing policy: no new cheat-tolerant park categories

> **Historical framing note (2026-08-30):** this rule predates the removal of the
> regfix/asmfix rule system (retired at zero rules; machinery deleted). Where the
> symptom text says a function "carries a rule", read it as "the honest build shows
> this diff shape vs target". The technique itself is unchanged.

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
- **Build-time assembly rewriting in ANY form.** The retired regfix/asmfix
  rule system (driven to zero rules 2026-08-25, machinery deleted
  2026-08-30) may never come back in any spelling: no rule/config files
  that transform compiler output, no new pipeline stages or
  sed/awk/script passes between cc1 and the linker, no Makefile or
  engine/pipeline edits that alter emitted bytes per-function, no
  prebuilt-.o or asm-file substitution for a function claimed as C.
  Bytes come from compiling the committed C (or an authorized
  canonical-asm body) — there is no third source. The Judge and the
  cheat-reviewer both treat reintroduction as an automatic FAIL.
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
  - **Clarification (owner ruling 2026-08-17, func_8001979C escalation).**
    This entry's SOTN backing is the *shape* shipped in `w_037.c`
    (`randy = basePoint.x; baseX = randy;`, "FAKE but makes register
    allocation work") — a once-written, once-read fresh local whose value
    is real and consumed. The trailing "to bias LUID" clause is BB2's own
    mechanism-class annotation from the 2026-06-02 census, NOT a scope
    limit: SOTN's acceptance was never conditioned on a GCC pass. A fresh
    named intermediate therefore qualifies under this entry **whatever GCC
    pass it acts through** (LUID bias, cse.c re-materialization, allocno
    priority), provided ALL of: (1) once-written — **relaxed from
    "once-written, once-read" by owner ruling 2026-08-31
    ([[ordinary-c-judge-decidable]]; evidence: the SOTN-master PSX
    `new_var_temp` class, docs/reference/sotn-construct-index.md:649 —
    NOTE the caveat carried in that rule's § Ruling 1: the index carries
    declaration lines only, so this is precedent for the existence of
    fresh RA-purposed locals in SOTN PSX master, not a line-for-line shape
    match; the owner ruled with that caveat presented): a fresh local
    holding a real, consumed value may be read any number of times.** Multi-WRITE carriers remain NOT this entry (the `y1` FAIL,
    decisions.md:1833, and the 2026-08-30 func_80045878 `c` FAIL stand);
    (2) real value — the intermediate holds a computation that appears in
    the target's own bytes and only relocates where the value is named;
    pure no-op copies stay with the dead-store family and its
    prerequisites; (3) byte-neutral — `build_insns == target_insns`, the
    compiler folds the copy; (4) fresh local, not a borrow —
    [[staged-value-reused-variable]] keeps its own bounds; (5) destination
    not live-pre-initialized (the `x/tx` FAIL, decisions.md:4251, stands);
    (6) standard prerequisites: dump-proven named mechanism, documented
    lever exhaustion, `/* FAKE: ... */` annotation, layer-1 + layer-2
    review. Does not relax any other frozen entry; licenses no extra
    *handles* to one object.
- **Per-word splat symbol → aggregate merge** (owner ruling 2026-08-17,
  func_8003B9D0 escalation; SOTN precedent: `include/game.h` `Vram`
  struct merges, PRs #1175 / bd612229 / 88344c03 — "standard, encouraged,
  ongoing" per [[sotn-prototype-struct-precedent-2026-08-10]]): two or
  more splat-invented `D_<addr>` scalars may be replaced by a single
  aggregate declaration. Prongs, ALL mandatory: (a) the object model is
  established by evidence independent of and predating the byte-chasing
  session — cross-TU stride indexing, base+offset addressing in the
  original binary, or a committed naming-census schema; (b) the merged
  declaration reflects that documented shape (a struct/record table where
  the evidence shows records; a flat array only where the evidence shows
  a flat array) — an index that encodes a record stride as a magic number
  does NOT qualify; (c) the merge is complete: every merged per-word
  symbol is removed from C and from the splat symbol config, leaving
  exactly one C handle per storage location; (d) spelled at the canonical
  declaration in the shared header, never TU-local, never a per-use
  pointer pun; (e) byte-neutrality verified for every other consumer,
  full `verify-oracle --rebuild`, layer-2 cheat-reviewer.
- **`do { ... } while (0);` wrap** (empty or non-empty body)
  ([[do-while-zero-exception]] / [[sotn-do-while-zero-research-2026-06-04]]):
  sanctioned as a pure-C match device for ANY codegen effect, including
  register allocation (owner ruling 2026-07-06, which SUPERSEDES the
  2026-06-04 reorg.c/LABEL_OUTSIDE_LOOP_P-only scoping; that scoping is
  abolished and must not be cited as a FAIL ground). Last-resort, mandatory
  `/* FAKE: ... */` annotation, documented lever-exhaustion, named GCC-pass
  mechanism; nested wraps need a single-level-insufficient justification.
  SOTN evidence: 18+ instances in master across `sprintf.c`, `5087C.c`,
  `c_004.c`, `w_045.c`, etc., with two PR-merge messages explicitly
  accepting it. User policy 2026-06-04: this is the ONE no-semantic-purpose
  wrapper sanctioned in BB2 source. Other syntactic equivalents
  (`for (i=0;i<1;i++)`, `while(1) { ...; break; }`, `if (1) { }`) are NOT
  sanctioned by this rule's existence — they have to clear the same
  SOTN-evidence bar independently.

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

- **Duplicated statement into arms** ([[duplicated-statement-into-arms]],
  second 2026-07-01 ruling, own evidence pass) — a REAL statement
  duplicated into 2+ control-flow arms instead of label-shared, incl.
  when cross-jump re-merges it byte-neutrally and the effect is a
  reg_n_refs priority lift. SOTN duplicates assignments across arms
  routinely (7-arm / 11-arm instances in doppleganger.c); redundant
  match-annotated duplicate stores in dra/42398.c + menu.c; MGS
  "no match if we don't". Prereqs: byte-neutrality verified +
  exhaustion + FAKE annotation + layer-1/2 review.

- **Written-never-read local array** ([[dead-vars-local-array]] carve-out,
  third 2026-07-01 ruling, own evidence pass) — sanctioned ONLY when the
  target bytes contain the corresponding dead stores (oracle-enforced),
  written (not merely declared), exhaustion-documented, FAKE-annotated,
  dual-reviewed. SOTN evidence: `u8 sp70[4]` written 4×/read 0× in two
  matched dra-core functions (62DEC.c), `s16 z[5]` ×2, annotated
  `volatile u32 pad[4]; // FAKE`. The `(void)&local` form remains
  forbidden.
  - **Re-scope (owner ruling 2026-08-17, func_8001E404 escalation).**
    Direct inspection of SOTN master (`db41b28`) exhibits
    `volatile u32 pad[4]; // FAKE` (`src/st/sel/stream.c:80`) and
    `volatile u32 pad; // !FAKE:` (`src/st/sel/2C048.c:560`) — both
    declared first, never written, never read, in fully matched PSX code.
    The 2026-07-01 carve-out mis-scoped itself against the very exemplar
    it cites: the cited pad IS unwritten. Accordingly, an **unwritten
    leading local pad** — spelled `volatile`, FAKE-annotated, with
    documented lever exhaustion and layer-2 review — is sanctioned for
    frames whose residual is provably a single allocated-but-untouched
    leading region confirmed by frame-term forensics. Scope: applies to
    `func_8001E404`, `func_8001E6E4`, `func_8003CF84` ONLY; any further
    use requires a fresh owner ruling.
    - **Extension (owner ruling 2026-08-18, func_8003CF84 only):** that
      function's frame census shows a SECOND, trailing 8-byte
      allocated-but-untouched object above `vec` (leading 16 + trailing
      8; 14 honest spellings and all three [[phantom-slot-frame-lever]]
      producers measured inert — memory/wip/func_8003CF84/notes.md).
      Its `volatile u32 pad2[2];` is sanctioned under the same prongs
      (volatile, FAKE-annotated, exhaustion-documented, engine
      allowlist row). Trailing position is granted for THIS function
      only; it does not generalize. Prerequisite (mechanical): the
    volatile respelling must be oracle-verified, and the sanctioned pads
    are allowlisted in the engine's volatile-cheat detector so honest
    floors read true.
- **Fabricated dead call site ("reconstructed compiled-out call site") —
  REFUSED (owner ruling 2026-08-17, func_8001E404 escalation).** An
  `if (0) { call(...); }` (or any never-executed call) added to move the
  outgoing-args frame partition is NOT a sanctioned family and may not be
  re-proposed in any spelling: zero SOTN-master precedent (census run
  2026-08-17 against `db41b28`: no `if (0)` block in matched PSX code
  contains a call), it fabricates a callee symbol against
  [[names-require-evidence]], and it is a parameterized general-purpose
  frame lever (argument count selects the frame delta).

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

**2026-08-18 additions (owner ruling b, evidence: the 2026-08-18 SOTN
family surveys — full-tree master-branch sweeps at commit 8bd7c777,
methodology per [[sotn-prototype-struct-precedent-2026-08-10]]; owner
directive verbatim: "Go ahead with all your recommendations on these").**
Each is a LAST-RESORT sanction with the standard prerequisites
(documented lever-exhaustion, named GCC-pass mechanism, mandatory
annotation, layer-1+2 review):

- **Compound-address duplication across call arg-lists** (F3 survey,
  ESTABLISHED): writing a compound address expression (`&base[i] + k`
  class) character-for-character at multiple call argument positions
  instead of binding it to a pointer local. Routine SOTN style in
  fully-matched files (`src/dra/5087C.c:559` "ugly casts"; gte_ldrgb
  dual-position exhibit; up to 14 repetitions of one expression in one
  function). Prerequisites: the duplicated expression is real (value
  consumed at each site); annotation at the duplication site.
- **Semantically-null fabricated statement pairs (cancellation-pair /
  redundant-condition class)** (F6 survey, ESTABLISHED): an adjacent
  same-variable increment/decrement pair (`i++; i--;`) or a fabricated
  redundant condition / empty-if inserted solely for codegen. Direct
  exhibit `src/saturn/game_3b.c:1450` (SH-2 matched target — owner
  accepts the cross-ISA caveat); MIPS class-siblings
  `src/dra/5D5BC.c:770` (`if (!i) { }`, "permuter found it") and
  `src/st/st0/cutscene.c:203` (`if (prim && prim)`). Sanctions ONLY the
  exact cancellation pair and empty-condition shapes — the `+= 2 / -= 1`
  respelling FAILed by the Judge (decisions.md:1833 lineage) remains
  banned. Prerequisites: `!FAKE`-style annotation; exhaustion ledger.
- **Unconditional-common-store duplication into both branch arms** (F7
  survey, ESTABLISHED as construct): duplicating common-tail stores into
  both if/else arms where cross-jump may or may not re-merge them —
  sanctioned at the CONSTRUCT level regardless of which GCC pass the
  duplication feeds (consistent with [[duplicated-statement-into-arms]]'s
  own note that merge behavior is invisible to the author; SOTN ships 25
  fully-identical-arm if/else constructs and 1,275 identical-store-in-
  both-arms sites in matched code). Prerequisites: annotation; the
  stores' values real and required.
- **Phantom-frame-slot volatile pad local** (off-brief survey exhibit
  `src/st/sel/2C048.c:564` `volatile u32 pad; // !FAKE:` in an
  INCLUDE_ASM=0 file): an unused `volatile` pad local declared solely to
  reserve target's untouched stack bytes. Supersedes the per-function
  2026-08-17/18 leading/trailing-pad carve-outs with a general family.
  FORM CONSTRAINT: applications use the ARRAY form
  (`volatile u32 pad[N];` — the engine allowlist
  `engine/volatile_cheats.py _SANCTIONED_UNWRITTEN_PADS` requires it and
  a per-function row), first-decl position (trailing does not generalize
  per the 2026-08-17 func_8003CF84 ruling), no `(void)pad;` shims.
  Prerequisites: `// !FAKE` annotation; ledger frame-forensics showing
  the target slot genuinely untouched ([[phantom-slot-frame-lever]]
  procedure); honest producers measured inert first.

Surveyed and NOT extended (2026-08-18, refusals stand): F1
constant→local→local staging chain (WEAK — genus shipped, species not),
F2 signedness-split dual read (WEAK), F4 cross-symbol arithmetic idiom
(ABSENT — SOTN's norm is the struct merge), F5 union-constructor CLOBBER
(ABSENT).

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
   /[[canonical-asm-retirement]], via the pipeline grant path —
   [[judge-sole-gate]], owner ruling 2026-08-18).
3. **The fallback for a function without those signals is to keep it
   parked**, in the existing "INCOMPLETE — search continues" state. The
   queue is a worklist of unfinished work; it can hold parked items
   indefinitely without inventing a new "done" category for them.

Escalations to a waiting owner no longer exist (owner rulings 2026-08-18
[[judge-sole-gate]] and 2026-08-31 [[ordinary-c-judge-decidable]]). The
exhaustive dispositions are: (a) more search, (b) the pipeline
canonical-asm grant path IF the hand-coded signals support it, (c) the
silent `foreclosed` queue state (proof-of-foreclosure recorded to
decisions.md, skipped by `queue next`, re-activated on new evidence or
owner unpark). There is no (d) "new infrastructure carve-out", and a
proposed frozen-list extension is a clean FAIL(CONSTRUCT) logged to
`docs/grind/borderline.md` — never granted in-pipeline, never filed as a
question to the owner.

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
