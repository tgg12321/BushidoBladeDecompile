# Naming sweep 2026-09-25 — INFERRED-name audit

The follow-up the 2026-09-24 sweep left open: every function name at census tier **INFERRED**
(668 rows, 645 from the old naming-analyzer; "plausible but unreviewed — not defended, just not
contradicted") was audited against the owner's false-positive directive
([[owner-directives]]). Same recipe as 2026-09-24: read-only **miners** per vein, then a
**fresh default-refute verifier** per vein that re-derived every claim from the EXE words /
asm / disc image with its own harness. **Only CONFIRM rows landed.**

Per row a miner chose one verdict:

| Verdict | Meaning | Bar |
|---|---|---|
| RESET | the body **contradicts** the name → `func_<ADDR>` | a concrete contradiction ("unsupported" is KEEP, not RESET) |
| RENAME | an admitted evidence class supports a different name | admitted class, full chain |
| UPGRADE | the current name itself is proven (census tier rises) | admitted class, full chain |
| KEEP | neither | stays INFERRED |

## Veins

| Vein | Rows | HIGH to verifier | Verifier outcome |
|---|---|---|---|
| `gte` | 61 | 5 | 4 CONFIRM, 1 PLAUSIBLE (no name claimed a GTE op its body lacks) |
| `gfx` | 110 | 9 | 7 CONFIRM, 1 PLAUSIBLE, 1 REFUTE |
| `motion_ai` | 132 | 18 | 18 CONFIRM |
| `mode_ui` | 126 | 20 | 19 CONFIRM, 1 PLAUSIBLE |
| `sys_snd_io` | 116 | 46 | 45 CONFIRM, 1 PLAUSIBLE (split in two verifiers: heap cluster + rest) |
| `misc` | 116 | 27 | 23 CONFIRM, 4 PLAUSIBLE |
| `followups` | 7 + held items of 09-24 | 11 | 11 CONFIRM |

Applied: **117 function rows** (80 RESET, 21 RENAME, 16 UPGRADE) + **7 data RESETs** (3 more CONFIRMed data
rows deferred — see Held). The census
went INFERRED 668 → 553, VERIFIED 406 → 412, CORROBORATED 76 → 106, AUTO 304 → 383.

## Files

- `func_manifest.csv` — applied function rows (consumed by `docs/naming/build_census.py`; applied
  with `tools/naming_wave.py --from-census --only-file`). `evidence` = the miner's chain,
  `verifier_note` = the verifier's independent re-derivation (the authoritative record).
- `data_manifest.csv` — applied data rows (`tools/data_wave.py --manifest-csv`).
- `held.csv` — PLAUSIBLE / REFUTE / deferred rows, **not applied**, with the reason.
- `verify/<vein>.csv` — every verifier verdict.
- `keep/<vein>.md` — every KEEP row with its one-line reason; `rejected/<vein>.md` — ideas examined
  and dropped. **Read both before re-mining a vein.**
- `miner-brief.md`, `verifier-brief.md` — the briefs the agents worked from.

## Notable findings

- **The `snd_*` family in `src/sound.c` is a model/data heap, not sound.** Slot 8 ("tiny model";
  the EXE prints "Destruction tiny model." after freeing it) holds MAR model files — all 54
  reachable NDATA files were read from the disc image, none carries VAB/VAG/SEQ magic; slot 10
  likewise. `snd_LoadBgm`/`snd_PlayBgm`/`snd_LoadSe`/`snd_SetVolume`/`snd_CalcFade`… RESET.
  **Correction (sweep 2026-09-25b):** the heap is not sound-free everywhere — slot 6 and the player
  slots hold VAB banks. Only slots 7/8/10 are proven sound-free; the applied RESETs rest on those,
  except `snd_LoadSe` (slot 9), whose removal adds no name but whose stated reason was too broad.
- **The `obj_*` family is libsnd/libspu teardown** (SsVabClose, reverb off, SPU voice init) —
  no object state anywhere; `cpu_init_game_objects` calls only them.
- **Three `mode_handler_NN_NoOp` are 360–544-insn handlers**; several `game_*` / `camera_*` /
  `replay_camera_*` names sit on digit-sprite builders, CD audio, the SIO link handshake and
  memory-card code; five functions named `copy` copy nothing; four named `stub` have bodies.
- **Correction to 2026-09-24:** 0x800526A0 is **not** an exact isqrt (the 09-24 check accepted a
  1% tolerance). It is bit-identical to Sony `SquareRoot0` over all of [0, 2^31), 0..260 below
  floor(sqrt) → `math_SquareRoot0`; `math_Length3D` = SquareRoot0(x²+y²+z²), s16 args.
- `note2pitch`, `_spu_FiDMA`, `_spu_Fr_`, `_spu_2pitch` (VERIFIED) and `SsUtKeyOffV`
  (CORROBORATED) were Sony names stuck at INFERRED only because their XDEFs sat mid-function
  in `libscan/rename_manifest.csv` before the boundary splits (census tier fix, no rename).
- **Refuted by the verifier:** `gpu_enable_and_state_reset` — ResetGraph(1) → Sony `_reset` sets
  DPCR's GPU-DMA enable bit, so "enable" is not contradicted (stays INFERRED).
- **PsyQ 4.0 libraries** (psyz `psyq400.tar.gz`, sha256-verified, gitignored under
  `tmp/libscan/psyq40/`): CDREAD.OBJ carries **no** names for its initialized `.data` statics, so
  the libcd cdread.c statics block stays unnamed (closed, KEEP).
- **Sony `.bss` statics + missed data XDEFs** (`data_manifest_bss.csv`, verify/followups_bss.csv): PsyQ
  OBJs DO record LOCAL names for uninitialized statics. Each module's .bss/.data base was recovered
  from its own HI16/LO16 relocations read back against the EXE (unanimous per module; validated on
  `Alarm`, `n`=g_rand_state and 10 already-named CD_*/GPU_printf XDEFs). 31 CONFIRM applied:
  `patch0`, `column`, `Result`, `regs`, `CombWaitCallback`, `sen`, `ctlbuf` (was `g_gpu_color_table` —
  it is the GP1 control-command shadow `_ctl` writes), 13 `<base>_plus_0xN` members, 6 false-alias
  RESETs inside LIBGPU SYS `p0.87`, and the exported `CD_status1`, `CD_nopen`, `_qlog`, `_qin`, `_qout`.
  The `g_spu_xfer_*` names on the LIBCOMB block were contradicted (serial-link code only).
  LIBMCRD is not linked. The download also carries LIBSN (verbatim at 0x800836EC SNMAIN, OPEN/CLOSE/
  LSEEK/READ/WRITE/SNREAD) — a future libscan pass.

## Held (not applied) — see `held.csv`

- PLAUSIBLE (no admitted class, or not a concrete contradiction): `gpu_SetDrawEnvBg` for
  0x80016768 (DRAWENV struct-field restatement, no library call — the old name was RESET instead,
  verifier-justified), `stage_ApplyLighting` (only calls an empty stub — compiled-out hook?),
  `cdrom_LoadCommonBbm` (callees only CORROBORATED/INFERRED), `rng_Next`/`rng_SetSeed` and
  `scratchpad_Save`/`scratchpad_Restore` (the computation ruling excludes functions touching game
  globals, even pair-private ones), `gte_NormalizeIR` (result is not left in IR; ±32/4096 approx).
- Deferred data RESETs (CONFIRMed, not applied): `g_game_p1_ctrl` (0x800F6656) and `g_snd_volume`
  (0x800A33D0) — the address is declared under two C types in one TU, so collapsing the aliases
  needs a type reconciliation inside matched C bodies (its own oracle proof), not a rename;
  `g_file_heap_base` (0x800A38BC) — data_wave cannot retire one alias while keeping the held
  `g_rng_state` untouched.
- `.bss` held: `Alarm` UPGRADE (a data_wave would retype `extern s32 D_800F19B8` into a clash with
  `extern Alarm_t Alarm` in system.c — drop the 0-use duplicate `g_vsync_timeout_deadline` by hand
  instead); `rec` + 2 members and `n` (one-letter/185-local global spellings) and `p0.87`/`p1.88`
  (dotted GCC static names; psyz spells `p0_dot_87`) await an owner spelling ruling; `CD_cbread`,
  `DS_active` (exact XDEF placement, but no code reference).
- Deferred: 0x80046954 `empty_stub` (src defines it as the unsupported `snd_SeNullCallback` —
  link-map desync to resolve by hand); retiring the false data alias `g_snd_irq_data` on the
  `_spu_FiDMA` function (live C use at src/main.c needs a prototype + oracle run).
- Next batch (same contradictions, not yet verified): `snd_StopBgm`, `snd_SeNullCallback`,
  `snd_StopAll`, `snd_GetMaxFade`, 0x80046934, `game_SndCleanup` (0x80047550), data
  `g_snd_se_id` / `g_snd_bgm_id` / `g_snd_cached_bgm_arg` / `g_snd_play_count` /
  `g_snd_fade_pos` / `g_snd_fade_amt`, `g_file_heap_base_plus_4/_plus_5`, the six MEDIUM
  `gte_mvmva` RESETs (an incidental inline MVMVA is not a contradiction under this bar).

> **Trimmed 2026-10-01:** `keep/`, `rejected/`, `verify/*.md` and the miner/verifier briefs were removed; they resolve at git tag `pre-slim-2026-10-01` (same paths). Read the tagged `rejected/` and `keep/` before re-mining.
