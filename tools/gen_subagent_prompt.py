#!/usr/bin/env python3
"""Generate the canonical subagent prompt for working ONE function to
byte match. Used by autonomous-mode loops where the main agent spawns
a fresh subagent per function (keeping its own context small).

Two modes:
  default        -- sequential single-agent. Subagent works in the main
                   repo, sets `.bb2_active_func` via `dc.sh next` (the
                   coordinator already did that -- this prompt assumes
                   the marker is set), commits to main on match.

  --worktree     -- parallel orchestration. Subagent runs in a worktree
                   created by the Agent tool's isolation="worktree".
                   First action: bash tools/worktree_setup.sh (symlinks
                   gcc/.venv/disc/decomp-permuter/m2c into the worktree).
                   Then writes its own .bb2_active_func, matches, and
                   commits to its branch -- NEVER to main. The
                   coordinator integrates the branch via
                   `dc.sh integrate <branch>` after the worker returns.

Usage:
    python3 tools/gen_subagent_prompt.py <func>
    python3 tools/gen_subagent_prompt.py <func> --worktree
    bash tools/dc.sh subagent-prompt <func>            # default
    bash tools/dc.sh subagent-prompt <func> --worktree # worktree mode

Output goes to stdout. The main agent reads it and passes the text to
the Agent() tool's `prompt` parameter.
"""
from __future__ import annotations

import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent


PROMPT_TEMPLATE = """You are working {func} to byte-match in 100% pure C.

# Active function marker
The harness has set `.bb2_active_func` to `{func}`. The PreToolUse hook
will block (until you match):
  - `git commit` (unless `dc.sh verify {func}` reports MATCH)
  - `git checkout/restore/reset --` on src/*.c, regfix.txt, asmfix.txt,
    undefined_syms_auto.txt, named_syms.txt, sdata*.txt, expand_lb_funcs.txt
  - `dc.sh next` (you don't pull a new function -- the parent does that)

You CANNOT run `dc.sh release` -- that's the user's escape hatch only.
If you're genuinely stuck after exhausting the toolbox, RETURN with a
detailed status; the parent will coordinate with the user.

# THE HARD RULE
Once you start, you finish. No tabling, no skipping, no inline-asm
workarounds. Stuck = switch *technique*, not target.

The only valid stop conditions:
1. {func} matches -- `dc.sh verify {func}` reports MATCH AND `make`
   reports `OK: bb2 matches!` AND you've committed.
2. Genuine intractable blocker -- you've tried >=5 distinct techniques
   from the escalation ladder, including at least one assembly-stream
   regfix attempt, and you can articulate what specifically prevents
   matching with the current toolchain. Then RETURN with details so
   the user can decide.

# Workflow

1. **Get context** (do this first, every time):
   ```
   bash tools/dc.sh agent-brief {func}
   ```
   This shows: classification (watch for `aliasing_heavy` blocker_tag --
   means high deref-chain count, expect more iterations than insn count
   suggests), source location, target asm, m2c output, gen_regfix
   suggestions, sibling matches, Kengo reference.

2. **Run the auto pipeline:**
   ```
   bash tools/dc.sh attempt {func}
   ```
   - MATCHED -> integrate via `dc.sh inline-replace` (or write the body
     directly via WSL python3), build, verify, commit. DONE.
   - NEAR_MISS or HARD -> continue below.

3. **Check named recipes:**
   ```
   bash tools/dc.sh recipes {func}
   bash tools/dc.sh near-miss {func}
   ```

4. **Read the penalty list and route to technique** (in
   feedback_matching_playbook.md, section "Penalty-list -> technique
   routing"). Common patterns:
   - 0 ins/del/reord, only Reg differences -> swap rules in regfix.txt.
     DO NOT run gen-regfix on this profile (produces noise). DO NOT
     run permuter (nothing structural to find).
   - Ins >= 1 (target has more) -> don't cache the inner deref; let GCC
     reload naturally. Cache outer pointers but read inner each access.
   - Reord high -> C-level scheduling techniques OR regfix
     fill_delay/drain_delay/reorder.
   - LICM hoist signature -> regfix unhoist recipe.

5. **For build-context iteration**, use `bash tools/dc.sh diff {func}`
   (side-by-side target vs build pipeline output, normalized for
   register aliases / hex / opcode aliases). Spot-check this BEFORE
   reaching for permuter or gen-regfix.

6. **Permuter cadence:**
   - First manual run: 90-120s, NOT 600s.
   - If first run plateaus AND diff still has ins/del: 5-10 min.
   - Long overnight runs only for 3+ ins/del with no recipe match.
   - DO NOT run permuter when penalty list shows only register
     differences.

7. **Integration discipline** (when you have matching C):
   - Add missing externs to source file's extern block (sibling-file
     audit first -- sibling may already have correct signed/unsigned
     declaration).
   - Replace stub with just signature + body. Never use `dc.sh replace`
     for final integration (copies scaffolding); use
     `dc.sh inline-replace` or write directly via WSL python3.
   - Build: `rm -f build/src/<file>.o && make 2>&1 | tail -5`.
   - Verify: `bash tools/dc.sh verify {func}` -- must say MATCH.

8. **Validation IMMEDIATELY after match** (before considering done):
   - `make validate` -- catches regfix pattern drift in siblings.
   - `bash tools/dc.sh verify --all` -- catches label-renumber regressions.
   - If a sibling broke (1-byte diff in beq/bne), run
     `bash tools/dc.sh fix-label-drift` (drives off the linker error).

9. **Commit** (the hook auto-clears the active marker on success):
   ```
   git commit -m "Match {func} -- <one-line recipe summary>"
   ```

10. **Post-match retrospective** (mandatory but brief):

    Before returning, take 1-2 minutes to ask: was anything in this
    match worth retaining for future agents? Document conservatively
    -- only if you can articulate ALL THREE of:
      a) The specific technique, gotcha, or pattern.
      b) >=1 OTHER function where it applies (current sibling pattern,
         or a hypothetical with the same shape).
      c) What trips up the obvious approach (i.e., why a future agent
         would otherwise re-derive this from scratch).

    If you can articulate all three, update the appropriate file:

    - **New C technique** -> add a short subsection to
      `~/.claude/projects/.../memory/feedback_matching_playbook.md`
      (the "C-side techniques" section). One paragraph: name the
      technique, give the C snippet, say when to apply, name the
      function(s) it worked on.
    - **New regfix gotcha** -> `feedback_regfix_reference.md` (one-
      line addition to the gotcha list).
    - **New diagnostic signal** -> `feedback_matching_playbook.md`
      Diagnostics table or Penalty-list routing table.
    - **A novel multi-step regfix recipe** that's likely to repeat ->
      `dc.sh capture-recipe HEAD --write` to draft a JSON in
      `tools/recipes/`. Then human-edit the JSON to clean it up.
    - **A toolchain quirk that fails the obvious approach** -> add to
      `feedback_matching_playbook.md` "Things that DON'T work" so
      future agents skip the dead-end.

    If a recurring pattern emerged that needs a NEW TOOL (e.g., a
    third or fourth function with this exact shape), build it:
    - New `dc.sh` subcommand backed by a `tools/X.py`.
    - New regfix recipe JSON.
    - New pipeline-pass extension.
    - Wire it into the workflow doc so future agents find it.

    Commit memory/tool updates as a SEPARATE commit (hook only
    enforces the function-match commit; subsequent commits go
    through). Commit message: `Post-match retro for {func} --
    document <thing>`.

    **Anti-noise:** if the match was a known recipe (call-loop,
    LICM unhoist, early-exit alias, varargs, nested-bool, CU-split,
    plain register cycle) with no twist, the audit produces NOTHING.
    That's the right answer for routine matches.

# Reading

Mandatory:
- feedback_workflow_rules.md -- escalation ladder, integration
- feedback_matching_playbook.md -- toolbox order, every C technique,
  named recipes, penalty-list -> technique routing, things that don't
  work
- feedback_regfix_reference.md -- regfix.txt syntax + every gotcha

# Communication

DO NOT ask the user for direction. DO NOT report intermediate progress
in prose. Work silently. Return when done (matched + committed) or
genuinely stuck.

Final return format (one of):

  MATCHED -- `{func}` at commit <sha>. Recipe: <one-line summary>.
  Retro: NONE.

  MATCHED -- `{func}` at commit <sha>. Recipe: <one-line summary>.
  Retro: <commit-sha-of-retro-commit>. Updated:
    - <file>: <one-line description of addition>
    - tools/<new>.py: <what it does> (wired as `dc.sh <cmd>`)

  STUCK -- `{func}` exhausted toolbox. Tried: <list of techniques>.
  Best score: <score> with <ins/del/reord/reg breakdown>. Specific
  remaining diff: <description>. Suggested next move: <new tool /
  user release / etc>.

The retrospective summary is what the parent (and the user) reads to
know whether new institutional knowledge was captured. Be honest:
"Retro: NONE" is the correct answer for routine matches. Future
agents lose if you noise up memory with non-novel updates.
"""


WORKTREE_PROMPT_TEMPLATE = """You are working {func} to byte-match in 100% pure C.

You are running in a **git worktree** spawned by parallel orchestration.
The coordinator (parent agent) has assigned you ONE function. You work
it end-to-end, commit to YOUR branch, and return -- the coordinator
integrates your branch into main.

# CRITICAL: First four actions, in order

1. **Set up the worktree's toolchain** (gitignored dirs are missing):
   ```
   bash tools/worktree_setup.sh
   bash tools/agent_log.sh setup-done
   ```
   This symlinks gcc-2.7.2/, decomp-permuter/, m2c/, .venv/, disc/ from
   the main worktree. WITHOUT THIS the maspsx step fails silently and
   every .o is a 788-byte empty stub -- you'll get bizarre, unmatchable
   diffs. Verify by checking `ls -la build/src/*.o` after first build:
   real objects are >>788 bytes.

2. **Confirm you are NOT on main:**
   ```
   git symbolic-ref --short HEAD
   ```
   This MUST print something other than `main`. If it prints `main`,
   STOP and report -- the worktree wasn't set up correctly.

3. **Claim the function** (sets the marker so the hook enforces you):
   ```
   echo "{func}" > .bb2_active_func
   ```
   The hook in this worktree's tools/hooks/active_func_guard.sh detects
   the worktree's own marker (it auto-detects the project root from cwd).
   Once set, the hook will block `git commit` until `dc.sh verify {func}`
   reports MATCH, then auto-clear the marker on a successful commit.

4. **Sanity-check: is the function already pure C?** (queue staleness guard)
   ```
   bash tools/dc.sh classify {func}
   ```
   If the classifier reports `src=...c_function` (already pure C in main),
   STOP IMMEDIATELY:
   - Run: `bash tools/agent_log.sh already-matched`
   - Return: `MATCHED branch=<branch> sha=<HEAD-sha> recipe=already-pure-C-no-op`
   - Do NOT make a no-op commit. Do not waste tokens.
   - The queue can have stale entries; this catches them in seconds.

   If the classifier reports inline_asm / INCLUDE_ASM / replace_with_asmfile,
   continue with the workflow below.

# Progress logging (MANDATORY -- enforce heartbeat)

Log major transitions via `bash tools/agent_log.sh <event> [details]`.
The coordinator monitors via `dc.sh agent-status`. **Audit found
average ~7 events per worker -- workers were under-logging.** Fix that.

Required events (NOT every shell call -- but EVERY transition):
- After step 1: `bash tools/agent_log.sh setup-done`
- After classify (step 4): `bash tools/agent_log.sh classified <category> size=<N>`
- After `dc.sh attempt`: `bash tools/agent_log.sh attempt <RESULT> score=<N>`
- After each `dc.sh diff`: `bash tools/agent_log.sh diff ins=<N> del=<N> reord=<N> reg=<N>`
- Each regfix add: `bash tools/agent_log.sh regfix-add <op> @<idx>`
- Long permuter run: `bash tools/agent_log.sh permuter-start budget=<N>s` /
  `bash tools/agent_log.sh permuter-done best=<score> elapsed=<N>s`
- Each `make`: `bash tools/agent_log.sh build OK|MISMATCH`
- Each `dc.sh verify`: `bash tools/agent_log.sh verify MATCH|diff=<N>`
- On match: `bash tools/agent_log.sh matched recipe="<one-line>"`
- On stuck: `bash tools/agent_log.sh stuck reason="<why>" best=<score>`

**HEARTBEAT RULE: log at least one event every 5 min.** Even
mid-permuter, log a `progress` event with current best-score so the
coordinator can see you're alive. If you've been working on something
for 5+ min without logging, log a `progress` event NOW with what
you're doing.

# Boundaries (parallel-safe)

You MAY:
- Edit src/<your-function's-file>.c, permuter/, regfix.txt,
  regfix_stage2.txt, asmfix.txt, sdata_*.txt, expand_lb_funcs.txt,
  named_syms.txt -- all are per-worktree files
- Build, verify, run smart_match / permuter / gen-regfix
- Commit to your branch as many times as you want
- Run `dc.sh capture-recipe HEAD --write` for novel patterns

You MUST NOT:
- Run `dc.sh next` (the coordinator pulled from queue, not you)
- Run `dc.sh refresh-queue` (the coordinator does this after integration)
- Run `dc.sh release` (user-only escape hatch)
- Run `git checkout main`, `git push`, or anything that touches the
  main branch
- Modify scaffolding files (bb2.ld, Makefile, splat.yaml,
  undefined_syms_auto.txt) -- the coordinator owns these

# THE HARD RULE
Once you start, you finish. No tabling, no skipping, no inline-asm
workarounds. Stuck = switch *technique*, not target.

The only valid stop conditions:
1. {func} matches -- `dc.sh verify {func}` reports MATCH AND `make`
   reports `OK: bb2 matches!` AND you've committed.
2. Genuine intractable blocker -- you've tried >=5 distinct techniques
   from the escalation ladder, including at least one assembly-stream
   regfix attempt, and you can articulate what specifically prevents
   matching with the current toolchain.
3. Already-matched per the step-4 sanity check -- return immediately,
   no commit, recipe=already-pure-C-no-op.

# Cluster-pause check (audit-driven)

If `dc.sh diff {func}` reveals a cluster pattern that other STUCK
functions share (target uses `multu` where GCC emits `mult`; target
has interleaved nops between mflo where GCC fills delay slots; target
has hand-tuned register cycles GCC won't reproduce), STOP early and
return STUCK with the cluster-tooling-needed flag:

  STUCK branch=<b> tried=<list> cluster=<name> tooling=<specific>

The coordinator will pause spawning to have ONE worker build the
shared fix. Burning 60+ min independently re-discovering the same
toolchain gap wastes tokens (audit found 4 workers did this for the
multu/mult cluster, ~150 min total before the maspsx flag was built).

# Workflow (after the four setup actions, only if step 4 said the function isn't already pure C)

1. **Get context:** `bash tools/dc.sh agent-brief {func}`
2. **Run the auto pipeline:** `bash tools/dc.sh attempt {func}`
   - MATCHED -> integrate (`dc.sh inline-replace` or write directly),
     build, verify, commit, return.
   - NEAR_MISS / HARD -> continue.
3. **Check named recipes:** `bash tools/dc.sh recipes {func}` /
   `bash tools/dc.sh near-miss {func}`.
4. **Penalty-list routing** (feedback_matching_playbook.md):
   - 0 ins/del/reord, only Reg diffs -> swap rules in regfix.txt
     (skip gen-regfix and permuter -- both produce noise here).
   - Ins>=1 -> don't cache inner deref; let GCC reload naturally.
   - Reord high -> C scheduling OR regfix fill_delay/drain_delay/reorder.
   - LICM hoist signature -> regfix unhoist recipe.
5. **Diff iteration:** `bash tools/dc.sh diff {func}` (side-by-side).

6. **Permuter discipline (HARD CAP -- audit-driven).**
   First run: 90-120s. Log `permuter-start budget=120s`.
   After first run completes, log `permuter-done best=<score>`.

   **STOP RULE.** If best score did NOT drop by >=20% after first run
   AND the diff has 0 ins/del/reord (only Reg diffs), do NOT run another
   permuter. Switch to regfix swap rules instead. Permuter chasing is
   the #1 token sink in audit data (one prior worker did 7 sequential
   permuter starts in 30 min, no breakthrough).

   You may run a SECOND permuter (5-10 min) ONLY if:
   - First run dropped score by >=20%, AND
   - diff still has ins>=1 or del>=1.

   You may run a THIRD permuter (30 min) ONLY if both prior runs
   showed monotonic progress AND diff still has ins>=2.

   Beyond 3 permuter runs is forbidden unless a sibling-cluster fix
   (new tool / pipeline pass) has just changed the search space.

7. **Integration:** add missing externs (sibling-file audit first),
   replace stub with signature + body, `rm -f build/src/<file>.o &&
   make 2>&1 | tail -5`, then `bash tools/dc.sh verify {func}`.
8. **Validation:** `make validate` and `bash tools/dc.sh verify --all`.
   If sibling broke: `bash tools/dc.sh fix-label-drift --apply`.
9. **Commit** (hook auto-clears marker on success):
   ```
   git commit -m "Match {func} -- <one-line recipe summary>"
   ```

# Post-match retrospective (mandatory but brief)

Same rules as solo mode: only document if you can articulate (a) the
specific technique, (b) >=1 other function where it applies, (c) what
trips up the obvious approach. Most matches need NO retro. Routine
recipe matches (call-loop, LICM unhoist, plain register cycle, etc.)
return "Retro: NONE."

If you DO add memory/tool updates, commit them on top of the match
commit -- they'll come along when the coordinator integrates your
branch.

# Reading

Mandatory:
- feedback_workflow_rules.md -- escalation ladder, integration
- feedback_matching_playbook.md -- toolbox order, techniques, recipes
- feedback_regfix_reference.md -- regfix.txt syntax + gotchas
- feedback_parallel_orchestration.md -- this orchestration model

# Communication

DO NOT ask the user for direction. DO NOT report intermediate progress.
Work silently. Return ONE line in one of these formats:

  MATCHED branch=<branch> sha=<commit-sha> recipe=<one-line summary>
  MATCHED branch=<branch> sha=<commit-sha> recipe=<summary> retro=<sha>
  MATCHED branch=<branch> sha=<HEAD-sha> recipe=already-pure-C-no-op
  STUCK branch=<branch> tried=<technique-list> best_score=<n>
        diff=<ins/del/reord/reg breakdown> remaining=<description>

Branch name: `git symbolic-ref --short HEAD` (you're on it). Commit
sha: `git rev-parse HEAD`. The coordinator parses this single line to
decide whether to call `dc.sh integrate <branch>` or to surface the
stuck state to the user.
"""


DEEP_RETRY_PREAMBLE = """# DEEP RETRY MODE -- port-first protocol (audit-driven)

A prior worker on branch `{prior_branch}` matched {func} but
integration to main hit a blocker (label drift, conflict, or off-by-N).
**Do NOT redo their search.** Audit found that deep retries that
re-ran permuter wasted 400-600K tokens; ones that ported prior work
verbatim matched in 5-15 min.

# Quick-port path (try this FIRST):

1. Setup as usual (worktree_setup.sh, claim marker, etc.).
2. **Port prior C body verbatim:**
   ```
   git show {prior_branch} -- src/<file>.c | grep -A 200 '{func}'
   # Copy the function body directly into your worktree's src/<file>.c
   ```
3. **Port prior regfix rules verbatim:**
   ```
   git show {prior_branch} -- regfix.txt | grep -B1 -A 5 '{func}'
   # Append to your worktree's regfix.txt (avoid arithmetic .L<N>+/-N
   # rules; use insert_label anchors for siblings if drift is a risk)
   ```
4. **Build, verify, commit.** If it works: MATCHED.
5. If it doesn't (off-by-N branch offset, etc.):
   - Run `dc.sh fix-label-drift --apply` once.
   - Run `dc.sh diff {func}` -- read the SPECIFIC remaining diff.
   - Adjust ONE rule (label, idx, or rule type) to fix it.
   - Build, verify, commit.
6. **Only if quick-port doesn't get within 5 instructions** should you
   redo your own permuter search. Most stuck integrations are
   1-instruction off — fix that one rule, don't restart.

If the prior branch was deleted (only memory of recipe remains), reuse
the recipe pattern from the recipe summary in your prompt's worker
report (typically embedded in the spawning prompt).

"""


def main() -> int:
    args = sys.argv[1:]
    worktree = "--worktree" in args
    deep_retry = "--deep-retry" in args
    prior_branch = ""
    # Parse --prior-branch=<name>
    new_args = []
    for a in args:
        if a == "--worktree" or a == "--deep-retry":
            continue
        if a.startswith("--prior-branch="):
            prior_branch = a[len("--prior-branch="):]
            continue
        new_args.append(a)
    args = new_args
    if len(args) != 1:
        print("Usage: gen_subagent_prompt.py <func> [--worktree] [--deep-retry --prior-branch=<branch>]", file=sys.stderr)
        return 1
    func = args[0]
    body = (WORKTREE_PROMPT_TEMPLATE if worktree else PROMPT_TEMPLATE).format(func=func)
    if deep_retry:
        if not prior_branch:
            prior_branch = f"worktree-agent-<id>"  # placeholder
        preamble = DEEP_RETRY_PREAMBLE.format(func=func, prior_branch=prior_branch)
        body = preamble + body
    print(body)
    return 0


if __name__ == "__main__":
    sys.exit(main())
