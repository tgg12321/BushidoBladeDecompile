---
name: decomp-manual
description: >-
  Drive ONE Bushido Blade 2 function by hand on main, with the Grinder's full context loaded:
  stops the Grinder, pops the queue top (or a named function), prints the dossier, then works it
  with free choice of approach and lands it through the mandatory layer-2 adversarial review. Use
  when asked to work a function manually / by hand, take over from the Grinder, close a plateaued
  function, or clear an operator-only blocker the pipeline cannot touch.
---

# BB2 decomp — the manual lane

You are the hand on the wheel for ONE function. The Grinder (`decomp-grind`) stays the default
pipeline; this lane is for what it is structurally bad at — the long-tail functions where cold
restarts keep re-deriving what is already on disk. The CLAUDE.md non-negotiables apply unchanged
(two completion states, oracle SHA1 is the only truth, no cheats on main, INCOMPLETE =
`INCLUDE_ASM`, no deferral). The Grinder's Judge does not run here: **the fresh layer-2
`cheat-reviewer` IS the gate.** **The bar is `.claude/rules/completion-bar.md` (owner ruling Q91,
SOTN-equivalent) — read it before you start.** A construct that only shapes codegen is fine
with a `/* FAKE: <measured reason> */` label, nothing false asserted, the simplest known form;
exhaustion dossiers, frame proofs and symbol retirement are hygiene (record as debt, never a
blocker). Older ledger notes that a construct is "outside the frozen list" no longer bind.

## 0. Handshake — always start here

```powershell
pwsh tools/manual_session.ps1 begin              # queue top
pwsh tools/manual_session.ps1 begin -Func <name> # a specific function
pwsh tools/manual_session.ps1 status             # what's open right now
```

`begin` stops the Grinder (waits for it to release the lock — a clean stop lands at the next
session boundary, up to ~10 min; **never force-kill it**), refuses a dirty tree or a red oracle,
pops the target, and prints the context bundle: dossier, siblings, Sony-library provenance,
canonical route, the banked candidate's floor + instruction diff. It flags ledger `.c` files that
never mention the function (pre-rename leftovers — don't trust them).

`end` restarts only what `begin` stopped; `-NoRelaunch` leaves the Grinder down either way.
**Never work over a live Grinder** — its scope check discards a session on any foreign edit.

## 1. Read the diff before reaching for a lever

```powershell
& tools/wteng.ps1 main sandbox <func> --disable all --diff --candidate memory/grind/<func>/candidate.c
```

| Hunk class | Meaning |
|---|---|
| **source-level** | different C would emit different instructions — a real lever |
| **operand-only** | same instructions, different registers — reg-alloc/scheduling |
| **not-scored** | equal once masked (branch displacement, section addend) — chasing it is wasted work |

All hunks `not-scored` ⇒ the function is done and the residual is a scorer artifact; prove it with
`verify-oracle` and say so. A nonzero score with zero scored hunks is a scorer defect, not a floor.

**Library code:** if `begin` prints a *Sony library provenance* block, fetch the named reference C
(SOTN psxsdk / psyz) into `tmp/` and measure it before writing your own spelling. It is a lead, not
the answer — BB2 links a different build.

## 2. The loop — iterate on the candidate, never on `src/`

Edit `memory/grind/<func>/candidate.c` and score with the command above. **`--candidate` is
required**: main carries `INCLUDE_ASM`, so a bare `sandbox <func>` scores the stub
(`no_c_body: true`). Several spellings at once (full output in `tmp/sandbox_sweep/<func>/`):

```powershell
pwsh tools/sandbox_sweep.ps1 -Func <func> -Variants tmp/<f>/a.c,tmp/<f>/b.c [-Hunks]
```

Scratch variants live in `tmp/`; bank winners (`candidate.c`) and instructive losers
(`rejected/<slug>-<score>.c`). **Keep the working tree clean the whole time** — `dossier`,
`queue *` and `sandbox` can silently revert uncommitted tracked edits. Src edits happen only at
landing. The score is cheat-invisible; pins and `__asm__` injection cannot move it.

Your freedoms, each with its string:
1. **Choose your own approach** — no modality ladder. When a spelling class is exhausted, say so
   and change mechanism; don't re-run a dead sweep.
2. **Touch files outside the function** (Makefile, rodata config, shared header) — the oracle is the
   check, and the change rides in the same commit with its reasoning.
3. **Record findings in prose** — but in `memory/grind/<func>/` (`evidence.md`, `hypotheses.md`)
   so the next session resumes from them.
4. **Keep full context** — and bank to the ledger as you go.

When local levers run out: `tools/fake_ablate.py`, `tools/loop_movables.py`,
`tools/nrefs_census.py`, `tools/label_census.py` (read-only diagnostics);
`tools/permuter_annotate.py` (directed permuter — outputs are proposals, still reviewed);
`tools/decomp_me_scrape.py`; `tools/find_duplicates.py`; `engine diagnose <func>`; the
instrumented cc1 (`tools/gcc-2.7.2/cc1`, `BB2_*_DEBUG`) for pass-level dumps.

## 3. Landing it

1. Splice the candidate into `src/<file>.c` (`<file>` = the queue item's TU id, e.g.
   `main/psxsdk/libapi/sendpad`), replacing the `INCLUDE_ASM` line (LF; the Write tool is
   LF here). If the banked candidate is a whole-file snapshot, move ONLY the function's definition —
   copying the file reverts every sibling completed since. The staged diff should delete exactly the
   `INCLUDE_ASM` line.
2. `& tools/wteng.ps1 main verify-oracle --rebuild --allow-dirty` → SHA1 must equal the oracle.
   (`--allow-dirty` only here — the dirty tree is the intended new reference. Never during the loop.)
3. `python3 tools/reviewer_precheck.py --func <f> --staged [--msg-file tmp/msg_<f>.txt]` — settles
   the procedural facts so the reviewer spends its effort on semantics.
4. **Spawn a fresh `cheat-reviewer`.** Paste the precheck output; brief it adversarially (default
   FAIL, your verdict not credited, `.claude/rules/completion-bar.md` items 2-5 on this diff, audit
   any rule doc the commit adds; hygiene gaps go in its `hygiene_debt`, not a FAIL), and give it
   the landed `rules:` commit hash for any owner ruling the body relies on. Its verdict must carry
   `body_hash` (`layer2 hash <func>`). Wait for every reviewer you started; a split verdict is a FAIL.
5. **Canonical-asm verdict:** before `queue done`, record the region grant in
   `tools/canonical_asm_regions.json` (`{file, sha256:[…]}`, hashes from
   `engine.completion.region_hashes` — never by hand; keep the file's indent).
6. **Record the verdict** with the reviewed body still in `src/`:
   `& tools/wteng.ps1 main layer2 record <func> --verdict-file <reviewer JSON> --reviewer <id> --scope <match|cheat-cleanup|auth>`
   (or `--verdict … --expect-hash … --notes …`). A PASS refuses if `src/` no longer holds the
   reviewed body — then re-review, never re-hash.
   - **PASS** → commit `Match: <func> — COMPLETED-C (manual)` with `layer2.jsonl` →
     `& tools/wteng.ps1 main queue done <func>` → `python3 tools/check_completion_integrity.py`.
   - **FAIL** → revert `src/` to `INCLUDE_ASM`, bank the body as `rejected/<slug>.c` with the
     reviewer's reasoning; the objection is the next frontier. A FAIL is never grounds to rotate.

Commit rules:
- **PASS precedes commit.** Never commit a completion "and fix it after".
- **Completion-class subject** (`Match:` / `auth:` / `cheat-cleanup:`) — audits key off it.
- **Commit exactly your paths** (`git commit -F tmp/msg_<f>_<topic>.txt -- <paths>`), because a
  bare `git commit` sweeps whatever else is staged. If a path is also dirty from a peer (`MM`),
  stage only your hunk and commit from the index instead. After fixing anything a reviewer flagged,
  re-stage, and check `git show :src/<file>.c`. Use a unique message filename written with the Write
  tool (stale `tmp/msg*.txt` files get committed silently if your write fails); check
  `git log -1 --format=%s` and `git show --stat HEAD` after every commit.

## 4. Banking without a match

Legitimate — the lane need not close every function in one sitting.
1. Write the measured floor, what you killed and how, and the new frontier into
   `memory/grind/<func>/`. A false floor is worse than none.
2. `pwsh tools/manual_session.ps1 end` — asserts a clean tree, commits the ledger, relaunches the Grinder (`-NoRelaunch` to
   leave it down).
3. **Do NOT `queue rotate`** after one session, one layer-2 FAIL, an open owner question, or when the
   item is close — only when truly stuck across multiple sessions (rotation-not-foreclosure rule).

## 5. Operator-only blockers

Some proven functions strand because the remedy lives in a file no grind session may write
(`Makefile`, `tools/grinder/owner_cluster_grants.txt`, `tools/grinder/scope_allow.txt`); they are
filed in `docs/grind/owner_actions.md`. Clear one only when the remedy is **already authorized by a
landed ruling**, cited by date. Otherwise log a `policy-question` entry in
`docs/grind/borderline.md` and move on — never self-authorize a new grant.

## Footguns

- `$?` is unreliable through `wsl bash -c` — use PowerShell's `$LASTEXITCODE`.
- A masked sandbox `0` with a full-build mismatch usually means cheat-asm in the source.
- `make setup` is forbidden; never recreate `asm/data/*.rodata*.s` or `bb2-work-*` worktrees.

## Quick reference

| Command | Purpose |
|---|---|
| `pwsh tools/manual_session.ps1 begin [-Func <f>] [-DryRun]` / `end [-NoRelaunch]` / `status` | take / hand back the wheel |
| `& tools/wteng.ps1 main sandbox <f> --disable all --diff --candidate memory/grind/<f>/candidate.c` | honest floor + where it differs |
| `pwsh tools/sandbox_sweep.ps1 -Func <f> -Variants a.c,b.c [-Hunks]` | score many variants |
| `& tools/wteng.ps1 main canonical <f>` / `dossier <f>` | C-vs-asm route / full live picture |
| `& tools/wteng.ps1 main verify-oracle --rebuild --allow-dirty` | the only truth (landing step) |
| `& tools/wteng.ps1 main layer2 hash <f>` / `record <f> …` / `check <f>` | reviewed-body key / record verdict / the gate |
| `& tools/wteng.ps1 main queue done <f>` | record completion (cheats + layer-2 PASS on this body + SHA1) |
| `python3 tools/reviewer_precheck.py --func <f> --staged` | procedural facts for the reviewer brief |
| `python3 tools/check_completion_integrity.py` | standing audit of every completed function |
