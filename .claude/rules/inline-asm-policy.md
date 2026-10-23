---
name: inline-asm-policy
paths: ["src/**/*.c"]
description: "Inline asm: CANONICAL (GTE/cop2 as calls to the vendored PsyQ macros, BIOS, HW pokes) is allowed; CHEAT asm (pins, hardcoded-$N injection, alias renames, barriers, GPR-opcode asm) is forbidden and keeps a function INCOMPLETE."
metadata:
  type: rules
  tier: blocking
---

# Inline-asm policy

- **canonical-body**: whole function as file-scope `__asm__("glabel ...")`, original was
  hand-written asm; **canonical**: islands of opcodes with no C form (GTE/cop2, BIOS vector
  jumps `j 0xA0/B0/C0`, hardware pokes). Both are COMPLETED-INLINE-ASM-CANONICAL only when the
  `canonical` gate and an authorization route allow it ([[canonical-asm-authorization-recipe]],
  [[judge-sole-gate]] rules 3 and 3a, the GTE macro calls below; listed in `inline_asm_canonical.txt`).
- **cheat**: anything steering GCC's allocator/scheduler or emitting GPR instructions C could
  express. INCOMPLETE: the sandbox strips it and `queue done` refuses.

## The cheat catalog (`engine/volatile_cheats.py`, `engine/inlineasm.py`)

- **Register pins** `register T x asm("$N")`: diagnostic only ([[register-asm-pins]]).
- **Hardcoded-`$N` injection** (`__asm__("addu $8, $3, $zero")`, no `%N`): the regfix cheat
  moved into C. Placeholder `move %0,%1` + pins is equally a cheat. Restructure the C instead.
- **Alias renames** `extern T name asm("Y")` (name != Y): a second C handle to one object.
- **Volatile coercion** on game-state globals, except [[legitimate-volatile-interrupt-touched]]
  and [[mmio-volatile-type-level]].
- **Scheduling barriers** (`__asm__("" ::: "memory")`), macro-hidden asm, GPR opcodes in asm.
- **Token pasting** (`##` / `%:%:`) in a src TU: it can build a macro or asm name the
  checks never see; the engine refuses it.

A verified SOTN citation ([[sotn-precedent-suffices]], Q55) can admit a construct this ban
refuses, with Q53's prerequisites, on the manual path.

## GTE leaf wrappers (auto-authorize, 2026-05-26)

A leaf of only cop2 ops plus MECHANICAL packaging (loads/stores feeding or draining GTE
registers, the load-delay `nop`) has no C form; `canonical` routes it to `authorize`. Remove
any pin, keep the cop2 `.word`s and timing `nop`, `verify-oracle --rebuild`, add the row,
`queue done`. A leaf that also COMPUTES something C can express needs the normal evidence route.

## PsyQ GTE macro calls (owner direction 2026-10-08, rulings 2026-10-15)

GTE code in a C body is CALLS to the macros of Sony's PsyQ 4.0 headers, vendored whole in
`include/psxsdk/` (`inline_o.h`, `inline_c.h`, `gtemac.h`; provenance and pins in
`engine/gtemacro.py` `VENDORED`). The function is COMPLETED-INLINE-ASM-CANONICAL (row tag
`gcc-cannot-emit:gte_psyq_macro_call`) when ALL hold:
1. **Vendored, never edited.** Each file is Sony's text; the only differences are LF line
   ends and DMPSX's command word for the 11 owner-approved placeholders in `DMPSX_WORDS`.
   Every other placeholder stays Sony's, and calls to its macros are refused.
   `check_vendored()` re-hashes to Sony's SHA-256. A new word or header is an `engine:` commit
   with layer-2; a new DMPSX word, or a new vendored header whose macros carry a trap
   (`break` / `syscall`), also needs an owner ruling.
2. **One inline header per TU** (`inline_o.h` or `inline_c.h`; `gtemac.h` with either),
   included by the `.c` file. No other header may define asm macros (`bios.h`,
   `include_asm.h` and the vendored `libsn.h` excepted); a same-file asm `#define` or a redefinition of a vendored name
   is a cheat.
3. **Calls, not copies.** Each GTE operation is a statement-level call by name. No `__asm__`
   spelling of a header macro (written-out bodies, joined statements, edited clobbers, hand
   command words) and no standalone call of a macro without a cop2 or trap instruction
   (`gte_nop()`).
   (Q126) The call may sit in a `static inline` helper defined in the same TU; the engine
   attributes a called helper's calls and tokens to each caller (region hashes, layer-2 key,
   integrity). Until it does, such a body cannot land.
4. **Arguments are C**, reviewed as such; Q115 governs respelling them.
5. **Bytes and record.** Sandbox 0, full-build SHA1 == oracle; call-site hashes in
   `tools/canonical_asm_regions.json` (`engine/completion.py region_hashes`; an argument or
   header switch voids them); layer-2 checks every call against the TU's header; the usual
   cheat-reviewer PASS. A class grant: no per-function owner row. Grinder-applicable once the
   driver's grant door and scope learn the class; until then the manual or batch lane applies it.

**Other vendored PsyQ macros.** SN's LIBSN.H is vendored whole as `include/psxsdk/libsn.h`
under rule 1: `pollhost()` / `PSYQpause()` expand to `break 1024` / `break 1031`, a trap
with a code field C cannot emit. Rules 1 and 3-5 apply; libsn.h needs no inline header but
the `.c` file includes it directly, and its calls are keyed `libsn.h:` in the region hash.
Admission is `gtemacro.Macro.admitted` (a cop2 or a trap instruction in the expansion).
Row tag `gcc-cannot-emit:psyq_pollhost_break`.

**Macro choice** where two expand identically: the macro named for the operation the code
performs (a `gtemac.h` composite when the code does that whole operation, e.g. `gte_Lzc`);
a ruling that names a macro wins (Q29: `gte_rtv0tr` in func_800187F4). `tools/gte_macro_rewrite.py`
lists the alternatives; the reviewer decides.

The written-out island routes (rulings 2026-09-23/24/26, the Q11/Q29/Q61/Q92/Q93 rows) admit
nothing new: their functions are converted to calls. "Admit nothing new" is reviewer-enforced
until `completion.REFUSE_WRITTEN_OUT_UNITS` is turned on. Until the last is converted the
sandbox still scores a qualifying written-out unit (scorer ruling 2026-09-25,
`gtemacro.unit_spans`).

## Owner ruling Q115 (2026-10-06) — typed operand expressions in authorized islands

Typing work may respell the ARGUMENTS of a GTE macro call (or the operand expressions of another
authorized island) as the member form of the same address (`obj + 0xA8` -> `&rec->unkA8`) when ALL hold:
(A) macro, template text, constraints, clobbers and operand order stay unchanged; (B) each new
expression denotes the same address (the reviewer checks every offset against the type); (C) sandbox 0
and full-build SHA1 == oracle; (D) the function's `tools/canonical_asm_regions.json` hashes are recomputed
(`engine/completion.py region_hashes`) in the same commit and layer-2 diffs every island old -> new. No alias
local kept only for the asm; no operand moved into or out of the asm. Template edits stay refused.

Related: [[cop2-addressing-preamble-cluster]] · [[no-new-park-categories]]
