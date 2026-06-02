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

### Run engine commands via the PowerShell wrapper (`tools/eng.ps1`)
The engine runs under WSL (Linux toolchain), but **do not hand-author
`wsl bash -c 'cd … && source .venv && python3 -m engine.cli …'`** — through the harness that
nests three shells (Git Bash → wsl → bash) and the quoting silently eats awk/sed/heredocs. Use
the **PowerShell tool + `tools/eng.ps1`** (zero quoting; also stamps `CLAUDE_SESSION_ID` so the
run is attributed in metrics):
```
& tools/eng.ps1 <cmd>     # e.g. queue next | canonical func_X | sandbox func_X --disable all | verify-oracle --rebuild
```
`tools/hooks/shell_footgun_guard.py` (PreToolUse) BLOCKS the hand-rolled `python3 -m engine.cli`
form and the footguns below.

### PowerShell-first scripting (enforced by `shell_footgun_guard.py`)
- **Engine/build commands → PowerShell tool + `tools/eng.ps1`.** No `cd`, no `source`, no quoting.
- **Anything beyond ONE simple command** (awk/sed, multi-statement pipelines, shell functions,
  heredocs) → **write a `.py`/`.sh`/`.ps1` file to `tmp/` and run that file.** Inline complex shell
  through `wsl bash -c '…'` is the #1 time-waster here; a Python script is more robust *and* readable
  (e.g. `tmp/norm_diff.py` — a disassembly normalizer — worked first try where inline `awk` failed thrice).
- **Multi-line commit messages → `git commit -F tmp/msg.txt`** (Write the file, then commit). Never the
  `<<'EOF'` heredoc or the `'"'"'` quote-escape dance.
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
Every function in the codebase is in exactly ONE state. There are no gradations,
no "almost done," no "done with a hint." Cheats — regfix/asmfix rules, register
pins, cheat-asm `__asm__` blocks, scheduling barriers — are NEVER an end state.

  - **INCOMPLETE** — in `engine/queue.json`. Carries a cheat (regfix/asmfix
    rule, cheat-asm pin/__asm__, OR non-zero honest pure-C distance). Stays
    in the queue until it reaches a COMPLETED state.
  - **COMPLETED-C** — zero rules, zero cheat-asm in source, byte-matches.
    Not in the queue. Not listed in `inline_asm_canonical.txt`. The SOTN
    community bar; the default goal for every function.
  - **COMPLETED-INLINE-ASM-CANONICAL** — zero rules, canonical inline asm
    (GTE/cop2 ops, BIOS/syscall trampolines) or whole-body `__asm__("glabel
    ...")` that is its accepted finished form. Listed in
    `inline_asm_canonical.txt`. Reserved for functions whose ORIGINAL CODE
    was hand-written assembly — the `canonical` gate decides what qualifies,
    not the agent.

### The non-negotiables (enforced by construction, not by nagging)
1. **The completion standard** — only the two COMPLETED states above may be
   committed as done. **Enforced, not honor-system:** `queue done` and `regen`
   refuse to record a function as done if it carries any regfix/asmfix rule OR
   any cheat-asm (register pins, hardcoded-`$N` asm, scheduling barriers)
   unless it's authorized canonical-asm in `inline_asm_canonical.txt`. SHA1
   alone can't catch a cheat-asm "match" (the asm emits the right bytes), so
   the gate audits the source. `tools/check_completion_integrity.py` is the
   standing audit; the orchestrator's `headless_review` flags any cheated
   committed match.
2. **The oracle is the only truth** — "done" = full build+link SHA1 ==
   `62efab4f73f992798c43e8c730aa43baa10bb4fa`. Intermediate exit codes and isolated scores are hints.
3. **Cheating can't help** — the sandbox scores with regfix/asmfix disabled and
   cheat-asm stripped, so adding a regfix rule / pin / `__asm__` injection does
   not move the score. The integrated gate catches anything snuck in. Don't
   reach for cheats; they're inert here.
4. **Canonical-asm is the gate's call** — if `canonical` says ASM-region or ASM-STRUCTURAL, do NOT
   grind it in pure C. Genuine no-C-form constructs get authorized inline asm; never recreate target
   bytes via hardcoded-`$N` `__asm__` injection.

## Workflow: a single focused agent, on main
Decomp work runs **directly on `main`** — one focused agent, end-to-end. No worktrees, no
subagents, no orchestrator/worker split: the engine is a toolkit the agent drives itself.
**Build files (`src/*.c`, `*.h`, `*.s`, `Makefile`, `*.ld`, pipeline `*.txt`) MUST use LF line
endings** — edit via WSL or an LF-enforcing editor (the Write tool produces LF on this machine).

### Optional: headless driver (`tools/headless_loop.ps1`)
The same single-agent loop can run **unattended**: `pwsh tools/headless_loop.ps1` invokes `claude -p`
once per queue item to take the top function to COMPLETED (still one focused agent per function — it
just launches the sessions for you). Guardrails: `verify-oracle --rebuild` baseline + authoritative
post-check each iteration (stops on any oracle break), progress check (stops if the function neither
completes nor parks), a `--max-budget-usd` cap, `-MaxIterations 1` default, and it **never pushes**.
Each run is logged to `metrics/headless_runs.jsonl` (func, model, session, cost, tokens, turns,
oracle_ok). Full autonomy needs `-PermissionMode bypassPermissions` (the default); use `-DryRun` to
preview and `-PermissionMode acceptEdits` to supervise. Metrics attribution: the runner sets a
per-run `CLAUDE_SESSION_ID` (so `engine/metrics.py` stamps events.jsonl); `tools/eng.ps1` does the
same for interactive sessions.

**Audit a run without reading the transcript** — `tools/headless_audit.py` digests a session's huge
`~/.claude/projects/<munged>/<sid>.jsonl` into a few lines of signal (turns, tokens, est cost,
tool-call mix, tooling errors, `shell_footgun_guard` blocks, retried/duplicate commands, engine-loop
trace). `python3 tools/headless_audit.py --all` is a one-line-per-run dashboard (REVIEW vs OK);
`--session <uuid>` / `--latest` drills into one; `--flags` shows only concerning runs. The headless
runner folds these signals straight into each `headless_runs.jsonl` record, so the run log is
self-auditing — you only open the transcript via this tool if a number looks off.

### Orchestrator post-run protocol (autonomous operation)
> Invoke the **`/decomp-orchestrate`** skill to bootstrap an orchestrator session — it bundles this
> whole playbook (the completion standard + enforcement, PowerShell-first tooling, the per-function
> loop, headless worker driving + review, the escalation boundary, auto-handle categories, footguns,
> and the rule/memory pointers) in one place.

When driving workers (interactively or headless), the **orchestrator** (the main agent) runs this
after **each** worker finishes — it is what lets the loop run unattended for long stretches:

1. **Review packet** — `python3 tools/headless_review.py --latest` (or `--func`/`--session`).
   One compact view: outcome (COMPLETED / PARKED / STUCK / ORACLE-BREAK / ERROR), the audit signals
   (tooling errors, footgun blocks, retried commands, engine-loop trace), the worker's commit
   (`--stat`), the park reason, and a **mechanical park-confirmation** + an `ACCEPT`/`ESCALATE`
   recommendation. Exit code: `0`=ACCEPT, `10`=ESCALATE.
2. **Confirm findings** — on PARK/STUCK, don't trust the worker's rationale. `headless_review`
   auto-confirms *known* park categories (e.g. jtbl-infra: verifies the rules really are
   jump-table asmfix, references `jtbl_*`, zero regfix). A novel/unconfirmable park ⇒ ESCALATE.
3. **Apply tooling/workflow fixes** — if the audit shows recurring friction an agent shouldn't have
   to fight (a too-tight guard, a crashing tool, a function class that should be auto-routed), fix
   it and commit so future workers don't hit it. (This pass produced the 500-char guard bump, the
   sandbox no-crash fix, and the jtbl→`authorize` auto-route.)
4. **Continue or escalate** — per the **escalation boundary (maximal autonomy)**: keep going
   autonomously for clean completions and auto-confirmed parks; **STOP and surface to the user only
   for**: an oracle break, a worker error, a stuck/no-progress run, a park you can't mechanically
   confirm, an architecture/policy decision (e.g. the global rodata reorder behind jtbl-infra), or
   the budget cap. Everything else is logged (`headless_runs.jsonl`) for later review, not blocked on.

**Orchestrator auto-authorize categories (NOT escalate triggers):**
- **GTE leaf wrappers** (pure cop2 ops — `mtc2`/`avsz3`/`avsz4`/`mfc2` etc., no C form): the
  orchestrator authorizes them itself (user policy, 2026-05-26) — remove the cheat-asm
  `register asm` pin (GCC returns in `$v0` naturally), `verify-oracle --rebuild`, add to
  `inline_asm_canonical.txt`, `queue done`. See [[gte-wrapper-misroute-park]]. *Contrast
  whole-body hand-coded asm (custom ABI / trapping ops / hand scheduling) — that still needs a
  user judgment call → escalate.*
- **jtbl-infra** (rodata-split jump tables): auto-confirmed park (`cheats.is_jtbl_infra`); the
  *global rodata reorder* to truly pure-C them is the architecture decision that escalates.

## The queue IS the worklist (`engine queue`)
All outstanding work lives in ONE ordered list — `engine/queue.json` — covering every function
still carrying a cheat (a regfix/asmfix rule OR a load-bearing cheat-asm pin/inline-asm). Every
queue item is INCOMPLETE by definition; the moment a function reaches a COMPLETED state it drops
off the queue. The queue is **pre-ordered easiest-first** by honest pure-C distance, so there is
**no triage and no cherry-picking**: you work the **top active item** to COMPLETED, mark it `done`,
and take the next. Everything gets decompiled eventually, so don't skip "hard" items or hunt for
"easy" ones — just work the top. The SessionStart hook surfaces the top each session;
`queue status` shows progress.

| `queue` action | Purpose |
|---|---|
| `queue next` | print the top active item (func, file, verdict, distance, rule count) |
| `queue done <func>` | mark complete — re-checks ZERO rules + ZERO non-canonical cheat-asm + build SHA1 == oracle (refuses otherwise). On success the function is REMOVED from the queue (queue presence = INCOMPLETE). `python3 tools/check_completion_integrity.py` audits that every completed function still satisfies the invariants. |
| `queue park <func> --reason "…"` | block an item (e.g. needs user canonical-asm auth); `next` skips it |
| `queue status` | counts by status/verdict + the current top |
| `queue regen` | rebuild the queue (preserves done/parked); run after big changes |

Items routed `ASM-STRUCTURAL` / `ASM-WHOLE` sit in an `authorize` bucket (not `active`) — they need
user canonical-asm sign-off, so they never block pure-C work.

## WIP checkpoints (`memory/wip/<func>/`)
Most outstanding INCOMPLETE functions need MULTIPLE sessions to close — a partial-progress lever
gets the score from 20 → 12 but doesn't retire the rules, the next agent comes in cold and
re-derives what the prior session already knew, momentum is lost. **WIP entries are the fix.**

A WIP entry checkpoints the prior session's best candidate body + measured floor + technique +
remaining-gap analysis + next-step hypotheses + cheat-reviewer verdict — all session-survivable,
all tracked in git, all OUTSIDE the build pipeline (so `verify-oracle` is unaffected). A fresh
agent resumes from `meta.json` in one read, applies `candidate.c` to `src/`, and confirms the
documented floor with `sandbox` before iterating.

| File | Purpose |
|---|---|
| `memory/wip/<func>/candidate.c` | The candidate C body. Copy-pasteable into `src/<file>.c`. NOT compiled — `memory/` is outside cpp/cc1/maspsx. |
| `memory/wip/<func>/meta.json` | Machine-readable: scores (HEAD floor + candidate floor), sessions[] ledger, reviewer verdict, remaining_gap, next_hypotheses, rejected_forms. Schema in `memory/wip/README.md`. |
| `memory/wip/<func>/notes.md` | Human-readable: TL;DR, resume instructions, pointers to deeper `memory/project/` notes. |
| `memory/wip/<func>/rejected/<slug>.c` | OPTIONAL: forms that hit lower floors but failed cheat-reviewer. Preserved as "don't re-derive this" warnings. |

**When to author a WIP entry.** Whenever a session ends with the function still INCOMPLETE but
some measured progress (a lower floor than HEAD's). The `wip:` commit prefix scopes it; the
`park_src_guard` hook doesn't fire (`memory/` is outside its scope). When the function eventually
reaches COMPLETED-C, **delete `memory/wip/<func>/`** — the entry's purpose is served; lessons
that generalize move into `.claude/rules/`.

**Cheat discipline.** The cheat-reviewer must be invoked on the candidate before saving. A FAIL
verdict means save the form under `rejected/` instead, with the violated rule named. WIP entries
are NOT a back-door for un-vetted cheats — the reviewer verdict is recorded in `meta.json` for
every checkpoint.

**Hook integration.** The SessionStart hook (`tools/hooks/queue_top.py`) checks for a WIP entry
on the top active function and prints a one-line banner: candidate floor, sessions count, latest
lever, reviewer status. `engine queue next`'s JSON output includes a `wip` block when present.
See `memory/wip/README.md` for the full contract.

## The loop (per function)
The agent *is* the gap-closer — the engine measures, routes, and gates; you write the C.

0. **Take the top of the queue** — `queue next` (also surfaced by the SessionStart hook). Work THAT
   function to completion before taking another; don't cherry-pick. (Override only if the user names
   a specific function.) **If the top function has a WIP checkpoint** (banner in the SessionStart
   hook + `wip` block in `queue next`'s output), READ `memory/wip/<func>/meta.json` + `notes.md`
   FIRST — apply `candidate.c` to `src/<file>.c`, confirm the documented floor with `sandbox`,
   and continue from there instead of starting from HEAD. The `rejected_forms` field lists
   constructs the prior agent ruled out; don't re-derive them.
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
   just needs `verify-oracle --rebuild` to confirm the byte+link match. **Note (masked-0 caveat):**
   the sandbox distance is masked (register names normalised out), so a `0` can hide a real register
   diff — `retire`/`verify-oracle` (full SHA1) is the only proof. If `retire` rolls back, the gap is
   genuine reg-alloc work; keep editing.
   Then `queue done <func>` records completion (it re-verifies ZERO rules + SHA1 == oracle), AND
   delete `memory/wip/<func>/` if one existed (the checkpoint's purpose is served on close-out). If
   the item is genuinely not pure-C-closable (canonical-asm needing user auth, or a documented
   plateau), `queue park <func> --reason "…"` instead so the queue advances.
5b. **Score lowered but not 0 ⇒ checkpoint.** If you measurably lowered the floor below HEAD's but
   couldn't close, do NOT modify `src/` (oracle stays green). Save the progress as a WIP entry under
   `memory/wip/<func>/`: candidate.c + meta.json (append to `sessions[]`, update
   `scores.candidate_floor`, record any rejected forms) + notes.md. **Invoke `cheat-reviewer` on the
   candidate FIRST** — record the verdict in `meta.json.reviewer`. Commit under `wip: <func>`.
   Next session resumes from your checkpoint instead of from HEAD. The next agent reads `meta.json`,
   applies `candidate.c`, confirms the documented floor, iterates from there.
6. **Register findings.** Before committing, ask: did this match reveal a *reusable* codegen
   pattern or a non-obvious gotcha that the next agent would benefit from? If yes, record it where
   future agents will actually see it:
   - **reusable pattern** ⇒ add/update a path-scoped doc in `.claude/rules/<slug>.md` (with a
     `paths:` glob so it auto-loads when an agent reads a matching source file; the metrics layer
     fingerprints it as a technique `slug`). Link related rules with `[[other-slug]]`.
   - **function-specific fact** ⇒ a `memory/` entry (per the memory rules in this file).
   - **routine / no-op match** ⇒ skip; don't manufacture a finding.

   This human-written record is the durable one — the old `capture-recipe` tool and the
   `tools/recipes/` library were archived 2026-05-26 (`archive/dcsh_workflow_2026-05-26/recipes/`).
   `retire` prints this reminder on success.
7. **Commit** (`cheat-cleanup:` / `Match` / `engine:` prefix per docs/COMMIT_CONVENTIONS.md).

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
The engine's own logic (distance metric, canonical gate, cheat-stripping) is guarded by `engine
test` — a regression suite that must stay green whenever you change `engine/` code. Its
cheat-invisibility tests are the mechanical proof that cheats are score-inert, so there is **no
commit-time cheat audit** (`audit_asm_cheats.py` remains only as a manual detector).

## Guards (hooks)
Active: root-write cleanliness, CRLF/tooling-error (WSL env-failure) detection, the cc1psx-footgun
block, memory/CLAUDE.md hygiene, a SessionStart metrics-preflight (notifies if the metrics
Postgres is down; never blocks), and a `commit-msg` **park_src_guard** (blocks `park:` commits
that modify build-pipeline files — install via `cp tools/hooks/park_src_guard.py
.git/hooks/commit-msg`; override with `[skip-park-src-guard]` in the commit body when a `park:`
legitimately fixes a sibling). The legacy decomp-loop hooks (`grind_check`, `resilience_judge`,
`escape_valve`, …) and the main-branch publish-only guard have been removed.

## Metrics (`metrics/` + `tools/metrics/`)
Decomp runs are measured automatically: function worked, agent/model, time, tokens, outcome,
technique links. **Capture is silent and best-effort** — `engine/cli.py` calls
`engine/metrics.py` after each result; it only appends to `metrics/events.jsonl` (committed source
of truth) and can never raise, print, or perturb a command's output. The Postgres analytics layer
(`bb2_metrics`) is offline: run `python tools/metrics/sync.py` then `report.py` on the **Windows
side** to query. Postgres down = query-staleness, never data loss. Full design + heuristics:
`metrics/README.md` and the [[metrics-system]] memory. Don't add metrics writes to the hot path
that aren't guarded by the silence/swallow contract (pinned by `engine test`).

## Commit conventions
See [docs/COMMIT_CONVENTIONS.md](docs/COMMIT_CONVENTIONS.md). Engine work uses the `engine:` prefix.

## Getting help
CLI help: `/help`. New-session orientation: `memory/MEMORY.md` (auto-loaded) +
`memory/project/greenfield-engine-v2.md`.
