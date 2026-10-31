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
  It refuses a row another live lane holds; take the next one. PREPARE claims only the row. At
  LAND, before the applier starts, claim in one `lanes.py claim` call (it is atomic) every TU the
  wave edits, spelled as the TU id (`topic:tu:main/51268`: the path under `src/` without `.c`,
  taken from the `src/` paths in `dry.txt`'s edit list; for local pairs, the TU defining each
  function), plus `topic:type:<Type>` for member waves; then `lanes.py fresh <lane>`, so the
  apply and keycheck see every name another lane landed. `fresh` refuses while an earlier
  unit's commits are unlanded (its `land` still running): finish that land first; never
  `fresh --discard` here. If any claim is refused, unclaim what you took and do another unit or
  stop; never wait for it. After any row claim succeeds, `lanes.py fresh <lane>` and re-read the
  row on main: go on only if its stage and note are what you picked it by (the Codex lane may
  have just moved it). Claims are held until the unit's stopping point. Older row notes that
  say to claim TUs or types before PREPARE are superseded.
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
  `git worktree remove`). Remove the scratch you and your subagents made under main's `tmp/`
  (e.g. `tmp/p3w<N>`, review copies, unconsumed `*.done` / `*.tmp` markers in `tmp/<lane>-done/` and that folder;
  CLAUDE.md: verify, then delete: resolve the folder to an absolute path, list it, confirm it is your own scratch, then `rm -rf "<absolute path>"` (never chained after `cd`, never a bare glob or a possibly-empty variable; a folder holding a worktree or junction goes through `tools/safe_remove_worktree.ps1`); the research extracts in `tmp/codex/` stay). Report one line per unit, say plainly that the session is complete, and
  stop. Don't hand off and don't wait on anything. The next session starts from the backlog.
- **Nothing waits on Codex (owner 2026-10-27).** Codex is a separate parallel lane (its usage may
  run out), so a session researches with an Opus subagent (PREPARE step 1). The Codex lane
  researches ahead (owner 2026-10-28: use all of Codex's weekly usage): when it launches a run on an
  `idea` row it claims the row and lands it `researching` with the run id, then `researched`
  with `research: Codex <run id>` once the file is checked. A row it claims is
  skipped like any claimed row; never wait for it.
- **Tracks run in parallel, batches don't.** A track (locals, globals, members) has at most one
  open row (`idea` .. `reviewed`) at a time, and parallel lanes take different rows. Names can
  still meet across tracks (a local and a global in one TU), so two lanes never hold one
  `topic:tu` claim during a LAND (§Claims).
- **Pick order:** LAND first (oldest `reviewed` row that no other lane claims and whose TUs no
  other lane claims), then PREPARE (first unclaimed `researched` row, or `researching` row whose
  cited Codex run has finished, then `idea` row).

## Agents are bounded

- No long-lived teammates. Every subagent gets **one task**, returns, and is done.
- Run subagents in the **foreground** (the call blocks until it returns) so the session never idles
  waiting for a notification. Independent tasks can go as several Agent calls in one message.
  Never launch an Agent with `run_in_background`: a backgrounded applier once left a session
  idle 80 minutes after it had finished. A subagent whose own command can outlast a tool timeout
  backgrounds that command and waits for it itself.
- **Done markers (2026-10-28).** A fallback for when an Agent call returns without its result:
  in lane sessions the harness may run every Agent in the background anyway, and its completion
  notice sometimes never arrives (drafters and reviewers finished while sessions sat idle 25-80
  min). It does not replace the foreground rule.
  - Markers live in `<main>\tmp\<lane>-done\<task>.done`; the session creates that folder
    before its first Agent call (`New-Item -ItemType Directory -Force`). `<task>` is unique per
    Agent call (with the batch number, e.g. `r6-b2`; a re-run gets a new name, `r6-b2-2`).
    Delete any existing marker of that name first.
  - Every subagent brief ends with: "as your very last step, after all other files are written,
    write `<main>\tmp\<lane>-done\<task>.tmp` in one write, then rename it to `<task>.done` in
    that folder (absolute paths, quoted: they contain spaces), containing your complete final
    answer (a reviewer: the whole JSON verdict; others: the one-line result and the paths of
    your output files)". A reviewer has no Write tool: it uses PowerShell, `Set-Content
    -LiteralPath "<tmp>" -Value <the whole final answer>` then `Rename-Item -LiteralPath "<tmp>" -NewName <task>.done`.
  - If an Agent call returns its result, ignore the marker (it goes at cleanup). If it returns
    only a launch acknowledgement, wait with one `Bash` call per marker, `run_in_background:
    true` (one completion notice, no 30-min cap, as for `lanes.py land`), on the quoted bash path
    (`/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile/tmp/<lane>-done/<task>.done`):
    `for i in $(seq N); do [ -s "<marker>" ] && { echo FOUND; exit 0; }; sleep 15; done;
    echo TIMEOUT`, N = 180 (45 min; 360 for an applier). FOUND: read the marker (and the files
    it names), delete it, go on. TIMEOUT: check once more for the marker, then the subagent's
    files; re-run it under a new name only if they are incomplete (unchanged for 10 min).
- Models: `naming-reviewer` for every review; `general-purpose` with `model: sonnet` for drafting
  and mechanical applies; `general-purpose` with `model: opus` for research, never Sonnet
  (owner 2026-10-27).
- The only other thing you wait on, besides the done markers above, is `lanes.py land`: run it with `run_in_background: true`, because
  it can outlast a tool timeout. It builds in a private landing worktree, so you may prepare the
  next unit in the lane worktree meanwhile. Commits made during a land are not included; they
  are rebased onto the new main afterwards and land next time. If land notes that the branch
  was left as is, nothing needs doing: the next land skips the already-landed commits.

## Backlog (`docs/naming/phase3/backlog.tsv`)

The columns are `id  kind  stage  draft  note`. `kind` is `func`, `member`, `data` or `local`.
Each kind is one track: `local` = locals, `data` = globals, `member` = members, `func` = functions
(the second pass over the NO_NAME functions, `func-second-pass` rows; owner 2026-10-28). Kind
`audit` (stage `active`, the Codex lane's `name-audit` index) is never picked. Kind `file` =
file names (naming-bar N6), §File waves.
`draft` is the tracked draft dir `docs/naming/phase3/pending/<id>/` (rows.py, member_manifest.csv,
pairs.txt, dry.txt). Stages:

`idea` -> `researching` -> `researched` -> `reviewed` -> `landed` (or `dropped`, with the reason in `note`)

Commit a stage change with the files that justify it, for example `docs: naming backlog -- m09
reviewed`.

## File waves (kind `file`, naming-bar N6; owner 2026-10-28)

An address-named game TU takes a subsystem name only when every function's admitted name and
every datum it defines agree on that subsystem; a mixed file keeps its ROM-offset name.
- PREPARE: research lists, per candidate TU, every function (docs/naming/function-names.csv)
  and every symbol it defines (build/bb2.map of a fresh lane build), and proposes a name in the
  `cdrom.c` / `memcard.c` style, narrower when the subsystem spans several files
  (`cdrom_mix.c`). `tools/move_tu.py` refuses on a dirty tree, so run every dry run first, on
  the clean worktree, with output under `tmp/`; then write the draft: `pending/<id>/moves.tsv`
  (`old_id  new_id  evidence`, TU ids as in AGENTS.md, `main/<OFF>`) and `dry.txt` (the copied
  dry runs and, per moved TU, the git-grep hits below). `naming-reviewer` pre-apply judges each row against N6 (every function and datum
  agrees; the name claims nothing the code doesn't show; no clash with an existing file).
- LAND: claim, in one `lanes.py claim` call, `topic:tu:<old>` for every row and `topic:tu:<id>`
  for every other `src/` TU in `dry.txt`'s grep hits; `lanes.py fresh`; then per row: `python
  tools/move_tu.py <old> <new> --apply` and commit at once, rename only (`naming: <id> -- <old>
  -> <new>`, so git follows the file). Then one comment commit: `move_tu`'s leftover list
  misses the usual citation form, so also run `git grep -n -w -E "<OFF>(\.c)?" -- src include
  symbol_addrs.txt named_syms.txt .claude/rules` per moved TU and rewrite each live citation of
  the file to its new name (other files' boundary evidence included); leave ROM-offset
  mentions (`ROM 0x<OFF>`, asm byte comments) and `docs/` / `memory/` history as written; keep
  each moved TU's top comment and its own boundary evidence (`ROM 0x<OFF>` stays), but rewrite
  its citations of other moved files like any other. Handle `move_tu`'s own leftover list too.
  Then `python tools/format.py` on the C files you edited; if a file's line count changed,
  re-point every `<file>:<line>` citation into it (`git grep -n "<newname>.c:"` over the same
  paths). The row still moves `reviewed` -> `landed`; `naming_keycheck`, `naming_wavefollow`
  and `naming_noname add` do not apply. Gates once on the final tree
  (`verify-oracle --rebuild` SHA1 unchanged, `tus-check`, integrity, `engine test`), R6 on the
  wave's diff, land all its commits together.

## Unit PREPARE (idea or researched -> reviewed)

1. `idea`: write the research prompt (templates: `tmp/codex/prompt-research-m05.md` for members,
   `tmp/mkprompt.py` and `tmp/queued_in.py` for functions). Tell it which names are taken and which
   earlier rows failed, and to write its result to main's `tmp/codex/<id>-research.md` (in the
   shape `extract_final.py` gives a Codex run; this overrides any output file a template names)
   and return a one-line summary.
   **Wave size: up to 80 rows** (raised from 40 on 2026-10-27 under the owner's "fix these
   structural issues"; apply, R6 and land cost ~45 min whatever the size; older `max 40` caps in
   row notes are superseded), so ask for up to ~90 proposals. Run it
   through one `general-purpose` subagent with `model: opus` (foreground). Note `research: Opus`
   on the row and
   go on to step 3 in this unit; if the unit stops before step 5, land the row as `researched`.
   For functions, take the target list from `python tools/naming_noname.py free <tu,...>`: it
   skips every function in the NO_NAME ledger (`docs/naming/phase3/no_name.tsv`). Bring one back
   only with evidence specific to it and newer than its ledger `commit`: a callee or caller, or a
   global, member or type it touches, named since; or a newer Q124 capture or only-X row covering
   it. Cite `file:line`. Local or parameter names are never revive evidence (naming-bar N3); a
   landed name only points at code to re-read. `--revive <addr>=<evidence>`. Put the printed `# revive` lines in the prompt and the row note.
2. `researched` with `research: Opus` or `research: Codex <run id>`: the file is main's
   `tmp/codex/<id>-research.md` (it is untracked: if it is missing, extract it from the Codex run
   or redo step 1); go to step 3. A Codex row with no research file yet (`researching`, or
   `researched` citing a run id but no `tmp/codex/<id>-research.md`): `python tmp/codex/extract_final.py <run dir> tmp/codex/<id>-research.md`. If the run
   has no `meta.json` or ended other than `ok` (`failed` / `timeout` / `cancelled`), redo the
   research by step 1 (Opus), replacing the run id in the note with `research: Opus`, or mark the
   row `dropped` with the reason. A run still in progress: take another unit; if it is still running when its track is the
   only work left, redo it by step 1 (Opus) and note the abandoned run id.
3. Sonnet drafter (foreground) writes `docs/naming/phase3/pending/<id>/`. For members this
   includes a compiled dry run, run from PowerShell inside the worktree:
   `cd "<wt>"; python tools/naming_member_apply.py --dry-run <pairs.txt>`.
   The drafter re-derives every use and drops anything it can't show.
4. `naming-reviewer` pre-apply (foreground). Over 40 rows, split them into batches of about 40 by
   the file or type that defines each name, one fresh reviewer per batch, in parallel (several
   Agent calls in one message). Rows that share a proposed name (or name family) stay in one
   batch, even past 40. Give each reviewer the manifest path, its batch's row list (judge only
   those rows, every use wherever it is) and the whole manifest's proposed names (R5 across the
   wave). Each row's verdict is its batch reviewer's; paste every reviewer's JSON in the commit
   body.
   - Rows that FAIL are dropped.
   - A name the reviewer only *suggests* may replace a failed one, but it is marked
     `suggested` in the manifest, and R6 must judge it fresh at every use.
   - If no row survives, the unit ends as `dropped`; if the track still has targets, add its
     next `idea` row in the same commit.
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
   old names in live docs and registries, and suggested names judged fresh. Over 40 renames,
   split them across parallel fresh reviewers as in pre-apply, each with the full keycheck
   report and its own pairs' comments and scoped notes; one of them also owns the wave-level
   checks (pairs equal the CONFIRMed rows, moved layer-2 keys, survival grep for old scoped
   names, registry note lines). R6 is OK only when every reviewer returns `r6: OK`.
3. In the lane worktree:
   - Apply R6's blocking fixes verbatim, amended into the wave's `docs:` comment commit (one
     commit per purpose, owner 2026-10-25: no separate `R6 fixes` commit).
   - For function waves, run `cd "<wt>" && python tools/naming_wavefollow.py <id> symbol_addrs.txt`.
     It is deterministic, so commit it on its own with the trailer
     `Lane-Replay: python3 tools/naming_wavefollow.py <id> symbol_addrs.txt`.
   - For function waves, record the research's NO_NAME rows in the ledger:
     `python tools/naming_noname.py add tmp/codex/<id>-research.md --tag <research tag, e.g. w78>`
     (it also prunes rows named since; commits stay `-` until the wave is on main, and the next
     `add` or `prune` fills them). Commit `docs/naming/phase3/no_name.tsv` together with the
     backlog row set to `landed`, in one commit.
   - If the track still has unnamed targets and no open row (`idea` .. `reviewed`), add its next
     batch as an `idea` row in that same commit (`var-globals-c`, `var-members-rest-4`, ...): the
     note says what is left and what this wave left neutral on purpose.
   - Run `python tools/lanes/lanes.py land <lane>`. It rebuilds, regenerates the census, runs
     `test_label_census.py` and the integrity check, fast-forwards main and pushes. That is the
     stopping point.
4. If R6 fails on a name, revert that pair on the lane branch (or redo the apply without it) and
   run R6 again. On a second failure, mark the row `dropped` (and queue the track's next `idea`
   row, as in step 3).

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
