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
2. **Phase 2 types** — in progress (started 2026-10-03; census, harness and plan:
   `memory/grind/phase2-2026-10-03/README.txt`; gate `tools/check.sh`, commits via `tools/commit.sh`;
   helpers in `tools/game/` and `tools/struct/`). Every commit: SHA1 == oracle, all objects identical
   (or relocation-only for a symbol merge/retirement, stated in the body), INTEGRITY OK, a fresh
   cheat-reviewer PASS and layer-2 records for each moved body.
   - Items 1-5 (Sony prototypes) done, 9475a2b5a..3c25b4a5c: one PsyQ-spelled prototype per reached Sony
     function, audited against real PsyQ 4.5 headers (open-ribbon `include/psyq/`).
   - Item 6 (game declarations) done, edb03ae18..338a31eb8; item 7 (Unk80101EC8Record member access)
     done, d5e5a9976..d73f96546 (record sites ~500 -> 7). Implicit pairs 219 -> 157 over items 1-8.
   - Item 8 (small structs) in progress, 7a7bb6dcd..1d461415f: Rec44 +0x00 is a Vec3i32; MotionFrame
     callee params and s16 channels; scratchpad 16-byte points / func_8002304C Vec4i32 *; the
     Unk8001CD68Rec 4-byte record; merges D_800A3678[3] and g_pad_buf[2][9].
   - **Next session starts here:** s5c (D_800F1140/44/48 -> one Vec4i32; func_80041E10(Vec4i32 *) in
     bb2.h) is prepared and measured but NOT reviewed — `tools/struct/s5c.py`, evidence `s5c.ev.txt`,
     draft message `msg_s5c.draft.txt`; re-take the base, re-run, stage, then fresh review. Then s5d
     (one `extern s32 *D_80103608[]`; `tools/struct/s5d.py`, measured with tc.sh only). tc.sh needs
     bb2.h in tmp/p2/wk/include; headdiag.sh rebuilds tmp/p2/wk from HEAD.
   - After item 8: the deferred OT pointers + AddPrim / ClearOTagR batch (type the primitive holders in
     3AB48 / 51268 / 5ED34 / 64FD8 first); func_800203B4's island operand (`auth:` re-hash).
   - Kept on purpose: original-call facts as commented declarations (func_80019568 / func_80044100 /
     func_80052C10 K&R; snd_VabFakeOpen, func_8005C2A8, func_80054434, func_80060414 local).
     **Owner ruling 2026-10-04:** func_8001C820 keeps its raw `(s32)((u8 *)s0 + 0x536)` argument rather
     than the typed container_of spelling (score 0, judged too hard to read), so func_800325E0 stays
     declared locally in 9F9C. Remaining smaller debt is in each commit body's Hygiene debt row
     (`git log --grep "Hygiene debt" 1a40f4535..`).
   - No "Entity" rename: no evidence (Obj80106A78 and Unk80101EC8Record differ in size, table, consumers).
   - **Scope (owner ruling Q110, 2026-10-04):** Phase 2 stays open until the long tail is typed too — the
     ~3,500 raw-offset casts on objects no header declares yet (`memory/grind/phase2-2026-10-03/casts.tsv`:
     untyped bases, scratchpad workspaces, unknown). Orchestrator plan: evidence-built aggregates, one
     object family per reviewed batch; order after item 8 — primitive holders + the OT batch, then the long
     tail by object (largest families first), then func_800203B4.
3. **Phase 3 naming** — owner ruling Q111 (2026-10-04): naming standards are loosened to the SOTN standard
   (explainable from the code plus an adversarial review); a naming-specific adversarial reviewer may be created.
   Orchestrator plan: step 0 writes the rule text (what the SOTN bar requires, e.g. every read, write and call
   of the named thing bears the name out; how the evidence classes relate), reviewed, in docs/naming/README.md
   and the sweep README, and builds the naming reviewer in `.claude/agents/` if cheat-reviewer's rubric doesn't
   fit; names then land through `tools/naming_wave.py`, and address-named game files take subsystem names file
   by file (`tools/move_tu.py`). Sequencing per the original plan: after Phase 2.
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
