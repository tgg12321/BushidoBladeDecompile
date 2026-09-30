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

1. **maspsx, two global fixes.** `_uses_gp` (`tools/maspsx/maspsx/__init__.py`) returns False for an operand
   with a base register (the indexed-operand fix, evidence doc § 3), and `.local` + `.comm` is modelled as
   `.lcomm` (today it raises, `__init__.py` "uninitialized static ... is not modelled"). Both are global: no
   list, no function or symbol name.
2. **Flags.** In the Makefile's `MASPSX_FLAGS` and `MASPSX_FLAGS_GP`, `--sdata-syms`, `--sdata-funcs` and
   `--sdata-exclude` are removed and maspsx's own `-G8` switch is added; `engine/buildconfig.py` mirrors the
   Makefile verbatim. `sdata_syms.txt`, `sdata_funcs.txt` and `sdata_exclude.txt` are deleted. This `-G8` is
   the maspsx option (`tools/maspsx/maspsx.py`, `-G<n>`). cc1's per-file `-G8` (`GP_FILES`) is a different
   switch and stays governed by [[compiler-flags-canonical]], unchanged.
3. **Definitions, file splits and merges** in the C, and initialized data moved from
   `asm/data/91C98.data.s` into the C file that owns it, per the two sections below.

## Definitions follow evidence

"Original access" means an instruction in the shipped bytes (`asm/funcs/<func>.s` of a function in F) that
loads or stores S directly. A definition of S in F is admitted ONLY when both hold:

- **(E1) Evidence.** At least one original access in F to S is gp-relative.
- **(E2) Whole-file consistency.** Under the definition's kind (table above), the rule predicts every original
  access in F to S: each gp access is one the rule makes gp, and each `lui`/`%lo` access is one the rule makes
  non-gp. One contradicting access fails E2, whatever the other accesses do.

**Explicit-relocation asm.** The gp rule governs macro-form symbol accesses (the form cc1 emits and ASPSX
expands). An access written with an explicit `%hi`/`%lo`/`%gp_rel` operator in canonical hand-written asm text
(a function listed in `inline_asm_canonical.txt`) is assembled as written, not by the macro decision. It is
excluded from E1, E2 and the split test. The ledger lists each exclusion (function, symbol, asm line), plus one
ASPSX probe showing that ASPSX leaves an explicit `%hi`/`%lo` access to a file-defined small symbol as written.
(Example: `g_gpu_ot256_ptr`, reached `lui`/`%lo` in `func_80051D08` / `func_80051ED4`, evidence doc § 4.)

A definition that meets E1 but fails E2 is not fixed by choosing a different kind to suit the accesses; the
kind is fixed by (K1)-(K3). Its F goes to the file-boundary test below. The kind of definition:

- **(K1) Tentative definition, `T S;`.** For a bss object (at or above 0x800A3308, evidence doc § 2) that F
  reaches gp at its base only. Owner ruling Q62's conditions apply unchanged ([[maspsx-gate-lists]] § The global COMMON model: the
  original bytes over the whole object are zero, no file gives it an initializer, its address comes from a
  symbol-file row). The EXE image ends at 0x800A3800 (header t_size 0x93800 from load address 0x80010000,
  b_size 0). An object wholly at or above 0x800A3800 is uninitialized bss by construction; there are no disc
  bytes, and its zero condition is met by the EXE header's image size (cite it in the ledger). For an object
  that straddles 0x800A3800, the part inside the image must be zero (disc offset and bytes cited), and the part
  above is met as above. Every file that meets E1 and E2 for S gets its own `T S;`.
- **(K2) `static`, `static T S;`.** For a bss object that F reaches gp at a nonzero offset.
  Every reference to S in the whole program (C and asm) lies in F, and F is the only file that defines it. If
  another file references S, K2 does not apply. The object's declaration moves OUT of every header and into F:
  any header `extern` for S is deleted in the same commit. The `.lcomm` object gets its address from bss link
  order alone: F's `.bss` position in `bb2.ld` plus the object's order within F, with no per-symbol address pin
  or linker-script symbol assignment. If link order cannot produce the symbol-file address, the case goes to
  `docs/grind/borderline.md` and that commit does not land.
- **(K3) Initialized definition, `T S = value;`.** For an object in the initialized small-data region (the
  `.sdata` range that ends at 0x800A3308, inside the EXE image, evidence doc § 2), including one whose original
  bytes are zero. The initializer equals the
  original bytes. It is defined in exactly ONE file, the one whose original accesses reach it gp. Its label and
  bytes are removed from `asm/data/91C98.data.s` in the same commit. If more than one file reaches it gp, the
  merge test below decides.

T is the object's type as its existing declaration and evidence give it; its layout is judged under the
aggregate-merge entry of [[no-new-park-categories]] as before. No definition goes in a shared header: headers
keep `extern` declarations, except that a (K2) object's header `extern` is deleted. A symbol reached gp only from INCLUDE_ASM text needs no definition until that
function becomes C (evidence doc § 4).

## File boundaries come from evidence only

These are tests, not an inventory: which files they split, merge or move is computed at adoption. The evidence
doc's findings were taken at `a739dbd20` and are recomputed then.

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
- **Merge.** Adjacent files are built as one file only when a (K3) initialized symbol is reached gp from each of
  them and they are contiguous in link order. The merge moves source text verbatim, in link order, and changes
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
  fits more than one boundary, a static referenced from two files, a static whose address link order cannot produce, an initialized
  symbol reached from non-adjacent files, a merge whose declarations cannot be reconciled by evidence, a merge
  across a rodata-rule boundary that the Merge bullet's boundary move does not replace) is not decided by
  picking what matches. It is logged to `docs/grind/borderline.md` as a
  policy-question, and the commit that needs it does not land.

## What this is not

- **Not a license to add a definition to flip one access.** E1 and E2 hold for every definition, across the
  whole file.
- **Not a list in any spelling.** No file, flag, pragma or maspsx option that names functions or symbols may
  select gp, including the proof of concept's `--poc-noncomm-syms` stand-in. `maspsx_comm_syms.txt` stays
  retired (Q62).
- **Not a compiler change.** cc1 is untouched; both maspsx changes are models of measured ASPSX behaviour
  ([[no-compiler-divergence]]).

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
