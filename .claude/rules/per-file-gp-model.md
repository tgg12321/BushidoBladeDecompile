---
name: per-file-gp-model
paths: ["sdata_*.txt", "Makefile", "engine/buildconfig.py", "engine/pipeline.py", "tools/maspsx/**", "src/*.c", "bb2.ld"]
description: "Owner ruling 2026-09-30 (Q65, 'Adopt fully'): gp-relative addressing comes from Sony ASPSX 2.34's per-file rule (a file gets gp for a symbol of 8 bytes or less only if it defines it; .comm at base only, .lcomm/.sdata at every offset, extern never), replacing sdata_syms/sdata_funcs/sdata_exclude with maspsx -G8. Definitions and file boundaries come from evidence only; never a definition to flip one access. Lands as byte-identical, layer-2-reviewed commits."
metadata:
  type: rule
---

# Per-file gp model (owner ruling 2026-09-30, thirty-first batch, Q65)

Owner (Trenton) chose, verbatim: **"Adopt fully (Recommended)"**, whose text is: "Replace all 3 lists with
Sony's per-file rule. It lands as a series of separately reviewed, byte-identical commits after the current
landings finish. It's the most faithful option: no per-function lists remain, and gp use comes from where the C
defines each variable, as in the original source." The question and the other options are verbatim in
docs/grind/owner-rulings-2026-09-26.md, thirty-first batch. Evidence: `docs/grind/gp-model-2026-09-30.md`, a
banked copy of `tmp/q56/MODEL.md` (`tmp/` is gitignored; the scripts and probe outputs it names stay untracked
in `tmp/q56/`). Record: docs/grind/decisions.md 2026-09-30 OWNER RULING — the per-file gp model.

Everything below the owner's words is the author's narrowing, not the owner's words.

**Amendment 2026-09-30 (evidence addendum; owner rulings Q67 and Q68, thirty-second batch).** The clauses
marked (A1) and (A2) below rest on owner ruling Q67, and (A3) on owner ruling Q68 (both verbatim in
docs/grind/owner-rulings-2026-09-26.md, thirty-second batch). Q67: the owner chose **"Yes, follow the evidence
(Recommended)"**: "Adopt the full evidence-based layout: ~180 statics, 4 file merges in total, and exact-size
filler variables where the original had unused data. The source ends up matching how the original files were
really organized." Q68: the owner chose **"Allow, as a model (Recommended)"**: "Treat it like -msoft-float and
-mel: a global change that reproduces the original compiler's measured behaviour, proven byte-identical, with
an engine test. It completes the adoption with no lists left." Evidence: `docs/grind/gp-model-2026-09-30.md`
§ Addendum 2026-09-30 (the Sony PSYLINK 2.37 layout probes, the BB2 bss survey and the cc1psx section-choice
calibration). The conditions below are the author's narrowing, not the owner's words.

**Amendment 2026-09-30 (owner rulings Q69-Q72, thirty-third batch).** The clauses marked (A4), (A5), (A6) and
(A7) below rest on owner rulings Q69, Q70, Q71 and Q72 (verbatim in docs/grind/owner-rulings-2026-09-26.md,
thirty-third batch). Q69: the owner chose **"Everywhere, Sony libs off (Recommended)"**: "On for every file,
like a project-wide build flag. Sony's library code stays off, as it was compiled originally. Only one library
file (libcd's system.c) is measured to actually need this. No per-file list: one rule plus the library
exception." Q70: **"Accept all three (Recommended)"**: "Keep existing types where nothing contradicts them, use
the proven struct for D_800153F0, and defer the func_8004153C tidy-up. Nothing new is claimed." Q71:
**"Smallest aligned pieces (Recommended)"**: "Split each run into the fewest aligned filler variables (named
D_<addr>), as the series does now and proven byte-identical. Later, code6cac_c_mid's repeating 4-byte pattern
may be re-expressed as one array of small records, with its own proof." Q72: **"Include it (Recommended)"**:
"Clarify that a data-only file lying between members of an approved group joins that group. Its bytes are
already in the right place; this just puts them in the right source file." The conditions below are the
author's narrowing, not the owner's words.

## The rule (Sony ASPSX 2.34, measured; evidence doc § 1)

For a load or store in file F naming symbol S directly (`S` or `S+k`, no base register):

| F's own declaration of S | gp-relative? |
|---|---|
| none, or `extern` | never |
| `.comm` (a tentative definition), S of 8 bytes or less | at the base (`S+0`) only; `S+k`, k ≠ 0, is `lui`/`%lo` (Q62) |
| `.lcomm` (an uninitialized `static`), 8 bytes or less | at every offset |
| `.sdata` (an initialized definition), 8 bytes or less | at every offset |
| any definition of S larger than 8 bytes | never |

An indexed operand `S($reg)` is never gp (it expands through `$at`). The decision is per FILE: every function in
F gets the same answer for S. The evidence doc checked this against the real ASPSX on 6,337 accesses with 0
disagreements, and its negative controls show that with no definitions ASPSX gives gp to none of them.

## What the adoption changes

1. **maspsx, two global fixes and one global section choice (A3).** `_uses_gp` (`tools/maspsx/maspsx/__init__.py`) returns False for an operand
   with a base register (the indexed-operand fix, evidence doc § 3), and `.local` + `.comm` is modelled as
   `.lcomm` (today it raises, `__init__.py` "uninitialized static ... is not modelled"). Both are global: no
   list, no function or symbol name.
   **(A3) Section choice, modelling cc1psx (owner ruling Q68).** When maspsx runs with its `-G8` switch, and
   only then, an initialized object that our cc1 emits into `.data` with a total size of 8 bytes or less is
   emitted into `.sdata` instead. The calibration (evidence doc § Addendum A.3) shows cc1psx at `-G8` placing
   such objects in `.sdata` while our cc1 places them in `.data` at `-G0` and at `-G8`: the upstream fork
   switched that choice off in `tools/gcc-2.7.2` commit feeaecf "Fix sdata issues"
   (`config/mips/mips.c`:5471-5483, the commented-out `SMALL_DATA_SECTION ()` branches). This is a new GLOBAL
   maspsx behaviour change; it is admitted by Q68 as the owner-policy sign-off that
   `.claude/rules/no-compiler-divergence.md`:95-98 requires, and cc1 itself stays unpatched. The object is a label in `.data`
   followed by data directives up to the next label or section change; its size is the sum of those
   directives. The calibration covers global objects with zero and nonzero initializers; a `static`
   initialized object is moved only if the landing commit adds a cc1psx calibration showing cc1psx places it
   in `.sdata` too. The change is global and names no symbol, function or file. Its landing commit is
   byte-proven (full-build SHA1 == oracle, and every `src/*.c` object built both ways and compared) and adds
   an `engine test` case covering it: under `-G8` an initialized object of 8 bytes or less moves to `.sdata`,
   and an object larger than 8 bytes, or any object without `-G8`, stays in `.data`.
2. **Flags.** In the Makefile's `MASPSX_FLAGS` and `MASPSX_FLAGS_GP`, `--sdata-syms`, `--sdata-funcs` and
   `--sdata-exclude` are removed and maspsx's own `-G8` switch is added; `engine/buildconfig.py` mirrors the
   Makefile verbatim. `sdata_syms.txt`, `sdata_funcs.txt` and `sdata_exclude.txt` are deleted. This `-G8` is
   the maspsx option (`tools/maspsx/maspsx.py`, `-G<n>`). cc1's per-file `-G8` (`GP_FILES`) is a different
   switch and stays governed by [[compiler-flags-canonical]], unchanged.
   **(A4) Which files get maspsx `-G8` (owner ruling Q69).** maspsx's `-G8` is on for every C file except Sony
   library code, which keeps `-G0` as it was originally compiled. A file in `src/` is Sony library code when its link-map `.text` input section is non-empty and lies entirely
   within the census's contiguous library span (`memory/closer/psyq-library-census.md` line 14:
   0x80078948..0x8008D070). Any other file, including one with no `.text`, gets `-G8`. The exception is
   exactly the set of files that pass this test: however the build spells it, every file in it passes and every
   file that passes is in it. The landing commit adds the implementing tool as `tools/psyq_library_files.py`
   (the auditor's `tmp/q56/adopt/psyq_library_files.py`), with an `engine test` case covering it, and records
   the classification of every `src/*.c` file with its `.text` range. Measured classification (survey, Q56
   auditor): `system`, `gpu`, `display`, `comb`, `text1b_b_tu2` (prnt), `text1b_b_tu3` (sprintf), `ings2` and
   `main` are library. Two files are not:
   - `main_post` holds data labels plus the LIBAPI A71/A72 stubs `AddDrv`/`DelDrv` (census rows at
     0x8008D050..0x8008D070); its `.text` ends at 0x8008D120, past the span.
   - `text1b_b` holds game code (from 0x80077B30) followed by compiled Sony library modules from 0x80078948
     to 0x80079244: the LIBAPI BIOS-call stubs and the LIBAPI COUNTER (SetRCnt...), PAD (PAD_init, InitPAD,
     _Pad1...), PATCH, SENDPAD and CHCLRPAD modules, plus LIBC2 memcpy, rand/srand, strcpy, strlen and printf
     (`build/bb2.map`:1804-1857; `memory/closer/psyq-library-census.md`:23-57). Its `.text` starts before
     the span.

   The SN Systems runtime needs no separate prong: `PCclose`, `__SN_ENTRY_POINT` and `__main` already lie
   inside the span. The test is per file. A Sony module that shares a file with game code (`text1b_b`,
   `main_post`) therefore gets `-G8`. `text1b_b` must be `-G8` because its game code reaches gp-defined
   statics; its `-G8` object reproduces the oracle, so the Sony modules inside it are byte-unaffected in this
   build. `tmp/q56/adopt/g8_test.sh` proves `-G8`/`-G0` neutrality only for files outside the gp-reach set
   (`main_post` among them: identical object 301f4b3b at `-G0` and `-G8`); `system.c` is the one library file
   measured to differ (evidence doc § Addendum A.7-A.8). Splitting
   such files at their PsyQ module boundaries, as was done for prnt and sprintf, is recorded as follow-up
   debt, not part of this adoption. The landing tool's GPREL16 check reads our own build objects, not the
   shipped code, and its docstring must say so. The landing commit is byte-identical, as every commit is.
3. **Definitions, file splits and merges** in the C, and initialized data moved from
   `asm/data/91C98.data.s` into the C file that owns it, per the two sections below.

## Definitions follow evidence

"Original access" means an instruction in the shipped bytes (`asm/funcs/<func>.s` of a function in F) that
loads or stores S directly. A definition of S in F is admitted ONLY when both hold:

- **(E1) Evidence.** At least one original access in F to S is gp-relative; or S meets the (A2) contiguity
  test below; or (A1) test 3 places S in F's static block (an object outside every block that F's code, and
  no other file of ours, references), in which case E2 and the size condition of (A1) apply to it as to any
  other object of that block.
- **(E2) Whole-file consistency.** Under the definition's kind (table above), the rule predicts every original
  access in F to S: each gp access is one the rule makes gp, and each `lui`/`%lo` access is one the rule makes
  non-gp. One contradicting access fails E2, whatever the other accesses do.

**(A2) Contiguity (owner ruling Q67).** A file's `.sdata` is one contiguous section, and so is its per-file
static block (A1) (evidence doc § Addendum A.1). An object S that lies in F's `.sdata` range or in F's static block, at an
address strictly between F's lowest-addressed and highest-addressed objects that meet E1 by a gp access in
that same range, is defined in F even if no access reaches it gp. This holds only when all of:
1. no other file of ours references S, by any access, address formation or relocation (otherwise the merge
   test or `docs/grind/borderline.md` decides);
2. E2 holds for S in F. Every object (A2) admits is 8 bytes or less (the size condition of (A1)), and for
   such an object the rule table makes every direct access gp, so any direct `lui`/`%lo` load or store of S in
   F fails E2: S is reached, if at all, only through `la` (address formation) or an indexed `S($reg)`
   operand;
3. S's kind is the kind of the range it lies in ((K2) in a static block, (K3) in `.sdata`).

Unreferenced gap bytes (no access, no address formation and no pointer word in the EXE resolves into them)
are defined as one object per maximal unreferenced run, at its exact size, named `D_<addr>` after its start
address, and listed in the ledger. Bytes that the build's own alignment of the next object already produces
are padding, not an object, and are not defined (shown by building without them: the next object's address
is unchanged). An unreferenced run longer than 8 bytes cannot be one object there (the size condition of
(A1)); it is split under (A6). E1's (A2) route allows exactly this.

**(A6) Runs no single object can occupy (owner ruling Q71).** An unreferenced run that no single object can
occupy at its address is split into the fewest aligned pieces. That is the case when the run is longer than 8
bytes (the size condition of (A1): an object over 8 bytes cannot sit in the small-data block, evidence doc
§ Addendum A.4), or when a build shows that an object of the run's size is placed at a different, aligned
address. The split is deterministic: in address order, each piece is the largest size of 8 bytes or less that
the build places exactly at its start address and that still fits in the rest of the run. Each piece is named `D_<addr>` after its start, defined at its exact size, and listed in the ledger,
and the commit is byte-identical. A later re-expression of such pieces (for example code6cac_c_mid's repeating
4-byte pattern as one array of small records) needs its own proof under the aggregate-merge entry of
[[no-new-park-categories]].

**Explicit-relocation asm.** The gp rule governs macro-form symbol accesses (the form cc1 emits and ASPSX
expands). An access written with an explicit `%hi`/`%lo`/`%gp_rel` operator in canonical hand-written asm text
(a function listed in `inline_asm_canonical.txt`) is assembled as written, not by the macro decision. It is
excluded from E1, E2 and the split test. The ledger lists each exclusion (function, symbol, asm line), plus one
ASPSX probe showing that ASPSX leaves an explicit `%hi`/`%lo` access to a file-defined small symbol as written.
(Example: `g_gpu_ot256_ptr`, reached `lui`/`%lo` in `func_80051D08` / `func_80051ED4`, evidence doc § 4.)

A definition that meets E1 but fails E2 is not fixed by choosing a different kind to suit the accesses; the
kind is fixed by (K1)-(K3). Its F goes to the file-boundary test below.

**(A1) Which bss objects are K1 and which are K2 (owner ruling Q67).** Sony PSYLINK lays out each file's `.lcomm` statics as one
per-file block, the blocks in link order, then every `.comm` tentative after all of them (evidence doc
§ Addendum A.1). BB2's bss has that shape (§ Addendum A.2). The static region is the longest run from the bss start in which the files reaching each object gp, taken in address order, never step back in bb2.ld .bss link order (each of the four groups named in the Merge bullet counts as one file, at its link position; a group must be contiguous in bb2.ld .bss link order, files with no .bss line in bb2.ld, or whose .bss input section is empty (0 bytes in the link map), being skipped, otherwise the case goes to borderline.md); it ends at the end of the last gp-reached object before the first object that steps back. The COMMON block runs from there to the end of the highest bss object that any file reaches gp. (Survey result only: on
the scratch reference build at base f777bdddd (evidence doc § Addendum A.2) the first object that steps back is 0x800A3688 (code6cac_c_mid), which gives a static region
0x800A3308-0x800A3618 and a COMMON block from 0x800A3618; the addresses are recomputed by this test at
adoption.)
Every object inside a file's per-file static block is K2 (static in that file); only objects in the COMMON
block are K1. The block boundaries are read from the bss layout and recorded in the evidence doc, by this
test:
1. F's block runs from the start of F's lowest-addressed object in the static region that F reaches gp to the
   end of its highest such object; (A2) places the objects between them in F.
2. The recorded blocks follow the `bb2.ld` `.bss` link order and do not overlap, except that files the merge
   test joins share one block. A layout that breaks this goes to `docs/grind/borderline.md`.
3. An object in the static region outside every block (between two blocks, or before the first or after the
   last) belongs to the block of the one file of ours whose code references it, when that file's block is
   next to the object in link order. If no file of ours references it, more than one does, or the one that
   does is not next to it, the case goes to `docs/grind/borderline.md`.
4. **Size condition** (evidence doc § Addendum A.4). The static region and the COMMON block hold objects of 8
   bytes or less only: PSYLINK places a static larger than 8 bytes in `.bss`, after the whole `.sbss` section,
   in per-file blocks of large statics in link order, with the large tentatives after those. An object larger
   than 8 bytes is never reached gp (rule table), so this rule gives it no definition, and (A1)-(A2) never
   place one in a small block. An object larger than 8 bytes found inside the static region or the COMMON
   block contradicts the layout and goes to `docs/grind/borderline.md`. The `.sdata` range likewise holds
   objects of 8 bytes or less only, by the `-G8` threshold: ASPSX treats a definition larger than 8 bytes as
   not small (rule table, evidence doc § 1), and cc1psx at `-G8` places an initialized object in `.sdata`
   only when it is 8 bytes or less (§ Addendum A.3). A referenced object larger than 8 bytes inside the
   `.sdata` range goes to `docs/grind/borderline.md`.

The kind of definition:

- **(K1) Tentative definition, `T S;`.** For a bss object in the COMMON block (A1); under E2, F reaches it gp
  at its base only. Owner ruling Q62's conditions apply unchanged ([[maspsx-gate-lists]] § The global COMMON model: the
  original bytes over the whole object are zero, no file gives it an initializer, its address comes from a
  symbol-file row). The EXE image ends at 0x800A3800 (header t_size 0x93800 from load address 0x80010000,
  b_size 0). An object wholly at or above 0x800A3800 is uninitialized bss by construction; there are no disc
  bytes, and its zero condition is met by the EXE header's image size (cite it in the ledger). For an object
  that straddles 0x800A3800, the part inside the image must be zero (disc offset and bytes cited), and the part
  above is met as above. Every file that meets E1 and E2 for S gets its own `T S;`.
- **(K2) `static`, `static T S;`.** For every object inside F's per-file static block (A1), whatever offset F
  reaches it at. F is the only file that defines it. A K2 object reached gp from more than one of our files
  triggers the merge test below, the same as a K3 object does; a K2 object referenced from another of our
  files in any other way goes to `docs/grind/borderline.md`. The object's declaration moves OUT of every header and into F:
  any header `extern` for S is deleted in the same commit. The `.lcomm` object gets its address from bss link
  order alone: F's `.bss` position in `bb2.ld` plus the object's order within F, with no per-symbol address pin
  or linker-script symbol assignment. If link order cannot produce the symbol-file address, the case goes to
  `docs/grind/borderline.md` and that commit does not land.
- **(K3) Initialized definition, `T S = value;`.** For an object in the initialized small-data region (the
  `.sdata` range that ends at 0x800A3308, inside the EXE image, evidence doc § 2), including one whose original
  bytes are zero. The initializer equals the
  original bytes. It is defined in exactly ONE file, the one whose original accesses reach it gp or the file
  (A2) places it in. Its label and
  bytes are removed from `asm/data/91C98.data.s` in the same commit. If more than one file reaches it gp, the
  merge test below decides.

T is the object's type as its existing declaration and evidence give it; its layout is judged under the
aggregate-merge entry of [[no-new-park-categories]] as before. No definition goes in a shared header: headers
keep `extern` declarations, except that a (K2) object's header `extern` is deleted. A symbol reached gp only from INCLUDE_ASM text needs no definition until that
function becomes C (evidence doc § 4).

**(A5) Three typing defaults (owner ruling Q70).** For exactly these three symbols the type is decided by the
owner, not by evidence, and nothing new is claimed about them: (1) `D_80102C00`, whose address is its only use,
keeps its existing `s32` type; (2) `D_800153F0`, 44 bytes read as 22 halfwords, is declared as a struct of 22
halfwords copied in one assignment, with that form's byte-identity proof banked in its landing commit; (3)
`func_8004153C`, which some callers call with no argument, keeps its current declaration, and its tidy-up is
deferred. No other symbol may cite this clause. Each landing is byte-identical and gets its own layer-2.

## File boundaries come from evidence only

These are tests, not an inventory: which files they split, merge or move is computed at adoption. The evidence
doc's findings were taken at the commit it names for each part (sections 1-5 at `a739dbd20`; the addendum's
bss survey on the scratch reference build at base `f777bdddd`) and are recomputed then.

- **Split.** File F is split into two files only when one symbol S has two original accesses in F that no
  single definition of S in F can both produce: one is gp and the other is a direct `lui`/`%lo` access the rule
  would make gp under every kind of definition of S that F could carry. The cut lies between the functions
  holding those two accesses; the split moves source text verbatim, in order, and changes nothing else except
  the externs and includes each part needs to compile. Each part defines only the symbols it reaches gp, and no
  gp symbol is shared across the cut. A new part inherits the parent's `GP_FILES`, `NO_SR_FILES`,
  `EXPAND_LB_FILES` and `EXPAND_LH_FILES` memberships (as [[compiler-flags-canonical]] (iv) requires for
  splits).
- **Cut position.** When the split test proves a boundary, every cut position in the window (from the last
  function that must stay in the earlier part through the first function that must be in the later part) is
  listed. Each position is tested for: no gp-reached small symbol shared across the cut; and
  [[rodata-object-alignment]] conditions 3-4. For condition 2: if the new part owns compiled rodata, its rodata
  start must be an item start it owns, with a valid phase. If it owns none, condition 2 is met. For a gp cut,
  rodata-object-alignment condition 3 applies only in its section-order, byte-neutrality and window-record
  parts. The gp convention replaces its placement convention. The gp convention: the cut sits immediately
  before the function holding the access the earlier part's assembly cannot produce. If every surviving
  position gives identical bytes, the conventional position is used, provided it survives. If surviving
  positions give different bytes, or the conventional position is not a survivor, the case goes to
  `docs/grind/borderline.md` and that commit does not land. The other surviving positions are recorded.
  Cut outcome, at the conventional position: (i) if a boundary set under rodata-object-alignment has a
  recorded window that contains the conventional position, that boundary moves to the conventional position
  (its current position may lie outside the gp window) and no new file is created; (ii) otherwise a new part begins at the
  conventional position. Boundaries not set under rodata-object-alignment (legacy splits with no recorded
  evidence) are never moved by the split test. They are kept or removed only under the merge test. Any case
  that fits neither (i) nor (ii), or where (i) fits more than one boundary, goes to borderline.md and that
  commit does not land. When (i) moves a boundary, its record in
  `docs/grind/rodata-align-2026-09-30.md` is updated in the same commit.
- **Merge.** Adjacent files are built as one file only when a (K2) static or (K3) initialized object (A1) is
  reached gp from each of them and they are contiguous in `bb2.ld` link order in every section (for each
  section, a file with no line for it in `bb2.ld`, or whose input section for it is empty, 0 bytes in the link
  map, is skipped; evidence doc § Addendum A.6b). Any merge other than the two Q65 groups
  (`code6cac_b2_pre` + `replay_camera_rob_back_loose2` + `code6cac_b2_post`; `code6cac_c2` + `config`) and the
  two Q67 groups (`text1a_c2` + `text1a_b` + `sound` + `text1b`; `text1b_tu2` + `text1b_b`), each with the
  data-only files (A7) admits, goes to `docs/grind/borderline.md`. The tail of `text1a_c` (after the Q65 split
  before `func_80044800`) shares no gp object with the first Q67 group (evidence doc § Addendum A.6,
  2026-09-30), so it stays its own file; a merge that includes it goes to `docs/grind/borderline.md`.
  **(A7) Data-only files (owner ruling Q72).** A data-only file (one that defines no function and contributes
  no `.text`, `.data`, `.sdata` or `.bss` section, only `.rodata`) joins an owner-approved merge group when, in
  `bb2.ld`'s `.rodata` link order, the nearest file before it and the nearest file after it that are not
  data-only files are both members of that group. A data-only file that the test places in no group stays its
  own file. A file with no `.rodata` bytes is not data-only under this definition, and joins no group under
  (A7). (Survey, § Addendum A.6-A.6b: `text1a_b_pre_rodata` sits between `text1a_b` and `text1b` and joins
  the first Q67 group; `text1a_b_mid_rodata` is empty, with no `.rodata` input section in `build/bb2.map`, so
  it is not data-only and stays its own file, and the second group's `.rodata` is contiguous through the
  empty-section skip.) The merge moves source text verbatim, in link order, and changes
  nothing else. If the verbatim merged text does not compile because the parts declare the same symbol
  differently, the declarations are reconciled FIRST in a separate byte-identical commit. That commit gives
  each symbol its one truthful type, chosen by evidence from the target bytes (its accesses) and each
  consumer's use, and is layer-2 reviewed on its own. If the evidence does not determine one type, the case
  goes to borderline.md and the merge does not land. If the gp users of a symbol that the merge test would
  otherwise join lie on both sides of a boundary set under rodata-object-alignment, and that boundary's
  recorded window contains a position that puts all of those users on one side, the boundary moves to that
  position in place of a merge. If there are several such positions, use the one closest to the boundary's
  current position; if two are equally close, or the move would split another symbol's users, the case goes
  to borderline.md. The record is updated in the same commit.
- **Rodata.** A gp-evidenced split must satisfy conditions 2-4 of [[rodata-object-alignment]] § New TU
  boundaries (rodata position, text cut, moves only), applied to each cut position as the Cut position bullet
  states; its existence is supplied by the split test above in place of that rule's condition 1. A boundary set
  under rodata-object-alignment may also be moved within its recorded window by this rule's cut outcome (i) or
  by its Merge-bullet boundary move; that move's placement replaces condition 3's placement clause for that
  boundary, and its record in docs/grind/rodata-align-2026-09-30.md is updated in the same commit. A merge that
  would remove a boundary the rodata rule established, and that the boundary move in the Merge bullet does not
  replace, goes to `docs/grind/borderline.md`, and that commit does not land.
- **Undecided cases.** A case these tests do not decide (surviving cut positions that give different bytes,
  a conventional cut position that is not a survivor, a cut outcome that fits neither (i) nor (ii) or where (i)
  fits more than one boundary, a (K2) static referenced from another file other than by gp, a static whose
  address link order cannot produce, a bss layout or an object outside every static block that (A1) does not
  place, a (K2) or (K3) object reached gp from non-adjacent files, a merge whose declarations cannot be reconciled by evidence, a merge
  across a rodata-rule boundary that the Merge bullet's boundary move does not replace) is not decided by
  picking what matches. It is logged to `docs/grind/borderline.md` as a
  policy-question, and the commit that needs it does not land.

## What this is not

- **Not a license to add a definition to flip one access.** E1 and E2 hold for every definition, across the
  whole file.
- **Not a list in any spelling.** No file, flag, pragma or maspsx option that names functions or symbols may
  select gp, including the proof of concept's `--poc-noncomm-syms` stand-in. `maspsx_comm_syms.txt` stays
  retired (Q62).
- **Not a compiler change.** cc1 is untouched. The two maspsx fixes model measured ASPSX behaviour, and the
  (A3) section choice models measured cc1psx behaviour under owner ruling Q68; cc1psx is used only as a
  calibration, never as a build path ([[no-compiler-divergence]]; the Claude harness memory rule
  `rules/cc1psx-calibration-only`, not a repo path).

## How it lands

As a series of commits after the current landings finish. Each commit:

1. is byte-identical: full-build SHA1 == oracle, and every `src/*.c` object it touches built both ways and
   compared (a maspsx or flag change compares every object);
2. keeps `engine test` green;
3. gets a fresh default-FAIL layer-2 `cheat-reviewer` PASS on its exact diff, which checks E1, E2 and (K1)-(K3)
   for each definition it adds and the split or merge tests for each boundary it moves.

The commit that deletes the three lists also updates every rule and doc that tells agents to use them
(e.g. `canonical-asm-authorization-recipe.md`, `compiler-flags-canonical.md`). Until the adoption lands, the
lists stay in force, and rows required by existing rules (e.g. [[compiler-flags-canonical]]'s `sdata_syms.txt`
requirement) may still be added with their usual evidence. The adoption retires every row, including those. The Q56 row-by-row audit of
`sdata_exclude.txt` is not continued: the owner chose full adoption over the option "Don't adopt; audit the
lists", whose text was "Q56 step 2 as written: keep the lists. [...] Per-function lists stay in the build."
(Q65, docs/grind/owner-rulings-2026-09-26.md, thirty-first batch). The adoption retires every row.

## Related

[[maspsx-gate-lists]] · [[rodata-object-alignment]] · [[compiler-flags-canonical]] ·
[[no-compiler-divergence]] · [[no-new-park-categories]]
