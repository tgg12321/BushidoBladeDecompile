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
