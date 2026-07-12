# Project Status

**Live snapshot.** Refreshed 2026-07-12 to reflect the current engine
workflow. For the live worklist run `& tools/wteng.ps1 main queue next`
(and `queue status` for counters); for build health run
`verify-oracle`. The workflow itself lives in
[`../CLAUDE.md`](../CLAUDE.md).

## Build

| | |
|---|---|
| Branch | `main` |
| Oracle SHA1 | `62efab4f73f992798c43e8c730aa43baa10bb4fa` |
| Build match | ✅ (verify-oracle green) |
| Current worklist top | via `& tools/wteng.ps1 main queue next` |

## Function inventory (2026-07-12)

| | Count |
|---|------:|
| Total identified functions (`asm/funcs/*.s`) | 1,435 |
| **COMPLETED-C** (approx: total − queue − canonical) | **~826** |
| **COMPLETED-INLINE-ASM-CANONICAL** (`inline_asm_canonical.txt`) | 170 |
| **INCOMPLETE** (queue items) | 439 |
| — active (worker-eligible) | 370 |
| — parked (blocked / plateau) | 69 |

Queue verdict breakdown:

| Verdict | Count | Meaning |
|---|---:|---|
| C | 258 | Pure-C reachable |
| ASM-PARTIAL | 48 | Contains canonical GTE/BIOS/HW asm |
| ASM-SUSPECT | 107 | Distance >50, no hand-coded signal — try pure-C first |
| ASM-STRUCTURAL | 26 | Distance >500 + hand-coded tier — surface for user auth |

Debt indicators:

| | Count |
|---|------:|
| Functions carrying `regfix.txt` rules | 139 |
| Functions carrying `asmfix.txt` rules | 153 |
| Functions with `replace_with_asmfile` bridge splices | 140 |

## Source-file distribution

| File | Lines |
|---|---:|
| `text1b.c` | 17,743 |
| `code6cac_b.c` | 4,252 |
| `display.c` | 3,703 |
| `main.c` | 3,695 |
| `code6cac.c` | 3,407 |
| `text1a_c.c` | 1,982 |
| `text1b_b.c` | 1,975 |
| `code6cac_c_mid.c` | 1,878 |
| `code6cac_c2.c` | 1,698 |
| `text1a.c` | 1,467 |
| `system.c` | 1,234 |
| `sound.c` | 1,023 |
| `ings.c` | 962 |
| `ings2.c` | 817 |
| `gpu.c` | 652 |
| `code6cac_c_ab.c` | 645 |
| `config.c` | 581 |
| `text1a_filepaths.c` | 574 |
| Other 13 files | < 550 each |
| **Total** | **51,002** |

31 C source files total (Phase B of the rodata-cleanup project split
several files into `_pre` / `_post` sub-TUs).

## Recent velocity

| Window | Commits |
|---|---:|
| Last 2 weeks | 421 |
| Last month | 839 |
| Total since project start (2026-03-23) | 3,366 |

## Standing initiatives

### Closer Phase 3 — PsyQ psxsdk adoption
Adopt SOTN-matched psxsdk C source for BB2's 92 census-proven verbatim-linked
Sony PsyQ 4.0 library functions. **Session 14 (2026-07-12):**
~46 of 92 census hits closed since the mission started (2026-07-09);
LIBGTE canonical-asm pass retired 10 functions on 2026-07-11 (10 GTE
modules, 27 forbidden regfix rules cleared alongside); saTan2Main closed
2026-07-12 with a newly-sanctioned pure-C lever
(`hoist-shared-arm-computation-defeats-copy-pref`). Ledger:
`memory/closer/phase3-progress.md`.

### The Grinder — default autonomous pipeline
`tools/grinder/` is the standing single-lane driver for the queue's top
active item. It runs on `main`, persists per-function ledgers in
`memory/grind/<func>/`, and gates every completion through a default-FAIL
Judge. Spec: `docs/superpowers/specs/2026-07-06-grinder-pipeline-design.md`;
skill: `decomp-grind`. Owner audits: `docs/grind/decisions.md` +
`docs/grind/journal.md`.

### Manual close-out path
Manual per-function work uses the `decomp-orchestrate` skill (single focused
agent on `main` driving the engine as a toolkit). Layer-2 fresh
`cheat-reviewer` remains mandatory for any completion-class commit per
[[review-discipline-before-commit]].

## Health / debt

| Item | State |
|---|---|
| Root cleanliness | ✅ 0 suspicious, 0 unknown (`tools/check_root_cleanliness.py`) |
| CLAUDE.md / memory hygiene | Guards active (root-write, LF, CRLF-tooling-error, memory-write) |
| Oracle | Green (`build/bb2.exe` == `disc/SLUS_006.63`) |
| Rodata cleanup project | Phase A (block retirement) DONE; Phase B (per-function follow-up) via TU re-split — ongoing per `docs/rodata-cleanup-project.md` |
| Volatile-grant proposal queue | Empty (§1-§4 all ratified — `memory/closer/volatile-grant-proposals.md`) |
| Standing banked candidates | 7 (post-session-14): `cdcontrol_trio`, `cdcw_tslTm2LoadImage`, `exec_game_sotn_hybrid`, `marionation_p6_volatile1496`, `marionation_vAT1_notailwrap`, `spu_writebyio_splice`, `spusetreverbmodeparam_struct` |

## Regenerating this snapshot

```pwsh
# Queue counts + verdict breakdown
& tools/wteng.ps1 main queue status

# File inventory (from the project root, WSL side)
ls asm/funcs/*.s | wc -l
grep -cE '^[a-zA-Z_]' inline_asm_canonical.txt
grep -c 'replace_with_asmfile' asmfix.txt
wc -l src/*.c

# Velocity
git log --oneline --since='2 weeks ago' | wc -l
git log --oneline | wc -l

# Oracle
sha1sum disc/SLUS_006.63
& tools/wteng.ps1 main verify-oracle
```
