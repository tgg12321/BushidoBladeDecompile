# Naming sweep 2026-10-03 (research 2026-10-02 "naming3")

## Applied (2026-10-03)

An independent default-refute verifier re-derived the HIGH accepted-class rows. Its verdicts are
in `verify-verdicts.tsv` (16 ACCEPT). The 0x8004C388 emulation harness it cites
(`tmp/naming3/verify/emu_c388.py`) is untracked scratch. Applied through the tools:

- `func_manifest.csv` via `tools/naming_wave.py --from-census --only-file`. build_census.py
  consumes it.
  - 9 RESETs (reset-contradicted): `cpu_ai_pick_move_for_situation`, `cpu_set_move_command_and_dir`,
    `check_dodge_kawashi`, `cpu_check_same_dir_timer`, `cpu_check_special_move_input`,
    `cpu_calc_move_pattern_trajectory`, `efc_particle_queue_entry`, `mario_test_get_guard_power`,
    `mario_test_get_charm_bonus`.
  - 1 RENAME (computation-restatement): 0x8004C388 `satan_helper` -> `math_MidpointS16x3U8x2`.
- `data_manifest.csv` via `tools/data_wave.py --manifest-csv`.
  - 0x800A2604 -> `i_stat` and 0x800A260C -> `d_pcr`. These are Sony's static names, as spelled in
    SOTN `src/main/psxsdk/libetc/intr.c:42,44`. They replace the verifier's `g_intr_i_stat_reg` /
    `g_intr_dpcr_reg`, the same way 0x800A2608 became `i_mask` in 78838a969.
  - 0x800A1510 -> `g_vsync_gpu_stat_reg` and 0x800A1514 -> `g_vsync_rcnt1_count_reg`. SOTN's
    `vsync.c` leaves both unnamed (`D_8002C2A8/AC`).
  - 0x800FF610 `g_gte_vector_template` -> RESET to `D_800FF610`.
- Skipped: 0x800A2608 (already `i_mask`).
- Census: INFERRED 522 -> 512, AUTO 396 -> 405, CORROBORATED 118 -> 119.

Everything below is the research write-up as it stood before the apply. Its 0x800A2604/0C names are
superseded by the list above. Rows not listed above were not applied (MEDIUM, new-class or held).

## Research write-up (2026-10-02)

The working tree was dirty from another lane during the run (src/system.c, include/*, registries).
Line citations are against that tree as of 2026-10-02 evening. `tmp/naming3/` paths are untracked scratch.

## Method

1. **Census regenerated** read-only: `build_census.py` copied to
   `tmp/naming3/build_census.py` with its output redirected, writing `tmp/naming3/function-names.csv`.
   Universe 1455. Tiers: VERIFIED 418, CORROBORATED 118, INFERRED 522, AUTO 396, SUSPECT 1
   (`save_vc_ctrl`, owner-excluded). Two LINK-MAP DESYNC rows: `snd_AllocSe` (0x80046934) and
   `snd_SeNullCallback` (0x80046954). The only drift from the committed CSV is two glabel cells.
2. **Fact tables** (`build_table.py`): `functions.json` (per function: tier, callees with their
   tiers, %hi/%lo/%gp_rel data refs, cop2 use, C definition), `datarefs.json`, `symmap.json`.
3. **Four veins**, each with a default-refute stance and checked against the 09-24/25/25b/29
   rejected and keep lists (`git show pre-slim-2026-10-01:docs/naming/sweep-2026-09-29/...`):
   - `v1_func/`: functions under accepted classes. Includes an R3000A interpreter over raw EXE words: `emu.py`, `test_*.py`.
   - `v2_data/`: data under accepted classes. The C-level Sony-API argument scan is `api_args.csv`. This vein's
     agent stalled; I finished it by hand (`write_rows.py`).
   - `v3_audit/`: INFERRED names that the typed C now contradicts. These are RESET rows.
   - `v4_newclass/`: one new evidence class, defined for an owner ruling (`class.md`).
4. `merge.py` writes `proposals.tsv` with columns `vein kind addr current_name proposed_name verdict
   evidence_class class_status confidence decisive_evidence wrong_reading_ruled_out hazards`.
   Rows are sorted ACCEPTED first, then HIGH first.

## Counts (61 rows)

| class_status | kind | HIGH | MEDIUM | other |
|---|---|---:|---:|---:|
| ACCEPTED | func | 1 RENAME + 9 RESET | 2 RENAME + 6 RESET | 2 HOLD |
| ACCEPTED | data | 5 RENAME + 1 RESET | 3 RENAME + 1 NEW + 2 RESET | — |
| NEW-CLASS-PROPOSED | func | 7 | 4 (incl. 1 cascade family row) | — |
| NEW-CLASS-PROPOSED | data / member | 2 / 11 | 0 / 5 | — |

This is short of the 20-40 per kind the brief targeted. Under the accepted classes, the function
veins are close to exhausted. The census has not moved since the 09-29 wave, so no callee-tier
promotions opened up. Every AUTO wrapper, leaf and cop2 body already has a prior verdict, and the
55 INFERRED `gte_*` names all pass the op check. Most of the new yield is RESETs, which the new
types make provable, plus rows that depend on the proposed new class.

## Top candidates (accepted classes)

- **The `cpu_*` / `mario_test_*` misnomer cluster (RESET, HIGH, contradicted-by-body; v3).** The
  typed records show these are not what their names say:
  - 0x80030D7C `cpu_ai_pick_move_for_situation` is the physics step for the 12 Obj80106A78 objects.
  - 0x80030A2C `cpu_set_move_command_and_dir` spawns an object with random velocity and spin.
  - 0x80030BA8 `check_dodge_kawashi` finds a resting object, consumes it and returns its kind.
  - 0x8003339C `cpu_check_same_dir_timer` compares the move's current *frame*, not a direction, and runs for both fighters.
  - 0x80032C50 `cpu_check_special_move_input`: no input is read and nothing is returned. 09-29 had held it at MEDIUM.
  - 0x80023F08 `cpu_calc_move_pattern_trajectory` is the shared per-fighter update. For a human fighter it takes the real pad record.
  - 0x800455AC `efc_particle_queue_entry` is a heap-slot allocator whose blocks are disc-read destinations.
  - 0x80021974 / 0x800219E4 `mario_test_get_guard_power` / `_charm_bonus` return MoveScript pointers.
- **INTR/VSYNC MMIO pointer statics (RENAME, HIGH, hardware-role; v2).** Each pointer's .data word is
  a register address, and every user is VERIFIED LIBETC code:
  - 0x800A2608 `g_sys_irq_counter` → `g_intr_i_mask_reg`. It holds 0x1F801074 and is read and written by GetIntrMask/SetIntrMask.
  - 0x800A2604 → `g_intr_i_stat_reg` (0x1F801070).
  - 0x800A260C → `g_intr_dpcr_reg` (0x1F8010F0).
  - 0x800A1510 `g_ings2_vsync_block_ptr` → `g_vsync_gpu_stat_reg` (0x1F801814).
  - 0x800A1514 `g_vsync_counter_ptr` → `g_vsync_rcnt1_count_reg` (0x1F801110, timer 1, not a vblank counter).

  These do not overlap the system.c register-pointer corrections landed today.
- **0x800FF610 `g_gte_vector_template` → RESET (HIGH).** It is the MATRIX output of MulMatrix0
  (= 0x8007E4DC, the very function its comment cites as the consumer of a "vector template").
- **0x8004C388 `satan_helper` → `math_MidpointS16x3U8x2` (HIGH, computation-restatement; v1).**
  Run on raw EXE words: 122k cases, 0 mismatches. The verifier must re-run it with its own harness.
- MEDIUM rows:
  - The DR_MOVE / DR_MODE / TILE packet cursors and buffers (`g_dr_move_cursor`, `g_dr_move_buf`,
    `g_dr_mode_cursor`, `g_tile_cursor`). The element type is pinned by SetDrawMove, SetDrawMode and
    SetTile [VERIFIED]; the open question is whether the "cursor" restatement is acceptable.
  - `calc_NormVector` → `math_NegNormalizeVector`. It is approximate: exact only for |v| ≥ 128.
  - `mem_RelocPtrArray`.
  - 5 further MEDIUM RESETs.

## Owner ruling Q103 (2026-10-03): all four items below adopted as recommended

Items 1 and 2 are accepted classes from this date; 3 and 4 are RESETs. Every row was re-verified
against its class test (default refute) before applying.

**Applied, function wave** (`func_manifest_q103.csv` via `naming_wave.py --from-census --only-file`):
- typed-restatement, 7 RENAMEs: `cdrom_GetFileSize` 0x80036F28, `pad_ClearStateBits` 0x8001BE08,
  `pad_ResetState` 0x800194F4 (item 4), `snd_CloseVab1` 0x8005B6FC, `snd_CloseListedVabs` 0x8005BDF0,
  `snd_VabFakeOpen9` 0x8005BA6C, `snd_VabFakeOpen8And4` 0x8005B98C.
- basis-withdrawn, 17 `cpu_helper_<ADDR>` RESETs. These are the aliases whose parent names were all reset in
  c934e38db.
- Census: INFERRED 512 -> 494, CORROBORATED 119 -> 126, AUTO 405 -> 416.

**Applied, data / member / typedef batch** (`data_manifest_q103.csv` via `data_wave.py`, plus C edits):
- typed-restatement data:
  - 0x80102788 `g_pad_state`, with `_plus_0x2`..`_plus_0x14` for its interior registry aliases. This
    retires `g_pad_input_combined` (that is `.pressed`) and `g_practice_lesson_init_done`.
  - 0x8009AD18 `g_vab_id_list` (retires the contradicted `g_byte_lookup_table_256`).
- Item 3:
  - 0x80101EC8 `g_practice_menu_table` -> `D_80101EC8`.
  - typedef `PracticeMenuRec` -> `Unk80101EC8Record`, the tree's majority address-keyed spelling
    (`Unk<ADDR>Record`, as `Unk80101DF0Record`).
- basis-withdrawn data, 3 aliases spelled only from a reset name:
  - `g_practice_menu_table_p2_plus_6` -> `D_8010231A`;
  - `g_practice_menu_index` -> `D_800A3748`;
  - `g_byte_lookup_table_256_plus_4` -> `D_8009AD1C` (registry line, by hand).
- typed-restatement members:
  - `Unk80101EC8Record.other` / `.index`;
  - Obj80106A78 `owner` / `slot` / `mtx` / `pos` / `prev_pos` / `vel` / `rot` / `rot_vel`;
  - new `CdFileEntry {CdlLOC loc; u32 size;}`. It replaces `CamPair` for `g_cd_file_table` and
    `D_80101E58.rec.pair`; `CdlLOC` is added to `include/libcd.h`.

**Refused on re-verification** (not applied):
- T2, more than 12 statements in the matched C:
  - `memcard_CountFreeBlocks` 0x80037AA4 (16 statements);
  - `memcard_HasFileNamePrefix` 0x80037B00 (about 22 statements).
  Both names describe the bodies correctly.
- T5, noun not on the closed relation-noun list: PadState `type[2]` / `valid[2]` and Obj80106A78
  `kind`. **Applied after the Q105 re-check (2026-10-03).** Each holds in every write and read of
  the matched code. `type`: the InitPAD byte-1 nibble (5/7 folded to 4; 4 when the status byte != 0;
  4 from pad_ResetState / func_80055B60; func_8003A728 can replace it with link-exchange nibbles).
  `valid`: 1 iff the status byte == 0 (and 1, 1 from pad_ResetStateMarkValid); func_80019568 / func_800693CC
  treat 0 as no pad. `kind`: a spawn id, -1 when free, and an index into D_8008E194 / D_8008EB80.
  func_80019568 now uses a local `PadState` and copies the two members by index, so no walk crosses
  a member.
- T5, relation does not hold for every writer. These two refusals are final (re-checked under Q105:
  the facts stand):
  - Obj80106A78 `age`: it counts only while unk_50 != 0.
  - Record `pad` (+0x24): it is not a VERIFIED API argument, and the CPU-side writer's source is the
    func_80055B60 synthesized record.
- `pad_ResetStateMarkValid` 0x80019534: refused after the Q105 re-check on the then-closed T4 list
  ("Mark" was not listed; "Set" means storing arguments, and "Reset ... Valid" reads as clearing the flags).
  Owner ruling Q107 made T4 a principle; applied a154ce3a8 (func_manifest_q107.csv).
- basis-withdrawn, 3 aliases: the parents of `cpu_helper_80026DA4` / `cpu_helper_80029454`
  (`cpu_exec_main_game_loop_frame` 0x8002C61C) and `cpu_helper_8003F3D4`
  (`cpu_init_stage_and_camera_setup` 0x8001E404) are MEDIUM v3 rows that were never reset, so their basis
  stands.

1. **New class `typed-restatement`** (CORROBORATED tier). A name may restate a matched C body
   built only from admitted objects, with a verb that describes the whole effect (T4 as amended by Q107). The class has six tests:
   - T1: the function is matched.
   - T2: at most 12 statements, no `jalr`, and all callees are VERIFIED or CORROBORATED.
   - T3: every touched object is admitted by an accepted row.
   - T4: the name accounts for every store and the return value.
   - T5: each member or data role is pinned by an exhaustive access scan.
   - T6: no game nouns and no claims about purpose.

   Its 28 rows are all tagged NEW-CLASS-PROPOSED. Strongest:
   - 0x80036F28 `cdrom_GetFileSize`. This closes the 09-25 "no class for table reads" rejection.
   - 0x80037AA4 `memcard_CountFreeBlocks`.
   - 0x80037B00 `memcard_HasFileNamePrefix`.
   - 0x8001BE08 `pad_ClearStateBits`.
   - Data `g_pad_state` (0x80102788), whose builder reads the InitPAD [VERIFIED] buffers.
   - The Obj80106A78 members `pos` / `prev_pos` / `vel` / `rot` / `rot_vel` / `mtx` / `slot` / `owner`.
   - PracticeMenuRec `other` / `index`.

   **T4 as amended by owner ruling Q107 (2026-10-03):** the verb must describe the whole effect (every
   store and the return); game nouns still need an accepted class. Examples (the former closed list,
   fixed meanings): Get (return a member, no store), Set (store args), Clear (zero/neutral
   constants to exactly the named members), Reset (the full store list), Copy, Swap, Find (linear
   search returning an entry or NULL), Has (0/1), Count/Sum, Open/Close (a VERIFIED Sony call
   restated); literal ids go in the name. **T5 as amended by owner ruling Q105 (2026-10-03):** a
   member or data name may use any generic, non-game noun whose meaning holds in every write and
   read of the field in the matched code; game nouns still need an accepted class. Examples (the
   former closed list): `other`/`index`/`slot`/`owner`; `pos`/`prev_pos`/`vel`/`rot`/`rot_vel`/`age`;
   `size`/`count`/`list`/`id` (as passed to an admitted callee); a Sony type noun for a VERIFIED
   API argument. C identifiers and comments are never evidence, only the matched operations.
   Each row lists the admitting rows it depends on; if one is later reset, its dependants are
   re-derived.
   Member renames are C edits inside matched files, so they need the manual lane with the oracle and a layer-2 review.
2. **New class `basis-withdrawn`** (v3, one MEDIUM family row; RESET). If a name's only recorded basis is
   another name that gets RESET, it is RESET too. This covers 20 `cpu_helper_<ADDR>` aliases.
3. **`g_practice_menu_table` / `PracticeMenuRec`** (0x80101EC8, MEDIUM RESET). The record holds the
   two fighters' per-frame state, so "menu" is false. The name came from commit 80796420e with no
   evidence; the same batch named `g_isqrt_lut`, since proven wrong. No accepted class yields a
   positive name, because "character" and "fighter" are game nouns. Renaming the typedef is a C-wide
   manual-lane edit.
4. **Conflict at 0x800194F4.** v3 gives RESET `replay_camera_helper` under an accepted class. v4
   gives `pad_ResetState`, which applies only if the new class is admitted. Apply the RESET either
   way.
5. Holds, unchanged:
   - `snd_SeNullCallback` (0x80046954; 09-25 ruling: manual lane).
   - `save_vc_ctrl` (action-overrides).
   - `snd_AllocSe`: the C-definition desync is a respelling job.

## Leads not closed

- LIBSND/LIBSPU holds (func_80087770, func_8008B488, func_80084CC0, func_800858D0) need the PsyQ 4.0
  LIBs. They are gone from `tmp/libscan/`; re-fetch them to set `PSYQ_LIB_DIR`.
- `v2_data/api_args.csv` holds 558 VERIFIED/CORROBORATED call-site arguments, but only the rows above
  were worked. Most D_ arguments are strings, already-reset Sony statics, or library-internal.
- Hygiene seen on the way, not naming:
  - `include/m2c_context.h:686` mistypes 0x8004C388's arguments.
  - func_80019568 has pad-word locals named `voice_mask` / `voice2`.
