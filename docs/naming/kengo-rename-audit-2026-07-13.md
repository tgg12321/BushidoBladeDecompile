# `tools/rename_funcs.py` map safety audit — 2026-07-13

Audit-only pass (READ-ONLY; no renames applied) cross-checking the legacy
renamer's map against the **current** state of `named_syms.txt` +
`symbol_addrs.txt` on `main`. Companion to
[`LEGACY_RENAMER_AUDIT.md`](LEGACY_RENAMER_AUDIT.md), which recorded the
2026-05-17 cluster audits but did so **in a worktree** — this pass verifies
what actually reached `main`.

Method: `tmp/audit_rename_map.py` (in the job scratch dir) parses the
`RENAMES` dict, resolves each `func_XXXXXXXX` to its literal address, and
looks up every symbol at that address in `named_syms.txt` +
`symbol_addrs.txt` and every stem in `asm/funcs/*.s`. Base-name comparison
strips the project's `_<ADDR>` disambiguation suffix, so
`snd_GetSeId_8004678C` counts as the legacy `snd_GetSeId` applied.

## Format finding

The map is a **Python `dict` literal** `RENAMES = { "func_XXXXXXXX":
"semantic_name", ... }` (lines 11–420 of `tools/rename_funcs.py`),
`old_placeholder → new_name`, exactly as the task assumed. **381 entries**
(the "~400" estimate). Section-header comments band the entries by
confidence tier (sound.c / ings.c / ings2.c clusters; Kengo size-match;
Kengo affinity HIGH/MED; Kengo near-miss HIGH/MED/LOW; Item 6/7 readability;
system.c/config.c regfix). Those comments are the primary evidence-tier
signal; `kengo_matches.csv` (`confidence` column: `name-unique` vs
`size-only-ambiguous`) is the parallel per-function tier for the Kengo bands.

## Per-category counts (381 entries)

| Category | Count | Meaning |
|---|---:|---|
| **APPLIED (in named_syms)** | **228** | A symbol whose base name == the proposed name already exists at that address in `named_syms.txt`/`symbol_addrs.txt` (usually as `<name>_<ADDR>`). |
| **APPLIED (glabel / decompiled C, NOT in named_syms)** | **114** | Name is live in the tree via a glabel-renamed `asm/funcs/<name>.s` stub or a decompiled C definition, but has **no** `= 0xADDR;` line in the symbol files. |
| **SUPERSEDED** | **25** | A *different* semantic name is applied at that address; the map's proposal was not adopted (or was rejected). |
| **PENDING** | **14** | Only the `func_XXXXXXXX` placeholder exists at that address — no semantic name anywhere. |
| **STALE** | **0** | — |

**Reconciliation with the task premise:** the map is **~90 % already
resolved** (342 of 381 names are live in the tree one way or another). The
"never propagated into `named_syms.txt`" framing is only half-true: 228 *are*
in `named_syms.txt` today (the 2026-05-17 worktree audit, or equivalent work,
did reach `main` — with `_<ADDR>` suffixes), and another 114 are live via
glabel/C without a `named_syms.txt` line. Only **39 entries** (25 superseded
+ 14 pending) are genuinely unresolved.

## PENDING — the only applicable entries (14)

All 14 are from the **system.c cdrom / config.c** "applied with regfix" band
— **none are Kengo size-only matches**. Each address still carries only its
`func_XXXXXXXX.s` stub with no semantic name. (Verified: e.g. `0x800800DC`
has `asm/funcs/func_800800DC.s`, zero symbol-file entries.)

| Address | old_name | proposed new_name | module |
|---|---|---|---|
| 0x800800DC | func_800800DC | `cdrom_CheckReady` | system.c |
| 0x80080168 | func_80080168 | `cdrom_SetDebugLevel` | system.c |
| 0x800806A4 | func_800806A4 | `cdrom_FramesToBcd` | system.c |
| 0x800807A8 | func_800807A8 | `cdrom_BcdToFrames` | system.c |
| 0x80081718 | func_80081718 | `cdrom_SendCmd` | system.c |
| 0x800817A0 | func_800817A0 | `cdrom_ClearIrq` | system.c |
| 0x80081880 | func_80081880 | `cdrom_ConfigSPU` | system.c |
| 0x80081974 | func_80081974 | `cdrom_Shutdown` | system.c |
| 0x80081D1C | func_80081D1C | `cdrom_DmaToRam` | system.c |
| 0x80081E1C | func_80081E1C | `cdrom_DmaChain` | system.c |
| 0x8003F168 | func_8003F168 | `stage_ExecInitFunc` | config.c |
| 0x8003F218 | func_8003F218 | `game_SetPlayerCount` | config.c |
| 0x8003F274 | func_8003F274 | `stage_InitCollision` | config.c |
| 0x8003F5CC | func_8003F5CC | `stage_ApplyLighting` | config.c |

### SAFE_TO_APPLY vs RISKY split

The task's **RISKY** criterion is "size-only Kengo match (confidence floor
0.00), the #1 false-positive source." Applying that criterion:

- **RISKY (size-only Kengo): 0 pending entries.** Every Kengo size-match /
  affinity entry in the map is *already applied* (see the 38 `Kengo size` +
  44 `HIGH`/`MED` rows under APPLIED, glabel path). The false-positive-prone
  band has no pending work left to guard against.
- **SAFE-tier (readability / PsyQ-idiom, not size-only): all 14.** These are
  the CD-ROM and config wrapper names from the Item-6/7-adjacent "regfix"
  band. That band was adopted at ~100 % in the 2026-05-17 audit after
  spot-checking representatives. They are **not body-verified in this pass**
  — treat as "one-line body spot-check, then apply."

## SUPERSEDED (25) — proposal-engine feedback material

A different name is already applied at these addresses. Three sub-patterns:

**(A) Equivalent synonym / naming-convention variant — applied name is fine,
no action:**

| Address | map proposal | applied name |
|---|---|---|
| 0x80046780 | `snd_GetBgmId` | get_snd_bgm_id_80046780 |
| 0x80046798 | `stage_GetId` | get_stage_id_80046798 |
| 0x80083688 | `sys_GetVideoMode` | get_sys_video_mode_80083688 |
| 0x80046954 | `snd_SeNullCallback` | empty_stub_80046954 |
| 0x8007A898 | `ot_IsEnd` | is_ot_terminator_8007A898 |

**(B) Map proposal is MORE specific than a generic auto-name — candidate
*improvements* (bigger call: replacing an applied name touches blame
history; surface to proposal engine, do not batch-apply here):**

| Address | map proposal | applied auto-name |
|---|---|---|
| 0x8008009C | `cdrom_GetMode` | get_global_8008009C |
| 0x800800AC | `cdrom_GetReadyFlag` | get_global_800800AC |
| 0x80080228 | `cdrom_SetCallbackA` | copy_80080228 |
| 0x80080240 | `cdrom_SetCallbackB` | copy_80080240 |
| 0x8003F1C8 | `game_GetMode` | stub_8003F1C8 |
| 0x8003F1D4 | `game_GetCharData` | stub_8003F1D4 |
| 0x8003F1E4 | `game_SetControllerPorts` | set_global_8003F1E4 |
| 0x8003F268 | `game_GetPlayerCount` | stub_8003F268 |
| 0x8003F52C | `stage_SetCollision` | set_global_8003F52C |
| 0x8003F54C | `stage_GetCollision` | get_global_8003F54C |
| 0x8003F5A8 | `stage_SetLightPosDir` | set_global_8003F5A8 |

**(C) Map proposal is WRONG / disputed — applied name is more accurate
(confirms the map's known false positives; do NOT apply):**

| Address | map proposal | applied name | why the map is wrong |
|---|---|---|---|
| 0x80047530 | `game_SndInit` | motion_CheckSituation_wrapper_80047530 | body calls `motion_CheckSituation`, not sound init (LEGACY audit already REJECTED) |
| 0x80047550 | `game_SndCleanup` | saEft03Start_wrapper_80047550 | body calls `saEft03Start`, not sound cleanup (REJECTED) |
| 0x80046914 | `snd_StopBgm` | channel_helper_8_80046914 | cross-name dispute, HELD in LEGACY audit |
| 0x80046934 | `snd_AllocSe` | channel_helper_9_80046934 | cross-name dispute, HELD |
| 0x80046A60 | `snd_StopSelection` | channel_helper_A_80046A60 | cross-name dispute, HELD |
| 0x8007A87C | `ot_GetTag` | tslSmdSendVu1Code_8007A87C / ptr_load_kseg0 | applied names disagree; unverified |
| 0x8007A5C4 | `gpu_LoadClut256` | suDraw2DLib_8007A5C4 | applied name implies 2D-draw-lib, not a CLUT loader |
| 0x8007A62C | `gpu_LoadClut16` | suDraw2DLib_8007A62C | same |
| 0x800819C4 | `cdrom_Initialize` | tsltm2loadimage_helper_800819C4 | applied name implies texture-load helper, contradicts "cdrom Initialize" |

Group-B/C note: several group-B addresses (`get_global_`/`stub_`/`set_global_`
/`copy_`) and the group-C `channel_helper_*` sit at addresses that also still
carry `katinuki_game_get_katinuki_max_num_*` alias lines — documented
misnomers per `MISNOMERS.md`. The map's cdrom_/game_/stage_ names are
plausibly better than the generic auto-names; the group-C proposals are the
opposite and should be dropped from the map entirely.

## Recommendation for a next-step apply-batch

1. **Apply-batch candidate: the 14 PENDING entries only**, after a one-line
   body spot-check each (they are PsyQ/CD-ROM/config idioms, the band that
   adopted at ~100 %, and none is a size-only Kengo match — the risky class
   is empty). Add them to `named_syms.txt` as `<name>_<ADDR> = 0xADDR;` to
   match the established suffix convention. Keep the cdrom cluster
   **internally consistent**: the 10 pending `cdrom_*` sit interleaved with
   the group-B `cdrom_*` addresses that currently carry `get_global_`/`copy_`
   auto-names, so a clean CD-ROM naming pass should decide the whole cluster
   at once rather than naming half of it.

2. **Do NOT batch-apply the 25 SUPERSEDED.** Group A is a no-op; group B is a
   name-*replacement* decision (blame-history cost) better routed to the
   proposal engine as "map has a more specific name than the auto-generated
   one"; group C are the map's confirmed false positives and should be
   **deleted from `tools/rename_funcs.py`** so a future blind `--apply` can't
   reintroduce `game_SndInit`/`gpu_LoadClut256`/`cdrom_Initialize` over the
   correct names.

3. **The 342 already-applied entries need no work.** The 114 glabel/C-only
   names are optional `named_syms.txt` back-fills (cosmetic — the glabel is
   already the linker-authoritative symbol).

Net: the safe, unambiguous forward move is a **14-entry** apply-batch
(spot-checked), plus a **9-entry deletion** (group C) from the map to stop it
re-proposing known-wrong names.

---

## Correction + apply record (2026-07-13, verification session)

Independent re-verification before the apply-batch (owner directive: re-derive
opus-authored audit claims) found two errors in this report:

1. **The 14 "PENDING" entries were NOT name-less.** Each stub's `glabel`
   already carries the semantic name (e.g. `asm/funcs/func_800800DC.s` opens
   `glabel cdrom_CheckReady`) — applied long ago by commit `2971df5e`
   ("readability: 30 config.c/system.c function renames with regfix"). The
   missing piece was only the `named_syms.txt` registry line. The claim
   "0x800800DC has ... zero symbol-file entries" was true of the symbol FILES
   but the "only the func_XXXXXXXX placeholder exists" framing was wrong.
2. **Group C is 8 entries, not 9.** `0x800819C4 → cdrom_Initialize` is NOT a
   map false positive: the tree's linker-authoritative glabel IS
   `cdrom_Initialize`, and the body is called by `cdrom_CheckReady` between
   `cdrom_Shutdown` and `cdrom_ConfigSPU` (CD-ROM re-init idiom). The
   "applied name" `tsltm2loadimage_helper_800819C4` was a first-pass
   auto-tag; it is now marked MISNAMED in `named_syms.txt`.

Applied: 15 registry lines added to `named_syms.txt` (the 14 + cdrom_Initialize),
unsuffixed to match the existing glabels; bodies spot-checked (cdrom cluster
verified against the g_cd_index_reg/g_cd_irq_reg/g_cd_dma_ctrl register-pointer
block; config.c four verified by call-graph). Deleted: the 8 group-C wrong
entries from `tools/rename_funcs.py` (tombstone comments in place) — deletion
is protective, not cosmetic: `--apply` renames `jal` sites but skips glabels,
so a blind apply of a wrong entry would produce undefined linker symbols.
Group B (11 more-specific-than-auto-name candidates) remains routed to the
proposal engine, unapplied, as recommended.
