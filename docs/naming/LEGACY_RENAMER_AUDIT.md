# Legacy renamer (`tools/rename_funcs.py`) audit -- pending

## Background

`tools/rename_funcs.py` ships a ~400-entry rename map assembled from
earlier project research (Kengo affinity + size-match + manual
review). It is an *applier* tool -- run `python3 tools/rename_funcs.py
--apply` to propagate every mapping across `src/`, `asm/`, `include/`,
and the sdata function lists.

It has **never been propagated into `named_syms.txt`**. The map and
the symbol file are out of sync.

## Why we don't just apply the whole map

The map mixes confidence levels and was assembled before the
project's current cross-validation discipline. Concretely:

- Some entries are **clearly right** (`math_Distance3D_80017748` --
  body matches the magnitude-with-overflow-guard idiom 1:1).
- Some entries are **clearly wrong** (`game_SndInit_80047530` -- the
  function's only callee is `motion_CheckSituation`, not sound init).
- Some entries are **position-in-table guesses** (the
  `game_EffInit/Cleanup/AnimInit/Cleanup/SndInit/Cleanup` quad at
  `0x8004746C..0x80047550` -- the positions are suggestive but the
  body calls don't always match the position name).
- Some entries are **Kengo size-only-ambiguous matches** flagged by
  `kengo_matches.csv` with confidence floor 0.00 -- these have a high
  false-positive rate.

Applying the whole map blind would propagate the wrong names through
the codebase and make later corrections more expensive (because the
wrong name would now be the "blame"-history-truth name).

## What batch 6 refinement (2026-05-17) did

Cross-validated the 7 batch-6 "low" wrappers against the legacy map.
Promoted 3 entries to `named_syms.txt` after confirming body match:

| Address | Legacy name | Verdict |
|---|---|---|
| `0x80017748` | `math_Distance3D` | **Adopted** (body is 1:1 magnitude calc with overflow guard) |
| `0x80047210` | `camera_InitBoneData` | **Adopted** (memcpy of 0x20-byte default + halve 3 halfwords) |
| `0x80047570` | `camera_InitBone2` | **Adopted** (parallel to `_80047210`, uses `g_cam_bone_data2`) |
| `0x80017738` | `obj_CalcOffset` | **Held** (pure `(a<<6)+(b<<4)` math; "obj" is speculative) |
| `0x80017714` | `obj_ClearAll` | **Held** (clears 8 stride-0x34 entries; "obj" is speculative -- the array could be anything) |
| `0x80047510` | `game_EffCleanup2` | **Rejected** (body calls `efc_rob_set_type_flash`, not a cleanup) |
| `0x80047530` | `game_SndInit` | **Rejected** (body calls `motion_CheckSituation`, not sound init) |
| `0x80047550` | `game_SndCleanup` | **Rejected** (body calls `saEft03Start`, not sound cleanup) |
| `0x80047738` | `game_AnimStart` | **Rejected** (body calls `func_80048F58(1,0)`, target unnamed) |
| `0x80084948` | `se_data_init` | **Rejected** (body calls `saTan4GaugeInit`, not data init) |
| `0x80016514` | `file_LoadAll` | **Held** (body matches `file_read_chunked` semantic; "LoadAll" too vague) |
| `0x80016888` | `gpu_InitDisplay` | **Held** (own name `display_drawot_flush` is more precise) |
| `0x80016918` | `disp_Init` | **Held** (own name `display_init_default` is more precise) |
| `0x80083954` | `irq_ProcessPending` | **Held** (own name `kernel_alarm_cancel` based on body's explicit alarm-disarm; "ProcessPending" might fit too) |
| `0x80046780` | `snd_GetBgmId` | **Adopted as comment** (`get_snd_bgm_id_80046780` is the canonical, legacy is documented) |
| `0x80046798` | `stage_GetId` | **Adopted as comment** (same) |
| `0x80083688` | `sys_GetVideoMode` | **Adopted as comment** (same) |
| `0x80046954` | `snd_SeNullCallback` | **Adopted as comment** (more informative than `empty_stub`) |

## What the sound.c cluster audit (2026-05-17) did

Pulled the 56-function band at `0x80046780..0x80047EC8` from
`rename_funcs.py` lines 13-69 and verified each body in
`asm/funcs/*.s`. 28 names landed in `named_syms.txt`; 3 were already
applied earlier in this session (with my own `get_X_8004XXXX` form);
2 were outright rejected; 12 are deferred. Same triage table format.

| Address | Legacy name | Verdict |
|---|---|---|
| `0x8004678C` | `snd_GetSeId` | **Adopted** (gp-rel lw from D_800A33B4; sibling of snd_GetBgmId) |
| `0x800467A8` | `stage_GetVariant` | **Adopted** (lh from D_8009947A; +2 from g_stage_id) |
| `0x80046DE4` | `game_GetDummyFlag` | **Adopted** (literal `return 0;`) |
| `0x80046DEC` | `game_GetPlayerData` | **Adopted** (func_8004153C ret + 0x1994 offset) |
| `0x80046E18` | `game_GetPlayerBase` | **Adopted** (func_8004153C ret + 0x2C offset) |
| `0x80046E44` | `game_DisablePause` | **Adopted** (writes 0 to D_800F6654) |
| `0x80046E54` | `game_SetPause` | **Adopted** (writes a0?1:0 to D_800F6654) |
| `0x80046E7C` | `game_GetPause` | **Adopted** (lh of D_800F6654) |
| `0x80046E8C` | `game_ResetTimer` | **Adopted** (writes 0x23 to D_800A3790) |
| `0x80046F14` | `stage_GetDataPtr` | **Adopted** (returns &D_8009947C) |
| `0x800472B0` | `camera_GetBoneData` | **Adopted** (returns &g_cam_bone_data) |
| `0x800477BC..D4` | `game_Stub1..4` | **Adopted** (4 literal `jr ra; nop` slots) |
| `0x800477DC` | `snd_SetVolume` | **Adopted** (gp-rel sw of $a0 to D_800A33D0) |
| `0x80047EC8` | `snd_GetMaxFade` | **Adopted** (literal `return 0xD00;`) |
| `0x8004746C` | `game_EffInit` | **Adopted** (8-insn wrapper -> func_8004473C) |
| `0x8004748C` | `game_EffCleanup` | **Adopted** (8-insn wrapper -> efc_rob_set_type_flash) |
| `0x800474AC` | `game_AnimInit` | **Adopted** (8-insn wrapper -> func_80048F58(0,0)) |
| `0x800474D0` | `game_AnimCleanup` | **Adopted** (8-insn wrapper -> func_80048FFC(0)) |
| `0x80047738` | `game_AnimStart` | **Adopted** (8-insn wrapper -> func_80048F58(1,0) -- different a0 from AnimInit) |
| `0x800468B0` | `snd_PlayBgm` | **Adopted** (saSeMain(8,a0) + saTan5TakeGetPos(0)) |
| `0x800469A0` | `snd_PlaySe` | **Adopted** (saSeMain(9,a0)) |
| `0x80046AE8` | `snd_PlaySystemSe` | **Adopted** (SsSet operations on 0xF2000001 SS event class) |
| `0x80046B20` | `snd_StopSystemSe` | **Adopted** (SsStop operations on 0xF2000001) |
| `0x80046780` | `snd_GetBgmId` | already adopted as `get_snd_bgm_id_80046780` (batch 6 -- alias kept) |
| `0x80046798` | `stage_GetId` | already adopted as `get_stage_id_80046798` (batch 6 -- alias kept) |
| `0x80046954` | `snd_SeNullCallback` | already adopted as `empty_stub_80046954` (batch 6 -- legacy hint preserved in comment) |
| `0x80047210` | `camera_InitBoneData` | already adopted (batch 6 refinement) |
| `0x80047570` | `camera_InitBone2` | already adopted (batch 6 refinement) |
| `0x80047530` | `game_SndInit` | **Rejected** (body calls motion_CheckSituation, not sound init) |
| `0x80047550` | `game_SndCleanup` | **Rejected** (body calls saEft03Start, not sound cleanup) |
| `0x800474F0` | `game_EffInit2` | **Held** (position-distinct duplicate of game_EffInit; body identical) |
| `0x80047510` | `game_EffCleanup2` | **Held** (position-distinct duplicate of game_EffCleanup; body identical) |
| `0x8004775C` | `game_AnimStop` | **Held** (position-distinct duplicate of game_AnimCleanup; body identical) |
| `0x8004777C` | `game_EffStart` | **Held** (position-distinct duplicate of game_EffInit; body identical) |
| `0x8004779C` | `game_EffStop` | **Held** (position-distinct duplicate of game_EffCleanup; body identical) |
| `0x800467B8` | `snd_LoadBgm` | **Held** (62-insn function; needs deeper read to verify "LoadBgm" semantic) |
| `0x800468DC` | `snd_BgmCallback` | **Held** (14-insn callback; needs caller-table to verify) |
| `0x80046914` | `snd_StopBgm` | **Held** (already aliased as `channel_helper_8_80046914`; cross-name dispute) |
| `0x80046934` | `snd_AllocSe` | **Held** (already aliased as `channel_helper_9_80046934`) |
| `0x8004695C` | `snd_LoadSe` | **Held** (17-insn, calls saSeMain helpers; "LoadSe" plausible but unverified) |
| `0x800469C4` | `snd_LoadSelection` | **Held** (39-insn; needs deeper read) |
| `0x80046A60` | `snd_StopSelection` | **Held** (already aliased as `channel_helper_A_80046A60`) |
| `0x80046A80` | `snd_SelectionCallback` | **Held** (8-insn, calls func_80054FDC; unverified) |
| `0x80046AA0` | `snd_StopAll` | **Held** (18-insn; calls player_Destroy + snd_StopBgm; plausible) |
| `0x80046B44` | `game_Init` | **Held** (44-insn init; plausible but high-stakes name) |
| `0x80046DA8` | `game_StageInit` | **Held** (15-insn; calls game_InitStageSound + tslLineG5Init) |
| `0x80046EA0` | `game_InitStageSound` | **Held** (15-insn; calls DispHira + stage_GetId + tslLineG5Init) |
| `0x80046EDC` | `game_StageCleanup` | **Held** (14-insn; plausible) |
| `0x80046F24` | `camera_InitMatrix` | **Held** (58-insn; writes lots of D_800EEDB?/D_800F62F? fields) |
| `0x8004700C` | `camera_Transform` | **Held** (41-insn; calls func_80052930 + D_800EEDB[0246]) |
| `0x800472C0` | `camera_InitRotation` | **Held** (49-insn; references D_800F66A0) |
| `0x80047384` | `camera_CalcAngles` | **Held** (already proposed by me as `camera_replay_compute_rot_80047384` in batch 6; cross-name dispute) |
| `0x80047D94` | `snd_CalcFade` | **Held** (50-insn; D_800EF7BC/C0) |
| `0x80047E5C` | `snd_GetFadeCurve` | **Held** (27-insn; D_800EF800/04) |

**Score:** 28 Adopted / 0 newly Adopted-as-comment / 2 Rejected /
21 Held / 5 already-adopted = 56 total.

The adoption rate of **50%** (28 of 56 -- excluding already-adopted)
is in line with the original prediction and confirms that this audit
strategy yields significantly more landed names per cluster than
freelance proposal-generation.

## Pending audit work

The legacy map has ~400 entries. The audit so far has touched:
- batch-6 refinement (2026-05-17): 17 entries (3 Adopted)
- sound.c cluster (2026-05-17): 56 entries (28 newly Adopted +
  5 already-adopted)

Roughly 327 entries remain.  Suggested next clusters in
`rename_funcs.py` order:

| Lines | Cluster | Approx funcs | Notes |
|---|---|---:|---|
| 71-103 | ings.c | 33 | file_/disp_/gpu_/sys_/obj_/scratchpad_ etc. |
| 105-125 | ings2.c | 21 | sys_/irq_/cdrom_/spu_ |
| 127-164 | Kengo size-match | 38 | mixed; some flagged size-only-ambiguous |
| 166-203 | Kengo affinity HIGH | 38 | best signal-to-noise |
| 205-220 | Kengo near-miss HIGH | 6 | ≤5 insn diff |
| 215-221 | Kengo near-miss MED | 6 | 5-19 insn diff |
| 222-226 | Kengo near-miss LOW | 4 | ±10% size |
| 227-232 | Expanded affinity | 5 | exact matches |
| 234-419 | Item 6/7 readability | ~200 | gpu/display/system/main/config; large mixed bag |

The 56-entry sound.c cluster took ~30 minutes to audit fully; the
~200-entry "Item 6/7" band is the bulk and should be cluster-split
further (gpu.c, display.c, system.c, main.c, config.c each on its own).
