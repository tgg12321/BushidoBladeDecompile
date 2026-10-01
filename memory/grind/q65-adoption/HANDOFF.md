# Q65 adoption (per-file gp model): handoff, 2026-10-01

**State.** The adoption is ready but NOT applied. It is a series of 16 byte-identical commits. The last full
run was on main `64c69153a` (2026-10-01). On that run every step built to the oracle
`62efab4f73f992798c43e8c730aa43baa10bb4fa`, the maspsx unit tests showed only the two baseline failures, and
`engine test` was green at every step whose output survived:
- 1070-1076 passed, 0 failed;
- step 15's test lines were cut off by the log tail, but steps 14 and 16 around it are green;
- re-check step 15 when you regenerate.

The series is generated, never hand-applied. Main moves under the lanes, so always regenerate on the
then-current main. Commit ids per step: `q56/adopt/series_base.txt`. Patches: `q56/adopt/NN-*.patch`, against
64c69153a.

## Regenerating on current main

1. Copy the bank back to its working layout. The scripts hard-code `tmp/q56/`, the scratch clone
   `/tmp/q56/adopt tree` and WSL `/tmp/q56/`:
   `cp -r memory/grind/q65-adoption/q56/. tmp/q56/` (tmp/ is gitignored).
2. In WSL, run `bash tmp/q56/adopt/series.sh <main-commit>`, then `bash tmp/q56/adopt/files.sh`. This:
   - makes a fresh `git clone --shared` scratch clone (never a worktree, never main);
   - applies `sNN_apply.py` one commit per step;
   - for each step: clean full build, SHA1 check, commit + tag `stepNN`, engine test, maspsx unit tests,
     and `NN-<name>.patch`.

   It takes about 1 hour. Every definition, block and blob piece is recomputed from that commit's own
   oracle build.
3. Read the step-15 log `/tmp/q56/s10_log.txt` (PADDING / END-PAD / A6 lines) and re-run the evidence:
   `bash tmp/q56/adopt/post_run.sh` writes `inventory.md` (step14) and `m34_evidence.md` (step07).
4. Apply from the scratch branch: rebase or cherry-pick `step01..step16` onto main. Do this only with the
   lanes quiet and holding `tools/reintegrate_lock.ps1`. After step 16, re-run `relocate_records.py` if
   queue.json moved.

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
- cast-only call-site edits in sound, text1a_c2, text1b and text1b_b (steps 08 and 10)

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

## Files here

- `q56/` holds the Q56 audit and model work: RESULTS.md, MODEL.md, dead_rows.txt, the ASPSX / PSYLINK /
  cc1psx probe scripts and their results, mergecheck2.py, psyqobj.py, adopt_setup.sh and etest.sh.
- `q56/adopt/` holds the series: series.sh, step.sh, sNN_apply.py, s08_msg.txt, the adoptlib / splitc /
  mergec helpers, psyq_library_files.py, test_psyq_library_files.txt, the check harnesses, PLAN.md,
  inventory.md, m34_evidence.md, cut_windows.md, explicit_exclusions.md, files.md, series_base.txt and the
  patches.
- Binaries (`.obj`), build trees and the scratch clone are not banked.
