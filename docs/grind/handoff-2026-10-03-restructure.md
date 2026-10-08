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

1. **`_SendPAD`** — done: COMPLETED-INLINE-ASM-CANONICAL by owner ruling Q112 (rules 7b2e1706b, auth 2a3b1e6e4;
   PsyQ 4.0's own object names the module source `sendpad.s`). The queue is empty.
2. **Phase 2 types** — closed 2026-10-06 (started 2026-10-03; census, harness and plan:
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
   - Item 8 done (s5c 211bbda71, s5d 486221d00). Item 3 of the plan (primitive holders, OT, AddPrim /
     SetDraw*) done fd40277d3..a47145798: P1 setlen, P2 AddPrim def, P3 pool / context records
     (31bb65e13), P4-P6 holder flips, D1-D8 3AB48 chunk-builder records, P7a, P8 libgpu.h + bb2.h.
     Rulings taken this session are in those commit bodies: g_gpu_ot_ptr is u8 *; 51268's layout-A
     context is a word array (value conversions at GPU calls); 3AB48 declares `extern int AddPrim();`
     (implicit-int evidence), so AddPrim stays out of libgpu.h; func_80060768 keeps int arithmetic
     (member TILE stores 192 -> 191 insns); a cast to a callee's declared parameter type is a boundary
     conversion; asm text / operands are never edited and no alias local exists to keep them compiling.
   - Owner questions (1) / (2) closed by Q113 / Q114 (rules 6dd0496dd): func_8006C21C passes `(RECT *)tw`
     (f3f430cf9); AddPrim stays out of libgpu.h.
   - Corrections to committed messages: 31bb65e13 (P3) and e6a30adb9 (P5) state "insns" counts that
     included objdump relocation lines (e.g. P5's 650 -> 641 is 552 -> 543; full table
     `memory/grind/phase2-2026-10-03/lt/insn_audit.txt`); verdicts unchanged. dbf53f73d (D1) says the
     caller passes s32; it passes the u32 D_800A38B4.
   - Deferred: P7b (D_800A38B4 as a u32 * word cursor; IDENTICAL in 5 TUs, two callers over-advance by
     ret words) and P7c (368E4 g_gpu_ot256_ptr) wait for the long-tail cleanups they pull in; plan and
     evidence in `memory/grind/phase2-2026-10-03/lt/item3_plan2.txt`.
   - Long tail (plan item 4) in progress: census `lt/families.tsv` / `lt/plan.txt` (3303 sites, 20
     families + 744 scattered). F02 (17AFC collision scratchpad, Unk1F8002B8Rec in ScrPad.unk2B8):
     batch 1 ef5686d36, batch 2 9ccd8e382, batch 3 3351da420 (unk00 = union { raw; LeafPos
     unk00[2] }), batch 4 557ce2b3b (ScrPad.unk2B8 = union { rec; v8005344C }; Work_80053E9C in
     game.h).
   - F02 batch 5 6c37fa778: func_8002AB08 typed; unk00's point array is `LeafPos unk00[6]`.
   - F02 batch 6 a564052cd: func_800290B8 / func_80029454 (+ box_overlap) / func_8002C22C /
     func_8002C61C typed. F02 is done except the raw-offset asm-operand bodies (func_8002CD58 /
     D518 / D780 / DAD0 / FC80 / FDB0): typing them edits GTE asm operand expressions (hashed in
     tools/canonical_asm_regions.json), which lt/f02/plan.txt marks RULING NEEDED.
   - Long-tail progress 2026-10-05/06 (two workers; owner chose bigger batches and a second worker in its
     own worktree, then stepped away): F01 done (b9b07dd27, 643264f38, ec8c6dcd2, d8c591f23, 63ca60f46);
     F03 + F19 (71073622e, 110ccc84e); F13 / F14 / F20 settings-record merge (7797854fe); F04 / F15 / F07 /
     F17 + 5ED34 work block + 2B344 groups (ac9d98537); 3AB48 VAB pack / stage header (4290c05ec); worker 2:
     F06 + F12 (7712eeca3), F09 + F18 (662320305). Debt rows are in each commit body.
   - **Process (owner 2026-10-06):** one shared checklist `memory/grind/phase2-2026-10-03/lt/checklist.txt` —
     the worker self-checks and the reviewer judges the same BLOCKING items B1-B8 (cheats + regressions);
     comment / message errors are non-blocking fixes the orchestrator applies. Short messages, no line cites.
     Agents report once via SendMessage and end their turn with one line (idle notices repeat final text).
   - **Two lanes.** Worker 1 (main checkout; build lock `tmp/orch/lock.ps1`): 51268 / 5ED34 / 64FD8 / 63D2C /
     3AB48 / 25788 / 2B344 / game.h, then FZZ in 87A0 / 6CF8 / 760D0 / 32D04 / cdrom / memcard / pad. Worker 2
     (worktree `C:/Users/Trenton/Desktop/bb2-worktrees/p2-w2`, branch p2/w2; commits per batch on the
     branch, reviewed in a throwaway worktree, landed by the orchestrator with `git cherry-pick -n` +
     verify-oracle + layer2 record + tools/commit.sh when main's index is clean): libspu / libgpu / 9F9C /
     17AFC / 28708 / 309CC / 31548 / 31D3C / 35000 / 368E4 / 31CFC and the libraries.
   - **Long tail CLOSED 2026-10-06** (609a9a09a): `lt/plan.txt` is the current-state record; census 685 sites
     (2026-10-05: 3303): ruled 180, debt 246, owner-blocked 196, boundary 63, open 0. Session landings
     8fc462a1c..b9a504cd0 (fimg1, w2 b5 / P7b / b7-b12, fsmall + engine f2b56796b, fdecl1, fot1 = g_gpu_ot_ptr
     u32 * reversing P5, fres1 / fres2). Tooling: fdfb27257 (data-as-code needs included-asm evidence),
     97dd7cc7e (close-ledger normpath).
   - **Owner rulings Q115-Q117 (2026-10-06, rules a6939b16f), all implemented:** typed GTE operand expressions
     (87A0 43a813168; 17AFC CD58 / DAD0 / D780 6151b1808; func_80067D14 / func_800203B4 / func_80027AD8 21a094d90);
     _SpuSetAnyVoice reads `_spu_RQ[reg - 0xC4]` (D_800F7298 was a phantom base; 643796e7f); Q96 covers
     func_80031B24 (6151b1808). Phase 2 has no owner-blocked item left; what remains is ruled or debt rows.
   - Process: never export GIT_DIR / GIT_WORK_TREE in a shell that runs tests (2026-10-06 the grinder tests
     rewrote main's .git/config).
   - func_800203B4's island grant: closed (21a094d90 typed the operands; its region grant verifies).
   - Pre-Phase-3 cleanup (2026-10-06): C style (`.clang-format`, `tools/format.py`, format guard);
     asm-region grants hash tokens (schema 2); source comments slimmed to labels + short descriptions.
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
3. **Phase 3 naming** — in progress. Rule: `.claude/rules/naming-bar.md` (Q111; step-0 answers 2026-10-07: the
   SOTN bar with advisory evidence classes; identifier-only key moves certified by `tools/naming_keycheck.py` +
   the reviewer's R6; N6 file names only for single-subsystem files). Reviewer: `.claude/agents/naming-reviewer.md`.
   Census origin `sotn-review` (CORROBORATED) from `docs/naming/phase3/<wave>/func_manifest*.csv`.
   - Landed: wave01 c560d4f52 (+ prose 12d46a075); wave02 02a604835 (camera_GetBoneData, g_cam_bone_data*, the
     0x800A38B8 counter, 145 `*_helper` tags; + prose 43fbce957); wave03 0cf2ccc20 (132 resets incl. 54 gte_*, 45
     motion_*/draw_*, the 6CF8 / 368E4 analyzer names; eff_ClearInitFlag, snd_InitAndLoadCommonVab, g_listener_cam;
     KEEPs disp_Init / sys_Init / camera_CalcAngles; + prose 741468925, rule cite 8c605bbef); wave04 193a2e6d0 (26
     resets: 14 mode_handler_NN_*, the six held 2B344.c stage_* / game_GetCharData, six efc_*; + prose 0680e9c48).
     Each: SHA1 == oracle, keycheck OK, fresh pre-apply reviews, post-apply R6 PASS. Scripts: tmp/p3w2..p3w4/
     (apply.sh, gates.sh; wave04 had no data rows).
   - **Next: wave05.** RESET candidate from wave04's R6: stage_SetCollisionAtPos_8003F420 (alias; its basis was a
     call to the retired stage_SetCollision). Re-mine:
     game_StageCleanup 0x80046EDC (cited contradiction was false), 0x800421C8 (needs a prefixed game-style name;
     SetLightingAndBgColor FAILed), sys_GameInit KEEP ("sys" unexplained), 16 efc_spawn + 6 mode_handler_* keep
     candidates (tmp/codex/c6-efc-mode-result.md), draw_bg_and_sprites, stage_ClearLighting (glabel
     efc_buki_get_vertex_color), g_stage_* data, D_800A377B alias. Neutral by mining: the three D_800A3834 writers,
     func_80046BF4. Remaining INFERRED by subsystem: game_, get_/set_, mario_, ... (census tier INFERRED).
   - Wave mechanics learned: verify-oracle --rebuild needs --allow-dirty on the applied tree; a FAILed row is dropped
     or re-mined (a fresh reviewer per round); PASS-row evidence fixes from a reviewer are applied before commit.
   - Prose debt (follow-up commits): docs/engine/*.md cite retired names (list in wave03's R6, commit 741468925 body);
     locals named after reset claims (17AFC.c playerData, 368E4.c player); ledger stubs memory/grind/_completed/*.json
     keep old func names (naming_wave moves only layer2.jsonl: tool-scope question).
   - Codex lane (`tmp/codex/backlog.md`): C1 3353d059f, C3 212fb353c, C7 43f758f31 (naming_wave own-file P4)
     landed; C2 / C4 / C6 research done; C5 (keycheck per-scope pairs, codex/keycheck-scoped-pairs) in review
     round 4; when it lands, update naming-bar.md:85 / :118 (members / locals limits). The stale May
     ~/.codex/rules/default.rules (allow rules incl. `make` in main) was moved aside; read-only questions
     via `codex_worker.py research`.
   - Codex scope (owner 2026-10-07): keep Codex out of the main working tree; containment against a rogue
     agent is not a goal. The isolated CODEX_HOME and the machine guard were withdrawn; read-only questions
     run in `codex_worker.py research` snapshots, write tasks in codex-worker worktrees.
   - Debt: wire tools/check_retired_names.py into the integrity audit / hooks; bank the keycheck probe suites
     (`tmp/p3s0/kc_test.sh`, `tmp/kcrev*/`) as engine tests; keycheck hygiene (same_tokens belt, clang-format
     version check, report against the formatted text); comment nits (25C38 "these files", the Makefile
     lb/lh section header); stale registry comments.
   - File split audit (2026-10-07, scripts `tmp/p3split/`, not kept): every byte-proven boundary is a cut; 9
     game-TU starts have no evidence either way (87A0, 175A4, 25C38, memcard, 28514, 28708, 2B344, 31CFC,
     31D3C) and no merge is provable; the large files hold several original TUs whose cuts the bytes cannot
     place (data order narrows but never places one). Done from it: 25C38's boundary relabelled LEGACY. Not
     done: folding d_15EC, because its path pointers (0x800955E0) have no code reader, so 32D04 and 35000 are
     equal owners (cheat-reviewer FAIL of the fold, 2026-10-07). **Deferred by the owner (2026-10-07):** 3AB48 stays one file; revisit after
     naming, when logical domains are known. The question: is its hand-written asm block its own module(s)?
     ASPSX probe (2026-10-07, real PsyQ 3.5 / 4.0 tools; scripts `tmp/p3probe/`, not kept): no Sony tool pads
     a module; only a source `.align` does, relative to the object's start, and file-scope `.align 4` in a C
     file gives the same bytes, so pads never prove an object. Sony's asm libraries rounded modules to 16 by
     habit. The pad ends (0x8004C404, 0x8004E564, 0x80052720, 0x80052D00) cannot all come from one object at
     0x8004A348, so the current file implies filler nops. Read as alignment pads, they give objects
     0x8004C1F4.., 0x8004C404..0x8004E564, one unplaceable start in 0x8004E564..0x80052720, and
     0x80052720..0x80052D00 (two starts land exactly on the previous padded end, ~1/16 by chance), plus the
     rodata-forced C|C cut before func_80058580. The bytes cannot decide between the two readings.
   - Debt: `EXPAND_LB_FILES` (Makefile) is a byte no-op, since every file already gets `--expand-lb`.
   - Debt: the retired-name comment scan (1035 names, 0 hits on 2026-10-07) and the 18-case naming_keycheck
     probe suite were scratch (`tmp/p3s0/`); neither is an engine test. Each wave's keycheck covers only its
     own pairs.
4. **Debt** —
   - Q96-Q99 retire as soon as a one-object spelling matches. Measured 2026-10-06 (Codex report, base
     71073622e; none retires): Q97 array / struct / union forms score func_8003B2C8 12, func_8003B328 16;
     Q96 aggregate forms func_80027AD8 2 (alias-cluster removal 83 / 6); Q98 direct element 9 / 8, cluster
     17; Q99 direct members 2, typed state pointer 67, member pointers 6.
   - Comments in src/, inline_asm_canonical.txt row bodies and ledgers cite pre-restructure file names
     (D9: they stay; resolve via tu_renames.tsv).
   - Game symbols still declared locally in several TUs (Phase 2 hoisted the identical ones, 84373e6f8).
   - Hygiene rows in the 2026-10-03 commit bodies (`git log --grep="Hygiene debt" --since=2026-10-03`).
   - Resolved since: D_8007E08C fold + SpuGetVoiceVolume prototype (54e21bed1); data-as-code labels
     (d51106fd8, fdfb27257); self-vet nested citations (adb5c8ba8); internal-header scope grants
     (817e8d366); near_manifest SsUtGetDetVVol (0efaafd19); func_80031B24's &D_800A37E8 (Q117, 6151b1808).
