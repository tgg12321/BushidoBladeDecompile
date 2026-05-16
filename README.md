# Bushido Blade 2 (SLUS-00663) — Matching Decompilation

A matching decompilation of *Bushido Blade 2* (SquareSoft / Lightweight, 1998, PlayStation 1, NTSC-U). The build produces a byte-identical copy of the original `SLUS_006.63` executable
(SHA1 `62efab4f73f992798c43e8c730aa43baa10bb4fa`).

Matching decompilation rebuilds the original program from human-written C that compiles, byte-for-byte, to the same machine code the original release shipped with. The result is a working source tree for a game whose source code was never published.

## Status

The build currently verifies SHA1 against the original. About 1,030 of the 1,410 identified functions exist as real C; the rest are still authored as inline assembly or as `replace_with_asmfile` bridges that produce the right bytes by substituting hand-disassembled MIPS at build time. Both of those forms count as "not done" — the project goal is pure C plus a small, audited set of canonically hand-assembled functions (BIOS trampolines, GTE primitives, custom calling conventions).

| Metric | Value |
|---|---|
| Total functions identified | 1,410 |
| Functions with semantic names | ~530 (~38%) |
| C source files | 21 (`src/*.c`, ~46,900 lines) |
| Active decomp queue | 62 |
| `replace_with_asmfile` retirement queue | 147 |
| Authorized canonical inline-asm | ~80 (BIOS trampolines, GTE primitives, hand-coded asm) |
| Build SHA1 | matches original |

Live counts are kept up to date in [`WORK_QUEUE.md`](WORK_QUEUE.md).

## Project goals (1.0 release criteria)

1. **Pure C source.** Every function compiled from C in `src/*.c` — no `replace_with_asmfile` bridges, no `INCLUDE_ASM` stubs. The only inline `__asm__()` permitted is for genuinely hand-coded original assembly (BIOS calling conventions, GTE coprocessor ops, a handful of custom-ABI math kernels). Authorized list: [`inline_asm_canonical.txt`](inline_asm_canonical.txt).
2. **Byte-identical to original.** Every commit must rebuild to SHA1 `62efab4f…`. The match is preserved by a per-function build pipeline (see [`docs/ARCHITECTURE.md`](docs/ARCHITECTURE.md)).
3. **Named and organized.** Functions, globals, structs, and constants given semantic names where evidence supports it. Subsystem boundaries clear enough to navigate.
4. **Mod-ready foundation.** Once 1.0 lands, this source tree should be usable as the starting point for translation patches, gameplay mods, ports, and engine research.

## Tech specs

| Component | Detail |
|---|---|
| Platform | Sony PlayStation 1 (R3000A, 2 MB RAM) |
| Developer | Lightweight (SquareSoft subsidiary, 40%) |
| Publisher | SquareSoft (Japan: Square Co., Ltd.) |
| Original release | 1998 (NA: SLUS-00663) |
| Engine | "Marionation" (Lightweight proprietary; later reused for *Kengo* on PS2) |
| Build date | Fri Aug 7 22:26:32 1998 (embedded in `.rodata` at `0x8001004C`) |
| Compiler | GCC 2.7.2 (SN Systems fork / `cc1psx`) |
| Assembler | ASPSX ~2.34 |
| SDK | PsyQ 3.5 (DTL-S3000), copyright 1993–1997 |
| Linked PsyQ libraries | `libgpu`, `libcd`, `libapi`, `libspu`, MDEC (overlay) |

EXE layout:

| Property | Address / Value |
|---|---|
| Load address | `0x80010000` |
| Entry point | `0x800836EC` |
| Stack pointer | `0x801FFFF0` |
| GP register | `0x800A30CC` |
| Text + data size | `0x93800` (604,160 bytes) |
| EXE file size | 606,208 bytes (header 0x800 + image) |
| FMV overlay | `disc/STR/MOVOVL.EXE` (122,880 bytes, loads at `0x801D8800`) |

See [`docs/ARCHITECTURE.md`](docs/ARCHITECTURE.md) for the full build-pipeline graph and toolchain notes.

## Quick start

1. Set up a build environment (Windows + WSL Ubuntu 24.04, or pure Linux). Full walkthrough: [`BUILD.md`](BUILD.md).
2. Place an unmodified North American disc image as `Bushido Blade 2 (USA).bin` + `.cue` in the project root.
3. Extract the disc, build, verify:
   ```bash
   python3 tools/extract_iso.py
   make setup        # run splat once to generate asm/
   make              # builds and prints "OK: bb2 matches!" on success
   ```

If the final line is `OK: bb2 matches!`, you have a byte-identical rebuild.

## Repository layout

```
.
|-- asm/                       # Disassembly. .text in 6CAC.s + asm/funcs/<name>.s; data in asm/data/
|-- asm/funcs/                 # Per-function .s files (1,433 total); shrinks as src/ grows
|-- build/                     # All build artifacts (gitignored)
|-- disc/                      # Extracted disc filesystem (gitignored; reproduced by extract_iso.py)
|-- docs/                      # Contributor / maintainer documentation
|-- include/                   # Project headers (common.h, gte.h, psx.h, game.h, etc.)
|-- Kengo/                     # Sister-engine reference: Kengo (PS2) debug symbols, ~2,500 named functions
|-- memory/                    # (gitignored copy of) Claude-internal feedback files; see CLAUDE.md
|-- src/                       # Decompiled C source — 21 files, ~46,900 lines
|-- tools/                     # Build pipeline (regfix, asmfix, maspsx, dc.sh, decomp-permuter, ...)
|-- WORK_QUEUE.md              # Canonical ordered work list
|-- splat.yaml                 # splat split configuration
|-- bb2.ld                     # Linker script (generated by splat, hand-edited)
|-- Makefile                   # Build pipeline
|-- regfix.txt                 # Post-maspsx assembly rewrites (per-function rules)
|-- asmfix.txt                 # Pre-final-assembly source patches (slices and bridges)
|-- inline_asm_canonical.txt   # Functions where file-scope __asm__ is the authorized form
|-- named_syms.txt             # Linker aliases for renamed symbols
|-- symbol_addrs.txt           # Manual symbol address overrides for splat
|-- CLAUDE.md                  # Internal instructions for the Claude Code agent
|-- BUILD.md / CONTRIBUTING.md # Setup and contribution guides
```

Per-file C source content map: [`SUBSYSTEM_MAP_2026-05-12.md`](SUBSYSTEM_MAP_2026-05-12.md).

## Documentation

| File | Purpose |
|---|---|
| [`BUILD.md`](BUILD.md) | End-to-end build setup (WSL toolchain, disc extraction, first build, troubleshooting) |
| [`CONTRIBUTING.md`](CONTRIBUTING.md) | Work-queue model, per-function workflow, escalation ladder, PR conventions |
| [`docs/ARCHITECTURE.md`](docs/ARCHITECTURE.md) | Build pipeline, splat split, regfix/asmfix model, PS1 memory map, EXE layout |
| [`docs/TOOLS.md`](docs/TOOLS.md) | Tool catalog: every `dc.sh` subcommand, every standalone Python tool, grouped by purpose |
| [`docs/MATCHING.md`](docs/MATCHING.md) | Matching techniques distilled from the playbook (penalty routing, C-side tricks, regfix patterns, recipes) |
| [`docs/GLOSSARY.md`](docs/GLOSSARY.md) | Terminology: PsyQ, MIPS, decomp, BB2-specific terms |
| [`SUBSYSTEM_MAP_2026-05-12.md`](SUBSYSTEM_MAP_2026-05-12.md) | What lives in each `src/*.c` file and at what address |
| [`CLAUDE.md`](CLAUDE.md) | Operating instructions for the Claude Code agent that does most of the decomp work |

## Credits and acknowledgements

- **Square Co., Ltd.** and **Lightweight Co., Ltd.** for the original game and the Marionation engine.
- **Sony Computer Entertainment** for the PsyQ SDK that built it.
- **[splat](https://github.com/ethteck/splat)** (Ethan "ethteck" Roseman et al.) for the binary splitter that initialized this project.
- **[decomp-permuter](https://github.com/simonlindholm/decomp-permuter)** (Simon Lindholm) for the C permutation search that finds matching codegen variants.
- **[maspsx](https://github.com/mkst/maspsx)** (Matt "mkst" Stevenson) for the ASPSX compatibility layer that makes GNU `as` emit PsyQ-equivalent encodings.
- **[mips-gcc-2.7.2](https://github.com/decompals/mips-gcc-2.7.2)** ([decompals](https://github.com/decompals)) for the rebuilt PsyQ-era GCC cross-compiler.
- **Kengo Project (PS2)** for the sister-engine debug symbols (~2,482 named functions extracted via [`ccc`](https://github.com/chaoticgd/ccc) / `stdump`) used as a naming reference.
- **PS1/PS2 Decompilation community** on Discord — methodology, peer review, recipes. [decomp.me](https://decomp.me/) for collaborative matching, [decomp.dev](https://decomp.dev/) for progress tracking, [decomp.wiki](https://decomp.wiki/) for documented techniques.
- Reference projects whose tooling and conventions influenced this one: [sotn-decomp](https://github.com/Xeeynamo/sotn-decomp), [rood-reverse](https://github.com/ser-pounce/rood-reverse) (Vagrant Story), [ff7-decomp](https://github.com/Xeeynamo/ff7-decomp), [silent-hill-decomp](https://github.com/Vatuu/silent-hill-decomp), [chrono-cross-decomp](https://github.com/jdperos/chrono-cross-decomp), [psy-q-decomp](https://github.com/sozud/psy-q-decomp).

## Legal

*Bushido Blade 2* is a copyrighted work, © 1997–1998 SquareSoft / Lightweight. **This repository contains no original game assets**: no executable bytes, no audio, no textures, no models, no scripts. The disc image itself is intentionally gitignored — you must legally obtain your own NTSC-U disc and rip it locally before the build can produce anything.

This project is decompilation research. The C source here is hand-written by contributors, not extracted from copyrighted material, and is licensed for use as research and educational reference. By contributing you agree that your contributions are similarly hand-written, derived from observation of the published game's externally-visible behaviour, and free of any direct copy of leaked or otherwise non-public source code.

If you are a current rights holder for *Bushido Blade 2* and have a concern about anything in this repository, please open an issue.
