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
  `s32 one = 1;` to prevent compiler bit-test transforms (not as a
  dummy array subscript or pointer offset: refused, owner ruling
  2026-09-27 Q22, [[named-local-fake-exception]]). SOTN's
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
    decisions.md:1731, and the 2026-08-30 func_80045878 `c` FAIL stand;
    a fresh local written more than once is admitted ONLY if it meets
    every prong of [[ordinary-c-judge-decidable]] Ruling 5 (2026-09-23),
    as amended for identical-text writes by its 2026-09-23 extension, or
    every prong of its Ruling 6 (2026-09-23), or, for vmNoiseOn's `temp`
    alone, every prong of its Ruling 8 (2026-09-24), or every prong of its
    Ruling 9 (2026-09-25, one meaning at several constant offsets) or of its
    Ruling 10 (2026-09-25, the verified original source's own variable,
    verbatim), or, when none of those admits it, every prong of its
    Ruling 11 (2026-09-26, a reused local proven necessary by allocator
    dumps, with an honest generic name and layer-2), or, under owner ruling
    Q51 (2026-09-30), a verified SOTN reuse citation per § Owner ruling
    2026-09-30 — SOTN precedent suffices, with Q53; whichever ruling
    applies governs that variable exclusively: the reused variable itself may
    not also claim this entry or [[staged-value-reused-variable]]; other
    locals in the same body, including a Ruling 5 1(b)(ii) selector
    binding, are judged under their own entries);
    (2) real value — the intermediate holds a computation that appears in
    the target's own bytes and only relocates where the value is named;
    pure no-op copies stay with the dead-store family and its
    prerequisites (a copy of a stack-passed parameter is admitted only
    under [[ordinary-c-judge-decidable]] Ruling 12, 2026-09-26, and every
    prong there); (3) byte-neutral — `build_insns == target_insns`, the
    compiler folds the copy; (4) fresh local, not a borrow —
    [[staged-value-reused-variable]] keeps its own bounds; (5) destination
    not live-pre-initialized (the `x/tx` FAIL, decisions.md:4149, stands);
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
  original binary, or a committed naming-census schema — OR by
  compiler-necessity evidence meeting every condition (a1)-(a4) of the
  2026-09-26 amendment below (for a mixed-field struct, (a1), (a2) and
  (a4′) in place of (a3)-(a4)); (b) the merged
  declaration reflects that documented shape (a struct/record table where
  the evidence shows records; a flat array only where the evidence shows
  a flat array) — an index that encodes a record stride as a magic number
  does NOT qualify; (c) the merge is complete: every merged per-word
  symbol is removed from C and from the splat symbol config, leaving
  exactly one C handle per storage location — **amendment 2026-09-03
  (operator, under the owner's 2026-08-19 self-serve discretion; func_80062020
  20:46 layer-1 FAIL): a per-word symbol row may STAY in
  `undefined_syms_auto.txt` / `named_syms.txt` while a still-`INCLUDE_ASM`
  sibling's `asm/funcs/*.s` references it (deleting it would break that
  sibling's assembly), provided no C code names the symbol and the row is
  suffixed `/* alias of <base>+N; retire with <sibling> */`. Prong (c) is
  then satisfied; the row retires when the sibling lands**; (d) spelled at the canonical
  declaration in the shared header, never TU-local, never a per-use
  pointer pun; (e) byte-neutrality verified for every other consumer,
  full `verify-oracle --rebuild`, layer-2 cheat-reviewer.
  - **Amendment (owner ruling 2026-09-26) — compiler-necessity evidence for
    prong (a).** Filed question: docs/grind/borderline.md 2026-09-26
    func_800620B8 (four adjacent sprite-table labels) and its session-2
    addendum. The question put to the owner, verbatim: "Accept 'only one
    object compiles to these bytes' (confirmed by the original PsyQ
    compiler) as evidence for merging adjacent data labels into one
    table?" The framing: "accept compiler necessity as evidence when it's
    confirmed by the original compiler and the merged object is the
    smallest span that covers the labels actually used." Owner (Trenton)
    chose, verbatim: **"Accept, minimal span (Recommended)"**, whose text
    is: "Compiler-necessity proof + cc1psx confirmation + merged object
    limited to the labels actually used. Unblocks 800620B8, likely
    8005D814." What follows is the author's narrowing, not the owner's
    words. Prong (a) is met by compiler-necessity evidence only when ALL of
    (a1)-(a4) hold, or, for a mixed-field struct, (a1), (a2) and every part
    of (a4′):
    - **(a1) Necessity, proven from dumps.** The function's ledger banks the
      compiler dumps (cc1 RTL dumps such as `.cse`, `.loop`, `.lreg`,
      `.greg`, and/or the instrumented cc1's `BB2_*_DEBUG` output,
      `tools/gcc-2.7.2/cc1`, with command lines) for the split-label
      spelling and the merged spelling. It names the compiler decision that
      yields the target's codegen, by pass and source location in
      `tools/gcc-2.7.2` (e.g. cse relating the addresses as offsets of one
      symbol, so loop.c hoists the base). It shows that decision depends on
      the addresses being offsets of ONE object, so that no spelling with
      separate objects can produce it, not only the spellings measured. The
      best split-label floor and at least one structural respelling are
      recorded with their `sandbox --disable all` scores. Only a split
      spelling that relies on a REFUSED or BANNED construct (e.g. F4
      cross-symbol arithmetic, a per-use pointer pun) is set aside: it does
      not count against (a1) and does not land. A split spelling whose every
      construct is admissible under a sanctioned family with that family's
      prerequisites met (e.g. a FAKE-annotated pointer alias under
      [[pointer-alias-fake-exception]]) counts: if it reaches the target's
      codegen, (a1) is not met.
    - **(a2) The original compiler agrees** (a calibration use). The
      original PsyQ compiler, run through `tools/cc1psx_wrapper.sh` on the
      same preprocessed translation unit with the build's cc1 flags, is run
      twice: once with the split declarations and once with the merged
      declaration. With the split declarations it does NOT produce the
      target's shape; with the merged declaration it DOES. "The target's
      shape" means the instructions that form and hold the merged labels'
      addresses match the target's in opcode, register and offset. The
      ledger banks both outputs and their diff against the target. If
      cc1psx produces the target's shape from the split declarations too,
      or does not produce it from the merged one, (a) is not met. This is a
      calibration use of cc1psx under [[cc1psx-calibration-only]] and
      [[no-compiler-divergence]]: cc1psx is never a build path, the
      committed build compiles with the project's cc1, and the oracle SHA1
      decides the match.
    - **(a3) The minimal span.** The merged object covers exactly the bytes
      from the start of the lowest-addressed label the function under
      judgment references to the end of the highest-addressed label it
      references. Every splat label inside that span is merged (prong (c)
      applies to each, including any another function references); a label
      inside the span that no code references is included only because it
      lies inside the span. No label outside the span is merged in, even
      when records of the same shape continue past either end (as they do
      before 0x8009BA00 for func_800620B8).
    - **(a4) One element shape.** Every element in the span has one record
      layout (the same size, with the same field types at the same offsets),
      shown by the function's own accesses and by the original data. The
      declaration is an array of that record type, with a length of exactly
      the span divided by the record size. Prong (b) applies unchanged,
      including its ban on encoding a record stride as a magic number.
      - **(a4′) Mixed-field struct (owner ruling 2026-09-26, second
        batch, Q7).** The question put to the owner, verbatim: "Your
        table-merge ruling covered one repeated record type. func_80034708
        needs 0x78–0x87 declared as ONE struct with mixed fields (a u16
        pair, four byte pairs, four bytes). Both compilers, including the
        original PsyQ one, match the shipped code only with that struct,
        and it spans exactly the bytes the function uses. Extend the ruling
        to mixed-field structs under the same proof?" Owner (Trenton)
        chose, verbatim: **"Allow with same proof (Recommended)"**, whose
        text is: "Same dump proof + cc1psx confirmation + minimal span of
        bytes actually used; layer-2 still reviews. Unblocks func_80034708
        (with -G8)." What follows is the author's narrowing. In place of
        (a4) and of (a3)'s label bounds, a single struct with fields of
        different types is admitted ONLY when (a1) and (a2) hold for that
        struct against the separate labels AND all of (1)-(5) hold:
        1. **The span is exactly the bytes the function uses.** It runs from
           the lowest-addressed byte to the highest-addressed byte that the
           function under judgment accesses, as its original instructions
           show: every load, store and address formation into the span,
           including indexed or walking accesses over their reachable range.
           Every splat label inside the span is merged (prong (c)); no label
           or byte outside it is.
        2. **Members for used bytes only.** Each named member covers only
           bytes the function accesses. Its width comes from the width of
           the function's accesses to it. Its signedness comes from ONE
           declared type under which every one of the function's accesses
           to that member is ordinary C: explicit value casts are allowed,
           pointer puns are not. (A byte read both with `lb` and with `lbu`,
           such as D_80102785, gets one declared type, and the other
           accesses are spelled with ordinary casts.) An array member is
           used only where the function indexes or walks those bytes, and
           its length is exactly the reachable range.
        3. **Gaps, and nothing else.** A byte inside the span that the
           function never accesses is covered either by alignment padding
           the compiler inserts, or by one filler member named by its
           offset (`u8 unkNN[k];`, no role name) whose size is exactly that
           gap. No other padding or filler member is admitted, and no member
           may cover a byte outside the span.
        4. **The layout is shown member by member.** The ledger lists each
           member: offset, width, declared type, and every one of the
           function's instructions that access it, by address and opcode
           (or, for a filler, that none does).
        5. **Everything else unchanged.** Prongs (b)-(e) apply: the struct
           is the canonical declaration in the shared header, the merge is
           complete, every other consumer of a byte in the span is respelled
           through the struct's members with no per-use pointer pun (prong
           (d)), and byte-neutrality, `verify-oracle --rebuild` and a fresh
           layer-2 cheat-reviewer are required. A struct whose members a
           necessary other consumer cannot use without a pun fails.

        func_80034708's struct is judged fresh against (a1), (a2) and
        (1)-(5); nothing here pre-decides it. Record:
        docs/grind/decisions.md 2026-09-26 OWNER RULING —
        aggregate-merge (a4′): mixed-field struct.

        **Amendment (owner ruling 2026-09-26, sixth batch, Q13): forced-in
        bytes typed by their real users.** The question put to the owner,
        verbatim: "func_80036140 needs a struct that must cover a few bytes
        it never touches itself. Other functions DO use those bytes: one as
        a 16-bit value, another as a 32-bit value. The mixed-struct rule
        says untouched bytes must be an anonymous filler, but then those
        other functions would need pointer tricks to reach them, which is
        banned. May such forced-in bytes be proper named fields, typed by
        how the other functions actually access them?" Owner (Trenton)
        chose, verbatim: **"Yes, typed by real users (Recommended)"**, whose
        text is: "Only bytes the necessity proof forces inside the span;
        each field's type must match another function's actual access
        width/signedness in the original bytes; layer-2 reviews." (Record:
        docs/grind/owner-rulings-2026-09-26.md, batch 6.) What follows is
        the author's narrowing. It amends (2)-(4) only:
        - **Forced-in bytes.** A byte is forced in when it lies inside the
          span of (1) and the function under judgment never accesses it:
          the (a1) proof requires the one object, and the object is
          contiguous, so it must cover that byte. This amendment does not
          move the span. (1) stands unchanged, and no byte outside the span
          of the function's own accesses is admitted.
        - **A named member for a forced-in byte.** In place of the (3)
          filler, forced-in bytes may be covered by a named member ONLY
          when some other function accesses exactly those bytes in its
          ORIGINAL bytes (`asm/funcs/<func>.s`). The member's offset and
          width equal those accesses' offset and width. When several
          functions access the same bytes, they all use that one offset and
          width, or the member is not admitted. Its declared signedness
          EQUALS the signedness that at least one of those accessors shows
          in its original bytes, and the ledger cites that evidence:
          - for a byte or halfword member, the load opcode (`lb`/`lh` is
            signed, `lbu`/`lhu` is unsigned);
          - for a word member, or where the accessors only store (`sw`,
            `sh`, `sb` carry no signedness), named evidence from an
            accessor's original instructions acting on the value, limited
            to choices GCC makes BY signedness: an ordered compare (`slt`/
            `slti` versus `sltu`/`sltiu` testing less-than or greater-than);
            a plain shift right (`sra` versus `srl`) that is not part of a
            divide-by-a-power-of-two expansion; or a divide (`div` versus
            `divu`). These are NOT evidence: an equality or zero test
            (`sltiu x,1`, `sltu $0,x`), a jump-table bounds check, a shift
            inside a divide expansion, and `mult`/`multu` (the low word is
            the same, and GCC 2.7.2 emits `mult` for both). No rule sets a
            default signedness for such a member: without admissible
            evidence it is not admitted, and its bytes stay a filler per
            (3), except as the Q14 clause below allows.
          - **When the binary cannot tell (owner ruling 2026-09-26, seventh
            batch, Q14).** The question put to the owner, verbatim: "For
            func_80036140's struct, one forced-in 32-bit field (the expected
            disc position at 0x80101EA0) is only ever loaded, compared for
            equality, incremented and stored — so the shipped code can't
            reveal whether it was signed; signed and unsigned compile to
            identical bytes. May it keep the type it already has on main
            (s32, which also matches libcd's CdPosToInt returning int), when
            no instruction in the binary can tell them apart?" Owner
            (Trenton) chose, verbatim: **"Keep existing type
            (Recommended)"**, whose text is: "When no signedness-revealing
            instruction exists anywhere and both choices are byte-identical,
            the member keeps its current declared type on main (or the SDK
            type it's assigned from); layer-2 checks byte-identity."
            (Record: docs/grind/owner-rulings-2026-09-26.md, batch 7.) The
            author's narrowing: a member that the word/store-only clause
            above would leave a filler for lack of evidence is admitted,
            with a kept signedness, ONLY when all of these hold:
            1. **Nothing reveals it, anywhere.** The ledger lists, for every
               function whose original bytes access the member, each access
               (function, address, opcode) and each instruction that the
               loaded value flows into up to its last use, and shows that
               none is an instruction whose choice depends on signedness.
               That test is wider than the evidence list above, which only
               says what counts as positive evidence. Any
               signedness-dependent instruction blocks this clause,
               including a sign test on the value (`bltz`, `bgez`, `bgtz`,
               `blez`), the sign fix-up of a divide-by-a-power-of-two
               sequence, and a sub-word read of it with `lb`/`lh` versus
               `lbu`/`lhu`. If such an instruction is admissible evidence,
               the evidence clause governs; if it is not, the member is not
               admitted and its bytes stay a filler per (3).
            2. **Both choices are byte-identical.** Every accessor compiled
               from C is built with the build's exact per-file recipe twice,
               once with the member declared signed and once unsigned, and
               the two builds of that whole function are byte-identical. The
               ledger banks, for each such accessor, either both full
               listings of the function or both object hashes, with the
               build command lines, and the layer-2 reviewer checks the
               byte-identity. An accessor still committed as `INCLUDE_ASM`
               assembles from its `.s` file, so its bytes cannot depend on
               the declared type; for it, the (1) no-reveal listing of its
               original instructions stands in for this proof, and it is
               not a reason to refuse the member.
            3. **The kept type is named.** The member keeps the signedness
               of the type main declares today for those bytes (the symbol
               it replaces, cited by file and line). Only when main has no
               declaration for them does it take the signedness of the SDK
               type of the value assigned to it (a PsyQ/SDK prototype,
               cited). The ledger names which source applies. Width still
               comes from the accesses, as above.
            This clause sets no default where evidence exists, admits no
            byte the function under judgment accesses (those follow (2)),
            and changes nothing else in this amendment.
          Every one of those accesses is then ordinary C under that declared
          type (explicit value casts are allowed, pointer puns are not), as
          in (2). The name follows the project's naming-evidence
          rules: an offset-derived name is always admissible, and a role
          name needs its own evidence ([[names-require-evidence]]).
        - **Everything else is a filler.** A forced-in byte that no other
          function's original bytes access in that way stays covered by
          compiler alignment padding or an offset-named filler exactly as
          (3) says.
        - **The ledger shows it.** (4)'s member table lists each such
          member with its offset, width and declared type, and every other
          function's access to it by function, address and opcode, marked
          as not accessed by the function under judgment, and the
          signedness evidence (the load opcode, or the named ordered
          compare, plain shift or divide) by address, or, for a member
          under the Q14 clause, the no-reveal listing, the byte-identity
          proof and the named source of the kept type.
        - **Review.** A fresh layer-2 `cheat-reviewer` reviews the struct
          with the member table, as (5) already requires. Everything else in
          (a1), (a2) and (1)-(5) stands.

        func_80036140's struct is judged fresh against (a1), (a2), (1)-(5)
        and this amendment; nothing here pre-decides it. Record:
        docs/grind/decisions.md 2026-09-26 OWNER RULING — aggregate-merge
        (a4′): forced-in bytes typed by their real users.

    Prongs (b)-(e) are unchanged: the declaration is canonical in the shared
    header, the merge is complete, and byte-neutrality for every other
    consumer, `verify-oracle --rebuild` and a layer-2 cheat-reviewer are
    required. The amendment decides nothing about func_80036140's
    build-model question. Of its two parts, per-file `-G8` was ruled
    separately later the same day (owner ruling 2026-09-26, second batch,
    [[compiler-flags-canonical]] § "Per-file -G8 by proof", its own prongs
    (i)-(vi)). The maspsx COMMON model was ruled in the fourth batch the
    same day, as the gated list `maspsx_comm_syms.txt` with its own prongs
    (a)-(d) in [[maspsx-gate-lists]]. WITHDRAWN 2026-09-30: the owner ruled
    that per-function gate a cheat, the list is retired and empty, and no
    merge or landing may depend on it (decisions.md 2026-09-30 OWNER RULING);
    this amendment admits no gate row. Record:
    docs/grind/decisions.md 2026-09-26 OWNER RULING — aggregate-merge prong
    (a): compiler-necessity evidence, minimal span.
  - **Exception to prongs (c)/(d): per-file declarations of the same bytes
    (owner ruling 2026-09-26/27, twelfth batch, Q21).** The question put to
    the owner, verbatim: "func_8001CE60 needs two player byte-pairs declared
    as 2-element arrays, but a completed neighbour in a different file
    (func_800340A0) only matches with the same bytes declared as separate
    single bytes — the original compiler (confirmed with Sony's cc1psx)
    can't produce both from one declaration. SOTN keeps annotated
    declaration mismatches when the original bytes require them. May each
    file declare those bytes the way its own code needs (arrays in
    code6cac.c, single bytes in code6cac_b.c), annotated, with the compiler
    evidence? A full build of this form matches the original." Owner
    (Trenton) chose, verbatim: **"Allow, per file, annotated
    (Recommended)"**, whose text is: "Only when proven that no single
    declaration compiles both files (dumps + cc1psx), declarations kept
    file-local (not in a shared header), each annotated; layer-2 reviews."
    (Record: docs/grind/owner-rulings-2026-09-26.md, batch 12.) The SOTN
    norm the question cites is the prototype-contradiction practice: the
    byte match decides the declaration, and a mismatch is kept, isolated and
    annotated, only where the original binary demands it (2026-08-10 SOTN
    master precedent research, recorded in the Claude harness memory, not in
    this repo: e.g. `src/main/psxsdk/libsnd/stop.c:5` "seems to require
    wrong prototype"). Its prototype-norm examples are function prototypes; applying the
    norm to data declarations is this owner ruling, not the research. What
    follows is the author's narrowing. Two TUs may declare the same bytes
    with different C types ONLY when ALL of (1)-(6) hold:
    1. **Proven: no single declaration compiles both files.** The ledger
       banks every single-declaration respelling attempted (each type or
       layout tried for those bytes, with the respellings of each file's
       functions under it) and its score. The ledger banks the compiler
       dumps (cc1 RTL dumps such as `.cse`, `.loop`, `.lreg`, `.greg`,
       and/or the instrumented cc1's `BB2_*_DEBUG` output,
       `tools/gcc-2.7.2/cc1`, with command lines) for each file's functions
       under its own declaration and under each banked single declaration.
       They name, by pass and source
       location in `tools/gcc-2.7.2`, the compiler decision each file's
       target depends on, and show that it depends on a property of the
       declaration itself, so that EVERY spelling of that file's functions
       under a declaration lacking the property misses the target, whatever
       its statement order, locals or respelling, including spellings that
       use any construct on the frozen sanctioned list that needs no FAKE
       annotation, with that entry's prerequisites met (spellings that need
       a FAKE-annotated construct are set aside under owner ruling Q23
       below, and those relying on a refused or banned construct are set
       aside as stated below). An argument that covers only the spellings
       measured fails (1). The ledger then shows, for every admissible
       single declaration, which file's property it lacks, so that no
       admissible declaration has both files' properties. A declaration for
       which this is not shown defeats (1). The single declarations considered are every
       admissible declaration of those bytes: at least each file's own form
       used as the one shared declaration, plus any struct or other
       aggregate the evidence admits. A single-declaration spelling that
       relies on a REFUSED or BANNED construct (a per-use pointer pun, F4,
       F5 union CLOBBER, an asm alias rename, an array subscript or
       pointer offset read of a dummy local, one whose value at every one
       of its reads is fixed at compile time (the same on every feasible
       path at that read, possibly differing between reads), per owner
       ruling 2026-09-27 Q22 in
       [[named-local-fake-exception]]) is set aside. **Owner ruling
       2026-09-27, thirteenth batch, Q23 ("Per-file wins over FAKE"):** a
       single-declaration spelling that needs any construct whose frozen-list
       entry, or the rule file that entry links, expressly requires a FAKE
       or !FAKE annotation (for example a constant-holder or dead scalar
       local, [[named-local-fake-exception]] prerequisite 3; a C-level
       pointer alias, [[pointer-alias-fake-exception]] prerequisite 3; a
       dead store or self-assign, [[dead-store-fake-exception]]; a
       do-while(0) wrap, [[do-while-zero-exception]]) is also set aside: it
       does not count against (1), and the ledger need not show that it
       misses. For each spelling set aside under Q23, the ledger names the
       family and quotes the sentence of its entry (or linked rule) that
       requires the annotation. A construct whose entry does not expressly
       require a FAKE annotation, or that has a truthful semantic reading as
       ordinary C, is not set aside and counts; in doubt, it counts. The question put
       to the owner, the answer and the other options are verbatim in
       docs/grind/owner-rulings-2026-09-26.md (batch 13); the owner chose
       "Per-file wins over FAKE (Recommended)", whose text is: "For the
       per-file-declaration rule, a one-declaration spelling that needs any
       FAKE-annotated construct (any family) doesn't count against it. The
       per-file form (zero FAKE constructs) lands, still with the compiler
       proof, annotations and layer-2. This matches the existing 'fewest
       no-purpose constructs wins' principle." The author's narrowing: the
       per-file form itself must then carry no FAKE-annotated construct in
       the functions the (1) proof covers (otherwise it is not the "zero
       FAKE constructs" form the owner preferred, and (1) is judged without
       this set-aside); spellings whose constructs need no FAKE annotation
       (e.g. ordinary C, a Ruling 11 reused local) still count. One whose
       every construct is admissible and that needs no FAKE annotation
       counts, and if it compiles both files, (1) is not met. Record:
       docs/grind/decisions.md 2026-09-27 OWNER RULING — per-file
       declarations win over FAKE-construct spellings. **Owner ruling
       2026-09-27, thirteenth batch, Q25 ("Mechanism + search"):** the
       owner chose "Mechanism + search (Recommended)", whose text is:
       "Accept when: the core compiler mechanism is shown with dumps +
       cc1psx, every rewrite a reviewer proposes is measured and misses the
       original, and no reviewer can produce one that matches. A new kind
       of rewrite that still misses gets banked as more evidence, not a
       FAIL. Only an actual matching rewrite defeats the per-file form."
       (Question and other options verbatim:
       docs/grind/owner-rulings-2026-09-26.md, batch 13.) The author's
       narrowing: this SUPERSEDES the universal requirements above that
       the dumps show EVERY spelling misses and that "an argument that
       covers only the spellings measured fails (1)", and it replaces the
       sentence "The ledger then shows, for every admissible single
       declaration, which file's property it lacks ... A declaration for
       which this is not shown defeats (1)" with: for every admissible
       single declaration, the ledger records, for each banked counting
       spelling under it, whether it hits or misses its file's full
       target, and names a covered file for which no counting spelling
       under that declaration hits. Minimum measurement: for each single
       declaration considered (at least each file's own form used as the
       shared one, plus every aggregate the evidence admits), the ledger
       banks at least one counting spelling of each covered file under it;
       without that, it may not name a file "for which no counting
       spelling hits". (1)'s proof
       burden is met when ALL of: (a) the core compiler mechanism by which each
       file's target depends on its declaration is named, by pass and
       source location in `tools/gcc-2.7.2`, from banked dumps, with the
       cc1psx corroboration below (Q24); (b) every single-declaration
       spelling proposed by the author or by any reviewer is banked in the
       ledger and measured under the build's cc1 and cc1psx, and for each
       one that counts (not relying on a refused or banned construct, not
       set aside under Q23) the ledger records whether it hits or misses
       its file's full target under the build's cc1 ("full target": the
       instructions of every function of that file the proof covers are
       byte-identical to the target's), and it passes the Q24 agreement
       test; the hit/miss defeat decision is left to (c); (c) no single
       declaration is DEFEATING. A single declaration is defeating when,
       for EVERY file the proof covers, some counting spelling of that
       file's functions under that declaration (from anyone, banked or
       proposed) hits that file's full target under the build's cc1; the
       files' spellings are judged separately, each in its own TU. A
       reviewer's new counting spelling that does not make its declaration
       defeating, once banked and passing the Q24 agreement test, is
       evidence and not a FAIL ground under (1); a reviewer proposal not
       yet banked is a banking step before landing, not a (1) failure;
       only a defeating declaration defeats (1). The annotation
       required by (4) states only what the banked measurements show
       (e.g. "no single declaration compiles both files with a counting
       spelling": for each banked shared declaration, a covered file whose
       counting spellings all miss; spellings set aside under Q22/Q23 are
       excluded), not an unproven universal. A reviewer's proposal is
       banked and measured before a fresh layer-2 PASSes the landing. The original PsyQ cc1psx (`tools/cc1psx_wrapper.sh`,
       calibration use only, never a build path), run on the same
       preprocessed TUs with the build's cc1 flags, gives the same result
       for every single declaration banked under (1): at least one of the
       two files' functions misses its target shape. Under each file's own
       declaration, that file's functions reach it. **Owner ruling
       2026-09-27, thirteenth batch, Q24 ("cc1psx only where it can"):**
       the owner chose "cc1psx only where it can (Recommended)", whose text
       is: "cc1psx must agree on every part of the proof where it
       reproduces the already-matching form; for parts it can't reproduce
       even there (like this store structure), our compiler's dumps and
       measurements decide. Record that as a narrow clarification, then
       land func_8001CE60 per-file after a fresh layer-2." (Question and
       other options verbatim: docs/grind/owner-rulings-2026-09-26.md,
       batch 13.) The author's narrowing, applied instruction by
       instruction: (i) a file's REFERENCE form is the exact body, in the
       exact translation unit, that will be on main after the landing (for
       an already-completed function the landing leaves unchanged, its
       committed body in its committed TU), and no other spelling; every
       banked spelling is measured in that same TU. (ii) Take the reference form's `.s` from the
       build's cc1 (byte-identical to the target) and from cc1psx: each
       compiler's own `-S` output of the same preprocessed TU under the
       build's cc1 flags, before maspsx (as
       memory/grind/func_8001CE60/probes/calib_800340A0/calib_b.sh
       produces), cut to the function. Strip directives, blank lines,
       comments and label lines; compare symbol operands by the address
       they resolve to (a spelling's own name for the same bytes, plus
       offset, is canonicalised to the target's symbol, e.g. a probe's
       `g_sc+1` to `D_800A3899`); replace every register operand except
       `$0` and every label operand with a placeholder BEFORE aligning;
       align the two instruction lists with Python
       `difflib.SequenceMatcher(autojunk=False)` and bank that alignment.
       After alignment, a branch or jump counts as equal to its aligned
       counterpart only if the counterpart's destination is the cc1psx
       instruction aligned, in an equal run, with the instruction at its
       own destination, or both leave the function; one failing this is
       treated as an instruction of a changed run with no equal
       counterpart. Wherever target shape is judged, branch and jump
       destinations are compared the same way, by aligned position, never
       by label name. (iii) A governed instruction is EXEMPT only if it
       lies in a changed run of that alignment AND no instruction on the
       cc1psx side of that run equals it after the placeholders; for an
       exempt instruction the build's cc1 dumps and measurements alone
       decide. Every other governed instruction, including one with an
       equal counterpart inside a changed run, is subject to AGREEMENT:
       for every spelling banked under (1) and not set aside under Q22 or
       Q23, each file's functions hit or miss the non-exempt governed
       instructions under cc1psx exactly as they do under the build's
       cc1 (a miss = at least one non-exempt governed instruction of the
       reference form differs from the spelling's output under that
       compiler, compared by the same procedure; under cc1psx the governed
       instructions are the equal counterparts, in the reference
       alignment, of the non-exempt target-side governed instructions, and
       cc1psx-only instructions are not governed; a spelling whose only
       differences are inserted instructions counts as a hit, which can
       only block a landing). GRAIN: the ledger states, for each part of
       the (1) mechanism, whether the position of its governed
       instructions is part of what that mechanism decides (e.g. a store
       and branch structure: yes; an addressing mode: no). For a part
       where position is not decided, both the exemption test and the
       hit/miss test count a governed instruction as reproduced when an
       instruction equal to it after the placeholders is present anywhere
       in the compared function's output, counted with multiplicity (a
       displacement-only difference is a hit, which can only block); for
       a part where position is decided, the aligned-position procedure
       above applies. (iv) The ledger banks
       the script, the alignment, the governed instructions (by target
       address, with the (1) mechanism each belongs to; "every instruction
       of the function" is allowed when stated), and the list of exempt
       instructions with their target addresses. The two cc1psx sentences
       above are always read as this agreement test on the non-exempt
       governed instructions (all governed instructions when none is
       exempt); together with Q25's (c) it is the only defeat test. "Target shape" means the
       instructions the (1) mechanism governs match the target's in opcode
       and in every operand the mechanism decides (for an addressing
       mechanism: an absolute symbol versus a base register, how that base
       register is formed, and the offset). Destination and other registers
       chosen by allocation outside the mechanism are compared under the
       build's cc1 only (sandbox 0 and the full build), because cc1psx is
       calibration-only and its allocation (and, outside the governed
       instructions, its jump structure) can differ from the target's
       across a whole function. (Correction, 2026-09-27, to the author's own
       narrowing: the earlier text required every register of the governed
       instructions to match under cc1psx, which the committed COMPLETED-C
       scalar form of func_800340A0 does not: 70 differing lines under
       cc1psx (30 differ in label name only, 20 in register only, 4 in
       both, and 16 in one tie-break hunk whose jump structure differs;
       no governed address operand differs), identical under the build's cc1;
       memory/grind/func_8001CE60/probes/calib_800340A0/SUMMARY.txt. Record:
       docs/grind/decisions.md 2026-09-27 rule correction — Q21 condition
       (1) "target shape" under cc1psx.) The ledger banks every cc1psx
       output and its diff against the target.
    2. **Same bytes, same accesses.** The two declarations cover exactly the
       same bytes and differ only in grouping: an array of an element type
       against separate scalars of that type, or a struct against its
       members as scalars. Every element's width equals the width of every
       access to it in that file's original instructions
       (`asm/funcs/<func>.s`). Its signedness is one declared type under
       which each of that file's accesses is ordinary C, with explicit value
       casts allowed and pointer puns not. A declaration that covers bytes
       at a different width than the other file's (e.g. a `u16` over two
       `u8`), or that is reached through a cast, a pointer pun, a union or
       an `asm("sym")` alias, is refused. Prong (d)'s ban on per-use pointer
       puns applies unchanged. **Owner ruling 2026-09-30, twenty-fifth
       batch, Q52:** a local cast re-view (`(T *)&D_80xxxxxx`) that SOTN marks as a hack or debt (the author's narrowing, not the owner's words: any hack or debt signal SOTN attaches to the construct, in any form: a comment at or naming it anywhere in the file or header — `!FAKE`, `FAKE`, `fake`, `hack`, `HACK`, `TODO`, `FIXME` or similar; a label, identifier or macro name calling it a hack or fake, e.g. `goto hack;`, `CREATE_FACTORY_FAKE_ARGS`; or code compiled only under `HACKS` or a hack- or fake-named define; when a reviewer finds any such signal, ours carries `/* FAKE: … */`)
       is NOT refused by (2) or by prong (d)'s pun ban when SOTN ships the
       same construct (a citation meeting § Owner ruling 2026-09-30 — SOTN
       precedent suffices below) and ours carries a `/* FAKE: ... */`
       marking, with Q53's family prerequisites met. Such a re-view is
       admitted on that citation, not by (1)-(6)'s admission test. The
       family prerequisites it owes under Q53 are, explicitly: this
       exception's own annotation requirement ((4): an inline comment
       citing the evidence and this ruling, here the SOTN citation) and
       its exhaustion requirement ((1)'s banked, measured single-declaration
       respellings), plus [[pointer-alias-fake-exception]] prerequisite 3's
       FAKE annotation; layer-2 per (6). An unmarked SOTN cast re-view (one SOTN presents as
       ordinary code) rests on the general Q50 section below and is likewise
       not refused by (2).
    3. **File-local.** Each declaration sits in its own `.c` file. No shared
       header declares those bytes in either form. The exception covers only
       the files whose functions the (1) proof covers, and each of them
       keeps exactly one C handle per storage location within itself.
       Splat symbol rows that either declaration needs stay (prong (c)'s
       one-handle rule is relaxed only across these files). A function in
       any other file that later references those bytes is admitted only by
       a fresh (1)-(6) submission that extends the proof to it. Until then
       no other file declares them.
    4. **Annotated.** Each declaration carries an inline comment that names
       the other file's declaration (file and line), states that the
       mismatch is kept because no single declaration compiles both files
       with a counting spelling (for each banked shared declaration the
       ledger names a covered file whose counting spellings all miss;
       spellings set aside under Q22/Q23 are excluded; owner ruling Q25 in
       (1)), and cites the ledger evidence and this ruling.
    5. **Bytes.** `verify-oracle --rebuild` (full-build SHA1 == oracle) and
       sandbox 0 for the functions landing with it.
    6. **Review.** Admission needs a fresh layer-2 `cheat-reviewer` PASS
       that walks (1)-(5) against the ledger. On the Grinder path a Judge
       PASS is not enough: the Judge may not admit a declaration under this
       exception, and a body that relies on it lands only through the
       manual path's layer-2.

    This does NOT license mismatched declarations for convenience: a
    mismatch without the (1) proof is refused, and a declaration a single
    shared form can serve stays unified in the header as prongs (c)/(d)
    require. Prongs (a), (b) and (e) apply unchanged to each declaration
    that merges splat symbols, except that where (a3) or (a4′)(5) restate
    prong (c) (merge completeness) or prong (d) (the canonical declaration
    in the shared header), they are read with this exception's relaxations
    of (c) and (d), and only across the proven files. The per-file array still needs its (a)
    object-model evidence and its (b) shape, and every other consumer in
    each file stays byte-neutral. Prong (d)'s pun ban also stands (except
    the SOTN-cited cast re-views in (2)); only its
    shared-header and never-TU-local requirements are relaxed, and only for
    the proven files. On data declarations, the same research found
    volatile-in-one-TU essentially absent ecosystem-wide, and cross-TU
    width/type conflicts common but concentrated in imported library code,
    with curated game code unified. Its data-side findings on second typed
    views are SOTN's local cast re-views (`(T*)&D_80xxxxxx`), which ship in
    matched code carrying !FAKE/TODO debt markers and which cleanup PRs move
    away from, and Silent Hill's practice, where a unified view cannot
    re-match, of keeping two typed views through a union and a second linker
    symbol at the same address, each annotated @hack. (2) refuses the
    Silent Hill spelling absent a verified SOTN citation (Q55); SOTN's cast
    re-view is admitted only under the Q52 amendment to (2) or the general
    Q50 section. The extension to data
    declarations in game code therefore rests on this owner ruling alone.
    func_8001CE60's per-file form is judged fresh
    against (1)-(6); nothing here pre-decides it. Record:
    docs/grind/decisions.md 2026-09-27 OWNER RULING — per-file declarations
    of the same bytes.
  - **Amendment to prong (d): a union word view over small fields (owner
    ruling 2026-09-29, twentieth batch, Q33).** The question put to the
    owner, verbatim: "Some shipped code clears two adjacent 16-bit values
    with one 32-bit store. Right now the only way to write that is a
    pointer cast, which the reviewers reject. It affects func_8005E54C (a
    local pair of round counters) and func_80070188's neighbour (the
    0x800A3560 slot records, the open item in borderline.md). A cast-free
    way is to declare the storage as a union of the small fields and one
    32-bit word. May we use a union like that, when the shipped bytes show
    the single 32-bit access?" Owner (Trenton) chose, verbatim: **"Allow,
    with evidence (Recommended)"**, whose text is: "A union of the real
    fields plus one word member is allowed only where the original bytes
    show a single word store or load over those fields. Layer-2 reviews
    every use. This closes func_8005E54C's counters and lets 0x800A3560 be
    declared as records, which drops about 9 filler variables." (Record:
    docs/grind/owner-rulings-2026-09-26.md, batch 20.) What follows is the
    author's narrowing, not the owner's words. An object (a global, a
    local or, under the Q46 extension below, a struct member) may be
    declared as a union with a word member ONLY when ALL of (1)-(7) hold:
    1. **The original bytes show one word access.** At every site that
       names the word member, the function's original instructions
       (`asm/funcs/<func>.s`) show ONE `lw` or `sw` spanning exactly the
       bytes the word member covers, cited in the ledger by function,
       address and opcode, and the build emits that one instruction there.
       Only a 32-bit access qualifies (the question and the answer speak of
       a single 32-bit, word, store or load). A halfword view, an
       unaligned `lwl`/`lwr` or `swl`/`swr` pair, a block copy and any
       access wider than a word are outside this amendment.
    2. **Exactly the real object plus one word.** The union has exactly two
       members: the real object (the record array, struct or field array
       that its own evidence establishes, e.g. under prongs (a)-(b) for a
       merged global) and one `s32` or `u32` member. Union members start
       at offset 0, so the word covers the object's first four bytes: the
       original word access must start at the object's first byte, and the
       object must be at least four bytes long. No third member, no nested
       union, no padding or filler member added for the union, no word
       member at any other offset. Where an instruction acting on a loaded
       word reveals signedness (the evidence list of the Q13 amendment to
       (a4′) above), the word member's type has that signedness. Its name
       claims only that it is the word view (e.g. `word`); a role name
       needs its own evidence ([[names-require-evidence]]).
    3. **The word member only at the word sites.** The word member is
       named only at sites whose original bytes show the (1) access. Every
       other access to those bytes, in every function, goes through the
       real object's members. A word-member store writes a value the target
       stores there; a word-member load's value is consumed by the
       function's logic.
    4. **No cast, no constructor.** No pointer cast to, from or through the
       union or either member at any site; no cast-to-union constructor
       (`(union u)x`); the union is not reached through a union-typed
       parameter or return value. Prong (d)'s ban on per-use pointer puns
       is otherwise unchanged: a pointer cast over the object stays refused
       even where a union would serve, except the one cast store on a local
       array that the Q36 amendment below admits.
    5. **Globals, locals and struct members.** For a global, the union is the object's one
       canonical declaration in the shared header, and every aggregate-merge
       prong (a)-(e), with its amendments, that governs that declaration
       holds. The word member is not object-model evidence for (a) and
       changes nothing in (b). Every other consumer is respelled through
       the union's members and stays byte-neutral ((e)). For a local, the
       union is the local's one declaration at its scope. Its real member
       is typed by the function's own accesses as any local is, and every
       other rule that governs the local (e.g. Rulings 5-12 of
       [[ordinary-c-judge-decidable]] for a reused local) applies
       unchanged. For a struct member (Q46 extension below), the union is
       that member's one declaration in the struct type's one definition,
       where the union replaces the adjacent members it spans, at their
       existing offsets: the struct's size (except for the trailing
       alignment padding the Q57 clause below admits) and
       every other member's offset are unchanged, and the word member is
       not evidence for the struct's layout, which rests on its own
       evidence. The struct may be a global, a local or reached through a
       pointer; (1)'s access is then the one at the member's offset from
       the struct's base, and the struct object or pointer stays governed
       by the rules that govern it. Every other access to the member, in
       every function, goes through the union's real member and stays
       byte-neutral.
    6. **Layer-2 reviews every use.** Every commit that adds a union under
       this amendment, or adds a site naming a word member, needs a fresh
       layer-2 `cheat-reviewer` PASS that checks (1)-(5) at each site
       against the ledger's cited addresses. On the Grinder path a Judge
       PASS is not enough: such a body lands only through the manual path's
       layer-2. Sandbox 0 and full-build SHA1 == oracle apply as always.
    7. **What this is not.** This is NOT the F5 union-constructor CLOBBER
       ("Surveyed and NOT extended" below; refused 2026-07-19), which
       retypes a scalar as a single-member union and assigns through a
       cast-to-union constructor so that GCC emits a bare CLOBBER that
       changes jump2's cross-jump: a union with no data purpose, used only
       for an RTL side effect. That family, the dead union local, the
       single-member union and every other USE/CLOBBER-manufacture spelling
       stay refused; a union that misses any of (1)-(6) gets nothing from
       this amendment. The Q21 exception's condition (2) above still
       refuses a union as a per-file second view of the same bytes, and the
       Silent Hill two-view practice stays refused.

    Nothing is pre-decided: func_8005E54C's local, and a record declaration
    of 0x800A3560 (func_80070188, landed COMPLETED-C 9e69a87c7 without it;
    func_8006E534 and the object's other consumers), are fresh submissions
    judged against (1)-(7). Record: docs/grind/decisions.md 2026-09-29
    OWNER RULING — a union word view over small fields.

    **Extension to struct members (owner ruling 2026-09-30, twenty-third
    batch, Q46).** The recommendation put to the owner, verbatim: "Extend
    it, same conditions. Your Q33 condition is that the shipped bytes show
    one word access over the small fields. Whether the storage is a
    global, a local or a struct field doesn't change that. Keep the
    evidence requirement and a second review for every use." The owner
    adopted it, verbatim: "Go ahead with all your recommendations. Just
    know my highest priority is avoiding regressions, cheats or
    workarounds being introduced. And weeding out any remaining cheats
    that might be lurking in our project. SOTN is the gold standard when
    in doubt" (Record: docs/grind/owner-rulings-2026-09-26.md, twenty-third
    batch, commit d023686ea.) Only the object's scope widens: the object
    may also be a member of a struct, whether the struct is reached
    through a pointer or not. (1)-(7) apply unchanged at every site, with
    the struct-member clause of (5); the evidence requirement and a fresh
    layer-2 for every commit adding such a union or a word-member site
    ((6)) stay. The Q36 cast store below stays locals-only (its (7)).
    Record: docs/grind/decisions.md 2026-09-30 OWNER RULING — Q33 union
    word views on struct members.

    **Trailing alignment padding (owner ruling 2026-09-30, twenty-ninth
    batch, Q57).** The question put to the owner, verbatim: "Q33/Q46 union
    word views: when an s32 union member makes the compiler round a
    struct's size up for alignment (0x92 -> 0x94), with no member offset
    changed and no filler added, is that allowed?" Owner (Trenton) chose,
    verbatim: **"Allow it (Recommended)"**, whose text is: "Allowed when
    every member offset is unchanged, no filler member is added, and the
    build stays byte-identical. Unblocks the SelWork cluster." (Record:
    docs/grind/owner-rulings-2026-09-26.md, twenty-ninth batch.) What
    follows is the author's narrowing, not the owner's words. In the
    struct-member clause of (5), "the struct's size ... unchanged" is
    relaxed ONLY for the size change the compiler itself makes when the
    union's `s32` or `u32` word member raises the struct's alignment to
    four bytes, so that sizeof rounds up to the next multiple of four (e.g.
    SelWork_800768DC, 0x92 -> 0x94). That size change is admitted ONLY when
    ALL of (a)-(d) hold:
    (a) **Every member offset is unchanged.** Every member of the struct,
        the union included, sits at the offset it had before the union
        replaced the adjacent members it spans; the added bytes lie after
        the last member.
    (b) **No filler member is added.** No member is declared, and no
        member is widened, to occupy or account for the added bytes: they
        are the compiler's own trailing padding. (2)'s ban on a padding or
        filler member added for the union stands unchanged.
    (c) **The build stays byte-identical.** Full-build SHA1 == oracle with
        the grown struct in place, and sandbox 0 for every function the
        commit changes, as (6) already requires.
    (d) **Whole-program layout check** (author's narrowing of "every member
        offset is unchanged"). Search the C in `src/` and `include/` AND the
        target's own bytes (`asm/funcs`, asm data, EXE/disc pointer words)
        for: (1) any containing layout — the struct as an array element or
        as a member of another aggregate; (2) any use of the struct's size
        or stride — `sizeof` in any spelling, a C literal or asm immediate,
        a computed multiplier or stride (shift/add sequences, mult by a
        loaded constant), or a copy/clear/allocation length; (3) any symbol,
        dlabel, pointer word or access that starts inside the added bytes
        `[old end, new end)`, or a partial access that reaches into them.
        Record the search and every hit in the ledger; unsearched = FAIL.
        Each hit is decided by the FIRST of these steps that applies:
        1. **Added bytes — FAIL.** Anything (a symbol, dlabel, pointer
           word, access, or copy/clear sequence) that starts inside an
           instance's added bytes `[old end, new end)`, or reaches into
           them, unless it is a single access or copy/clear sequence that
           starts at that instance's base and covers exactly the new size.
           Another object or member lives there.
        2. **Unrelated — ignored, with the reason recorded.** The hit does
           not address this struct type or any instance of it, e.g. an
           equal immediate used for another quantity, or an offset from a
           base of another type.
        3. **Containing layout — FAIL.** The struct as an array element (a
           stride of any size) or as a member of another aggregate.
        4. **Old size — FAIL.** Any use of the struct's old size (`sizeof`
           in any spelling, a C literal or asm immediate, a computed
           multiplier, a copy/clear/allocation length). An original type
           carrying the word member would itself be rounded, so the old
           size is evidence against the union model.
        5. **New size — passes.** A use of the new size, or a single access
           or copy/clear sequence starting at the base and covering exactly
           the new size.
        6. **Anything else — FAIL.**

        The instructions of one copy or clear sequence that starts at an
        instance's base count as one access, judged by the sequence's total
        length.
    The ledger records the struct's size without and with the union, and
    (6)'s layer-2 checks (a)-(d) together with (1)-(5). Nothing else
    changes: (1)-(7) apply unchanged at every site, the global and local
    clauses of (5) are unchanged, and a size change of any other origin (a
    member moved, added or widened, or a union at a new offset) still fails
    (5). Nothing is pre-decided: the SelWork cluster
    (memory/grind/func_800768DC/selwork-cluster-2026-09-30.md) is a fresh
    submission judged against (1)-(7) and this clause. Record:
    docs/grind/decisions.md 2026-09-30 OWNER RULING — Q33 trailing
    alignment padding.
  - **Amendment: one cast store on a local array (owner ruling 2026-09-29,
    twentieth batch, Q36, a follow-up to Q33).** The question put to the
    owner, verbatim: "Follow-up on the union answer. For func_8005E54C's
    local pair of 16-bit counters, measurement shows the union can't
    reproduce the shipped code. GCC 2.7.2 keeps a 4-byte union in a
    register and moves it to the stack too late, leaving the function 197
    instructions off. The only matching form is almost certainly what the
    original programmer wrote: a local `s16 vals[2]` cleared with one cast
    store, `*(s32 *)vals = 0;`, at a single site. May that one cast store
    on a local array be allowed?" Owner (Trenton) chose, verbatim:
    **"Allow narrowly (Recommended)"**, whose text is: "Only a local
    array, written once through a 32-bit cast at a site where the target
    bytes show exactly that one word store covering exactly the array,
    after the union form was measured and failed. It must be annotated and
    pass layer-2. The union answer still covers globals like 0x800A3560."
    (Record: docs/grind/owner-rulings-2026-09-26.md, batch 20.) What
    follows is the author's narrowing, not the owner's words. One
    statement of the form `*(s32 *)arr = value;` (or `*(u32 *)arr =
    value;`) is admitted ONLY when ALL of (1)-(7) hold:
    1. **A local array with its real element type.** `arr` is a local
       array of this function (not a parameter, global, `static`, struct
       member or pointer), declared with the element type its other
       accesses show (e.g. `s16 vals[2]` for halfword counters), and its
       total size is exactly four bytes. The array is also accessed through
       its elements (`arr[i]`) elsewhere in the function, at least one such
       access is a read whose value the function consumes, and the element
       type is narrower than 32 bits (s8, u8, s16, u16). An array with no
       element read is governed by [[dead-vars-local-array]], not by this
       amendment.
    2. **One word store in the target, exactly over the array.** The
       target function's original instructions show, at that statement's
       position, ONE `sw` at exactly the array's frame offset, covering
       exactly its four bytes (e.g. func_8005E54C 0x8005EA44
       `sw $zero,0x18($sp)`), cited in the ledger by address; the build
       emits that one instruction there, and the stored value is the one
       the target stores.
    3. **Written once, a store only.** The body contains exactly one such
       cast statement, and it is a store. A cast load, a second cast
       store, a halfword or byte cast, a cast of a pointer to part of the
       array (`(s32 *)&arr[1]`), a cast to any other type, and any other
       cast of that array's address anywhere in the function fail. Every
       other access to the array goes through its elements (`arr[i]`).
    4. **The union form was measured and missed first.** The ledger banks
       the Q33 union spelling of the same local (the array and one word
       member, used at this site) on the same body, with its
       `sandbox --disable all` score, and it does not reach the target.
       A union spelling that reaches the target wins, and this amendment
       does not apply.
    5. **Annotation.** An inline comment at the cast statement says that
       it is one word store over the whole array, cites the (2) address,
       the ledger file holding the (4) measurement, and this ruling.
    6. **Layer-2.** A fresh layer-2 `cheat-reviewer` PASS checks (1)-(5)
       against the ledger. On the Grinder path a Judge PASS is not enough:
       such a body lands only through the manual path's layer-2. Sandbox 0
       and full-build SHA1 == oracle apply as always.
    7. **Locals only.** A global, a static, a struct member, a merged
       aggregate or any object reached through a pointer never qualifies:
       globals take the Q33 union form (the owner's answer names
       0x800A3560), and prong (d)'s pun ban stands for them. This admits no
       other pointer cast of any kind, and no cast-offset access
       ([[cast-offset-access-not-in-struct]] class).

    Nothing is pre-decided: func_8005E54C's `*(s32 *)vals = 0;` is a fresh
    submission judged against (1)-(7). Record: docs/grind/decisions.md
    2026-09-29 OWNER RULING — one cast store on a local array.
- **`do { ... } while (0);` wrap** (empty or non-empty body)
  ([[do-while-zero-exception]] / [[sotn-do-while-zero-research-2026-06-04]]):
  sanctioned as a pure-C match device for ANY codegen effect, including
  register allocation (owner ruling 2026-07-06, which SUPERSEDES the
  2026-06-04 reorg.c/LABEL_OUTSIDE_LOOP_P-only scoping; that scoping is
  abolished and must not be cited as a FAIL ground). Match device, not a
  first resort — prefer natural geometry, but exhaustion is not a hard
  gate for single-level wraps (rule prerequisite 2); mandatory
  `/* FAKE: ... */` annotation naming the observed effect; nested wraps
  need a written single-level-insufficient justification (rule
  prerequisite 3). SOTN evidence: 18+ instances in master across
  `sprintf.c`, `5087C.c`,
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
  frame lever (argument count selects the frame delta). (Owner ruling Q55, 2026-09-30: a verified Q50 citation of matched SOTN code admits a construct this text refuses, with Q53's prerequisites; [[no-new-park-categories]] § Owner ruling 2026-09-30 — SOTN precedent suffices, Precedence.)

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
  respelling FAILed by the Judge (decisions.md:1731 lineage) remains
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
  - **Extension: a trailing unused local array with sibling evidence
    (owner ruling 2026-09-29, twentieth batch, Q35).** The question put to
    the owner, verbatim: "func_8005E54C's stack frame has 8 bytes after a
    local struct that no instruction touches. The sibling functions keep a
    digit array at exactly that spot, so the original probably declared an
    array there that this function never uses. An unused, labelled pad
    local is allowed today only as the FIRST local, and the trailing
    position was granted to one function only. Without it the function is
    47 instructions off. Allow a trailing unused local array when sibling
    functions show a real array at that slot?" Owner (Trenton) chose,
    verbatim: **"Allow with sibling evidence (Recommended)"**, whose text
    is: "A FAKE-labelled unused array in a non-leading position is allowed
    only when the frame layout proves the bytes are untouched and
    completed sibling functions declare a real array of that size at that
    offset. It needs honest names, the frame measurements recorded, and
    layer-2. Lane B's func_8005C8A8 may hit the same frame gap." (Record:
    docs/grind/owner-rulings-2026-09-26.md, batch 20.) What follows is the
    author's narrowing, not the owner's words. It relaxes, for an array
    meeting ALL of (1)-(8), only three parts of the FORM CONSTRAINT above:
    the first-decl position (and with it the 2026-08-17/18 func_8003CF84
    "trailing does not generalize" line), the `pad` name, and the `u32`
    element type (replaced by the cited sibling array's own type).
    Everything else in this entry stands.
    1. **Frame forensics prove the bytes untouched.** The ledger lists
       every `$sp`-relative access in the target function (address, opcode,
       offset) and shows that no instruction touches the region. It gives
       the region's exact offset and size, and shows that it lies in the
       locals area, not the outgoing-argument, callee-save or reload spill
       area ([[phantom-slot-frame-lever]] instruments: the `.frame`
       gradient and the spill-slot order).
    2. **Completed siblings declare a real array there.** At least two
       COMPLETED-C functions in the same source file each declare, in
       committed source, a real array of the same element type and count
       as the (3) declaration. Each array sits at the same offset from the
       start of a local object whose leading members, over the size of the
       object the region follows, have that object's layout and are used
       the same way. Each is cited by file:line. At least one is a separate
       local declared immediately after that object, and that is the array
       (3) copies. The others may be separate locals or trailing members of
       the sibling's own struct. Each is real: the sibling's own code reads
       or writes it (line cited). A same-offset array of a different size,
       or one that is itself unused, FAKE-annotated or admitted under this
       family, is corroboration only and does not count toward the two.
    3. **Same array, same place.** The declaration copies the (2)
       separate-local sibling array's element type and element count
       exactly. It is a
       separate local declared immediately after the local object it
       follows; a member added to a struct type, a scalar, and any filler
       beside the array are outside this extension. The element type is one
       the engine's unused-array detector reads (`s8`, `u8`, `s16`, `u16`,
       `s32`, `u32`, `char`, `short`, `int`, `long`). With the array, the
       build's frame equals the target's (frame size, locals-region size and
       every `$sp` offset), and the array's bytes, rounded up to the
       compiler's stack-slot alignment, are exactly the (1) region.
    4. **Form: the sibling's declaration with `volatile` added.** A
       landing declares the cited sibling array's exact element type,
       count and identifier, with `volatile` added (e.g. `volatile s16
       digit[3];` after the object it follows), with no initializer, never
       referenced, and no `(void)` shims. This is the family's FORM
       CONSTRAINT with only the position, name and element type changed. The reason for
       `volatile`: it is the engine's sanctioned-pad marker, not a codegen
       device. The sandbox strips every never-referenced local array unless
       `_SANCTIONED_UNWRITTEN_PADS` has a row for it, and that allowlist
       admits only a `volatile`-qualified declaration (see (8)). The ledger
       banks that the same declaration without `volatile` compiles
       byte-identically (both listings or object hashes, with the command
       lines), so the qualifier is shown to do no codegen work.
    5. **An honest name.** The name is the cited sibling array's own
       identifier (e.g. `digit`), which states the role the siblings' array
       has. `pad`, `pre_pad`, `pad2`, `dummy`, `spill`, `slack`, `_unused`,
       `_tmp` and other coercion-announcing names are not admitted under
       this extension: the FORM CONSTRAINT's `volatile u32 pad[N]` is the
       family's example, and the engine allowlist keys on the exact name,
       not on `pad`.
    6. **Annotation.** A `/* FAKE: ... */` (or `// !FAKE:`) comment at the
       declaration says the array is unused here, cites the (2) sibling
       declarations by file:line, the ledger file holding the (1) census
       and the (7) measurements, and this ruling.
    7. **Honest producers measured inert first.** The ledger banks, with
       `sandbox --disable all` scores and the `.frame` gradient: the body
       without the array, all three [[phantom-slot-frame-lever]] producers,
       and every real local of the function that could occupy the region,
       each leaving the frame short of the target's.
    8. **Engine row and review.** The sandbox's
       `find_unused_local_arrays` flags every never-referenced local array,
       and `engine/volatile_cheats.py` `_SANCTIONED_UNWRITTEN_PADS` admits
       one only through an exact (function, name, element count) row with
       the declaration `volatile`-qualified. Each landing therefore adds
       its own per-function row, a detector-config change to that table
       only (no engine code change), reviewed by the landing's layer-2,
       citing this ruling, with `engine test` kept green; this ruling adds
       no row. A fresh layer-2 `cheat-reviewer` PASS walks
       (1)-(7) against the ledger; on the Grinder path a Judge PASS is not
       enough. Sandbox 0 and full-build SHA1 == oracle apply as always.

    Nothing is pre-decided: func_8005E54C (whose ledger names
    func_8005D814, func_8005E098 and func_8005F1C8 as siblings) and
    func_8005C8A8 are fresh submissions judged against (1)-(8). Record:
    docs/grind/decisions.md 2026-09-29 OWNER RULING — a trailing unused
    local array with sibling evidence.

Surveyed and NOT extended (2026-08-18, refusals stand): F1
constant→local→local staging chain (WEAK — genus shipped, species not),
F2 signedness-split dual read (WEAK), F4 cross-symbol arithmetic idiom
(ABSENT — SOTN's norm is the struct merge), F5 union-constructor CLOBBER
(ABSENT).

## Owner ruling 2026-09-30 — the D_800A37D2 / D_800A37D3 byte pair (thirtieth batch, Q63), conditional

**Question and answer** (record: docs/grind/owner-rulings-2026-09-26.md, thirtieth batch, Q63; filed question:
docs/grind/borderline.md 2026-09-30 "func_8001C8DC (+ func_8003CF84) — two scalar bytes indexed from the first
one's address (F4)"; evidence memory/grind/func_8001C8DC/evidence.md s1 and s1/). The operator recommended
"A narrowly, after one more check. [...] I'd have the struct measured first. If it also fails, admit A for this
pair only." The owner (Trenton) answered, verbatim: **"Go ahead with your recommendations."**

**Rule text.** The author's narrowing, not the owner's words. The F4 refusal above stands for every other
symbol pair. For the two bytes at 0x800A37D2 / 0x800A37D3 only:

1. **Condition precedent — the single-object forms are measured and fail.** Before anything spends this
   ruling, the ledger banks, with the scripts and command lines, a scratch full-build SHA1 and the listing of
   differing words for each (a whole-tree change: every consumer in five files moves together, so the
   per-function sandbox cannot score it): the one-array form (`u8 D_800A37D2[2]`; SHA1 687de142 and 42
   differing words, banked in evidence.md s1) AND a two-member struct form (one struct object at 0x800A37D2
   with a `u8` member per byte, every C consumer of either byte converted), each failing to reproduce the
   target. If any single-object declaration (array, struct, union) matches, this ruling lapses: that form is
   used and the aggregate-merge entry decides it.
2. **The admitted form.** Two adjacent scalar declarations (`u8 D_800A37D2;` / `u8 D_800A37D3;`, the second
   keeping its own symbol row), each byte reached by its own name wherever the target does, and the
   indexed access spelled from the first one's address (`p = &D_800A37D2; p[t != 0]++`,
   `(&D_800A37D2)[D_800A3748]`). Each indexed use carries a `/* FAKE: ... */` annotation naming this ruling
   and the ledger proof (the target reaches each byte by its own symbol, which a single object's constant-offset
   element access cannot produce). The pointer local `p` is a C-level pointer alias to a global, which the
   retro-audit FAIL named as a separate defect: its assignment carries its own `/* FAKE: ... */`, and it meets
   [[pointer-alias-fake-exception]]'s prerequisites on their own (exhaustion recorded — the index without `p`,
   s1/xsym.c, scores 19 — and the named mechanism).
3. **The functions.** func_8001C8DC lands with this form, sandbox 0, oracle SHA1 and its own fresh layer-2
   PASS. func_8003CF84's existing `(&D_800A37D2)[D_800A3748]` (COMPLETED, on main) is admitted with the same
   annotation in a `cheat-cleanup:` commit carrying its own layer-2; if condition 1 lapses, it is respelled to
   the matching single object instead.
4. **Nothing else.** No other symbol pair, and no other construct, may cite this ruling; every other F4
   occurrence stays refused and returns only through Q55's own route. Record: docs/grind/decisions.md
   2026-09-30 OWNER RULING — the D_800A37D2 / D_800A37D3 byte pair.

## Owner ruling 2026-09-30 — the practice-menu per-word reads (thirty-fourth batch, Q73), conditional

**Question and answer** (record: docs/grind/owner-rulings-2026-09-26.md, thirty-fourth batch, Q73: the question,
verbatim, and the options not chosen). Owner (Trenton) chose, verbatim: **"Allow narrowly, Q63-style
(Recommended)"**, whose text is: "Only for these 8 words in these 2 functions, and only if dumps prove every
single-object spelling fails for this compiler reason. Keep the separate per-word declarations, each annotated FAKE
with the evidence. Everything else about the table goes through the struct."

**Rule text.** The author's narrowing, not the owner's words. Prong (c)'s one-handle rule and the F4 refusal stand
for every other storage location and symbol. For these eight words of `g_practice_menu_table` only (two 1,100-byte
records): `D_80101FA0`, `D_80101FA8`, `D_801023EC`, `D_801023F4`, `D_80101FBC`, `D_80101FC4`, `D_80102408`,
`D_80102410`:

Definitions. "These reads" are only the reads the target makes of one of the eight words through that word's
own absolute `%hi`/`%lo` address in `func_8002BC68` or `func_8002BEA0`, each cited by instruction address in the
ledger. A spelling "reaches the target" when the function scores `sandbox --disable all` 0 and the full build's
SHA1 equals the oracle.

1. **Condition precedent — every single-object spelling fails, for the named reason.** Before anything spends
   this ruling, each function's ledger (`memory/grind/func_8002BC68/`, `memory/grind/func_8002BEA0/`) banks, for
   each of these minimum single-object spellings of these reads, its `sandbox --disable all` score and a dump
   (with command lines) showing that it misses for the reason the question put to the owner: "the compiler
   notices they share a base and reuses a register". Stated precisely: the single-object spelling reaches the
   words relative to one shared base (a constant symbol address or a base register), so the compiler reuses
   that base, while the target reaches each word through its own absolute `%hi`/`%lo` address. The forms and
   their dumps:
   (a) `g_practice_menu_table[k].field` directly: the `.cse` dump (cc1 `-ds`) showing the cause, cse relating
       the constant addresses as offsets of one symbol (`use_related_value`, `tools/gcc-2.7.2/cse.c`:1408-1429,
       1781), and also the final-addressing evidence every form needs (below);
   (b) the read through the typed base pointer, declared exactly `PracticeMenuRec *t2_base =
       g_practice_menu_table;` with `t3_base` likewise (`t2_base + 1`, or `g_practice_menu_table + 1`, matching
       the target), read as `t2_base->field` / `t3_base->field`, with every other `t2_base`/`t3_base` cast in
       the function converted to fields;
   (c) a per-record pointer local, `PracticeMenuRec *r = &g_practice_menu_table[k]; r->field`.
   Final addressing, for all three forms: the `.greg` dump (cc1 `-dg`) or the final `.s`, showing that the read
   at each cited target instruction address is still addressed off the shared base register where the target
   uses an absolute `%hi`/`%lo` address. A `.cse` dump alone does not show this: reload can still fold an
   unallocated pseudo with a `reg_equiv_constant` back into an absolute address after cse
   (`tools/gcc-2.7.2/reload1.c`:586, 2829-2834).
   A miss caused by anything other than this shared-base relation does not count toward this condition. Every
   single-object spelling a reviewer proposes is measured and banked the same way (owner ruling Q31's
   mechanism + search standard, as in [[ordinary-c-judge-decidable]] Ruling 11 (D)(3)): a banked spelling that
   misses is evidence, not a FAIL ground, and only a spelling that reaches the target defeats the ruling. If
   one does, the ruling lapses for that function and that spelling is used.
2. **The admitted form.** Each of the eight words keeps its own per-word declaration, overlapping the table, and
   is read by that name only at these reads. Each per-word declaration carries a
   `/* FAKE: ... */` annotation naming this ruling and the ledger file that holds the (1) evidence.
3. **Everything else goes through the struct.** Every other access to the table, in these two functions and in
   every other function, including writes to these eight words and any read of them that is not one of these
   reads, uses `g_practice_menu_table`. No function other than these two may newly read the per-word names;
   the existing readers convert under this route. Six landed C functions read them today: `func_8001E878`,
   `func_8001F888`, `func_8001C8DC`, `func_8001EA84` (`code6cac_tu2.c`), `func_8003C9A4` and
   `func_8003CE18` (`code6cac_c2.c`). The PracticeMenuRec cleanup series converts them to struct members, each
   with its own layer-2. A Q73 landing in `func_8002BC68`/`func_8002BEA0` does not certify them and does not
   wait for them. If any of them cannot convert byte-neutrally, that is a new owner question; this ruling does
   not extend to it. The asm `func_8002AB08` references `D_80101FBC`; its symbol row stays as an alias until
   that function is C.
4. **Nothing else.** No other word, function or construct may cite this ruling. The functions land with sandbox 0,
   oracle SHA1 and their own fresh layer-2 PASS. Record: docs/grind/decisions.md 2026-09-30 OWNER RULING — the
   practice-menu per-word reads (Q73).

## Owner ruling 2026-09-30 — SOTN precedent suffices (twenty-fourth batch, Q50)

Context (record): the orchestrator flagged a tension between Q48
(volatile locals admitted only with target-byte proof; "'SOTN does it'
isn't evidence") and the owner's same-day guidance "SOTN is the gold
standard when in doubt", and asked whether SOTN precedent should be
enough. Owner (Trenton), verbatim: **"Wait on a project wide sweep. But
SOTN precedent is good enough for any constructs if they verifiably
exist in the SOTN repo"**. (Record: docs/grind/owner-rulings-2026-09-26.md,
twenty-fourth batch, Q50, commit a68bde8ee.)

As recorded: a construct is admissible when it verifiably exists in the
SOTN decomp repository. This supersedes Q48's "SOTN does it isn't
evidence" clause; [[legitimate-volatile-interrupt-touched]] § volatile
locals now has a SOTN-precedent route beside the target-byte proof.

**"Verifiably exist" — the author's reading, as the record states it,
not the owner's words (flag to the owner if a case turns on it):**

1. **A PSX build file.** The citation is a file:line in the SOTN repo
   (`C:/Users/Trenton/Desktop/sotn-decomp`) in a file that is part of
   a PS1 build (`splat.us.*` / `splat.hd.*`) (correction to the author's reading, 2026-09-30:
   an earlier text said "PSX (GCC 2.7.2)"; SOTN's PS1 build uses
   `bin/cc1-psx-26`, and the owner never named a compiler):
   `config/splat.*.yaml` membership (PS1 configs
   are `splat.us.*` / `splat.hd.*`; `src/maria/` and `splat.pspeu.*`-only
   files are PSP, `src/saturn/` is SH-2; a `src/servant/` citation must sit
   outside any PSP version guard). In a file
   shared with other versions, the cited line is not inside a non-PSX
   version guard (memory reference/sotn-citation-requires-version-check).
   The author's reading, not the owner's words: every citation names the
   SOTN commit it was read at (the clone at
   `C:/Users/Trenton/Desktop/sotn-decomp` is at db41b28), and citations are
   C source constructs only, never SOTN build, toolchain, Makefile or
   assembler configuration. The author's reading, not the owner's words
   (flag to the owner): a line in a header (`include/**/*.h` or
   `src/**/*.h`: a macro, type, static inline or included function body)
   counts when it is cited together with a use site in a splat-member file
   (a `splat.us.*` / `splat.hd.*` member) whose preprocessed text includes
   it (directly or through other headers, as the preprocessor resolves it
   under that file's PS1 build defines), and **the use site itself** meets (2) and (4) (it
   applies the construct to the same kind of object — global, local or
   struct member — in the same way as ours). The construct is read as its
   expansion under the defines SOTN's PS1 build passes for that file. (4)
   and the Q52 markers are checked at both the definition and the use
   site: a definition compiled only under `HACKS` makes the construct
   SOTN-marked. An asm or `volatile` expansion owes that family's Q53
   paperwork.
2. **It does the same thing as ours when read,** not merely shares its
   spelling: the cited file is opened and the construct read in its
   context. A line that only matches a shape class in
   docs/reference/sotn-construct-index.md is not a citation (memory
   feedback/citation-check-reads-the-cited-code).
3. **Layer-2 checks the citation**: a fresh layer-2 `cheat-reviewer`
   verifies (1), (2) and (4) against the SOTN source itself. Author's narrowing, not the owner's words — flag to the owner: on the Grinder path a Judge PASS is not enough: a body relying on a Q50 citation lands only through the manual path's layer-2.
4. **Matched SOTN code** (owner ruling Q55, 2026-09-30, below). In the
   owner's words, the cited code "actually compiles to a match in its PS1
   build — not INCLUDE_ASM, not NON_MATCHING/disabled C, not an unmatched
   function". The author's reading, not the owner's words: the cited line
   is not inside an `INCLUDE_ASM` / `INCLUDE_RODATA` stub, not under
   `#ifdef NON_MATCHING` / `#ifndef NON_MATCHINGS`, not in any branch that
   the defines SOTN's PS1 build passes for that file
   (`tools/builds/gen.py:771` `cpp_defs` plus the per-file `$cpp_flags`,
   e.g. gen.py:453) leave disabled, and not
   in a function SOTN still carries as asm.

Q50 names only SOTN: psyz and every other decompilation project is still
not evidence for a construct. Author's narrowing, not the owner's words:
Every construct admitted on a SOTN citation carries an inline comment naming the citation (`/* SOTN: <file>:<line> @<commit> */`), so SOTN-derived constructs stay findable. The tag names no symbol, only the path, line and commit (see project memory comments-in-src-c-affect-queue-generation).

**Precedence (owner ruling Q55, 2026-09-30, twenty-seventh batch).** The
question put to the owner, verbatim: "Older rules still flatly refuse
specific tricks: the inline-assembly ban, 'volatile only on
interrupt-touched globals', never-executed calls, and constructs you said
'may not be re-proposed in any spelling'. Does a verified SOTN citation
override those too?" Owner (Trenton) chose, verbatim: **"Yes, matched SOTN
code only (Recommended)"**, whose text is: "Q50 wins over every older refusal, but the citation must be SOTN code that actually compiles to a match in its PS1 build — not INCLUDE_ASM, not NON_MATCHING/disabled C, not an unmatched function. Paperwork (FAKE comment, simpler spellings tried, second review) still applies." (Record:
docs/grind/owner-rulings-2026-09-26.md, twenty-seventh batch, commit
e7c8e3ef5.) Q50 is newer than every refusal in `.claude/rules/` and in the
`cheat-reviewer` brief. Where an older rule, owner ruling, catalog entry or
banned list refuses a construct (e.g. [[inline-asm-policy]]'s default ban,
[[legitimate-volatile-interrupt-touched]]'s two prongs, the
fabricated-dead-call refusal above, entries that "may not be re-proposed
in any spelling"), a citation meeting (1)-(4) still admits it, with Q53's
prerequisites. The older text governs every construct without such a
citation. Record: docs/grind/decisions.md 2026-09-30 OWNER RULING — Q50
precedence over older refusals, matched SOTN code only.

**Owner rulings 2026-09-30, twenty-fifth batch, Q51-Q53** (questions,
answers and the options not chosen verbatim in
docs/grind/owner-rulings-2026-09-26.md, twenty-fifth batch, commit
f7b29afeb). They settle how Q50 meets earlier rule text:

- **Q51 — reused variables.** Question: "Does your new 'SOTN precedent is
  good enough for any constructs' override that?" (the 2026-09-24
  decline of "a scratch variable reused exactly as SOTN's matched code
  reuses it", repeated in Rulings 9, 10 and 11). Owner chose **"Yes, Q50
  overrides it (Recommended)"**: "A variable reused exactly the way a
  PS1-build SOTN file reuses one is admitted on that citation. Reopens the
  question for e.g. func_8008B488's `rate` (SOTN's `var_a2`). Newer,
  explicit ruling wins." Author's narrowing, not the owner's words: such a variable is admitted on a citation meeting
  (1)-(4) above, instead of the Ruling 5-11 proof packages of
  [[ordinary-c-judge-decidable]]; the cited SOTN variable must be reused
  the same way when read (the same roles, written and read at
  corresponding statements).
- **Q52 — SOTN's self-marked fakes.** Question: "Do SOTN's own
  self-admitted fakes count as precedent?" Owner chose **"Yes, with the
  same FAKE marking (Recommended)"**: "Admitted when SOTN ships the same
  construct, but ours must carry the same `/* FAKE */` annotation SOTN
  uses, so it stays visible as a workaround." Author's narrowing, not the owner's words: a construct
  that SOTN marks as a hack or debt (any hack or debt signal SOTN attaches to the construct, in any form: a comment at or naming it anywhere in the file or header — `!FAKE`, `FAKE`, `fake`, `hack`, `HACK`, `TODO`, `FIXME` or similar; a label, identifier or macro name calling it a hack or fake, e.g. `goto hack;`, `CREATE_FACTORY_FAKE_ARGS`; or code compiled only under `HACKS` or a hack- or fake-named define; when a reviewer finds any such signal, ours carries `/* FAKE: … */`) (e.g. a local `(T*)&D_...` cast re-view, the Q52
  amendment to Q21 condition (2) above) counts as precedent under (1)-(4),
  and ours carries a `/* FAKE: ... */` annotation at the construct.
- **Q53 — family prerequisites still owed.** Question: "When a construct
  is admitted on SOTN precedent, does it still owe its family's usual
  paperwork (FAKE annotation where match-motivated, proof that simpler
  spellings were tried)?" Owner chose **"Yes, still owed
  (Recommended)"**: "SOTN precedent answers 'is this kind of construct
  allowed', not 'was it needed here'. Keeps workarounds visible and stops
  them being used where plain C would match." Author's narrowing, not the owner's words: a construct
  admitted on SOTN precedent still owes its family's usual paperwork:
  every annotation, exhaustion, byte-neutrality and review prerequisite of
  the family entry that governs it, but not that entry's test of whether
  the kind of construct is allowed, which the citation answers (e.g. a Q51
  reuse owes no Ruling 5-11 proof package). In any case it carries a
  `/* FAKE: ... */` annotation where it is match-motivated and has ledger
  proof that simpler spellings were tried (documented lever exhaustion; the simplest-known-form rule of
  [[ordinary-c-judge-decidable]] Ruling 1(4) applies).

The owner deferred the project-wide sweep of pre-2026-09-19 completions;
it is not started. Records: docs/grind/decisions.md 2026-09-30 OWNER
RULING — SOTN precedent suffices, and the Q51, Q52 and Q53 entries.

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
question to the owner. Exception, owner ruling 2026-09-30 (Q50): a construct with a verified SOTN citation ([[no-new-park-categories]] § Owner ruling 2026-09-30 — SOTN precedent suffices) is admissible on that citation, with Q53's prerequisites. Author's narrowing, not the owner's words (flag to the owner): it lands only through the manual path's layer-2, never on a Judge PASS; it is not a frozen-list extension; VS/ESA or other non-SOTN precedent, and any construct without a verified citation, keep the disposition above.

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
