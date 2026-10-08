---
name: codex-worker
description: >-
  Work ONE Bushido Blade 2 issue end to end with an OpenAI Codex agent: usually a line item of
  tmp/codex/backlog.md. Codex implements it in its own scratch worktree; Claude harvests and
  verifies it, fresh default-FAIL adversarial reviewers must PASS, then the commit is rebased,
  conflicts resolved, fast-forwarded onto main under the reintegration lock, and pushed. Use when
  asked to have Codex solve, fix or work an item or issue, or to orchestrate Codex agents on this
  repo. Read-only Codex questions on this repo run through `codex_worker.py research` (a disposable snapshot worktree, never the main checkout).
---

# Codex worker: one issue, scratch worktree to pushed main

Invoked with an issue: a backlog line (`item 13`, `F06`), or a described problem. The run
**ends in exactly one of two states**:
- **landed**: one commit on `main`, pushed to `origin`, every required reviewer PASS on that exact
  diff, and a clean build of that exact commit byte-identical (SHA1 `62efab4f…`);
- **stopped with a reason**: blocked on an owner ruling, Codex quota, or a peer owning the area.
  Report the reason, and which gate is still open.

Everything runs through **`python tools/codex_worker.py`** (Windows `python`). It enforces the
gates, so never hand-roll their git steps. It builds on the user-level `codex` skill: quota
gate, one task at a time, and verifying everything Codex claims.

## The two worktrees, and why

| | `../bb2-worktrees/codex-<item>`: **scratch** | `../bb2-worktrees/codexv-<item>`: **trusted** |
|---|---|---|
| who writes it | Codex (its sandbox's only writable root), and you when you make a fix | only `codex_worker.py`, and you when you resolve a rebase conflict |
| what it is | untrusted **data**: detached, no toolchain | branch `codex/<item>`, toolchain junctions, private `build/` |
| what runs there | **nothing, ever** | every build, score, hook, test, commit, rebase and the landing |

A sandboxed agent can tamper with anything in its own tree: swap a junction, forge the `.git`
file, set index flags, plant ignored `.pyc` files or `build/` objects. So nothing in the scratch
tree is executed or trusted. Its content is **harvested**: read through main's own worktree
registry with a throwaway index, refused if it contains any symlink or junction, and turned into
a git tree. That tree is refused before anything checks it out if it contains any of:
- symlink or submodule entries;
- git metadata files (`.gitignore`, `.gitattributes`, …);
- paths main's `.gitignore` ignores, which is where the toolchain junctions, `build/` and
  `disc/` live;
- paths under a reparse point in the trusted tree.

Builds run on that tree, checked out into the trusted worktree. Scratch is only ever deleted
with `rmdir /s /q`, which doesn't follow junctions, and only once Codex is idle. The sandbox preflight
proves on every run that Codex can't write the trusted worktree, the main checkout or main's
`.git`.

## Hard rules

- Codex runs only via `codex_worker.py run` / `follow-up`. Never call `codex.exe`, or
  `codex_bridge.py run --sandbox workspace-write`, directly for this repo.
- Main changes only through `land`: a fast-forward to the verified commit. Never commit, merge,
  reset, stash, check out or rebase in the main checkout. The Phase 2 lanes commit through its
  shared index; a fast-forward keeps their staged, unstaged and untracked work on other paths,
  and refuses on a shared path.
- Git writes for the item go through `codex_worker.py` (`commit`, `rebase`). The main-lock hook
  blocks `git -C <worktree>` writes anyway.
- Never touch other worktrees or branches (`git worktree list`: `p2-w2`, `rev-*`, older
  `codex-*`). Don't remove, reset, rebase, build or check them out.
- Never force-push, steal a *fresh* reintegration lock, skip hooks, or reuse a reviewer that
  already ruled on the item. Each review is a new agent.

## Pipeline

**0. Scope the issue.** Read the backlog line and its ground rules (the reserved files in
particular). Stop if any of these hold:
- it's marked taken, on hold, or owner-blocked;
- another lane owns the area (check `git worktree list` and `git branch -a` for a live branch on
  the same files);
- it needs a new ruling. Log it per `docs/grind/borderline.md` and ask the owner in plain language.

If it completes a queue function (a `Match:` commit with layer-2 and `queue done`), hand it to
`decomp-manual`; that landing path is different. Slug: kebab-case, `<= 48` chars.

**1. Quota.** `python ~/.claude/skills/codex/codex_bridge.py status`. If it says BLOCKED, stop.

**2. Prompt.** Write `tmp/codex/prompt-<item>.md` with the Write tool, `<=` ~4000 chars. The
script adds the project rules and how to build. Make it self-contained:
- the goal, the exact files and functions, and the governing rule or ruling (`path:line`);
- the files it may edit. Everything else is off-limits, including the backlog's reserved files;
- what "done" means: `make` matches the SHA1, `sandbox <f> --disable all` == 0, `FAKE` labels in
  the rule's form;
- the report you want back.

Point at rules, never at another worker's diff.

**3. Run** (background, 2–30 min):
`python tools/codex_worker.py run <item> --prompt-file tmp/codex/prompt-<item>.md [--effort low]`.
It prepares both worktrees, runs the sandbox preflight, and pins the run, so Codex's build door
works only while the run is live. Afterwards it audits by harvesting the change and scanning
for junctions. Higher effort than medium needs the owner.

**4. Verify, then iterate.**
- Read the audit, `<scratch>/tmp/codex/report-<item>.md` and the harvested diff
  (`codex_worker.py audit <item>`).
- Re-score it yourself: `python tools/codex_worker.py shadow <item> make` / `… sandbox <f> --disable all`.
  This uses the same harvest-and-build-in-trusted path as Codex. Codex's numbers are claims.
- Wrong or unfinished: `codex_worker.py follow-up <item> --prompt-file <specific findings>`.
- Still stuck after a few follow-ups: fix it yourself by editing the **scratch** files. Run
  `audit <item>` first; it refuses if there's a junction you'd write through. That's a change of
  method, not of target.
  - The worktree-contamination guard treats any path that isn't `bb2-work-*` as main and blocks
    the Edit tool on build inputs. So write the edit as a small script in **main's** `tmp/` that
    asserts the target is the scratch path and not a link, then run it with `python -I`, never
    from inside scratch.
  - Measure each codegen-shaping construct you keep or add. Score the natural form with
    `shadow <item> sandbox <f> --disable all --candidate tmp/<variant>.c` (the variant file goes
    in scratch `tmp/`). The scores go in its `FAKE` label and the commit body.
- `shadow` refuses changes outside `src/`, `include/` and `asm/`, because the build would execute
  them. For such items (`engine/`, `tools/`, `Makefile`), the clean build and `engine test` in
  `land` (step 7) are the first execution, and they run only after review.

**5. Commit.** Write `tmp/codex/msg_<item>.txt` (Write tool) per `docs/COMMIT_CONVENTIONS.md`, and
include the Codex run id in the body. Then:
`python tools/codex_worker.py commit <item> --message-file tmp/codex/msg_<item>.txt`.
- It harvests the scratch tree into **the** item commit; re-running it replaces the commit.
- Its parent is the item's base, and it contains no `tmp/`.
- Changed C files are formatted (`tools/format.py`, token-preserving); when that changes anything,
  scratch is re-created at the commit.
- No hooks run yet, because the content is unreviewed; `land` runs them after review.
- It prints the review key and the reviewers required:
  - `cheat-reviewer` for `src/`, `include/`, `asm/`, rules, grants, symbols, `.ld` files and the
    `*_funcs.txt` / `*_syms.txt` / `*_files.txt` pipeline lists;
  - **both** reviewers for `Makefile`, `engine/` and `tools/` (the build pipeline can rewrite
    output);
  - `code-reviewer` for everything else.

**6. Adversarial review.** Spawn every required reviewer fresh and in parallel, and wait for all of
them. A split verdict is a FAIL.
- **cheat-reviewer** (`subagent_type: cheat-reviewer`): default FAIL, and your verdict counts for
  nothing. Give it `git show codex/<item>`, `.claude/rules/completion-bar.md` items 2–5, the landed
  `rules:` commit for any ruling the diff relies on, and the `shadow` build and score. It checks
  every `FAKE` label is true and measured, and every name and comment is true.
- **code-reviewer** (`subagent_type: general-purpose`): default FAIL. Brief it to hunt for real
  defects in the diff: correctness, regressions, missing or weak tests, Windows/WSL and
  line-ending traps, concurrency with other sessions, guard or hook bypasses, and the doc budget
  (CLAUDE.md). PASS only if it finds no blocking defect.

Record each verdict:
`python tools/codex_worker.py review <item> --kind <cheat-reviewer|code-reviewer> --verdict <PASS|FAIL> --agent <id> --summary "<one line>"`.
- The key is the commit's diff, whitespace-exact, with 3 lines of context, modes and binary
  content included. Only line numbers are dropped, so a clean rebase keeps it unless main
  touched the surrounding lines. A hand-resolved conflict invalidates every earlier verdict.
- Records live in main's `tmp/`, where Codex can't write.

On any FAIL: fix it (by Codex follow-up or by editing scratch), `commit` again, and run new
reviewers on the new key. Repeat until every required kind PASSes on the same key. A FAIL that
needs an owner ruling stops at step 0's rule.

**7. Land.** `python tools/codex_worker.py land <item>` (use `--dry-run` first if you like). It
refuses while any Codex run is live, and refuses unless Codex is idle before it rebases. Then, in the trusted tree:
1. Rebases onto the current main if main moved, and re-creates scratch at the result.
2. Re-checks the review gate.
3. Re-commits the same tree, so the repo hooks run on the reviewed content.
4. Runs `make clean-check` and computes `build/bb2.exe`'s SHA1 itself.
5. Runs `check_completion_integrity.py`, and `engine test` if code paths changed.
6. Takes the reintegration lock, unless this session already holds it.
7. Refuses if a peer has uncommitted work on the same paths in the main checkout.
8. Runs `merge --ff-only`, checks main == the verified commit, and pushes `origin main`.

If main moves mid-way it retries. The first push also publishes every unpushed commit on main;
the count is printed. Exit codes:

| exit | meaning | do |
|---|---|---|
| 6 | rebase conflict | `codex_worker.py rebase <item>` leaves the conflict in the **trusted** worktree. Edit the files there (Edit tool), then `rebase <item> --continue`, which refuses while conflict markers remain and re-creates scratch afterwards. Land again; the gate says whether the resolved diff needs new reviews. |
| 5 | lock held by a live session | wait (~10 min) and land again. Steal only a lock `reintegrate_lock.ps1 status` calls stale. |
| 7 | a peer has uncommitted work on the same paths in the main checkout | wait until they commit or drop it (or message that session), then land again. If their commit moved main, `land` rebases onto it. |
| 9 | review gate | step 6 for the missing or failed kind. |
| 8 | push rejected | main is landed locally. Never force: `git fetch`, then report to the owner. `land` again retries only the push. |

**8. Close.** `python tools/codex_worker.py cleanup <item>` removes both worktrees (junction-safe)
and the branch, and refuses unless the branch is on main. `--drop` discards an unlanded item, and
only with the owner's OK. Then add the landed hash to the item's Status line in
`tmp/codex/backlog.md`. Report: the item, run ids, Codex usage before→after, each reviewer and
verdict, the build SHA1, the landed hash, and the push result.

## Read-only research: `codex_worker.py research`

Questions, inventories and second opinions run as
`python tools/codex_worker.py research <label> --prompt-file f.md [--effort low] [--timeout N]`: a read-only Codex task in a
disposable detached worktree at main's HEAD (`../bb2-worktrees/codexr-<label>`, tracked files only: no
`build/`, `tmp/` or toolchain); on exit the processes working in it are stopped and it is removed. Never point Codex at the main checkout, not even
read-only: Claude edits, commits and lands there while it reads.

Codex never blocks Claude's own mainline commits. The steps that delete or re-create an item's scratch tree
(`land`'s and `rebase`'s rebases, `rebase --continue`, `layer2`, `cleanup`, and `commit` when formatting
changed the tree) **refuse** (exit 2) unless no Codex task is running: a Codex process's writable reach cannot
be bounded from outside its sandbox. Each check runs before anything moves or is removed, so a refusal there leaves
nothing half-done; if a task starts mid-step, a later check refuses and re-running the step finishes it. `land`, `rebase`, `commit`, `layer2`
and `cleanup` also refuse while any item's run is live (the pin is global).

The goal (owner 2026-10-07) is that Codex never works in the main working tree; containing a hostile
agent is not. `run` / `follow-up` start only while no other Codex task is running, and warn when
`~/.codex/rules/` holds rules besides the build door (an allow rule may run a command outside the sandbox).

## Codex's build door: `tools/codex_eng.ps1`

`run` installs `~/.codex/rules/bb2-codex-eng.rules`, which lets exactly
`pwsh -NoProfile -File "<main>/tools/codex_eng.ps1" <cmd>` run outside the sandbox, one command
per shell call. That script is main's copy, which Codex can't edit, and it calls
`codex_worker.py shadow --from-pin`. That path:
- works only while the pinned run's process lives;
- harvests the scratch tree, and refuses any change outside `src/`, `include/` and `asm/`;
- builds in the trusted tree with a scrubbed environment;
- accepts only `make` with no arguments, or a read-only engine subcommand with plain relative
  arguments. `--candidate` files are copied from scratch `tmp/`, and must be plain files.

**The sandbox preflight is load-bearing.** `tools/wsl_bridge.ps1`'s daemon runs any request file
in `%TEMP%/bb2_wsl_bridge` in WSL as the user, and Codex's sandbox may write `%TEMP%`. Every
`run` and `follow-up` therefore:
- strips Codex's sandbox SIDs from that folder's ACL;
- proves from inside a real Codex sandbox that it can write none of: the bridge folder, main's
  `tmp/` or `.git`, the trusted worktree or its `build/`, or the worktrees root;
- proves the sandbox can't start WSL itself.

Never relax it.

## Worktree hazard

The trusted worktree junctions main's `.venv`, `tools/gcc-2.7.2`, `decomp-permuter` and `disc`.
**Never `git worktree remove --force`**: it follows the junctions and deletes main's toolchain.
`cleanup` uses `tools/safe_remove_worktree.ps1` for both trees. If a tampered scratch `.git`
defeats that, cleanup deletes the directory only after confirming no reparse point remains.
