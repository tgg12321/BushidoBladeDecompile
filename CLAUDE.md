# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This is a matching decompilation project for **Bushido Blade 2** (SLUS-00663), a PlayStation 1 fighting game published by SquareSoft (1998, North America). The goal is to produce C source code that compiles to a byte-identical copy of the original executable.

**Target platform:** PlayStation 1 (MIPS R3000A CPU, 2 MB RAM, PsyQ SDK)
**Developer:** Lightweight (40% Square subsidiary)
**Engine:** "Marionation" (Lightweight's proprietary engine, later reused for Kengo on PS2)
**Build date:** Fri Aug 7 22:26:32 1998

## Toolchain (confirmed from binary analysis)

- **PsyQ SDK 3.5** (DTL-S3000), copyright "1993-1997"
- **Compiler:** GCC 2.7.2 (SN Systems fork / cc1psx)
- **Assembler:** ASPSX ~2.34
- **Section order:** `.rodata → .text → .data → .bss` (PsyQ standard)
- Identified via CVS `$Id:` tags in linked PsyQ libraries:
  - `sys.c v1.129 1996/12/25` (libgpu), `bios.c v1.86 1997/03/28` (libcd), `intr.c v1.76 1997/02/12` (libapi)

## Executable Details

- **Main EXE:** `disc/SLUS_006.63` (606,208 bytes)
  - SHA1: `62efab4f73f992798c43e8c730aa43baa10bb4fa`
  - PS-X EXE format, load address `0x80010000`, entry point `0x800836EC`
  - Stack: `0x801FFFF0`, GP: `0x800A30CC`
  - Text+data size: `0x93800` (604,160 bytes), loads at file offset `0x800`
  - Code ends ~`0x8008D070`, data/tables from ~`0x8008D080` to `0x800A3800`
  - 1,410 functions identified by splat
- **Overlay EXE:** `disc/STR/MOVOVL.EXE` (122,880 bytes) — FMV/MDEC playback overlay
  - Loads at `0x801D8800`, entry `0x801DA084` — no overlap with main EXE
- **PsyQ libraries linked:** libgpu, libcd, libapi, libspu, MDEC decode (overlay)

## Splat Configuration

The binary is split using [splat](https://github.com/ethteck/splat) v0.39.0 via `splat.yaml`.

```bash
# Re-run the split (regenerates asm/, linker script, symbol files)
python -m splat split splat.yaml
```

### Current split layout
- `asm/data/800.rodata.s` — .rodata section (strings, jump tables, constants)
- `asm/6CAC.s` — .text section (1,410 functions, all assembly)
- `asm/data/7D920.data.s` — .data section (initialized globals)
- `bb2.ld` — generated linker script
- `undefined_syms_auto.txt` — auto-detected data symbol addresses
- `undefined_funcs_auto.txt` — auto-detected external function addresses
- `symbol_addrs.txt` — manually identified symbol names (add known names here)

## Disc Structure

Extracted to `disc/` via `tools/extract_iso.py` from the MODE2/2352 BIN/CUE image.

| Directory | Purpose |
|-----------|---------|
| `LOADSE/`, `LOADSE1/` | Stage geometry/data (STAGE00–STAGE37.BIN) + sound effect banks (.SE) |
| `MOTION/` | Character animation bundles (.BBM), plus WIN.DAT |
| `NDATA/` | Large packed data archive (NDATA.DAT ~55 MB + NDATA.INF index) |
| `TIM2D/` | 2D textures (.TIM), UI/menu data, sound banks (.BNK), selection screens |
| `U_PIC/` | Stage background images (STG00–STG29.BIN) |
| `XA_0/`, `XA_1/` | XA-ADPCM streamed audio (music, voice, endings) |
| `STR/` | FMV opening (OPENING.STR), title screen (TITLE.TIM), movie overlay (MOVOVL.EXE) |

## Build Commands

All build commands run inside WSL (Ubuntu 24.04):

```bash
# Enter project directory in WSL
cd /mnt/c/Users/Trenton/Desktop/"Bushido Blade 2 Decompile"
source .venv/bin/activate

make setup    # re-run splat (regenerate asm, linker script)
make          # build and verify SHA1 match
make clean    # remove build artifacts
```

Or from Git Bash: `wsl bash -c 'cd /mnt/c/Users/Trenton/Desktop/"Bushido Blade 2 Decompile" && source .venv/bin/activate && make'`

### Build pipeline (per C file)
```
mipsel-linux-gnu-cpp | cc1 (GCC 2.7.2) | maspsx (--aspsx-version=2.34) | mipsel-linux-gnu-as → .o
```
All objects linked with `mipsel-linux-gnu-ld`, stripped to binary via `objcopy`, then `make_psexe.py` prepends the original 0x800-byte PS-EXE header. Final SHA1 compared against original.

## Tools

- `tools/extract_iso.py` — extracts PS1 disc filesystem from BIN/CUE into `disc/`
- `tools/make_psexe.py` — constructs PS-X EXE from raw binary + original header
- `tools/setup_wsl.sh` — automated WSL toolchain installer
- `tools/maspsx/` — ASPSX compatibility layer (cloned from mkst/maspsx)
- `tools/gcc-2.7.2/` — PsyQ-era GCC cross-compiler (built from decompals/mips-gcc-2.7.2)

## Key Conventions for PS1 Decomp

- All addresses are MIPS virtual addresses in KSEG0 (`0x80000000`+)
- `0x1F800000`–`0x1F8003FF` = scratchpad RAM (fast 1KB SRAM)
- The PsyQ SDK provides BIOS calls, GPU, SPU, CD, controller, and memory card APIs
- PS1 uses little-endian MIPS I instruction set
- Struct padding and alignment must match the PsyQ compiler's behavior
- When adding known symbol names, add them to `symbol_addrs.txt` and re-run splat
