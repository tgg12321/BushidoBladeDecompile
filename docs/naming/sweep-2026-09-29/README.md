# Naming sweep 2026-09-29 — third sweep

Third naming sweep, after `docs/naming/sweep-2026-09-24/` and `sweep-2026-09-25/` + `-25b/`. The first
two mostly removed wrong names. This one looked mainly for **new** names that clear the bar, plus a
re-audit of the INFERRED names and data aliases.

**The owner's bar (restated 2026-09-29): false positives are much more costly than no name at all;
only very-high-confidence names are allowed.** In practice: a name is proposed only under an
admitted evidence class (no new classes this sweep, no PLAUSIBLE holding pen), it claims nothing the
evidence does not prove, and a computation name needs an emulation of the EXE's own words with 0
mismatches over an exhaustive domain or ≥100k random inputs plus every edge case. A miner marked a
row HIGH only if it would "bet the project on it"; everything else stayed MEDIUM and was never applied.

## Recipe

1. **Six read-only miners** (shared brief `miner-brief.md`), one per vein, each writing
   `candidates.csv` + `rejected.md` (+ `keep.md` for the INFERRED vein).
2. **One fresh default-refute verifier per vein** (brief `verifier-brief.md`). The miner's text is
   not evidence: every fact is re-derived from the asm, the EXE words, the PsyQ OBJs and headers,
   with the verifier's own harness. Default verdict REFUTE.
3. **A final cross-vein reviewer** re-read every CONFIRM row against the others (a name one vein
   relies on that another vein resets, shared addresses, apply hazards) and wrote the apply
   checklist. Its verdicts are `verify/final_review.csv`.

| Vein | What | Targets | Candidates (HIGH / MEDIUM) | Verifier CONFIRM |
|---|---|---:|---:|---:|
| `api` | AUTO functions whose bodies call VERIFIED library APIs | 84 | 1 / 1 | 1 |
| `compute` | leaf computations, emulated from EXE words | 127 | 2 / 1 | 2 |
| `struct` | small wrappers / field-offset composites of CORROBORATED callees | 189 | 3 / 0 | 3 |
| `lib` | PsyQ 4.0 library pass: functions and `.data` XDEFs the committed scan missed | 400 | 8 / 9 | 8 |
| `inferred` | re-audit of the INFERRED function names | 526 | 3 / 5 | 3 |
| `data` | data-alias audit over the three registries (+ the 0x800EEDB0 camera-matrix lane) | 3,035 | 46 / 1 | 46 |

Every HIGH row was CONFIRMed (63), and the final reviewer applied all 63; only 0x8008D118's name was
corrected on the way (the verifier's `g_sqrt_table_u8`, not the miner's `g_isqrt_lut`).

## Applied

- **Functions (10)**, `func_manifest.csv`, via `tools/naming_wave.py --from-census --only-file`:
  - 7 new names (tier CORROBORATED): `math_MatrixToAnglesYXZ` (0x80042FA0), `math_TransposeMatrixInPlace`
    (0x80042ED8), `gte_ReadIR1IR2Sra2` (0x80052CD4), `gpu_OffsetTPageClutAt0And4` (0x80043E98),
    `gpu_OffsetTPageClutAt6And2` (0x80043F0C) — computation-restatement; `rcnt_StartCnt1Wrapper`
    (0x800168F8) — api-restatement; `_SsSeqGetEof` (0x80084A7C) — libscan-near.
  - 3 RESET: `camera_InitMatrix` (0x80046F24), `camera_Transform` (0x8004700C), `se_helper` (0x80062020).
- **Data (53)**, `data_manifest.csv` (52 rows, via `tools/data_wave.py --manifest-csv`) +
  `data_manifest_hand.csv` (1 row, a hand registry edit):
  - 7 Sony `.data` XDEFs (libscan-verbatim): `_spu_addrMode`, `_spu_mem_mode`, `_spu_mem_mode_unit`,
    `_spu_voice_centerNote` + `_spu_voice_centerNote_plus_0x2E`, `Hcount`, `ratan_tbl`.
  - 3 proven tables (computation-restatement): `g_sqrt_table` (0x80015620, u16 floor(512·√i)),
    `g_sqrt_table_u8` (0x8008D118, u8 floor(8·√i), i = 0..1023), `g_rsqrt_table` (0x800154A0,
    s16 floor(32768/√(64+i))).
  - `g_scratchpad_save` (0x800F5370) upgraded (pair-private state of scratchpad_Save/Restore); the C
    spelling `D_800F5370` retires.
  - Two link-cable buffer interiors respelled (api-restatement): `g_comb_recv_buf_plus_0x4`,
    `g_comb_send_buf_plus_0x4` (the second replaces the unsupported `g_pad_rec_current_hash`).
  - 40 contradicted aliases RESET to `D_` (39 by data_wave; `g_sound_3d_cursor` at 0x800A3820 by hand,
    because data_wave retires every name at an address and `g_hira_packet_cursor` is not covered).
- Hand edits: stale registry tail comments that restated the retired claims, the `inline_asm_canonical.txt`
  DelDrv prose, forward pointers in the earlier sweep records, `docs/naming/data_evidence/D_8008D118.md`,
  and a `docs/grind/borderline.md` name-drift alias table (`math_TransposeMatrixInPlace = func_80042ED8`,
  `_SsSeqGetEof = func_80084A7C`).

## Findings

- **0x800EEDB0 is not a camera matrix.** func_80046F24 writes `[[0x1000,-a,0],[0,0,0],[0,-b,0x1000]]`
  (row 1 all zero: singular), with a, b built from 0x800F62F8/FA/FC, which are row 0 of a record whose
  rows func_8004A940 loads straight into the GTE light matrix. func_8004700C multiplies a node by that
  matrix and forces `t.y = a2`: a planar projection along the light direction (a planar-shadow
  transform). The caller applies the view step afterwards, as for every node. So `camera_InitMatrix`,
  `camera_Transform`, `g_cam_matrix` (+ m1..m8, `g_camera_matrix_data`) and `g_cam_fov_x/_div/_z`
  are all RESET. The 2026-09-25 KEEP reason (`keep/motion_ai.md`: "0x1000 diagonal + fov-derived
  shear") was wrong on both counts.
- **Three square-root tables, all verified by exact integer recomputation.** 0x8008D118 is a *plain*
  square-root table (`floor(8·√i)`); the old `docs/naming/data_evidence/D_8008D118.md` reading
  ("inverse-sqrt LUT", proposed `g_isqrt_lut`) was false. Its label sits inside DelDrv's whole-body
  inline asm in `src/main.c` (was `g_module_type_tbl`, an 8-byte `.type @function` block), and the
  table continues into the dlabel at 0x8008D120.
- **`_SsSeqGetEof`** answers the 2026-09-24 rejection: PsyQ 4.0 LIBSND MIDIREAD placed at the VERIFIED
  `_SsSeqPlay`; the 145-word `_SsSeqGetEof` chunk has 0 unmasked mismatches at the exact XDEF offset,
  and its 10 masked relocations land on `_ss_score`, VERIFIED symbols or intra-module jumps — the near-tier ruling's "closer build" condition.
- **LIBSPU/LIBETC/LIBGTE `.data`:** section bases recovered from each module's own HI16/LO16 relocs read
  back against the EXE, OBJ `.data` byte-identical to the EXE. `g_spu_state_table_9` was really eight
  separate Sony XDEFs; `g_vsync_counter_snapshot` is `Hcount`; `g_display_lookup_a0928` is `ratan_tbl`.
- **Data aliases that named the wrong thing:** `g_snd_callback` is the SIO device control block passed to
  AddDrv; `g_snd_se_bank` / `g_snd_ch_data` are SetDrawMove VRAM-move parameters and records;
  `g_str_r_*` are SVECTORs loaded with lwc2; `g_sound_3d_*` are the GTE renderer's draw list and OT
  pointer copy; several `*_plus_N` aliases pointed outside their base object.

## Held / deferred — see `held.csv`

- **`g_hira_packet_cursor` (0x800A3820)**: unsupported but not concretely contradicted by this sweep's
  verdict; candidate for the next sweep's RESET audit. Only its sibling alias `g_sound_3d_cursor` was
  retired.
- 12 MEDIUM miner rows (never verified, not applied): `gpu_AddPolyG4_640x240`, `math_PointInTriangleXZ`,
  the `cpu_check_special_move_input` reset, `_SsGetSeqData`, `SsFCALL`, six `_svm_voice_plus_0xN`
  interiors and `_svm_sreg_buf_plus_0x4`.
- Extent follow-ups (manual lane, byte-neutral dlabel work): `_spu_voice_centerNote`'s dlabel covers
  0x2E of the 0x30-byte array (element [23] is its own dlabel; merging needs a C change);
  `ratan_tbl`'s dlabel spans 0x804 bytes vs Sony's 0x802 (2 bytes of pad); `g_sqrt_table_u8`'s label
  is an 8-byte function-typed block inside DelDrv.
- Grind ledgers under `memory/grind/` still spell `D_8008D118` / `D_800A28A4` in banked candidates
  (history, per the naming-wave doctrine); a candidate landed after this sweep must use
  `g_sqrt_table_u8` / `_spu_voice_centerNote`.

## Files

`func_manifest.csv`, `data_manifest.csv`, `data_manifest_hand.csv` (verdict `CONFIRM-HAND-ONLY`, so
data_wave never consumes it), `held.csv`, `verify/<vein>.csv` (every verifier verdict; `data_cam.csv` is
the camera-matrix lane; `final_review.csv` is the cross-vein review), `rejected/<vein>.md`,
`keep/inferred.md`, `miner-brief.md`, `verifier-brief.md`. Read `rejected/` and `keep/` before re-mining.
