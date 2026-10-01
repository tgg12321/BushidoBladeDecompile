---
name: compiler-flags-canonical
paths: [".claude/rules/compiler-flags-canonical.md"]
description: "Compiler FLAGS are a dead avenue: -O2 -mel -msoft-float (+ per-file GP_FILES / NO_SR_FILES) is proven canonical. Don't flag-hunt; the walls are source structure. New GP_FILES members only by proof."
metadata:
  type: reference
---

# Compiler flags are canonical — do not flag-hunt

The original build used our flags. Do not toggle `-O` levels or `-f*` flags hoping a function falls out — the
divergence is always in C source structure (scheduling, RA, cross-jump; [[cross-jump-call-merge]]).

**Proof (2026-05-20).** GCC 2.7.2 has per-TU flags only (no per-function mechanism). Every `.c` file with
remaining work also holds many byte-exact `-O2` matches, so each file is flag-uniform at `-O2`. A 24-flag
sweep on saTan0Main (`tmp/flag_sweep.sh`) showed no flag produces its partial cross-jump merge.

## The canonical flag set (frozen)

- `-O2 -G0 -funsigned-char -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -mel -msoft-float`.
- `-mel` (2026-08-04) and `-msoft-float` (2026-09-07) are configuration-fidelity corrections of the
  `mips-mips-gnu` cc1 triple's wrong defaults (big-endian; hard float — cc1psx prints
  `# Cc1 defaults: -mgas -msoft-float`), not flag-hunting precedents. cc1psx's printed defaults header is the
  authoritative record of the original configuration.

| Per-file mechanism (Makefile) | Flag | Notes |
|---|---|---|
| `GP_FILES` | `-G8` | only by the per-file -G8 proof below (text1a files owner-approved 2026-08-05) |
| `NO_SR_FILES` | `-fno-strength-reduce` | files where strength reduction diverges |
| `FIX_LWL_FILES` | — | RETIRED 2026-08-04 (obsolete under `-mel`) |

Nothing else varies per file. (maspsx's own `-G8` under the per-file gp model is a separate switch —
[[per-file-gp-model]].)

## Per-file -G8 by proof (owner ruling 2026-09-26)

A function may move into a new TU in `GP_FILES` ONLY when ALL hold (record: decisions.md 2026-09-26 OWNER
RULING — per-file -G8 by proof; Q16 split/respelling order; -G8 screening scope):

- **(i)** The ledger lists every gp-relative access in the function's ORIGINAL bytes; no adjacent function
  staying outside the TU has a gp access to any of those symbols; the best `-G0` score and why `-G0` can't
  produce them are recorded.
- **(ii)** cc1psx (`tools/cc1psx_wrapper.sh`, calibration only) on the TU's preprocessed source emits the
  listed gp accesses at `-G8` and none at `-G0`; both outputs banked.
- **(iii)** The TU holds only functions meeting (i)+(ii), contiguous in address order; no file-scope `__asm__`,
  `INCLUDE_ASM` or `INCLUDE_RODATA`. **Screening:** every <=8-byte extern it references is in gp range
  (`sdata_syms.txt`) or honestly typed > 8 bytes — unless a both-ways build (full per-file pipeline at `-G8`
  and `-G0`) shows every access to it byte- and relocation-identical, banked in the ledger. It keeps the
  source file's `NO_SR_FILES`, `EXPAND_LB_FILES`, `EXPAND_LH_FILES` memberships.
- **(iv)** Every other function moves to the remaining file or a new adjacent `-G0` TU in original order, as
  a textually identical diff, with the source file's exact flags. A respelling under another rule lands FIRST
  (or is proven byte-neutral unsplit) — or, only when the ledger banks that it cannot be byte-neutral before
  the split, AFTER it as a separate commit, each commit oracle-green and both layer-2 reviewed (Q16).
- **(iv-a)** Retired with `RODATA_ALIGN2_FILES`: jump-table alignment is now the uniform object-relative
  rule ([[rodata-object-alignment]]).
- **(v)** Makefile changes are limited to the new TU names/memberships; `engine/buildconfig.py` mirrors them;
  `bb2.ld` (hand-maintained, never `make setup`) gains only object lines in address order; no
  `LINKED_ASM_FUNCS` entry; LF endings.
- **(vi)** `verify-oracle --rebuild` passes, `engine test` green, fresh layer-2 PASS on the split, build files
  and ledger. Manual path only (the Grinder cannot touch Makefile/bb2.ld/engine).

`-G8` is never admitted for a measured score improvement alone.

## Related

[[no-compiler-divergence]] (the toolchain is frozen; this rule is its corollary) · [[cross-jump-call-merge]] ·
[[per-file-gp-model]]
