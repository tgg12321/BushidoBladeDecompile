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

## Pending audit work

The legacy map has ~400 entries. The batch-6 refinement touched ~17
of them. The remaining ~380 entries should be audited the same way:

1. For each `func_XXXXXXXX => legacy_name` mapping, read the asm body.
2. Check whether the body's named callees / global accesses align
   with the legacy name.
3. Classify as Adopted / Adopted-as-comment / Held / Rejected.
4. Apply Adopted entries to `named_syms.txt`.

Suggested ordering: by *cluster*. Each file in `rename_funcs.py` has a
contiguous range (sound.c at `0x800467xx..0x800477xx`, ings.c at
`0x800164xx..0x800179xx`, etc). Audit a cluster at a time and document
the result. This avoids whack-a-mole and exposes systematic mis-names.

The map's most reliable section is the "Kengo affinity batch HIGH"
band (`func_80018300..func_8008A928` -- ~40 entries flagged HIGH by
the audit at lines 167-186). These are most likely to pass
cross-validation in bulk.
