# Codex Session Handoff (2026-04-27)

This file is a current handoff for Claude or other agents. It is meant to be
standalone and reflects the repo after matching `cpu_check_run_attack`,
tightening the build invalidation rules, and matching `func_8003EB84` in the
working tree.

## Current Repo State

- Branch: `main`
- Current local HEAD at session start: `4843a67` (`Match cpu_check_run_attack`)
- Verified in this session after the Makefile/doc/function updates:
  `wsl make clean-check`
  passes with:
  - `build/bb2.exe: OK`
  - `OK: bb2 matches!`
- Remaining stub count in live source: **63**

Important:
- This handoff describes the current worktree, not just `HEAD`. The worktree
  includes the uncommitted `func_8003EB84` match and the Makefile/doc updates.
- `4843a67` is still the latest committed matched milestone integrating
  `cpu_check_run_attack`.
- The repo has a large amount of untracked lab/archive scratch material. It is
  noisy but not currently blocking the build.

## What Changed In This Session

### 1. `cpu_check_run_attack` is fully integrated

Key files:
- [src/code6cac.c](</C:/Users/Trenton/Desktop/Bushido Blade 2 Decompile/src/code6cac.c>)
- [regfix.txt](</C:/Users/Trenton/Desktop/Bushido Blade 2 Decompile/regfix.txt>)
- [regfix_stage2.txt](</C:/Users/Trenton/Desktop/Bushido Blade 2 Decompile/regfix_stage2.txt>)
- [asmfix.txt](</C:/Users/Trenton/Desktop/Bushido Blade 2 Decompile/asmfix.txt>)

What mattered:
- the isolated function decomp was already correct
- the real blocker was stale label-coupled `regfix` / `asmfix` rules in the
  same `code6cac` translation unit
- after integrating the function body, the remaining full-EXE mismatch shrank
  to 5 bytes across:
  - `DispPracticeMenuTex_A`
  - `func_8001EFA0`
  - `camera_set_zoom`
- retargeting those stale local-label assumptions restored the full match

Useful historical note:
- [archive/tabled_attempts/cpu_check_run_attack_notes.md](</C:/Users/Trenton/Desktop/Bushido Blade 2 Decompile/archive/tabled_attempts/cpu_check_run_attack_notes.md>)

### 2. Build invalidation was tightened

Key file:
- [Makefile](</C:/Users/Trenton/Desktop/Bushido Blade 2 Decompile/Makefile>)

Problem:
- C object rebuilds only depended on `src/*.c`
- changing `regfix.txt`, `regfix_stage2.txt`, `asmfix.txt`, maspsx config, or
  helper scripts could leave stale objects in place
- this caused misleading incremental builds during the `cpu_check_run_attack`
  integration work

Fix:
- added `PIPELINE_DEPS` covering the shared post-pass/toolchain inputs used by
  every C object rule
- updated the C object pattern rule to depend on `$(PIPELINE_DEPS)`
- added a `clean-check` target so agents can force the safe full rebuild path

Practical rule now:
- if you edit C and no shared pipeline/config files, incremental `make` is
  more trustworthy than before
- if you edit matching infrastructure or want certainty, still prefer:
  - `wsl make clean-check`

### 3. `func_8003EB84` is now integrated in the worktree

Key files:
- [src/code6cac_c2.c](</C:/Users/Trenton/Desktop/Bushido Blade 2 Decompile/src/code6cac_c2.c>)
- [asmfix.txt](</C:/Users/Trenton/Desktop/Bushido Blade 2 Decompile/asmfix.txt>)
- [archive/tabled_attempts/func_8003EB84_notes.md](</C:/Users/Trenton/Desktop/Bushido Blade 2 Decompile/archive/tabled_attempts/func_8003EB84_notes.md>)

What mattered:
- the live source now has a semantic C body for `func_8003EB84`
- pure-C work got close enough to confirm the remaining problem was GCC 2.7.2
  register-hoisting behavior, not semantics
- the final landing uses a function-local `asmfix` full-body replacement, which
  preserves the integrated C while forcing the verified target emission

Practical note:
- if a future agent wants to retire the local `asmfix`, the best source-only
  starting points are recorded in the archived function notes
- until then, treat this as a successful late-stage match, not a live stub

## Remaining Stub Distribution

Current concentration by source file:
- `src/code6cac_b.c`: 18
- `src/code6cac.c`: 15
- `src/main.c`: 14
- `src/code6cac_c2.c`: 3
- `src/code6cac_b2.c`: 3
- `src/system.c`: 3
- `src/ings2.c`: 2
- `src/text1a_c.c`: 2
- `src/code6cac_c_mid.c`: 1
- `src/config.c`: 1
- `src/ings.c`: 1

Largest visible remaining stubs by instruction count from `kengo_matches.csv`:
- `func_80023F08` (`code6cac.c`) — 2983
- `func_80029454` (`code6cac_b.c`) — 1025
- `func_8008C464` (`main.c`) — 763
- `func_800198D0` (`code6cac.c`) — 749
- `func_80030D7C` (`code6cac_b.c`) — 709
- `func_80022580` (`code6cac.c`) — 621

Interpretation:
- the remaining count is decent, but the tail is still top-heavy
- `code6cac.c`, `code6cac_b.c`, and `main.c` remain the main risk areas for
  translation-unit fallout and late-stage regfix coupling

## Notable Project Risks

### 1. Shared-TU label/index coupling

This repo now has multiple successful late-stage matches that depend on:
- `regfix.txt`
- `regfix_stage2.txt`
- `asmfix.txt`

The more C replaces `INCLUDE_ASM` inside a dense TU, the more likely stale
local label references become. `cpu_check_run_attack` is the clearest example.

Advice:
- whenever a new integration changes a previously matching full build, use:
  - [tools/trace_diff_to_symbols.py](</C:/Users/Trenton/Desktop/Bushido Blade 2 Decompile/tools/trace_diff_to_symbols.py>)
  - `tools/regfix_verify.py <func>`
  - [tools/dump_func_pipeline.py](</C:/Users/Trenton/Desktop/Bushido Blade 2 Decompile/tools/dump_func_pipeline.py>)
- assume stale label assumptions before assuming the new C body is wrong

### 2. Custom `maspsx` dependency is still live

This remains true from the older handoff:
- the build depends on local changes inside `tools/maspsx`
- do not casually reset, replace, or update that nested repo without checking
  the build/toolchain story first

See also:
- [CODEX_HANDOFF_2026-04-17.md](</C:/Users/Trenton/Desktop/Bushido Blade 2 Decompile/CODEX_HANDOFF_2026-04-17.md>)

### 3. Repo bookkeeping was stale

Updated in this session:
- [CLAIMS.md](</C:/Users/Trenton/Desktop/Bushido Blade 2 Decompile/CLAIMS.md>)

Previously stale:
- stub count said 99; live count is now 63
- `single_game_VoiceContorol` was still listed as an active claim

Agents should trust the live source tree and build output over older counts in
notes/handoffs unless the note explicitly says it is current.

## Recommended Next Steps

If the goal is safest progress:
1. Prefer thin remaining files first:
   - `system.c`
   - `text1a_c.c`
   - `ings2.c`
   - `config.c`
   - `code6cac_c_mid.c`
   - `code6cac_b2.c`
2. Use `wsl make clean-check` after any change to:
   - `regfix.txt`
   - `regfix_stage2.txt`
   - `asmfix.txt`
   - `Makefile`
   - `tools/maspsx/*`
3. Expect the hardest tail to live in:
   - `code6cac.c`
   - `code6cac_b.c`
   - `main.c`

If the goal is to attack the biggest remaining pain first:
- start with a deliberate lab-based workflow for one of the large stubs in
  `code6cac.c` or `code6cac_b.c`
- expect `regfix` / `asmfix` promotion work to be part of the match, not a
  fallback

## Files Touched In This Session

- [Makefile](</C:/Users/Trenton/Desktop/Bushido Blade 2 Decompile/Makefile>)
- [CLAIMS.md](</C:/Users/Trenton/Desktop/Bushido Blade 2 Decompile/CLAIMS.md>)
- [src/code6cac_c2.c](</C:/Users/Trenton/Desktop/Bushido Blade 2 Decompile/src/code6cac_c2.c>)
- [asmfix.txt](</C:/Users/Trenton/Desktop/Bushido Blade 2 Decompile/asmfix.txt>)
- [archive/tabled_attempts/cpu_check_run_attack_notes.md](</C:/Users/Trenton/Desktop/Bushido Blade 2 Decompile/archive/tabled_attempts/cpu_check_run_attack_notes.md>)
- [archive/tabled_attempts/func_8003EB84_notes.md](</C:/Users/Trenton/Desktop/Bushido Blade 2 Decompile/archive/tabled_attempts/func_8003EB84_notes.md>)
- [CODEX_HANDOFF_2026-04-27.md](</C:/Users/Trenton/Desktop/Bushido Blade 2 Decompile/CODEX_HANDOFF_2026-04-27.md>)

## Addendum: `func_8003EB84` Result

This session started as a fresh live probe on the tabled stub
`src/code6cac_c2.c:func_8003EB84` and ended with a successful worktree match.

Outcome:
- the stub is gone from live source
- the function is integrated in C
- the full build still matches `bb2.exe`

Why the final landing used `asmfix`:
- pure-C work was enough to show that the remaining gap was not semantics
- the blocker was GCC's long-lived base-hoist choices for `D_800A4750` /
  `D_800A6690` once the lookup-base hoists were corrected
- a function-local full-body replacement was the pragmatic way to land the
  match without reopening TU-wide fallout

See:
- [archive/tabled_attempts/func_8003EB84_notes.md](</C:/Users/Trenton/Desktop/Bushido Blade 2 Decompile/archive/tabled_attempts/func_8003EB84_notes.md>)
