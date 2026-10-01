# Q65 adoption — the per-file gp model as a series of byte-identical commits (scratch only)

Regenerate on the then-current main: `bash tmp/q56/adopt/series.sh <main-commit>` (WSL). It makes a scratch
`git clone --shared` at `/tmp/q56/adopt tree` (never a worktree, never main), applies `sNN_apply.py` one per
commit, and for each step: clean full build, exe SHA1 == `62efab4f73f992798c43e8c730aa43baa10bb4fa`, commit +
tag `stepNN`, `engine test`, maspsx unit tests (baseline failures only: `test_div_expand_li_nop`,
`test_expand_li_0x1`), and `NN-<name>.patch`. Every definition, block and blob piece is recomputed from that
commit's own oracle build. Per-step touched files: `files.md` (written by `files.sh` after a run). Last full
run: see `series_base.txt`.

Evidence files (all in `tmp/q56/adopt/` unless noted): `m34_evidence.md` (M3/M4 merge test, section-7 style:
link order, gp reach per member, mergecheck2, jump-table phases, PSYLINK probe), `cut_windows.md` (step 2/3
windows), `explicit_exclusions.md` (canonical-asm `%hi/%lo` accesses excluded from E1/E2), `inventory.md`
(dated small-data inventory: every block object and gap, with the rule note), `../aspsx_hilo_results.txt`
(ASPSX leaves explicit `%hi/%lo` as written), `../psylink_probe2_results.txt` (PSYLINK per-file `.lcomm`
blocks, >8-byte statics in `.bss`).

## The series

| # | Step | What |
|---|---|---|
| 01 | maspsx-indexed-operand-gp | `_uses_gp`: an indexed `sym($reg)` operand is never gp (global fix, in rule). |
| 02 | b_tu3-boundary-move | code6cac_b_tu2 / b_tu3 boundary to func_800343F0 (cut outcome (i); `cut_windows.md`). |
| 03 | text1b_tu1c-boundary-move | text1b / text1b_tu1c boundary to func_80060A68 (Merge-bullet move inside the section-9 window; record updated). |
| 04 | text1a_c-split | text1a_c split before func_80044800 into text1a_c_tu2 (cut outcome (ii)). |
| 05 | merge-code6cac_b2 | code6cac_b2_pre + replay_camera_rob_back_loose2 + code6cac_b2_post (Q65 group). |
| 06 | reconcile-c2-config-decls | one type for func_8004153C / D_800A3708 before M2. |
| 07 | merge-code6cac_c2-config | code6cac_c2 + config (Q65 group). |
| 08 | reconcile-text1b-decls | M3 declarations reconciled by evidence (rationale per symbol in `s08_apply.py`). |
| 09 | merge-text1b | text1a_c2 + text1a_b + text1a_b_pre_rodata + sound + text1b (Q67 group; pre_rodata is an owner question). |
| 10 | reconcile-text1b_b-decls | M4 declarations reconciled by evidence (`s10_apply.py`). |
| 11 | merge-text1b_b | text1b_tu2 + text1b_b (Q67 group; the empty text1a_b_mid_rodata.c stays). |
| 12 | maspsx-static-lcomm | `.local`+`.comm` modelled as `.lcomm` (global fix, in rule). |
| 13 | maspsx-small-data-sdata | under -G8, an initialized object of 8 bytes or less goes to `.sdata` (A3 / Q68). |
| 14 | reconcile-static-decls | one declaration each for D_800A3468 (text1b_tu1c) and g_anim_hit_flags (text1a_post). |
| 15 | per-file-gp-switch | definitions (K1/K2/K3, A1/A2), gap clause, blob cut, maspsx -G8 per file, lists retired. |
| 16 | tooling-records | tools drop the retired lists; records follow every moved function (`relocate_records.py`). |

Every src move (02-05, 07, 09, 11) is verbatim (`splitc.py` / `mergec.py`; mergec drops only verbatim-identical
re-declarations, compared on the raw text — the 2026-09-30 fix: string literals had been blanked, so distinct
`__asm__("glabel ...")` blocks looked identical and were dropped).

## Body changes in completed functions (each gets its own layer-2 at application)

Each is byte-neutral: the step's full build is the oracle and every object is compared (`objcompare.sh`).

- **stage_InitCollision (config.c, step 06).** `*(s32 *)(D_800A3708 + 0x4C)` / `+ 0x54` become
  `D_800A3708->work.t[0]` / `[2]`, with D_800A3708 declared `Unk80101DF0Record *` (code6cac_c2's type).
  `work` is at +0x38 and `t` at +0x14 in it, so the same two `lw` at the same offsets from the same pointer.
- **game_StageCleanup (sound.c, step 08).** `(s32 a0)` -> `(s32 a0, s32 a1)`, and `func_800460E4(a0)` ->
  `func_800460E4(a0, a1)`. Its caller in text1b passes two arguments ($a1 set) and func_800460E4 uses its
  second; $a1 arrives and leaves in the same register untouched, so no instruction is added.
- **game_GetPlayerData (sound.c, step 08).** `(void)` -> `(s32 a0)`, forwarding `a0` to func_8004153C (defined
  `(s32 a0)`). Every caller passes the player index; $a0 passes through untouched.
- **func_800486FC (text1b.c, step 08).** Return type `s16` -> `s32`. Its body returns the `lh` of an s16 global
  (already sign-extended, same bytes); its callers in sound and text1b test all 32 bits (an s16 declaration
  makes them add `sll 16`).
- **func_80049F4C (text1b.c, step 08).** `__builtin_memcpy(sp10, &D_800153F0, 44)` on `s32 sp10[11]` becomes
  `sp10 = D_800153F0;` on a `Unk800153F0Record` (22 halfwords; func_8004A09C walks it as u16). The target's
  copy has a run-time alignment test; a struct assignment uses the type's 2-byte alignment, which emits the
  same test. D_800153F0's definition (text1a_b_pre_rodata.c) becomes the same record, same bytes. Owner question 2.
- **func_8006E950 (text1b_tu1d.c, step 10).** `s32 *a0` + `s0_addr = (s32)a0` -> `s32 a0` + `s0_addr = a0`
  (callers pass integers 0x32 / 0x5F; the body uses it only as an integer).
- **Cast-only call-site edits (steps 08 / 10)**, no statement added or removed: sound (func_80045694 callbacks
  as `(s32)`, ApplyMatrix / gte_MulMatrix0ClearTrans SDK types, func_80044FA0 `(s32)v0`, D_800A3708->xf.rot,
  D_800F62E0[0]), text1a_c2 (func_80044010 `(s32 *)` first argument), text1b (`(s32)func_8004153C(..)`,
  `(s32 *)p1/p2`, `(s32)&D_80102C00`, D_800F62E0[0]), text1b_b (`(s32 *)D_800A35F4/360C`, `(s32)&s.header`,
  `(s32)&s.a`, `(s32 *)D_8009BD24`).
- **func_80060A68 (text1b_tu1c.c, step 14).** Its block-scope `extern struct Ob *D_800A3468` goes (the file
  declares it `s32` everywhere else; it becomes one static). The function reads it through
  `#define OB ((struct Ob *)D_800A3468)`, the spelling text1b.c uses for its own s32 work pointer. Same load of
  the same word, same member offsets.
- **func_800420D0 (text1a_post.c, step 14).** Its block-scope `extern s16 g_anim_hit_flags;` goes; it writes
  `g_anim_hit_flags[0] = 0` through the file's one array declaration. Same `sh` to the same address.

## Rule fit (per-file-gp-model.md as amended A1-A3, 7679f31f4)

- **maspsx -G8 (owner ruling Q69).** Makefile: `$(if $(filter $1,$(PSYQ_LIBRARY_FILES)),, -G8)`, mirrored in
  `engine/buildconfig.py` / `engine/pipeline.py`. `PSYQ_LIBRARY_FILES` is generated by the new
  `tools/psyq_library_files.py`: every file whose whole .text lies inside the provenance census's library span
  (memory/closer/psyq-library-census.md, 0x80078948..0x8008D070; 177 verbatim PsyQ 4.0 placements plus the
  newer-build LIBSND/LIBSPU gaps). At c800ccfe7: text1b_b_tu2 text1b_b_tu3 gpu display system ings2 main comb
  (none has a gp access, as -G0 predicts; text1b_b and main_post straddle the span and are game files). New
  engine test `test_psyq_library_files`: Makefile == buildconfig, and == the evidence set over the link map.
  Proof (`g8_test.sh`): -G8-unless-library and the per-file reach set build every object byte-identical;
  -G8 on everything changes only system.o (its 3-byte static Intr moves to .sdata; link fails).
- **Gap clause (owner ruling Q71).** One object per piece, exact size, named `D_<addr>` (or by the blob's own
  label where some code or data references it). Alignment-only gaps are not defined (13; the next object's
  alignment, or SUBALIGN(2) after an odd block end, produces them). 20 pieces fit no single object; Q71 splits
  them into the smallest aligned `D_<addr>` pieces (logged `INTERIM` in /tmp/q56/s10_log.txt).
- **Merges.** No member boundary is phase-proven (`m34_evidence.md` section 4); the text1a_c tail shares no
  object with M3 and stays its own file.

## Owner rulings spent by the series (answered as recommended)

Q69 (-G8 except Sony library code, evidence-defined), Q70 (D_80102C00 s32; D_800153F0 a 22-halfword struct;
func_8004153C unprototyped for now), Q71 (smallest aligned D_<addr> pieces), Q72 (text1a_b_pre_rodata.c joins
M3). Their rule text must land (rules: commit) before the series is applied.
