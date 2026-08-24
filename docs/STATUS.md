# Project Status

**Live snapshot.** Refreshed 2026-08-24 (post parked-set review + migration
sweep 2). For the live worklist run
`& tools/wteng.ps1 main queue next` (and `queue status` for counters); for
build health run `verify-oracle`. The workflow itself lives in
[`../CLAUDE.md`](../CLAUDE.md).

## Build

| | |
|---|---|
| Branch | `main` |
| Oracle SHA1 | `62efab4f73f992798c43e8c730aa43baa10bb4fa` |
| Build match | ✅ green — every `Match:` commit re-verifies full-build SHA1; last full verify 2026-08-24 |
| Grinder | stopped 2026-08-23 on a usage-limit stall (not a fault); relaunch resumes func_8002FC80 s1 exactly |
| Current worklist top | via `& tools/wteng.ps1 main queue next` |

**Representation (owner ruling 2026-08-19, [[asm-until-matched]]):** an
INCOMPLETE function is committed as `INCLUDE_ASM("asm/funcs", <func>);` — no
rules, no cheat-asm, no draft C on `main`. Candidates live in
`memory/grind/<func>/`; queue distance comes from the pinned/ledger honest
floor. **56** functions are byte-coupling deferred (jtbl-referencing /
rodata-emitting / position-coupled) and keep the legacy representation until
solved. NOTE (2026-08-24): the original deferral heuristic had false
positives — 9 parked "rodata-emitting" deferrals were actually eligible and
were migrated in sweep 2 (`a7892ba2`); the remaining 56 deferral reasons are
individually recorded in `tmp/migration-deferred.txt` / borderline.md but the
set has NOT been re-proven member-by-member.

## Function inventory (2026-08-24)

Counts from `python3 tools/check_completion_integrity.py` (the authority — it
applies the 2026-08-07 data-as-code structural filter; raw `asm/funcs/*.s` file
counts do not).

| | Count |
|---|------:|
| **COMPLETED-C** | **1,051** |
| **COMPLETED-INLINE-ASM-CANONICAL** (`inline_asm_canonical.txt`) | 179 |
| **INCOMPLETE** (queue items) | 244 |
| — active (grinder-eligible) | 240 |
| — escalated (decision packets awaiting owner rulings — [[escalation-not-parked]] 2026-08-24) | 4 |
| Data-as-code symbols (excluded by ruling) | 12 |

Queue verdict breakdown (2026-08-24):

| Verdict | Active | Parked | Meaning |
|---|---:|---:|---|
| C | 176 | 32 | Pure-C reachable |
| ASM-PARTIAL | 35 | 1 | Contains canonical GTE/BIOS/HW asm |

Debt indicators — the 2026-08-19 asm-until-matched migration (191 functions)
plus the 2026-08-24 sweep 2 (9 more) converted 200 INCOMPLETE functions to
`INCLUDE_ASM` and retired their rules wholesale:

| | Count | Was 2026-08-19 | Was 2026-08-17 |
|---|------:|------:|------:|
| Rule-carrying functions (all in the 56 deferred) | 37 | 41 | 96 |
| Total regfix+asmfix rules outstanding | 689 | 708 | 1,573 |

Owner rulings 2026-08-06/2026-08-19: **all** rules are debt; the end state is
zero regfix + zero asmfix ([[asmfix-all-debt-end-state]], [[asm-until-matched]]).
The remaining 689 retire per function at COMPLETED-C (their bodies are
byte-coupled to the build; the wave-2 mechanical retirement was measured a
dead end — borderline.md 2026-08-19).

## Source-file distribution (2026-08-24)

| File | Lines |
|---|---:|
| `text1b.c` | 6,938 |
| `display.c` | 3,601 |
| `code6cac_b.c` | 3,457 |
| `main.c` | 3,338 |
| `code6cac.c` | 2,957 |
| `text1a_c.c` | 1,856 |
| `text1b_b.c` | 1,915 |
| Other 25 files | < 1,900 each |
| **Total** | **37,606** |

32 C source files total. Line counts SHRINK during migration waves (bodies
move to `asm/funcs/`) and grow again as functions reach COMPLETED-C —
neither direction is a health signal by itself.

## Recent velocity

| Window | Commits |
|---|---:|
| Last 2 weeks | 532 |
| Last month | 1,071 |
| Total since project start (2026-03-23) | 5,113 |

186 `Match:` completions landed since 2026-07-12.

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
never indefinite). All 33 former parks converted: 29 returned to active
(with per-item directives; the RA/scheduler endgame class gets the new
`solver` grinder modality), 4 escalated with packets (main — maspsx
branch-fill fidelity; func_80048AD0 — prototype correction;
func_80083794 + motion_Close — prebuilt-object routing).

### Solver tooling (2026-08-04/05)
Two models of GCC 2.7.2's back end now back the hard cases: `tools/ra_solver`
(the whole allocation stack — global.c, local-alloc, reload/retry, suggested
registers) and `tools/sched_solver` (sched.c's list scheduler, exact on
6,978/6,978 blocks, both passes). The engine's inverse solver composes them into
minimal-perturbation lever hypotheses — concretely `tools/ra_solver/inverse_compose.py`
(classify / hypothesis) and `tools/ra_solver/sweep.py` (batch driver); the Grinder
dispatches them via the `solver` modality (added 2026-08-24).

### Manual close-out path
Manual per-function work uses the `decomp-orchestrate` skill (single focused
agent on `main` driving the engine as a toolkit). Layer-2 fresh
`cheat-reviewer` remains mandatory for any completion-class commit per
[[review-discipline-before-commit]].

### Retired
- **Closer Phase 3** (PsyQ psxsdk adoption) — retired 2026-07-13. All 7 banked
  candidates map to functions still in `engine/queue.json`, so the Grinder
  inherits them by construction; no dedicated-session work remained.
- **Multi-agent fleet** (`tools/fleet/`) — retired 2026-07-06.

## Health / debt

| Item | State |
|---|---|
| Root cleanliness | ✅ (verified 2026-08-24, `tools/check_root_cleanliness.py`) |
| Completion integrity | ✅ all completed functions satisfy their category's invariants (verified 2026-08-24) |
| CLAUDE.md / memory hygiene | Guards active (root-write, LF, CRLF-tooling-error, memory-write) |
| Oracle | Green (`build/bb2.exe` == `disc/SLUS_006.63`) |
| Function naming | Census + phase-2 reset wave complete (2026-08-07/10); 334 Sony names applied byte-neutral. `docs/naming/README.md` |
| Rodata cleanup | Phase A + B COMPLETE (2026-06-09; `bb2.ld` hand-maintained since) |
| maspsx gate-dependent completions | 6, tracked for transparency (fidelity-class assembler gates, not cheats) |
| Owner escalation shelf | retired 2026-08-24 — the `_spu_FiDMA` fork-crash question is a borderline.md policy-question entry |
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
grep -oE '^[A-Za-z_][A-Za-z0-9_]*:' regfix.txt | sort -u | wc -l
wc -l src/*.c

# Velocity
git log --oneline --since='2 weeks ago' | wc -l
git log --oneline | wc -l

# Oracle
& tools/wteng.ps1 main verify-oracle
```
