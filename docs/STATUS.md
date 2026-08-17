# Project Status

**Live snapshot.** Refreshed 2026-08-17. For the live worklist run
`& tools/wteng.ps1 main queue next` (and `queue status` for counters); for
build health run `verify-oracle`. The workflow itself lives in
[`../CLAUDE.md`](../CLAUDE.md).

## Build

| | |
|---|---|
| Branch | `main` |
| Oracle SHA1 | `62efab4f73f992798c43e8c730aa43baa10bb4fa` |
| Build match | ✅ green — every `Match:` commit re-verifies full-build SHA1; grinder drills GO 2026-08-17 |
| Current worklist top | via `& tools/wteng.ps1 main queue next` |

## Function inventory (2026-08-17)

Counts from `python3 tools/check_completion_integrity.py` (the authority — it
applies the 2026-08-07 data-as-code structural filter; raw `asm/funcs/*.s` file
counts do not).

| | Count |
|---|------:|
| **COMPLETED-C** | **1,022** |
| **COMPLETED-INLINE-ASM-CANONICAL** (`inline_asm_canonical.txt`) | 178 |
| **INCOMPLETE** (queue items) | 274 |
| — active (grinder-eligible) | 227 |
| — parked (needs owner auth / blocked) | 47 |
| Data-as-code symbols (excluded by ruling) | 12 |
| `asm/funcs/*.s` files | 1,434 |

Queue verdict breakdown:

| Verdict | Active | Parked | Meaning |
|---|---:|---:|---|
| C | 143 | 44 | Pure-C reachable |
| ASM-PARTIAL | 37 | 3 | Contains canonical GTE/BIOS/HW asm |
| ASM-SUSPECT | 47 | 0 | Distance >50, no hand-coded signal — try pure-C first |

(No ASM-STRUCTURAL items remain routed: the 2026-08-07 canonical-gate fix
relabelled 26 stale ones.)

Debt indicators — the asmfix column collapsed in Campaign 4 (2026-08-06),
which retired the whole canonical-extraction wiring class:

| | Count | Was 2026-07-12 |
|---|------:|------:|
| Functions carrying `regfix.txt` rules | 82 | 139 |
| Functions carrying `asmfix.txt` rules | 14 | 153 |
| `replace_with_asmfile` bridge splices | 3 | 140 |

Owner ruling 2026-08-06: **all** `asmfix.txt` entries are debt — the end state
is zero regfix + zero asmfix ([[asmfix-all-debt-end-state]]).

## Source-file distribution

| File | Lines |
|---|---:|
| `text1b.c` | 7,440 |
| `code6cac_b.c` | 4,079 |
| `display.c` | 3,697 |
| `main.c` | 3,674 |
| `code6cac.c` | 3,197 |
| `text1a_c.c` | 2,061 |
| `text1b_b.c` | 1,961 |
| `code6cac_c_mid.c` | 1,859 |
| `code6cac_c2.c` | 1,666 |
| `system.c` | 1,259 |
| `sound.c` | 1,024 |
| `ings.c` | 974 |
| `text1a_pre.c` | 880 |
| `ings2.c` | 814 |
| `gpu.c` | 659 |
| `code6cac_c_ab.c` | 652 |
| `text1a_post.c` | 616 |
| `text1a_filepaths.c` | 574 |
| `config.c` | 573 |
| Other 13 files | < 550 each |
| **Total** | **40,377** |

32 C source files total. `text1b.c` shrank from 17,743 to 7,440 lines when the
2026-08-06 canonical-extraction wave moved 65 hand-coded bodies out to
`asm/funcs/`.

## Recent velocity

| Window | Commits |
|---|---:|
| Last 2 weeks | 599 |
| Last month | 1,274 |
| Total since project start (2026-03-23) | 4,833 |

161 `Match:` completions landed since 2026-07-12.

## Standing initiatives

### The Grinder — the only active pipeline
`tools/grinder/` is the standing single-lane driver for the queue's top active
item. It runs on `main`, persists per-function ledgers in `memory/grind/<func>/`,
and gates every completion through a default-FAIL Judge. Spec:
`docs/superpowers/specs/2026-07-06-grinder-pipeline-design.md`; skill:
`decomp-grind`. Owner audits: `docs/grind/decisions.md` + `docs/grind/journal.md`.

### Solver tooling (2026-08-04/05)
Two models of GCC 2.7.2's back end now back the hard cases: `tools/ra_solver`
(the whole allocation stack — global.c, local-alloc, reload/retry, suggested
registers) and `tools/sched_solver` (sched.c's list scheduler, exact on
6,978/6,978 blocks, both passes). The engine's inverse solver composes them into
minimal-perturbation lever hypotheses.

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
| Root cleanliness | ✅ 0 suspicious, 0 unknown (`tools/check_root_cleanliness.py`) |
| Completion integrity | ✅ all completed functions satisfy their category's invariants |
| CLAUDE.md / memory hygiene | Guards active (root-write, LF, CRLF-tooling-error, memory-write) |
| Oracle | Green (`build/bb2.exe` == `disc/SLUS_006.63`) |
| Function naming | Census + phase-2 reset wave complete (2026-08-07/10); 334 Sony names applied byte-neutral. `docs/naming/README.md` |
| Rodata cleanup | Phase A + B COMPLETE (2026-06-09; `bb2.ld` hand-maintained since) |
| maspsx gate-dependent completions | 6, tracked for transparency (fidelity-class assembler gates, not cheats) |
| Owner escalation shelf | `docs/escalations/` — incl. the cc1 fork-divergence ruling request (`_spu_FiDMA`) |
| WSL nonpaged-pool leak | Known host issue (~1 Job object per `wsl.exe` call); reboot when convenient ([[wsl-kernel-object-leak-audio]]) |

## Regenerating this snapshot

```pwsh
# Completion counts (the authority)
wsl bash -lc "cd '<repo>' && source .venv/bin/activate && python3 tools/check_completion_integrity.py"

# Queue counts + verdict breakdown
& tools/wteng.ps1 main queue status

# File inventory (WSL side)
ls asm/funcs/*.s | wc -l
grep -cE '^[a-zA-Z_]' inline_asm_canonical.txt
grep -c 'replace_with_asmfile' asmfix.txt
grep -oE '^[A-Za-z_][A-Za-z0-9_]*:' regfix.txt | sort -u | wc -l
wc -l src/*.c

# Velocity
git log --oneline --since='2 weeks ago' | wc -l
git log --oneline | wc -l

# Oracle
& tools/wteng.ps1 main verify-oracle
```
