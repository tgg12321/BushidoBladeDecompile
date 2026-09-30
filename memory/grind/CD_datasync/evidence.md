# CD_datasync — evidence

Identity: PsyQ libcd `bios.c` v1.86 `CD_datasync(int mode)` (memory/closer/libcd-identity.md).
C reference: SOTN `src/main/psxsdk/libcd/bios.c:459` @db41b28 (matched PS1 code).

## ff-intr cheat-cleanup (2026-09-30) — one object for 0x800A1494..96 (owner ruling Q42)

Retro-audit FAIL (tmp/audit-2026-09-29/SUMMARY.md, "Shared-declaration follow-up"): the timeout report
read the Intr bytes through `extern volatile u8 g_cd_status_a` while the TU defines Sony's
`static volatile CD_intr Intr`.

Change: Intr is declared once in the TU (SOTN bios.c:80 @db41b28), with no other C handle. CD_datasync
uses `volatile CD_intr *intr = &Intr` and reads `intr->sync` / `intr->ready`. Every other statement is
unchanged.

### How it was scored

Same method and caveat as memory/grind/CD_sync/evidence.md (a byte-identical candidate scores 2 before
the rebuild; `--diff` is the check).

| candidate | score | --diff |
|---|---|---|
| `idx_1494 = &Intr.sync`, `idx_1494[0]` / `idx_1494[1]` | 2 | 6 hunks, all not-scored (branch-target cascade) |
| **landed: `intr = &Intr`, `intr->sync` / `intr->ready`** | 2 | 6 hunks, all not-scored |
| plain `Intr.sync` / `Intr.ready`, no handle (rejected/ff-intr-plain-member-20.c) | 20 | 5 source-level hunks |

Single-site ablations on the u8-handle form: t0 `Intr.sync` = 10; tb `Intr.ready` = 18. The handle is
needed.

### Post-rebuild

`pwsh tmp/orch/lock.ps1 rebuild ff-intr` (verify-oracle --rebuild, 2026-09-30): build SHA1
62efab4f73f992798c43e8c730aa43baa10bb4fa == oracle (build_matches, artifact_matches). With main's object
now addressing Intr too, `sandbox CD_datasync --disable all` = 0 (91/91 insns).
`tools/check_completion_integrity.py`: OK. `layer2 hash CD_datasync` = 93368c12196f1df9 (uncommitted tree; diff
tmp/audit-2026-09-29/ff-intr.diff). Awaiting a fresh layer-2 cheat-reviewer.
