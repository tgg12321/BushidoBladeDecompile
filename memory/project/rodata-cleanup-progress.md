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
cascade-math machinery (since these blocks emit no bytes).

### 2026-06-09 — Cascade test on `101C.rodata_post.s` (4 bytes) — FAILED, REVERTED

**Outcome**: oracle broke (build_sha1 `f21fe078...` ≠ expected `62efab4f...`).
Reverted via `git restore`.

**Setup**:
- Block contents: single `.word 0x00000000` (4 bytes of zero) at address
  `0x80010D88-0x80010D8C`, between `code6cac_c2.o(.rodata)` (24-byte switch
  jtbl for the only switch in code6cac_c2.c) and `101C.rodata_c2_post.o`
  ("Multipul Model" string at `0x80010D8C`).
- Grep for `0x80010D88` / `D_80010D88` across the entire repo → ONLY found
  in `asm/data/101C.rodata.s` (the splat parent's definition site). Zero
  references from any source, .s function, .data table, asmfix rule, or
  named symbol. The bytes appeared functionally orphan.

**Hypothesis tested**: with no static references, removing the 4 bytes would
either be a no-op (if SUBALIGN(2) happens to absorb the gap) or break only
trivial alignment. Empirical test was cheap.

**Empirical result**: removing the block AND deleting the .s file produced a
non-matching SHA1. The 4 bytes ARE load-bearing despite having no
detectable symbolic references. The cascade flowed through compiled-in
`%lo` immediates in downstream functions' TEXT, which encode the exact
addresses of every rodata symbol downstream of the cut.

**What this proves**:
1. Even a "no-references" 4-byte block participates in the address layout
   contract. Removing it without compensation breaks every downstream
   `lui+%hi/addiu+%lo` pair that references rodata > `0x80010D88`.
2. Trivial-byte-retirement requires either (a) compensating 4 bytes of
   rodata added to an adjacent C file with TU-boundary evidence, or
   (b) Phase 3 cascade-math machinery (`tools/re_attribute_rodata.py`)
   that updates every downstream relocation in coordination.

**What this rules out**: simple "delete trivial blocks from bb2.ld" as a
broad strategy. The two retirements from earlier today (`101C.rodata_c_pre.s`,
`101C.rodata_text1a_DB8.s`) succeeded ONLY because their `.rodata` sections
contained ZERO bytes (`.space 0`, lone `.align 2` directive after an already
4-aligned offset). Any non-zero-byte block cascades.

**Implication for the project plan**: §5 Phase 3 step 2 ("compute the
cascade") and §11.3 (Known unknown — is cascade prediction tractable?) move
from theoretical to required-before-next-step. No further retirement is
possible without either:
- Building the cascade math tooling (`tools/re_attribute_rodata.py` per §7.2)
- OR finding TU-boundary evidence that a compensating rodata constant
  belongs in an adjacent C file's source (§8.1 evidence threshold)

## Phase 1b pilot — partially complete

The trivial zero-byte case is done (2 blocks retired). The cascade-bearing
case is **empirically untractable without Phase 3 tooling** — confirmed by
the 4-byte test above.

Next pilot candidates (from `memory/project/rodata_clusters.csv`):

1. `101C.rodata_post.s` — 0 owners, trivial-but-4-bytes. **Next, if removable
   without cascading**: tests the "4 bytes of padding absorbed by the
   surrounding C-file rodata's natural alignment" hypothesis.
2. `101C.rodata_pre_post.s` — single-function single-file (216B, owner
   `func_80038170` active in queue, code6cac_c_mid). First true cluster
   retirement after the owner is matched.
3. `101C.rodata_c2_post.s` — single-function single-file (15B, owner
   `func_8003FA24` stub, config). Smallest stub-bearing cluster.
