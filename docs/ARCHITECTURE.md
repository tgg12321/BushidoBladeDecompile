# Architecture

This document describes the engineering structure of the BB2 matching decompilation: the target binary's memory map, the build pipeline that reproduces it, the role of each post-pass tool, and the conventions used to keep the build byte-identical to the 1998 release.

If you want the matching techniques themselves (penalty profiles, C-side tricks, regfix recipes), read [`MATCHING.md`](MATCHING.md). If you want the catalog of standalone tools, read [`TOOLS.md`](TOOLS.md).

## Target binary

The build produces `disc/SLUS_006.63` (606,208 bytes, SHA1 `62efab4f73f992798c43e8c730aa43baa10bb4fa`). The original is a PS-X EXE: a 0x800-byte Sony header followed by a single contiguous load image.

### Memory map

| Region | Address range | Use |
|---|---|---|
| KUSEG | `0x00000000`–`0x7FFFFFFF` | User space (unused by BB2's main code; cached) |
| KSEG0 | `0x80000000`–`0x9FFFFFFF` | Cached kernel space — **all BB2 code and data lives here** |
| KSEG1 | `0xA0000000`–`0xBFFFFFFF` | Uncached mirror of KSEG0 (used for I/O) |
| KSEG2 | `0xC0000000`–`0xFFFFFFFF` | Reserved |
| Scratchpad RAM | `0x1F800000`–`0x1F8003FF` | 1 KB fast on-CPU SRAM (8 ns vs 250 ns for main RAM) |
| I/O registers | `0x1F801000`–`0x1F803000` | GPU, SPU, DMA, timers, controllers |
| VRAM (GPU) | 1 MB on a separate bus | Texture pages and framebuffers via GP0/GP1 |
| SPU RAM | 512 KB on a separate bus | Sound samples (VAB banks loaded here) |

The main 2 MB of RAM is mapped at `0x80000000` in KSEG0. BB2's executable loads at `0x80010000`, leaving the first 64 KB for the Sony BIOS and reserved area.

### Executable layout

| Property | Value |
|---|---|
| Format | PS-X EXE (Sony PlayStation executable format) |
| Header size | `0x800` bytes (2 KB) |
| Load address (`PC0`) | `0x80010000` |
| Entry point | `0x800836EC` |
| Stack top (`gp/sp`) | `0x801FFFF0` |
| GP register (`gp`) | `0x800A30CC` |
| Text + data size | `0x93800` (604,160 bytes) |
| File offset of image | `0x800` (immediately after header) |
| Total file size | 606,208 bytes |

The header bytes are preserved verbatim from the original (`disc/SLUS_006.63[0:0x800]`) and prepended to the linker's output by `tools/make_psexe.py`. The rebuilt image only needs to match the body.

### Sections

PsyQ-built executables use a non-standard section order: `.rodata → .text → .data → .bss`, the opposite of what GNU `ld` produces by default. The linker script [`bb2.ld`](../bb2.ld) replicates this ordering inside a single `.main` output section starting at `0x80010000`. Approximate boundaries:

| Section | Start | End | Contents |
|---|---|---|---|
| `.rodata` | `0x80010000` | `~0x80016AB0` | Strings, jump tables, const data, debug strings (`"Marionation over flow"`, etc.) |
| `.text` | `~0x80016AB0` | `~0x8008D070` | Code (1,410 functions) |
| `.data` | `~0x8008D080` | `~0x800A3800` | Initialized globals (animation state tables, etc.) |
| `.bss` | `~0x800A3800` | end of image | Zero-initialized globals (uninitialized; reserved at runtime) |

The exact boundaries depend on which `src/*.c` files are participating in each section; the linker script interleaves C objects with asm objects to preserve the original ordering. See `bb2.ld` for the explicit order.

### Build identification

The original was built **Fri Aug 7 22:26:32 1998** (string embedded at `0x8001004C` in `.rodata`). Toolchain identity was inferred from CVS `$Id:` tags in the linked PsyQ libraries:

| Library | CVS tag | Confirms |
|---|---|---|
| `libgpu` | `sys.c v1.129 1996/12/25` | PsyQ ≤ 3.6 |
| `libcd` | `bios.c v1.86 1997/03/28` | PsyQ ≤ 3.6 |
| `libapi` | `intr.c v1.76 1997/02/12` | PsyQ ≤ 3.6 |

Combined with the `1993-1997` PsyQ copyright string, this places the build at **PsyQ 3.5 (DTL-S3000)** with the **GCC 2.7.2 / ASPSX 2.34** compiler and assembler — confirmed by byte-identical reproduction.

### Linked PsyQ libraries

| Library | Purpose |
|---|---|
| `libgpu` | GPU command FIFO, OT chains, GsRECT helpers |
| `libcd` | CD-ROM filesystem, XA streaming |
| `libapi` | Kernel API: events, threads, callbacks |
| `libspu` | SPU sound, ADPCM, VAB management |
| MDEC decode | In the overlay only — `MOVOVL.EXE` for FMV playback |

These are not built from source here. They appear pre-linked in `asm/funcs/` for now; once-named symbols in `named_syms.txt` route the linker through the right addresses.

### FMV overlay

`disc/STR/MOVOVL.EXE` is a separate 122,880-byte PS-X EXE that loads at `0x801D8800` (entry `0x801DA084`) when the game plays an opening FMV or the title screen video. It contains MDEC decode functions not in the main executable. The overlay does **not** overlap with the main EXE (main ends around `0x800A3800`); both can coexist in the 2 MB RAM. The overlay is not currently part of the matching build — it's tracked separately.

## Build pipeline

The build is driven by [`Makefile`](../Makefile). Each `.c` file in `src/` goes through a long post-pass pipeline before reaching the assembler; `.s` files in `asm/` go straight to `as`. Everything is then linked, stripped to raw binary, and a PS-EXE header is prepended.

### C compilation pipeline (per `.c` file)

```
src/<name>.c
    |
    v
[cpp]                  mipsel-linux-gnu-cpp -Iinclude -undef -Wall -lang-c -fno-builtin
    |                  +PsyQ defines (-Dmips -D__GNUC__=2 -D__OPTIMIZE__ -D__psx__ ...)
    v
[cc1 (GCC 2.7.2)]      tools/gcc-2.7.2/build/cc1 -O2 -G0 -funsigned-char -mcpu=3000 -mips1 -w
    |                  (decompals/mips-gcc-2.7.2; PsyQ-era GCC 2.7.2 cross-compiler)
    v
[prologue_fix]         tools/prologue_fix.py
    |                  Applies special prologue rewrites (delay-slot sw $ra,
    |                  varargs prologue fixups). Driven by config files like
    |                  delay_slot_ra_funcs.txt and tools/prologue_config.json.
    v
[maspsx]               tools/maspsx/maspsx.py --aspsx-version=2.34 --expand-div ...
    |                  ASPSX compatibility layer over GNU as. Converts GCC
    |                  output to ASPSX-equivalent encodings: gp-rel pseudoinstr
    |                  expansion, lb/lh widening, multu padding, sdata splits.
    v
[fix_lwl]              tools/fix_lwl.py  (per-file opt-in via FIX_LWL_FILES)
    |                  GCC 2.7.2 (decompals build) emits lwl/lwr/swl/swr with
    |                  big-endian byte offsets. fix_lwl XORs each offset with 3
    |                  to match PS1's little-endian convention.
    v
[rodata align fix]     sed "s/\.align\t3/.align\t2/"  (per-file opt-in)
    |                  GCC emits .align 3 (8-byte) for switch tables. After a
    |                  rodata split, those 8-byte alignments insert unwanted
    |                  padding. Files in RODATA_ALIGN2_FILES get them downgraded
    |                  to .align 2 (4-byte).
    v
[multu_pad]            tools/multu_pad.py --funcs multu_pad_funcs.txt
    |                  Per-function injection of nops after `multu` for
    |                  hand-coded-asm functions that have specific multu pacing.
    v
[regfix]               python3 tools/regfix.py  (reads regfix.txt)
    |                  Per-function assembly stream rewrites: register swaps,
    |                  substs, reorders, delete/insert, fill_delay, drain_delay.
    |                  The main lever for closing the last register-allocation
    |                  and scheduling diffs that source-level changes can't.
    v
[regfix_stage2]        regfix_stage2.txt
    |                  Second pass — needed for label-aware substitutions that
    |                  reference labels created during the first regfix pass.
    v
[asmfix]               tools/asmfix.py  (reads asmfix.txt)
    |                  Pre-final-assembly source patches: label renames,
    |                  replace_first regex substs, delete_between / insert_before
    |                  for split-rodata / shared-jtbl cases, and the
    |                  replace_with_asmfile bridge for functions still being
    |                  retired to pure C.
    v
[as]                   mipsel-linux-gnu-as -march=r3000 -mtune=r3000 -O1 -G0
    |
    v
build/src/<name>.o
```

The full command in `Makefile` (slightly reformatted):

```
$(CPP) ... | $(CC1) ... | $(PROLOGUE_FIX) | $(MASPSX) ... |
  $(fix_lwl) | $(rodata_align_fix) | $(MULTU_PAD) |
  $(REGFIX) | $(REGFIX_STAGE2) | $(ASMFIX) |
  $(AS) -o $@
```

### Assembly pipeline (per `.s` file)

```
asm/<name>.s   ->   mipsel-linux-gnu-as -O1 -G0   ->   build/asm/<name>.o
```

No post-passes; assembly files contain what splat extracted from the original binary verbatim.

### Link and finalization

```
build/src/*.o + build/asm/*.o
    |
    v
mipsel-linux-gnu-ld -nostdlib --no-check-sections
  -T bb2.ld
  -T undefined_funcs_auto.txt    # auto-generated by splat (PsyQ entry points)
  -T undefined_syms_auto.txt     # auto-generated by splat (data references)
  -T named_syms.txt              # hand-curated semantic aliases
    |
    v
build/bb2.elf
    |
    v
mipsel-linux-gnu-objcopy -O binary -j .main   ->   build/bb2.bin
    |
    v
python3 tools/make_psexe.py disc/SLUS_006.63 build/bb2.bin   ->   build/bb2.exe
    |                  (prepends original 0x800-byte PS-EXE header)
    v
sha1sum -c bb2.sha1   ->   "OK: bb2 matches!"
```

## The splat split

The binary is split into per-function assembly files and per-segment data files using [splat](https://github.com/ethteck/splat), configured by [`splat.yaml`](../splat.yaml). Running `python3 -m splat split splat.yaml` (wrapped by `make setup`) regenerates:

| Output | Contents |
|---|---|
| `asm/funcs/<func>.s` | One file per identified function (1,433 files currently). As functions are decompiled, they move to `src/*.c`; their `asm/funcs/` file remains but is no longer linked. |
| `asm/data/*.rodata*.s`, `asm/data/*.data.s` | Constant tables, strings, jump tables, initialized globals — anything that isn't code. Split into pre/post chunks where C objects need to interleave with asm objects. |
| `asm/6CAC.s`, `asm/text1a.s`, `asm/text1b.s`, etc. | Container files that `#include` individual `asm/funcs/<func>.s` files. These ensure `splat` keeps a stable file layout even as individual functions move to C. |
| `bb2.ld` | The linker script (regenerated by splat, then hand-edited for `RODATA_ALIGN2_FILES` and the BB2-specific object interleaving). |
| `undefined_syms_auto.txt`, `undefined_funcs_auto.txt` | Symbol address tables for PsyQ entry points and external data references that the linker needs. |

The split is governed by a few key parameters in `splat.yaml`:

- `vram: 0x80010000` — load address.
- `gp_value: 0x800A30CC` — sets the GP register, needed for GP-relative addressing.
- `section_order: [".rodata", ".text", ".data", ".bss"]` — PsyQ convention.
- `subalign: 2` — minimum 4-byte alignment for sections.
- `find_file_boundaries: True` — splat heuristically guesses where files boundaries were in the original build.

Once split, the linker script `bb2.ld` interleaves C objects with their corresponding asm chunks to preserve the original layout. For example, the `.rodata` segment of `src/code6cac.c` is placed between `asm/data/800.rodata_pre.s` and `asm/data/800.rodata_post.s` — those two asm files are the original `.rodata` segment with a hole carved out for where `code6cac.c`'s rodata goes.

This sandwich-and-interleave layout is the price of partial decomp. As more `.c` files are added, the linker script grows correspondingly. There are three sandwich techniques used in the project (monolithic split, sandwich split, C-file split); details in [`MATCHING.md`](MATCHING.md) under "CU split for jtbl interposition."

## The post-pass tools

The pipeline runs three custom post-passes on every C file's assembly between `cc1` and `as`: **regfix**, **asmfix**, and **maspsx-helpers** (`prologue_fix`, `fix_lwl`, `multu_pad`, sed). They exist because GCC 2.7.2 and ASPSX 2.34 are not literally identical to GCC + GNU as in all respects, and because closing the last few percent of byte-matches requires per-function surgery that can't be expressed in C.

### regfix.txt — per-function assembly rewrites

[`regfix.txt`](../regfix.txt) is a line-based config (one rule per line, `func: op args [@ idx]`) that drives `tools/regfix.py`. The tool re-parses maspsx output and applies the rules in a fixed phase order:

```
1. swap (bidirectional register rename)
2. subst (regex replace)
3. fill_delay (move source insn into a nop delay slot)
4. drain_delay (move delay-slot insn before its branch)
5. delete (remove instruction at idx)
6. insert (add before idx, post-delete numbering)
7. insert_after (add after idx, post-insert numbering)
8. reorder (rearrange a range of instructions)
9. insert_label (add a label without an instruction)
```

Each rule targets one function. The indices are 0-based per-function, counting only TEXT instructions (not directives, labels, comments, or `.word` data). Phase ordering matters: substs use original indices, deletes use original indices, inserts use post-delete indices, reorders use post-insert indices.

Example:
```
# cdrom_FramesToBcd: GCC picks t2 ($10) for mfhi temp, target uses t3 ($11)
cdrom_FramesToBcd: $10 <-> $11

# PutShadowRmd: v0/v1 swapped in two regions
PutShadowRmd: $2 <-> $3 @ 1-5
PutShadowRmd: $2 <-> $3 @ 14-21
```

`regfix.txt` currently has ~5,500 rules across the project. The file is large because matching is hard; every commit that adds a function typically adds 0–30 rules.

A second pass (`regfix_stage2.txt`) runs immediately after `regfix.txt`. It's needed because some rules need labels created by the first pass.

### asmfix.txt — pre-final-assembly source patches

[`asmfix.txt`](../asmfix.txt) is the next post-pass after regfix, applied by `tools/asmfix.py`. It handles operations that work on the assembly text as a whole rather than instruction-by-instruction:

| Operation | Purpose |
|---|---|
| `rename "old" "new"` | Replace one label name with another (word-boundary-safe regex). |
| `replace_first "regex" "replacement"` | One-shot regex substitution. |
| `delete_between "start_regex" "end_regex"` | Remove a range of text. |
| `insert_before "match_regex" "new_text"` | Insert text before a matching line. |
| `replace_with_asmfile "path/to/.s"` | **Bridge:** replace the entire function body with raw asm from a file. The escape hatch for functions not yet retired to C. |

The `replace_with_asmfile` form is the project's bridge mechanism. While ~148 functions remain bridged, those entries substitute hand-disassembled asm at build time and the C body in `src/*.c` is dead code. They count as work-in-progress, not matched — see the retirement workflow in [`CONTRIBUTING.md`](../CONTRIBUTING.md).

Bridged functions never reach the assembler with their C body. The substitution happens at the asmfix stage, so `make` succeeds and SHA1 matches — the bridge is invisible to the build outputs.

### inline_asm_canonical.txt — authorized inline asm

[`inline_asm_canonical.txt`](../inline_asm_canonical.txt) is a list of functions whose original assembly was hand-written and cannot be expressed in GCC 2.7.2 C. These functions are allowed to keep file-scope `__asm__()` blocks in their `.c` files as their final form.

Authorized categories:

- **PSX BIOS A/B/C-vector trampolines** (3-instruction sequences with the function number in `$t1` in the `jr` delay slot — no C analog).
- **Hand-coded math kernels** (sin/cos rotation, 3-axis Euler rotation — show evidence of hand-scheduled multu pacing and INT_MIN guard idioms).
- **Custom calling conventions** (`$s0` passed in without prologue save).
- **GTE primitives** that don't have a `gte_*()` macro in PsyQ's `include/gte.h`.

Tools that scan for `inline_asm_debt` skip names listed here. Adding to the list requires per-function evidence (the engine's `canonical` gate routes each function ASM-region / ASM-STRUCTURAL / C and provides the diagnostic signal).

### Other post-passes

Smaller per-function or per-file post-passes:

| Tool | Purpose | Driven by |
|---|---|---|
| `tools/prologue_fix.py` | Swaps `sw $ra` into delay slots of first conditional branch in certain functions. | `tools/delay_slot_ra_funcs.txt` |
| `tools/fix_lwl.py` | XORs `lwl/lwr/swl/swr` offsets with 3 to convert from GCC's big-endian byte ordering to PS1's little-endian. | Per-file `FIX_LWL_FILES` in `Makefile` |
| `tools/multu_pad.py` | Injects fixed-position nops after `multu` for hand-coded-asm functions that have specific multu pacing. | `multu_pad_funcs.txt` |
| sed rodata-align fix | Downgrades `.align 3` (8-byte) to `.align 2` (4-byte) for switch tables in split rodata files. | Per-file `RODATA_ALIGN2_FILES` in `Makefile` |

These are all conceptually similar to regfix and asmfix: per-function deterministic transformations applied at fixed points in the pipeline to coax the GNU toolchain into emitting the same bytes ASPSX 2.34 originally did.

## The C/asm interleaving model

The original game was built from many `.c` files in a specific order. Splat doesn't know that order — it sees a final linked binary. The project reconstructs it incrementally:

1. **Initially:** every function lives in `asm/funcs/<func>.s`. The linker script lists them all in (heuristic) address order.
2. **As functions are decompiled:** the `INCLUDE_ASM("asm/funcs", <func>)` stub (or, later, the inline `__asm__()` block) in `src/<file>.c` is replaced with a C body. The build now compiles the C, but its `asm/funcs/<func>.s` still exists (orphaned — no longer linked).
3. **As C files are populated:** they need to be interleaved with the asm chunks in `bb2.ld` to preserve the original section ordering. The link order in `bb2.ld` is the **source of truth** for where each C file's text/data/bss/rodata ends up.

There are 21 `src/*.c` files today, covering ~46,900 lines of C. The file boundaries roughly match those of the original (inferred from rodata gaps and call clustering), but in some cases a single original `.c` was split into several here (`code6cac.c` + `code6cac_b.c` + `code6cac_b2.c` + `code6cac_c.c` + ... + `code6cac_c2.c`) because intervening rodata or jump tables required carving out separate compilation units.

The full map of "what lives in each `.c` file" is in [`SUBSYSTEM_MAP_2026-05-12.md`](handoffs/2026-05-12-subsystem-map.md).

## Key globals and address-space conventions

- All BB2 addresses are MIPS virtual addresses in **KSEG0** (`0x80000000`+). The leading `8` in addresses like `0x80012345` is part of the address, not a tag.
- The PSX has no MMU; KSEG0 is just RAM-with-cache. Same physical memory as KUSEG; same RAM as KSEG1 (uncached mirror).
- **Scratchpad RAM** at `0x1F800000`–`0x1F8003FF` is 1 KB of fast on-CPU SRAM, used by some functions (e.g., the `cpu_check_tubazeri` cluster) for hot temporaries. Accessed directly with `lw`/`sw` to those addresses.
- **GP register** is `0x800A30CC`. Loads addressed `gp + signed_offset` can reach `0x800A2CCC`–`0x800A48CC` in one instruction instead of two (`lui + addiu`). The `sdata_*` config files control which symbols use GP-relative addressing.
- **Stack** grows down from `0x801FFFF0` toward `0x80100000`. BB2's stack usage is bounded (no deep recursion).

## Why so many post-passes?

A reasonable question: why all the regfix/asmfix/prologue_fix/fix_lwl/multu_pad machinery? Why not just rely on a faithful PsyQ-era toolchain?

Three reasons:

1. **The original `cc1psx` and `aspsx` are not freely redistributable in source form.** The project uses `decompals/mips-gcc-2.7.2`, which is the same GCC 2.7.2 SN Systems fork built from publicly-released sources, and `mkst/maspsx`, which emulates ASPSX 2.34's behavior on top of GNU `as`. Both are byte-identical to PsyQ for most inputs but diverge on edge cases.

2. **Even with a perfect toolchain, GCC 2.7.2's register allocator and scheduler are sensitive to source-level details that the original developer could observe and tune.** When you can't see the developer's exact source, you have to reverse-engineer it from the output. The C variants you'd need to write to match every quirk are sometimes ugly, sometimes impossible. The regfix layer lets you write reasonable C and patch the differences in assembly — a tradeoff between source-code quality and tooling complexity.

3. **Some original functions were hand-written assembly, not C.** The `inline_asm_canonical.txt` mechanism authorizes them; the alternative would be writing C with embedded inline asm that GCC mangles. The asmfix `replace_with_asmfile` bridge serves the in-progress retirement of those (and other) functions.

The end state is: every function in `src/*.c` is real C, regfix.txt is mostly small register-naming fixups, asmfix.txt has zero `replace_with_asmfile` lines, and `inline_asm_canonical.txt` is a small list of well-justified exceptions.

## Further reading

| Topic | File |
|---|---|
| The matching playbook (techniques, recipes, gotchas) | [`MATCHING.md`](MATCHING.md) |
| Every standalone tool | [`TOOLS.md`](TOOLS.md) |
| Terminology (PsyQ, MIPS, decomp, BB2-specific) | [`GLOSSARY.md`](GLOSSARY.md) |
| Per-file source content map | [`handoffs/2026-05-12-subsystem-map.md`](handoffs/2026-05-12-subsystem-map.md) |
| Build / setup walkthrough | [`../BUILD.md`](../BUILD.md) |
| Internal Claude Code agent instructions (useful context for humans too) | [`../CLAUDE.md`](../CLAUDE.md) |
