# Naming sweep 2026-09-25b — second batch

Follow-up to `docs/naming/sweep-2026-09-25/` (the INFERRED-name audit). Same recipe: read-only
miners (brief: `miner-brief.md`), then a fresh default-refute verifier per vein with its own
harness; **only CONFIRM rows land**. Every data row was additionally checked for C-type conflicts
across each translation unit's header closure (the trap that forced three deferrals in the first
batch) — none found.

| Vein | What | HIGH → verifier | Outcome |
|---|---|---|---|
| `snd_followups` | the rest of the sound.c heap family + out-of-vein leads | 7 func + 41 data | 48 CONFIRM |
| `libsn` | library-scan pass over PsyQ 4.0 LIBSN (never in the committed scan) | 15 | 12 CONFIRM, 3 PLAUSIBLE |
| `medium_revisit` | the first batch's MEDIUM rows, gaps closed or confirmed open | 20 func + 15 data | 30 CONFIRM (+1 REFUTE applied as its RESET fallback), 4 PLAUSIBLE |

Applied: **25 function ops** (21 RESET, 4 RENAME) + **8 tier upgrades** + **57 data ops** + one hand
registry edit. Census: INFERRED 553 → 531, VERIFIED 412 → 418, AUTO 383 → 401.

## Files
`func_manifest.csv`, `data_manifest.csv` (applied rows, evidence + verifier notes), `held.csv`
(not applied, with reasons), `verify/<vein>[_func|_data].csv` (every verdict), `keep/` + `rejected/`
per vein — read before re-mining.

## Findings

- **Correction to the first batch:** the Marionation heap is *not* sound-free. Slot 6 (NDATA 0x83+)
  and the player slots a0+3 (NDATA 0x00–0x1E) hold VAB banks and are freed beside SsVabClose. Only
  slots **7, 8 and 10** are proven sound-free (every reachable NDATA file read from the disc image,
  with positive controls finding pBAV in 0x00–0x24 and 0x83+). Every applied RESET rests on those
  three slots. The first batch's `snd_LoadSe` RESET (slot 9) removed a name without adding one, but
  its stated reason ("the heap is not sound") was too broad.
- **snd family, reset:** `snd_StopBgm`, `snd_StopSelection` (C names on auto glabels — build_census
  now lets a verified RESET retire a LINK-MAP-DESYNC C name), `snd_GetBgmId`, `snd_GetMaxFade`,
  `game_SndCleanup`/`saEft03Start_wrapper`, `marionation_GetFrameOffset` (the EXE table at
  0x800963EC is byte-identical to NDATA.INF), and ~40 `g_snd_*` data aliases: heap-relocated
  pointers into slots 7/8, the wave-mesh grid drawn through RotTransPers3, a transform node, camera
  angles/translation, two slots of the rotation-matrix function table.
- **LIBSN:** `pcdrv_ReadRaw` → `_SN_read` (Sony XDEF; the 2026-09-24 ruling admitted the restatement
  only until SN's symbol was known), `__do_global_dtors`, data `__heapbase`, `_stacksize`, the false
  alias `g_irq_cdrom_initialized` retired (it is the ctor/dtor once-flag). `PCopen`/`PCclose`/
  `PClseek`/`PCread` → VERIFIED; `__main`, `main`, `InitHeap` confirmed. LIBGUN and LIBMCRD are not
  linked.
- **MEDIUM revisit:** `game_StageInit`, `game_Set/GetPlayerCount`, the three pause functions,
  `game_ResetTimer`, `calc_dir_from_points`, `get_global`, two `copy`, `…SetTextureParam`, two
  `replay_camera_helper`, `se_LoadStreamData` reset; `math_Grayscale3`, `gte_SetMatrixRotTransIRVec`
  renamed; `gte_rtpt_batch` upgraded (== RotTransPers3); data `g_game_timer`/`g_round_timer`,
  `g_game_pause`/`_flag`, `g_file_vram_timer`, 7 `g_voice_state_*` (camera offsets),
  `g_game_player_count`, two `g_round_timer_*` (a unit vector) reset.
- `tools/rename_funcs.py` still maps many retired names — deliberately NOT edited: it is a
  provenance input build_census reads, and naming_wave excludes it by design.

## Held — see `held.csv`

- **Owner calls:** `_start` → `__SN_ENTRY_POINT` (`_start` is accurate; the 2026-08-07 style ruling
  only covers stripping a project prefix off a Sony name); `math_Distance3D`/`_16` (approximate —
  the proposed `_Shr2`/`_Shr4` suffix reads as "distance >> 2"; needs a spelling).
- **APPLIED in follow-up (c781d1760 + next commit, `data_manifest_followup.csv`):** the five dlabels were
  split (byte-neutral) and `__heapsize`, `_ramsize`, `CD_cbread`, `DS_active` landed; `_spu_rev_offsetaddr`
  no longer covers `_spu_rev_attr`; `g_vsync_timeout_deadline` and `g_file_heap_base` retired
  (`g_rng_state` untouched). The six SNMAIN `__text`..`__bsslen` words stay unnamed (no code reference).
- **Was blocked on a dlabel split** (identity certain; the Sony symbol sits inside a wider asm dlabel,
  so renaming would make the name cover neighbours): `__heapsize` (0x800A2670, dlabel 0x1C bytes,
  also covers `__text`…`__bsslen`), `_ramsize` (0x800A2690), `CD_cbread` (inside `CD_cbready`),
  `DS_active` (inside `CD_com`). The 2026-09-07 wave already landed one such over-wide name
  (`_spu_rev_offsetaddr` covers `_spu_rev_attr`) — a defect to fix with the same split.
- 0x80042ED8 is an in-place 3×3 transpose — RESET applied; `math_TransposeMatrixInPlace` unverified.
- Next leads: `g_sound_3d_cursor` / `g_sound_3d_data_buffer` (0x800A3820 / 0x80102C00) are the GTE
  renderer's draw list; the six MEDIUM rows that stayed MEDIUM (`keep/medium_revisit.md`).

## Owner ruling 2026-09-25 — applied (see `ruling-2026-09-25.md`)

- `_start` → `__SN_ENTRY_POINT`; 0x80016768 → `gpu_SetDrawEnvBg` (sony-struct-restatement);
  `math_Distance3D`/`_16`, `rng_SetSeed`/`rng_Next`, `scratchpad_Save`/`_Restore` upgraded as-is;
  0x800A38BC → `g_rng_state`; the `rec` block's contradicted `g_spu_xfer_*` aliases RESET
  (`func_manifest_ruling.csv`, `data_manifest_ruling.csv`). `n`, `rec`, `p0.87`, `p1.88` not applied;
  `gte_NormalizeIR` not applied.
- Deferred first-batch data rows landed via the manual lane (layer-2 cheat-reviewer PASS):
  `g_snd_irq_data` retired (SpuStart now passes `_spu_FiDMA`, as Sony's s_ini.c does);
  `g_game_p1_ctrl` → `D_800F6656` and `g_snd_volume` → `D_800A33D0` after their C-type conflicts were
  reconciled (`data_manifest_typefix.csv`).
- `snd_SeNullCallback` (0x80046954): a fresh verifier REFUTED a contradiction RESET
  (`verify/senull_verdict.md`) — the function is heap slot 9's relocation callback, so "NullCallback"
  is accurate; slot 9 is dead code, so "snd_Se" is unsupported, not contradicted. It stays a recorded
  link-map desync (census `empty_stub`, C `snd_SeNullCallback`). Side note: `docs/engine/sound.md`
  describes this as a "sample finished" callback — it is a heap-relocation callback.
