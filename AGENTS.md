# AGENTS.md

Tool-agnostic project facts for any AI coding agent ([agents.md](https://agents.md/) standard).
Claude-Code-specific workflow lives in `CLAUDE.md`.

> **About to decompile a function? Read [`docs/DECOMP_WORKFLOW.md`](docs/DECOMP_WORKFLOW.md) first**
> (what "done" means, the per-function loop, the honest-C standard, mandatory adversarial review).
> `docs/MATCHING.md` is the technique catalog.

## Project

Matching decompilation of **Bushido Blade 2** (SLUS-00663, PS1, SquareSoft/Lightweight 1998, the
"Marionation" engine): C source that rebuilds a byte-identical executable. Single-person project
(Trenton, `tgg12321@gmail.com`). History: `docs/HISTORY.md`; state: `docs/STATUS.md`; disc layout,
asset formats and inspectors: `docs/formats/`.

## Toolchain (confirmed from binary analysis)

- **PsyQ SDK 3.5** (DTL-S3000; CVS tags libgpu `sys.c v1.129`, libcd `bios.c v1.86`, libapi `intr.c v1.76`).
- **Compiler:** GCC 2.7.2 (SN cc1psx); the build uses the open-source port `decompals/mips-gcc-2.7.2`.
- **Assembler:** ASPSX 2.34 via [maspsx](https://github.com/mkst/maspsx) (`--aspsx-version=2.34`).
- **Section order:** `.rodata → .text → .data → .bss` (PsyQ standard).
- Per C file: `mipsel-linux-gnu-cpp | cc1 | maspsx | mipsel-linux-gnu-as → .o`; link with
  `mipsel-linux-gnu-ld`, `objcopy`, then `make_psexe.py` prepends the original 0x800-byte header.
- **`-mel` and `-msoft-float` in `CC_FLAGS` are load-bearing — do not remove.** The prebuilt cc1's
  `mips-mips-gnu` triple defaults to big-endian and hard float; the PS1 is little-endian with no
  FPU, and PsyQ's cc1psx reports `-msoft-float`.
- `tools/cc1psx.exe` (original PsyQ cc1psx) is for calibration only — never a build path.

## Executable

- **Main EXE** `disc/SLUS_006.63` (606,208 bytes), **SHA1 `62efab4f73f992798c43e8c730aa43baa10bb4fa`**
  — the oracle; "done" means the full build reproduces it.
- PS-X EXE, load `0x80010000`, entry `0x800836EC`, stack `0x801FFFF0`, GP `0x800A30CC`;
  text+data `0x93800` bytes from file offset `0x800`; code ends ~`0x8008D070`.
- Overlay `disc/STR/MOVOVL.EXE` (FMV/MDEC): loads at `0x801D8800`, no overlap with the main EXE.

## Build

Builds run inside WSL (Ubuntu 24.04); toolchain setup: `bash tools/setup_wsl.sh`.

```bash
cd /mnt/c/Users/Trenton/Desktop/"Bushido Blade 2 Decompile" && source .venv/bin/activate
make          # build and verify SHA1
make clean
```

- **Never run `make setup`.** `bb2.ld` is hand-maintained and `asm/data/*.rodata*.s` are
  deliberately deleted; re-running splat breaks the build (recovery procedure in `splat.yaml`).
- **Build files** (`src/*.c`, `*.h`, `*.s`, `Makefile`, `*.ld`, pipeline `*.txt`) **must be LF.**
  Windows-side editors and Windows Python text-mode writes produce CRLF and silently break the
  GNU toolchain.

### PowerShell-first scripting

Calling WSL from a Windows-side agent nests shells (Git Bash/PowerShell → wsl → bash); every `$`,
quote and backslash is re-parsed per layer, silently breaking inline awk/sed, heredocs and
hand-escaped quotes.
- Engine commands: `& tools/wteng.ps1 main <cmd>` (PowerShell) — builds the WSL call internally
  and pins the repo.
- Anything beyond one simple command → write a `.py`/`.sh`/`.ps1` under `tmp/` and run the file.
- Multi-line commit messages → `git commit -F <file>`.

## Conventions

- **Not-yet-matched functions are committed as `INCLUDE_ASM("asm/funcs", <func>);`** — no cheat
  constructs and no draft C on `main`; C lands once, when it byte-matches honestly. In-progress
  candidates live in `memory/grind/<func>/`.
- GTE (cop2) ops and BIOS/syscall trampolines have no C form — inline `__asm__` for those is canonical.
- Addresses are KSEG0 (`0x80000000`+); `0x1F800000`–`0x1F8003FF` is scratchpad RAM.
- Scratch goes in `tmp/` (gitignored); don't add files at the repo root
  (`tools/check_root_cleanliness.py`).
- Commit subjects/bodies follow [`docs/COMMIT_CONVENTIONS.md`](docs/COMMIT_CONVENTIONS.md) — the
  commit-msg guard chain and audits parse them.
