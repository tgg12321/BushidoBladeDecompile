# Q65 adoption (per-file gp model): handoff, 2026-10-01

**State (round 5, 2026-10-01, laneA).** Round 4 review: 09, 14 and all bodies PASS; step 15 FAIL on one item, fixed:
s15 now adds a symbol-file row for every K1 tentative definition that had none (ROW-ADDED: D_800A36DC, D_800A36E0,
D_800A36E4, D_800A3724, all text1b_tu1c; byte-neutral, owner ruling Q62), and the struct alignment model is
cc1's DATA_ALIGNMENT word (4). Regenerated on the same base: only step 15 differs from round 4 (r5diff/15.diff),
bodies identical, all steps oracle, integrity OK; step16 a07ec08df. Round 4 below.

**Round 4 (2026-10-01).** Base `43e73e46b`
(func_80058580 and func_80055B60 landed; no build input changed after it). Scratch clone `/tmp/q56/adopt tree`
frozen with tags step0..step16 (step16 80c6a3ab1). Every step: full build SHA1 == oracle (series_run.txt);
verify-oracle + check_completion_integrity OK at step16; changed completed bodies identical to round 3
(body_hashes.txt). Diff vs the round-3 patches (base 9e4e103aa): q56/adopt/r4diff/NN.diff (pdiff.py, ignores ids
and hunk line numbers): 01/05/07/11/12/13/16 identical; 02/03/04/06/08/10 commit body only (round-3 fixes);
09 dedup 118 -> 120 + line numbers (the landings); 14 body/subject, `const char D_80010AAC[]`, D_80010A2C row
removed, queue-reopen counts; 15 follows main's new declarations (D_800A325C / D_800A3260 `u8[4]` from
func_80058580, D_800A36F2 tentative `u8[2]` from 23581b8fc) and the new STRUCT-INIT path: an initialized
object of a struct type whose members are all scalars / arrays of scalars (layout size == cc1 sizeof, else the
generator stops) is initialized member by member from the original bytes, aligned to its widest member;
here only `PadBitTable D_800A3258 = { { 0xd, 0xf, 0xc, 0xe } };` (func_80055B60's type), disclosed in the
step-15 body. Run from step 15 again after that generator change: resume15.sh. On PASS: apply under the landing
and reintegration locks with the lanes quiet (§ Application plan).

## Regenerating on current main

1. Copy the bank back to its working layout. The scripts hard-code `tmp/q56/`, the scratch clone
   `/tmp/q56/adopt tree` and WSL `/tmp/q56/`:
   `cp -r memory/grind/q65-adoption/q56/. tmp/q56/` (tmp/ is gitignored).
2. In WSL, run `bash tmp/q56/adopt/series.sh <main-commit>`, then `bash tmp/q56/adopt/files.sh`. This:
   - makes a fresh `git clone --shared` scratch clone (never a worktree, never main);
   - applies `sNN_apply.py` one commit per step;
   - for each step: clean full build, SHA1 check, commit + tag `stepNN`, engine test, maspsx unit tests,
     and `NN-<name>.patch`.

   It takes about 12-15 minutes. Every definition, block and blob piece is recomputed from that commit's own
   oracle build.
3. Read the step-15 log `/tmp/q56/s10_log.txt` (PADDING / END-PAD / A6 lines) and re-run the evidence:
   `bash tmp/q56/adopt/post_run.sh` writes `inventory.md` (step14) and `m34_evidence.md` (step07).
4. Apply: see § Application plan below (never by hand-editing src; never with the lanes landing).

## Application plan (round 2; prepared 2026-10-01, not executed)

**Pending landings first, then regenerate.** Any `src/` change on main after the series base (0107288ac)
makes the reviewed patches stale even where they would still apply: step 14's A8 respelling, step 15's
definitions, extern removal and blob cut are generated from the base tree, and steps 14 / 16 edit
`engine/queue.json`, which every `queue done` rewrites (textual conflict). The pending landings:
- func_80058580 (text1b.c; it stays in text1b after step 03's boundary move, which ends text1b at
  func_80060768; func_8005C8A8 is banked, not landing - owner Q77) - affects steps 08/09 (text1b's declarations, the M3 merge), 14 (any new use of
  D_800A33E8/EA, D_800A345C/5E, D_800A350C..12, g_anim_*, D_800A344C/50/54/58 gets respelled; a new
  declaration of one of them with a different type is a compile error the regeneration surfaces) and 15
  (text1b's sdata/static block typing takes each name's first declaration; new gp uses change nothing if
  INCLUDE_ASM already had them).
- func_8002AB08 (code6cac_b_tu2.c) - step 02 moves only func_800343F0; step 15 defines code6cac_b_tu2's
  `.sdata` block (D_800A3140 gp from func_8002AB08, D_800A3144, D_800A314C); a new C declaration's type
  becomes the definition's type.
So: land them, freeze src, then run the series once on that HEAD (`run_all.sh <HEAD>`, ~12 min), `post_run.sh`,
`body_hashes.py`, and compare each new `NN-*.patch` with the reviewed one (diff of the `+`/`-` lines only).
Steps whose content changed beyond context get a targeted re-review (with their new body hashes); unchanged
steps keep their round-2 PASS. Expected: 08 (if the new bodies touch reconciled declarations), 14 (new
respellings), 15 (definitions / externs), 16 (records) - a few lines each, or none.

**Apply (landing lock + `tools/reintegrate_lock.ps1`, lanes quiet, series base == main HEAD):**
1. `git fetch "/tmp/q56/adopt tree" q56-adopt:refs/q65/adopt` (script file; read-only use of the clone).
2. For NN = 01..16: `git cherry-pick stepNN` (a fast-forward-equivalent pick: same base), then
   `git commit --amend --reset-author --no-edit` (author Trenton; runs the commit-msg chain - no new .md, the
   step bodies are the generated sNN_msg texts); `lock.ps1 rebuild laneA` -> SHA1 must be
   62efab4f73f992798c43e8c730aa43baa10bb4fa (stop and revert the pick if not); then the step's
   `layer2 record` calls (below) with `--expect-hash`.
   Step 14's pick carries camera_CalcAngles' `queue reopen` (queue.json + asm/funcs/camera_CalcAngles.s); check
   `queue status` shows it active. Step 16 carries relocate_records' output (func_800770B8 is done, so no queue
   move is expected; state.json / regions / scope lines may move).
3. After step 16: `engine test`, `fixtures-verify`, `check_completion_integrity.py`; `engine verify-oracle
   --rebuild` (the manifest still lists the three deleted lists and the old maspsx fingerprint) ->
   `engine oracle-lock` -> commit `engine: oracle re-lock after the Q65 adoption` (oracle/manifest.json only).
4. `rules:` commit (own layer-2): per-file-gp-model.md:12-13 ("Until the adoption lands ... stay in force")
   -> "Adopted <date> in <step01..step16 commits>; the lists are deleted"; maspsx-gate-lists.md:30 likewise;
   compiler-flags-canonical.md:47 and canonical-asm-authorization-recipe.md:18 replace `sdata_syms.txt` with
   the file's own small-data definition. Then a `docs:` commit for docs/GLOSSARY.md:45 (gp addressing now
   follows each file's definitions). Left as-is: tools/hooks/test_tooling_error_guard.py:141 (a glob
   signature test, no file needed), the exists-guarded legacy tools rename_funcs / apply_kengo_names /
   kengo_globals, and historical docs (handoff-2026-09-30, owner rulings, borderline).
5. Release both locks.

**layer2 record on application** (keys from q56/adopt/body_hashes.txt; reviewer = the round-2 group that
PASSes the step; scope cheat-cleanup unless noted):
- step 06 (group 06/08/10): stage_InitCollision b5c48386649a15d1.
- step 08 (group 06/08/10): func_800460E4 7103e9254aa35494, func_800467B8 8519d94b993f2526, func_8004695C
  6be407fb1b9aaad7, func_800469C4 86c538e9bbad80a3, func_80046BF4 945557971edb0bd4, func_8004700C
  b9d23fd0d90b1398, func_800470B0 d8c649df7378cc1f, func_800475A4 22cadf9e368bb73c, func_800486FC
  91ef82282b393c38, func_80048AD0 c8370ec348919a0c, func_80049E4C db8c2261e847d775, func_80049F4C
  e1c8b97541369c3a, func_80054604 def31064c14fda49, func_80054F68 bfd45f6a0f9cbec9, game_GetPlayerData
  2171330b16b14a5d, game_StageCleanup c2bdf6db0491ecfb. camera_CalcAngles' step-08 body (b75a3c79c1c92db6) is
  not recorded: step 14 returns it to INCLUDE_ASM (Q84) and it is re-queued.
- step 10 (group 06/08/10): func_8006E950 a7c9d42698332ae2, func_80077D00 a8866fe4ff77687b, func_80077D94
  a63f216b21fca356, func_800784E4 3b8c117737402a72, func_80078654 ae1f1a217ee5f11c, func_80078824
  d3d49804b8cfc737.
- step 14 (group 14): func_800420D0 b7fd21173e7e5938, func_8004211C 4b925b3b4e033fcd, func_8004939C
  e5f9816a4bb6f3b5, func_800494D4 f34311e386c27e9e, func_80049584 a382b4582eb01019, func_80049C24
  4b5c268bae334cf6, func_80060A68 15c55ccbc19f6940, func_80060C60 bfef4059a61c599a, func_80063AF0
  4834ae991f4cf373, func_80063B34 702d553c85cccef8, func_80063B78 163ad96e80695b89, func_80063BA4
  132111cb441d4d6e, func_80068F70 80840d5d9a47d3a5, func_8006B578 7341b498b578ccd6, func_8006B92C
  46405721f57b4de3, func_8006D5D4 05bc8f376f4102c9, func_8006DF68 9e7068065078ca45.
- steps 01-05, 07, 09, 11-13, 15, 16: no completed body changes (moves verbatim; body keys unchanged).
If the pre-application regeneration changes any of these keys, the new key and its re-review replace it.

Helpers: `trial_seq.sh <tag> <sNN_apply.py>...` (try steps without committing), `objcompare.sh TAG...` (every
object both ways), `implicit_check.sh`, `g8_test.sh` (the Q69 neutrality proof), `libfiles.py` /
`libfuncs.py` (library classification), `conflict_info.py` / `showfn.py` / `lines.py` (declaration evidence),
`atstep.sh <tag> <cmd>` (read-only command at a tag).

## The steps

| # | Step | Touches (at 64c69153a) | Rule clause relied on | Review |
|---|---|---|---|---|
| 01 | maspsx indexed operand never gp | tools/maspsx/maspsx/__init__.py, tools/maspsx/tests/test_indexed_gp_nop.py, engine/test_engine.py | adoption change 1 (global fix) | layer-2 (substrate) |
| 02 | code6cac_b_tu2/b_tu3 boundary -> func_800343F0 | src/code6cac_b_tu2.c, src/code6cac_b_tu3.c, docs/grind/rodata-align-2026-09-30.md | Split / Cut position, cut outcome (i) | layer-2 |
| 03 | text1b/text1b_tu1c boundary -> func_80060A68 | src/text1b.c, src/text1b_tu1c.c, docs/grind/rodata-align-2026-09-30.md | Merge-bullet boundary move (section 9 window) | layer-2 |
| 04 | text1a_c split -> text1a_c_tu2 (before func_80044800) | bb2.ld, src/text1a_c.c, src/text1a_c_tu2.c | Split, cut outcome (ii) | layer-2 |
| 05 | merge code6cac_b2_pre + replay_camera_rob_back_loose2 + code6cac_b2_post | bb2.ld, the three src files | Merge (Q65 group) | layer-2 |
| 06 | reconcile code6cac_c2/config declarations | src/code6cac_c2.c, src/config.c | Merge: reconcile first | layer-2 incl. **stage_InitCollision** body |
| 07 | merge code6cac_c2 + config | bb2.ld, src/code6cac_c2.c, src/config.c | Merge (Q65 group) | layer-2 |
| 08 | reconcile M3 declarations | src/sound.c, text1a_b.c, text1a_b_pre_rodata.c, text1a_c2.c, text1b.c | Merge: reconcile; (A5) | layer-2 incl. bodies below |
| 09 | merge text1a_c2 + text1a_b + text1a_b_pre_rodata + sound -> text1b.c | bb2.ld + those five src files | Merge (Q67 group), (A7) data-only join, contiguity skip | layer-2 |
| 10 | reconcile M4 declarations | src/text1b_b.c, text1b_tu1d.c, text1b_tu2.c | Merge: reconcile | layer-2 incl. func_8006E950 |
| 11 | merge text1b_tu2 + text1b_b -> text1b_b.c | bb2.ld, src/text1b_b.c, src/text1b_tu2.c | Merge (Q67 group); text1a_b_mid_rodata is empty, so it is not data-only and stays | layer-2 |
| 12 | maspsx `.local`+`.comm` -> `.lcomm` | tools/maspsx/maspsx/__init__.py, tests test_comm_global.py / test_static_lcomm.py, engine/test_engine.py | adoption change 1 (global fix) | layer-2 (substrate) |
| 13 | maspsx -G8 small initialized object -> `.sdata` | tools/maspsx/maspsx/__init__.py, tests/test_small_data_sdata.py, engine/test_engine.py | (A3), Q68 | layer-2 (substrate) |
| 14 | one declaration each for D_800A3468, g_anim_hit_flags | src/text1b_tu1c.c, src/text1a_post.c | (K2): one declaration per static | layer-2 incl. **func_80060A68**, **func_800420D0** |
| 15 | the switch | Makefile, bb2.ld, engine/{buildconfig,buildstamp,oracle,pipeline,queue,test_engine}.py, include/game.h, include/sound.h, named_syms.txt, undefined_syms_auto.txt, sdata_{syms,funcs,exclude}.txt (deleted), asm/data/91C98.data.s + ~20 new asm/data/<off>.data.s pieces, docs/grind/gp-model-2026-09-30.md, tools/psyq_library_files.py (new), src: code6cac_{b2_post,b3,b4,b4_post,b5,b_tu2,c,c0,c2,c_ab,c_mid,tu2}, ings, text1a_{c,c_tu2,post,pre}, text1b, text1b_{b,tu1c,tu1d,tu1e} | E1/E2, (K1)-(K3), (A1), (A2), (A4) library test, gap clause + (A6) | layer-2: every definition against E1/E2/K1-K3 |
| 16 | tooling follows the retired lists; records follow moved functions | tools/{check_root_cleanliness,data_wave,desync_audit,naming_wave}.py, tools/grinder/grindlib.py (+ queue/grind/regions records if relocate_records finds moves) | — | normal |

Rule: `.claude/rules/per-file-gp-model.md`, as committed in 64f71676c (Q65/Q66), 7679f31f4 (A1-A3, Q67/Q68)
and 64c69153a (A4-A7, Q69-Q72). Owner rulings: Q65 (adopt fully), Q66, Q67 (full evidence layout, the four
merge groups), Q68 (maspsx `.sdata` choice), Q69 (-G8 everywhere except Sony library code, defined by the
census span), Q70 (D_80102C00 stays s32; D_800153F0 becomes a 22-halfword struct; func_8004153C stays
unprototyped), Q71 (smallest aligned D_<addr> pieces), Q72 (data-only files join groups). The rulings are in
docs/grind/owner-rulings-2026-09-26.md (thirty-first to thirty-third batches).

## Completed-function body changes (each needs its own layer-2 on application)

- stage_InitCollision (step 06)
- game_StageCleanup, game_GetPlayerData, func_800486FC and func_80049F4C (step 08)
- func_8006E950 (step 10)
- func_80060A68 and func_800420D0 (step 14)
- cast-only call-site edits in sound, text1a_c2, text1b and text1b_b (steps 08 and 10), incl. func_800475A4's
  MulMatrix0 call (step 08, added 2026-10-01). Full list with keys: `q56/adopt/body_hashes.txt` (26 bodies).

What changes in each, and why it is byte-neutral, is in `q56/adopt/PLAN.md` § Body changes. The per-symbol
reconciliation evidence is in the docstrings of `s08_apply.py` and `s10_apply.py`. `layer2 record` keys on the
body hash, so record each verdict on the exact body.

## Known pitfalls

- **mergec glabel bug (fixed).** `mergec.py` drops verbatim-identical re-declarations. It used to compare the
  parser's text, which blanks string literals, so distinct `__asm__("glabel ...")` blocks looked identical
  and were dropped. M4 lost text1b_b's BIOS trampolines and the link failed. It now compares the raw lines.
- **Makefile -G8 expression.** It is `$(if $(filter $1,$(PSYQ_LIBRARY_FILES)),, -G8)`, generated by
  `tools/psyq_library_files.py` from the census span over the link map; `--check` and
  `test_psyq_library_files` keep it in sync. Earlier drafts used an `SDATA_FILES` list (the files with gp
  accesses). That list is gone: do not reintroduce it, and do not drive -G8 from GP_FILES. GP_FILES is
  our cc1's own codegen -G8; it is unrelated and unchanged.
- **The system.c library exception.** Global -G8 moves libcd's 3-byte static Intr into `.sdata`, and the link
  fails ("defined in discarded section .sdata"). Under the A4 library test all of system, gpu, display,
  comb, text1b_b_tu2, text1b_b_tu3, ings2 and main are library (-G0). g8_test.sh shows that every other file
  is byte-identical at -G8 and -G0. text1b_b and main_post straddle the span edges and get -G8; their Sony
  modules may later be split at module boundaries (recorded debt).
- **Rodata files.** text1a_b_pre_rodata.c joins M3 under (A7). text1a_b_mid_rodata.c is empty and stays.
  sound's `.rodata` line in bb2.ld is empty (0 bytes). M3's rodata runs 0x800152B4..0x800158B4 in one piece
  (`m34_evidence.md` § 1).
- **Gap pieces.** 13 alignment-only gaps are not defined. Block ends rely on the linker's SUBALIGN(2), and a
  blob piece after an odd block end starts at the next even address. 20 runs that no single object can
  occupy are split under (A6). Most are code6cac_c_mid's 3-byte odd runs, which are a later aggregate-merge
  candidate (Q71).
- **Header externs of new statics** must be deleted (step 15 does it), or cc1 emits statics in header-first
  order and the layout moves.
- **WSL / hooks.** Commands containing `git checkout`/`commit`/`reset` or `python3 -m engine.cli` are blocked
  inline; put them in script files (`atstep.sh`, `trial_seq.sh`). `/tmp` in Git Bash is not WSL's `/tmp`.
- **After adoption, re-measure func_800770B8** (its distance depends on the gp model).
- **After adoption, Q87 cleanup (owed):** retype D_800A3220 as a RECT and D_800A328C as its 8-byte record,
  folding in step 15's separate D_800A3224 / D_800A3290 (aggregate-merge, own layer-2; decisions.md 2026-10-01
  Q86-Q88).

## Files here

- `q56/` holds the Q56 audit and model work: RESULTS.md, MODEL.md, dead_rows.txt, the ASPSX / PSYLINK /
  cc1psx probe scripts and their results, mergecheck2.py, psyqobj.py, adopt_setup.sh and etest.sh.
- `q56/adopt/` holds the series: series.sh, step.sh, sNN_apply.py, s08_msg.txt, the adoptlib / splitc /
  mergec helpers, psyq_library_files.py, test_psyq_library_files.txt, the check harnesses, PLAN.md,
  inventory.md, m34_evidence.md, cut_windows.md, explicit_exclusions.md, files.md, series_base.txt and the
  patches.
- Binaries (`.obj`), build trees and the scratch clone are not banked.
