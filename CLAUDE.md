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
The engine runs under WSL, but hand-authored `wsl bash -c 'cd … && source .venv && python3 -m
engine.cli …'` nests three shells and the quoting silently eats awk/sed/heredocs — the PreToolUse
guard BLOCKS that form and the footguns below.
- **Engine/build commands → PowerShell tool + `tools/eng.ps1 <cmd>`** (zero quoting; stamps
  `CLAUDE_SESSION_ID` for metrics) — e.g. `& tools/eng.ps1 sandbox func_X --disable all`.
- **Anything beyond ONE simple command** (awk/sed, multi-statement pipelines, heredocs) → **write
  a `.py`/`.sh`/`.ps1` file to `tmp/` and run that file** (more robust AND readable than inline).
- **Multi-line commit messages → `git commit -F tmp/msg.txt`.** Never heredocs or quote-escape dances.
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
`pwsh tools/headless_loop.ps1 -MaxIterations N` invokes `claude -p` once per queue item (one
focused agent per function). Guardrails: oracle baseline + authoritative post-check per
iteration, dirty-tree stop, never pushes; runs log to `metrics/headless_runs.jsonl` with
self-auditing efficiency signals from `tools/headless_audit.py` (`--all` = dashboard,
`--session <sid>` = drill-down). Full autonomy = `-PermissionMode bypassPermissions` (default);
`-DryRun` to preview. Both the runner and `tools/eng.ps1` stamp `CLAUDE_SESSION_ID` for metrics.

### Orchestrator post-run protocol (autonomous operation)
> The **`/decomp-orchestrate`** skill IS the orchestrator playbook (worker driving, review,
> retro-audit, escalation boundary, auto-handle categories, footguns). Invoke it for any
> orchestrator session; the load-bearing summary:

1. Per worker run: `python3 tools/headless_review.py --latest` (exit `0`=ACCEPT, `10`=ESCALATE);
   confirm PARK/STUCK findings yourself — workers over-claim.
2. **MANDATORY retro-audit (user directive 2026-06-10):** every completion-class commit
   (`Match:` / `cheat-cleanup:` / `auth:` / rule-doc additions) needs a FRESH adversarial
   `cheat-reviewer` (default-FAIL, worker's verdict not credited) before acceptance. FAIL ⇒
   byte-identical revert (`[infra-rule: reviewer-fail-revert]` for rule restorations; clean
   levers → WIP). NEEDS_USER ⇒ surface. Run `python3 tools/reviewer_precheck.py --func <f>`
   FIRST and paste its output into the brief — reviewer tokens go to semantics, not re-derived
   procedural facts. Batch sweeps: gate-tampering diff, `check_completion_integrity.py`,
   canonical-registry delta.
3. Fix recurring tooling friction the audit surfaces, and commit the fix.
4. Escalate ONLY for: oracle break, worker error, stuck/no-progress, unconfirmable park,
   architecture/policy decisions. **No deferral (user directive 2026-06-12):** never skip or
   rotate a stuck top item — change MODALITY (orchestrator deep-dive with sustained context,
   bulk variant sweeps, better diagnostics) on the SAME function.

**Auto-handle (NOT escalate):** GTE leaf wrappers (pure cop2, no C form — orchestrator
authorizes per user policy 2026-05-26, [[gte-wrapper-misroute-park]]; whole-body hand-coded asm
still escalates) and jtbl-infra parks (auto-confirmed; per-function rule retirement is queue work).

## Rodata cleanup project — COMPLETE 2026-06-09 (pointer)
All 12 `asm/data/*.rodata*` segments retired; jtbl-infra rules deleted; oracle unchanged. Full
log + recipes: `memory/project/rodata-cleanup-progress.md`. **Two standing warnings:**
1. **`bb2.ld` is HAND-MAINTAINED — do NOT run `make setup`** (re-adds dead rodata lines +
   conflicts with const decls now in `src/*.c`; recovery procedure in `splat.yaml`).
2. **`asm/data/*.rodata*.s` are deliberately DELETED** — don't re-create them.

## The queue IS the worklist (`engine queue`)
All outstanding work lives in ONE ordered list — `engine/queue.json` — covering every function
still carrying a cheat (a regfix/asmfix rule OR a load-bearing cheat-asm pin/inline-asm). Every
queue item is INCOMPLETE by definition; reaching a COMPLETED state drops it off the queue. The
queue is **pre-ordered easiest-first** by honest pure-C distance: **no triage, no cherry-picking,
and NO DEFERRAL (user directive 2026-06-12)** — work the **top active item** to COMPLETED, however
many sessions it takes; a stuck item changes MODALITY (deep-dive, bulk sweeps, new diagnostics),
never target. The SessionStart hook surfaces the top each session; `queue status` shows progress.

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
Multi-session functions checkpoint their best candidate + measured floor + hypotheses +
cheat-reviewer verdict in `memory/wip/<func>/` (candidate.c + meta.json + notes.md +
rejected/) — tracked in git, OUTSIDE the build pipeline, surfaced by the SessionStart hook and
`queue next`'s `wip` block. Full schema + usage: `memory/wip/README.md`. Load-bearing rules:
- **Resume from the checkpoint, not HEAD** — apply candidate.c, confirm the documented floor
  with `sandbox`, don't re-derive `rejected_forms`.
- **Cheat discipline:** invoke `cheat-reviewer` on the candidate BEFORE saving; FAIL ⇒ save
  under `rejected/<slug>.c` (named violated rule), not as candidate.c.
- **Compaction contract (ENFORCED by `wip_compaction_guard.py`, 2026-06-12):** WIP files are
  CURRENT-STATE docs — notes.md ≤120 lines (ONE TL;DR, rewritten in place), meta.json
  sessions[] ≤3 (fold older into `prior_sessions_summary`, one line each). History lives in git.
- On COMPLETED-C, **delete `memory/wip/<func>/`**; generalizable lessons → `.claude/rules/`.

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
   Exploring >3 candidate forms? Sweep them in ONE call: `python3 tools/sweep_variants.py --func
   <f> --file <stem> --variants tmp/<f>_variants/` (scores each form via the sandbox, restores src;
   low scores are PROPOSALS — vet against the cheat catalog).
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
   - **reusable pattern** ⇒ add/update a doc in `.claude/rules/<slug>.md` with `paths:
     [".claude/rules/<slug>.md"]` (ON-DEMAND — technique rules do NOT get a broad `src/*.c`
     glob; that cost ~200k tokens/session pre-2026-06-11) **AND add a one-line symptom entry
     to `.claude/rules/codegen-technique-index.md`** (the index is what auto-loads). The
     metrics layer fingerprints the `slug`. Link related rules with `[[other-slug]]`. Only
     enforcement-critical POLICY rules keep a broad glob.
   - **function-specific fact** ⇒ a `memory/` entry (per the memory rules in this file).
   - **routine / no-op match** ⇒ skip; don't manufacture a finding.
7. **Commit** (`cheat-cleanup:` / `Match` / `engine:` prefix per docs/COMMIT_CONVENTIONS.md).

**Reference gotcha:** the sandbox scores your edited, cheat-stripped `.o` against
`build/src/<file>.o`, which must stay the *pristine* canonical build (= the target bytes). During
the edit loop use only `sandbox` — it builds into `tmp/` and never touches `build/`. **Enforced
2026-06-12:** `verify-oracle --rebuild` REFUSES (exit 3) while build-input files have uncommitted
edits — that refusal means you're misusing it as an iteration tool; `--allow-dirty` overrides for
legitimate cases (e.g. mid-revert restoration). The final SHA1 gate is always honest.

## Substrate — do NOT break
Load-bearing stage tools: cc1, maspsx, `regfix.py`/`asmfix.py`, `prologue_fix`, `multu_pad`,
`fix_lwl`, `as`/`ld`/`objcopy`, `make_psexe`, splat, decomp-permuter — plus the original EXE,
`asm/`, `src/`, `include/`, `disc/`, the `*.txt` configs, and the Makefile. The oracle guards
every change; the engine's own logic (distance metric, canonical gate, cheat-stripping) is pinned
by `engine test` — keep it green whenever you touch `engine/` code. (`audit_asm_cheats.py` is a
manual detector only; the sandbox's cheat-invisibility is the mechanical enforcement.)

## Guards (hooks)
Active: root-write cleanliness, CRLF/tooling-error (WSL env-failure) detection, the cc1psx-footgun
block, memory/CLAUDE.md hygiene (pre-commit), a SessionStart metrics-preflight (never blocks), and
the `commit-msg` chain (`tools/hooks/commit_msg_chain.sh` → install via `cp` to
`.git/hooks/commit-msg`): **park_src_guard** (blocks `park:` commits touching build-pipeline
files; `[skip-park-src-guard]` override), **no_new_regfix_guard** (net rule additions forbidden;
`[infra-rule: <category>]` escape), **wip_compaction_guard** (WIP current-state caps;
`[skip-wip-compaction]` override). Legacy decomp-loop hooks are removed.

## Metrics (`metrics/` + `tools/metrics/`)
Capture is silent and best-effort: `engine/cli.py` appends to `metrics/events.jsonl` (committed
source of truth) and can never raise or perturb output (contract pinned by `engine test`). The
offline Postgres layer is query-staleness only, never data loss (`tools/metrics/sync.py` +
`report.py`, Windows side). Full design: `metrics/README.md` + [[metrics-system]].

## Commit conventions
See [docs/COMMIT_CONVENTIONS.md](docs/COMMIT_CONVENTIONS.md). Engine work uses the `engine:` prefix.

## Getting help
CLI help: `/help`. New-session orientation: `memory/MEMORY.md` (auto-loaded) +
`memory/project/greenfield-engine-v2.md`.
