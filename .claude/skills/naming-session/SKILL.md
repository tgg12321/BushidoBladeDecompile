---
name: naming-session
description: >-
  Run one bounded Phase 3 naming session on BB2: read docs/naming/phase3/backlog.tsv, do at most
  three units (LAND a reviewed wave, or PREPARE a new one), leave main clean and pushed after each
  unit, and stop. Use when asked to continue naming, run a naming session, land waves, or keep the
  naming work going.
---

# BB2 naming: one bounded session

The rules for names live in `.claude/rules/naming-bar.md`. This skill covers how a session is run.

## Shape of a session

- **Start:** `git status` must be clean on `main`. Then read `docs/naming/phase3/backlog.tsv`.
- **Work:** do at most **3 units**, each from the list below. A unit ends at a **stopping point**:
  main is clean, gated and pushed, and the backlog row is updated in the same commit.
- **Stop:** after 3 units, or earlier if context passes ~300k or a unit fails twice. Report one line
  per unit, then stop. Don't hand off and don't wait on anything. The next session starts from the
  backlog.
- **Pick order:** LAND first (oldest `reviewed` row), then PREPARE (first `idea` row).

## Agents are bounded

- No long-lived teammates or lanes. Every subagent gets **one task**, returns, and is done.
- Run subagents in the **foreground** (the call blocks until it returns) so the session never idles
  waiting for a notification. Independent tasks can go as several Agent calls in one message.
- Models: `naming-reviewer` for every review; `general-purpose` with `model: sonnet` for drafting
  and mechanical applies; Codex (`tools/codex_worker.py research`) for research only.
- Codex is the one background job. Start it as the **last** step of a session, and record the
  backlog row as `researching`. The next session extracts the result.

## Backlog (`docs/naming/phase3/backlog.tsv`)

The columns are `id  kind  stage  draft  note`. `kind` is `func`, `member` or `data`.
`draft` is the tracked draft dir `docs/naming/phase3/pending/<id>/` (rows.py, member_manifest.csv,
pairs.txt, dry.txt). Stages:

`idea` -> `researching` -> `researched` -> `reviewed` -> `landed` (or `dropped`, with the reason in `note`)

Commit a stage change with the files that justify it, for example `docs: naming backlog -- m09
reviewed`.

## Unit PREPARE (idea or researched -> reviewed)

1. `idea`: write the Codex prompt (templates: `tmp/codex/prompt-research-m05.md` for members,
   `tmp/mkprompt.py` and `tmp/queued_in.py` for functions). Tell it which names are taken and which
   earlier rows failed. Launch it, mark the row `researching`, and that ends the unit.
2. `researching` or `researched`: `python tmp/codex/extract_final.py <run dir> tmp/codex/<id>-research.md`.
3. Sonnet drafter (foreground) writes `docs/naming/phase3/pending/<id>/`. For members this
   includes a compiled dry run (`tools/naming_member_apply.py --dry-run`, run from PowerShell).
   The drafter re-derives every use and drops anything it can't show.
4. `naming-reviewer` pre-apply (foreground).
   - Rows that FAIL are dropped.
   - A name the reviewer only *suggests* may replace a failed one, but it is marked
     `suggested` in the manifest, and R6 must judge it fresh at every use.
   - If no row survives, the unit ends as `dropped`.
5. Set VERDICT CONFIRM with a verifier note, commit the draft dir and the backlog row as
   `reviewed`, and push. That is the stopping point.

## Unit LAND (reviewed -> landed)

1. A Sonnet applier (foreground) works on a fresh branch off main, in the worktree
   `bb2-worktrees/apply` (create it if it is missing; never `bb2-work-*`). It runs the wave apply
   (naming-bar §The wave, step 4) and the comment pass, then the gates:
   - `verify-oracle --rebuild`
   - `tus-check`
   - the integrity check
   - `engine test`
   - `naming_keycheck`

   It commits the wave plus a `docs:` comment commit and returns the hashes. It does not touch
   main.
2. `naming-reviewer` R6 (foreground) on those commits: identifiers only, comments true, no stale
   old names in live docs and registries, and suggested names judged fresh.
3. On main:
   - `git cherry-pick` the commits.
   - Apply R6's blocking fixes verbatim in a `docs:` commit.
   - Rebuild, then run `python docs/naming/build_census.py` and commit any census change.
   - Run `python tools/test_label_census.py`.
   - Run `python tmp/wavefollow.py <id> symbol_addrs.txt` (function waves).
   - Run the integrity check.
   - Set the backlog row to `landed`.
   - Push. That is the stopping point.
4. If R6 fails on a name, revert that pair on the branch (or redo the apply without it) and run R6
   again. On a second failure, mark the row `dropped`.

## Gate commands (from PowerShell)

- `& tools/wteng.ps1 main verify-oracle --rebuild` (commit first, because it refuses dirty build
  inputs)
- `wsl bash '/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile/tmp/reaudit/land/integ.sh'`
- If a cherry-pick conflicts on the census: `git checkout --ours docs/naming/function-names.csv`,
  then regenerate it after the rebuild.

## Hard limits

- Never `make setup`, never recursive force deletes, and no attribution lines. Commit with `-F`.
  Python writes use `newline='\n'`.
- Quoting through shells is blocked by the footgun guard: put scripts in `tmp/*.py` and messages in
  `tmp/*.txt`.
- Owner rulings: don't revisit Kengo symbols; packed-RECT words in Sony code are idiom; one member
  name may be shared across types only when the code shows the same meaning and every pair lands
  in one wave.
