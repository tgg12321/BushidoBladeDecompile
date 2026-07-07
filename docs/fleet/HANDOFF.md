> **SUPERSEDED 2026-07-06.** The fleet is retired; the Grinder
> (`tools/grinder/`, spec `docs/superpowers/specs/2026-07-06-grinder-pipeline-design.md`)
> is the autonomous pipeline. `tools/fleet/` is kept for reference until the
> grinder lands its first completions, then may be deleted. The 91 regressions
> were folded into the queue (origin=regression) on 2026-07-06.

# Fleet handoff — 2026-06-24

Read this first if you're picking up the BB2 decomp / Autonomous Decomp Fleet.
Operator guide: `tools/fleet/README.md`. Full design:
`docs/superpowers/specs/2026-06-15-autonomous-decomp-fleet-design.md`.

## Current state (CLEAN, ready for fresh work)

- **Fleet is STOPPED** (no supervisor/lanes running).
- **Oracle is GREEN.** `main` SHA1 == `62efab4f73f992798c43e8c730aa43baa10bb4fa`.
- **Tree is clean** (only `metrics/events.jsonl` modified, append-only engine log).
- **910 COMPLETED-C + 159 canonical-asm + 407 in queue** (per
  `tools/check_completion_integrity.py` 2026-06-24 — all completed
  functions satisfy their category's invariants).
- **Engine regression suite green** (161/161 passed) after the
  encoding fix (commit `2c7376c7`).

## What landed on 2026-06-24

### Owner rulings on pending_owner items (3 → blocked)
Per standing policy [[no-park-permanently]] (codified this session — see
`memory/feedback/no-park-permanently.md`): "park permanently" is not an
available disposition. Three functions ruled send-back-to-blocked with
the closest-form findings + concrete un-tried lever hypotheses propagated
to their WIP entries:

- `func_8003B10C` — split-read-defeats-hoist same-symbol plateau, v9
  (arm-local + per-arm-duplicated-call) reached masked floor 13 = baseline+3
  (closest ever recorded). Next worker resumes from v9 and attacks the s2/s0
  RA cascade in the jal `func_800493E4` delay slot.
- `func_80078EC0` — VBlank IRQ-probe boolean-fold wall. Variant F proves
  bytes C-reachable via Lever D (forbidden). Un-tried lever class:
  `sltu`/`sltiu`-shape boolean computation, or wider-return-type forms the
  caller narrows — both bypass the jump-opt fold trigger.
- `motion_SetMotion` — RA-priority + cross-jump-merge coupled wall (cluster
  with saEft00Add / marionation_Exec / cpu_side_move_dir_4 / func_8007C97C).
  Un-tried levers: cross-jump suffix-break via USE/CLOBBER, case-9/11
  single-shared-store letting cross-jump RESPREAD, combined sel2-lifetime +
  post-early-exit sequencing rework.

Commit: `4d80cb08`.

### Engine encoding fix
`engine/cheats.py`, `engine/volatile_cheats.py`, `engine/inlineasm.py`:
all text-file `Path.read_text()` callsites now pass `encoding="utf-8"`.
Previously the locale default (cp1252 on Windows) crashed on the 398
non-ASCII bytes in `regfix.txt` (em-dashes in comments), breaking
`tools/check_completion_integrity.py`. Engine test suite 161 passed.
Commit: `2c7376c7`.

### WIP stub for the one remaining stranded Match
`func_8003D9A0` — branch `work/blkB2-0614` had a Match commit
(`e1f215db`, 2026-06-14) that scored sandbox 0 + SHA1==oracle on the
branch but scores 19 on current main (compilation-context drift per
[[stranded-branch-work-not-transplantable]]). WIP stub propagates the
branch's loop-counter-hoist technique + concrete resume steps. The other
two June-14 stranded Matches (`func_80061064 + gnd_get_fog` on blkA,
`func_8003FE40` on blkB) were already reintegrated via fresh per-function
loop work in mid-June and are now COMPLETED-C on main.
Commit: `809a31b1`.

### Regression-backlog sweep (6 cleanups + 3 WIP-fail-stubs)
Owner-directed sweep of the 19 fresh regressions accumulated 2026-06-17
through 2026-06-22 (the re-audit patrol's findings during the campaign).
Triaged as trivial-or-easy (2), easy (7), hard (10); sweep attempted the
first 9 and SHA1-gated + cheat-reviewer-PASSed each survivor.

**6 cheat-cleanups landed:**
- `func_80065680` — conflicting `extern s16` redecl deleted (`4b5b71ff`)
- `func_80044170` — drop volatile cast on `a0` (`3c087c92`)
- `func_80044DE4` — `sp18/sp28[4]` -> `[3]` (`9c9b8d9e`)
- `func_8006C168` — void/s32 type mismatch fixed (`d1d80523`)
- `func_80075830` — verbose shift-pair -> `* 16` (`cf4c762b`)
- `saTan0Main` — `char b` -> `u8 b` + false comment removed (`e46c5115`)

**3 WIP-fail-stubs** — the strip attempt scored non-zero (construct
load-bearing despite auditor's expected no-op fix). Each carries
concrete un-tried lever hypotheses:
- `func_8006E49C` — `int new_var;` decl IS load-bearing (RA bias). 0 -> 35.
- `func_80044670` — empty `do{}while(0)` IS load-bearing (reorg.c peephole). 0 -> 4.
- `func_800455AC` — `s16 (*new_var)[]` IS load-bearing (address-caching). 0 -> 20.

**10 deferred to fleet campaign:** `func_80040510`, `func_800417D0`,
`func_80068F70`, `func_8006A3CC`, `func_8006F038`, `func_80074220`,
`hirahira_w_frie`, `math_Distance3D`, `mottest_disp`, `se_data_set`.
Hard bucket: load-bearing cheats needing structural pure-C re-derivation.

Full resolution log: `docs/fleet/regressions.md` § "Resolution log — 2026-06-24 sweep".

### Worktree + branch cleanup
Per [[stranded-branch-work-not-transplantable]]: 9 obsolete worktrees +
12 branches removed safely via `tools/safe_remove_worktree.ps1`. All
removed branches' target functions are already COMPLETED-C on main
(cross-checked 2026-06-24 against queue + canonical registry).

Remaining infrastructure (KEEP — fleet relaunch reuses these):
- `bb2-work-fleet-bw1`, `bb2-work-fleet-bw2`, `bb2-work-fleet-blk`,
  `bb2-work-fleet-adj` — the live fleet's worktrees.

## How to run

```powershell
# RESUME the campaign — the active queue's tractable-tier reshuffle is preserved
# in tmp/fleet/state.json. The re-audit cursor wraps (a one-pass fix is still
# deferred); for a long unattended run consider seeding fresh first.
pwsh tools/fleet/launch.ps1 -Workers 1 -NoBlocked    # default config
pwsh tools/fleet/status.ps1                           # monitor
pwsh tools/fleet/stop.ps1                             # stop (-Force to hard-kill)

# Re-apply the tractable reshuffle if state is reset:
pwsh tools/fleet/reshuffle.ps1

# FRESH campaign:
pwsh tools/fleet/stop.ps1 -Force ; Remove-Item tmp/fleet/state.json
pwsh tools/fleet/seed_reaudit.ps1
pwsh tools/fleet/launch.ps1 -Workers 1 -NoBlocked
pwsh tools/fleet/reshuffle.ps1
```

## Recommended priorities for the next worker / campaign

In rough order of return-on-effort:

1. **The 10 deferred hard-bucket regressions** — load-bearing cheats on
   COMPLETED-C functions on main. Each has a precise auditor diagnosis +
   recommended next_action in `docs/fleet/regressions.md`. Highest
   semantic-debt-reduction per session (each clean-up retires a real cheat
   from main's COMPLETED-C corpus). Recommended as the next fleet campaign's
   primary lane.

2. **The 3 WIP-fail-stubs from today's sweep** — `func_8006E49C`,
   `func_80044670`, `func_800455AC`. Each has empirical floor data + named
   un-tried levers. Tractable for one focused session each.

3. **The 3 blocked-lane WIPs from today's pending-owner rulings** —
   `func_8003B10C`, `func_80078EC0`, `motion_SetMotion`. Each has v9 /
   sltu / cross-jump-USE-CLOBBER hypotheses propagated. Hard but not
   structurally exhausted.

4. **The forward decomp queue** — 328 active items in the active tier,
   reshuffled to surface verdict-C + low-distance + low-rules first. The
   queue's top item (per SessionStart hook): `func_80045294`
   (text1a_c.c) — verdict C, distance 2, 0 rules. Already in the queue
   though previously parked; check WIP for its history before grinding.

## Known items / gotchas for the next agent

- **Standing policy [[no-park-permanently]]**, codified 2026-06-24:
  every function reaches one of two terminal states (authorized
  canonical-asm with definitive STRONG hand-coded signals, OR clean
  pure-C match). Anything not yet there cycles in blocked, no matter
  how many sessions. Cluster precedent + circumstantial evidence does
  NOT lower the bar.

- **Open meta-questions for owner deliberation (NOT acted on this session):**
  1. **IRQ-probe-mmio-single-read carve-out** under
     `legitimate-volatile-interrupt-touched.md` — would NOT close
     func_80078EC0 (the wall is the jump-opt fold, not the load) but may
     unlock sibling IRQ probes registered via `bios_SysEnqIntRP`. Needs
     SOTN-master evidence.
  2. **"Documented N-function cluster" canonical-asm signal** — if the
     RA-priority + cross-jump-merge coupled-wall cluster (saEft00Add /
     marionation_Exec / cpu_side_move_dir_4 / func_8007C97C /
     motion_SetMotion) reaches 6+ members with permuter benchmark + SOTN
     cross-check, may justify a STRONG-signal-set expansion.

- **Re-audit cursor wraps** (the deferred fix from the 2026-06-15
  handoff) — for multi-day unattended runs, the cursor will re-audit
  clean functions. Worth fixing before the next long campaign.

- **Never edit tracked fleet code while the fleet is running** — see
  `Assert-MainClean` / merge pre-check.

- **Committing to `main` while worker worktrees are live needs the
  reintegration lock** (`& tools/reintegrate_lock.ps1 acquire` … `release`).

- **Worktrees** — remove only via `tools/safe_remove_worktree.ps1`
  (junction-follow hazard). Never `git worktree remove --force`.

- **gh token** — board updates retry a transient 401; if it fully
  lapses, `gh auth refresh`.

- **Orphaned agent processes / RAM bloat** — `pwsh tools/reap_orphans.ps1`
  (dry-run default; `-Execute` to reap). See [[orphan-process-reaper]].

## Build history (this session)

- `4d80cb08` — fleet: owner rulings on 3 pending_owner items
- `2c7376c7` — engine: read_text(encoding="utf-8") for all text readers
- `809a31b1` — wip: func_8003D9A0 stub from stranded branch
- `4b5b71ff`, `3c087c92`, `9c9b8d9e`, `d1d80523`, `cf4c762b`, `e46c5115`
  — 6 cheat-cleanups from the regression sweep
- `<wip-stub commit>` — 3 WIP-fail-stubs from the regression sweep
- `<this commit>` — handoff + regressions.md resolution log + worktree
  cleanup memo
