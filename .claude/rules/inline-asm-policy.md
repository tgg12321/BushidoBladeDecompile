---
name: inline-asm-policy
paths: ["src/*.c"]
description: "Two-category inline-asm policy: CANONICAL (GTE/cop2/BIOS/HW) is authentic and fine; CHEAT (register pins, INLINE_MOVE_ALIASING, scheduling barriers) is forbidden — a function carrying any cheat-asm is INCOMPLETE."
metadata:
  type: rules
---

# Inline-asm policy

> **Historical framing note (2026-08-30):** this rule predates the removal of the
> regfix/asmfix rule system (retired at zero rules; machinery deleted). Where the
> symptom text says a function "carries a rule", read it as "the honest build shows
> this diff shape vs target". The technique itself is unchanged.

> **Policy ([[completion-standard]], 2026-05-21; expanded 2026-05-31):** a
> function is in exactly one of three categories:
>
>   * **INCOMPLETE** — in `engine/queue.json`. Carries a regfix/asmfix rule,
>     a cheat-asm pin/__asm__, OR a non-zero honest pure-C distance.
>   * **COMPLETED-C** — zero rules, zero cheat-asm in source, byte-matches.
>     Not in queue. Not in `inline_asm_canonical.txt`. The SOTN bar.
>   * **COMPLETED-INLINE-ASM-CANONICAL** — zero rules, has canonical inline
>     asm (GTE/cop2/BIOS/HW) or whole-body `__asm__("glabel ...")` as its
>     accepted finished form. Listed in `inline_asm_canonical.txt`.
>
> A function whose only path to byte-match is a register pin, inline-move,
> scheduling barrier, regfix rule, OR any of the codegen-coercion cheats in
> the **expanded cheat catalog** below is **INCOMPLETE** — not "done with
> a hint." There is **no attempts-log escape valve** to commit cheat-asm.
> Cheat-asm is the *work*, not the answer.
>
> **EXPANDED CHEAT CATALOG (2026-05-31).** The engine's `volatile_cheats`
> detector now also flags these patterns as cheat-asm debt — previously
> slipped through detection as "documented techniques":
>
> - **Alias renames** — `extern (volatile)? T name asm("Y")` with name != Y
>   (separate C handle for an existing global, used to defeat CSE / force
>   re-materialization). See [[inline-asm-injection]] § alias renames.
> - **Inline volatile casts on game-state globals** —
>   `*(volatile T *)&D_xxxxxxxx` (forces non-volatile globals to act
>   volatile for scheduling/CSE coercion). Game RAM globals were never
>   volatile in the original source.
> - **Plain `extern volatile T D_xxxxxxxx;`** — same coercion at declaration
>   level (was documented in [[split-read-defeats-hoist]] as a technique
>   until 2026-05-31 — now forbidden by default; see
>   [[legitimate-volatile-interrupt-touched]] for the SOTN-grounded narrow
>   carve-out applying ONLY to globals asynchronously mutated by an
>   identifiable IRQ handler at use sites that demonstrably require
>   CSE-defeat. The default ban is unchanged for every case OUTSIDE the
>   two-pronged criterion in that rule. **2026-07-01: for hardware
>   I/O-register addresses (0x1F801000-0x1F802FFF) volatile is now
>   TYPE-LEVEL hardware semantics — all shapes incl. single reads —
>   per [[mmio-volatile-type-level]]; the two-prong gate governs
>   game-state memory only.**)
> - **Unused fixed-size local arrays** — `s32 buf[N];` declared with no use,
>   to force GCC to reserve frame bytes. See [[dead-vars-local-array]] for
>   the deprecated rationalization.
> - **Dead self-assignments of function parameters** — `arg0 = 0;` where
>   `arg0` is a parameter never referenced afterward, used to break GCC's
>   value-association for register allocation. See [[register-alloc-pure-c]]
>   Lever D. **2026-07-01: narrow `/* FAKE */`-annotated last-resort
>   carve-out sanctioned — [[dead-store-fake-exception]]; un-annotated
>   instances remain forbidden and detector-flagged.**
> - **Macro-hidden `__asm__`** — `#define X ... __asm__(...) ...` macros
>   (e.g., `PAD_NOPS_*` in `code6cac_*.c`) that expand to inline asm at
>   every use site. The existing detector skipped `#define` lines; the new
>   detector catches them.
>
> Functions affected by any of these are INCOMPLETE after `queue regen` and
> must reach genuine pure-C COMPLETED-C OR canonical-asm authorization.
> The "documented technique" justification is no longer accepted.

When BB2's inline-asm policy is honest, every `__asm__` block in
`src/*.c` falls into one of the categories below:

| Category | What it is | Acceptable? |
|---|---|---|
| **canonical-body** | Full canonical-asm function body — original was hand-written asm, or the function emits via file-scope `__asm__("glabel ...")`. Listed in `inline_asm_canonical.txt`. | ✅ COMPLETED-INLINE-ASM-CANONICAL |
| **canonical** | Inline `__asm__` in a C function body using opcodes that ONLY EXIST in asm form: GTE coprocessor ops (`ctc2`/`mtc2`/`mfc2`/`lwc2`/`swc2`/`.word 0x4XXXXXXX`), BIOS vector jumps (`j 0xA0`/`B0`/`C0`), cache/DMA register pokes (`0x1F8003xx`). Authentic — original devs wrote these. | ✅ COMPLETED-INLINE-ASM-CANONICAL |
| **cheat** | Inline `__asm__` or `register T x asm("$N")` pin used to steer GCC's allocator or scheduler. General-purpose opcodes (`move`, `addu`, `nop`, `lui`, `negu`, etc.) that have C equivalents but we wrote them in asm to force matching. NOT in original source — they're workarounds for our `mips-gcc-2.7.2` fork diverging from the original `cc1psx`. | ❌ INCOMPLETE — **the BB2-specific gap** |
| **(no asm)** | Pure C, no inline asm. Matches byte-for-byte without hints. | ✅ COMPLETED-C — gold standard |

## Owner ruling 2026-09-23 — verbatim PsyQ GTE macro islands in ordinary C

Question put to the owner (manual session), verbatim: "func_800678A8 now
matches the original exactly. It is ordinary C except for two short snippets
that talk to the PS1's 3D math chip. That hardware has no C equivalent, and
both snippets are copied word-for-word from Sony's official SDK header. The
reviewer passed all the C. It blocked the commit only because nobody has
ruled that a function using plain Sony-header snippets like these may be
marked 'finished with approved inline asm'. Should I record that approval and
land it?"

Options, verbatim:
- "Approve (Recommended)": "Land a standing ruling: verbatim Sony SDK GTE
  header snippets in otherwise-plain C are approved. Then land func_800678A8
  with a fresh review. This also covers future functions in the same
  situation."
- "Approve this one only": "Grant func_800678A8 alone. Record it first as a
  separate rules commit, then land the match with a fresh review."
- "Don't approve": "Leave it as INCLUDE_ASM with the candidate saved in the
  ledger, and log the question to borderline.md."

Owner (Trenton) selected, verbatim: **"Approve (Recommended)"**.

Was the owner shown the LOW scan tier (2/8, S3+S4) or the
[[escalation-not-parked]] AUTO-REJECT clause? **No, the question did not
show them.** The owner ruled on the plain-language framing above.
Why this is still adequate: scan_hand_coded detects functions that were hand-written in asm as a whole. LOW is the expected result for C that calls SDK macros, so it does not bear on what the owner approved. The AUTO-REJECT test (no SOTN precedent) is not met, per the informed 2026-09-02 Condition 3 ruling.

Author's context (not shown to the owner): func_800678A8 scores 0 on
`sandbox --disable all` (283/283) and its full-build SHA1 matches the oracle.
The first layer-2 cheat-reviewer passed every C construct and the text of
both islands. It FAILed the commit on authorization alone:
`scan_hand_coded --single` rates the function LOW, there is no owner-cluster
registry row, and the `inline_asm_canonical.txt` row was a self-grant.

**What it admits.** A C function body may carry GTE inline-asm islands and be
classified COMPLETED-INLINE-ASM-CANONICAL on **macro-provenance evidence** in
place of a STRONG `scan_hand_coded` tier. Every one of these must hold:

1. **Verbatim macro, pinned provenance.** Each island is one named GTE macro
   from a Sony PsyQ `inline_c.h` (DMPSX) release. It matches that macro
   character for character in instruction text, operand constraints and
   clobber list. The only differences allowed are separators and whitespace
   (`;` vs `\n`, tabs vs spaces). The row and the source comment cite the
   header release, the macro name and its line range. Provenance is pinned:
   record the header's `$PSLibId` line, its source (URL and commit hash) and
   the SHA-256 of the header file. Then confirm the macro text, character for
   character, against a second, independent copy of the same header release
   (another project's vendored PsyQ headers). If only one copy can be found,
   the island is not admitted.
2. **Nothing else in the islands.** No instruction outside the macro text. That
   rules out an addressing preamble, a materialize-then-copy pair, an extra
   `nop`, and any GPR literal beyond the macro's own. The preamble class stays
   under [[cop2-addressing-preamble-cluster]] and its owner-enumerated
   registry. The operand seat is left to cc1: no register pin and no
   `register ... asm("$N")` feeding an operand.
3. **Inline, and the only asm.** The islands are written out inline in the
   function body in `src/*.c`, so the sandbox's cheat-stripping and the
   region gate both see them. Reaching the macros through a header
   (`#include`, a BB2-local `gte.h`, macro-by-name) is NOT admitted under
   this ruling. A header lets cheat-asm score as asm-free (Judge ruling,
   func_8002DAD0, decisions.md 2026-09-18). For this class, this overrides
   the "Preferred future form: a BB2-local GTE macro header" sentence in
   [[cop2-addressing-preamble-cluster]] § Condition 3 clarified. A header is
   not a route to admission under this ruling. Everything outside the islands is
   ordinary C that passes normal review on its own merits (family list,
   rename test, annotations). The islands are authorized as units and cover
   nothing around them.
4. **Bytes and hashes.** `sandbox --disable all` == 0, full-build SHA1 ==
   oracle, and island hashes are recorded in
   `tools/canonical_asm_regions.json`, so any later edit to an island voids
   the grant.
5. **Record and review.** The `inline_asm_canonical.txt` row carries the new
   tag value `gcc-cannot-emit:gte_cop2_sdk_macro`. The evidence-tag grammar in
   `tools/audit_asm_cheats.py` (`TAG_PATTERNS`) accepts it: `gcc-cannot-emit:[a-z][a-z0-9_]*`.
   The row cites this ruling. The row lands in its own `auth:` commit before the body, and the message of
   each of the two commits carries a `Pure-C attempts:` block with at least 3
   entries. This ruling requires both of these for every island. The audit
   enforces them only in part: in audit_asm_cheats, G2 validates the tag and
   G6 (`needs_attempt_log`) requires the attempts block on the row commit. On
   the body commit, G1 (`same_commit_self_auth`) and G6 fire only when an
   island contains an instruction outside the §6.1 whitelist
   (`_is_whitelisted_insn`: nop/cop2/ctc2/mtc2/mfc2/cfc2/lwc2/swc2/GTE
   `.word`), for example a `lw`/`sw`. For whitelisted-only macros the
   reviewer checks the split and the attempts block by hand. A layer-2 cheat-reviewer then checks the
   macro text against the pinned header.

**Relation to standing policy.** This is an owner exception, for this class
only, to [[judge-sole-gate]] rule 3 ("Without STRONG evidence, asm remains
refused") and to the [[escalation-not-parked]] AUTO-REJECT bullet on
overriding the canonical-asm evidence bar. It is not AUTO-REJECT class,
because SOTN-master precedent exists: SOTN `#include`s Sony's `inline_c.h`
and calls these GTE macros by name ([[cop2-addressing-preamble-cluster]]
§ Condition 3 clarified, owner ruling 2026-09-02). The LOW scan tier is
recorded as it stands; it is not re-scored. For all asm outside this class,
both standing texts apply unchanged. Hand-written islands that only resemble
a macro, edited macros (reordered loads, dropped clobbers, changed
constraints), non-GTE macros and GPR-only asm are not admitted. Whole-body
`glabel` asm still needs hand-coded-signal evidence
([[hand-coded-asm-recognition]]).

**Manual path only.** Letting the Grinder's driver (`grindlib.grant_canonical_asm`)
apply this route would be a separate `rules:`/`engine:` change. That change
needs its own owner ruling and layer-2 review, and this ruling does not
authorize it. Record: docs/grind/decisions.md 2026-09-23 OWNER RULING —
verbatim PsyQ GTE macro islands.

### Extension (owner, 2026-09-24) — DMPSX placeholder command words

Question put to the owner (manual session), verbatim: "func_80067200 matches
the original exactly. It is ordinary C plus five short snippets for the PS1's
3D math chip, copied from Sony's SDK header. Your 2026-09-23 ruling approved
such snippets only if they are word-for-word copies. Four of the five are.
The fifth isn't quite. Sony's header writes the 'rotate vector' command as a
placeholder number (0x0000013f). A separate Sony tool swapped that placeholder
for the real command (0x4A486012) after compiling. We don't have that tool, so
the snippet carries the real command, which is what the original game
contains. Should a snippet that differs from the header only by this
placeholder-to-real-command swap count as word-for-word?"

Options, verbatim:
- "Approve (Recommended)": "Record a standing ruling: under the 2026-09-23
  rule, swapping a Sony placeholder command word for the real post-tool
  command still counts as verbatim. Then land func_80067200 with a fresh
  adversarial review. This also covers future functions in the same
  situation."
- "Approve this one only": "Grant func_80067200 alone. Record it first as a
  separate rules commit, then land the match with a fresh review."
- "Don't approve": "Leave it as INCLUDE_ASM, save the matching candidate in
  the ledger, and log the question to borderline.md."

Owner (Trenton) selected, verbatim: **"Approve (Recommended)"**.

**Rule text.** This is the author's narrowing of that answer, not the owner's
words. It amends condition 1 above ("Verbatim macro, pinned provenance") in
one respect only. An island still counts as verbatim when its single
difference from the pinned header macro, beyond separators and whitespace, is
this substitution, and ALL of the following hold:

- **(A) Only the placeholder word changes.** The header macro's text contains
  a `.word` directive whose operand is a DMPSX placeholder (a word of the form
  `0x0000xxxx` that Sony's DMPSX post-processor rewrites into a cop2
  instruction after compilation, e.g. `gte_rtv0()`'s `.word 0x0000013f`,
  inline_c.h 4.3 :499-502). The island replaces ONLY that operand with a GTE
  command word (bits 31-25 = 0100101, COP2 with bit 25 set), still spelled
  `.word 0x........`. Every other
  character of the macro's instruction text, its operands, constraints and
  clobber list stays exactly as condition 1 requires (the macro's own `nop`s
  included, nothing added, nothing removed).
- **(B) The word is DMPSX's word for that placeholder, and the original's.**
  The substituted word is the command word Sony's DMPSX tool emits for that
  exact placeholder operand. This mapping is shown by a source independent of
  the BB2 binary: a Sony document or tool output, or a second project's
  real-command-word (no-DMPSX) spelling of the same macro (for example an SDK header that
  carries real command words), cited with URL and commit hash. The word is
  also byte-identical to the instruction at the corresponding position in the
  original binary (`asm/funcs/<func>.s`). The layer-2 reviewer decodes every
  field (sf bit 19, mx bits 17-18, v bits 15-16, cv bits 13-14, lm bit 10,
  funct bits 0-5, and the command-number bits 20-24) and confirms that all of
  them agree with the independent mapping. For `gte_rtv0()`: 0x0000013f ->
  0x4A486012 = MVMVA sf=1, mx=rotation, v=V0, cv=none, lm=0; independent
  sources: pcsx-redux/nugget@22037bd3 `psyq/include/inline_n.h` :516-520
  ("special version for Nugget (NO DMPSX)") and
  Lameguy64/PSn00bSDK@5d9aa2d3 `libpsn00b/include/inline_c.h` :1183-1186,
  both `"nop;" "nop;" "cop2 0x0486012;"`. If only the target shows the word,
  or any cited independent source disagrees in any field, the island is not
  admitted.
- **(C) Disclosed at the island.** The island's source comment names the
  macro and its header line range, states that the command word is the
  post-DMPSX word replacing the header's placeholder, and gives both words.
  The `inline_asm_canonical.txt` row says the same.
- **(D) Nothing else relaxes.** The rest of condition 1 and conditions 2-5
  of the 2026-09-23 ruling apply unchanged: pinned provenance with a second independent copy, nothing
  else in the islands, written inline in `src/*.c`, sandbox 0 + oracle SHA1 +
  region hashes, the `gcc-cannot-emit:gte_cop2_sdk_macro` tag, the separate
  `auth:` commit and the Pure-C attempts blocks, and the layer-2
  cheat-reviewer, who checks the header text AND decodes the substituted
  word. A word that differs from the original's bytes, a word substituted
  into a macro whose header text has no placeholder, or any other edit to a
  macro is not admitted. This Extension is the only exception to the
  2026-09-23 "edited macros ... are not admitted" sentence. The 2026-09-23
  "Relation to standing policy" and "Manual path only" paragraphs also apply
  unchanged; the Grinder driver may not apply this Extension.

Record: docs/grind/decisions.md 2026-09-24 OWNER RULING — DMPSX placeholder
command words.

### Scorer ruling (owner, 2026-09-25) — header-exact GTE macro statements are scored as written

**Question and answer.** After the 2026-09-25 manual-lane run, the owner asked
the operator for recommendations on four open questions in
docs/grind/borderline.md. This one is the 2026-09-25 entry "func_800288C8 —
scorer strips verbatim header GTE statements". func_800288C8's body writes
PsyQ `gte_Lzc` (gtemac.h) out as the six statements of its inline_o.h
expansion. Its full build matches the oracle, but `sandbox --disable all`
scores it 90. The reason: engine/inlineasm.py strips every `__asm__`
statement that has no cop2 instruction, which deletes the header's own two
`move $12,%0` statements and two `nop` statements before compiling. The
question put to the owner, verbatim: "Sony's header writes this chip snippet
as six lines. Our scorer deletes two `move` lines and two `nop` lines before
comparing. Should those header lines count when scoring?" The operator's
recommendation, verbatim:

> **Recommendation: treat it as a scorer bug and fix it in the engine.** The
> anti-cheat stripping is meant to remove injected assembly, not lines from
> Sony's own approved macros. The approval hashes already pin the exact text
> of each assembly block. So the scorer can recognise an approved block by its
> hash and score it whole, while still stripping anything unapproved. That
> keeps the protection against cheats and removes the false score of 90. The
> full build already matches the oracle.
>
> One thing to check first: func_80018300 reached a sandbox score of 0 using
> the same six-line header form, so the scorer may already handle it in some
> cases. The change needs `engine test` coverage.
>
> Fixing the scorer alone won't complete func_800288C8, because `tbl` failed
> separately on its merits. It would, however, stop the scorer from pushing
> future assembly blocks into the joined form that doesn't match.

Owner (Trenton), verbatim: **"Go ahead and do your recommendations then"**.

**Rule text.** This is the author's narrowing of that recommendation, not the
owner's words. It authorizes an ENGINE BUG-FIX to the sandbox's cheat-stripping
(`engine/inlineasm.py`), and nothing else. The stripping must not remove the
statements of a **qualifying macro unit**. Their GPR-only `move $12,%0` and
`nop` statements are kept and scored as written. A qualifying macro unit meets
ALL of (A)-(C):

- **(A) The whole verbatim expansion of one named macro.** It is a contiguous
  run of `__asm__` statements, written inline in `src/*.c`, with only
  whitespace and comments between them. Statement for statement, the run is
  the complete expansion of ONE named GTE macro from a pinned PsyQ header:
  `inline_c.h`, `inline_o.h`, or a `gtemac.h` macro, expanded through the
  header macros it calls. It has the same number of statements in the same
  order. Each statement matches the header character for character in
  instruction text, operand constraints and clobber list. The only
  differences allowed are separators and whitespace, and the DMPSX
  placeholder substitution where every prong of § Extension (2026-09-24) is
  met. The engine recognises a unit by comparing against a committed, pinned
  copy of the header text. Each macro entry records its header release
  (`$PSLibId` line), its source URL and commit, the header's SHA-256, its line
  range, and confirmation against a second, independent copy, as condition 1
  of the 2026-09-23 ruling requires. A partial unit, a reordered unit, a unit
  with an added or dropped statement, or a unit with any other edited
  character is NOT a qualifying unit. That includes an equivalent-encoding
  respelling such as `0($12)` for the header's `($12)`. Admitting such a
  respelling needs its own owner ruling. (One such ruling exists, for scoring only: the second-batch amendment below makes the recognizer treat `0(reg)` as equal to `(reg)` in a memory operand, and nothing else. It does not rule on admission.)
- **(B) It talks to the GTE.** The unit's expansion contains at least one
  GTE/cop2 instruction. A macro whose whole expansion is GPR-only or `nop`,
  such as a standalone `gte_nop()`, is not a qualifying unit on its own, and
  its statement is stripped as today. Its statement is kept only inside a
  larger qualifying unit that expands to it, such as `gte_Lzc`'s two
  `gte_nop()`.
- **(C) Recognition is by the pinned header text, not by grant.** The
  recommendation proposed recognising an approved block by its hash. The
  per-function region hashes in `tools/canonical_asm_regions.json` are
  written at grant time, and the grant itself requires sandbox 0 (condition 4
  above; cluster Check 1). Recognition by grant hash alone would therefore
  leave every not-yet-granted body unscoreable, including func_800288C8. The
  author's reading: the pinned header text in (A) is what those hashes pin,
  so the engine recognises units against it, whether or not the function is
  granted. The region hashes keep their existing job: an edit to a granted
  island voids the grant. This is the author's interpretation of the
  recommendation's mechanism.

**Everything else is stripped exactly as today.** That includes a
byte-identical `move $12,%0` or `nop` statement outside a qualifying unit,
register pins, `register` hints, and every other GPR-only `__asm__` statement.
This ruling admits nothing reached through `#include` or macro-by-name
(condition 3 above is unchanged).

**Scoring is not admission.** Keeping a unit's statements changes what the
sandbox measures. It authorizes no island. Every per-function requirement
stands unchanged:
- the region grant (hashes in `tools/canonical_asm_regions.json`);
- the `inline_asm_canonical.txt` row;
- the admission route. For inline_c.h islands that is the 2026-09-23 route
  above. For the inline_o.h class (and gtemac.h macros built on it) it is an
  owner-instructed row in `tools/grinder/owner_cluster_grants.txt`, which the
  layer-2 cited for func_800288C8. inline_o.h carriers have been approved one
  function at a time (func_80018300, func_8002CD58, func_8002DAD0).
  (Since 2026-09-26, islands that meet every prong of § Owner ruling
  2026-09-26 below need no per-function row; islands that do not still do.)

This ruling does not create that row for func_800288C8 or any other function,
and the owner's approval of the recommendation is not an instruction to add
one. func_800288C8 also failed separately on its `tbl` copy. (The
second-batch ruling below settles func_800288C8's row: none is granted now,
and it is granted when a body passes review.)

**Engine-change requirements** (all required before the fix lands):
1. **Check func_80018300 first.** func_80018300 reached sandbox 0 (307/307)
   with inline_o.h islands. Its grant row describes joined `move $12,%0` +
   cop2 statements, which the current stripper keeps whole as mixed blocks,
   and two bare `nop` (`gte_nop`) islands. Record which of its statements the
   current stripper keeps or strips, and why its score is 0 anyway. Limit the
   change to what that finding leaves unhandled.
2. **`engine test` pins both directions.** Positive cases:
   - a six-statement `gte_Lzc` expansion is kept whole;
   - a two-statement inline_o.h `gte_ldlzc` is kept whole.

   Negative cases, each stripped exactly as today:
   - a lone byte-identical `move $12,%0` with no following macro statement;
   - a unit with one character edited: a dropped clobber, a changed
     constraint, or `0($12)` (the second-batch amendment below moves
     `0($12)` to the positive cases and adds its own negatives);
   - a unit with an extra `nop` statement inserted or appended;
   - a reordered unit;
   - a unit split by an intervening C statement;
   - a standalone `gte_nop()` `nop`.
3. **Tree-wide distance comparison.** The change records sandbox distances
   before and after for every function. Any function whose distance changes
   must carry a qualifying unit.
4. **Review.** An `engine:` commit, with a layer-2 cheat-reviewer on the
   diff, because it changes cheat-stripping.

Record: docs/grind/decisions.md 2026-09-25 OWNER RULING — scorer:
header-exact GTE macro statements.

#### Scorer amendment (owner, 2026-09-25, second batch) — `0(reg)` equals `(reg)`

**Question and answer.** The first-batch record flagged a gap in (A).
func_800288C8's islands write the header's `swc2 $31,($12)` as
`swc2 $31,0($12)`, because maspsx cannot parse the bare form. Under (A) that
is an edit, so even the fixed scorer (de71fb41f, which pins `0($12)` as a
negative in `engine test`) strips the unit. After the first batch was carried
out, the operator reported three open decisions and the owner asked,
verbatim, "What are your recommendations?". The operator's recommendation on
func_800288C8, verbatim:

> ## 3. func_800288C8's approval entry: don't grant it yet
> **Recommendation: no per-function entry until a body passes review.** The
> body separately failed on `tbl`, a pointless copy, so an entry now would
> authorise a function that can't land.
>
> One narrow fix is worth making now. Our assembler (maspsx) can't parse the
> header's `($12)`, so it has to be written `0($12)`, which assembles to
> identical bytes. I'd let the macro recogniser treat `0(reg)` and `(reg)` as
> the same. It's a forced difference in assembler syntax, not an edit to the
> macro, and without it the verbatim-macro fix can never apply to the store
> macros. Every other part of the match stays exact. When func_800288C8 later
> has a passing body, grant its entry then.

Owner (Trenton), verbatim, answering all three recommendations together:
**"Go ahead with your recommendations"**.

**Rule text.** This is the author's narrowing of that recommendation, not the
owner's words. When the engine's unit recognizer (`engine/gtemacro.py`)
compares a statement with the pinned header text under (A), it treats a
memory operand written `0(REG)` as equal to one written `(REG)`, in either
direction, when REG is the header's register. The grounds are the
recommendation's: maspsx cannot parse the header's `($12)`, and the two
spellings assemble to identical bytes. **Nothing else is normalized.** Each of
these is still an edit: the run is not a qualifying unit and is stripped
exactly as today.
- **Any other offset.** `4($12)` for `($12)`, `($12)` or `0($12)` for a
  header `4($12)`, and every other spelling of zero: `0x0($12)`, `00($12)`,
  `-0($12)`, `+0($12)`. Only the single character `0` is equal to an empty
  offset. The zero-spelling list is the author's narrowing.
- **Any other register.** `0($13)` for `($12)`, or `$12` spelled any other
  way.
- **Any other edit.** Everything (A) lists stays an edit. The equivalence
  applies only to a memory operand inside an instruction's text. It never
  applies to operand constraints, clobber lists or operand expressions.

**Engine requirements** (an `engine:` commit with a layer-2 cheat-reviewer, as
for the scorer ruling):
- `engine test` moves `0($12)` for the header's `($12)` into the positive
  cases: a six-statement `gte_Lzc` whose `swc2` is written `0($12)` is kept
  whole.
- It pins each negative above: `4($12)`, `0x0($12)`, `00($12)`, `0($13)`, and
  a `0(...)` rewrite outside a memory operand.
- It records the tree-wide before/after distances, and every function whose
  distance changes must carry a unit that the equivalence completes.

**Scope.** This amends the recognizer only. "Scoring is not admission" holds
unchanged. The region hashes pin each island as written, and every admission
route keeps its own terms and its own reviewer.

**func_800288C8's row.** No per-function `tools/grinder/owner_cluster_grants.txt`
row is granted now. Per the approved recommendation ("When func_800288C8
later has a passing body, grant its entry then"), the row is granted when a
func_800288C8 body passes review. The author's reading of "passes review" is
that all of the following hold:
- `sandbox --disable all` is 0 under the fixed scorer;
- the full-build SHA1 matches the oracle;
- a fresh layer-2 cheat-reviewer PASSes every construct in the body and states
  that the missing row is the only outstanding item.

The operator then adds the row, citing this ruling, in its own commit before
the body lands. The body that lands is byte-identical to the body the layer-2 PASSed. No new owner question is needed. A body that still carries
the `tbl` copy, or anything else a reviewer FAILs, gets no row. Record:
docs/grind/decisions.md 2026-09-25 OWNER RULING — scorer amendment `0(reg)`
≡ `(reg)`, and 2026-09-25 OWNER RULING — func_800288C8 owner-cluster row.

## Owner ruling 2026-09-26 — verbatim inline_o.h GTE macro blocks, granted as a class

**Question and answer.** Filed question: docs/grind/borderline.md 2026-09-26
"func_8002DE20 — addendum: layer-2 FAIL; TWO owner items", item (2). The
question put to the owner, verbatim: "Grant GTE blocks copied verbatim from
PsyQ's inline_o.h (with its `move $12` setup step) as a class?" Owner
(Trenton) chose, verbatim: **"Grant as a class (Recommended)"**, whose text is:
"Character-identical to a pinned inline_o.h copy; no per-function owner row
needed." The framing: "grant this class, provided the block matches a pinned
header copy character for character."

**Rule text.** This is the author's narrowing of that answer, not the owner's
words. A C function body may carry GTE islands written in PsyQ inline_o.h
form and be classified COMPLETED-INLINE-ASM-CANONICAL WITHOUT a per-function
row in `tools/grinder/owner_cluster_grants.txt` and without a STRONG
`scan_hand_coded` tier, when every island meets ALL of (A)-(E):

- **(A) The pinned copy is the reference.** The reference text is PsyQ
  Run-time Library Release 4.3 `inline_o.h` ("Macro definitions of DMPSX
  version 3"), and `gtemac.h` for macros built on it, exactly as
  `engine/gtemacro.py` `PINNED` records them: source
  `github.com/shdecompilations/silent-hill-decomp@a1f407cb1ed0992997ace33a024e52b47001fdac`
  `include/psyq/inline_o.h`, SHA-256
  `76f28032e381a78a4c96347eeee753150cfb55b9f0f0be414fd5040bf4c6e47d`, confirmed
  byte-identical against a second, independent copy,
  `github.com/ladysilverberg/xenogears-decomp@54d7ef3e221578afc39d39f34fcd8c15ed83928c`
  `include/psyq/inline_o.h`; for `gtemac.h`, the same two projects'
  `include/psyq/gtemac.h`, SHA-256
  `9fe028fd2a187bed8147a67a2c98210b6f1663c2e05d2e36bb1243512c3357d5`, recorded
  the same way. A macro is in the pinned copy for this ruling only
  once its verbatim header lines are an entry in `PINNED`, with their line
  range and excerpt SHA-256, which `engine test` re-hashes. That is also the
  text the sandbox recognizes units against, so the admission reference and
  the scoring reference are one text. Adding a macro's lines to `PINNED` is an
  `engine:` commit, reviewed by a layer-2 cheat-reviewer who checks the
  excerpt against the pinned header. Other copies are not the reference.
  Xeeynamo/croc@f30ff1ee `include/psyq/inline_o.h` (the copy at
  `tmp/croc-ref/`, which the func_8002DE20 ledger and its 2026-09-26 layer-2
  treated as the only copy) is reformatted by a code formatter and carries an unexpanded
  `$PSLibId$`, so it is not character-identical to Sony's text.
  `tmp/libscan/psyq40/INCLUDE/INLINE_O.H` (Release 4.0, CRLF) is an
  unpinned local file.
- **(B) Character for character, statement for statement.** Each island is a
  qualifying macro unit as § Scorer ruling (owner, 2026-09-25) (A)-(B)
  defines it: a contiguous run of inline `__asm__ volatile` statements that
  is, statement for statement and in order, the complete expansion of ONE
  named macro in the pinned copy, with at least one cop2 instruction. Each
  statement matches its header statement character for character in
  instruction text, operand constraints and clobber list, including the
  header's own `move $12,%0` statements and every statement's
  `"$12","$13","$14","$15","memory"` clobbers. Only whitespace may differ.
  Joining several header statements into one `__asm__` statement (the form
  in which func_80018300, func_8002CD58 and func_8002DAD0 were approved one
  at a time) is not character-identical and is not covered by this class.
- **(C) No respelling of any kind.** The class admits no substitution. Each
  of these is not character-identical and is not admitted by this class:
  - **DMPSX placeholder command words.** An island that replaces a header
    `.word` DMPSX placeholder with the real command word (e.g. `gte_rtv0()`'s
    `.word 0x0000013f` as `.word 0x4A486012`) is not character-identical.
    § Extension (owner, 2026-09-24) scoped that swap to the 2026-09-23
    inline_c.h rule, and this ruling does not extend it.
  - **`0(REG)` for `(REG)`.** The § Scorer amendment's `0(REG)` ≡ `(REG)`
    equivalence is for scoring only; that amendment says it "does not rule
    on admission", and this ruling does not either. A macro with a header
    `($12)` memory operand (for example `gte_ldv0`, `lwc2 $0,($12)`) cannot
    be admitted under this ruling while maspsx cannot assemble `($12)`.
  - **Every other edit**: a dropped or added clobber, a changed constraint,
    a reordered, added or dropped statement, a partial expansion, any GPR
    instruction outside the macro text.

  Islands excluded here stay on the per-function owner-row route.
- **(D) Every other requirement stands.** Conditions 2-5 of § Owner ruling
  2026-09-23 apply unchanged, with this class in place of the inline_c.h
  route: nothing else in the islands and no register pins; written inline
  in `src/*.c` (no `#include`, no BB2-local header, no macro-by-name);
  `sandbox --disable all` == 0 and full-build SHA1 == oracle; island hashes in
  `tools/canonical_asm_regions.json`; the `inline_asm_canonical.txt` row,
  tagged `gcc-cannot-emit:gte_cop2_sdk_macro`, citing this ruling and the
  `PINNED` entries, in its own `auth:` commit before the body; a `Pure-C
  attempts:` block with at least 3 entries on each of the two commits. The
  engine's `canonical <func>` gate is run and its route recorded: each island
  sits inside a span the gate routes to asm by its cop2 signal. Each island's
  source comment names the macro and its pinned line range.
- **(E) Layer-2.** A fresh layer-2 `cheat-reviewer` checks every island
  against the pinned `PINNED` excerpt, statement for statement, and walks
  (A)-(D), as well as reviewing the C body.

**What this changes, and what it does not.** It removes only the
per-function owner row for islands that meet (A)-(E); those need no new
owner question. Islands outside the class (joined statements, DMPSX placeholder
substitutions, `0($12)` respellings, anything else in (C)) keep the existing route: an
owner-instructed row, as for func_80018300, func_8002CD58 and func_8002DAD0.
The rows already granted are unaffected. The Relation-to-standing-policy
paragraph of § Owner ruling 2026-09-23 applies to this class too: an owner
exception to [[judge-sole-gate]] rule 3 and to the [[escalation-not-parked]]
AUTO-REJECT bullet for this class only.

**Manual path, until the driver learns the class.** The Grinder driver
cannot apply this ruling today. Its PASS path (`tools/grinder/grind.ps1`,
owner Ruling C 2026-09-02) sends every island-carrying body through
`grindlib.grant_canonical_asm`, which admits only a STRONG scan tier or an
`owner_cluster_grants.txt` row, and otherwise refuses the merge. Teaching it
this class is a separate reviewed `engine:`/`rules:` change that this ruling
does not make. Until then the class applies on the manual path only. Record:
docs/grind/decisions.md 2026-09-26 OWNER RULING — inline_o.h GTE macro
blocks as a class.

**Second batch (owner, 2026-09-26): prong (C) kept strict.** The owner chose
"Keep strict wording" ("Only exact character copies; func_8002DE20's blocks
need a per-function grant."). Prong (C) stands exactly as written above.

### Per-function grant: func_8002DE20 (owner, 2026-09-26, fourth batch)

**Question and answer** (record: docs/grind/owner-rulings-2026-09-26.md,
batch 4, Q11). func_8002DE20's banked islands (commit 72c3b4d41) are written as
separate verbatim inline_o.h macro statements with two tool-forced deviations:
D1, `0($12)` for the header's `($12)` in 6 statements, forced by maspsx's
load/store parser (`tools/maspsx/maspsx/__init__.py` `parse_load_or_store`,
which requires a non-empty offset); and D2, `.word 0x4A486012` for the
header's `.word 0x0000013f` in 3 statements, the DMPSX post-pass substitution.
The question put to the owner, verbatim: "For func_8002DE20's GTE blocks: fix
our assembler shim so it accepts the header's exact `($12)` spelling (2-line
parser fix, all builds byte-identical), and grant a per-function row for the
one remaining difference — the 3 command words Sony's DMPSX tool would have
patched in?" Owner (Trenton) chose, verbatim: **"Fix parser + grant row
(Recommended)"**, whose text is: "Blocks become header-exact except the
DMPSX-patched word; per-function owner_cluster_grants row for that only. Plus
the engine recognizer update. Layer-2 reviews everything."

**This is a named per-function grant, not a class widening.** The class's
prong (C) ("No respelling of any kind") is unchanged for every other function.
No other function may cite this grant; another function with a placeholder
word needs its own owner ruling. What follows is the author's narrowing.

1. **The maspsx parser fix (a tool-fidelity fix).** maspsx's load/store parser
   is fixed to accept a memory operand with an empty offset, `($REG)`, exactly
   as the pinned header writes it. Sony's ASPSX 2.34 accepts the bare form:
   a calibration run through the real assembler (commit 4ef521cdd,
   `memory/grind/func_8002DE20/aspsx-paren-check/`) assembles `($12)` with 0
   errors to the same words as `0($12)` (c9800000, e9990000, c9810004).
   maspsx rejecting it is a gap in our shim, not a property of the original
   tools. The fix is bug-fix scope under [[no-compiler-divergence]] item 2 and must be BYTE-NEUTRAL: built
   once with the stock maspsx and once with the fixed one, using the
   Makefile's exact per-file recipe, every `src/*.c` object is byte-identical,
   and `verify-oracle --rebuild` and `engine test` are green. With the fix in
   place, func_8002DE20's islands spell every `($12)` exactly as the header
   does; D1 no longer exists, so no `0($12)` statement is admitted by this
   grant. The § Scorer amendment's scoring equivalence is unaffected.
2. **The owner-row scope: the DMPSX word only.** func_8002DE20's
   `tools/grinder/owner_cluster_grants.txt` row admits exactly its three
   `gte_rtv0()` units, each identical to the pinned header's expansion except
   that the `.word 0x0000013f` operand is `.word 0x4A486012`. Each of those
   three substitutions meets prongs (A)-(C) of § Extension (owner, 2026-09-24),
   whose word mapping (MVMVA sf=1, mx=rotation, v=V0, cv=none, lm=0) and
   independent sources are recorded there, and the word is byte-identical to
   the instruction at that position in `asm/funcs/func_8002DE20.s`. The row
   admits nothing else.
3. **Every other island is class-exact.** Every other island in the body is a
   qualifying unit under § Owner ruling 2026-09-26 prongs (A), (B) and (D):
   statement for statement and character for character (whitespace only)
   against the pinned `PINNED` excerpts. The engine recognizer update the
   owner approved (the pinned `engine/gtemacro.py` excerpts for the macros the
   body uses, and the Extension-bounded placeholder substitution that the
   2026-09-25 scorer ruling already allows for scoring) is an `engine:` commit
   with `engine test` positive and negative cases and its own layer-2.
4. **Everything else stands.** Conditions 2-5 of § Owner ruling 2026-09-23
   apply: inline in `src/*.c`, sandbox 0 and full-build SHA1 == oracle, region
   hashes in `tools/canonical_asm_regions.json`, the `inline_asm_canonical.txt`
   row in its own `auth:` commit with the Pure-C attempts blocks. A fresh
   layer-2 `cheat-reviewer` reviews the parser fix, the recognizer update, the
   row and the body. The body's C, including its shared cross-product
   variables, is judged under [[ordinary-c-judge-decidable]] (Ruling 11) on
   its own merits.

**Not in the rules commit.** The owner_cluster_grants.txt row, the maspsx
parser fix and the engine recognizer update land with the function, each
under the terms above, not in the commit that records this ruling. Record:
docs/grind/decisions.md 2026-09-26 OWNER RULING — func_8002DE20 per-function
GTE grant.

### Per-function grant: func_800187F4 (owner, 2026-09-28, sixteenth batch)

**Question and answer** (record: docs/grind/owner-rulings-2026-09-26.md, batch 16, Q29). func_800187F4's
islands are written as separate verbatim inline_o.h macro statements. Seven of them are units of four
command macros whose header text carries a DMPSX placeholder `.word`: `gte_rtv0tr()` (twice), `gte_sqr0()`
(twice), `gte_gpf0()` (once) and `gte_gpl12()` (twice). The question put to the owner, verbatim: "The same function uses four chip commands (a rotate, a square and two interpolation commands) that Sony's header writes as placeholder numbers; Sony's separate post-compile tool swapped in the real command numbers. We don't have that tool, so the snippets must carry the real numbers. Two independent SDK projects confirm each number, and they match the game's bytes exactly. You approved this same swap for func_8002DE20 as a one-function grant. Grant it here?"
Owner (Trenton) chose, verbatim: **"Grant, this function (Recommended)"**, whose text is: "A per-function approval row like func_8002DE20's, covering only these four commands; everything else in the snippets must be character-exact."

**This is a named per-function grant, not a class widening.** The class's prong (C) ("No respelling of any
kind") is unchanged for every other function. No other function may cite this grant; another function with
a placeholder word needs its own owner ruling. What follows is the author's narrowing.

1. **The owner-row scope: the four DMPSX words only.** func_800187F4's
   `tools/grinder/owner_cluster_grants.txt` row admits exactly those seven units, each identical to the
   pinned header's expansion (PsyQ Release 4.3 `inline_o.h`, the `engine/gtemacro.py` PINNED source and
   SHA-256 of § Owner ruling 2026-09-26 (A); `gte_rtv0tr` :451-455, `gte_sqr0` :646-650, `gte_gpf0`
   :721-725, `gte_gpl12` :726-730) except that the `.word` operand is the post-DMPSX command word:

   | macro | header placeholder | post-DMPSX word | fields (cmd bits 20-24, sf 19, mx 17-18, v 15-16, cv 13-14, lm 10, funct 0-5) | target |
   |---|---|---|---|---|
   | `gte_rtv0tr` | `0x0000027f` | `0x4A480012` | MVMVA: cmd 4, sf=1, mx=rotation, v=V0, cv=TR, lm=0, funct 0x12 | 0x800188B4, 0x80018A98 |
   | `gte_sqr0` | `0x00000f3f` | `0x4AA00428` | SQR: cmd 10, sf=0, lm=1, funct 0x28 | 0x80018DD4, 0x80018F4C |
   | `gte_gpf0` | `0x000012ff` | `0x4B90003D` | GPF: cmd 25, sf=0, lm=0, funct 0x3D | 0x80019034 |
   | `gte_gpl12` | `0x0000133f` | `0x4BA8003E` | GPL: cmd 26, sf=1, lm=0, funct 0x3E | 0x800190A8, 0x80019104 |

   Each substitution meets prongs (A)-(C) of § Extension (owner, 2026-09-24). Independent sources, each a
   no-DMPSX spelling `"nop;" "nop;" "cop2 IMM;"` whose word is 0x4A000000 | IMM:
   pcsx-redux/nugget@22037bd3 `psyq/include/inline_n.h` (`gte_rtv0tr` :546-550 `cop2 0x0480012`,
   `gte_sqr0` :780-784 `cop2 0x0A00428`, `gte_gpf0` :870-874 `cop2 0x0190003D`, `gte_gpl12` :876-880
   `cop2 0x01A8003E`) and Lameguy64/PSn00bSDK@5d9aa2d3 `libpsn00b/include/inline_c.h` (:1208-1211,
   :1404-1407, :1516-1519, :1521-1524, the same four words). Each word is byte-identical to the instruction
   at the listed position in `asm/funcs/func_800187F4.s`. The layer-2 reviewer decodes every field against
   both sources. The row admits nothing else.
2. **Every other island is class-exact.** Every other island in the body is a qualifying unit under § Owner
   ruling 2026-09-26 prongs (A), (B) and (D), statement for statement and character for character
   (whitespace only) against the pinned `PINNED` excerpts. The engine recognizer update (the pinned
   `engine/gtemacro.py` excerpts for the macros the body uses, and the four words in `DMPSX_WORDS`, the
   Extension-bounded scoring substitution that § Scorer ruling (owner, 2026-09-25) (A) already allows) is an
   `engine:` commit with `engine test` positive and negative cases and its own layer-2.
3. **Everything else stands.** Conditions 2-5 of § Owner ruling 2026-09-23 apply: inline in `src/*.c`,
   sandbox 0 and full-build SHA1 == oracle, region hashes in `tools/canonical_asm_regions.json`, the
   `inline_asm_canonical.txt` row in its own `auth:` commit with the Pure-C attempts blocks. A fresh layer-2
   `cheat-reviewer` reviews the recognizer update, the row and the body. The body's C is judged under
   [[ordinary-c-judge-decidable]] on its own merits (its reused locals under Ruling 11, the leading-zero-count
   input copy under Ruling 11 (C)(3)'s 2026-09-28 GTE-macro input copy clause).

**Not in the rules commit.** The owner_cluster_grants.txt row and the engine recognizer update land with
the function, each under the terms above, not in the commit that records this ruling. Record:
docs/grind/decisions.md 2026-09-28 OWNER RULING — func_800187F4 per-function GTE grant.

# Why this distinction matters

For a long time the BB2 project lumped canonical and cheat asm together as
"inline asm" and treated the whole category as suspect. But:

- The original PSX devs *did* write inline asm — for GTE ops, BIOS
  trampolines, hardware register pokes. That's canonical.
- They almost never wrote inline asm for general-purpose register
  allocation or scheduling control. That's cheat-asm, and it's BB2's gap
  from the SOTN community standard.

> **Superseded 2026-08-19 ([[asm-until-matched]]):** an INCOMPLETE function
> IS committed as `INCLUDE_ASM("asm/funcs", <func>)` — the honest floor
> lives in `memory/grind/<func>/migration_pin.json`, the retired chassis in
> `retired-chassis-2026-08/`. Cheat-asm on main is now (near-)zero by
> construction, not a tracked debt metric; only the byte-coupling deferred
> set still carries legacy cheats. `INCLUDE_ASM` is therefore ambiguous at
> the source level: it marks EITHER not-yet-decompiled work (queue item) OR
> a COMPLETED-INLINE-ASM-CANONICAL body (listed in
> `inline_asm_canonical.txt`) — disambiguate via `engine/queue.json` /
> `inline_asm_canonical.txt`, never by the token alone.

# Concrete examples in BB2

Canonical (authentic):
```c
__asm__ volatile ("ctc2 %0, $0" :: "r"(t5));        // GTE control reg load
__asm__ volatile (".word 0x4A486012" :: ...);       // mvmva GTE op
__asm__ volatile ("sw %0, 0x1F800360" :: "r"(v0));  // scratchpad poke
```

Cheat (toolchain workaround — forbidden):
```c
register s32 cached asm("$16");                            // allocation hint
__asm__ volatile("move %0, %1" : "=r"(dst) : "r"(src));    // INLINE_MOVE_ALIASING
__asm__ volatile("" ::: "memory");                          // scheduling barrier
```

# How categories are surfaced

`python3 tools/classify_inline_asm.py` walks `src/*.c`, classifies every
`__asm__` block and every `register T x asm("$N")` pin, and emits:

```
=== inline-asm classification ===
  canonical (authentic GTE/BIOS/HW): N instances across X pure-canonical funcs
  cheat (toolchain workaround):  M instances (A __asm__ blocks + B register-asm pins) across Y pure-cheat funcs
  mixed (both categories in one func): Z funcs

  GAP TO SOTN BAR:
    (Y + Z) functions use cheat inline asm
    M total cheat instances to retire
```

# Route to SOTN bar

**Measured 2026-05-18 (commit `32b2da9`):** on a 16-function sample of
the cheat-asm corpus, **0 functions are COMPILER_FIXABLE.** Even Sony's
actual `cc1psx` fails identically to decompals on the pure-C
reconstructions. The compiler-patch route does not work; see
[[compiler-patch-low-roi]] for the data + methodology and
`docs/diagnostics/cc1psx_cheatasm_diagnostic_16funcs.csv` for the raw
results.

The only path with measured non-zero ROI for cheat-asm retirement:

**Per-function pure-C re-attempt.** For each cheat-asm function, drive it
to COMPLETED-C (pure C). A successful retirement removes the function's
cheat-asm and rules. There is **no "declare it needs hints and commit"
outcome** — a function that won't reach pure C stays INCOMPLETE; you
keep switching technique (escalation-ladder (archived)). The only non-pure-C
finish is canonical-asm authorization (COMPLETED-INLINE-ASM-CANONICAL)
for a construct proven physically un-compilable — never cheat-asm.
(Attempt logging in `.bb2_attempts/` is still useful as a record of what
was tried, but it no longer unlocks a cheat-asm commit — see
attempts-log-gate (archived).)

Secondary path: **reclassify cheat-asm functions whose inline asm is
load-bearing.** The diagnostic surfaced 5 functions where stripping the
inline asm broke the C body — strong evidence the asm wasn't a workaround
hint, it was authentic computation. Those belong in the canonical
category, not cheat. An audit pass could shrink the cheat-asm count
without changing any source code.

# Related

- [[inline-move-aliasing]] — the most common cheat-asm pattern
- [[register-asm-pins]] — pin reliability + when they're cheats
- attempts-log-gate (archived) — gate enforcement for new cheat-asm commits
- [[compiler-patch-low-roi]] — 0/16 measured ROI for the compiler-
  patch route (so it's not on the table)
- [[cc1psx-calibration-only]] — prior project decision to not switch
  the build to cc1psx
- [[community-standard]] — what SOTN/Vagrant/etc accept
