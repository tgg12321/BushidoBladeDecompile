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
   - **Next session starts here:** worker 1 — descriptor unification batch (i) (one sheet header / cell /
     Unk8007352CEnv in game.h; 63D2C / 64FD8 / 5ED34; scratch `lt/fdesc/fdesc1.py`), then (ii) 51268 + 3AB48,
     then FZZ. Worker 2 — batch 3 (the per-player model object Unk80045878Obj / Node; g_player_ptrs /
     func_8004153C retype incl. its 2B344 / 3AB48 users; `lt/f08/w2b3.py`), then batch 4 (F05 + P7c +
     368E4 vehicle sites + F11, `lt/f05/w2b4.py`; F11's eight walkers stay as debt rows, B5 trials 15-60).
     Still open: P7b (D_800A38B4 word cursor); the volatile RAM shadow D_800F7298 in _SpuSetAnyVoice
     (borderline.md 544ac7e3a, owner question); Codex backlog `tmp/codex/backlog.md` (gitignored).
   - func_800203B4's island operand (`auth:` re-hash) is still open, after the long tail.
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
