# CLAUDE.md

Project-level configuration for the **Bushido Blade 2** (SLUS-00663) matching decompilation.
Tool-agnostic project facts (toolchain, build pipeline, splat config, disc structure, executable
details) live in [AGENTS.md](AGENTS.md) — `@AGENTS.md` imports them into context.

@AGENTS.md

## The workflow IS the engine (`engine/`)

All decomp work goes through the **decomp engine** (`engine/`, run under WSL) — a deterministic
spine: route C-vs-asm → measure the honest cheat-free distance → triage → verify a byte+link
match. By construction, cheating can't help and asm functions aren't pure-C-grinded. Full design:
[memory/project/greenfield-engine-v2.md](memory).

### PowerShell-first scripting (enforced by `shell_footgun_guard.py`)
`wsl bash -c 'cd … && source .venv && python3 -m engine.cli …'` nests three shells and the quoting
silently eats awk/sed/heredocs — the PreToolUse guard BLOCKS that form. (AGENTS.md carries the
tool-agnostic rationale.) Claude-Code specifics:
- **Engine/build commands → PowerShell tool + `& tools/wteng.ps1 main <cmd>`** (zero quoting; stamps
  `CLAUDE_SESSION_ID`; pins the repo — never bare `make` / relative `eng.ps1`, a guard blocks them).
- **Beyond ONE simple command** (awk/sed, pipelines, heredocs) → write a `.py`/`.sh`/`.ps1` to `tmp/`
  and run the file. **Multi-line commit messages → `git commit -F tmp/msg.txt`.**
- Plain one-command WSL calls (git / grep / make) are fine via the Bash tool, or `bash tools/wsl.sh '<cmd>'`.

### CLI
| Command | Purpose |
|---|---|
| `queue next` / `done <func>` / `status` | **the worklist** — take the top item, mark it COMPLETED, see progress |
| `verify-oracle [--rebuild]` | confirm the tree still builds byte-identical (the oracle) |
| `build` | full clean-driver build → SHA1 check |
| `canonical <func>` | C-vs-asm gate: ASM-region / ASM-structural / C. **Run BEFORE any pure-C work.** |
| `sandbox <func> --disable all` | cheat-invisible score = the honest pure-C distance (cheat-asm stripped) |
| `diagnose <funcs...>` | classify a gap: matchable / control-flow / canonical / plateau |
| `scan-redundant --all` | low-level: per-file redundant-rule scan (`queue` is the consolidated worklist) |
| `retire <func>` | delete a function's rules + full-build SHA1 verify (auto-rollback on mismatch) |
| `fixtures-verify` | tool-health: golden fixtures still byte-match |
| `test` | engine regression suite (distance / gate / cheat-stripping) — keep green when you touch engine code |

### The three function categories
Every function is in exactly ONE state — no gradations, no "almost done." Cheats (regfix/asmfix
rules, register pins, cheat-asm `__asm__` blocks, scheduling barriers) are NEVER an end state.
Full policy: [[completion-standard]].
  - **INCOMPLETE** — in `engine/queue.json`; carries a cheat (regfix/asmfix rule, cheat-asm
    pin/`__asm__`, OR non-zero honest pure-C distance). Stays queued until it reaches a COMPLETED state.
  - **COMPLETED-C** — zero rules, zero cheat-asm, byte-matches. Not in the queue, not in
    `inline_asm_canonical.txt`. The SOTN community bar; the default goal for every function.
  - **COMPLETED-INLINE-ASM-CANONICAL** — zero rules, canonical inline asm (GTE/cop2, BIOS/syscall
    trampolines) or whole-body `__asm__("glabel …")` as accepted finished form. Listed in
    `inline_asm_canonical.txt`. Reserved for functions whose ORIGINAL CODE was hand-written asm —
    the `canonical` gate decides what qualifies, not the agent.

### The non-negotiables (enforced by construction, not by nagging)
1. **The completion standard** — only the two COMPLETED states may be committed as done.
   **Enforced:** `queue done` / `regen` refuse a function carrying any regfix/asmfix rule OR cheat-asm
   (register pins, hardcoded-`$N` asm, scheduling barriers) unless authorized in
   `inline_asm_canonical.txt`. SHA1 alone can't catch a cheat-asm "match", so the gate audits source.
   `tools/check_completion_integrity.py` is the standing audit; the Grinder's default-FAIL Judge gates
   every autonomous completion.
2. **The oracle is the only truth** — "done" = full build+link SHA1 ==
   `62efab4f73f992798c43e8c730aa43baa10bb4fa`. Intermediate exit codes and isolated scores are hints.
3. **Cheating can't help** — the sandbox scores with regfix/asmfix disabled and cheat-asm stripped, so a
   regfix rule / pin / `__asm__` injection can't move the score. The integrated gate catches anything
   snuck in. They're inert here.
4. **Canonical-asm is the gate's call** — if `canonical` says ASM-region / ASM-STRUCTURAL, do NOT grind
   it in pure C. Genuine no-C-form constructs get authorized inline asm; never recreate target bytes via
   hardcoded-`$N` `__asm__` injection.

## Workflow: the Grinder (default)
The **default** autonomous workflow is the **Grinder** (`tools/grinder/`) — a deterministic single-lane
driver that grinds the queue top to COMPLETED-C, however many sessions it takes: persistent per-function
ledger (`memory/grind/<func>/`), driver-enforced modality ladder, no `blocked` outcome (invalid sessions
discarded + respawned), bytes proven on main BEFORE a default-FAIL Judge (owner's static policy) makes
the final call. **Invoke the `decomp-grind` skill** to operate it (orient → drill → launch detached →
report; also status/stop/incident verbs). Direct: `pwsh tools/grinder/grind.ps1` (`-Once` = one
supervised iteration) · `pwsh tools/grinder/status.ps1` · `pwsh tools/grinder/grind.ps1 -Stop`. Owner
audit surfaces: `docs/grind/decisions.md` + `docs/grind/journal.md`. Spec:
`docs/superpowers/specs/2026-07-06-grinder-pipeline-design.md`. The fleet (`tools/fleet/`) is RETIRED
(superseded 2026-07-06 — see `docs/fleet/HANDOFF.md`).

### Fallback: a single focused agent, on main
For driving ONE function by hand, decomp work can run **directly on `main`** — one focused agent,
end-to-end, the engine as a toolkit (the `decomp-orchestrate` skill). Manual path only; the Grinder is
the autonomous pipeline. **Build files (`src/*.c`, `*.h`, `*.s`, `Makefile`, `*.ld`, pipeline `*.txt`)
MUST use LF line endings** (also in AGENTS.md) — edit via WSL or an LF-enforcing editor (the Write tool
produces LF on this machine).

### Manual-path review discipline (autonomous path: the Judge handles this)
Any MANUAL completion-class commit (`Match:` / `cheat-cleanup:` / `auth:` / rule-doc additions) requires
the layer-2 fresh adversarial `cheat-reviewer` (default-FAIL, author's verdict not credited) before
acceptance — full protocol in [[review-discipline-before-commit]]. No deferral (2026-06-12): a stuck top
item changes MODALITY, never target. Auto-handle without escalation: GTE leaf wrappers
([[gte-wrapper-misroute-park]], user policy 2026-05-26) and jtbl-infra parks.

## Standing warnings — rodata cleanup (COMPLETE 2026-06-09)
All 12 `asm/data/*.rodata*` segments retired; jtbl-infra rules deleted; oracle unchanged (full log in git
history). Two load-bearing warnings persist:
1. **`bb2.ld` is HAND-MAINTAINED — do NOT run `make setup`** (re-adds dead rodata lines + conflicts with
   const decls now in `src/*.c`; recovery procedure in `splat.yaml`).
2. **`asm/data/*.rodata*.s` are deliberately DELETED** — don't re-create them.

## The queue IS the worklist (`engine queue`)
All outstanding work lives in ONE ordered list — `engine/queue.json` — covering every function still
carrying a cheat (a regfix/asmfix rule OR a load-bearing cheat-asm pin/inline-asm). Every queue item is
INCOMPLETE by definition; reaching a COMPLETED state drops it off. The queue is **pre-ordered
easiest-first** by honest pure-C distance: **no triage, no cherry-picking, NO DEFERRAL (user directive
2026-06-12)** — work the **top active item** to COMPLETED, however many sessions it takes; a stuck item
changes MODALITY (deep-dive, bulk sweeps, new diagnostics), never target. The SessionStart hook surfaces
the top each session; `queue status` shows progress.

| `queue` action | Purpose |
|---|---|
| `queue next` | print the top active item (func, file, verdict, distance, rule count) |
| `queue done <func>` | mark complete — re-checks ZERO rules + ZERO non-canonical cheat-asm + build SHA1 == oracle (refuses otherwise). On success the function is REMOVED from the queue. `python3 tools/check_completion_integrity.py` audits the invariants still hold. |
| `queue park <func> --reason "…"` | block an item (e.g. needs user canonical-asm auth); `next` skips it |
| `queue status` | counts by status/verdict + the current top |
| `queue regen` | rebuild the queue (preserves done/parked); run after big changes |

Items routed `ASM-STRUCTURAL` / `ASM-WHOLE` sit in an `authorize` bucket (not `active`) — they need user
canonical-asm sign-off, so they never block pure-C work.

## The per-function loop + WIP checkpoints
The manual-path loop (`queue next` → `canonical` → `sandbox --disable all` → edit pure C → `retire` →
`queue done`), the WIP-checkpoint schema (`memory/wip/<func>/`), the near-duplicate-lead shortcut, the
score-lowered-not-0 checkpoint procedure, findings-registration, and the sandbox-vs-`build/` reference
gotcha are all in **[.claude/rules/decomp-loop.md](.claude/rules/decomp-loop.md)** ([[decomp-loop]]).
Load-bearing invariants: resume from the WIP checkpoint not HEAD; invoke `cheat-reviewer` on a candidate
BEFORE saving (FAIL ⇒ `rejected/<slug>.c`); WIP files are CURRENT-STATE docs under the
`wip_compaction_guard` caps; on COMPLETED-C delete `memory/wip/<func>/`. The Grinder uses its own richer
ledgers (`memory/grind/<func>/`) and seeds them from any existing WIP entry.

## Substrate — do NOT break
Load-bearing stage tools: cc1, maspsx, `regfix.py`/`asmfix.py`, `prologue_fix`, `multu_pad`, `fix_lwl`,
`as`/`ld`/`objcopy`, `make_psexe`, splat, decomp-permuter — plus the original EXE, `asm/`, `src/`,
`include/`, `disc/`, the `*.txt` configs, and the Makefile. The oracle guards every change; the engine's
own logic (distance metric, canonical gate, cheat-stripping) is pinned by `engine test` — keep it green
whenever you touch `engine/` code. (`audit_asm_cheats.py` is a manual detector only; the sandbox's
cheat-invisibility is the mechanical enforcement.)

## Guards (hooks)
Active: root-write cleanliness, CRLF/tooling-error (WSL env-failure) detection, the cc1psx-footgun block,
memory/CLAUDE.md hygiene (pre-commit), a SessionStart metrics-preflight (never blocks), and the
`commit-msg` chain (`tools/hooks/commit_msg_chain.sh` → install via `cp` to `.git/hooks/commit-msg`):
**park_src_guard** (blocks `park:` commits touching build-pipeline files; `[skip-park-src-guard]`
override), **no_new_regfix_guard** (net rule additions forbidden; `[infra-rule: <category>]` escape),
**wip_compaction_guard** (WIP current-state caps; `[skip-wip-compaction]` override). Legacy decomp-loop
hooks are removed.

**PreToolUse worktree-era guards — DORMANT since 2026-07-06** (no `bb2-work-*` worktrees exist; both are
NO-OPs in solo/grinder mode and fail open): **worktree_contamination_guard** + **main_reintegration_lock**
re-arm automatically if worker worktrees are ever recreated — don't recreate them while the Grinder runs
(see the decomp-grind skill's invariants; protocol in the hook docstrings + `tools/reintegrate_lock.ps1`).

## Metrics (`metrics/` + `tools/metrics/`)
Capture is silent and best-effort: `engine/cli.py` appends to `metrics/events.jsonl` (committed source of
truth) and can never raise or perturb output (contract pinned by `engine test`). The offline Postgres
layer is query-staleness only, never data loss (`tools/metrics/sync.py` + `report.py`, Windows side).
Full design: `metrics/README.md` + [[metrics-system]].

## Commit conventions
See [docs/COMMIT_CONVENTIONS.md](docs/COMMIT_CONVENTIONS.md). Engine work uses the `engine:` prefix.

## Getting help
CLI help: `/help`. New-session orientation: `memory/MEMORY.md` (auto-loaded) +
`memory/project/greenfield-engine-v2.md`.
