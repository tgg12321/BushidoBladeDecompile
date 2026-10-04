# Bushido Blade 2 (SLUS-00663) — Matching Decompilation

A matching decompilation of *Bushido Blade 2* (SquareSoft / Lightweight, 1998, PlayStation 1, NTSC-U). The build produces a byte-identical copy of the original `SLUS_006.63` executable
(SHA1 `62efab4f73f992798c43e8c730aa43baa10bb4fa`).

Matching decompilation rebuilds the original program from human-written C that compiles, byte-for-byte, to the same machine code the original release shipped with. The result is a working source tree for a game whose source code was never published.

## Status

The build verifies SHA1 against the original. 1,483 of 1,484 functions are complete (the worklist
first reached zero on 2026-10-02): 1260 are **COMPLETED-C** (pure C, zero cheats, byte-identical) and
223 are **COMPLETED-INLINE-ASM-CANONICAL** (genuinely hand-written original assembly — BIOS trampolines,
GTE primitives, PsyQ library asm, custom calling conventions — listed in
[`inline_asm_canonical.txt`](inline_asm_canonical.txt)). 136 of the canonical ones stay as
`INCLUDE_ASM("asm/funcs", <func>);`, their final form. A function that is not yet matched is
**INCOMPLETE**: queued in [`engine/queue.json`](engine/queue.json) and committed as `INCLUDE_ASM`
until it matches (asm-until-matched ruling, 2026-08-19). The one INCOMPLETE function is `_SendPAD`,
split out of `FlushCache`'s listing on 2026-10-03. The project is now in its cleanup / readability
phase. Counts: `python3 tools/check_completion_integrity.py`; dated snapshot in
[`docs/STATUS.md`](docs/STATUS.md).

## Project goals (1.0 release criteria)

1. **Pure C source.** Every function compiled from C in `src/` — no build-rule rewrites. (`INCLUDE_ASM` is the mandated representation of a not-yet-matched function — asm-until-matched ruling 2026-08-19 — and the final form of canonical hand-written asm; since 2026-10-02 no function is unmatched.) The only inline `__asm__()` permitted is for genuinely hand-coded original assembly (BIOS calling conventions, GTE coprocessor ops, a handful of custom-ABI math kernels). Authorized list: [`inline_asm_canonical.txt`](inline_asm_canonical.txt).
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
   make              # builds and prints "OK: bb2 matches!" on success
   ```

If the final line is `OK: bb2 matches!`, you have a byte-identical rebuild.

## Repository layout

```
.
|-- asm/                       # Disassembly: header.s (EXE header), asm/funcs/<name>.s, data in asm/data/
|-- asm/funcs/                 # Per-function .s files (~1,438): reference target listing for every function, kept after completion
|-- build/                     # All build artifacts (gitignored)
|-- disc/                      # Extracted disc filesystem (gitignored; reproduced by extract_iso.py)
|-- docs/                      # Contributor / maintainer documentation
|-- include/                   # Headers: game.h (game types), bb2.h (game declarations), psxsdk/lib*.h (Sony), common.h, gte.h
|-- Kengo/                     # Sister-engine reference: Kengo (PS2) debug symbols, ~2,500 named functions
|-- memory/                    # TRACKED pipeline state: grind ledgers (memory/grind/<func>/), WIP checkpoints, closer research
|-- src/main/                  # Game C, one file per translation unit (subsystem or ROM-offset names)
|-- src/main/psxsdk/<lib>/     # PsyQ library C/asm, one file per Sony module
|-- tools/                     # Build pipeline (maspsx, prologue_fix, decomp-permuter, engine/, grinder/, ...)
|-- engine/                    # The workflow spine: queue, canonical gate, sandbox, retire, verify-oracle
|-- engine/queue.json          # Canonical ordered work list (`& tools/wteng.ps1 main queue status`)
|-- splat.yaml                 # splat split configuration
|-- bb2.ld                     # Linker script (generated by splat, hand-edited)
|-- Makefile                   # Build pipeline
|-- inline_asm_canonical.txt   # Functions where file-scope __asm__ is the authorized form
|-- named_syms.txt             # Linker aliases for renamed symbols
|-- symbol_addrs.txt           # Manual symbol address overrides for splat
|-- CLAUDE.md                  # Internal instructions for the Claude Code agent
|-- BUILD.md / CONTRIBUTING.md # Setup and contribution guides
```

Subsystem map (what lives where, by address): [`docs/engine/README.md`](docs/engine/README.md).

## Documentation

| File | Purpose |
|---|---|
| [`BUILD.md`](BUILD.md) | End-to-end build setup (WSL toolchain, disc extraction, first build, troubleshooting) |
| [`CONTRIBUTING.md`](CONTRIBUTING.md) | How to contribute (short) |
| [`docs/DECOMP_WORKFLOW.md`](docs/DECOMP_WORKFLOW.md) | The operating manual: what "done" means, the per-function loop, review |
| [`docs/MATCHING.md`](docs/MATCHING.md) | Matching primer; the living technique catalog is `.claude/rules/` |
| [`docs/ARCHITECTURE.md`](docs/ARCHITECTURE.md) | Build pipeline, splat split, PS1 memory map, EXE layout |
| [`docs/ORACLE-COMPILER.md`](docs/ORACLE-COMPILER.md) | The build compiler: identity, recipe, manifest |
| [`docs/GLOSSARY.md`](docs/GLOSSARY.md) | Terminology: PsyQ, MIPS, decomp, BB2-specific terms |
| [`docs/engine/`](docs/engine/README.md) | What the game engine does, per subsystem |
| [`docs/formats/`](docs/formats/README.md) | Disc asset formats |
| [`docs/naming/`](docs/naming/README.md) | Function/data naming census and evidence |
| [`docs/grind/`](docs/grind/) | Grinder owner-audit surfaces: `decisions.md` (Judge rulings), `journal.md` (one line per session), `borderline.md` (logged policy questions/grants), `owner-rulings-2026-09-26.md` (verbatim owner answers) |
| [`docs/STATUS.md`](docs/STATUS.md) / [`docs/HISTORY.md`](docs/HISTORY.md) | Current snapshot / the one timeline |
| [`CLAUDE.md`](CLAUDE.md) / [`AGENTS.md`](AGENTS.md) | Operating instructions for the agents that do most of the decomp work |

Older docs, handoffs, plans and campaign reports were removed 2026-10-01; they resolve at git tag
`pre-slim-2026-10-01`.

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
