---
name: per-file-gp-model
paths: ["sdata_*.txt", "Makefile", "engine/buildconfig.py", "engine/pipeline.py", "tools/maspsx/**", "bb2.ld"]
description: "Owner ruling Q65 (2026-09-30): gp addressing follows ASPSX 2.34's per-file rule (gp only for a <=8-byte symbol the file defines), replacing the sdata lists. Definitions/file boundaries from evidence only."
metadata:
  type: rule
---

# Per-file gp model (owner rulings Q65, Q67-Q72, 2026-09-30)

Records: docs/grind/decisions.md 2026-09-30 OWNER RULING — the per-file gp model (and the Q67-Q72 entries);
evidence `docs/grind/gp-model-2026-09-30.md` (+ § Addendum). Until the adoption lands, `sdata_syms.txt`,
`sdata_funcs.txt`, `sdata_exclude.txt` stay in force; the adoption retires every row.

## The rule (Sony ASPSX 2.34, measured: 6,337 accesses, 0 disagreements)

For a direct load/store of S (`S` or `S+k`) in file F:

| F's own declaration of S | gp-relative? |
|---|---|
| none, or `extern` | never |
| `.comm` (tentative), S <= 8 bytes | at `S+0` only; `S+k`, k≠0, is `lui`/`%lo` |
| `.lcomm` (uninitialized `static`), <= 8 bytes | every offset |
| `.sdata` (initialized), <= 8 bytes | every offset |
| any definition > 8 bytes | never |

An indexed operand `S($reg)` is never gp. The answer is per FILE, identical for every function in F.

## What the adoption changes

1. **maspsx, global only:** `_uses_gp` is False for a base-register operand; `.local`+`.comm` is modelled as
   `.lcomm`; (A3, Q68) under maspsx `-G8` only, an initialized object cc1 emits into `.data` with total size
   <= 8 bytes goes to `.sdata` (models cc1psx; cc1 stays unpatched). A `static` initialized object moves only
   with a cc1psx calibration showing it. Landing commit adds an `engine test` case.
2. **Flags:** `--sdata-syms/--sdata-funcs/--sdata-exclude` removed, maspsx `-G8` added; buildconfig mirrors
   the Makefile; the three lists are deleted. (A4, Q69) maspsx `-G8` is on for every C file except Sony
   library code (`.text` non-empty and entirely within 0x80078948..0x8008D070,
   `memory/closer/psyq-library-census.md`), implemented by `tools/psyq_library_files.py` with an engine test.
   Mixed files (`text1b_b`, `main_post`) get `-G8`. cc1's per-file `-G8` (`GP_FILES`) is a different switch,
   governed by [[compiler-flags-canonical]].
3. **Definitions, file splits/merges** in the C per the tests below; initialized data moves from
   `asm/data/91C98.data.s` into its owning C file.

## Definitions follow evidence

A definition of S in F is admitted ONLY when:
- **(E1)** some original access in F to S is gp-relative; or S meets (A2); or (A1) test 3 places S in F's
  static block;
- **(E2)** under the definition's kind, the rule predicts EVERY original access in F to S. One contradicting
  access fails E2. A failing file is not fixed by changing the kind; it goes to the split test.

**(A2) Contiguity.** An object strictly between F's lowest and highest E1-by-gp objects in F's `.sdata` range or
static block is defined in F if: no other file references it; E2 holds (so only `la`/indexed access); its kind
is its range's kind. Unreferenced gap runs become one `D_<addr>` object at exact size; build-alignment padding
is not an object. **(A6)** A run no single object can occupy (> 8 bytes, or misplaced by the build) splits
deterministically into the fewest aligned pieces (largest <= 8 bytes the build places at each start).

**Explicit-relocation asm** (`%hi`/`%lo`/`%gp_rel` in canonical asm) is excluded from E1/E2/split, listed in
the ledger with one ASPSX probe.

**(A1) bss layout.** PSYLINK lays out per-file `.lcomm` blocks in link order, then all `.comm`. The static
region runs from bss start while the gp-reaching files, in address order, never step back in `bb2.ld` `.bss`
link order (merge groups count as one file; empty/absent `.bss` files skipped); the COMMON block follows to the
highest gp-reached object. Objects in a file's block are K2, in the COMMON block K1. Blocks must follow link
order without overlap; an orphan object joins the adjacent block of the one file referencing it; only objects
<= 8 bytes live in these blocks and in `.sdata`. Anything else → `docs/grind/borderline.md`.

Kinds:
- **(K1) `T S;`** — COMMON-block object; Q62 conditions ([[maspsx-gate-lists]] § The global COMMON model):
  original bytes zero (objects at/above 0x800A3800 are bss by the EXE header), no initializer, address from a
  symbol-file row. Every file meeting E1+E2 gets its own `T S;`.
- **(K2) `static T S;`** — object in F's static block; defined only in F; header `extern` deleted; address from
  link order alone (no pin). Reached gp from two files → merge test; referenced otherwise → borderline.md.
- **(K3) `T S = value;`** — object in the `.sdata` range (ends 0x800A3308); initializer = original bytes; ONE
  defining file; label+bytes removed from `91C98.data.s` in the same commit.

No definition in a shared header. Types follow existing declarations/evidence (aggregate-merge entry of
[[no-new-park-categories]]). **(A5, Q70)** only `D_80102C00` (keeps `s32`), `D_800153F0` (struct of 22
halfwords) and `func_8004153C` (declaration unchanged) take owner-decided types.

## File boundaries come from evidence only

- **Split** only when one symbol has two original accesses in F (one gp, one direct `lui`/`%lo` that every
  possible definition kind would make gp). Source moves verbatim; parts inherit `GP_FILES`, `NO_SR_FILES`,
  `EXPAND_LB_FILES`, `EXPAND_LH_FILES`. All cut positions in the window are tested (no shared gp symbol across
  the cut; [[rodata-object-alignment]] conditions 2-4); the conventional cut is immediately before the function
  holding the access the earlier part can't produce. A recorded rodata-rule boundary whose window contains it
  moves there instead of creating a file; legacy boundaries never move.
- **Merge** adjacent files only when a K2/K3 object is reached gp from each and they are contiguous in every
  `bb2.ld` section. Only the Q65 groups (`code6cac_b2_pre`+`replay_camera_rob_back_loose2`+`code6cac_b2_post`;
  `code6cac_c2`+`config`) and Q67 groups (`text1a_c2`+`text1a_b`+`sound`+`text1b`; `text1b_tu2`+`text1b_b`)
  are approved. **(A7, Q72)** a `.rodata`-only file between members of one group joins it. Declarations that
  conflict are reconciled first in a separate byte-identical, evidence-typed commit.
- **Undecided cases** (divergent surviving cuts, unplaceable objects, non-adjacent gp users, irreconcilable
  declarations, merges across a rodata-rule boundary) are logged to `docs/grind/borderline.md`; the commit
  does not land. Never pick what matches.

## What this is not

Not a license to add a definition to flip one access; not a list in any spelling (no file/flag/pragma/maspsx
option naming functions or symbols selects gp; `maspsx_comm_syms.txt` stays retired); not a compiler change
(cc1psx is calibration only, [[no-compiler-divergence]]).

## How it lands

A series of commits, each: byte-identical (full-build SHA1 == oracle; every touched `src/*.c` object — every
object for a maspsx/flag change — built both ways and compared); `engine test` green; a fresh default-FAIL
layer-2 `cheat-reviewer` PASS checking E1, E2, K1-K3 and the split/merge tests. The commit deleting the lists
updates every rule/doc that tells agents to use them.
