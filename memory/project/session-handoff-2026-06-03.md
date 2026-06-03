---
name: session-handoff-2026-06-03
description: Session wrap (2026-06-02 / 03) — built WIP candidate registry + ran 3 workflow rounds (all 5/5 NO_PROGRESS, 0 cheats slipped) + fixed two architectural bugs (stale-worktree hallucinations + junction-removal destruction). Oracle green at handoff. Open user decisions on B844 escalation, CBB0 score-52-vs-55 discrepancy, C7A0/C86C v6 ceiling.
metadata:
  type: project
---

# Session handoff — 2026-06-02 / 03

## Final state at handoff

- **Oracle:** GREEN at `62efab4f73f992798c43e8c730aa43baa10bb4fa`
- **Working tree:** clean (only pre-session untracked files: `.bb2_attempts/`, `docs/SATAN2MAIN_HANDOFF.md`, `docs/saTan2Main_handoff/`)
- **Engine tests:** 130 passed / 0 failed (up from 122 — added PAD_NOPS macro-strip regression test)
- **Workflow worktrees:** 15 locked worktrees (~440 MB) under `.claude/worktrees/wf_*` from rounds 1, 3 (round 2 worktrees may already be cleared). Safe to remove anytime via `pwsh tools/safe_remove_worktree.ps1 <path> -Force`.

## What landed (17 commits)

**Infrastructure:**
- `a82756d2` — WIP candidate registry (engine/wip.py + memory/wip/ contract + queue_top hook + cli integration)
- `01c91d16` — Initial WIP entries for C97C + CBB0
- `39030e42` — WIP entry for B844
- `e76d2036` + `68370f69` — setup_worktree.ps1 (bash failed Windows; PowerShell version works)
- `eb24afac` — Three-layer fix for stale-worktree hallucinations (HEAD parity, verification rule, head_sha helper, safe_remove_worktree.ps1)

**Workflow rounds (all 5/5 NO_PROGRESS, 0 cheats reached the reviewer):**
- `bd59c343` — Round 1
- `32ae4e5d` — Round 1 corrections (B844 sibling tail-diff; C97C canonical-asm REJECTED)
- `d736e150` — Round 2 (C86C v6 LEVER DISCOVERED — first known `$a3` flip)
- `29f731c3` — Round 3 (5 NO_PROGRESS; CBB0 worker reported phantom CE0C blocker)
- `a1622b94` — CBB0 verification: phantom blocker refuted empirically; score 52→55 discrepancy uncovered

**Other:**
- `45a7bb03` — Reverted the 0e845d25 Match: func_8007C97C (sp[4] write-only cheat caught by independent reviewer)

## WIP registry status

```
func_8007C7A0  floor 12  (HEAD 16)  | 6 sessions | reviewer=null
func_8007C86C  floor 12  (HEAD 20)  | 6 sessions | reviewer=null | v6 LEVER discovered
func_8007C97C  floor null (HEAD 24) | 3 sessions | reviewer=FAIL_on_prior_form_PASS_likely_on_candidate_BUT_canonical_asm_REJECTED
func_8007CBB0  floor 55  (HEAD 149) | 5 sessions | reviewer=null | SCORE-52-WIP-CLAIM-NOT-REPRODUCING
func_8007B844  floor 6   (HEAD 7)   | 5 sessions | reviewer=null | escalation candidate
```

## Open decisions on the user's plate

1. **B844 escalation** — exhausted across 5 grinding contexts (~70 structural variants + ~50k+ permuter iters). Three options:
   - Run directed-PERM permuter (untried; random permuter exhausted)
   - Park-with-cluster (join cpu_side_move_dir_4 / marionation_Exec documented-structural-ceiling cohort)
   - Fresh SOTN borderline-research pass

2. **CBB0 re-derivation** — `a1622b94` empirically proved the round-3 worker's CE0C meta-blocker was phantom (stale-worktree hallucination), BUT in the process discovered the WIP-documented "score 52 / build_insns 151 exact match" no longer reproduces (current measurement: score 55 / build_insns 129). Round 4's first task should be re-deriving the actual score-52 form, OR confirming the prior measurement was anomalous.

3. **C7A0 / C86C v6-lever ceiling** — Round 2 found the `s32 lim_x = D_8009BE78` block-local that flips the X-preserve pseudo to target's `$a3`. Round 3's 12 micro-variants proved the ceiling: every form that introduces a separate pseudo emits the +6 cascade; every form that folds back matches floor 12. No middle ground in the explored space. Same options as B844 — directed-PERM, park-cluster, or fresh research.

## Architectural improvements that landed

Three layers of defense against the round-3 failure mode:

1. **`tools/setup_worktree.ps1`** — HEAD parity enforcement. Detects worktree-stale state at worker startup, hard-resets to main HEAD before bootstrapping junctions. Policy via `$env:WORKTREE_STALE_POLICY` (sync/fail/warn).

2. **`.claude/rules/verify-claims-against-main.md`** — path-scoped rule auto-loading on `memory/wip/**`, workflow scripts, etc. Codifies the "verify before recording any file/line citation" discipline.

3. **`engine.wip.head_sha()`** — helper for pinning WIP `sessions[]` entries to the git HEAD at measurement time. Future agents detect drift via `git log <head>..HEAD`.

Plus the safety bonus: **`tools/safe_remove_worktree.ps1`** prevents the destructive `git worktree remove --force` flow that destroyed main's `.venv` when I tested junction removal. Now: `pwsh tools/safe_remove_worktree.ps1 <path> -Force` detaches reparse points first, THEN removes the worktree.

## Forbidden-technique catalog updates

- **`func_8007C97C` canonical-asm authorization REJECTED** as standing policy. Recorded as `rejected_forms[0]` in C97C's `meta.json` + standing-policy banner at top of `notes.md`. Round-1 + round-3 workers both proposed this; both rejected on the same evidence (S1=0, S2=none, S6=none, S8=no).

- **Write-only frame coercion** (`s32 sp[4]; sp[0..3] = …;`) added to the cheat catalog via the C97C revert (`45a7bb03`). Mechanical detector not yet wired — filed as `find_write_only_arrays` follow-up.

## Minor cleanup the user may want later

- `CLAUDE.md` is 301 lines (precommit hook target ≤200). Worth a future content-into-AGENTS.md split.
- `tmp/` is 132 MB of accumulated scratch (gitignored; `rm -rf tmp/*` reclaims).
- 15 locked workflow worktrees ≈ 440 MB (run safe_remove_worktree.ps1 on each to clean).

## How to resume

Next session's SessionStart hook will print the queue top + WIP banner. The WIP entries have full session ledgers. The verify-claims-against-main rule auto-loads on memory/wip/ reads. If a fresh workflow round is launched, workers will bootstrap their worktrees via setup_worktree.ps1 with HEAD-parity enforcement.
