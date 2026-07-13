# Contributing to the BB2 Decompilation

The Bushido Blade 2 decomp is a long-lived, queue-driven matching project. The
goal is C source that compiles — through a frozen GCC 2.7.2 + ASPSX 2.34
toolchain — to a **byte-identical** copy of the original `SLUS_006.63`
executable. Most contributions are one-function-at-a-time: take the top of the
queue, write its C body in pure C, and prove the whole binary still SHA1-matches.

All decomp work now runs through the **decomp engine** (`engine/`) — a
deterministic spine that routes C-vs-asm, measures the honest cheat-free
distance, triages, and gates a byte+link match. The authoritative operating
docs are [`CLAUDE.md`](CLAUDE.md) (the workflow) and [`AGENTS.md`](AGENTS.md)
(tool-agnostic project facts). This file is the contributor-facing summary; when
it and `CLAUDE.md` disagree, `CLAUDE.md` wins.

Setting up a build instead? Start with [`BUILD.md`](BUILD.md).

## Before you start — the build must be green

Nothing else matters until the oracle passes. The oracle is the only truth:
"done" means the full build+link SHA1 equals
`62efab4f73f992798c43e8c730aa43baa10bb4fa`.

```powershell
& tools/wteng.ps1 main verify-oracle      # PowerShell host (zero-quoting wrapper)
```

or, inside WSL:

```bash
source .venv/bin/activate
python3 -m engine.cli verify-oracle
```

If the SHA1 doesn't match, fix the build before pulling work — a broken baseline
makes every "match" a false positive.

## The worklist is the queue

There is exactly one ordered worklist: [`engine/queue.json`](engine/queue.json).
Every function still carrying a cheat (a regfix/asmfix rule, a load-bearing
cheat-asm pin/`__asm__`, or a non-zero honest pure-C distance) is in it; reaching
a completed state drops it off.

```powershell
& tools/wteng.ps1 main queue next         # the top active item
& tools/wteng.ps1 main queue status       # counts by status/verdict + current top
```

The queue is **pre-ordered easiest-first** by honest pure-C distance. **No
cherry-picking and no deferral** (user directive 2026-06-12): work the top active
item to COMPLETED-C however many sessions it takes. A stuck item changes
**modality** (deeper search, new diagnostics, bulk sweeps), never target.

## Two workflows

### Grinder — the default autonomous pipeline

The [Grinder](tools/grinder/) (`tools/grinder/`) is a deterministic single-lane
driver that walks the queue top to COMPLETED-C across as many sessions as it
takes: a persistent per-function ledger (`memory/grind/<func>/`), a
driver-enforced modality ladder, no `blocked` outcome, and a pre-authorized
**default-FAIL Judge** that makes the final call on every autonomous completion.

```powershell
pwsh tools/grinder/grind.ps1              # run the driver (add -Once for one supervised pass)
pwsh tools/grinder/status.ps1             # progress
pwsh tools/grinder/grind.ps1 -Stop        # stop
```

Operate it via the **`decomp-grind` skill** ("start our decomp pipeline"). Owner
audit surfaces: `docs/grind/decisions.md` + `docs/grind/journal.md`. Full design:
[`docs/superpowers/specs/2026-07-06-grinder-pipeline-design.md`](docs/superpowers/specs/2026-07-06-grinder-pipeline-design.md).

### Manual — one focused agent, on `main`

To drive ONE function by hand, run the per-function engine loop directly on
`main` (the **`decomp-orchestrate` skill**). The full procedure lives in
[`.claude/rules/decomp-loop.md`](.claude/rules/decomp-loop.md); the short spine:

1. `queue next` — take the top item (resume from `memory/wip/<func>/` if a
   checkpoint exists; don't re-derive its rejected forms).
2. `verify-oracle --rebuild` once at session start (makes `build/` the clean
   reference the sandbox scores against).
3. `canonical <func>` — route. `ASM-region` / `ASM-STRUCTURAL` ⇒ stop the pure-C
   effort (authorized inline asm only). `C` ⇒ continue.
4. `sandbox <func> --disable all` — the honest cheat-free distance. Edit
   `src/<file>.c` toward the target, re-run; the score is your gradient.
   `diagnose <func>` explains a stuck gap.
5. **Score 0 ⇒ finish:** `retire <func>` (drops rules, rebuilds, SHA1-gates),
   then `queue done <func>`. **Lowered but not 0 ⇒ checkpoint** a WIP entry under
   `memory/wip/<func>/` instead of touching `src/`.

## Completion standards — the three states

Every function is in exactly ONE state. There are no gradations, no "almost
done." Cheats are never an end state. (Authoritative: CLAUDE.md §"The three
function categories".)

| State | Meaning |
|---|---|
| **INCOMPLETE** | In `engine/queue.json`. Carries a cheat (regfix/asmfix rule, cheat-asm pin/`__asm__`, or non-zero honest pure-C distance). |
| **COMPLETED-C** | Zero rules, zero cheat-asm, byte-identical. The SOTN community bar and the **default goal for every function**. |
| **COMPLETED-INLINE-ASM-CANONICAL** | Zero rules, canonical inline asm (GTE/cop2 ops, BIOS/syscall trampolines) or a whole-body `__asm__` glabel that is the accepted finished form. Listed in [`inline_asm_canonical.txt`](inline_asm_canonical.txt). Reserved for functions whose ORIGINAL code was hand-written assembly — the `canonical` gate decides what qualifies, and it needs owner authorization. |

Cheat catalog and the "cheats by any spelling" posture:
[`.claude/rules/inline-asm-policy.md`](.claude/rules/inline-asm-policy.md) and
[`.claude/rules/no-new-park-categories.md`](.claude/rules/no-new-park-categories.md).

## The non-negotiables

1. **The completion standard** — only the two COMPLETED states may be committed
   as done. `queue done` and the Judge refuse a function that still carries any
   rule or unauthorized cheat-asm.
2. **The oracle is the only truth** — "done" = full build+link SHA1 ==
   `62efab4f73f992798c43e8c730aa43baa10bb4fa`. Isolated scores are hints.
3. **Cheating can't help** — the sandbox scores with regfix/asmfix disabled and
   cheat-asm stripped, so adding a rule/pin/`__asm__` injection is score-inert.
   Don't reach for cheats; they're mechanically inert here.
4. **Canonical-asm is the gate's call** — if `canonical` says ASM-region or
   ASM-STRUCTURAL, do NOT grind it in pure C and never recreate target bytes via
   hardcoded-`$N` `__asm__` injection.

## Adversarial review — mandatory for every completion

No completion-class change is accepted on the author's word. A separately-defined
adversarial **`cheat-reviewer`** agent (default-FAIL, read-only, the author's
verdict not credited) must clear it first — the semantic layer on top of the
mechanical detectors. Full protocol:
[`.claude/rules/review-discipline-before-commit.md`](.claude/rules/review-discipline-before-commit.md).

- **Autonomous path:** the Grinder's default-FAIL **Judge** gates every
  completion.
- **Manual path:** invoke the `cheat-reviewer` explicitly (layer-2, fresh,
  adversarial brief) before any `Match:` / `cheat-cleanup:` / `auth:` /
  rule-doc-adding commit. FAIL ⇒ do not commit; follow the reviewer's
  `next_action`. NEEDS_USER ⇒ the question goes to the user, never self-resolved.

## Commit conventions

Subject prefixes: `Match:`, `cheat-cleanup:`, `auth:` / `inline_asm_canonical:`,
`wip:`, `park:`, `engine:`, `grind:`, `naming:`, `tools:`, `hooks:`, `docs:`.
Function names use the full `func_XXXXXXXX` form. `Match` / `cheat-cleanup:` /
`auth:` commits require a body with a `Pure-C attempts:` block. Full catalog and
body structure: [`docs/COMMIT_CONVENTIONS.md`](docs/COMMIT_CONVENTIONS.md).

Multi-line messages go through a file — `git commit -F tmp/msg.txt` — never a
heredoc or quote-escape dance.

## Substrate you must not break

The oracle guards every change, but these load-bearing pieces are the ground the
oracle stands on — don't casually alter their schema or contents:

- The engine (`engine/`) — its distance metric, canonical gate, and
  cheat-stripping are pinned by `engine test`; keep it green when you touch
  engine code.
- The oracle SHA1 (`62efab4f73f992798c43e8c730aa43baa10bb4fa`).
- The stage tools: cc1, maspsx, `regfix.py`/`asmfix.py`, `prologue_fix`,
  `as`/`ld`/`objcopy`, `make_psexe`, splat, decomp-permuter.
- `bb2.ld` is **HAND-MAINTAINED** — do NOT run `make setup` (it re-adds dead
  rodata lines that conflict with const decls now in `src/*.c`).
- The `regfix.txt` / `asmfix.txt` rule schemas and `inline_asm_canonical.txt`.
- The `asm/data/*.rodata*.s` segments are deliberately DELETED — don't recreate
  them.

## Line endings, PowerShell, environment

- **Build files** (`src/*.c`, `*.h`, `*.s`, `Makefile`, `*.ld`, pipeline `*.txt`)
  MUST use **Unix LF** line endings — CRLF silently breaks the GNU toolchain.
  Edit via WSL or an LF-enforcing editor. Details: AGENTS.md §File-edit
  conventions.
- **PowerShell-first scripting** — the engine runs under WSL, but hand-nesting
  `wsl bash -c '…'` breaks quoting. Run engine/build commands via
  `& tools/wteng.ps1 main <cmd>`; anything beyond one simple command goes in a
  `.py`/`.sh`/`.ps1` file in `tmp/`, not an inline string. Enforced by
  `shell_footgun_guard.py`. Details: CLAUDE.md §PowerShell-first.
- **Root cleanliness** — scratch goes in `tmp/` (gitignored), not the repo root.
  Details: AGENTS.md §Root-directory cleanliness.

## Where to read further

| File | Purpose |
|---|---|
| [`CLAUDE.md`](CLAUDE.md) | The authoritative workflow (engine, Grinder, queue, non-negotiables). |
| [`AGENTS.md`](AGENTS.md) | Tool-agnostic project facts (toolchain, build pipeline, disc structure). |
| [`docs/superpowers/specs/2026-07-06-grinder-pipeline-design.md`](docs/superpowers/specs/2026-07-06-grinder-pipeline-design.md) | Full Grinder design. |
| [`.claude/rules/decomp-loop.md`](.claude/rules/decomp-loop.md) | The manual per-function loop in full detail. |
| [`docs/MATCHING.md`](docs/MATCHING.md) | The matching playbook (GCC 2.7.2 codegen techniques, gotchas). |
| [`docs/ARCHITECTURE.md`](docs/ARCHITECTURE.md) | Build pipeline, splat split, regfix/asmfix model. |
| [`docs/GLOSSARY.md`](docs/GLOSSARY.md) | Terminology: PsyQ, MIPS, decomp, BB2-specific. |
| [`docs/STATUS.md`](docs/STATUS.md) | Live progress counts. |

## Code of conduct

Be kind. Decompilation is collaborative archaeology; we're all trying to
understand a 1998 build with limited evidence. Disagreements about naming,
technique, or strategy are normal — discuss them openly. Don't claim functions
you can't finish, don't reach for cheats to avoid hard work, and don't push
commits that break the oracle.
