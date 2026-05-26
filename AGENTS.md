# AGENTS.md

Project-level facts for any AI coding agent (Claude Code, Codex, Cursor, Cline, Aider, Gemini CLI, Windsurf, Copilot, etc.) working on this repo. Tool-agnostic. See [agents.md](https://agents.md/) for the open standard.

Tool-specific guidance (hooks, skills, the active-marker workflow, memory system) lives in `CLAUDE.md` and is loaded only by Claude Code.

## Project overview

Matching decompilation of **Bushido Blade 2** (SLUS-00663) — a PS1 fighting game published by SquareSoft (1998, North America). The goal: C source that compiles to a byte-identical copy of the original executable.

- **Platform:** PlayStation 1 (MIPS R3000A, 2 MB RAM, PsyQ SDK)
- **Developer:** Lightweight (40% Square subsidiary)
- **Engine:** "Marionation" (Lightweight's proprietary engine, later reused for Kengo on PS2)
- **Build date:** Fri Aug 7 22:26:32 1998

## Toolchain (confirmed from binary analysis)

- **PsyQ SDK 3.5** (DTL-S3000), copyright "1993-1997"
- **Compiler:** GCC 2.7.2 (SN Systems fork / cc1psx)
  - Build uses the open-source port: `decompals/mips-gcc-2.7.2` (functionally equivalent for our purposes)
- **Assembler:** ASPSX ~2.34 (via the [maspsx](https://github.com/mkst/maspsx) compatibility layer)
- **Section order:** `.rodata → .text → .data → .bss` (PsyQ standard)
- Identified via CVS `$Id:` tags in linked PsyQ libraries:
  - `sys.c v1.129 1996/12/25` (libgpu)
  - `bios.c v1.86 1997/03/28` (libcd)
  - `intr.c v1.76 1997/02/12` (libapi)

## Executable details

**Main EXE:** `disc/SLUS_006.63` (606,208 bytes)
- SHA1: `62efab4f73f992798c43e8c730aa43baa10bb4fa`
- PS-X EXE format, load address `0x80010000`, entry point `0x800836EC`
- Stack: `0x801FFFF0`, GP: `0x800A30CC`
- Text+data size: `0x93800` (604,160 bytes), loads at file offset `0x800`
- Code ends ~`0x8008D070`, data/tables from ~`0x8008D080` to `0x800A3800`
- 1,410 functions identified by splat

**Overlay EXE:** `disc/STR/MOVOVL.EXE` (122,880 bytes) — FMV/MDEC playback overlay
- Loads at `0x801D8800`, entry `0x801DA084` — no overlap with main EXE

**PsyQ libraries linked:** libgpu, libcd, libapi, libspu, MDEC decode (overlay)

## Splat configuration

The binary is split using [splat](https://github.com/ethteck/splat) v0.39.0 via `splat.yaml`. Re-run via:

```bash
python -m splat split splat.yaml
```

This regenerates `asm/`, the linker script, and symbol files.

Current split layout:
- `asm/data/800.rodata.s` — .rodata section (strings, jump tables, constants)
- `asm/6CAC.s` — .text section (1,410 functions, all assembly)
- `asm/data/7D920.data.s` — .data section (initialized globals)
- `bb2.ld` — generated linker script
- `undefined_syms_auto.txt` — auto-detected data symbol addresses
- `undefined_funcs_auto.txt` — auto-detected external function addresses
- `symbol_addrs.txt` — manually identified symbol names (add known names here)
- `named_syms.txt` — additional named symbol assignments

## Disc structure

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

Per-format documentation in `docs/formats/`:
- `NDATA.md`, `BBM.md`, `TIM.md`, `STAGE_BIN.md`, `BNK.md`, `SE.md`, `XA.md`, `STR.md`, `MOVOVL.md`

## Build commands

All build commands run inside WSL (Ubuntu 24.04) for Linux toolchain compatibility:

```bash
cd /mnt/c/Users/Trenton/Desktop/"Bushido Blade 2 Decompile"
source .venv/bin/activate
make setup    # re-run splat (regenerate asm, linker script)
make          # build and verify SHA1 match
make clean    # remove build artifacts
```

From Git Bash on Windows host:
```bash
wsl bash -c 'cd /mnt/c/Users/Trenton/Desktop/"Bushido Blade 2 Decompile" && source .venv/bin/activate && make'
```

### PowerShell-first scripting (avoid the shell-nesting footgun)

Invoking WSL through a Windows-side agent nests multiple shells (Git Bash → wsl → bash, or
PowerShell → wsl → bash); every `$`, quote, and backslash is parsed at each layer, which silently
breaks inline `awk`/`sed`, shell-function definitions, heredocs, and hand-escaped quotes. Standards:

- **Run engine commands through `tools/eng.ps1`** (PowerShell): `& tools/eng.ps1 queue next`. Zero
  quoting; it builds the `wsl bash -c '…'` string internally.
- **Anything beyond one simple command** (awk/sed, multi-statement pipelines, heredocs) → **write a
  `.py`/`.sh`/`.ps1` file and run that file**, not an inline `-c` string. A Python script is more
  robust and readable than inline `awk`.
- **Multi-line commit messages → `git commit -F <file>`**, not a `<<EOF` heredoc.

Claude Code enforces this via `tools/hooks/shell_footgun_guard.py` (a PreToolUse block); other
agents should follow it by convention.

### Build pipeline (per C file)

```
mipsel-linux-gnu-cpp | cc1 (GCC 2.7.2) | maspsx (--aspsx-version=2.34) | mipsel-linux-gnu-as → .o
```

All objects linked with `mipsel-linux-gnu-ld`, stripped to binary via `objcopy`, then `make_psexe.py` prepends the original 0x800-byte PS-EXE header. Final SHA1 compared against original.

### Environment setup

Toolchain installer: `bash tools/setup_wsl.sh` (handles GCC 2.7.2 build, mipsel-linux-gnu toolchain, splat/m2c Python deps).

Required local deps (gitignored):
- `tools/gcc-2.7.2/` — PsyQ-era GCC cross-compiler (built from `decompals/mips-gcc-2.7.2`)
- `tools/maspsx/` — ASPSX compatibility layer (cloned from `mkst/maspsx`)
- `tools/decomp-permuter/` — [decomp-permuter](https://github.com/simonlindholm/decomp-permuter) for auto-matching C code
- `tools/cc1psx.exe` — original PsyQ cc1psx (calibration only — see CLAUDE.md notes)
- `.venv/` — Python virtual env with splat, m2c, etc.
- `disc/` — extracted disc filesystem

## File-edit conventions (cross-tool)

- **Build files** (`src/*.c`, `*.h`, `*.s`, `Makefile`, `*.ld`, the project `*.txt` files like `regfix.txt`, `asmfix.txt`, `named_syms.txt`, `sdata*.txt`, `expand_lb_funcs.txt`) **MUST be saved with Unix LF line endings**. Editing from Windows-side editors that default to CRLF silently breaks the GNU toolchain. Edit via WSL or configure your editor to enforce LF on these paths.
- The `tmp/` directory is for scratch — fully gitignored except `tmp/batch_attempt.csv` (queue working state).
- `permuter/` directories are gitignored (per-function permuter workspaces).
- `auto_matches/` and `codex_lab/` are gitignored (workspace artifacts).

## Root-directory cleanliness

**Don't create new files at the repo root unless they're project-essential.** Specifically:

| Where to write | What goes there |
|---|---|
| `tmp/` (gitignored) | One-off scripts, audits, scratch outputs, exploratory CSVs, log captures |
| `logs/` (gitignored, create on demand) | Long-lived logs if genuinely needed (we don't currently have any) |
| `docs/` | Permanent documentation |
| `tools/` | Reusable tools (committed) |
| root | Only the build files, configs, and tracked docs — see `tools/check_root_cleanliness.py` for the allowlist |

Tooling that enforces this:
- **`tools/hooks/root_write_guard.py`** (Claude Code PreToolUse) — blocks `*.log`, `*.csv`, `gccdump.*`, `*.bak`, `*.orig`, `triage_*`, `_tmp_*`, etc. at root. Suggests `tmp/<name>` as the replacement.
- **`tools/check_root_cleanliness.py`** — manual or briefing-time audit; reports unknown / suspicious root files. Integrated into `dc.sh start` (silent if clean, flags drift otherwise).

## Commit conventions

See [`docs/COMMIT_CONVENTIONS.md`](docs/COMMIT_CONVENTIONS.md) for the subject-prefix catalog and body structure used across this project. The tooling (`tools/dc.sh lessons`, `tools/hooks/llm_audit.sh`) parses commit subjects/bodies for structured information, so consistency improves searchability and audit reliability.

## Asset/format reverse engineering

Per-format inspectors live in `tools/`:
- `tools/inspect_ndata.py`, `inspect_bbm.py`, `inspect_tim.py`, `inspect_stage_bin.py`, `inspect_bnk.py`, `inspect_se.py`, `inspect_xa.py`, `inspect_str.py`

Asset catalog data: `docs/formats/ndata_filemap.csv` (763 entries with semantic naming).

## Key conventions for PS1 decomp

- All addresses are MIPS virtual addresses in KSEG0 (`0x80000000`+)
- `0x1F800000`–`0x1F8003FF` = scratchpad RAM (fast 1 KB SRAM)
- The PsyQ SDK provides BIOS calls, GPU, SPU, CD, controller, and memory card APIs
- PS1 uses little-endian MIPS I instruction set
- Struct padding and alignment must match the PsyQ compiler's behavior
- GTE (cop2) ops have no C analog — inline `__asm__` for those is canonical
- When adding known symbol names, add them to `symbol_addrs.txt` and re-run splat

## Project history

See `docs/HISTORY.md` for the timeline of major milestones (zero-stub completion 2026-04-27, foundation rebuild, etc.) and `docs/STATUS.md` for the current state.

## Getting help

This is a single-person project (Trenton, `tgg12321@gmail.com`). Issues / context for understanding what's been done previously: `docs/HISTORY.md` + git log. The repo's been worked on extensively in Claude Code; references to "the agent" and tools like `dc.sh` are part of that workflow.
