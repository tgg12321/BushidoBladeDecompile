# CLAUDE.md

Claude-Code configuration for the **Bushido Blade 2** (SLUS-00663) matching decompilation.
Tool-agnostic facts (toolchain, build, executable, PowerShell-first scripting) live in
[AGENTS.md](AGENTS.md), imported here:

@AGENTS.md

## The workflow is the engine (`engine/`)

All decomp work goes through the deterministic decomp engine: route C-vs-asm → measure the honest,
cheat-free distance → verify a byte+link match. Run it with **`& tools/wteng.ps1 main <cmd>`** from
the PowerShell tool (never bare `make`, a relative `eng.ps1`, or a raw `wsl bash -c '… engine.cli …'` —
guards block those).

| Command | Purpose |
|---|---|
| `queue next` / `queue status` | the top active item / progress (always JSON) |
| `canonical <func>` | C-vs-asm gate (ASM-region / ASM-structural / C). **Run before any pure-C work.** |
| `sandbox <func> --disable all [--diff] [--candidate <path>]` | honest pure-C distance (cheat-asm stripped); `--diff` shows WHERE, per hunk (source-level / operand-only / not-scored) — read it before choosing a lever |
| `diagnose <funcs…>` / `dossier <func>` | classify a gap / the full live picture of a function |
| `verify-oracle [--rebuild]` / `build` | confirm the tree builds byte-identical |
| `layer2 hash\|record\|check <func>` | key / record / gate the layer-2 review of the exact landed body |
| `queue done <func>` | mark complete — refuses without zero non-canonical cheat-asm, a layer-2 PASS on this body, and SHA1 == oracle |
| `queue rotate <func> --reason …` | back of the worklist — **only when truly stuck across multiple sessions** |
| `queue auto-return` / `unpark` / `reopen` / `regen` | return rotated items / re-add a dropped function / rebuild (clean tree only) |
| `test` / `fixtures-verify` | engine regression suite / golden fixtures — keep green when touching `engine/` |

## Completion states — every function is in exactly one

- **INCOMPLETE** — in `engine/queue.json`, committed as `INCLUDE_ASM("asm/funcs", <func>);`
  ([[asm-until-matched]]); candidates in `memory/grind/<func>/`.
- **COMPLETED-C** — pure C, zero cheat-asm, byte-matches. The default goal (SOTN bar).
- **COMPLETED-INLINE-ASM-CANONICAL** — GTE/cop2, BIOS/syscall trampolines, or whole-body asm whose
  original was hand-written; listed in `inline_asm_canonical.txt`. The `canonical` gate decides, not
  the agent.

Cheats (register pins, hardcoded-`$N` `__asm__`, scheduling barriers, build-time output rewriting)
are never an end state, by any spelling ([[no-new-park-categories]]). The sandbox scores with them
stripped and `queue done` audits the source, so they can't help. **The oracle is the only truth:**
full build+link SHA1 == `62efab4f73f992798c43e8c730aa43baa10bb4fa`; isolated scores and exit codes
are hints. `tools/check_completion_integrity.py` is the standing audit.

## The queue is the worklist

`engine/queue.json` is pre-ordered easiest-first. Work the **top active item** to COMPLETED — no
triage, no cherry-picking, no deferral. A stuck item changes MODALITY (deep-dive, sweeps, new
diagnostics), never target ([[rotation-not-foreclosure]]). `ASM-STRUCTURAL` / `ASM-WHOLE` items sit
in an `authorize` bucket and take the Judge-gated canonical-grant path ([[judge-sole-gate]]).

## Lanes

- **The Grinder** (`tools/grinder/`, default autonomous pipeline) — **invoke the `decomp-grind`
  skill.** Persistent per-function ledger, driver-enforced modality ladder, bytes proven on main
  before a default-FAIL Judge rules. Owner audit surfaces: `docs/grind/decisions.md`, `journal.md`.
  Never edit tracked files while it runs — its scope check discards the live session.
- **Manual lane** — one focused agent on `main`, one function: **invoke the `decomp-manual` skill**
  (`pwsh tools/manual_session.ps1 begin`). Per-function loop details: `.claude/rules/decomp-loop.md`.
- Every manual completion-class commit (`Match:` / `cheat-cleanup:` / `auth:` / rule additions)
  needs a fresh adversarial `cheat-reviewer` PASS first ([[review-discipline-before-commit]]).
  GTE leaf wrappers and jtbl-infra parks are handled without escalation
  ([[inline-asm-policy]]).

## Standing warnings

- `bb2.ld` is hand-maintained — never `make setup`; never recreate `asm/data/*.rodata*.s`.
- Don't recreate `bb2-work-*` worktrees (they re-arm the worktree-era guards and block the Grinder).
- Hooks self-explain when they block; the `commit-msg` chain installs via
  `cp tools/hooks/commit_msg_chain.sh .git/hooks/commit-msg`. Root-write cleanliness is wired in
  `settings.local.json` (a fresh clone must re-enable it).
- Metrics capture (`metrics/events.jsonl`) is silent and best-effort — see `metrics/README.md`.
- Commit conventions: `docs/COMMIT_CONVENTIONS.md` (engine work uses `engine:`).

## Documentation budget (owner directive 2026-10-01)

Files hold **current state**; **history lives in git** (commit-message bodies, tag
`pre-slim-2026-10-01`). The repo once held 16 MB of markdown, and reading one `src/*.c` file
auto-loaded ~100K tokens of rules. Don't rebuild that.
- **No new .md for an action.** Session reports, handoffs, audits, plans, campaign write-ups
  and investigation notes go in the commit message body, or a few lines in an EXISTING doc.
  A new file needs `[new-doc]` + a justification in the commit body.
- **Rules (`.claude/rules/`) hold the operative rule only:** what to do, the test, at most one
  short example. ≤ 8 KB each (the four policy authorities ≤ 24 KB). No Q&A transcripts, dated
  amendment chains, or case histories — edit the rule in place; the diff is the history.
  Rules matching `src/*.c` must total ≤ 60 KB; give new technique rules self-only `paths:` and an
  index line in `codegen-technique-index`.
- **Owner rulings:** record the operative change in the rule; put the verbatim exchange in the
  commit body, not a new doc.
- **Grind ledgers** ≤ 64 KB per file (`grindlib.py compact-ledger <func>`; the driver
  auto-compacts) and closed on completion. **Logs** are rotated: `tools/rotate_grind_logs.py`
  (decisions/journal), `tools/metrics/rotate.py` (events.jsonl).
- **Harness memory** ≤ 10 notes of ≤ 6 KB: non-obvious, still-true facts the repo can't tell
  you. Add a bullet to an existing note; never duplicate a rule or doc.
- Cite with `path:line`; content that's gone resolves at a tag or commit (`<tag>:path:line`).
Enforced by `tools/hooks/doc_budget_guard.py` (commit-msg) and `memory_write_guard.py`
(PreToolUse). Override with `[skip-doc-budget]` and a reason.
