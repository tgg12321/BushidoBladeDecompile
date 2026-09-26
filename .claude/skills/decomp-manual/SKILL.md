---
name: decomp-manual
description: >-
  Drive ONE Bushido Blade 2 function by hand, on main, with the Grinder's full
  context already loaded — the hand-driven lane for the hard tail. Stops the
  Grinder, pops the queue top (or a named function), assembles the complete
  dossier, then works it with free choice of approach and sustained context,
  landing it through the mandatory layer-2 adversarial review. Use when asked to
  work a function manually / by hand, to take over from the Grinder, to close a
  plateaued or stalled function, or to clear an operator-only blocker the
  pipeline cannot touch.
---

# BB2 decomp — the manual lane

You are the hand on the wheel for ONE function. The Grinder
(`decomp-grind`) is still the default autonomous pipeline; this lane exists for
the work it is structurally bad at.

**Why this lane exists** (measured 2026-09-21): 19 functions taking ≥20 grinder
sessions consumed **53% of all grinder session spend for 7% of its completions**,
and the remaining queue is now *all* that population — 93 of 94 active items sit
at distance ≥250. Cold restarts re-derive what is already on disk. A sustained
hand-driven session closed `func_800747D8` after 10 grinder sessions and
`sys_VSync` after 7 cold-start workers thrashed. Design:
`docs/superpowers/specs/2026-09-21-decomp-manual-design.md`.

---

## 0. Handshake — always start here

```powershell
pwsh tools/manual_session.ps1 begin              # queue top
pwsh tools/manual_session.ps1 begin -Func <name> # a specific function
pwsh tools/manual_session.ps1 status             # what's open right now
```

`begin` stops the Grinder, **waits for it to actually release the lock**,
refuses a dirty tree or a red oracle, pops the target, and prints the full
context bundle (dossier · siblings · Sony-library provenance when the function
is library code · canonical route · the banked candidate's floor + instruction
diff). It also flags ledger `.c` files that never mention the function — usually
pre-rename leftovers that should not be trusted.

`end` only restarts what `begin` stopped: if the Grinder was already down when
you began, `end` leaves it down rather than starting one behind your back.
`-NoRelaunch` forces that either way.

**Never work over a live Grinder.** Its end-of-session scope check treats any
foreign edit to a tracked file as contamination and discards that session's
entire work ([[grinder-clobbers-uncommitted-edits]]). `.claude/`, `tools/`,
`docs/superpowers/` are all outside its allowed-dirt list. One writer on `main`,
ever.

A clean stop lands at the Grinder's next session boundary and can take ~10
minutes. That is normal — the script reports elapsed time. **Never force-kill
it**: that loses the in-flight session's ledger write.

---

## 1. The non-negotiables — these do NOT relax

The freedoms in §3 are bounded by these. They are the whole point of the project.

- **Exactly two completion states.** `COMPLETED-C` (pure C, zero cheat-asm,
  byte-matches) or `COMPLETED-INLINE-ASM-CANONICAL` (the `canonical` gate's call,
  not yours). No gradations, no "almost done" ([[completion-standard]]).
- **The oracle is the only truth** — full build+link SHA1 ==
  `62efab4f73f992798c43e8c730aa43baa10bb4fa`. Sandbox scores and exit codes are
  hints.
- **A fresh adversarial layer-2 `cheat-reviewer` before every completion-class
  commit.** The Grinder's default-FAIL Judge does not run in this lane, so this
  IS the gate. Default-FAIL; your own verdict is not credited
  ([[review-discipline-before-commit]]).
- **No cheats on `main`.** Register pins, hardcoded-`$N` `__asm__`, scheduling
  barriers are cheats, not a finish. INCOMPLETE stays
  `INCLUDE_ASM("asm/funcs", <func>);` ([[asm-until-matched]]).
- **No deferral.** Close the function you popped, or bank honestly and say so.
  A stuck item changes MODALITY, never target
  ([[no-deferral-work-to-completion]]).

---

## 2. Read the diff before you reach for a lever

```powershell
& tools/wteng.ps1 main sandbox <func> --disable all --diff --candidate memory/grind/<func>/candidate.c
```

Of grinder sessions that ran `sandbox`, only **37% ever looked at an instruction
diff**; 257 sessions spent 834 sandbox calls mutating C against a bare integer.
`_SsSndCrescendo` plateaued four sessions at floor 130 and closed in one once
the diff was read — 12 of its 13 surplus instructions were source-level, not
register allocation.

Each hunk is classed for you:

| Class | Meaning |
|---|---|
| **source-level** | different C would emit different instructions — a real lever |
| **operand-only** | same instructions, different registers — reg-alloc/scheduling |
| **not-scored** | equal once masked (moved branch displacement, section addend) — **chasing it is wasted work** |

If every remaining hunk is `not-scored`, the function is done and the residual is
a scorer artifact — say so rather than grinding. **A nonzero score with zero
scored hunks is a scorer defect, not a floor** — prove it with `verify-oracle`
and file it. (Data symbols defined only as `dlabel`s in `asm/data/*.s` were
invisible to the scorer until 2026-09-22; CD_cw's "floor 4" was entirely that.)

**Library code: check for published reference C first.** If `begin` prints a
*Sony library provenance* block, fetch the reference it names (SOTN psxsdk /
psyz) into `tmp/` and measure it BEFORE writing your own spelling — it is a lead,
not the answer (BB2 links a different build), but on CD_cw it carried the whole
structure (inline `set_alarm`/`get_alarm`/`callback`/`_memcpy` helpers, the
`tbl[com + 0x40]` index) that ~15 hand variants never found.

---

## 3. The loop — iterate on the candidate, not on `src/`

Edit **`memory/grind/<func>/candidate.c`** and score it:

```powershell
& tools/wteng.ps1 main sandbox <func> --disable all --diff --candidate memory/grind/<func>/candidate.c
```

**`--candidate` is required.** `main` carries `INCLUDE_ASM` for an incomplete
function, so a bare `sandbox <func>` scores the stub (`no_c_body: true`) — it
prints a pointer to the candidate but never substitutes it (an implicit fallback
would let a Grinder session that left `INCLUDE_ASM` in `src/` pass the driver's
byte check on the ledger's score).

To score several spellings at once — one line each, full output kept in
`tmp/sandbox_sweep/<func>/`, `-Hunks` prints only the SCORED hunks:

```powershell
pwsh tools/sandbox_sweep.ps1 -Func <func> -Variants tmp/<f>/a.c,tmp/<f>/b.c [-Hunks]
```

Keep scratch variants in `tmp/`, and bank only the winners (`candidate.c`) and
the instructive losers (`rejected/<slug>-<score>.c`) to the ledger.

**The working tree stays clean the whole time.** This is not tidiness — it makes
a real hazard structurally impossible: `dossier`, `queue *` and `sandbox` run
the engine's rollback machinery, which treats operator dirt like failed-state
dirt and **silently reverts uncommitted tracked edits**
([[engine-queue-ops-revert-uncommitted-tree]]; it ate a verified match on
2026-08-25). Keep src edits for the landing step alone.

The score is cheat-invisible — pins and `__asm__` injections are stripped before
scoring, so they cannot move it. They are inert here by construction.

### Your four freedoms (and the string attached to each)

1. **Choose your own approach.** No modality ladder. Pick the lever the diff
   argues for. *But:* when a class of spelling is exhausted, say it is exhausted
   and move to a different mechanism — don't re-run a dead sweep.
2. **Touch files outside the function.** The Makefile, a rodata config, a shared
   header. A grind session loses its work for this; you don't. *But:* the oracle
   is the check, and anything you change rides in the same commit with its
   reasoning.
3. **Record findings in prose.** No outcome schema, no verdict enum. *But:*
   findings still land in `memory/grind/<func>/` — evidence in `evidence.md`,
   what you ruled out in `hypotheses.md`. The next session (yours or the
   Grinder's) resumes from it.
4. **Keep full context.** One conversation, no cold restarts. *But:* bank to the
   ledger as you go, so an interruption costs nothing.

### Useful when local levers run out

`tools/fake_ablate.py` · `tools/loop_movables.py` · `tools/nrefs_census.py` ·
`tools/label_census.py` (read-only diagnostics) · `tools/permuter_annotate.py`
(directed permuter search; outputs are PROPOSALS, still reviewed) ·
`tools/decomp_me_scrape.py` (GCC 2.7.2 PSX scratch corpus) ·
`tools/find_duplicates.py` (near-duplicate COMPLETED-C analogs) ·
`engine diagnose <func>` (matchable / control-flow / canonical / plateau).

---

## 4. Landing it

1. Splice the candidate into `src/<file>.c`, replacing the `INCLUDE_ASM` line.
   Build files stay **LF** — the Write tool produces LF here.
   **If the banked candidate is a whole-file snapshot** (older ledgers hold one;
   newer ones hold just the body), move ONLY the function's definition. The file
   has moved on since the snapshot, and copying it wholesale silently reverts
   every sibling completed since — a regression the oracle will not necessarily
   catch. Check the staged diff shows one deletion: the `INCLUDE_ASM` line.
2. `& tools/wteng.ps1 main verify-oracle --rebuild --allow-dirty` → SHA1 must
   equal the oracle. **`--allow-dirty` is required here**: plain `--rebuild`
   refuses on dirty build inputs to protect the canonical `build/` reference the
   sandbox scores against, and during a landing the dirty tree IS the intended
   new reference. (Never pass it during the edit loop — that is what the
   refusal is for.)
3. `python3 tools/reviewer_precheck.py --func <f> --staged [--msg-file tmp/msg.txt]`
   — it mechanically settles the procedural facts so reviewer tokens go to the
   semantic judgment (a layer-1 review once burned ~117k tokens re-deriving
   these).
4. **Spawn a fresh `cheat-reviewer` agent.** Paste the precheck output. Brief it
   adversarially: default to FAIL, do not credit your verdict, work the specific
   diff against the 6-test checklist, and audit any rule doc the commit adds
   (self-sanctioning docs are banned outright).
5. **If the verdict is canonical-asm**, record the region grant BEFORE
   `queue done`, or it refuses with *"C/assembly function has no reviewed region
   grant"*: write `{file, sha256:[…]}` for the function into
   `tools/canonical_asm_regions.json`, hashing each island with
   `engine.completion.region_hashes` (never by hand — it hashes the island's
   exact source text, operands and constraints included). That file is the
   record of **what layer-2 actually reviewed**, so any later island edit
   correctly invalidates the grant. Preserve the file's existing indent; a
   reformat buries the 9-line addition in 180 lines of churn.
6. **PASS** → commit (`Match: <func> — COMPLETED-C (manual)`, `git commit -F
   tmp/msg.txt`) → `& tools/wteng.ps1 main queue done <func>` → `python3
   tools/check_completion_integrity.py`.
   **FAIL** → revert `src/` to `INCLUDE_ASM`, bank the body as
   `memory/grind/<func>/rejected/<slug>.c` with the reviewer's reasoning, and
   treat the objection as the next session's frontier. The item stays active:
   a FAIL is never grounds to rotate (§5).

**After fixing anything a reviewer flagged, re-stage before committing.** A
post-review edit leaves the file `MM` — corrected in the working tree, FAILED
text still in the index — and `git commit` takes the index. Verify with
`git show :src/<file>.c`, not by reading the working tree. (Caught by the
reviewer itself on func_8002D780, 2026-09-21; it would have landed the failed
comment.)

### Committing during a landing — three rules, each learned the hard way

1. **The layer-2 PASS precedes the commit.** A FAIL means the work does NOT
   land; it returns to INCOMPLETE. Never commit a completion "and then fix it",
   and never land a fix-up on top of a completion that is sitting on main under
   an outstanding FAIL.
2. **Completion-class changes use a completion-class subject** (`Match:` /
   `auth:` / `cheat-cleanup:`). The commit-msg guard chain, `audit_asm_cheats.py`
   and the owner's audit surfaces all key off the subject, so a body + grant
   landing under a `docs:`/`skills:` subject is invisible to every audit that
   matters.
3. **Commit explicit paths, never the bare index.** `git commit` takes the
   WHOLE index, and during a landing the completion is already staged for the
   reviewer. `git add <unrelated file> && git commit -m …` therefore sweeps the
   staged completion into that unrelated commit. Always
   `git commit -F tmp/msg.txt -- <the exact paths>`, and check `git show --stat
   HEAD` afterwards.

All three fired at once on func_8002EBDC (2026-09-21): a skills-doc commit
swallowed the whole staged completion, putting a 207-line body and a
canonical-asm grant on main under a `skills:` subject while a layer-2 FAIL was
outstanding. The reviewer caught it; `git reset --soft` recovered it because
nothing had been pushed. Also note `git commit --amend` changes the hash — do
not report a pre-amend hash as the landed commit.

A citation is verified by reading what the cited construct *does*, not by
confirming the line exists ([[citation-check-reads-the-cited-code]]). The two
manual completions that needed repair on 2026-09-21 both failed on citation
defects, not on the C.

---

## 5. Banking without a match

Perfectly legitimate — the lane is not obliged to close every function in one
sitting.

1. Write what you learned into `memory/grind/<func>/` (the measured floor, what
   you killed and how, what the frontier is now). Be honest about a flat floor;
   a false floor is worse than no floor.
2. `pwsh tools/manual_session.ps1 end` — asserts the tree is clean, commits the
   ledger and the session's `metrics/events.jsonl` (separate `metrics:` commit),
   lists the session's commits, relaunches the Grinder (`-NoRelaunch` to leave
   it down).
3. **Do NOT `queue rotate` the function.** It stays active at the top. Owner
   ruling 2026-09-26 ([[rotation-not-foreclosure]] Ruling 4): rotate only when
   agents are truly stuck and have burned multiple sessions without progress.
   One session without a match is not grounds. A layer-2 FAIL bans a
   construct, not the function; the objection becomes the next frontier. An
   open owner question in borderline.md is not grounds either. A close item,
   such as one with a register-seat-only diff, always stays. (func_80043454
   was rotated after one session at 57/479, every instruction correct, and
   the owner reversed it.)

The Grinder picks up your ledger and continues. Nothing is lost between lanes —
that's why this lane writes `memory/grind/<func>/` rather than `memory/wip/`.

---

## 6. Operator-only blockers — this lane's other job

Some finished functions strand because the remedy lives in a file no grind
session or Judge may write (`Makefile`, `tools/grinder/owner_cluster_grants.txt`,
`tools/grinder/scope_allow.txt`). Three have hit this: `func_800747D8`,
`func_80018094`, `func_8002D780`. They are filed in
`docs/grind/owner_actions.md`, and the bytes are usually already proven.

Clearing one is exactly what freedom #2 is for — but the remedy must be
**already authorized by a landed ruling**, cited by date. If it is not, it is an
architecture decision: log a `policy-question` entry to `docs/grind/borderline.md`
and move on. Do not self-authorize a new grant
([[ruling-record-lands-before-code]]).

---

## 7. Footguns

- **`$?` is unreliable inside `wsl bash -c`** (clobbered across shells, silently
  reads 0) → use PowerShell's `$LASTEXITCODE`.
- **Never hand-author `wsl bash -c '…python3 -m engine.cli…'`** — three nested
  shells eat awk/sed/heredocs, and `shell_footgun_guard.py` blocks it. Engine
  commands go through `& tools/wteng.ps1 main <cmd>`. Anything beyond one simple
  command → write a `.py`/`.sh`/`.ps1` to `tmp/` and run the file.
- **Multi-line commit messages → `git commit -F tmp/msg.txt`**, never a heredoc.
  **Use a unique filename** (`tmp/msg_<func>_<topic>.txt`) and write it with the
  Write tool: `tmp/` holds ~150 stale `msg*.txt` files from earlier sessions, and
  if the step that writes yours fails (a blocked hook, a typo), `-F` silently
  commits the stale one. On 2026-09-22 a skills-doc change landed under an old
  `Match: func_8007526C` subject this way (caught and amended before push).
  Check `git log -1 --format=%s` after every commit.
- **`make setup` is forbidden** — `bb2.ld` is hand-maintained; `asm/data/*.rodata*.s`
  are deliberately deleted. Don't recreate them.
- **A masked `0`** can hide a register diff or a source cheat-asm barrier. If
  sandbox says 0 but the full build mismatches, look for cheat-asm in the source
  ([[sandbox-zero-retire-fails]]).
- **Never recreate `bb2-work-*` worktrees.** They arm `main_reintegration_lock`
  and break the Grinder.

---

## Quick reference

| Command | Purpose |
|---|---|
| `pwsh tools/manual_session.ps1 begin [-Func <f>] [-DryRun]` | take the wheel: stop Grinder, pop target, print full context |
| `pwsh tools/manual_session.ps1 end [-NoRelaunch]` | bank the ledger, hand back to the Grinder |
| `pwsh tools/manual_session.ps1 status` | what's open, is the Grinder up, is the tree dirty |
| `& tools/wteng.ps1 main sandbox <f> --disable all --diff --candidate memory/grind/<f>/candidate.c` | honest floor + WHERE it differs (`--candidate` required) |
| `pwsh tools/sandbox_sweep.ps1 -Func <f> -Variants a.c,b.c [-Hunks]` | score many variants, scored hunks only |
| `& tools/wteng.ps1 main canonical <f>` | C vs ASM-region vs ASM-structural route |
| `& tools/wteng.ps1 main dossier <f>` | the full live-verified picture |
| `& tools/wteng.ps1 main verify-oracle --rebuild --allow-dirty` | the only truth (landing step; `--allow-dirty` needed once src is spliced) |
| `engine.completion.region_hashes(text, f)` | island hashes for `tools/canonical_asm_regions.json` — required before `queue done` on a canonical function |
| `& tools/wteng.ps1 main queue done <f>` | record the completion (re-checks cheats + SHA1) |
| `python3 tools/reviewer_precheck.py --func <f> --staged` | procedural facts for the reviewer brief |
| `python3 tools/check_completion_integrity.py` | standing audit of every completed function |
