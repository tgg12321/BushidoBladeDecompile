---
name: naming-session
description: >-
  Run one bounded Phase 3 naming session on BB2 in its own lane worktree: read
  docs/naming/phase3/backlog.tsv, do at most three units (LAND a reviewed wave, or PREPARE a new
  one), land each through tools/lanes/lanes.py, and stop. Several sessions can run at once on
  different lanes. Use when asked to continue naming, run a naming session, land waves, or keep the
  naming work going. Argument: the lane (default naming-a).
---

# BB2 naming: one bounded session

The rules for names live in `.claude/rules/naming-bar.md`. This skill covers how a session is run.

Invocation: `/naming-session [lane]`, for example `/naming-session naming-b`. The lane defaults to
`naming-a`. If that lane is owned by another live session, take the next free `naming-<letter>`.

## The lane model

- The session owns **one** worktree, `../bb2-worktrees/ln-<lane>` (branch `lanes/<lane>`). Every
  apply, edit, build and commit happens there. Main is only a landing pad.
  `tools/hooks/lane_guard.py` blocks edits to main, edits to other lanes, and unpinned `git`
  mutations. The old shared `bb2-worktrees/apply` worktree is retired.
- Subagents start with their cwd in **main**. Give every subagent the absolute worktree path, and
  have it use absolute paths, `git -C "<wt>"`, `& tools/wteng.ps1 "<wt>" <cmd>`, and
  `cd "<wt>" && python …` for cwd-relative tools.
- **Claims.** Before taking a backlog row, run `python tools/lanes/lanes.py claim <lane> topic:row:<id>`.
  It refuses a row another live lane holds; take the next one. Before PREPARE, also claim what the
  wave will name: `topic:tu:<tu>` for function waves, or `topic:type:<Type>` for member waves. Pick
  targets no other lane has claimed (`lanes.py status`). Parallel lanes then never name the same
  thing.
- **Landing.** `python tools/lanes/lanes.py land <lane>` replays the lane's commits onto the current
  main in a private landing worktree, then:
  - runs the gates (`verify-oracle --rebuild`, `tus-check`, integrity, `engine test`,
    `test_label_census.py`) and checks the SHA1;
  - regenerates the census (never merge `function-names.csv` by hand);
  - merges backlog rows row by row;
  - fast-forwards main and pushes (the main lock is held only for this last step).

  A land takes many minutes, so run it with `run_in_background: true` and wait for the
  completion notice, never under a short tool timeout. After an interruption, run
  `lanes.py recover <lane>` (the next `land` also does it). Exit 4 means a gate failed; exit 5
  means main moved but the push failed: retry with `python tools/lanes/lanes.py push`, and tell
  the user if that fails too.
- **Bounces.** If `land` bounces (exit 3: a real conflict, for example the same line of
  `named_syms.txt` renamed by two lanes), run `lanes.py drop <lane> <sha>` for the commit it names
  (later commits, such as the next unit, are kept) and redo that apply on the new main. The wave
  tools are deterministic, so this takes minutes. `fresh --discard` throws away every unlanded
  commit (the tip is saved under `refs/lanes-backup/`); use it only when that is the intent.
  Never hand-merge into main.
- **Lane-Replay.** A commit that is exactly tool output (no hand edits) may carry
  `Lane-Replay: <shell command>` trailers, run at the worktree root in WSL with the venv. On a
  conflict, `land` re-runs the commands instead of bouncing.
- **Commit messages.** Write them to main's `tmp/` and pass an absolute path:
  `git -C "<wt>" commit -F "<main>\tmp\<msg>.txt"`. Under `-C`, a relative `tmp/...` resolves
  inside the worktree.
- **Freezes.** A foundation lane can freeze a shared file (for example `named_syms.txt!` while pins
  are removed). `land` then refuses until the freeze is released. Prepare the next wave meanwhile,
  or stop.

## Shape of a session

- **Start:** `python tools/lanes/lanes.py open <lane> --kind naming`, then `lanes.py fresh <lane>`
  (land or discard any leftover commits first). Read `docs/naming/phase3/backlog.tsv`.
- **Work:** do at most **3 units**, each from the list below. A unit ends at a **stopping point**:
  `lanes.py land` succeeded (main gated and pushed), with the backlog row updated in the landed
  commits, and the unit's claims released (`lanes.py unclaim <lane> <tokens>`).
- **Stop:** after 3 units, or earlier if context passes ~300k or a unit fails twice. Before calling
  the session ended, every lane commit must be on main (`lanes.py status` shows `unlanded=0`; land
  or drop what is left), then run `lanes.py close <lane> --remove`. It removes the lane and landing
  worktrees through `tools/safe_remove_worktree.ps1` and deletes the branch (never use a raw
  `git worktree remove`). Report one line per unit, say plainly that the session is complete, and
  stop. Don't hand off and don't wait on anything. The next session starts from the backlog.
- **Pick order:** LAND first (oldest `reviewed` row no other lane claims), then PREPARE (first
  unclaimed `idea` row).

## Agents are bounded

- No long-lived teammates. Every subagent gets **one task**, returns, and is done.
- Run subagents in the **foreground** (the call blocks until it returns) so the session never idles
  waiting for a notification. Independent tasks can go as several Agent calls in one message.
- Models: `naming-reviewer` for every review; `general-purpose` with `model: sonnet` for drafting
  and mechanical applies; Codex (`tools/codex_worker.py research`) for research only.
- Codex is the one background job you leave running. Start it as the **last** step of a
  session, and record the backlog row as `researching`. The next session extracts the result.
- The one thing you do wait on is `lanes.py land`: run it with `run_in_background: true`, because
  it can outlast a tool timeout. It builds in a private landing worktree, so you may prepare the
  next unit in the lane worktree meanwhile. Commits made during a land are not included; they
  are rebased onto the new main afterwards and land next time. If land notes that the branch
  was left as is, nothing needs doing: the next land skips the already-landed commits.

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
   For functions, take the target list from `python tools/naming_noname.py free <tu,...>`: it
   skips every function in the NO_NAME ledger (`docs/naming/phase3/no_name.tsv`). Bring one back
   only with new evidence (a neighbour named since, a type landed, a Q124 trace capture):
   `--revive <addr>=<evidence>`. Put the printed `# revive` lines in the prompt and the row note.
2. `researching` or `researched`: `python tmp/codex/extract_final.py <run dir> tmp/codex/<id>-research.md`.
3. Sonnet drafter (foreground) writes `docs/naming/phase3/pending/<id>/`. For members this
   includes a compiled dry run, run from PowerShell inside the worktree:
   `cd "<wt>"; python tools/naming_member_apply.py --dry-run <pairs.txt>`.
   The drafter re-derives every use and drops anything it can't show.
4. `naming-reviewer` pre-apply (foreground).
   - Rows that FAIL are dropped.
   - A name the reviewer only *suggests* may replace a failed one, but it is marked
     `suggested` in the manifest, and R6 must judge it fresh at every use.
   - If no row survives, the unit ends as `dropped`.
5. Set VERDICT CONFIRM with a verifier note, commit the draft dir and the backlog row as
   `reviewed` in the lane worktree, and `lanes.py land <lane>` (docs-only, so no build gates).
   That is the stopping point.

## Unit LAND (reviewed -> landed)

1. A Sonnet applier (foreground) works in the lane worktree `<wt>` on `lanes/<lane>`, freshly
   reset to main. It runs the wave apply (naming-bar §The wave, step 4) and the comment pass, then
   the gates in the worktree:
   - `& tools/wteng.ps1 "<wt>" verify-oracle --rebuild`
   - `& tools/wteng.ps1 "<wt>" tus-check`
   - the integrity check (§Gate commands)
   - `& tools/wteng.ps1 "<wt>" test`
   - `naming_keycheck`, in WSL inside the worktree (§Gate commands)

   It commits the wave plus a `docs:` comment commit in `<wt>` and returns the hashes. It never
   touches main.
2. `naming-reviewer` R6 (foreground) on those commits: identifiers only, comments true, no stale
   old names in live docs and registries, and suggested names judged fresh.
3. In the lane worktree:
   - Apply R6's blocking fixes verbatim in a `docs:` commit.
   - For function waves, run `cd "<wt>" && python tools/naming_wavefollow.py <id> symbol_addrs.txt`.
     It is deterministic, so commit it on its own with the trailer
     `Lane-Replay: python3 tools/naming_wavefollow.py <id> symbol_addrs.txt`.
   - For function waves, record the research's NO_NAME rows in the ledger:
     `python tools/naming_noname.py add tmp/codex/<id>-research.md --tag <id> --commit <wave sha>`
     (it also prunes rows named since). Commit `docs/naming/phase3/no_name.tsv` with the row.
   - Set the backlog row to `landed` and commit.
   - Run `python tools/lanes/lanes.py land <lane>`. It rebuilds, regenerates the census, runs
     `test_label_census.py` and the integrity check, fast-forwards main and pushes. That is the
     stopping point.
4. If R6 fails on a name, revert that pair on the lane branch (or redo the apply without it) and
   run R6 again. On a second failure, mark the row `dropped`.

## Gate commands (from PowerShell)

- `& tools/wteng.ps1 "<wt>" verify-oracle --rebuild` (commit first, because it refuses dirty
  build inputs)
- WSL commands inside the worktree go through a script file in main's tmp/ (shell quoting is
  guarded). For example, `tmp/<lane>_gate.sh`:
  `cd '<wt as /mnt/c/...>' && . .venv/bin/activate && python3 tools/check_completion_integrity.py`.
  Use the same form for `python3 tools/naming_keycheck.py --from-manifests ... --pairs-file ...`.
  Run it with `wsl bash '<main as /mnt/c/...>/tmp/<lane>_gate.sh'`.
- Census conflicts don't happen: `land` takes main's census and regenerates it.

## Hard limits

- Never `make setup`, never recursive force deletes, and no attribution lines. Commit with `-F`.
  Python writes use `newline='\n'`.
- Quoting through shells is blocked by the footgun guard: put scripts in `tmp/*.py` and messages in
  `tmp/*.txt`.
- Owner rulings: don't revisit Kengo symbols; packed-RECT words in Sony code are idiom; one member
  name may be shared across types only when the code shows the same meaning and every pair lands
  in one wave.
