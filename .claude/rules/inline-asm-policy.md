---
name: inline-asm-policy
paths: ["src/*.c"]
description: "Inline asm: CANONICAL (GTE/cop2, BIOS, HW pokes; verbatim PsyQ GTE macro islands under the 2026-09-23/26 rulings) is allowed; CHEAT asm (pins, hardcoded-$N injection, alias renames, barriers, GPR-opcode asm) is forbidden and keeps a function INCOMPLETE."
metadata:
  type: rules
---

# Inline-asm policy

| Category | What | Status |
|---|---|---|
| canonical-body | whole function as file-scope `__asm__("glabel ...")`; original was hand-written asm | COMPLETED-INLINE-ASM-CANONICAL, listed in `inline_asm_canonical.txt` |
| canonical | islands of opcodes with no C form: GTE/cop2 (`ctc2`/`mtc2`/`mfc2`/`cfc2`/`lwc2`/`swc2`/cop2 `.word`), BIOS vector jumps (`j 0xA0/B0/C0`), hardware pokes | COMPLETED-INLINE-ASM-CANONICAL when authorized |
| cheat | asm or pins that steer GCC's allocator/scheduler, or emit GPR instructions C could express | INCOMPLETE (sandbox strips it; `queue done` refuses) |
| none | pure C | COMPLETED-C (the goal) |

Only the `canonical` gate and an authorization route make asm finished
([[canonical-asm-authorization-recipe]], [[judge-sole-gate]] rule 3, the GTE classes below).
`INCLUDE_ASM` marks either queue work or an authorized canonical body; disambiguate via
`engine/queue.json` / `inline_asm_canonical.txt`.

## The cheat catalog (detected by `engine/volatile_cheats.py` / `engine/inlineasm.py`)

- **Register pins** `register T x asm("$N")`: diagnostic only, never committable
  ([[register-asm-pins]]).
- **Hardcoded-`$N` injection**: a single-instruction `__asm__("addu $8, $3, $zero")` with no
  `%N` operands is the regfix cheat moved into C; GCC never produced those bytes. Placeholder
  `move %0,%1` with pins is equally a cheat. If pins/injection is the only path, restructure
  the C or pursue canonical-asm evidence; never fall back to literal registers.
- **Alias renames**: `extern (volatile) T name asm("Y")` with name != Y (a second C handle to
  defeat CSE or force a width). Use one canonical declaration.
- **Volatile coercion** on game-state globals (`*(volatile T *)&D_x`, `extern volatile T D_x`)
  except [[legitimate-volatile-interrupt-touched]] and [[mmio-volatile-type-level]].
- **Scheduling barriers** (`__asm__("" ::: "memory")`), macro-hidden asm (`#define ... __asm__`),
  general-purpose opcodes (`move`, `addu`, `nop`, `lui`, `negu`, ...) in asm.
- Unused local arrays / dead param self-assigns: see [[dead-vars-local-array]] /
  [[dead-store-fake-exception]] (narrow FAKE carve-outs only).

A verified SOTN citation (Q50/Q55, [[no-new-park-categories]] § SOTN precedent suffices) can
admit a construct this default ban refuses, with Q53's prerequisites, on the manual path.

## GTE leaf wrappers (auto-authorize, 2026-05-26)

A leaf that is only cop2 ops plus MECHANICAL packaging (loads/stores feeding or draining GTE
registers, the GTE load-delay `nop`) has no C form: `canonical` routes it ASM-WHOLE →
`authorize`. Finished form: remove any pin (GCC returns `mfc2`'s result in `$v0` naturally),
keep the cop2 `.word`s and timing `nop`, `verify-oracle --rebuild`, add the
`inline_asm_canonical.txt` row, `queue done`. A leaf that also does general-purpose
COMPUTATION with a C form (e.g. summing `mfc2` outputs) is NOT in this class and needs the
normal evidence route.

## Owner ruling 2026-09-23 — verbatim PsyQ GTE macro islands

A C body may carry GTE islands and be COMPLETED-INLINE-ASM-CANONICAL on macro-provenance
evidence instead of a STRONG scan tier when ALL hold:
1. **Verbatim macro, pinned provenance.** Each island is one named GTE macro from a Sony PsyQ
   `inline_c.h` release, character-identical in instruction text, constraints and clobbers
   (only separators/whitespace may differ). Row and comment cite release, macro, line range;
   record the `$PSLibId`, source URL + commit, header SHA-256, and confirm against a second
   independent copy (one copy only ⇒ not admitted).
2. **Nothing else in the islands**: no preamble ([[cop2-addressing-preamble-cluster]]), no extra
   `nop`, no GPR literal beyond the macro's own, no pin feeding an operand.
3. **Inline and the only asm**: written out in `src/*.c` (no `#include`, BB2-local header or
   macro-by-name); everything else is ordinary C passing normal review.
4. **Bytes and hashes**: sandbox 0, full-build SHA1 == oracle, island hashes in
   `tools/canonical_asm_regions.json` (an edit voids the grant).
5. **Record and review**: `inline_asm_canonical.txt` row tagged
   `gcc-cannot-emit:gte_cop2_sdk_macro` citing this ruling, in its own `auth:` commit before the
   body; a `Pure-C attempts:` block (≥3 entries) on both commits; layer-2 checks the macro text
   against the pinned header.

Manual path only (the Grinder driver cannot apply it). Edited macros, look-alike hand-written
islands, non-GTE macros and GPR-only asm are not admitted. Record: docs/grind/decisions.md
2026-09-23 OWNER RULING — verbatim PsyQ GTE macro islands.

### Extension (owner, 2026-09-24) — DMPSX placeholder command words

Under the 2026-09-23 route only, an island still counts as verbatim when its single difference
is replacing the header's DMPSX placeholder `.word 0x0000xxxx` with a GTE command word, ALL of:
(A) only that operand changes, still spelled `.word 0x........`; (B) the word is DMPSX's word
for that placeholder per a source independent of BB2 (Sony doc/tool output, or a no-DMPSX SDK
spelling, cited with URL + commit) AND byte-identical to the original's instruction there; the
reviewer decodes every field (cmd 20-24, sf 19, mx 17-18, v 15-16, cv 13-14, lm 10, funct 0-5)
against the sources; (C) the island comment and the row give both words; (D) everything else
in the 2026-09-23 ruling applies. Example: `gte_rtv0()` `0x0000013f` → `0x4A486012`
(pcsx-redux/nugget@22037bd3 `psyq/include/inline_n.h:516-520`; PSn00bSDK@5d9aa2d3
`libpsn00b/include/inline_c.h:1183-1186`).

### Scorer ruling (owner, 2026-09-25) — header-exact GTE macro statements are scored as written

An engine bug-fix scope only: the sandbox's cheat-stripping keeps the GPR `move $12,%0` / `nop`
statements of a **qualifying macro unit**: (A) a contiguous run of inline `__asm__` statements
that is, statement for statement, the complete expansion of ONE named GTE macro from a pinned
header (`inline_c.h`, `inline_o.h`, `gtemac.h`), character-identical except whitespace and an
Extension-bounded placeholder word, recognised against the committed pinned excerpts
(`engine/gtemacro.py` `PINNED`); (B) the expansion contains a cop2 instruction (a standalone
`gte_nop()` is stripped); (C) recognition is by pinned header text, not by grant. Everything
else is stripped as before. **Scoring is not admission.**

#### Scorer amendment (owner, 2026-09-25, second batch) — `0(reg)` equals `(reg)`

The recognizer treats a memory operand `0(REG)` as equal to the header's `(REG)` (same
register). Nothing else is normalized (`4($12)`, `0x0($12)`, `00($12)`, `-0`, another register,
constraints, clobbers stay edits). Scoring only.

## Owner ruling 2026-09-26 — verbatim inline_o.h GTE macro blocks, granted as a class

Islands in PsyQ inline_o.h form need no per-function `tools/grinder/owner_cluster_grants.txt`
row and no STRONG tier when ALL hold: (A) the reference is PsyQ 4.3 `inline_o.h` (and
`gtemac.h`) exactly as `engine/gtemacro.py` `PINNED` records them (silent-hill-decomp@a1f407cb
`include/psyq/inline_o.h`, SHA-256 `76f28032…c6e47d`, confirmed against
xenogears-decomp@54d7ef3e; adding a macro's excerpt is an `engine:` commit with layer-2);
(B) each island is a qualifying macro unit, statement for statement and character for
character incl. the header's `move $12,%0` statements and `"$12","$13","$14","$15","memory"`
clobbers (only whitespace differs; joined statements are not covered); (C) NO respelling of
any kind: no DMPSX word swap, no `0($12)` for `($12)`, no other edit (those keep the
per-function owner-row route); (D) conditions 2-5 of § Owner ruling 2026-09-23 apply, and the
`canonical` gate's route is recorded; (E) layer-2 checks every island against `PINNED`. Manual
path until the driver learns the class.

### Per-function grants (owner rows for DMPSX command words only)

Named grants, no class widening; another function needs its own owner ruling. Each row admits
only the listed units, each otherwise class-exact, under the 2026-09-24 Extension's (A)-(C);
terms in docs/grind/decisions.md.

- **Per-function grant: func_8002DE20** (Q11, 2026-09-26): three `gte_rtv0()` units with
  `0x4A486012`; plus the byte-neutral maspsx `($REG)` empty-offset parser fix.
- **Per-function grant: func_800187F4** (Q29, 2026-09-28): `gte_rtv0tr` `0x4A480012`,
  `gte_sqr0` `0x4AA00428`, `gte_gpf0` `0x4B90003D`, `gte_gpl12` `0x4BA8003E`.
- **Per-function grants: func_8002D780, func_8002EBDC, func_8002F2D0, func_8002F770** (Q61,
  2026-09-30): their `gte_rtv0()` units with `0x4A486012`.

Related: [[register-asm-pins]] · [[canonical-asm-authorization-recipe]] ·
[[cop2-addressing-preamble-cluster]] · [[legitimate-volatile-interrupt-touched]] ·
[[no-new-park-categories]]
