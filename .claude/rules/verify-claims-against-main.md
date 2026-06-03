---
name: verify-claims-against-main
description: Workers in worktrees can read stale files and make confident claims about line numbers / sibling rules / link behavior that don't match current main. Always verify the file/line numbers and the empirical behavior before recording a claim in a WIP entry or as evidence in a commit message.
paths: ["memory/wip/**", ".claude/workflows/**", "tools/setup_worktree.ps1", "engine/wip.py"]
metadata:
  type: rule
---

# Verify file-line claims against MAIN, not worktree-stale state

> **Codified 2026-06-03** after a round-3 worker confidently reported a
> CBB0 meta-blocker (CE0C splice rules referencing absolute labels
> `.L152/.L154/.L158/.L174` at `regfix.txt:4208/4210/4212`) that turned
> out to be **stale-worktree hallucination** — those lines hold unrelated
> content in main, and CE0C's splice rules already use `{lbl#N}`
> function-local slots (drift-robust). The worker's stale `regfix.txt`
> pre-dated the `{lbl#N}` migration. Empirical deployment test proved the
> claimed link failure does not occur.

## The failure mode

A worker in an isolated git worktree reads a file (`regfix.txt`,
`asmfix.txt`, `src/<file>.c`, etc.) and finds content that **does not
match current main**, because the workflow harness sometimes creates
worktrees at a snapshot HEAD that lags by hours-to-days. The worker
makes a confident claim — quoting specific line numbers, label
references, or expected link behavior — and the synthesizer trusts it
and records it as a WIP `next_hypothesis` or a commit-message
evidence string.

Future workers then read the recorded claim, treat it as ground truth,
and either:
  - Pursue a "fix" for a problem that doesn't exist (the
    `{lbl#N}` migration for CE0C — already done, not needed)
  - Avoid a viable path because the stale claim said it was blocked

## What this rule requires

Before recording ANY of these in a WIP entry, `park:` evidence ledger,
commit message, or rule update:

| Claim type | Verification step |
|---|---|
| File X line N contains Y | `git show main:<file>` and check line N |
| Rule/code references label/symbol Z | `grep -n "Z" <file>` against current main |
| Link / build fails on configuration W | Actually run the build with that configuration and capture the error |
| Function F's regfix has N rules | `grep -c "^F:" regfix.txt` against current main |
| File X has fewer / more lines than expected | `wc -l <file>` against current main |
| Rule Y in `.claude/rules/<slug>.md` forbids Z | Read the rule's current content; do NOT cite an archived/superseded version |

If you cannot verify against main — e.g. you're in a worktree without
HEAD parity — **STATE IT EXPLICITLY**: "I read this in my worktree's
file but did NOT verify against main." The synthesizer will then know
to flag the claim for orchestrator-side verification before recording it.

## What WIP entries should record

When a session adds a `sessions[]` entry that cites file/line numbers or
sibling-rule effects, include:

```json
{
  "date": "YYYY-MM-DD",
  "git_head_at_measurement": "<7-char SHA>",  // git rev-parse --short HEAD
  ...
}
```

The HEAD SHA pins the citation to a verifiable repo state. A future
agent reading the entry can run `git log <head>..HEAD --stat` to see
exactly what changed since the measurement — and re-measure if anything
material changed in the cited files.

`engine.wip.record_head_sha()` provides a helper to read the current
SHA into a session entry.

## Standing protocols

1. **`setup_worktree.ps1` enforces HEAD parity** (commit `[this commit]`
   onwards). When invoked at the start of a worker run, it detects
   worktree HEAD ≠ main HEAD and hard-resets to main HEAD (default
   `WORKTREE_STALE_POLICY=sync`). Workers operating in a stale worktree
   without invoking this script are responsible for the contract above.

2. **Synthesizer responsibility.** When a worker's structured output
   includes specific file/line citations, the synthesizer should
   spot-check at least one before recording. If any check fails, the
   synthesizer flags the worker output for orchestrator escalation
   rather than trusting it.

3. **Re-measurement protocol.** If a WIP `sessions[]` entry's
   `git_head_at_measurement` is more than a few commits behind current
   HEAD AND the cited files have changed since, the documented score may
   not be reproducible. The next session's first action should be a
   re-measurement at current HEAD, with the result recorded in a new
   `sessions[]` entry.

## Confirmed case — func_8007CBB0 round 3 → orchestrator verification

The round-3 worker reported:
> "sibling func_8007CE0C carries 3 splice rules (regfix.txt:4208, 4210,
> 4212) that reference ABSOLUTE global labels (.L152, .L154, .L156,
> .L158, .L159, .L160, .L174)"

Orchestrator verification 2026-06-03 (commit `a1622b94`):
- `regfix.txt:4208/4210/4212` in current main hold a comment + an
  `exec_game` rule + blank line
- CE0C splice rules at `regfix.txt:4184/4186/4188/4189` use `{lbl#1}..{lbl#13}`
- Zero display.c functions have absolute `.L` label cheats anywhere in
  `regfix.txt` (audit script: `tmp/abs_label_audit.py`)
- Empirical deployment test: candidate body + bridge disabled →
  full build succeeded, no CE0C link failure

The worker's claim cost: round-3 synthesizer recorded the meta-blocker
as a `next_hypothesis` requiring `{lbl#N}` migration as a round-4
precondition. That work would have been a complete no-op. The
orchestrator caught it on review (commit `a1622b94` documents the
refutation + the standing-policy update in this rule).

## Related

- [[difficult-is-not-impossible]] — workers must own deriving facts;
  this rule is the verification arm of that discipline
- [[review-discipline-before-commit]] — the cheat-reviewer architecture
  for vetting candidate bodies; this rule extends "verify before commit"
  to citations of project state
- [[global-label-drift-sibling-cheat]] — the rule about the `{lbl#N}`
  mechanism the round-3 worker's stale state was unaware of
