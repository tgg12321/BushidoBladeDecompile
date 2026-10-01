# Naming sweep 2026-09-24

A naming pass over the still-unnamed functions and data symbols, run under the owner's
false-positive directive ([[owner-directives]]: a wrong name costs more than an auto
name). Every applied row was **mined** by one agent and **re-derived by a fresh
default-refute verifier** that treated the miner's evidence as a claim; only CONFIRM rows
landed (PLAUSIBLE rows stay unapplied, REFUTE rows either drop or land under the verifier's
own corrected name/RESET).

## Veins

| Vein | What it mined | Evidence class | Miner brief |
|---|---|---|---|
| `func_api` | AUTO/INFERRED functions whose body is VERIFIED library calls or a literal string | `api-restatement`, `in-binary-string` | `miner-brief.md` |
| `compute` | pure leaf functions (math, bit, GPU-primitive field arithmetic, GTE register moves) + SN PCdrv traps | `computation-restatement`, `libsn-pcdrv-protocol` (admitted by the owner this day — `ruling-2026-09-24.md`) | same |
| `data_api` | `D_` / `g_` globals whose role a VERIFIED API pins (DRAWENV/DISPENV/OT, CdlLOC/CdlFILE table, pad buffers, event handles, VAB tables) | `api-restatement` for data (the data-side analogue; member aliases `<base>_plus_0xN`) | same |
| `lib_followups` | open items of the 2026-08/09 Sony-library waves: `_svm_*` fields, `_ctype_`, `_spu_rev_param`, `__main`, Sony module statics, false aliases on library code | `libscan-xref` / `libscan-near` / Sony-static, `crt0-convention` | same |

## Files

- `func_manifest.csv` — function rows (consumed by `docs/naming/build_census.py`; applied with
  `tools/naming_wave.py --from-census`). `reset-contradicted` rows emit RESET while the
  contradicted name is still live.
- `data_manifest.csv` — data rows + the 8 Sony module-local statics that were still spelled
  `D_<addr>` (applied with `tools/data_wave.py --manifest-csv`).
- `verify/<vein>.csv` — the verifier's per-row verdict and note (the authoritative record).
- `rejected/<vein>.md` — every candidate examined and dropped, with the reason — read these
  before re-mining, so nobody re-proposes a rejected name blind.
- `ruling-2026-09-24.md` — the owner ruling admitting the two new classes.

## Notable corrections (names the evidence contradicted)

- `gpu_EnableDisplay` = `ResetGraph(1)` only; `gpu_DisableDisplay` = `SetDispMask(1)`, which
  turns the display ON. Both kept by a 2026-08-07 override as assumed corrections; now
  `gpu_ResetGraphMode1` / `gpu_SetDispMaskOn`.
- `SpecialCam` (0x8008EC34) is the disc file table: all 159 {CdlLOC, size} entries match the
  ISO9660 directory of the disc image → `g_cd_file_table`.
- `snd_PlaySystemSe`/`snd_StopSystemSe` start/read root counter 1; `buki_collision_alloc` is
  libsnd key-on bookkeeping; `replay_camera_Init`/`game_FrameInit` are CD read/pause;
  `bios_FileRead*`/`syscall_wrapper_break` are SN PCdrv host-file traps;
  `g_voice_packet_base_*` is the InitPAD buffer; `MarioCam_str` is the SpuInitMalloc table.
- `math_RotMatrixXYZ` was REFUTED for 0x80023C30 (it builds Rz·Ry·Rx) → `math_RotMatrixZYXAngles`.

## Held (not applied) — follow-ups

- `math_Length3D` (0x80052720): behaviour proven (callee 0x800526A0 emulated = exact isqrt),
  but it rests on `math_sqrt`, which is only analyzer-CORROBORATED; alias `stub` was RESET.
- `snd_FlushKeyOnTable` (0x8005C6D0): old alias RESET; the name itself PLAUSIBLE.
- `_svm_orev1/_svm_orev2` (0x800F1B14/0x800F2B68): Sony-source body order only, no relocation
  — false aliases RESET, Sony names held.
- libcd cdread.c statics block 0x800A14D0..: `g_CdRead_state` REFUTED (struct boundary is a
  SOTN modelling choice). Sony's own names should be recoverable from CDREAD.OBJ LOCAL
  records — `tools/libscan/manifest.py` keeps only text-section locals; needs PSYQ_LIB_DIR.
- `puts` (0x80082000) is 263 insns in splat: its tail holds `cb_read`/`cb_data` (named here
  on their C definitions); the `puts.s` boundary split is separate work.
- The census still tiers `note2pitch`, `_spu_FiDMA`, `_spu_Fr_`, `_spu_2pitch` INFERRED
  although they are Sony names; `named_syms.txt` defines `g_char_class_table` twice.
- The INFERRED obj_InitAll / obj_InitPair / obj_InitTask look like "SsVabClose + clear VAB
  tables" routines (data verifier) — candidates for an INFERRED-name audit.

> **Trimmed 2026-10-01:** `keep/`, `rejected/`, `verify/*.md` and the miner/verifier briefs were removed; they resolve at git tag `pre-slim-2026-10-01` (same paths). Read the tagged `rejected/` and `keep/` before re-mining.
