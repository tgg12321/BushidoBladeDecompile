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

## Workflow

Single-session, one function at a time. Pick the next unmatched stub and work it end-to-end — no scanning for easy targets, no pre-screening multiple candidates. Every function must be worked eventually, so just take whatever is next.

### Per function

1. **Pre-dive analysis** (MANDATORY): read the asm top to bottom, read the surrounding source file, consult Kengo via `tools/kengo_ref.py <func> --bb2`, consult matching-toolbox feedback memories. Form an explicit hypothesis BEFORE writing C. Quick blocker check: if the function has lwl/swl (needs fix_lwl.py), handwritten asm markers, or non-standard prologue, note it but still attempt — only table if confirmed intractable after trying.
2. **Iterate silently** — work without check-ins until the function matches or you're ready to table. Only interrupt for genuine blockers requiring user decision.
   - Every attempt needs a one-line `HYPOTHESIS: ...` written down (for your own tracking)

### Anti-spiral rules

- **Score regression → immediate revert.** Never build on a worse score.
- **Same score twice on C variants → stop C, escalate to permuter/regfix.** GCC flattens different C structures; repeating won't help.
- **3 attempts at the same plateau → STOP and check in with user before trying a 4th.** User may redirect or approve deeper work.
- **Hypothesis before every attempt.** If you cannot articulate one, STOP and escalate.

### Session limits

- No hard token cap — sessions run as long as the hypothesis pipeline is productive
- No hard attempt cap — limited by quality, not count
- Permuter can run multiple rounds / long durations (overnight OK with check-in)
- Regfix can layer compound recipes (25+ rules across multiple applications)
- Multi-session work on one function is allowed — commit best partial state with notes so future sessions can resume

### Matching Toolbox (proven techniques — try in this order)

When an attempt scores > 0, classify the diff before flailing:

1. **Structural alternatives (C-level)** — goto vs if/else vs switch, intermediate variables for subtraction order, declaration position for load timing. Use the permuter to explore.
2. **Register asm hints** — `register T x asm("regname")` constrains GCC's allocator. Useful for forcing specific s-reg/v-reg placements. WARNING: leaf temp regs cause scheduling side effects (see `feedback_regasm_leaf_danger.md`).
3. **Inline `__asm__`** — for GTE ops, BIOS calls, scratchpad access, and forcing specific instruction sequences. See `feedback_gcc272_asm_constraints.md` for pitfalls (no `+r`, `=r`/`0` pseudo-split).
4. **regfix.py (assembly stream rewrite)** — `subst`, `swap`, `delete`, `insert`, `insert_after`, `reorder`. Operates between maspsx and `as`. Use for register-allocation diffs and small structural fixes that survive after C-level options are exhausted.
5. **LICM unhoist recipe** — when target keeps a magic-constant `lui+ori` inline (with the ori filling a `lw` delay slot) and your variant hoists it to a t-reg in the preheader, apply the regfix recipe documented in `memory/feedback_licm_unhoist_recipe.md`. Proven on SetCurrentCursor (commit 22c1cc0). ~7 rules: delete preheader, insert inline (with `insert_after` for the ori delay-slot fill), cascade-rename freed reg.
6. **TABLE** — when 2 consecutive attempts score the same OR the diff is dominated by frame-size mismatch + multiple compounding blockers (regalloc spill differences, GCC strength reduction, delay-slot scheduling across many branches).

**Memory entries to consult before each function:**
- `feedback_gcc272_matching.md` — full matching guide
- `feedback_licm_unhoist_recipe.md` — when LICM signature appears
- `feedback_regasm_scheduling.md` — register asm for scheduling control
- `feedback_aspsx_scheduling.md` — when lui/ori split matters
- `feedback_gcc272_div_and_layout.md` — signed div, block layout, delay-slot patterns

### Tabling criteria

Only table when you have documented attempts at ALL of:
- Alternative C structures via permuter
- Regfix at the assembly stream level
- Consulted the full matching toolbox feedback memories
- Explained why the remaining diff is architecturally intractable (not merely "hard")

Table with full notes so a future session can resume.

## WSL Execution Rules (MANDATORY)

All decomp operations go through `tools/dc.sh` — never construct raw WSL pipelines for compile/score/build.

```bash
wsl bash -c 'cd /mnt/c/Users/Trenton/Desktop/"Bushido Blade 2 Decompile" && bash tools/dc.sh <command> [args]'
```

| Command | Purpose |
|---------|---------|
| `dc.sh compile <func_dir>` | Compile base.c + objdump disassembly |
| `dc.sh score <func_dir>` | Permuter score only (one line) |
| `dc.sh debug <func_dir>` | Full permuter --debug diff + score |
| `dc.sh build` | Incremental make, tail output |
| `dc.sh replace <src> <func> <c_file>` | Replace INCLUDE_ASM with C code (LF-safe) |
| `dc.sh setup <func> <src>` | Set up permuter directory |
| `dc.sh analysis <func>` | Run asm_analysis.py |

### File Editing

- **Build files** (`src/*.c`, `*.h`, `*.s`, `Makefile`, `*.ld`, `*.txt`): **ALWAYS write through WSL** (python3 -c, dc.sh replace, or heredoc). Never use Windows Edit/Write — CRLF silently breaks the GNU toolchain.
- **Non-build files** (memory, CLAUDE.md, tools/*.py, tools/*.sh): Native Edit/Write is fine.

### Execution Discipline

1. **Never use `run_in_background`** for compile/score/build commands — they take <15 seconds.
2. **Always use `2>&1`** on WSL commands to capture stderr.
3. **Ignore systemd warnings** — `"Failed to start systemd user session"` is cosmetic, the command still runs. Do not retry.
4. **For multi-function loops**, use python3 instead of bash for-loops (variable expansion through `wsl bash -c` is unreliable).

## Function Matching Process (MANDATORY)

### Starting a Function

1. **Try without register asm hints first.** GCC 2.7.2's natural allocation usually matches for ≤4 callee-saved regs. Hints often cause extra copies and inflate the frame.
2. **Check function definitions/prototypes in the source file BEFORE writing permuter base.c.** The permuter must see the same declarations the build sees. Wrong prototypes cause different register allocation and codegen. Check: is the function defined later in the same file? What argument types does it use? Are there forward declarations?
3. **Full diff analysis after first structurally-correct attempt.** Once frame size and save count match, do a complete side-by-side (`dc.sh debug`). List EVERY difference. Categorize as: register, scheduling, structural, or missing/extra. Plan fixes for all before next attempt.
4. **One variable per attempt.** Change exactly one thing, verify no regression.
5. **When permuter score ≠ 0 but structure is correct, check `make` output immediately.** The permuter compiles standalone base.c; the build compiles the full source with real prototypes. If they differ, fix the permuter to match, or skip straight to build + regfix.
6. **If 2 C-level attempts at a codegen quirk fail, escalate to pipeline fix.** Don't keep trying source-level tricks for assembler-level problems. Use regfix (swap, reorder, insert) instead.
7. **For regfix indices, always dump maspsx intermediate output first.** Count TEXT instructions (pseudo-insns like `la`/`lb sym` = 1 each), NOT binary instructions (where they expand to 2). Wrong indices silently corrupt the output.

### Key GCC 2.7.2 Techniques

- **`>> N` vs `/ 2^N`**: Check target for bgez/addiu rounding pattern. Absent → shift. Present → division.
- **Split expressions for load scheduling**: `s32 sum = expr; load = ...; result = sum >> N;` positions a load in the pipeline stall.
- **Intermediate variables control subtraction order**: `s32 val = A - B; result = val - C;` prevents GCC from commuting to `A - C - B`.
- **Variable declaration position = load timing**: Declare early → loads early. Declare after heavy computation → loads late.

### Permuter vs Regfix Decision

- **Permuter is for structural alternatives** — different loop styles, expression forms, variable orderings. It explores C-level variations.
- **Regfix is for register assignment** — when the C structure is correct but GCC assigns wrong registers.
- **Trivial loops with no structural alternatives → regfix immediately.** A simple `do { *p=0; n--; p++; } while(n>=0)` has only one C representation. If declaration reordering doesn't fix registers in 2 attempts, the permuter won't either.
- **When pipeline config changes (regfix.txt, sdata_funcs.txt) aren't reflected in build output**, the issue is stale object files: `rm -f build/src/<file>.o && make`. Don't debug the config file.

### Source Integration (after match)

- **Never use `dc.sh replace` for final integration.** It copies the standalone permuter base.c (with duplicate typedefs, externs, forward declarations) into the source file. This requires a cleanup cycle.
- **Write the function body directly** via WSL python3, reusing existing `#include "common.h"` and the file's extern block. Add any missing externs to the file's extern section first.
- **One check for permuter results.** `grep "score = 0"` or read the final line. Don't issue 3 separate calls.
- **Run `make validate` after every match.** GCC numbers `.L<N>` labels file-wide, so adding labels in your function silently shifts label numbers in EVERY later function in the same .c file — breaking any regfix `\.L<N>` substs in those functions. The validator runs the full pipeline and flags PATTERN MISMATCH on every drifted label rule. Fix drifted rules before committing. (See `feedback_label_renumber_breaks_regfix.md`.)
- **Sibling-file extern audit.** Before declaring `extern <type> <sym>` in your file, grep other .c files for the same symbol — a sibling file may already have a more accurate signed/unsigned declaration. Mismatched extern types flip lh/lhu and lb/lbu silently. (See `feedback_signed_load_widening.md`.)

### Anti-patterns

- Don't use register asm as a first resort — it's a last resort
- Don't change two things between attempts
- Don't spiral on scheduling without checking structural correctness first
- Don't run the permuter on trivial loops where there's only one possible C structure
- Don't use `dc.sh replace` for final integration — write the function body directly

## Error Response Protocol (MANDATORY)

When a tool call, command, or process fails, **do not just retry with a fix and move on.** Instead:

1. **Diagnose the root cause.** Why did it fail? Was it a one-off environment issue or a systemic problem?
2. **Fix it permanently.** If the fix is a script change, make the change. If it's a workflow pattern, update CLAUDE.md or the relevant tool.
3. **Commit the fix.** If you changed a script or tool, commit it so future sessions benefit.
4. **Update rules if needed.** If the error reveals a bad pattern (e.g., using Edit on build files, using run_in_background for compiles), add a rule to CLAUDE.md preventing it from happening again.

The goal: **every error should only happen once.** If you find yourself working around the same issue twice, that's a failure to enforce. The second occurrence means you should have fixed it the first time.

## Key Conventions for PS1 Decomp

- All addresses are MIPS virtual addresses in KSEG0 (`0x80000000`+)
- `0x1F800000`–`0x1F8003FF` = scratchpad RAM (fast 1KB SRAM)
- The PsyQ SDK provides BIOS calls, GPU, SPU, CD, controller, and memory card APIs
- PS1 uses little-endian MIPS I instruction set
- Struct padding and alignment must match the PsyQ compiler's behavior
- When adding known symbol names, add them to `symbol_addrs.txt` and re-run splat
