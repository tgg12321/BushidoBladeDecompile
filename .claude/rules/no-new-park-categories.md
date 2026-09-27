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
    decisions.md:1833, and the 2026-08-30 func_80045878 `c` FAIL stand;
    a fresh local written more than once is admitted ONLY if it meets
    every prong of [[ordinary-c-judge-decidable]] Ruling 5 (2026-09-23),
    as amended for identical-text writes by its 2026-09-23 extension, or
    every prong of its Ruling 6 (2026-09-23), or, for vmNoiseOn's `temp`
    alone, every prong of its Ruling 8 (2026-09-24), or every prong of its
    Ruling 9 (2026-09-25, one meaning at several constant offsets) or of its
    Ruling 10 (2026-09-25, the verified original source's own variable,
    verbatim), or, when none of those admits it, every prong of its
    Ruling 11 (2026-09-26, a reused local proven necessary by allocator
    dumps, with an honest generic name and layer-2); whichever ruling
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
    (a)-(d) in [[maspsx-gate-lists]]. A merge that depends on that gate is
    admitted only when its rows meet those prongs; this amendment admits no
    gate row. Record:
    docs/grind/decisions.md 2026-09-26 OWNER RULING — aggregate-merge prong
    (a): compiler-necessity evidence, minimal span.
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
