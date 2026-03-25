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
- `tools/decomp-permuter/` — [decomp-permuter](https://github.com/simonlindholm/decomp-permuter) for auto-matching C code
- `tools/permuter_compile.sh` — BB2 compilation script for the permuter
- `tools/permuter_setup.sh` — helper to set up a permuter run for a given function

## Using decomp-permuter

The permuter automatically tries thousands of C code permutations to find byte-matching output. It is useful when your decompilation is structurally correct but GCC's register allocation or instruction ordering differs.

### Quick Start (in WSL)

```bash
cd /mnt/c/Users/Trenton/Desktop/"Bushido Blade 2 Decompile"
source .venv/bin/activate

# 1. Set up a permuter directory for a function
bash tools/permuter_setup.sh func_80048A7C src/text1b.c

# 2. Edit permuter/func_80048A7C/base.c with your decompilation attempt

# 3. Run the permuter (use -j4 for 4 threads, --stop-on-zero to halt on match)
python3 tools/decomp-permuter/permuter.py permuter/func_80048A7C -j4 --stop-on-zero

# 4. Debug mode (shows side-by-side diff and score breakdown)
python3 tools/decomp-permuter/permuter.py permuter/func_80048A7C --debug
```

### Directory Structure

Each permuter run lives in `permuter/<func_name>/` with:
- `base.c` — your decompilation attempt (edit this)
- `target.s` / `target.o` — the original assembly to match against
- `compile.sh` — compilation script (auto-copied from tools/)
- `settings.toml` — permuter settings

### Manual Permutation Macros

Add these to `base.c` to guide the search:
- `PERM_GENERAL(a, b, ...)` — try each alternative
- `PERM_RANDOMIZE(code)` — allow random mutations within a block
- `PERM_LINESWAP(lines)` — try all orderings of lines
- `PERM_VAR(name, value)` / `PERM_VAR(name)` — define/use meta-variables

### Notes
- The `PERMUTER` preprocessor define is set during compilation, which makes `INCLUDE_ASM` expand to nothing (see `include/include_asm.h`)
- `permuter/` directories are gitignored
- Override compiler flags: `CC_FLAGS="-O2 -G8 ..." permuter/func_XXX/compile.sh ...`

## Cross-Session Coordination

Multiple Claude Code sessions may work on this project at the same time. To prevent conflicts, all sessions use `CLAIMS.md` as a shared coordination ledger.

### Before Starting Work

1. **Read `CLAIMS.md`** to see what files/scopes are already claimed by other sessions.
2. **Do not work on claimed files.** If the file you need is claimed, either:
   - Pick a different file to work on
   - Ask the user if the other session is still active
3. **Register your claim** by adding a row to the table in `CLAIMS.md`:
   ```
   | src/main.c | decompile Tier 1-2 stubs | 2026-03-24 18:30 | active |
   ```
4. **Shared files** (`bb2.ld`, `Makefile`, `undefined_syms_auto.txt`, `undefined_funcs_auto.txt`) should be claimed as `scaffolding:bb2.ld` etc. Only one session may hold a scaffolding claim at a time.

### While Working

- If you spawn sub-agents via worktrees, list each agent's file in your claim (e.g., `src/gpu.c, src/ings.c` under one session).
- Update the status column if your scope changes.

### When Done

- Update your claim status to `done` or remove the row.
- If you created branches that need merging, note them:
  ```
  | src/gpu.c | 5 funcs decompiled | 2026-03-24 19:00 | done — branch: decomp/gpu-c |
  ```

### Stale Claims

If a claim has no matching active session (e.g., the terminal was closed), it is stale. Before claiming a file held by a stale entry:
1. Check `git status` and `git branch` for in-progress work from that session
2. If there are uncommitted changes or unmerged branches, ask the user before overwriting
3. Remove the stale claim row

## Parallel Agent Protocol

When spawning multiple agents to work on this project simultaneously, follow these rules to prevent conflicts.

### Architecture: Worktree Isolation

Each agent runs in its own git worktree (`isolation: "worktree"`), giving it an independent copy of the repo on its own branch. This means:
- Agents cannot corrupt each other's files
- Each agent can build and verify independently
- Results are merged one branch at a time by the orchestrator

### Two-Phase Workflow

**Phase A — Scaffolding (sequential, orchestrator only):**
Changes to shared infrastructure — `bb2.ld`, `Makefile`, `undefined_syms_auto.txt`, `undefined_funcs_auto.txt`, `splat.yaml`, or creating new C files — must be done by the orchestrator on main BEFORE spawning parallel agents. Never let multiple agents modify these files.

**Phase B — Decompilation (parallel, isolated agents):**
Each agent owns exactly ONE C source file. It decompiles INCLUDE_ASM stubs into C, builds, and verifies. Multiple agents can run in parallel because they're in separate worktrees.

### Agent Rules

1. **WSL-only file writes.** All file modifications must go through WSL (e.g., `python3 -c` or `sed -i` via `wsl bash -c`). The Windows-side Edit/Write tools introduce CRLF line endings that break the build.
2. **One C file per agent.** Each agent declares which `.c` file it owns. It may only modify that file.
3. **No destructive git commands.** Agents must NEVER run `git checkout`, `git restore`, `git stash`, `git clean`, or `git reset`. To undo a change, rewrite the specific file content.
4. **No `make clean`.** Agents use incremental builds only (`make`). Only the orchestrator runs `make clean` for final verification.
5. **Add missing symbols to `undefined_syms_auto.txt` if needed.** If a decompiled function references a global not yet in the symbol files, the agent should note it in its report. The orchestrator adds it on main before merging.

### Merge Workflow

After agents complete:
1. Orchestrator verifies each branch builds independently
2. Merge branches into main one at a time with `git merge --no-ff`
3. After each merge, run `make clean && make` to verify combined build
4. If a merge causes a mismatch, investigate that branch's changes before proceeding

### Build Verification Command

Agents should use incremental builds for speed:
```bash
wsl bash -c 'cd /mnt/c/Users/Trenton/Desktop/"Bushido Blade 2 Decompile" && source .venv/bin/activate && make 2>&1 | tail -5'
```
Expected success output: `OK: bb2 matches!`

## Key Conventions for PS1 Decomp

- All addresses are MIPS virtual addresses in KSEG0 (`0x80000000`+)
- `0x1F800000`–`0x1F8003FF` = scratchpad RAM (fast 1KB SRAM)
- The PsyQ SDK provides BIOS calls, GPU, SPU, CD, controller, and memory card APIs
- PS1 uses little-endian MIPS I instruction set
- Struct padding and alignment must match the PsyQ compiler's behavior
- When adding known symbol names, add them to `symbol_addrs.txt` and re-run splat
