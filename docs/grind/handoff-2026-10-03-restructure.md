# Hand-off: padding retirement + Phase 1 restructure (2026-10-03) — closed

Phase 1 is complete. This note holds the current state only. The full plan (owner decisions Q101-Q109
in context, the pad and restructure surveys, per-step status and risks) is in git history:
`git show e75030af3:docs/grind/handoff-2026-10-03-restructure.md` (the last full version; first
version b710e0536); section citations elsewhere (§ 1, Part A, Part C § 3, ...) refer to it. Evidence and scripts: `memory/grind/restructure-2026-10-03/`. Layout and naming
conventions: AGENTS.md "Conventions"; old file names resolve through `tools/tu_renames.tsv` at tag
`pre-restructure-2026-10-03` (6e606ac0f).

## What landed

| Item | Commits |
|---|---|
| Rulings | f0abd598b (Q96-Q103), b710e0536 (Q104-Q106, incl. the D1-D10 restructure decisions), 2148e429c (Q107-Q109) |
| A. Padding retirement (Q101/Q102/Q104): no `PAD_NOPS` or rodata `.word 0` pad left | e5d16f841..d6f8170cf, 191f39673, e38643786, 7398e90f1 (re-lock); record: docs/grind/rodata-align-2026-09-30.md §§ 10-11 |
| B. Q105 naming re-check | 37d769472, 0c813f2d7 |
| C. Restructure step 0 (engine/tus.py, `tus-check`, Makefile recursion, tools/move_tu.py) | 7bdb18ff6..a38713186 |
| Steps 1-3 (pilot moves, 31 game renames, rodata folds) | 2c35cbd7d..01b806bcc |
| Step 4a-4f (library code split into one file per PsyQ module) | 59272fc29..36b6d8412; evidence: rodata-align doc §§ 12-13 |
| Step 5a/5b (include/psxsdk/lib*.h; include/game.h + include/bb2.h) | 0c2f2aa60..d4ae9d5cc |
| D. Q107-Q109 (pad_ResetStateMarkValid; FlushCache / _SendPAD split; sound-library gap cuts) | a154ce3a8, 2cc9d7eb5, a6f20ee2e..6a286cd62, 00ef19715 |
| Step 6 (docs, stale references, D10 segment-dump deletion, near-tier names, re-lock) | c9488fb5b..39de54052 and this note |

Determinism (step 6): five clean `make` builds (serial ×3, `-j8`, `-j16`) and one engine rebuild gave
identical objects; 25 recompiles each of the six largest TUs (incl. main/87A0) were identical. The one
differing main/87A0.o seen in step 5 did not reproduce (scripts: `memory/grind/restructure-2026-10-03/step6/`).

## Open items

1. **`_SendPAD`** — the one active queue item (Q108 split). Every honest C form floors at 4/10 ($v0 vs
   $t1; $ra saved at 0x10 vs 0x14; cc1psx the same); candidates in `memory/grind/_SendPAD/`. Its module
   mates are hand-written asm (`_send_pad`'s trapping `addi`, the func_800790A4 data-as-code), which is
   evidence for the Judge-gated canonical-grant path if pursued; no grant without its own evidence.
2. **Phase 2 types** — make Unk80101EC8Record / Obj80106A78 the "Entity" type in include/game.h; about
   1,238 raw-offset casts; the conflicting declarations (89 Sony, 73 game, notes) in
   `memory/grind/restructure-2026-10-03/step5/phase2_conflicts.tsv`; the `u8 *` / `s32 *` callee
   prototypes. Optional: move bb2.h's single-TU entries back into their TUs.
3. **Phase 3 naming** — the owner decides whether game-code naming moves from evidence classes to
   SOTN-style "explainable from the code + adversarial review"; address-named game files then take
   subsystem names file by file (`tools/move_tu.py`).
4. **Debt** —
   - func_80031B24 hands `&D_800A37E8` to func_800274BC / func_80032854 unlabelled (outside Q96).
   - Q97's struct form is unmeasured; Q96-Q99 retire as soon as a one-object spelling matches.
   - D_8007E08C is not folded into InitGeom.s (tools read a function's address from its .s; e38643786).
   - `queue regen` would list the 10 data-as-code labels (D_800521AC.., g_data_start..) as items.
   - grindlib's self-vet check does not existence-check nested `src/` citations.
   - Comments in src/, inline_asm_canonical.txt row bodies and ledgers cite pre-restructure file names
     (D9: they stay; resolve via tu_renames.tsv).
   - tools/grinder/scope_allow.txt's dormant func_800861BC grant names src/main/psxsdk/libsnd/libsnd_i.h,
     a header class the driver's `add-scope-allow` regex does not cover.
   - 87 game symbols are still declared locally in several TUs (some with conflicting types); hoist the
     identical ones, fix the rest with Phase 2.
   - SpuGetVoiceVolume has no prototype (two implicit calls in src/main/psxsdk/libsnd/ut_vvol.c);
     near_manifest.csv's 0x80085FD8 evidence text predates SsUtGetDetVVol's 3-parameter form.
   - Hygiene rows in the 2026-10-03 commit bodies (`git log --grep="Hygiene debt" --since=2026-10-03`).
