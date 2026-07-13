# Building Bushido Blade 2

This guide walks through everything needed to take a clean checkout of this repository, set up the toolchain, and produce a byte-identical rebuild of `SLUS_006.63`.

If it all works, the final line of `make` will read:

```
OK: bb2 matches!
```

That is the only success signal. SHA1 is compared to the original at every build.

## What you need before starting

| Item | Detail |
|---|---|
| A computer running Linux, or Windows 10/11 with WSL 2 | The build runs entirely in a POSIX shell with GNU tools. Windows-native builds are not supported. |
| About 4 GB of free disk space | Toolchain + extracted disc + build artifacts. The disc image alone is ~660 MB. |
| Python 3.10 or later | For `splat`, `maspsx`, and the project's pipeline tools. |
| An unmodified North American disc image | `Bushido Blade 2 (USA).bin` + `Bushido Blade 2 (USA).cue`, MODE2/2352 format. You must legally own and rip the disc yourself; no copyrighted material is distributed here. The expected SHA1 of the `.bin` is your verification — the build verifies the `SLUS_006.63` extracted from it. |
| Internet access for the first setup | The setup script clones `maspsx`, `mips-gcc-2.7.2`, and `decomp-permuter`. After that, builds are fully offline. |

## Platform setup

### Option A: Windows + WSL Ubuntu 24.04 (the canonical path)

1. From an elevated PowerShell, install WSL and Ubuntu 24.04:

   ```powershell
   wsl --install -d Ubuntu-24.04
   ```

   The convenience script `tools/install_wsl_ubuntu.ps1` runs the same command and prints the post-install instructions.

2. Launch Ubuntu and complete the initial user setup (you'll be prompted for a username and password the first time).

3. Open a WSL shell and `cd` into the project directory under `/mnt/c/`:

   ```bash
   cd /mnt/c/Users/<your_user>/path/to/Bushido\ Blade\ 2\ Decompile
   ```

   Adjust the path to wherever you cloned the repository. Watch for spaces in the path; quote with single quotes or escape with `\` as shown.

4. Run the toolchain installer:

   ```bash
   bash tools/setup_wsl.sh
   ```

   This installs `build-essential`, `binutils-mips-linux-gnu`, `cpp-mips-linux-gnu`, a Python virtualenv, `splat64`/`spimdisasm`, clones [`maspsx`](https://github.com/mkst/maspsx) into `tools/maspsx/`, and clones and builds [`mips-gcc-2.7.2`](https://github.com/decompals/mips-gcc-2.7.2) into `tools/gcc-2.7.2/`. Allow 10–20 minutes for the GCC build.

5. Activate the Python virtualenv (you'll do this once per shell):

   ```bash
   source .venv/bin/activate
   ```

### Option B: native Linux (Ubuntu 22.04+ or Debian)

The toolchain script is written for Ubuntu but works on Debian and derivatives. Skip the `wsl --install` step; the rest of `tools/setup_wsl.sh` runs unchanged.

On other distributions you'll need the equivalent of:

| Ubuntu package | Purpose |
|---|---|
| `build-essential` | `make`, `gcc`, headers needed for the GCC 2.7.2 build |
| `binutils-mips-linux-gnu` | `mipsel-linux-gnu-as`, `-ld`, `-objcopy`, `-nm`, `-objdump` |
| `cpp-mips-linux-gnu` | The MIPS C preprocessor used at the start of the pipeline |
| `python3` + `python3-venv` | Pipeline scripts and the `decomp-permuter` |
| `git`, `wget`, `cmake` | Cloning and building the cross-compiler |

The pipeline tools assume the binaries are named `mipsel-linux-gnu-*`. On distributions that name them `mips-linux-gnu-*` you may need to symlink or adjust the `Makefile`.

## Disc image extraction

The build doesn't use the disc image directly; it uses the extracted `SLUS_006.63` (the main executable) plus the in-game asset files. `tools/extract_iso.py` does the extraction:

```bash
python3 tools/extract_iso.py
```

This reads `Bushido Blade 2 (USA).cue`/`.bin` from the project root and writes:

- `disc/SLUS_006.63` — the main executable (606,208 bytes). The build verifies its SHA1 against this.
- `disc/LOADSE/`, `disc/MOTION/`, `disc/NDATA/`, `disc/TIM2D/`, `disc/U_PIC/`, `disc/XA_0/`, `disc/XA_1/`, `disc/STR/` — game assets, not used by the build but useful for engine research and modding.
- `disc/STR/MOVOVL.EXE` (122,880 bytes) — the FMV overlay; not part of the main build but loaded by the game at runtime.

Everything under `disc/` is gitignored — the directory's contents come from your own disc rip and stay on your machine.

## Building

```bash
make setup         # one-time: re-run splat to regenerate asm/, undefined_syms_auto.txt, etc.
make               # incremental build
```

The `make` invocation runs the full pipeline and prints `OK: bb2 matches!` on success. Subsequent `make` runs are incremental (about 3–10 seconds for typical edits).

Other handy `make` targets:

| Target | What it does |
|---|---|
| `make` / `make check` | Incremental build + SHA1 verify against `bb2.sha1`. Default target. |
| `make clean-check` | `make clean` then `make check`. Always reproducible; the safest verification. |
| `make clean` | Wipes `build/` so the next build is from scratch. |
| `make setup` | Re-runs splat. Use after edits to `splat.yaml` or when `asm/funcs/` looks stale. |
| `make validate` | Runs `tools/validate_regfix.py --live` to flag broken regfix rules without rebuilding. |

The full per-C-file pipeline, as defined in the [`Makefile`](Makefile):

```
cpp | cc1 (GCC 2.7.2) | prologue_fix | maspsx (aspsx 2.34) |
  [fix_lwl] | [rodata align fix] | multu_pad |
  regfix.txt | regfix_stage2.txt | asmfix.txt |
  mipsel-linux-gnu-as -> .o
```

Assembly objects (in `asm/`) take the much shorter route `mipsel-linux-gnu-as -> .o`. Everything is then linked with `mipsel-linux-gnu-ld -T bb2.ld`, stripped to raw binary with `objcopy`, and prepended with the original PS-EXE header via `tools/make_psexe.py`. The resulting `build/bb2.exe` is SHA1-compared to `62efab4f73f992798c43e8c730aa43baa10bb4fa`.

Details on each stage and why the pipeline has so many post-passes are in [`docs/ARCHITECTURE.md`](docs/ARCHITECTURE.md).

## Verifying a successful build

A clean rebuild ends with these two lines:

```
build/bb2.exe: OK
OK: bb2 matches!
```

If you see `MISMATCH: bb2 does not match`, the build linked but produced different bytes than the original. The most common causes:

| Symptom | Cause | Fix |
|---|---|---|
| First-time build mismatch after disc extraction | `SLUS_006.63` doesn't match the canonical SHA1 | Verify you have the NTSC-U release (SLUS-00663, NA, 1998). Other regions (PAL `SLES`, NTSC-J `SLPS`) will not match this project. |
| Mismatch after a `git pull` | Stale build cache picking up old objects | `make clean-check` |
| Mismatch when iterating on a single function | Cached `.o` files masking a real regression | `& tools/wteng.ps1 main verify-oracle --rebuild` (full rebuild + SHA1 gate) |
| Mismatch on a fresh WSL setup | `mipsel-linux-gnu-as` version differs | The pipeline assumes binutils ≥ 2.38. Check with `mipsel-linux-gnu-as --version`. |

To narrow a mismatch down to a single function:

```powershell
# PowerShell (Windows host); the engine runs under WSL for the actual build
& tools/wteng.ps1 main verify-oracle --rebuild    # full clean rebuild + SHA1 gate
& tools/wteng.ps1 main sandbox <funcname> --disable all   # cheat-invisible pure-C distance
& tools/wteng.ps1 main diagnose <funcname>        # classify the gap (control-flow / plateau / canonical)
```

The `engine/` CLI (invoked via `tools/wteng.ps1`) is the central wrapper for all decomp operations; see [`docs/TOOLS.md`](docs/TOOLS.md) for the full subcommand list. Under WSL directly you can invoke `python3 -m engine.cli <subcommand>` after activating `.venv`.

## Common errors

### "Failed to start systemd user session"

A cosmetic warning printed by WSL on first command. Ignore it; the command still ran.

### `mipsel-linux-gnu-as: not found`

Cross-tools weren't installed. Re-run `bash tools/setup_wsl.sh` and check that `binutils-mips-linux-gnu` is present (`dpkg -l | grep mipsel`).

### `cc1: error: -- not found` or similar at the start of a build

The GCC 2.7.2 build under `tools/gcc-2.7.2/build/cc1` didn't complete. Re-run:

```bash
cd tools/gcc-2.7.2 && make && cd ../..
ls -l tools/gcc-2.7.2/build/cc1   # should exist
```

### Build produces wrong bytes after editing a `.c` file from Windows

The PsyQ toolchain (specifically GNU `as`) refuses CRLF line endings silently — it doesn't error, it just emits broken assembly. **All edits to `src/*.c`, `*.h`, `Makefile`, `*.ld`, `regfix.txt`, `asmfix.txt`, `*.s`, and other build files must be made through WSL** (e.g., `vim`, `python3 -c '... open(...).write(...)'`, or VS Code with `files.eol: "\n"` enabled for the WSL remote).

Native Windows editors (Notepad, default VS Code on Windows, etc.) write CRLF and will silently corrupt the build. If you suspect CRLF contamination:

```bash
file src/*.c                              # should report "ASCII text" not "ASCII text, with CRLF"
dos2unix src/*.c                          # convert if needed (apt install dos2unix)
```

Non-build files (`README.md`, `docs/*.md`, `tools/*.py`, `tools/*.sh`) tolerate CRLF and can be edited from either side.

### `make` succeeds but `verify` reports mismatches

This is the build-cache regression trap: an edit to one `.c` file recompiles only that one object, and the linked result happens to match by coincidence because other cached `.o` files contain stale code. To force a clean rebuild and re-verify:

```powershell
& tools/wteng.ps1 main verify-oracle --rebuild
```

This wipes `build/`, rebuilds from source, and gates on SHA1 equality against the oracle.

### Shell scripts fail with "cannot execute: required file not found"

Script shebangs are `#!/bin/bash`; the files are line-ending sensitive. If you've cloned via Git for Windows or extracted from a zip, run:

```bash
dos2unix tools/*.sh tools/hooks/*.sh
```

### Splat (`make setup`) complains about missing symbols

`splat.yaml` references `symbol_addrs.txt`, `undefined_syms_auto.txt`, `undefined_funcs_auto.txt`, `reloc_addrs.txt`, and a few others. They're all checked in; if any are missing, you cloned a partial repository — re-clone.

## Next steps

Once your build verifies, you have everything needed to contribute. Read [`CONTRIBUTING.md`](CONTRIBUTING.md) for the work-queue model, the per-function decomp workflow, and the escalation ladder for stuck functions.
