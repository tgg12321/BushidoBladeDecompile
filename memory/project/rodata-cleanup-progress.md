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

### 2026-06-09 — Pilot retirement: zero-byte trivial blocks

**Retired**: `101C.rodata_c_pre.s` + `101C.rodata_text1a_DB8.s`

- Both contribute 0 bytes to the linked `.rodata` section (`.space 0` / lone
  `.align 2` directive). Confirmed via `mipsel-linux-gnu-objdump -h` on the
  build/asm/data/*.o objects.
- Zero cascade by construction: no output bytes changed, no downstream
  addresses shifted.
- Mechanical recipe: remove the `build/asm/data/<block>.o(.rodata);` line
  from `bb2.ld`; delete the `asm/data/<block>.s` source file;
  `verify-oracle --rebuild`.
- Bytes retired: 0
- Oracle SHA1 preserved: `62efab4f73f992798c43e8c730aa43baa10bb4fa`
- Cascade size: 0 downstream symbols affected

This pilot validates the basic mechanical workflow but does NOT exercise the
cascade-math machinery (since these blocks emit no bytes). The first
cascade-bearing retirement is still pending — `101C.rodata_post.s` (4
bytes) is the next test case for it.

## Phase 1b pilot — partially complete

The trivial zero-byte case is done. The cascade-bearing case (any block that
contributes >0 bytes to the link) is still untested. **Phase 1b is not
complete** until at least one >0-byte retirement validates the cascade-math
recipe.

Next pilot candidates (from `memory/project/rodata_clusters.csv`):

1. `101C.rodata_post.s` — 0 owners, trivial-but-4-bytes. **Next, if removable
   without cascading**: tests the "4 bytes of padding absorbed by the
   surrounding C-file rodata's natural alignment" hypothesis.
2. `101C.rodata_pre_post.s` — single-function single-file (216B, owner
   `func_80038170` active in queue, code6cac_c_mid). First true cluster
   retirement after the owner is matched.
3. `101C.rodata_c2_post.s` — single-function single-file (15B, owner
   `func_8003FA24` stub, config). Smallest stub-bearing cluster.
