# CLAUDE.md

This file is the project-invariant briefing loaded on every Claude Code session. All workflow rules, tools reference, recipes, and technique playbooks live in `memory/` — see `memory/MEMORY.md` for the indexed catalog. Skills (`/decomp-next`, `/decomp-cheat-cleanup`, `/cheat-audit`) live in `.claude/skills/`.

## START HERE — first action of every session

```
wsl bash -c 'cd /mnt/c/Users/Trenton/Desktop/"Bushido Blade 2 Decompile" && bash tools/dc.sh start'
```

A SessionStart hook auto-runs it. The briefing prints build status, active function marker, queue freshness, top-of-queue, cheat counts, and the rules summary. **Decide what to do next based on the briefing:**

1. **`Active: <funcname>`** → resume that function. The hook is enforcing — `dc.sh next*` and `git commit` are blocked until you match. Don't revert WIP src/ files (also blocked); edit forward. Use `dc.sh diff <funcname>` and `dc.sh verify <funcname>` to see where you are.
2. **`Active: NONE`** → `bash tools/dc.sh next --with-context` (pulls top of queue + sets marker + runs agent-brief).
3. **`Build: MISMATCH`** → STOP. Don't pull more work. Investigate (`dc.sh verify --clean`, recent commits) before any function work.

Work is solo end-to-end on a single function. No sub-agents, no worktrees, no orchestrator delegation — one focused session per function, technique-switch when stuck. See `memory/rules/hard-rule.md`.

## Project Overview

Matching decompilation of **Bushido Blade 2** (SLUS-00663) — PS1 fighting game published by SquareSoft (1998, NA). The goal: C source that compiles to a byte-identical copy of the original executable.

- **Platform:** PlayStation 1 (MIPS R3000A, 2 MB RAM, PsyQ SDK)
- **Developer:** Lightweight (40% Square subsidiary)
- **Engine:** "Marionation" (Lightweight's proprietary engine, later reused for Kengo on PS2)
- **Build date:** Fri Aug 7 22:26:32 1998

## Toolchain (confirmed from binary analysis)

- **PsyQ SDK 3.5** (DTL-S3000), copyright "1993-1997"
- **Compiler:** GCC 2.7.2 (SN Systems fork / cc1psx)
- **Assembler:** ASPSX ~2.34
- **Section order:** `.rodata → .text → .data → .bss` (PsyQ standard)
- Identified via CVS `$Id:` tags in linked PsyQ libraries: `sys.c v1.129 1996/12/25` (libgpu), `bios.c v1.86 1997/03/28` (libcd), `intr.c v1.76 1997/02/12` (libapi)

## Executable details

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

## Splat configuration

The binary is split using [splat](https://github.com/ethteck/splat) v0.39.0 via `splat.yaml`. Re-run via `python -m splat split splat.yaml` (regenerates asm/, linker script, symbol files).

Current split layout:
- `asm/data/800.rodata.s` — .rodata section (strings, jump tables, constants)
- `asm/6CAC.s` — .text section (1,410 functions, all assembly)
- `asm/data/7D920.data.s` — .data section (initialized globals)
- `bb2.ld` — generated linker script
- `undefined_syms_auto.txt` — auto-detected data symbol addresses
- `undefined_funcs_auto.txt` — auto-detected external function addresses
- `symbol_addrs.txt` — manually identified symbol names (add known names here)

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

Per-format documentation in `docs/formats/` (NDATA, BBM, TIM, STAGE_BIN, BNK, SE, XA, STR, MOVOVL).

## Build commands

All build commands run inside WSL (Ubuntu 24.04):

```bash
cd /mnt/c/Users/Trenton/Desktop/"Bushido Blade 2 Decompile"
source .venv/bin/activate
make setup    # re-run splat (regenerate asm, linker script)
make          # build and verify SHA1 match
make clean    # remove build artifacts
```

From Git Bash: `wsl bash -c 'cd /mnt/c/Users/Trenton/Desktop/"Bushido Blade 2 Decompile" && source .venv/bin/activate && make'`

Build pipeline per C file:
```
mipsel-linux-gnu-cpp | cc1 (GCC 2.7.2) | maspsx (--aspsx-version=2.34) | mipsel-linux-gnu-as → .o
```
All objects linked with `mipsel-linux-gnu-ld`, stripped to binary via `objcopy`, then `make_psexe.py` prepends the original 0x800-byte PS-EXE header. Final SHA1 compared against original.

## Tools

`tools/dc.sh` is the workflow driver — see `memory/reference/tools.md` for the full catalog. Quick hits:

| Tool | Purpose |
|------|---------|
| `dc.sh start` | Session briefing — run first |
| `dc.sh next [--with-context]` | Pull next function from `WORK_QUEUE.md` (sets active marker) |
| `dc.sh classify <func>` | Pre-dive: blockers, aliasing_heavy tag |
| `dc.sh attempt <func>` | Full mechanical pipeline (smart → permute → gen_regfix) |
| `dc.sh build-active <func>` | Incremental rebuild + bridge-aware verify-c |
| `dc.sh verify <func>` / `verify --all --force` | Per-function / full SHA1 check |
| `dc.sh diff-align <func>` | Sequence-aligned diff (cascade-immune) |
| `dc.sh refresh-queue` | Regen `WORK_QUEUE.md` post-match |
| `dc.sh release` | ESCAPE HATCH (user-only, typed confirm) |

Per-format inspectors: `tools/inspect_ndata.py`, `tools/inspect_bbm.py`, `tools/inspect_tim.py`, etc. (See `docs/formats/`.)

Reverse engineering: `tools/extract_iso.py`, `tools/make_psexe.py`, `tools/setup_wsl.sh`, `tools/maspsx/`, `tools/gcc-2.7.2/`, `tools/decomp-permuter/`.

Permuter quick-start: `bash tools/permuter_setup.sh <func> <src/file.c>` → edit `permuter/<func>/base.c` → `python3 tools/decomp-permuter/permuter.py permuter/<func> -j4 --stop-on-zero`.

## Workflow — where the rules actually live

Don't restate rules here; they're in `memory/rules/`. The high-leverage ones for new agents:

- `memory/rules/hard-rule.md` — once selected, work end-to-end to pure C
- `memory/rules/queue-selection.md` — pull from `WORK_QUEUE.md` top, no hunting; hook enforces
- `memory/rules/escalation-ladder.md` — when stuck, switch technique not target
- `memory/rules/communication.md` — DO NOT ASK for direction mid-work
- `memory/rules/pre-dive-analysis.md` — context-gathering before writing C
- `memory/rules/integration-discipline.md` — after-match verify protocol
- `memory/rules/no-voluntary-stops.md` — only `permanently_blocked:*` ends a function
- `memory/rules/minimize-regfix.md` — every new regfix rule is debt
- `memory/rules/minimize-asm-when-blocked.md` — only escape valve when pure-C provably can't byte-match
- `memory/rules/canonical-asm-retirement.md` — STRONG-signal-only gate for `inline_asm_canonical.txt`
- `memory/rules/bridge-is-not-decomp.md` — `replace_with_asmfile` requires explicit user authorization

Skills (`.claude/skills/`):
- `/decomp-next` — pulls next from queue + drives full matching workflow
- `/decomp-cheat-cleanup` — retires existing cheat orphans
- `/cheat-audit` — adversarial LLM auditor on a specific commit/function/file

Workflow mechanics (`memory/workflow/`):
- `file-editing-wsl.md` — build files (`src/*.c`, `*.txt` in pipeline) MUST be edited via WSL to avoid CRLF
- `pipeline-cache-busting.md` — `rm -f build/src/<file>.o && make` when config changes
- `crlf-via-edit-write.md` — Windows-side Edit/Write silently converts LF→CRLF on `.txt` files
- `active-marker-quirks.md` — when the auto-clear doesn't fire
- `auto-drift-repair.md` — automatic label-drift handling during build-active
- `parent-cd-after-subagent.md`, `parallel-harness-gotchas.md`, `parallel-worktrees.md` — sub-agent coordination

Recipes (`memory/recipes/`): GTE 3x3, scratchpad GTE, shared-end-label, retirement-recipes, cheat-cleanup-techniques, satan2-kabuto-pin, voice-control-playbook (the [[minimize-asm-when-blocked]] case study), exec-game-plateau (cheat orphan that resists pure-C).

Deep reference (`memory/reference/`): matching-playbook (40 KB master doc), quick-reference (symptom-indexed), regfix-reference, register-asm-pins, inline-move-aliasing, dead-vars-local-array, cc1psx-calibration, scoring-systems, tool-catalog, etc.

Audit/authorization (`memory/audit/`): pattern-observations, lost-codegen-insert-cheat, inline-asm-injection, bridge-signature.

Naming (`memory/naming/`): bulk-promotion, kengo-name-unreliable, raw-dx-unsafe, placeholder-refinement, kengo-cross-reference.

Current project state (`memory/project/`): status-and-attack, inline-asm-offenders, build-and-internals, satan0main-midi-dispatch.

Archived sessions (`memory/history/`): browse for traceability; not auto-loaded.

## Key conventions for PS1 decomp

- All addresses are MIPS virtual addresses in KSEG0 (`0x80000000`+)
- `0x1F800000`–`0x1F8003FF` = scratchpad RAM (fast 1 KB SRAM)
- The PsyQ SDK provides BIOS calls, GPU, SPU, CD, controller, and memory card APIs
- PS1 uses little-endian MIPS I instruction set
- Struct padding and alignment must match the PsyQ compiler's behavior
- When adding known symbol names, add them to `symbol_addrs.txt` and re-run splat

## Getting help

- User-facing CLI help: `/help` (Claude Code)
- Feedback / bugs: https://github.com/anthropics/claude-code/issues
- For project-level questions in a new session: read `memory/user/role.md` + `memory/MEMORY.md`, then ask
