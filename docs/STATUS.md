# Project Status

**Live snapshot.** Refreshed 2026-09-07 (post `-msoft-float` adoption). For
the live worklist run `& tools/wteng.ps1 main queue next` (and `queue status`
for counters); for build health run `verify-oracle`. The workflow itself lives
in [`../CLAUDE.md`](../CLAUDE.md).

## Build

| | |
|---|---|
| Branch | `main` |
| Oracle SHA1 | `62efab4f73f992798c43e8c730aa43baa10bb4fa` |
| Build match | ✅ green — full `verify-oracle --rebuild` re-verified 2026-09-07 under the new flag set |
| Grinder | relaunched 2026-09-07 after the `-msoft-float` adoption (Judge/recon/layer-1 on Fable 5.1, execution on Opus since 1930c839) |
| Current worklist top | via `& tools/wteng.ps1 main queue next` |

**Representation (owner ruling 2026-08-19, [[asm-until-matched]]):** an
INCOMPLETE function is committed as `INCLUDE_ASM("asm/funcs", <func>);` — no
cheat-asm, no draft C on `main`. Candidates live in
`memory/grind/<func>/`; queue distance comes from the pinned/ledger honest
floor.

**Canonical flags (2026-09-07):** `CC_FLAGS` carries `-mel -msoft-float` — both are
target-triple fidelity corrections (the prebuilt cc1 defaults big-endian and
hard-float; PsyQ cc1psx prints `# Cc1 defaults: -mgas -msoft-float`). Hard float
had doubled `loop.c`'s invariant-hoist threshold (122 vs 58); blast radius was
exactly two functions (`docs/grind/decisions.md` 2026-09-07 OWNER RULING).

**No post-processing rule machinery exists.** The build pipeline is
`cpp | cc1 | prologue_fix | maspsx | multu_pad | as` — every completed function
compiles to its bytes directly from C (or authorized canonical asm). The
historical regfix/asmfix rule system reached zero rules on 2026-08-25 and was
removed entirely (files, pipeline stages, tooling) on 2026-08-30; see git
history if archaeology is ever needed.

## Function inventory (2026-09-07)

Counts from `python3 tools/check_completion_integrity.py` (the authority — it
applies the 2026-08-07 data-as-code structural filter; raw `asm/funcs/*.s` file
counts do not).

| | Count |
|---|------:|
| **COMPLETED-C** | **1,114** |
| **COMPLETED-INLINE-ASM-CANONICAL** (`inline_asm_canonical.txt`) | 195 |
| **INCOMPLETE** (queue items) | 165 |
| — active (grinder-eligible) | 165 |
| — foreclosed | **0** (all unparked in the 2026-09-06/07 owner reviews) |
| — escalated | **0** |
| Data-as-code symbols (excluded by ruling) | 11 |

≈ 89% of the 1,474 in-scope functions are in a COMPLETED state.

Queue verdict breakdown (2026-09-07): 139 `C` (pure-C reachable) + 26
`ASM-PARTIAL` (contain canonical GTE/BIOS/HW asm). Top of queue:
func_800324D0 (reopened 2026-09-07 — its FAKE-carrying completion was an
artifact of the hard-float loop threshold) then the easiest-first tail.

**Escalation backlog: CLEARED 2026-08-30.** The owner ruled on the full
23-item escalated set in one batch (decisions.md "2026-08-30 — OWNER
RULINGS — escalation batch resolved", 10 rulings): instrumentation grants,
the special_camera_get_rot_dir header-fix scope grant, the motion_Close
prebuilt-object canonical grant (landed — COMPLETED-INLINE-ASM-CANONICAL),
the registry-bound owner-cluster canonical-grant door
(`tools/grinder/owner_cluster_grants.txt` — unblocks the COP2 cluster incl.
func_8002FC80 and func_8002D320), the func_800324D0 jtbl re-wiring lane,
two provenance adjudications, one bounded calibration probe, and the return
of every no-question item to active. No standard was lowered.

## Source-file distribution (2026-09-07)

| File | Lines |
|---|---:|
| `text1b.c` | 7,154 |
| `code6cac_b.c` | 3,684 |
| `display.c` | 3,632 |
| `main.c` | 3,452 |
| `code6cac.c` | 3,254 |
| `text1b_b.c` | 1,980 |
| `text1a_c.c` | 1,907 |
| Other 25 files | < 1,900 each |
| **Total** | **38,661** |

32 C source files total. Line counts SHRINK during migration waves (bodies
move to `asm/funcs/`) and grow again as functions reach COMPLETED-C —
neither direction is a health signal by itself.

## Recent velocity

| Window | Commits |
|---|---:|
| Last 2 weeks | 959 |
| Last month | 1,546 |
| Total since project start (2026-03-23) | 6,096 |

262 `Match:` completions landed since 2026-07-12 (75 since 2026-08-24, 33 since
2026-09-02). Recent grinder closes ran 1–88 sessions each — the easiest-first queue's
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
never indefinite). Currently ZERO items are escalated (batch resolved
2026-08-30).

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
| Root cleanliness | ✅ (last verified 2026-09-07, `tools/check_root_cleanliness.py`) |
| Completion integrity | ✅ all completed functions satisfy their category's invariants (verified 2026-09-07) |
| CLAUDE.md / memory hygiene | Guards active (root-write, LF, CRLF-tooling-error, memory-write) |
| Oracle | Green (`build/bb2.exe` == `disc/SLUS_006.63`, full rebuild 2026-09-07) |
| Function naming | Census + phase-2 reset wave complete (2026-08-07/10); 334 Sony names applied byte-neutral. `docs/naming/README.md` |
| Rodata cleanup | Phase A + B COMPLETE (2026-06-09; `bb2.ld` hand-maintained since) |
| maspsx gate-dependent completions | 11, tracked for transparency (fidelity-class assembler gates, not cheats) |
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
