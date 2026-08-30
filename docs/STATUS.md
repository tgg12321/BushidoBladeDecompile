# Project Status

**Live snapshot.** Refreshed 2026-08-30 (post rule-system removal). For
the live worklist run `& tools/wteng.ps1 main queue next` (and `queue status`
for counters); for build health run `verify-oracle`. The workflow itself lives
in [`../CLAUDE.md`](../CLAUDE.md).

## Build

| | |
|---|---|
| Branch | `main` |
| Oracle SHA1 | `62efab4f73f992798c43e8c730aa43baa10bb4fa` |
| Build match | ✅ green — full `verify-oracle --rebuild` re-verified 2026-08-30 |
| Grinder | stopped since closing func_800485EC (2026-08-27); next target func_8002D320, no ledger yet — relaunch starts it fresh |
| Current worklist top | via `& tools/wteng.ps1 main queue next` |

**Representation (owner ruling 2026-08-19, [[asm-until-matched]]):** an
INCOMPLETE function is committed as `INCLUDE_ASM("asm/funcs", <func>);` — no
cheat-asm, no draft C on `main`. Candidates live in
`memory/grind/<func>/`; queue distance comes from the pinned/ledger honest
floor.

**No post-processing rule machinery exists.** The build pipeline is
`cpp | cc1 | prologue_fix | maspsx | multu_pad | as` — every completed function
compiles to its bytes directly from C (or authorized canonical asm). The
historical regfix/asmfix rule system reached zero rules on 2026-08-25 and was
removed entirely (files, pipeline stages, tooling) on 2026-08-30; see git
history if archaeology is ever needed.

## Function inventory (2026-08-30)

Counts from `python3 tools/check_completion_integrity.py` (the authority — it
applies the 2026-08-07 data-as-code structural filter; raw `asm/funcs/*.s` file
counts do not).

| | Count |
|---|------:|
| **COMPLETED-C** | **1,072** |
| **COMPLETED-INLINE-ASM-CANONICAL** (`inline_asm_canonical.txt`) | 180 |
| **INCOMPLETE** (queue items) | 222 |
| — active (grinder-eligible) | 199 |
| — escalated (decision packets awaiting owner rulings — [[escalation-not-parked]]) | 23 |
| Data-as-code symbols (excluded by ruling) | 12 |

≈ 85% of the 1,474 in-scope functions are in a COMPLETED state.

Queue verdict breakdown (2026-08-30, active items): 165 `C` (pure-C
reachable) + 34 `ASM-PARTIAL` (contain canonical GTE/BIOS/HW asm).

The 23 escalated items carry decision packets in `docs/grind/decisions.md` /
`docs/grind/borderline.md` (notable members: `main`, the CD_* cluster
(CD_sync / CD_ready / CD_datasync), `motion_Close`, `func_800324D0`
(jtbl re-wiring question), `special_camera_get_rot_dir`, `func_8002FC80`).
They are non-blocking for the grind but each needs an owner ruling to return
to active.

## Source-file distribution (2026-08-30)

| File | Lines |
|---|---:|
| `text1b.c` | 6,484 |
| `display.c` | 3,520 |
| `main.c` | 3,263 |
| `code6cac.c` | 2,771 |
| `code6cac_b.c` | 2,642 |
| `text1b_b.c` | 1,914 |
| `text1a_c.c` | 1,748 |
| Other 25 files | < 1,700 each |
| **Total** | **35,194** |

32 C source files total. Line counts SHRINK during migration waves (bodies
move to `asm/funcs/`) and grow again as functions reach COMPLETED-C —
neither direction is a health signal by itself.

## Recent velocity

| Window | Commits |
|---|---:|
| Last 2 weeks | 540 |
| Last month | 1,216 |
| Total since project start (2026-03-23) | 5,373 |

208 `Match:` completions landed since 2026-07-12 (21 since 2026-08-24).
Recent grinder closes ran 1–37 sessions each — the easiest-first queue's
remaining tail is genuinely hard.

## Standing initiatives

### The Grinder — the only active pipeline
`tools/grinder/` is the standing single-lane driver for the queue's top active
item. It runs on `main`, persists per-function ledgers in `memory/grind/<func>/`,
and gates every completion through a default-FAIL Judge. Spec:
`docs/superpowers/specs/2026-07-06-grinder-pipeline-design.md`; skill:
`decomp-grind`. Owner audits: `docs/grind/decisions.md` + `docs/grind/journal.md`.

### Escalation model (owner ruling 2026-08-24, [[escalation-not-parked]])
The parked state is retired: every INCOMPLETE item is ACTIVE or ESCALATED
(a concrete decision packet awaiting an owner ruling; batched, non-blocking,
never indefinite). 23 items are currently escalated — the largest owner-facing
backlog; reviewing their packets is the biggest single lever for unblocking
queue breadth.

### Solver tooling (2026-08-04/05)
Two models of GCC 2.7.2's back end back the hard cases: `tools/ra_solver`
(the whole allocation stack — global.c, local-alloc, reload/retry, suggested
registers) and `tools/sched_solver` (sched.c's list scheduler, exact on
6,978/6,978 blocks, both passes). The engine's inverse solver composes them into
minimal-perturbation lever hypotheses — `tools/ra_solver/inverse_compose.py`
(classify / hypothesis) and `tools/ra_solver/sweep.py` (batch driver); the
Grinder dispatches them via the `solver` modality (added 2026-08-24).

### Manual close-out path
Manual per-function work uses the `decomp-orchestrate` skill (single focused
agent on `main` driving the engine as a toolkit). Layer-2 fresh
`cheat-reviewer` remains mandatory for any completion-class commit per
[[review-discipline-before-commit]].

### Retired
Retired initiatives (fleet, closer, the rule system) are recorded in
`docs/HISTORY.md` + git history; their docs and tooling are deleted.

## Health / debt

| Item | State |
|---|---|
| Root cleanliness | ✅ (last verified 2026-08-24, `tools/check_root_cleanliness.py`) |
| Completion integrity | ✅ all completed functions satisfy their category's invariants (verified 2026-08-30) |
| CLAUDE.md / memory hygiene | Guards active (root-write, LF, CRLF-tooling-error, memory-write) |
| Oracle | Green (`build/bb2.exe` == `disc/SLUS_006.63`, full rebuild 2026-08-30) |
| Function naming | Census + phase-2 reset wave complete (2026-08-07/10); 334 Sony names applied byte-neutral. `docs/naming/README.md` |
| Rodata cleanup | Phase A + B COMPLETE (2026-06-09; `bb2.ld` hand-maintained since) |
| maspsx gate-dependent completions | 6, tracked for transparency (fidelity-class assembler gates, not cheats) |
| Metrics Postgres | local instance unreachable (query staleness only; `metrics/events.jsonl` remains source of truth — run `python tools/metrics/sync.py` when back) |
| WSL nonpaged-pool leak | Known host issue (~1 Job object per `wsl.exe` call) — surfaced only when it actually breaks something ([[wsl-kernel-object-leak-audio]]) |

## Regenerating this snapshot

```pwsh
# Completion counts (the authority)
wsl bash -lc "cd '<repo>' && source .venv/bin/activate && python3 tools/check_completion_integrity.py"

# Queue counts + verdict breakdown
& tools/wteng.ps1 main queue status

# File inventory (WSL side)
ls asm/funcs/*.s | wc -l
grep -cE '^[a-zA-Z_]' inline_asm_canonical.txt
wc -l src/*.c

# Velocity
git log --oneline --since='2 weeks ago' | wc -l
git log --oneline | wc -l

# Oracle
& tools/wteng.ps1 main verify-oracle
```
