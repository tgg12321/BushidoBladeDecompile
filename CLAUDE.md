# CLAUDE.md

Project-level configuration for the **Bushido Blade 2** (SLUS-00663) matching decompilation.
Tool-agnostic project facts (toolchain, build pipeline, splat config, disc structure, executable
details) live in [AGENTS.md](AGENTS.md) — `@AGENTS.md` imports them into context.

@AGENTS.md

## The workflow IS the engine (`engine/`)

All decomp work goes through the **greenfield decomp engine** (`engine/`, run under WSL). It
replaced the old `dc.sh`-driven, skill-based loop. The engine is a deterministic spine:
route C-vs-asm → measure the honest cheat-free distance → triage → verify a byte+link match.
By construction, cheating can't help and asm functions aren't pure-C-grinded.

Read [memory/project/greenfield-engine-v2.md](memory) for the full design + current state.

### Run everything in WSL, from the repo root
```
wsl bash -c 'cd "<root>" && source .venv/bin/activate && python3 -m engine.cli <cmd>'
```

### CLI
| Command | Purpose |
|---|---|
| `verify-oracle [--rebuild]` | confirm the tree still builds byte-identical (the oracle) |
| `build` | full clean-driver build → SHA1 check |
| `canonical <func>` | C-vs-asm gate: ASM-region / ASM-structural / C. **Run BEFORE any pure-C work.** |
| `sandbox <func> --disable all` | cheat-invisible score = the honest pure-C distance |
| `diagnose <funcs...>` | classify a gap: matchable / control-flow / canonical / plateau |
| `scan-redundant --all` | rules the build doesn't need + a difficulty-ranked worklist |
| `retire <func>` | delete a function's rules + full-build SHA1 verify (auto-rollback on mismatch) |
| `fixtures-verify` | tool-health: golden fixtures still byte-match |

### The non-negotiables (enforced by construction, not by nagging)
1. **Tier-4 standard** — 100% pure C; zero regfix/asmfix/register-pins. The only allowed asm is
   *canonical* (GTE/cop2 ops, BIOS/syscall trampolines, or a construct with no C form), and
   `canonical` decides what qualifies — not the agent.
2. **The oracle is the only truth** — "done" = full build+link SHA1 ==
   `62efab4f73f992798c43e8c730aa43baa10bb4fa`. Intermediate exit codes and isolated scores are hints.
3. **Cheating can't help** — the sandbox scores with regfix/asmfix disabled and tier-3 inline-asm
   stripped, so adding a regfix rule / pin / `__asm__` injection does not move the score. The
   integrated gate catches anything snuck in. Don't reach for cheats; they're inert here.
4. **Canonical-asm is the gate's call** — if `canonical` says ASM-region or ASM-STRUCTURAL, do NOT
   grind it in pure C. Genuine no-C-form constructs get authorized inline asm; never recreate target
   bytes via hardcoded-`$N` `__asm__` injection.

## Workflow: a single focused agent, on main
Decomp work runs **directly on `main`** — one focused agent, end-to-end. No worktrees, no
subagents, no orchestrator/worker split: the engine is a toolkit the agent drives itself.
**Build files (`src/*.c`, `*.h`, `*.s`, `Makefile`, `*.ld`, pipeline `*.txt`) MUST use LF line
endings** — edit via WSL or an LF-enforcing editor (the Write tool produces LF on this machine).

## The loop (per function)
The agent *is* the gap-closer — the engine measures, routes, and gates; you write the C.

0. **Pick** a target — `scan-redundant --all` prints a difficulty-ranked worklist, or work a
   function the user named.
1. **`verify-oracle --rebuild`** once at session start. This makes `build/` the clean canonical
   reference the sandbox scores against. (Skip if `build/` is already clean.)
2. **`canonical <func>`** — route. ASM-region / ASM-STRUCTURAL ⇒ stop the pure-C effort
   (authorized inline asm only; never grind it or inject `$N` asm). `C` ⇒ continue.
3. **`sandbox <func> --disable all`** — the honest cheat-free pure-C distance (`0` = already
   matchable in pure C; just write/keep that C).
4. **Edit `src/<file>.c`** toward the target in pure C, then re-run step 3 — the score is your
   gradient. `diagnose <func>` explains a stuck gap (matchable / control-flow / canonical / plateau).
5. **Score 0 ⇒ finish.** `retire <func>` deletes the function's now-unneeded regfix/asmfix rules,
   rebuilds, and SHA1-gates (auto-rollback on mismatch). A pure-C function with no rules to remove
   just needs `verify-oracle --rebuild` to confirm the byte+link match.
6. **Commit** (`cheat-cleanup:` / `Match` / `engine:` prefix per docs/COMMIT_CONVENTIONS.md).

**Reference gotcha:** the sandbox scores your edited, cheat-stripped `.o` against
`build/src/<file>.o`, which must stay the *pristine* canonical build (= the target bytes). During
the edit loop use only `sandbox` — it builds into `tmp/` and never touches `build/`. Do **not** run
`build` / `verify-oracle --rebuild` while src has uncommitted edits: that rebuilds `build/` from
your half-finished source, and the sandbox then scores against garbage. If it happens, re-establish
a clean reference (revert or finish the edit, then rebuild). The final SHA1 gate is always honest.

## Substrate — do NOT break
The engine reuses proven stage tools as substrate; treat these as load-bearing:
cc1, maspsx, `regfix.py`/`asmfix.py` (pipeline stages), `prologue_fix`, `multu_pad`, `fix_lwl`,
`as`/`ld`/`objcopy`, `make_psexe`, splat, `classify_inline_asm.py`, decomp-permuter — plus the
original EXE, `asm/`, `src/`, `include/`, `disc/`, and the `*.txt` configs. The Makefile remains
until `engine build` fully supersedes it; the oracle (`engine verify-oracle`) guards every change.

## Guards (hooks)
Active: root-write cleanliness, CRLF/tooling-error (WSL env-failure) detection, the cc1psx-footgun
block, and memory/CLAUDE.md hygiene. The legacy decomp-loop hooks (`grind_check`, `resilience_judge`,
`escape_valve`, …) and the main-branch publish-only guard have been removed.

## Commit conventions
See [docs/COMMIT_CONVENTIONS.md](docs/COMMIT_CONVENTIONS.md). Engine work uses the `engine:` prefix.

## Getting help
CLI help: `/help`. New-session orientation: `memory/MEMORY.md` (auto-loaded) +
`memory/project/greenfield-engine-v2.md`.
