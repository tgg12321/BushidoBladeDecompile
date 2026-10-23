---
name: data-only-no-sdata
paths: ["Makefile", "engine/buildconfig.py", "tools/psyq_library_files.py", "bb2.ld", "asm/data/**"]
description: "Owner ruling Q122 (2026-10-22): a data-only C file whose small initialized objects the original keeps in .data (not .sdata) is built without maspsx -G8. The set is derived by tools/psyq_library_files.py from evidence, never listed by hand."
metadata:
  type: rule
  tier: blocking
---

# Data-only files without small data (owner rulings Q122, Q123)

Extends [[per-file-gp-model]] (A4, Q69), which turns maspsx `-G8` off only for Sony library code.
Under `-G8`, (A3) moves any initialized object of total size <= 8 bytes from `.data` to `.sdata`,
`static` or not, so a small table the original has in `.data` cannot be written as C in the file
that reads it.

## The rule

A C file is built without maspsx `-G8` when `tools/psyq_library_files.py` derives it from the linked
build: no `.text` input section and no function symbols; its `.data` input section lies entirely in
the original `.data` range below the `.sdata` range (0x800A30DC..0x800A3308); and it defines an
initialized object with `.size` <= 8. The derived set lives in its own Makefile/buildconfig variable
(`DATA_ONLY_NO_SDATA_FILES`), never in `PSYQ_LIBRARY_FILES`; `--check` fails on any hand-added or
missing file. A new file in the set must also meet:

1. **One run:** every byte it defines lies in one contiguous original `.data` run below 0x800A30DC;
   no uninitialized objects.
2. **Sized by evidence:** the <= 8-byte object's size is fixed by evidence (the next object has its
   own reader or a distinct type, or it is the run's end), not by a splat label. If the run can be
   typed honestly as objects > 8 bytes, it stays `-G8` and needs no file of its own.
3. **No owning neighbour:** each neighbouring module in `.data` link order either uses small data (its
   own `.sdata`, `.sbss` or gp accesses) or is Sony library code, and no `.text` between them is
   unaccounted for, so no code module could own the object. (Q123) A neighbouring game module with no
   small data counts as a possible owner only if it references objects in the run (loads, stores or
   takes their address, from its C, its INCLUDE_ASM bodies or its own data relocations): one that
   references none does not block the file; objects it does reference stay in asm as an open case,
   and the run splits around them (each side meets 1-4 on its own). Anything else goes to `docs/grind/borderline.md`; never
   "because it matches".
4. **Top comment** states 1-3: the address range, the small objects that place it, and its readers.

Contents are the original bytes, typed per [[per-file-gp-model]]'s K-kinds and existing declarations.
Naming: `src/main/d_<rom offset>.c` (data-only game file, AGENTS.md conventions); a Sony module's
data stays in its module file under `src/main/psxsdk/`.

## What this is not

Not a per-symbol or per-function switch, and not for any file with code: a file with functions keeps
`-G8` whatever it would help. Not a cc1 flag change ([[compiler-flags-canonical]], [[no-compiler-divergence]]).
The tool change lands byte-identical (full-build SHA1 == oracle) with an `engine test` case.
