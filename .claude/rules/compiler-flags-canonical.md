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
sweep on saTan0Main showed no flag produces its partial cross-jump merge.

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
  `INCLUDE_ASM` or `INCLUDE_RODATA`. **Screening:** every <=8-byte extern it references is a small-data
  object in gp range (a K1/K2/K3 object under [[per-file-gp-model]]: the `.sdata` range or an (A1) static or COMMON block)
  or honestly typed > 8 bytes — unless a both-ways build (full per-file pipeline at `-G8`
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

**Owner ruling Q89 (2026-10-01) — text1b's -G8 head part (camera_CalcAngles).** text1b may be split
immediately before its first whole-body hand-written asm function (the inline_asm_canonical.txt block
starting at math_RotMatrixZYX), the part before the cut joining `GP_FILES` (cc1 `-G8`). Evidence class:
under `-G8` cc1 emits every function after all file-scope asm, so a mid-file hand-written block cannot sit
inside one `-G8` file; camera_CalcAngles's A8 array store needs `-G8` (cc1psx and our cc1 alike; `-G0`
proof in `faa2ebb06^:memory/grind/camera_CalcAngles/g0proof/`). Staged, ALL required: func_80048FFC lands in C first;
the small-declared objects in that part get their real types first (each its own reviewed landing); the cut
passes [[per-file-gp-model]]'s split tests in full (no shared gp symbol of any kind across the cut, so no Q67
merge object's gp users are separated; [[rodata-object-alignment]] conditions 2-4; parts inherit
`NO_SR_FILES`/`EXPAND_LB_FILES`/`EXPAND_LH_FILES`). Q89 is an added evidence class, not a waiver: it replaces
only (iii)'s "the TU holds only functions meeting (i)+(ii)" for this part, whose other functions qualify by a
both-ways full build showing each byte- and relocation-identical. Everything else above still applies: (i)+(ii)
banked for camera_CalcAngles; the rest of (iii) (no file-scope `__asm__`, `INCLUDE_ASM` or `INCLUDE_RODATA` in
the `-G8` part; the extern screening); (iv); (v); (vi) incl. manual path only. Only this cut, only this file.

**Owner ruling Q94 (2026-10-02) — the pre_rodata block between Q89's parts.** The former
`text1a_b_pre_rodata.c` block (D_800153F0..D_80015840, merged into text1b by Q67 under
[[per-file-gp-model]] A7) is restored verbatim as its own `.rodata`-only `-G0` file linked between Q89's head
and tail. Evidence: under `-G8` cc1 emits every file-scope data object before every function's jump tables, so
inside the head the block lands ahead of `.L10/.L26/.L55`; at the tail's top the tail starts at phase 0, not
func_80058580's table phase 4 (`faa2ebb06^:memory/grind/camera_CalcAngles/q89split/`). A7 does not apply to this block.
Q89's other conditions are unchanged; split and `-G8` opt-in each land oracle-green with a layer-2.

## Related

[[no-compiler-divergence]] (the toolchain is frozen; this rule is its corollary) · [[cross-jump-call-merge]] ·
[[per-file-gp-model]]
