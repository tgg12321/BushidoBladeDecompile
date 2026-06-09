---
name: rodata-cleanup-progress
description: Per-cluster checkpoint log for the rodata-cleanup project (docs/rodata-cleanup-project.md). Append a record per cluster retired. Read this to find the current frontier.
metadata:
  type: project
---

# Rodata cleanup project — progress log

See `docs/rodata-cleanup-project.md` for the project plan. This file logs each
cluster retirement (or attempt) so future agents can resume.

**Project goal**: 0 `asm/data/*.rodata*.o(.rodata)` entries remain in `bb2.ld`.

## Starting state (2026-06-09)

- 12 linked `asm/data/*.rodata*.s` blocks in `bb2.ld`
- 9 non-trivial blocks (~21.8 KB of rodata; 122 unique symbols = 28 jtbls + 86 strings + 8 raw-data)
- 3 trivial blocks (deletion-only — `.space 0` / single `.word 0x00000000` filler):
  - `101C.rodata_c_pre.s`
  - `101C.rodata_post.s`
  - `101C.rodata_text1a_DB8.s`
- 145 `replace_with_asmfile` stubs total in `asmfix.txt`; ~50 stub→symbol owner
  relationships span the 9 non-trivial blocks (the rest of the 145 stubs are
  not rodata-blocked and belong to engine-queue work, not this project)
- 24 jtbl-infra asmfix rules (all on `replay_camera_rob_back_loose2`)
- Oracle SHA1: `62efab4f73f992798c43e8c730aa43baa10bb4fa`
- Phase 0 inventory: `memory/project/rodata_block_inventory.csv` (159 rows)

## Retirement log

(empty — no clusters retired yet)

## Pilot cluster (Phase 1b)

(not yet selected — Phase 1 cluster classification pending)
