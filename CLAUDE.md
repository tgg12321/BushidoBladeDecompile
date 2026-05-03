# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## START HERE — first action of every session

```
wsl bash -c 'cd /mnt/c/Users/Trenton/Desktop/"Bushido Blade 2 Decompile" && bash tools/dc.sh start'
```

This prints current state (build status, active function marker, queue freshness, top of queue) and the rules summary. A SessionStart hook auto-runs it; if you don't see the briefing in your session context, run it manually as your first command.

**The two things that determine what you do next:**

1. **`Active: <funcname>`** in the briefing → resume that function. The hook is enforcing — `dc.sh next` and `git commit` are blocked until you match. Don't try to revert src/ files (also blocked); edit forward instead. Read `dc.sh diff <funcname>` and `dc.sh verify <funcname>` to see where you are.

2. **`Active: NONE`** → pull from queue: `bash tools/dc.sh next --with-context`. This sets the active marker, gives you the function, and runs `agent-brief` to dump full context.

If the build is `MISMATCH` at session start — STOP. Don't pull more work. Investigate (`dc.sh verify --all`, recent commits) and resolve before any function work. The hook can't help here because there's no active marker — it's a repo state problem.

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

### THE HARD RULE — finish every function you start

**Once a function is selected, you work it end-to-end until matching in 100% pure C (score=0, builds, SHA1 OK). No exceptions for difficulty.** This is the user's durable directive and overrides every other heuristic.

**FORBIDDEN:** tabling, skipping, declaring "too hard", hunting for easier targets after starting, leaving inline `__asm__()` as the answer, leaving `replace_with_asmfile` as a final state, switching to a different function because you're stuck (stuck = switch *technique*, not target).

**The only valid out-of-scope categories** (auto-detected by `dc.sh classify`):
- `permanently_blocked:<reason>` — toolchain physically can't emit (`break`, `add`/`sub`/`addi` overflow ops, `$sp` swap, no `jr $ra`)
- `bios_or_syscall`, `psyq_stdlib_*`, `multi_function`, `aspsx_delay_swra` — accepted-as-asm exceptions

Anything else, you finish. "Hard" / "high score plateau" / "needs novel tooling" are NOT stopping points — they are signals to switch technique or build tooling. See `feedback_workflow_rules.md` THE HARD RULE for full text.

### Selecting the next function — pull from `WORK_QUEUE.md` (HOOK-ENFORCED)

The canonical, ordered work queue lives at `WORK_QUEUE.md` (project root). Pull from the top:

```
bash tools/dc.sh next       # print top 1 AND set active function in .bb2_active_func
bash tools/dc.sh next 5     # preview top 5 (does not set active marker)
```

`WORK_QUEUE.md` already filters out the auto-gated out-of-scope categories (`permanently_blocked`, `bios_or_syscall`, `psyq_stdlib_*`, `multi_function`, `aspsx_delay_swra`, `not_found`) so anything in the queue is in-scope and gets finished.

**THE HOOK** (`tools/hooks/active_func_guard.sh`, configured in `.claude/settings.local.json`) reads `.bb2_active_func` and BLOCKS the following while a function is active:
- `git commit` — unless `dc.sh verify <active>` reports MATCH (then it auto-clears the marker and allows the commit)
- `git checkout` / `git restore` / `git reset --` on `src/*.c`, `regfix.txt`, `asmfix.txt`, `undefined_syms_auto.txt`, `named_syms.txt`, `sdata*.txt`, `expand_lb_funcs.txt`
- `dc.sh next` — refuses to hand out another function while one is active

You literally cannot skip a started function or commit a partial state — the harness blocks the tool calls. This is the enforcement of THE HARD RULE.

**Lifecycle:**
- `dc.sh next` → writes function name to `.bb2_active_func`
- Function matches → `git commit` succeeds → hook auto-clears `.bb2_active_func`
- `dc.sh refresh-queue` → also clears (belt-and-suspenders)
- `dc.sh release` → ONLY escape hatch; requires typing the function name to confirm. **User-driven, NEVER agent-driven.** If you think a function should be abandoned, ask the user; do not run `release` yourself.

After matching: commit, then `bash tools/dc.sh refresh-queue` to regenerate (matched function falls off the queue).

**FORBIDDEN:**
- Searching `src/*.c` or `asm/funcs/*.s` for "easier" candidates
- Reading multiple function asms before picking one
- Re-classifying a function the queue already presents (the queue has already been classified)
- Skipping queue entries because they look hard / large / unclear — the queue is in (loose) complexity order; what's at the top is what's next, and queue order is not your decision
- **Manually deleting/editing `.bb2_active_func`** to bypass the hook — directly contradicts the rule the hook enforces. The only valid clear paths are: a successful `git commit` (auto-clear), `dc.sh refresh-queue` (post-match), or user-driven `dc.sh release`.
- **Reverting WIP src/* files** to "start fresh" on the same function — the hook blocks this for a reason. If the function got into a confusing state, edit forward (write the corrected body via WSL python3) instead of `git checkout`.

If the user says "do the next 5", "work through 10", or anything similar, pull from `dc.sh next` in order. If the user names a specific function, work that one. Either way, no hunting.

### Per function (attempt-first)

The **attempt-first** flow runs all the cheap mechanical steps before the model intervenes. The deterministic pipeline does what it can; you handle the rest.

1. **`dc.sh classify <func>`** — instant pre-dive. Read the recommendation. If it returns one of the out-of-scope categories above, the function is gated out (not your concern this session). For everything else, you finish the function. **Watch for `aliasing_heavy` in `blocker_tags`** — flagged for ptr-chasing patterns where instruction count under-predicts difficulty (queue already pushes them later); use `dc.sh diff` early on these to spot the asymmetric reload patterns.
2. **`dc.sh attempt <func>`** — runs setup → smart_match → permute_capped → gen_regfix automatically. Reports MATCHED / NEAR_MISS / HARD / SKIPPED.
   - **MATCHED** — `auto_matches/<func>.c` has the matched C. Use `dc.sh inline-replace <func> auto_matches/<func>.c`.
   - **NEAR_MISS** (score ≤ 200) — review `/tmp/<func>.regfix.suggestions`, run `dc.sh recipes <func>`, apply rules with `dc.sh add-regfix`.
   - **HARD** — pipeline didn't auto-close it. NOT a stopping point. Deepen manually via toolbox: alternative C structures → register asm → long permuter run → compound regfix → named recipes → new pipeline pass if needed. Use `dc.sh diff <func>` for build-context diff (target.s vs build pipeline output).
   - **SKIPPED** — only fires for the out-of-scope categories above.
3. **Manual deepening** — when NEAR_MISS or HARD, work through the escalation ladder until matched. **Read the penalty-list → technique routing table** in `feedback_matching_playbook.md` to pick the right tool for the symptom. Build new tooling if the existing toolbox can't reach pure C — that's expected.

**If a sibling function suddenly shows a 1-byte build error after your match:** that's `.L<N>` label drift. Run `bash tools/dc.sh fix-label-drift` (drives off the linker error, only fixes rules where the build genuinely failed).

### Escalation when stuck

Stuck means switch *technique*, not target. The ladder (in order):

1. C-level structural variants (the permuter explores these)
2. Register asm pinning — callee-saves only (leaf temp regs are dangerous; see playbook)
3. Long permuter runs (30 min, 1 hour, overnight)
4. regfix at the assembly stream — 30-rule recipes happen
5. Compound regfix layering across multiple blocker classes
6. Named recipes (LICM unhoist, call-loop, early-exit alias, varargs, nested-bool, CU-split)
7. New transformation pass — extend `prologue_fix.py`, add a pipeline stage, write a new regfix op
8. Capture novel patterns as new named recipes via `dc.sh capture-recipe`

### Per-attempt discipline

- **Score regression → immediate revert.** Never build on a worse score.
- **Same score twice on C variants → switch from C to permuter/regfix.** GCC flattens different C structures; repeating doesn't help.
- **Hypothesis before every attempt.** If you can't articulate one, escalate to the next ladder rung — don't flail.
- **Multi-session OK.** If the session ends before match, commit best partial state with hand-off notes; the NEXT session resumes the SAME function. Don't start a new function.

### DO NOT ASK FOR DIRECTION

The user has explicitly directed: keep working until the function is matched. Asking for status updates, direction, or approval mid-work is forbidden. The user is not on standby to redirect when you're stuck — they expect "matched" when they next look.

**FORBIDDEN mid-work prose:**
- "Score is plateauing — should I keep going or move on?" — keep going. You don't move on.
- "I've tried X, Y, Z. How would you like me to proceed?" — switch to the next escalation rung.
- "Want me to build a new pipeline pass?" — yes, just build it. Commit it.
- "Want me to run the permuter overnight?" — yes, just run it.
- "I'm stuck. Should I switch to a different function?" — no, never. Switch *technique*, not target.
- Status reports / progress dumps mid-work.

**The only times you stop and wait for the user:**
1. Function is matched + committed; either batch is done or it was a one-off ("do this function").
2. Classifier returned an out-of-scope category — function is gated out by the rule.
3. Genuinely catastrophic situation: WSL broken, build fundamentally corrupted, repo in an unexpected state requiring real decision. NOT "regfix is hard" or "I've made N attempts."

If tempted to check in because you're stuck: switch technique, not direction. Build the tool. Run the long permuter. Write the 60 regfix rules. Asking permission for in-scope decomp work is itself a violation.

See `feedback_workflow_rules.md` Communication section for the full rule.

### Matching Toolbox (proven techniques — try in this order)

When an attempt scores > 0, classify the diff before flailing:

1. **Structural alternatives (C-level)** — goto vs if/else vs switch, intermediate variables for subtraction order, declaration position for load timing. Use the permuter to explore.
2. **Register asm hints** — `register T x asm("regname")` constrains GCC's allocator. Useful for forcing specific s-reg/v-reg placements. WARNING: leaf temp regs cause scheduling side effects (see `feedback_matching_playbook.md`).
3. **Inline `__asm__`** — for GTE ops, BIOS calls, scratchpad access, and forcing specific instruction sequences. See `feedback_matching_playbook.md` for GCC 2.7.2 constraint pitfalls (no `+r`, `=r`/`0` pseudo-split).
4. **regfix.py (assembly stream rewrite)** — `subst`, `swap`, `delete`, `insert`, `insert_after`, `insert_label`, `reorder`, `fill_delay`, `drain_delay`. Operates between maspsx and `as`. Use for register-allocation diffs and small structural fixes that survive after C-level options are exhausted. Full reference in `feedback_regfix_reference.md`.
5. **Named recipes** — LICM unhoist (`tools/recipes/licm_unhoist.json`, ~7 rules), call-loop family (text1b.c regfix recipe), early-exit alias breaker (8 rules, auto-detected by `gen-regfix`), varargs prologue, nested-bool memcard family, CU split for jtbl interposition. All documented in `feedback_matching_playbook.md` "Named recipes" section.

**No tabling.** Iterate until score=0 + SHA1 OK in the same session — score plateau means switch *technique*, not stop. New tools/passes/recipes are acceptable. See `feedback_workflow_rules.md`.

**Memory entries to consult before each function:**
- `feedback_matching_playbook.md` — full matching guide (toolbox order, every C-side technique, named recipes, things that don't work)
- `feedback_regfix_reference.md` — regfix.txt syntax + every gotcha + verification procedure
- `feedback_workflow_rules.md` — workflow + WSL discipline + integration rules

## WSL Execution Rules (MANDATORY)

All decomp operations go through `tools/dc.sh` — never construct raw WSL pipelines for compile/score/build.

```bash
wsl bash -c 'cd /mnt/c/Users/Trenton/Desktop/"Bushido Blade 2 Decompile" && bash tools/dc.sh <command> [args]'
```

### Workflow-driver commands (try in this order)

| Command | Purpose |
|---------|---------|
| `dc.sh classify <func>` | Pre-dive report: size, blockers, BIOS/syscall/PsyQ tags, Kengo hint. Returns a `recommendation` (easy_attempt / standard / needs_rodata_split / multi_function / bios_or_syscall / psyq_stdlib_<n> / gte_function / aspsx_delay_swra / needs_lwl_fix / **permanently_blocked:&lt;reason&gt;**). The `permanently_blocked` tag is auto-detected from asm patterns (`break`, `add`/`sub`/`addi`, `$sp` swap, no `jr $ra`) plus the manual list in `known_blocked.txt`. |
| `dc.sh attempt <func>` | Full mechanical pipeline: classify → setup → smart_match → permute_capped → gen_regfix. Reports MATCHED / NEAR_MISS / HARD / SKIPPED with elapsed time. **Run this first**; only intervene manually for NEAR_MISS. |
| `dc.sh smart <func>` | smart_match.py: 16 automated transformation strategies (declaration reorder, cast variations, do-while barriers, register hints, etc). Pure-C exploration before permuter. |
| `dc.sh permute <func> [--max-time N] [--max-flat-seconds K]` | permute_capped.py: bounded permuter run with flat-score early termination. |
| `dc.sh recipes [<func>]` | List named regfix recipes / suggest recipes for `<func>`. `dc.sh apply-recipe <recipe> <func>` prints the concrete add-regfix commands. |
| `dc.sh agent-brief <func>` | All-in-one context dump for a session: classify + asm + base.c + gen_regfix + recipe suggestions + Kengo + neighbor functions. **First thing to run on a function.** |
| `dc.sh near-miss <func> [--apply]` | near_miss_attempt: auto-detects byte_arith_fix / drain_delay / plain_reg_substs patterns and applies them with try-and-revert. Default is dry-run; pass `--apply` to actually edit. |
| `dc.sh capture-recipe [<commit>]` | After committing a match, classifies the patterns used and reports if it matches an existing recipe or is novel. Use `--write` to save a draft JSON. |
| `dc.sh add-regfix <func> <op> ...` | Append a validated regfix rule (replaces ad-hoc tmp/add_regfix*.py). Auto-rolls back on validation failure. Ops: swap, subst, delete, insert, insert_after, reorder, fill_delay, drain_delay. |

### Inline-asm-aware setup (399 functions still inline)

| Command | Purpose |
|---------|---------|
| `dc.sh inline-locate <func>` | Find the inline `__asm__()` block for `<func>` in src/ |
| `dc.sh inline-verify <func>` | Verify inline asm matches `asm/funcs/<func>.s` |
| `dc.sh inline-setup <func>` | Stage `permuter/<func>/` for an inline-asm function (m2c base.c + target.s) |
| `dc.sh inline-replace <func> <c>` | After matching, swap the inline block for matched C |

### GTE helpers (Phase 4 -- 86 functions)

| Command | Purpose |
|---------|---------|
| `dc.sh gte <func>` | gte_classifier.py: which `gte_*()` macros from `include/gte.h` would the function need? |
| `dc.sh gte-migrate <func> [--stdout\|--setup]` | gte_migrate.py: convert inline-asm GTE function into a C scaffold using `gte_*()` macros + proper-constrained `__asm__` for the remaining mtc2/mfc2 ops. Writes `permuter/<func>/base.c`. |

### Lower-level building blocks

| Command | Purpose |
|---------|---------|
| `dc.sh compile <func_dir>` | Compile base.c + objdump disassembly |
| `dc.sh score <func_dir>` | Permuter score only (one line) |
| `dc.sh debug <func_dir>` | Full permuter --debug diff + score |
| `dc.sh build` | Incremental make, tail output |
| `dc.sh replace <src> <func> <c_file>` | Replace INCLUDE_ASM with C code (LF-safe) |
| `dc.sh setup <func> <src>` | Set up permuter directory (INCLUDE_ASM stub form) |
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
5. **When permuter score ≠ 0 but structure is correct, check `make` output immediately.** The permuter compiles standalone base.c; the build compiles the full source with real prototypes. If they differ, fix the permuter to match, or go directly to build + regfix.
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
- **Read the penalty list before reaching for a tool.** `dc.sh debug` shows: Insertions, Deletions, Reorderings, Register Differences, Stack Differences, Branch Differences. If `Ins/Del/Reorder = 0` and only `Register Differences` is nonzero, **don't run the permuter** — there's nothing structural to find. Go straight to regfix swap rules.
- **Don't run `gen-regfix` on a clean register-cycle case.** When the diff has 0 ins/del/reorder and just register renames (e.g., a 6-cycle of temp regs), gen-regfix flags every instruction as "structural" because all registers are different — produces 200+ lines of unactionable noise. gen-regfix is for swap/fill/reorder patterns it can detect; pure register cycles are best handled by writing the swap rules by hand from the asm.
- **Permuter cadence: short first, long if needed.** Default first manual permuter run = 90-120s, not 600s. Most cheap structural variants (`int new_var = 1`, decl reorder, `do { } while(0)` barrier) are found in the first minute. Escalate to 5-10 min only if the first pass plateaus well above target AND the diff still shows structural ins/del. Long overnight runs are reserved for genuinely stuck cases with 3+ ins/del that no recipe matches.
- **When pipeline config changes (regfix.txt, sdata_funcs.txt) aren't reflected in build output**, the issue is stale object files: `rm -f build/src/<file>.o && make`. Don't debug the config file.

### Source Integration (after match)

- **Never use `dc.sh replace` for final integration.** It copies the standalone permuter base.c (with duplicate typedefs, externs, forward declarations) into the source file. This requires a cleanup cycle.
- **Write the function body directly** via WSL python3, reusing existing `#include "common.h"` and the file's extern block. Add any missing externs to the file's extern section first.
- **One check for permuter results.** `grep "score = 0"` or read the final line. Don't issue 3 separate calls.
- **Run `make validate` AND `dc.sh verify --all` IMMEDIATELY** after `dc.sh verify <func>` says the function matches — not later, not pre-commit, not "if I remember." These checks catch label drift in OTHER functions caused by your new function shifting GCC's file-wide `.L<N>` numbering. If a sibling regfix subst with a literal `.L347` now points at a different label, fix the broken rule (typically: change pattern to `\.L\d+` regex with the current replacement label) before you even consider the work done. Treating these as "later cleanup" leads to discovering the regression after committing, which is wasteful.
- **Sibling-file extern audit.** Before declaring `extern <type> <sym>` in your file, grep other .c files for the same symbol — a sibling file may already have a more accurate signed/unsigned declaration. Mismatched extern types flip lh/lhu and lb/lbu silently. (See `feedback_matching_playbook.md` "type widening for signed loads".)

### Anti-patterns

- Don't use register asm as a first resort — it's a last resort
- Don't change two things between attempts
- Don't spiral on scheduling without checking structural correctness first
- Don't run the permuter on trivial loops where there's only one possible C structure
- Don't use `dc.sh replace` for final integration — write the function body directly

## Error Response Protocol (MANDATORY)

When a tool call, command, or process fails, **do not just retry with a fix and continue.** Instead:

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
