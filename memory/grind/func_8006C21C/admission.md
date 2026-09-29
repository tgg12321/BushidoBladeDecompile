# func_8006C21C — admission records (s8, 2026-09-28)

Admission evidence for the three reused/holder locals in candidate.c, written against the s8
candidate (setXYWH bar 1, chained 0x80 stores, `mode` set once at the top, `i` renamed `work`).
Update s9 (2026-09-28): the frame and colour gaps are closed by the owner-granted Q27 FAKE locals
(rules commit 32a6b3626); the landing body is landing-body-q27.c (sandbox 0/622, full-build SHA1 ==
oracle, measured 2026-09-28). Its first layer-2 FAILed on this file's `cells` (b) census gap and on
`work`'s (D) proof, which covered only lvl10; both are now banked below (§ `cells` (b) census,
§ `work` (D) on the Q27 landing body). These are the author's records; admission is a fresh
layer-2's call.

Instruments (all in probes/s8/tools, run from tmp/c21c/): `orph.py` (cc1 `.s` + frame),
`dump.sh -dl -dg`, `alloc.sh` (BB2_ALLOC_DEBUG from the private /tmp/gccdbg cc1, output checked
byte-identical to tools/gcc-2.7.2/build/cc1 on the same `.i`), `greg.py`, permuter workspaces
built by `mkperm.py` (standalone TU checked identical to the full TU modulo label names).
Command line for every compile: `tools/gcc-2.7.2/build/cc1 -O2 -G0 -funsigned-char -quiet
-mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float <tu>.i` (cpp as engine.buildconfig).

## `work` — Ruling 11 (reused local proven necessary by the allocator)

Values (writes that reach a common read): V2 phase-2 unlock-bit index (`for (work = 0; work < 4;
work++)`), V4 phase-4 sprite index (`< 6`), V6 phase-6 tile row (`< 11`), V8 phase-8 gauge level
(`work = *(s16 *)(D_800A34FC + j * 2 + 0x28)`, compared with 5 and used as `recs[work + 1]`).
- (A) function-scope local, declared once, address never taken, not static/register.
- (B)(1) every write is read (loop tests/indices; the level by the compares and the index).
  (B)(2) no write re-stores a held value: each loop init follows a loop that exits with the
  counter at 4 / 6 (a different value); the level is a fresh load each j iteration.
- (C)(1)-(2) one-variable-per-value spelling: `spl_all` (four locals `i_p2/i_p4/i_p6/i_lv`),
  same statement list. (C)(3) V2/V4/V6 are counters (arithmetic increments), V8 is a load.
- (D)(1) dumps: `.lreg`/`.greg` for cand10 and lvl10 (tmp/c21c/out, regenerable with dump.sh),
  plus BB2_ALLOC_DEBUG global-allocation order (below).
- (D)(2) mechanism: global.c allocates allocnos in priority order
  (floor_log2(refs) * refs / live_length, the `pri` column). The target keeps `j` in a stack slot
  (0x50) and the counter/level in `$fp`. With the merged variable the pseudo has 40 refs over
  live length 353, pri 5665, and is allocated `$fp` (ord 24) before `j` (refs 34, pri 5105,
  ord 26), which finds no register and is spilled:
  `ord=24 pseudo=81 hardreg=30 nrefs=40 livelen=353 pri=5665` / `ord=26 pseudo=73 hardreg=-1 nrefs=34 livelen=333 pri=5105`.
  With a separate `level` (lvl10): the counter (refs 28, pri 10275) takes `$s4` early, `j` takes
  `$fp` (`ord=26 pseudo=73 hardreg=30`), and the level (refs 12, livelen 244, pri 1475) is spilled
  (`ord=33 pseudo=82 hardreg=-1`).
- (D)(3) necessity: the target needs `j` spilled while the level value sits in `$fp` across the
  phase-8 loops, where both are live and conflict. `j`'s refs and live range are fixed by the
  target's instruction stream (every `lw/sw 0x50(sp)`), as are the level's: one `lh`, two compares
  per row and one index computation per j iteration, live from the `lh` through the row loop.
  Any spelling in which the level is its own variable gives that pseudo exactly those refs and that
  live range, so its priority (1475 measured) stays below `j`'s (5105) and global.c gives `$fp` to
  `j` first; declaration order, scope and type do not enter the priority (measured: s32, s16 and
  block-scoped `level` compile identically, lvl_s32/lvl_s16/lvl_block). Only merging the level's
  refs with the phase-2/4/6 counters' refs raises the pseudo above `j`.
- (D)(4) measured alternatives (cc1 `.s` vs the candidate; sandbox scores: see "Scores" below):
  spl_p2 (phase-2 counter split out) 87 lines differ; spl_p4 5; spl_p6 3; spl_lv (level split out,
  = lvl10) 59; spl_all 81. Permuter campaign from lvl10 (tmp/c21c/perm_lvl10, label
  r11-level-split, -j 2, --stack-diffs): see "Campaigns".
- (E) name `work`: a generic scratch word (E)(i); the values are different kinds (indices, a level).
- (F) annotation at the declaration in candidate.c.

### `work` (D) on the Q27 landing body (s9, 2026-09-28; supersedes the cand10/lvl10 receipts above)
Reuse spelling = landing-body-q27.c (= tmp/c21c9/land_sb.c). One-variable-per-value spelling
(C)(1) = probes/s9/work-r11/w_spl_all.c: `bit` (declared in the phase-2 `if` body), `sprite`,
`trow` (function scope, top-level loops), `level` (declared in the phase-8 j-loop body); same
statement list as the reuse body (tmp/c21c9/mk_split9.py renames identifiers only).
- (D)(1) dumps, both spellings (probes/s9/work-r11/dumps/: `<n>.excerpt.txt` = every `.lreg`
  "Register N used" summary + the `.greg` register dispositions for func_8006C21C; `<n>.allocdbg` =
  the full BB2_ALLOC_DEBUG global.c order; the full `-dl -dg -df` dumps regenerate with
  work-r11/alloc9.sh after tmp/c21c/orph.py builds the `.i`). Commands:
  `tools/gcc-2.7.2/build/cc1 -O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls
  -fno-builtin -w -mel -msoft-float -dl -dg -df <tu>.i` and `BB2_ALLOC_DEBUG=1
  tools/gcc-2.7.2/cc1 <same flags> <tu>.i`; the diagnostic cc1's func_8006C21C `.s` is identical
  to the build cc1's (842 / 820 lines; the TU files differ only in the `-d` flags echoed in the
  header comment), tmp/c21c9/fncmp.sh.
- (D)(2) global.c allocno priority order (floor_log2(refs) * refs / live_length):
  reuse: `ord=27 pseudo=81 hardreg=30 nrefs=40 livelen=353 pri=5665` (work -> $fp) before
  `ord=28 pseudo=73 hardreg=-1 nrefs=34 livelen=333 pri=5105` (j spilled to 0x50) = the target.
  one-variable-per-value: `ord=28 pseudo=73 hardreg=30 nrefs=34 livelen=333 pri=5105` (j -> $fp)
  and `ord=40 pseudo=234 hardreg=-1 nrefs=12 livelen=244 pri=1475` (level spilled) = not the target.
- (D)(3) unchanged (above): the level's own pseudo has the target-fixed refs/live range whatever
  its declaration, scope, type or statement order, so its priority stays below j's.
- (D)(4) `sandbox --disable all` (main 2d2a64e6d-era build/, 2026-09-28):
  | spelling | score | insns |
  |---|---|---|
  | reuse (landing body) | 0 | 622 |
  | full one-variable-per-value (w_spl_all, block-scoped bit/level) | 91 | 605 |
  | same, all four at function scope | 90 | 605 |
  | ablation: phase-2 bit split out | 93 | 611 |
  | ablation: phase-4 sprite split out | 5 | 622 |
  | ablation: phase-6 tile row split out | 3 | 622 |
  | ablation: phase-8 level split out | 69 | 605 |
  | structural: split + counters as `while` loops | 91 | 605 |
  | structural: split + `s16 level` | 91 | 605 |
  | structural: split + bar tests re-read the level from memory | 241 | 587 |
  | structural: split + bar tests compare `rec == &recs[6]` | 159 | 623 |
  Permuter campaign from the one-variable-per-value body (tmp/c21c9/perm_w_spl_all,
  -j 2, --stack-diffs, --stop-on-zero), two windows on the same seed (harvest telemetry in
  metrics/events.jsonl): `r11-spl-all-q27`, launched 2026-09-28T23:47:27Z, harvested after 114.3 s,
  677 iterations; `r11-spl-all-q27-long`, launched 23:50:51Z, harvested and stopped at 1126.6 s,
  1342 iterations. Base 4313; best 1695 (first window), best of the long window 1855. The reuse
  body scores 0 on the same scorer (workspace perm_w_reuse, base_score 0). What the finds reuse:
  the best (1695) reuses the phase-4 counter `sprite` as a phase-8 temp (`sprite = rec[row].x;`),
  i.e. the search moves back toward merging values into one variable; others add `new_var = poly`
  aliases (2030, 2245) or rewrite a loop test (`(sprite + 1) <= 6`, 1855). No find that keeps the
  four values separate approaches the reuse body. Best find banked as
  probes/s9/work-r11/perm-spl-all-best-1695.c.

## `cells` — Ruling 9 (one meaning, several writes at a constant offset)

Four writes, each `cells = s.header + 0xC;` immediately followed by `s.table = cells;` (phase 1;
phase-2 inner `if`; phase-4 head; phase-4 `work` loop).
- (a) one consumer, `s.table = cells;`, textually identical at every site.
- (b) BASE `s.header` (`u8 *`, a member of the local descriptor), K = 0xC at every write, no cast.
  BASE is a sprite sheet (`table[k]` of the gauge sprite set); BASE + 0xC is its cell array.
  Layout, independent of this function: `SprtHdrA` (text1b.c:12448-12456, 12 bytes: count +2,
  cx/cy +4/+6, ubase +8, vbase +0xA) and `SprtEntA` (text1b.c:12458-12462, 8 bytes);
  func_8007352C (text1b.c:12464-12507, COMPLETED-C) reads `env->header` as `SprtHdrA` and walks
  `env->table` as `SprtEntA[]`; func_80073728's `Ft4Sheet`/`Ft4Cell` (text1b.c:12512-12527) has the
  same 12-byte header + 8-byte cells; other builders store `header + 0xC` into the same field
  (text1b.c:12422 `s1 = idx + 0xC`; func_800753D8 `body = s.sp18 + 0xC`; func_8007636C `cells`).
  **Census (s9, closes the former data gap).** The sprite set is disc/TIM2D/MOD.BIN: arg0[1] =
  D_800A34FC->0x24 (func_8006E390) = the buffer func_80068F70 fills with func_8006E950(2, buf) ->
  func_80036EA8(2, 2) = D_8008F12C[2] + 2 = CD file #23, whose g_cd_file_table entry size 0xA0608
  is exactly MOD.BIN's size (the same decoder maps file #156 to MOVOVL.EXE, as the source comment
  on D_8008F12C[6] says). Header word +0x30 (relocated by func_8006E440) -> sheet table @0x5F8,
  whose entries func_8006919C relocates (header words +0x14..+0x40). Every slot the four sites can
  reach, tested two ways (header shape: tp1 == 0, pad == 0, count >= 1, CLUT row 480..511; cells
  after the headers all have nonzero w/h), is a ONE-header sheet, so K = 12 * 1 = 0xC at every site:
  | site | slots | headers N | K |
  |---|---|---|---|
  | phase 1 | table[0] (@0x1438, 4 cells) | 1 | 0xC |
  | phase-2 inner | table[13..16] (@0xB60/0xB74/0xB88/0xB9C, 1 cell each) | 1 | 0xC |
  | phase-4 head | table[1] (@0xB00, 6 cells) | 1 | 0xC |
  | phase-4 loop | table[2..7] (@0xB3C/0xA08/0xA24/0xA40/0xA5C/0xA70; 3,2,2,2,1,1 cells) | 1 | 0xC |
  Independent check: consecutive sheets are exactly 12 + 8 * count apart (0xA08 -> 0xA24 = 0x1C
  for 2 cells; 0xA5C -> 0xA70 and 0xB60 -> 0xB74 = 0x14 for 1 cell). Receipts:
  probes/s9/cells-census/ (mod_sheet_census.py + output, cdfile.py).
- (c) each write is read once, by the next statement, in the same compound statement.
- (d) the addition is in the target at every site (`addiu $a1,$v0,0xC`).
- (e) every consumer store is followed by a read of `s` (func_8007352C) before the next store
  (the phase-4 loop's j loop always runs twice).
- (f) name `cells`: the cell array, true of every write.
- (g) Ruling 5 prong 2: each write follows a reassignment of `s.header`; on the path where the
  phase-2 body never runs, the phase-4-head write still changes the value (table[0] -> table[1]).
- (h) declared once, function scope (the innermost scope enclosing all four writes).
- (i) receipts: carrier-free spelling nocells10 (`s.table = s.header + 0xC;` at every site):
  12 lines differ; the per-site sums land in $v0/$v1 (`addu $2,$2,12`) where the target and the
  carrier form put them in $a1 (`addu $5,$2,12`), because the carrier is one cross-block pseudo
  (pseudo 86, refs 14) allocated by global.c to $a1, while per-site temps are local-alloc'd to the
  first free register. Earlier sessions: per-site expression 59 (s2), direct cells 57 (s7).
  Permuter campaign from nocells10 (tmp/c21c/perm_nocells10, label r9-cells-free): see "Campaigns".

## `mode` — named-local-fake-exception (constant holder across calls)

`s32 mode = 0;` set once at the top, passed as func_8006E480's second argument at all three calls.
- (1) exhaustion: literal 0 at every call (modelit10) passes `$0` at the phase-3/5 calls where the
  target passes `$s5` (`move $5,$21`); s7 measured the literal form at 48/622.
- (2) mechanism: the once-set constant pseudo (pseudo 80, REG_EQUIV 0) is live across every call,
  so global.c gives it callee-save `$s5`; cse folds the phase-1 read to 0 inside the entry block
  (target `addu $a1,$zero,$zero` there).
- (3) `/* FAKE: ... */` annotation at the declaration in candidate.c.
- (4) layer-1 + layer-2 at any landing. Siblings with the same holder: func_800753D8 `zero`,
  func_800759D0 (`$fp`), func_8007636C `mode`.

## Scores

`sandbox --disable all`, 2026-09-28, main e3cd78f85 (score / build insns vs target 622):

| spelling | score | insns | scored hunks |
|---|---|---|---|
| candidate.c (work shared, cells carrier, mode holder) | 41 | 622 | 4 source-level (the two early r3 stores) |
| spl_p6 (phase-6 row split out) | 44 | 622 | 4 source-level |
| spl_p4 (phase-4 index split out) | 46 | 622 | 4 source-level |
| lvl10 = spl_lv (level split out) | 108 | 605 | 28 source-level |
| spl_all (all four values split) | 130 | 605 | 28 source-level |
| spl_p2 (phase-2 index split out) | 133 | 611 | 31 source-level |
| nocells10 (no cells carrier) | 53 | 622 | 12 source-level |
| modelit10 (literal 0 at every func_8006E480 call) | 44 | 622 | 5 source-level |

Every alternative is worse than the candidate; the candidate's own residual (frame + r3 order) is
unrelated to these three constructs.

## Campaigns

Scorer: decomp-permuter with --stack-diffs against the s2 workspace's target.o (standalone TU,
compile.sh = the project pipeline). Reference: the target-shaped candidate (cand10) scores 1180
on this scorer (its whole residual is the frame and the r3 order); lower is closer.
- `r11-level-split` (tmp/c21c/perm_lvl10, seed lvl10 = separate `level`, base 5331): 968 s,
  4577 iterations, -j 2, stopped by hand. Best 1863 (output-1863-1): keeps `level` separate but
  adds `int new_var = 0;` and stores it in place of the literal 0 colour stores (a zero
  constant-holder; not admissible, and still 683 above the reference). No find reaches 1180.
- `r9-cells-free` (tmp/c21c/perm_nocells10, seed nocells10 = no carrier, base 2000): 967 s,
  4635 iterations, -j 2, stopped by hand. Best 1646 (output-1646-1): re-introduces a shared
  `u8 *new_var = s.header + 0xC;` carrier and stores it at later sites without recomputing it
  (a semantics change). The search moves toward the `cells` carrier; no carrier-free find
  reaches 1180.
Both campaigns were still producing new finds when stopped, all of them through constructs that
re-introduce a holder/carrier; banked here as evidence, not as a closing claim.
