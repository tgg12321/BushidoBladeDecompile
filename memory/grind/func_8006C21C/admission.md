# func_8006C21C — admission records (s8, 2026-09-28)

Admission evidence for the three reused/holder locals in candidate.c, written against the s8
candidate (setXYWH bar 1, chained 0x80 stores, `mode` set once at the top, `i` renamed `work`).
These records do NOT make the function landable: the frame gap (4 untouched slots) and the r3
store order remain (floor 41), and the colour-variable loosening was withdrawn by the owner on
2026-09-28 (decisions.md "per-branch constants read only inside their branch: approved, then
WITHDRAWN"). A landing still needs sandbox 0, the oracle, and a fresh layer-2 on the final body.

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
  Data gap, stated honestly: the census of which sheets the four sites reach (all single-header,
  as K = 0xC assumes) is not yet done for this function's sprite set (`*(arg0[1] + 0x30)`).
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

(sandbox --disable all; pending — the peer session's staged src edit blocks engine runs)

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
