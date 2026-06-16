# Fleet handoff — 2026-06-15/16

Read this first if you're picking up the Autonomous Decomp Fleet. Operator guide:
`tools/fleet/README.md`. Full design: `docs/superpowers/specs/2026-06-15-autonomous-decomp-fleet-design.md`.

## Current state (STOPPED, clean)

- The fleet is **stopped** (no supervisor/lanes running). Oracle is green; `main` is clean.
- The historical **cheat-audit campaign** is partway: **~42 of ~895 completed functions
  re-audited, 7 cheats found, 1 plateau parked for owner.** Operational state is
  **preserved** in `tmp/fleet/state.json` (gitignored) so a relaunch RESUMES; the
  findings are durably committed in `docs/fleet/` + flagged on the GitHub board.
- Config in use: **single serial re-audit auditor + 1 backlog worker**, with per-role
  **model tiers** (cheap=sonnet, strong=opus) — see below.

## The payoff so far — 7 historical cheats found (act on these)

In `docs/fleet/regressions.md` (each has a precise construct + failed-test + next-action;
all flagged `REGRESSED` on the board, none auto-re-opened — your call to re-do):

1. `DispSamnailWindow` — constant-in-callee-save coercion (`s32 s0;` holding `1`).
2. `camera_InitBoneData` — do-while(0) + unused dead scalar + `new_var` pointer-alias
   (commit self-documented it as load-ordering control).
3. `camera_SetMatrix_8001DBE4` — empty-then-with-else branch-sense coercion.
4. `camera_Transform` — aggregated codegen-control (asymmetric hoist + recompute + inverted stores).
5. `cdrom_ConfigSPU` — unauthorized outer-`volatile` on a pointer (forensically confirmed: no IRQ writer).
6. `cdrom_GetCmdName` — `goto-end-with-ret-val` accumulator (forbidden family).
7. `cdrom_GetResultName` — same goto-accumulator family.

**Pending owner decision** (`docs/fleet/pending_owner.md` + `adjudications.md`):
- `func_8003B10C` — Adjudicator ruled a genuine pure-C plateau (split-read-defeats-hoist,
  both arms read the same global → GCC PRE re-merges; 11+ pure-C forms tested, floor 10).
  Recommendation: park permanently OR authorize canonical-asm. Your call.
- (`func_80078EC0` similarly adjudicated earlier — boolean-fold plateau.)

Backlog workers also left **WIP checkpoints** (`memory/wip/`, surfaced by SessionStart +
`queue next` when the next agent reaches each): `func_80057CC8` (reviewer PASS, floor 3 —
clean resume point), `func_8007C4B8` (notes + a rejected form, no candidate), and
`func_8001F938` — a triage record that raises its OWN user decision: the floor-2 path needs
an unconditional dual-typed read at +0x270 (cheat-reviewer FAIL, filed under `rejected/`),
while HEAD's *incomplete* body already uses a guarded form of the same read; decide whether
that read is sanctioned for this function or it's parked/canonical-asm. (`candidate.c` there
is floor-11 = HEAD — not a lower-floor resume, just the documentation anchor.)

## How to run

```powershell
# RESUME the campaign as-is (continues re-audit from ~42, keeps reshuffle + findings):
pwsh tools/fleet/launch.ps1 -Workers 1 -NoBlocked          # sonnet active + reaudit, opus adjudicator/blocked/forward-gate
pwsh tools/fleet/status.ps1                                 # monitor
pwsh tools/fleet/stop.ps1                                   # stop (add -Force to hard-kill)

# Re-apply the tractable reshuffle if you reset state (active worker hits winnable items first):
pwsh tools/fleet/reshuffle.ps1

# FRESH campaign (re-audit everything from scratch):
pwsh tools/fleet/stop.ps1 -Force ; Remove-Item tmp/fleet/state.json
pwsh tools/fleet/seed_reaudit.ps1 ; pwsh tools/fleet/launch.ps1 -Workers 1 -NoBlocked ; pwsh tools/fleet/reshuffle.ps1
```

## Decisions baked in this session (and why)

- **Model tiers** (`launch.ps1 -CheapModel sonnet -StrongModel opus`): active backlog
  decomp + the re-audit patrol + overseer = **sonnet**; blocked no-quit grind + forward
  merge-gate (auditor-normal + verifier) + adjudicator = **opus**. The auditor is tiered
  *by mode* (reaudit=cheap, forward-gate=strong). Validated: sonnet re-audit found real
  cheats with precise reasoning (matches the project's own `cheat-reviewer: model:sonnet`).
- **Active queue reshuffled** toward the tractable subset (verdict C + distance ≤15 +
  low rules first; asm-suspect pushed to the back). Reason: the active queue is the
  **hard tail** — the 895 easy functions are already done; only 1 sub-distance-8 low-rule
  item remained (already blocked). The reshuffle surfaces a `text1b` cluster of
  distance-9 0-rule C functions as the worker's best shot.
- **Re-audit findings are NOT auto-re-opened** — surfaced for owner review (a
  false-positive would churn workers on established completions).
- **Canonical-asm + novel techniques never auto-merge** — they park to `pending_owner`.

## Open question being watched

Whether **sonnet can do active decomp cleanly** is NOT yet settled: on the hard tail it
only ever *blocked* (0 in-review candidates, 0 cheats — so no waste, but no proof either).
The reshuffle gives it tractable targets now; the **first sonnet in-review candidate** is
the decisive test — if the opus auditor passes it (clean match) sonnet active is
vindicated; if it's a cheat (caught), and a pattern forms, revert active to opus
(`-CheapModel opus`). Re-audit on sonnet stays regardless (proven + the big cost saving).

## Known items / gotchas for the next agent

- **Never edit tracked fleet code while the fleet is running** — `Assert-MainClean` /
  the merge pre-check will `git checkout`/`clean` your uncommitted changes away as
  "contamination." Stop the fleet, edit, commit, relaunch.
- **Committing to `main` while worktrees are live needs the reintegration lock**
  (`& tools/reintegrate_lock.ps1 acquire` … `release`) — the PreToolUse hook blocks
  un-locked main mutations. Acquire is a *separate* tool call before the commit.
- **Re-audit currently wraps** (cursor cycles; after a full pass it re-audits clean
  functions again — wasteful on a long run). Deferred fix: make `reaudit-clean` remove
  the func from `reaudit.pending` so the campaign is a finite one-pass. Not harmful for
  ~2 days at the current pace; worth fixing before a multi-day unattended run.
- **Worktrees** `bb2-work-fleet-*` (+ stale `bb2-work-orch*/blk*/codex-*` from prior
  sessions) are left in place; the next launch re-syncs the fleet ones. Remove only via
  `tools/safe_remove_worktree.ps1` (junction-follow hazard — never `git worktree remove --force`).
- **gh token** had a transient 401 mid-campaign (board updates retry it now); if it
  fully lapses, `gh auth refresh` — the audit + `regressions.md` keep working without it.

## Build history (this session)

`a6405bfa` fleet · `d1109cd4`/`fe5eee2b`/`1024f909`/`44770b81` (drill+live fixes) ·
`879bb23e` board cheat-audit · `1446e7fc` model tiers + surgical cleanup · (+ this handoff).
